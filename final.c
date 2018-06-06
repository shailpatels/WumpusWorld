#include <stdio.h>
#include <stdbool.h>
//======================ROBOT
#include "ev3_output.h"
#include "ev3_command.h"
#include "ev3_button.h"
#include "ev3_timer.h"
#include "ev3_lcd.h"
#include "serial_read.h"
//======================
#define MAXSIZE 50
#define width 4
#define height 4
#define speed 30
//===========================tile, stack and queue

typedef struct tile_{
    //representation
    int x,y;
    bool isSafe, explored;    
} tile;
//constructor
void setTile(tile *a, tile b);
void setTile(tile *a, tile b){
    a->x = b.x;
    a->y = b.y;
    a->isSafe = b.isSafe;
    a->explored = b.explored;
}
void tileInit(tile* t){
    t->x = t->y = 0;
    t->isSafe = t->explored = false;
}
//==============================================
typedef struct stack_{
    //representation
    tile stk[MAXSIZE];
    int top,size;
}stack;
void stackInit(stack *s_){s_->top = 0; return;}

stack Stack;
void clearS(void);
void push(tile t);
tile pop(void);

tile pop (){
    tile tmp;
    if (Stack.top == - 1) return tmp;
    else{
        tmp = Stack.stk[Stack.top];
        Stack.top--;
        Stack.size--;
    }
    return(tmp);
}
void push (tile t){
    Stack.top++;
    Stack.size++;
    Stack.stk[Stack.top] = t;
    
    return;
}
void clearS(){while(Stack.size>0)pop(); return;}
//=======================
typedef struct Queue_{
    int top, bot,size;
    tile q[MAXSIZE];
}queue;
void queueInit(queue *q){
    q->top = q->bot = 0;
    q->size = 0;
    return;
}
queue Queue;
void enqueue(tile t);
void clearQ(void);
tile dequeue(void);

void enqueue(tile t){
    if(Queue.size == 0){
        Queue.q[Queue.top] = Queue.q[Queue.bot] = t; 
        Queue.size++;
        return;
    }
    else{
      Queue.bot++;
      Queue.size++;
      Queue.q[Queue.bot] = t;
    }
    return;
}
tile dequeue(){
    tile tmp;
    if(Queue.size == 0) return tmp;
    else{
        tmp = Queue.q[Queue.top];
        Queue.top++;
        Queue.size--;
    }
    return tmp;
}
void clearQ(){while(Queue.size>0)dequeue(); return;}
//===============================================
//prototypes
bool isSafe(int x_, int y_);
void faceDirection(int d);
bool moveTo(int x_, int y_);
void forward();
void forwardSensor();
void forwardTimer();
void turnRight();
void turnLeft();
void sense(int data);
void search(void);
void step(void);
void moveAdjacent(tile t);
bool isAdjacent(int a , int b);
//forward: 0, right: 1, backward: 2, left: 3
int direction = 0;      
int x, y;
tile world[width][height];
void initWorld(){
    //create a 4x4 array of tiles
    int i, j;
    x = y = i = j =0;
    for(i=0; i < width;i++){
        for(j=0;j < height;j++){
            world[i][j].x = i;
            world[i][j].y = j;
            world[i][j].isSafe = world[i][j].explored = false;
        }
    }
    //(0,0) is always safe
    world[0][0].isSafe = world[0][0].explored = true;
    return;
}

int information;
bool foundGold;
int main(){
    OutputInit();
    printf("%s\n", "Starting program!" );
    int x_;
    int y_;
    int numMoves = 25;
    x_ = y_ = 0;
    initWorld();
    foundGold = false;
    information = 0;
    direction = 0;
    world[x][y].isSafe = world[x][y].explored = true;
    printf("%s\n", "Initialized!" );
    while(!foundGold && numMoves > 0){
        step();
        numMoves--;
    }
    search();
    moveTo(0,0);
    //while
    OutputExit();
    return 1;
}
void step(){
    if(!world[x][y].explored){printf("%s\n", "Enter tile information, 8 = gold, 4 = glimmer, 2 = stench, 1 = breeze, 0 = empty\n");
        scanf("%d",&information);
    }
    if(information == 8){
        foundGold = true;
    }
    search();
    if(Stack.size > 0 && !foundGold){
        tile next;
        setTile(&next, pop());
        if(isAdjacent(next.x,next.y)){
            moveAdjacent(next);
        }
        else{ 
            faceDirection(2);
            forward();
            y--;
            if(isAdjacent(next.x,next.y)){
                moveAdjacent(next);
                world[x][y].explored = true;
                world[x][y].isSafe = true;
            }
        }
    }
    printf("final %d", x);
            printf("%d\n", y );
}
bool isSafe(int x_, int y_){
    return world[x_][y_].isSafe;
}
bool moveTo(int x_, int y_) {
    bool found = false;
    // keeps track of the shortest path
    int path[height * width];
    int i = 0;
    while(i < (height*width)){
        path[i] = -1;
        i++;
    }
    path[width * y_ + x_] = -2;
    tile end;
    setTile(&end, world[x_][y_]);

    enqueue(end);
    // Use a queue to perform BFS
    while (Queue.size > 0) {
        tile tmp;
        setTile(&tmp, dequeue() );
        if (tmp.x == x && tmp.y == y) {
            found = true;
            break;
        }
        if (tmp.x > 0 && isSafe(tmp.x - 1, tmp.y && path[width * tmp.y + (tmp.x - 1)] == -1)) {
            enqueue( world[tmp.x - 1][tmp.y] );
            path[width * tmp.y + (tmp.x - 1)] = width * tmp.y + tmp.x;
        }
        if (tmp.x < width - 1 && isSafe(tmp.x + 1, tmp.y) && path[width * tmp.y + (tmp.x + 1)] == -1) {
            enqueue( world[tmp.x + 1][tmp.x] );
            path[width * tmp.y + (tmp.x + 1)] = width * tmp.y + tmp.y;
        }
        if (tmp.y > 0 && isSafe(tmp.x, tmp.y - 1) && path[width * (tmp.y - 1) + tmp.x] == -1) {
            enqueue( world[tmp.x ][tmp.y - 1] );
            path[width * (tmp.y - 1) + tmp.x] = width * tmp.x + tmp.x;
        }
        if (tmp.y < height - 1 && isSafe(tmp.x, tmp.y + 1) && path[width * (tmp.y + 1) + tmp.x] == -1) {
            enqueue( world[tmp.x][tmp.y + 1] );
            path[width * (tmp.y + 1) + tmp.x ] = width * tmp.y + tmp.x;
        }
    }
    if(!found) printf("%s\n", "path not found" );
    if (found) {
        while (x != x_ || y != y_) {
            int current = (width * y) + x;
            int next_x = path[current] % width; 
            int next_y = path[current] / width; 
            if (next_x == x + 1) {
                faceDirection(1);
                printf("%s\n", " right");
            }
            else if (next_x == x - 1){
                faceDirection(3);
                printf("%s\n", " left");
            }
            else if (next_y == y + 1){
                faceDirection(0);
                 printf("%s\n", " up");
            }
            else if (next_y == y - 1){
                faceDirection(2);
                printf("%s\n", "down" );
            }
            forward();
            printf("current %d,", x);
            printf("%d\n", y );
            //update position
            if(direction == 0)y++;
            if(direction == 2)y--;
            if(direction == 1) x++;
            if(direction == 3)  x--;

            printf("final %d,", x);
            printf("%d\n", y );
        }
        clearQ();
        return true;
    }
    return false;
}
void sense(int data){
    world[x][y].explored = true;
    if(data == 0){
        if(x < width - 1) {
            world[(x+1)][y].isSafe = true;
        }
        if (x > 0) {
            world[(x - 1)][y].isSafe = true;
        }
        if (y < height-1) {
            world[x][(y + 1)].isSafe = true;
        }
        if (y > 0) {
            world[x][y - 1].isSafe = true;
        }
    }
    if(data == 1 || data == 3){
        if(x < width - 1) {
            world[(x+1)][y].isSafe = false;
        }
        if (x > 0) {
            world[(x - 1)][y].isSafe = false;
        }
        if (y < height-1) {
            world[x][(y + 1)].isSafe = false;
        }
        if (y > 0) {
            world[x][y - 1].isSafe = false;
        }
    }
    if(data == 2 || data == 3){
        if(x < width - 1) {
            world[(x+1)][y].isSafe = false;
        }
        if (x > 0) {
            world[(x - 1)][y].isSafe = false;
        }
        if (y < height-1) {
            world[x][(y + 1)].isSafe = false;
        }
        if (y > 0) {
            world[x][y - 1].isSafe = false;
        }
    }
    return;
}
void search() {
    tile t;
    setTile(&t, world[x][y]);
    sense(information);
    if (t.x > 0 && world[t.x - 1][t.y].isSafe && !world[t.x - 1][t.y].explored){
        push(world[t.x - 1][ t.y ]);
    }
    if (t.x < width - 1 && world[t.x + 1][t.y].isSafe && !world[t.x + 1][t.y].explored ) {
        push(world[t.x + 1][ t.y]);
    }
    if (t.y > 0 && world[t.x][t.y - 1].isSafe && !world[t.x][ t.y- 1].explored ) {
        push(world[t.x][ t.y - 1]);
    }
    if (t.y < height - 1 && world[t.x][t.y + 1].isSafe && !world[t.x][t.y + 1].explored ) {
        push(world[t.x][t.y + 1]);
    }
    //printf("%s\n", "possible moves" );
    return;
}
void faceDirection(int d){
    while(direction != d){
        turnRight();
    }
    return;
}
void turnRight(){
    if(direction!= 3)direction++;
    else direction = 0;

    //ROBOT
    ResetAllTachoCounts(OUT_AD);
    SetPower(OUT_A, -speed);
    SetPower(OUT_D, speed);
    //then we turn the motors on
    On(OUT_ALL);
    while(MotorRotationCount(OUT_D)<202){}
    Off(OUT_ALL);
    return;
}
void turnLeft(){
    if(direction!=0)direction--;
    else direction = 3;

    //ROBOT
     ResetAllTachoCounts(OUT_AD);
     SetPower(OUT_A, speed);
     SetPower(OUT_D, -speed);
     On(OUT_ALL);     
     while(MotorRotationCount(OUT_D)>-201){}
     Off(OUT_ALL);
     return;
}
void forward(){
    forwardSensor();
    forwardTimer();
    return;
}
//ROBOT
void forwardSensor(){
    int THRESHOLD = ( ReadSerial(1) & 0xFF )+ 5;
    ResetAllTachoCounts(OUT_AD);

    int sensor1, sensor2, temp1, temp2;
    sensor1 = sensor2 = temp1 = temp2 = 0;
    //We set the power level for the motors before moving forward until a line
    SetPower(OUT_A, speed);
    SetPower(OUT_D, speed);

    //Then we turn the motors on
    On(OUT_ALL);

    while(sensor1 < THRESHOLD || sensor2 < THRESHOLD){
        temp1 = ReadSerial(1);
        temp2 = ReadSerial(4);
        sensor1 = temp1 & 0x000000FF;
        sensor2 = temp2 & 0x000000FF;
        if(sensor1 > THRESHOLD) SetPower(OUT_A, 0);
        if(sensor2 > THRESHOLD) SetPower(OUT_D, 0);
    
    }
    Off(OUT_ALL);
    return;
}
//ROBOT
void forwardTimer(){
    SetPower(OUT_A, speed);
    SetPower(OUT_D, speed);
    On(OUT_ALL);
    Wait(2750);
    Off(OUT_ALL);
    return;
}
void moveAdjacent(tile t){
    int x_, y_;
    x_ = t.x;
    y_ = t.y;
    if(x_ > x){
        faceDirection(1);
        forward();
    }
    else if(x_ < x){
        faceDirection(3);
        forward();
    }
    if(y_ > y){
        faceDirection(0);
        forward();
    }
    else if(y_ < y){
        faceDirection(2);
        forward();
    }
    x = x_;
    y = y_;
    return;
}
bool isAdjacent(int x_ , int y_){
    if(x_-1 == x || x_ + 1 == x || x_ == x){
        if(y_-1 == y || y_+1 == x || y_ == y){
            return true;
        }
        else return false;
    }
    else return false;
}