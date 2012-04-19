#ifndef SQUARE_H_
#define SQUARE_H_

#include <vector>

/**
    struct for representing a square in the grid.
    Note: There are separete vectors for object tracking declared in state.h
*/
struct Square
{
    bool ant, explored, water, food, enemy, visible, guarded, enemyHill, hill, deadAnt, deadEnemy;
    // this is used to keep track of number of ants in a square. The game only allows for one.
    // And if everything else works as expected this value will never be higher then 1.
    // But for the unexpected cases this allows me to detect the error and fix it.
    unsigned short path;
    unsigned int visits;
    Square(){
        explored=water=food=enemy=visible=guarded=enemyHill=hill=deadAnt=deadEnemy=ant=0;
        path=visits=0;
    }

};

#endif //SQUARE_H_
