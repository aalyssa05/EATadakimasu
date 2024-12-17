#include <ncurses.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

//declaration of const global variables
#define maxSCORE 150
#define frameTIME 110000

//declaration of struct for coordinate and direction use
typedef struct{
    int x; int y;
} cord;

//declaration of struct for foods
typedef struct{
    char symbol;
    cord position;
    int scoreEffect;
    int healthEffect;
    float spawnRate;
    int despawnTime;
    time_t spawnTime;
    bool active;
} food;

//declaration of global variables

//main game parameters
int score = 0;
int health = 3;

//string to display the main game parameters
char scoreDisp[12];
char healthDisp[12];

//assigning values to snake structs
cord head = {1, 1};
cord direction = {1, 0};
cord body[maxSCORE + 1];

//assigning values to food structs, index[0] is sushi, index[1] is rat, and index[2] is takoyaki
food foods[3] = {
    { .symbol = '+', .position = {1, 1}, .scoreEffect = 1, .healthEffect = 0, .spawnRate = 1, .despawnTime = 0, .spawnTime = 0, .active = false }, 
    { .symbol = '@', .position = {1, 1}, .scoreEffect = 0, .healthEffect = -1, .spawnRate = 0.4, .despawnTime = 8, .spawnTime = 0, .active = false }, 
    { .symbol = 'O', .position = {1, 1}, .scoreEffect = 5, .healthEffect = 0, .spawnRate = 0.03, .despawnTime = 10, .spawnTime = 0, .active = false }
};

//game-loop condition parameter variables
bool isRunning = true;
bool skip = false;

//initialization of screen variable
WINDOW *win;

//window size information
int screenWidth = 60;
int screenHeight = 24;

//initializes the program screen
void initProg(){
    srand(time(NULL));

    //initialize screen
    initscr();
    win = newwin(screenHeight, screenWidth, 0, 0);
    refresh();

    //setup for player inputs; take input and hide cursor
    noecho();
    nodelay(win, true);
    keypad(win, true);
    curs_set(0);

    //makes border
    box(win, 0, 0);
}

/*clears the current contents of the screen; for scene transitions
also redisplays box and refreshes window contents*/
void clearWin(){
    werase(win);

    box(win, 0, 0);

    wrefresh(win);
}

//waits for user key to enter the space bar to return a scene change signal to clear window
void exitSceneSig(){
    int pressed;

    while((pressed = wgetch(win)) != ' '){
    }

    clearWin();
}

//returns the coordinate that would align string to center column position
int centerCol(const char *st){
    return (screenWidth - strlen(st)) / 2;
}

//returns the coordinate that would align string to center row position
int centerRow(const char *st){
    return screenHeight / 2;
}

//executes intro scene
void introScene(){
    //declare messages
    const char *title = "EATadakimasu!";
    const char *details = "a snake game rendition for cmsc 11";
    const char *names = "by campomanes, dela cruz, & latata";
    const char *instruct = "use your arrow keys to move!";
    const char *types[4] = {"food types:", "+ - adds 1 to score", "@ - removes 1 to health", "O - adds 5 to score"};
    const char *con = "PRESS [SPACEBAR] TO CONTINUE";
    const char *dedic = "inspired by @abuharth on github";

    //set the print coordinates 
    int titleCol = centerCol(title), titleRow = 4;
    int detailsCol = centerCol(details), detailsRow = titleRow + 1;
    int namesCol = centerCol(names), namesRow = titleRow + 2;
    int instructCol = centerCol(instruct), instructRow = namesRow + 3;
    int typesCol[4], typesRow[4];
    for(int i = 0; i < 4; i++){
        typesCol[i] = centerCol(types[i]);
        typesRow[i] = instructRow + 3 + i;
    }
    int conCol = centerCol(con), conRow = typesRow[3] + 3;
    int dedicCol = centerCol(dedic), dedicRow = screenHeight - 2;

    //prints the messages to the assigned coordinates
    mvwprintw(win, titleRow, titleCol, "%s", title);
    mvwprintw(win, detailsRow, detailsCol, "%s", details);
    mvwprintw(win, namesRow, namesCol, "%s", names);
    mvwprintw(win, instructRow, instructCol, "%s", instruct);
    for(int i = 0; i < 4; i++){
        mvwprintw(win, typesRow[i], typesCol[i], "%s", types[i]);
    }
    mvwprintw(win, conRow, conCol, "%s", con);
    mvwprintw(win, dedicRow, dedicCol, "%s", dedic);

    //for next scene transition
    exitSceneSig();
}

//executes message scene depending on status argument
void messageScene(bool winStatus){
    //declare messages
    const char *winMsg = "congrats, you won :)!";
    const char *loseMsg = "game over... better luck next time :(";
    const char *finalMsg;
    const char *scoreMsg = "FINAL SCORE: ";
    const char *con = "PRESS [SPACEBAR] TO CONTINUE";

    //set the correct message to the final message depending on the win status
    if(winStatus){
        finalMsg = winMsg;
    }
    else{
        finalMsg = loseMsg;
    }

    //set print coordinates
    int finalMsgCol = centerCol(finalMsg), finalMsgRow = centerRow(finalMsg) - 2;
    int scoreMsgCol = centerCol(scoreMsg) - 1, scoreMsgRow = finalMsgRow + 1;
    int conCol = centerCol(con), conRow = scoreMsgRow + 7;

    //print the message
    mvwprintw(win, finalMsgRow, finalMsgCol, "%s", finalMsg);
    mvwprintw(win, scoreMsgRow, scoreMsgCol, "%s %d", scoreMsg, score);
    mvwprintw(win, conRow, conCol, "%s", con);

    exitSceneSig();
}

//detects inputs and changes direction of snake
void inputHandling(){
    //get pressed keys from user
    int pressed = wgetch(win);

    /*changes directions according to key pressed
    also includes input validation for keys pressed in the opposite direction as the current direction of the snake
    the input validation is only applicable if the snake has a body*/
    if (pressed == KEY_LEFT) {
        if (direction.x == 1 && score != 0) {
            skip = true;
            return;
        }
        direction.x = -1;
        direction.y = 0;
    }
    if (pressed == KEY_RIGHT) {
        if (direction.x == -1 && score != 0) {
            skip = true;
            return;
        }
        direction.x = 1;
        direction.y = 0;
    }
    if (pressed == KEY_UP) {
        if (direction.y == 1 && score != 0) {
            skip = true;
            return;
        }
        direction.x = 0;
        direction.y = -1;
    }
    if (pressed == KEY_DOWN) {
        if (direction.y == -1 && score != 0) {
            skip = true;
            return;
        }
        direction.x = 0;
        direction.y = 1;
    }
}

//checks whether two points collides
bool collide(cord a, cord b){
    if(a.x == b.x && a.y == b.y){
        return true;
    }
    else{
        return false;
    }
}

//check whether the a point collides with its body
bool collideBody(cord point){
    for(int i = 0; i < score; i++){
        if(collide(point, body[i])){
            return true;
        }
    }
    return false;
} 

//spawns foods and updates the attributes of the food
void spawnFood(food *foodType){
    /*checks if the food is not spawned yet. and also randomly generates a value and checks if it is within the spawnRate.
    if it satisfies the conditions inside, it updates the corresponding values of the fruit for tracking
    */
    if(!foodType->active && (rand() % 100 < (int)(foodType->spawnRate * 100))){
        //the loop also makes sure that the food doesn't spawn on the coordinates of the snake
        do{
            foodType->position.x = 2 + rand() % (screenWidth - 2);
            foodType->position.y = 2 + rand() % (screenHeight - 2); 
        }while(collideBody(foodType->position) || collide(head, foodType->position));
        foodType->spawnTime = time(NULL);
        foodType->active = true;
    }
}

//despawns the fruit that has reached its maximum time
void despawnFood(food *foodType){
    //checks if the food is spawned and if the time since it has spawned is greater than or equal to the time that it is supposed to stay in-game
    if((foodType->active && foodType->despawnTime > 0) && (difftime(time(NULL), foodType->spawnTime) >= foodType->despawnTime)){
        foodType->active = false;
    }
}

//handles the snake and food collisions
void foodCollision(food *foodType){
    //if snake head collides with food it updates the score and health correspondingly and sets the food to an inactive state
    if(foodType->active && head.x == foodType->position.x && head.y == foodType->position.y){
        score += foodType->scoreEffect;
        health += foodType->healthEffect;
        foodType->active = false;
    }

    //updates the score and health display
    sprintf(scoreDisp, "SCORE: %d", score);
    sprintf(healthDisp, "HEALTH: %d", health);

    /*conditions for winning or losing
    player wins if the player reaches a score that is greater than the set max score
    player loses if the player's health reaches to or below 0*/
    if(score >= maxSCORE){
            isRunning = false;
            clearWin();
            messageScene(true);
        }
    if(health <= 0){
        isRunning = false;
        clearWin();
        messageScene(false);
    }
}

//updates the game parameters (health & score), checks for any win or lose conditions, spawns food
void updateParam(){
    //updates snake body position
    for(int i = maxSCORE; i > 0; i--){
        body[i] = body[i - 1];
    }
    body[0] = head;

    //moves snake
    head.x += direction.x;
    head.y += direction.y;

    /*handles collision will body and walls
    if satisfies the condition, it stops the loop and transitions to the losing game scene*/
    if (collideBody(head) || head.x <= 0 || head.y <= 0 || head.x >= screenWidth - 1 || head.y >= screenHeight - 1){
        isRunning = false;
        clearWin();
        messageScene(false);
    }

    //handles food collision
    for (int i = 0; i < 3; i++) {
        foodCollision(&foods[i]);
    }

    //handles spawning and despawning food
    for (int i = 0; i < 3; i++) {
        spawnFood(&foods[i]);
        despawnFood(&foods[i]);
    }

    usleep(frameTIME);
}

//display all the elements of the game scene
void displayGame(){
    //clear screens for every game update
    werase(win);

    //displays snake body
    for(int i = 0; i < score; i++){
        mvwaddch(win, body[i].y, body[i].x, ACS_CKBOARD);
    }

    //displays snake head
    mvwaddch(win, head.y, head.x, ACS_DIAMOND);

    //displays all active food
    for(int i = 0; i < 3; i++){
        if (foods[i].active) {
            mvwaddch(win, foods[i].position.y, foods[i].position.x, foods[i].symbol);
        }
    }

    //display the game border, score, and health
    box(win, 0, 0);
    mvwprintw(win, 0, 2, " %s ", scoreDisp);
    mvwprintw(win, 0, screenWidth - 15, " %s ", healthDisp);
}

//executes the game scene
void gameScene(){
    //initializes the score and health displays
    sprintf(scoreDisp, "SCORE: %d", score);
    sprintf(healthDisp, "HEALTH: %d", health);

    //loop to continually run the following functions of the game
    while(isRunning){
        inputHandling();
        //this condition handles the input validation of inputHandling
        if(skip == true){
            skip = false;
            continue;
        }
        updateParam();
        displayGame();
    }

}

//exits the program properly
void quitProg(){
    endwin();

    //clear screen, place cursor on top, and un-hide cursor
    printf("\e[1;1H\e[2J");
    printf("\e[?25h");

	exit(0);
}

//main function where the functions will be executed
int main(){
    //starts with initializing the window screen
    initProg();

    //then proceeds to execute the intro scene
    introScene();

    //and then the game scene; message scene not needed to be called here because it's already called in the specific conditions under the functions under game scene
    gameScene();

    //quits programs properly
    quitProg();

    return 0;
}
