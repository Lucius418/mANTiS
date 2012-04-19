#ifndef LOCATION_H_
#define LOCATION_H_
#include <ostream>
#include <string>
using namespace std;
/*
    struct for representing locations in the grid.
*/

class Location
{
    public:
    int row, col;
    Location() : row(-1), col(-1){}
    Location(int r, int c) : row(r), col(c){}

    bool operator==(const Location &l) const;
    bool operator!=(Location &l);
    bool operator<(const Location &l) const;
    bool operator>(const Location &l) const;
    friend ostream& operator<<(ostream &out, const Location &l){
        out << l.row << ":" << l.col;
        return out;
    }
};

inline bool Location::operator==(const Location &l) const{
    return (l.row==row&&l.col==col);
}
inline bool Location::operator!=(Location &l){
    return !(l==*this);
}
/** < and > compares row and column number. The comparision is only used for sorting if stored in a set.
*/
inline bool Location::operator<(const Location &l) const{
    if(row==l.row){
        return (col<l.col);
    }else{
        return (row<l.row);
    }
}

inline bool Location::operator>(const Location &l) const{
    return !(l<*this);
}


#endif //LOCATION_H_
