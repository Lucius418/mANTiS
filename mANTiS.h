#ifndef MANTIS_H
#define MANTIS_H

#define ASTARTIME 50 //time reduction for aStar to allow other tasks to finish if aStar takes to long
#include "State.h"
#include "AStar.h"
#include <cmath>
#include <algorithm>

using namespace std;

class mANTiS
{
    public:
        mANTiS();
        virtual ~mANTiS();
        void playGame();    //plays a single game of Ants
    protected:
    private:
        State state;
        AStar* m_pAS;
        Ant* closestFreeAnt(std::vector<Ant*> &ants, const Location &l, bool ignorePath=true);
        std::vector<Ant>::iterator closestAnt(const Location &l);
        void makeMoves();   //makes moves for a single turn
        void endTurn();
        void findFood(int &nAnts, std::vector<Ant*> &availAnts);
        void explore(int &nAnts, std::vector<Ant*> &availAnts);
        void guardHills(int &nAnts, std::vector<Ant*> &availAnts);
        void destroyHill();
        unsigned int destroyers(const Location &l) const;
        int freeAnts(std::vector<Ant*> &ants, bool bAggressive=false);
        int freeAnts();
        int collectAnts(std::vector<Ant*> &ants);
        typedef std::vector<Ant>::iterator AntIt;

        bool createAStarPath(Ant* ant, const Location l, bool append=false);
        void goToLeastVisited(Ant *ant);
        bool collision(std::vector<Ant>::iterator it);
        bool walkable(std::vector<Ant>::iterator it);
        bool targeted(const Location &l) const;
        bool safe(const Location &l) const;
        bool safe(const int row, const int col) const;
        unsigned int guards() const;
        unsigned int guards(const Location &l) const;
};

#endif // MANTIS_H
