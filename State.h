#ifndef STATE_H_
#define STATE_H_
#define CRD row*state.cols+col
#define TIMEFACTOR 0.8

#include <iostream>
#include <stdio.h>
#include <cstdlib>
#include <cmath>
#include <string>
#include <vector>
#include <algorithm>
#include <queue>
#include <stack>
#include "Bug.h"
#include "Timer.h"
#include "Square.h"
#include "Location.h"
#include "Ant.h"
#include <set>
#include <map>

/*
    constants
*/
const int TDIRECTIONS = 4;
const char CDIRECTIONS[4] = {'N', 'E', 'S', 'W'};
const int DIRECTIONS[4][2] = { {-1, 0}, {0, 1}, {1, 0}, {0, -1} };      //{N, E, S, W}

class State
{
    public:
        State();
        ~State();

    // Variables
        int rows, cols, turn, turns, nPlayers;
        double attackradius, spawnradius, viewradius;
        double attackradius2, spawnradius2, viewradius2;
        double loadtime, turntime, maxtime;
        bool gameover;
        std::vector<double> scores;
        std::vector<Square> grid;
        std::set<Location> myHills, enemyAnts, food, enemyHills;
        std::map<Location, unsigned short> myGuards;
        std::vector<Ant> myAnts;

    // Classes
        Timer timer;
        Bug bug;

    // Functions
        unsigned int calcCrd(int row, int col);
        unsigned int calcCrd(Location &l){return calcCrd(l.row, l.col);}
        void clearAnts();
        double distance(const Location &loc1, const Location &loc2);
        double distance(const Ant &a, const Location &loc);
        Location getLocation(const Location &startLoc, int direction);
        Location leastGuardedHill();
        void makeMove(const Location &loc, int direction);
        unsigned int manhattan(const Location &loc1, const Location &loc2);
        unsigned int manhattan(Ant &a, const Location &loc);
        void preProcess();
        void reset();
        void setup();
        void updateVisits(int row, int col);
        void updateVision(int row, int col);
        void updateVisionInformation();
        bool isNewAnt(const Location l);
};

// Operators
std::ostream& operator<<(std::ostream &os, const State &state);
std::istream& operator>>(std::istream &is, State &state);

#endif //STATE_H_
