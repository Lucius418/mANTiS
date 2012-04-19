#ifndef ASTAR_H
#define ASTAR_H
#define BUG 1
#include <vector>
#include <set>
#include <cmath>
#include <algorithm>
#include "Ant.h"
#include "Square.h"
#include "Location.h"
#include "Bug.h"
#include "Timer.h"

/** Extending the Location class to make it suitable for the AStar class. **/
class aLocation : public Location {
    public:
        aLocation(Location &l) : Location(l.row, l.col){}
        aLocation(int row, int col) : Location(row, col){}
        int g;
        int h;
        int f() const{return g+h;}
        const aLocation* parent;
        bool operator<(const aLocation &al) const;
        bool operator>(const aLocation &al) const;
        bool operator==(const aLocation &a1) const;
        bool operator==(const Location &l) const;
        bool operator<=(const aLocation &al) const;
        friend bool operator==(const aLocation *a, const Location &l){return (l.row==a->row&&l.col==a->col) ? true : false;}
};
inline bool aLocation::operator==(const aLocation &a1) const{return (a1.row==row&&a1.col==col) ? true : false;}
inline bool aLocation::operator==(const Location &l) const {return (l.row==row&&l.col==col) ? true : false;}
inline bool aLocation::operator<(const aLocation &al) const {return (f() < al.f() ? true : false);}
inline bool aLocation::operator>(const aLocation &al) const {return (f() > al.f() ? true : false);}
inline bool aLocation::operator<=(const aLocation &al) const {return (f() <= al.f() ? true : false);}


class AStar
{
    // Constructors
    public:
        AStar(std::vector<Square> *grid, const int rows,const int cols);
        ~AStar();
    // public functions
        bool findPath(Ant &ant, const Location &goal,const double timeleft);
    // private Functions
    private:
        void addFirstToOpen(Location &l);
        void addToOpen(const aLocation *al);
        unsigned int calcCrd(int row, int col);
        unsigned int calcCrd(const aLocation &l);
        bool closed(const Location &l);
        short cost(int r, int c);
        void freeMem();
        unsigned int manhattanDist(const Location &a, const Location &b);
        bool open(const Location &l);
        void savePath(Ant &a, const aLocation *al);
        bool used(const Location &l);
    // private variables
        Ant m_ant;
        bool m_atGoal;
        Bug bug;
        Location m_goal, m_start;
        std::vector<Square> *mvp_grid;
        int m_rows, m_cols;
        double m_time;
        Timer m_timer;
        int m_minRow, m_maxRow, m_minCol, m_maxCol;
        /// struct used to sort aLocation pointers in set
        /// < gives a FIFO tree and will always find the best path, but will search a large area and take to long
        /// <= gives a LIFO tree and is likely to find a path faster, but isn't guaranteed to find the best. (however it won't be horrible either)
        struct aPtrLess {bool operator() (const aLocation* const &a, const aLocation* const &b) const{return (*a<=*b);}};
        std::multiset<aLocation*, aPtrLess > mv_Open;
        std::vector<aLocation*> mv_Closed;
};

#endif // ASTAR_H
