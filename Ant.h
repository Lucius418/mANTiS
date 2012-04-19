#ifndef ANT_H
#define ANT_H

#include "Location.h"
#include "Square.h"
#include <stack>
#include <vector>

class Ant
{
    public:
    // Constructor
        Ant();
        Ant(int row, int col, int mrow, int mcol, std::vector<Square> *grid);
        virtual ~Ant();
    // public functions
        int row() const {return location.row;}
        int col() const {return location.col;}
        int erow() const {return expected.row;}
        int ecol() const {return expected.col;}
        int trow() const {return target.row;}
        int tcol() const {return target.col;}
        inline unsigned int crd() const {return calcCrd(location.row, location.col);}
        inline unsigned int ecrd() const {return calcCrd(expected.row, expected.col);}
        inline unsigned int tcrd() const {return calcCrd(target.row, target.col);}
        void addCollision(){m_collision++;}
        unsigned int calcCrd(int row, int col) const;
        unsigned int collisionCount(){return m_collision;}
        bool isExpectedLocation(const Ant &a) const;
        bool isExpectedLocation(const Location &l) const;
        unsigned short lastDir;
        int movePath();
        void resetCollision(){m_collision=0;}
        void resetPath();
        void update();
    // public variables
        enum AntTask { EXPLORE, FOOD, FIGHT, DESTROY, IDLE, GUARD };
        Location location;
        Location expected;
        Location target;
        std::stack<int> path;
        int max_row, max_col;
        //std::queue<Location> targetQ;

        AntTask task;
        Location tryMovePath() const;
        unsigned int m_collision;
        friend ostream& operator<<(ostream &out, const Ant &a){
            out << "ANT(L" << a.location << " E" << a.expected << " T" << a.target << ")";
            return out;
        }
        bool operator==(const Ant &a) const;
        bool operator!=(const Ant &a) const;
        bool operator<(const Ant &a) const;
        bool operator>(const Ant &a) const;
    protected:
    private:
        void move(int dir);
        std::vector<Square> *mvp_grid;
};

#endif // ANT_H
