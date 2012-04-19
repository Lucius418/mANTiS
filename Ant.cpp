#include "Ant.h"

Ant::Ant()
{
    //ctor
}
Ant::Ant(int row, int col, int mrow, int mcol, std::vector<Square> *grid)
    : max_row(mrow), max_col(mcol), task(IDLE), m_collision(0), mvp_grid(grid) {
    location.row=row;
    location.col=col;
    expected=location;
    lastDir=0;
}

Ant::~Ant()
{
    //dtor
}

/** DEPRECATED: return true if Ant a is at the expected location
*   @arg Ant to check
*   @return true if Ants expected location the same as the given ant **/
bool Ant::isExpectedLocation(const Ant &a) const{
    if(a.row()==erow()&&a.col()==ecol()) return true;
    return false;
}
/** DEPRECATED: return true if Ants expected location is the given location
*   @arg location to check
*   @return true if Ants expected location is the given location **/
bool Ant::isExpectedLocation(const Location &l) const{
    if(l.row==erow()&&l.col==ecol()) return true;
    return false;
}

/** Update ants expected position
*   @arg direction to move **/
inline void Ant::move(int dir){
    expected=location;
    switch(dir){
        case 0:
            expected.row--;
            if(expected.row<0) expected.row+=max_row;
            break;
        case 1:
            expected.col++;
            if(expected.col>=max_col) expected.col=0;
            break;
        case 2:
            expected.row++;
            if(expected.row>=max_row) expected.row=0;
            break;
        case 3:
            expected.col--;
            if(expected.col<0) expected.col+=max_col;
            break;
        // no default, incorrect values are silently ignored
    }
}

Location Ant::tryMovePath() const{
    int row(location.row);
    int col(location.col);
    switch(path.top()){
        case 0:
            row--;
            if(row<0) row+=max_row;
            break;
        case 1:
            col++;
            if(col>max_col) col=0;
            break;
        case 2:
            row++;
            if(row>max_row) row=0;
            break;
        case 3:
            col--;
            if(col<0) col+=max_col;
            break;
        // no default, incorrect values are silently ignored
    }
    return Location(row,col);
}

/** Moves ant according to previously defined path.
*   If this function is called on an empty path (don't do that)
*   it will try to move the ant north(0) just to behave as expected and avoid a crash
*   @return the direction the ant should be moved (pass as arg to State.makeMove) **/
int Ant::movePath(){
    if(path.empty()) {
        move(0);
        return 0;
    }
    int n=path.top();
    move(n); // update expected location
    path.pop();
    int c(crd());
    if((*mvp_grid)[c].path>0) (*mvp_grid)[c].path--;
    return n;
}

/** Update ants location (called after sucessful move) **/
void Ant::update(){
    location=expected;
}

/** Calculate the vector position for give coordinates
*   \arg row
*   \arg col
*   \return vector position
*/
unsigned int Ant::calcCrd(int r, int c) const{
    if(r<0) {
        r+=max_row;
    }
    else if(r>=max_row){
        r-=max_row;
    }
    if(c<0) {
        c+=max_col;
    }else if(c>=max_col) {
        c-=max_col;
    }
    return r*max_col+c;
}

/** Comparision operators **/
inline bool Ant::operator==(const Ant &a) const{
    return (a.location==location/*&&a.expected==expected*/);
}
inline bool Ant::operator!=(const Ant &a) const{
    return !(a==*this);
}
inline bool Ant::operator<(const Ant &a) const{
    return (a.crd()<crd());
}
inline bool Ant::operator>(const Ant &a) const{
    return !(a<*this);
}

/** clear path and remove path info from grid **/
void Ant::resetPath() {
    while(!path.empty()){
        movePath();
    }
    target.row=target.col=-1;
    expected=location;
}
