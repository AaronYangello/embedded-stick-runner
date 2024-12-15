/*
 * stickrun.c
 *
 *  Created on: Mar 15, 2018
 *      Author: Aaron
 */

#include "system.h"
#include "random_int.h"
#include "stddef.h"
#include "strings.h"
#include "game.h"
#include "timing.h"
#include "task.h"
#include "terminal.h"
#include "random_int.h"

#define MAP_WIDTH 80
#define MAP_HEIGHT 6
#define MIN_OBSTACLE_RATE 500
#define MAX_OBSTACLE_RATE 8000
#define MIN_BONUS_RATE 15000
#define MAX_BONUS_RATE 40000
#define MAX_OBSTACLES 8
#define MAX_BONUSES 2

/// game structure
struct stickrun_t {
    char x; // x coordinate of character
    char y; // y coordinate of character
    char c; // Character of player
    int score; // Score for the round
    int obstaclesPassed; //Number of obstacles cleared
    uint8_t id; // ID of game=
};

typedef struct {
    char x; // x coordinate of bonus character
    char y; // y coordinate of bonus
    char c; // Character of bonus
    char status; // Status of bonus
    int value; // Value of bonus
} bonus_t;

static struct stickrun_t game;
static char_object_t obstacles[MAX_OBSTACLES];
static bonus_t bonuses[MAX_BONUSES];

static void Help(void);
static void Play(void);
static void Receiver(uint8_t c);
static void Jump(void);
static void MoveCharacterUp(void);
static void MoveCharacterDown(void);
static void SendObstacle(void);
static void SendBonus(void);
static void MoveObstacle(char_object_t * obstacle);
static void MoveBonus(bonus_t * bonus);
static void CheckCollisonObstacle(char_object_t * obstacle);
static void CheckCollisonBonus(bonus_t * bonus);
static void RemoveObstacle(char_object_t * obstacle);
static void RemoveBonus(bonus_t * bonus);
static void ScorePoint(uint8_t points);
static void GameOver(void);
static void DrawRect(char x_min, char y_min, char x_max, char y_max);

void StickRun_Init(void) {
    game.id = Game_Register("StickRun", "A run and jump game...fun...", Play, Help);
}

void Help(void) {
    Game_Printf("Press SPACEBAR to jump over obstacles as they appear.");
}

void Play(void) {
    //Clear Screen
    Game_ClearScreen();

    //Draw play area
    DrawRect(0, 0, MAP_WIDTH, MAP_HEIGHT);

    //Initialize game variables
    game.x = 10;
    game.y = MAP_HEIGHT - 1;
    game.c = 178;
    game.score = 0;

    //Draw character in place
    Game_CharXY(game.c, game.x, game.y);

    //Set Cursor to Top left corner
    Game_CharXY(186, 0, 1);
    //Display Score
    Game_Printf("Score: %d", game.score);

    //Register reveiver for the game
    Game_RegisterPlayer1Receiver(Receiver);

    //Hide Cursor
    Game_HideCursor();

    //Send first obstacle in 5 seconds
    Task_Schedule(SendObstacle, 0, 5000, 0);

    //Send first bonus between 10 and 15 seconds
    Task_Schedule(SendBonus, 0, random_int(MIN_BONUS_RATE, MAX_BONUS_RATE), 0);
}

void Receiver(uint8_t c) {
    switch(c) {
         case ' ':
             Jump();
             break;
         default :
             break;
    }
}

void Jump(void) {
    uint16_t jumpSpeed = 100;
    //Check to see if character is on the ground
    if(game.y == (MAP_HEIGHT - 1)) Task_Schedule((task_t)MoveCharacterUp, &game, 0, jumpSpeed);
}

void MoveCharacterUp(void) {
    uint16_t jumpSpeed = 100;
    //Clear location
    Game_CharXY(' ', game.x, game.y);
    //Update Location
    game.y--;
    Game_CharXY(game.c, game.x, game.y);

    if(game.y <= MAP_HEIGHT - 4){
        Task_Remove(MoveCharacterUp, &game);
        Task_Schedule((task_t)MoveCharacterDown, &game, 4*jumpSpeed, jumpSpeed);
    }
}

void MoveCharacterDown(void) {
    //Clear location
    Game_CharXY(' ', game.x, game.y);
    //Update Location
    game.y++;
    Game_CharXY(game.c, game.x, game.y);

    if(game.y >= MAP_HEIGHT - 1) Task_Remove(MoveCharacterDown, &game);
}

void SendObstacle(void) {
    //Start on ground
    char y = MAP_HEIGHT - 1;
    //Iterator variable
    volatile uint8_t i;
    //Obstacle
    char_object_t * obstacle = 0;
    //Random move rate of obstacle
    uint16_t move_period = 250;
    if(game.obstaclesPassed) move_period = 250 - 2*game.obstaclesPassed;

    //Find unused obstacle
    for(i = 0; i < MAX_OBSTACLES; i++) if(obstacles[i].status == 0) obstacle = &obstacles[i];
    if(obstacle){
        obstacle->status = 1;
        obstacle->x = MAP_WIDTH - 1;
        obstacle->c = 'o';
        obstacle->y = y;

        //Use task scheduler to move obstacle left at the random move period
        Task_Schedule((task_t)MoveObstacle, obstacle, move_period, move_period);
        Game_CharXY('o', MAP_WIDTH - 1, y);
    }

    //Update period of MoveObstacle
    Task_ChangePeriod((task_t)MoveObstacle, move_period, 1);

    //Schedule next obstacle to come at a random time
    Task_Schedule(SendObstacle, 0, random_int(MIN_OBSTACLE_RATE, MAX_OBSTACLE_RATE), 0);
}

void SendBonus(void) {
    char y = MAP_HEIGHT - 4;

    //Iterator variable
    volatile uint8_t i;
    //Bonus
    bonus_t * bonus = 0;

    uint16_t move_period = 250;
    if(game.obstaclesPassed) move_period = 250 - 2*game.obstaclesPassed;

    for(i = 0; i < MAX_BONUSES; i++) if(bonuses[i].status == 0) bonus = &bonuses[i];
    if(bonus) {
        bonus->status = 1;
        bonus->x = MAP_WIDTH - 1;
        bonus->c = '*';
        bonus->y = y;
        bonus->value = 10;

        Task_Schedule((task_t)MoveBonus, bonus, move_period, move_period);
        Game_CharXY('*', MAP_WIDTH - 1, y);
    }

    //Update period of MoveBonus
    Task_ChangePeriod((task_t)MoveBonus, move_period, 1);

    //Schedule next bonus to come at a random time
    Task_Schedule(SendBonus, 0, random_int(MIN_BONUS_RATE, MAX_BONUS_RATE), 0);
}

void MoveObstacle(char_object_t * obstacle) {
    //Ensure the object can move
    if(obstacle->x > 1){
        //Clear Location
        Game_CharXY(' ', obstacle->x, obstacle->y);
        //Update Location
        obstacle->x--;
        //Reprint
        Game_CharXY(obstacle->c, obstacle->x, obstacle->y);
        //Check to see if the obstacle hit the player
        CheckCollisonObstacle(obstacle);
    }
    else{
        RemoveObstacle(obstacle);
    }
}

void MoveBonus(bonus_t * bonus) {
    //Ensure the object can move
    if(bonus->x > 1){
        //Clear Location
        Game_CharXY(' ', bonus->x, bonus->y);
        //Update Location
        bonus->x--;
        //Reprint
        Game_CharXY(bonus->c, bonus->x, bonus->y);
        //Check to see if the obstacle hit the player
        CheckCollisonBonus(bonus);
    }
    else{
        RemoveBonus(bonus);
    }
}

void CheckCollisonObstacle(char_object_t * obstacle) {
    if(obstacle->x == game.x && obstacle->y == game.y) GameOver();
    else if(obstacle->x == game.x - 1) {
        ScorePoint(1);
        game.obstaclesPassed++;
    }
}

void CheckCollisonBonus(bonus_t * bonus){
    if(bonus->x == game.x && bonus->y == game.y){
        ScorePoint(bonus->value);
        RemoveBonus(bonus);
        Game_Bell();
    }
}

void RemoveObstacle(char_object_t * obstacle) {
    //Set status to 0
    if(obstacle) obstacle->status = 0;

    //Remove character from screen
    Game_CharXY(' ', obstacle->x, obstacle->y);

    //Remove task used to move obstacle
    Task_Remove((task_t)MoveObstacle, obstacle);
}

void RemoveBonus(bonus_t * bonus) {
    //Set status to 0
    if(bonus) bonus->status = 0;

    //Remove character from screen
    Game_CharXY(' ', bonus->x, bonus->y);

    //Remove task used to move obstacle
    Task_Remove((task_t)MoveBonus, bonus);
}

void ScorePoint(uint8_t points) {
    //Increment Score
    game.score = game.score + points;
    //Set Cursor to Top left corner
    Game_CharXY(186, 0, 1);
    //Display Score
    Game_Printf("Score: %d", game.score);
}

void GameOver(void) {
    //Clean up all scheduled tasks
    RemoveObstacle(0);
    RemoveBonus(0);
    Task_Remove(SendObstacle, 0);
    Task_Remove(SendBonus, 0);

    Game_Bell();

    // Set cursor below bottom of map
    Game_CharXY('\r', 0, MAP_HEIGHT + 1);
    // Show score
    Game_Printf("Game Over! Final score: %d\r\n", game.score);
    // Unregister the receiver used to run the game
    Game_UnregisterPlayer1Receiver(Receiver);
    // Show cursor
    Game_ShowCursor();
}

void DrawRect(char x_min, char y_min, char x_max, char y_max) {
    char x, y;
    Game_CharXY(201, x_min, y_min); //
    for (x = x_min + 1; x < x_max; x++){
        Game_CharXY(205, x, y_min);
        DelayMs(1);
    }
    Game_CharXY(187, x_max, y_min); //
    for (y = y_min + 1; y < y_max; y++) {
        Game_CharXY(186, x_max, y);
        DelayMs(1);
    }

    while (Game_IsTransmitting()) DelayMs(2);
    Game_CharXY(188, x_max, y_max);
    for (x = x_min + 1; x < x_max; x++){
        Game_CharXY(205, x, y_max);
        DelayMs(1);
    }
    Game_CharXY(200, x_min, y_max); //
    for (y = y_min + 1; y < y_max; y++){
        Game_CharXY(186, x_min, y);
        DelayMs(1);
    }
}
