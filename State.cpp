#include "State.h"

using namespace std;

//constructor
State::State()
{
    gameover = 0;
    turn = 0;
    bug.open("./debug.txt");
};

//deconstructor
State::~State()
{
    bug.close();
};

//sets the state up
void State::setup()
{
    grid = vector<Square>(rows*cols);
    food.clear();
    myAnts.clear();
};


//outputs move information to the engine
void State::makeMove(const Location &loc, int direction)
{
    bug << "makeMove for " << loc << " in direction " << direction << endl;
    cout << "o " << loc.row << " " << loc.col << " " << CDIRECTIONS[direction] << endl;
    bug << "Out: o " << loc.row << " " << loc.col << " " << CDIRECTIONS[direction] << endl;
}

/// returns the euclidean distance between two locations with the edges wrapped
double State::distance(const Location &loc1, const Location &loc2)
{
    int d1 = abs(loc1.row-loc2.row),
        d2 = abs(loc1.col-loc2.col),
        dr = min(d1, rows-d1),
        dc = min(d2, cols-d2);
    return sqrt(dr*dr + dc*dc);
};
double State::distance(const Ant &a, const Location &l){
    return distance(Location(a.row(), a.col()) , l);
}

/// returns manhattan distance between two locations
unsigned int State::manhattan(const Location &l1, const Location &l2){
        int r=abs(l1.row-l2.row); // distance between points
        int c=abs(l1.col-l2.col);
        int dr=min(r, rows-r); // distance with wrapping
        int dc=min(c, cols-c);
        return (dr+dc); // return distance in squares
}
unsigned int State::manhattan(Ant &a, const Location &l){
    return manhattan(Location(a.row(),a.col()), l);
}

//returns the new location from moving in a given direction with the edges wrapped
Location State::getLocation(const Location &loc, int direction)
{
    return Location( (loc.row + DIRECTIONS[direction][0] + rows) % rows,
                     (loc.col + DIRECTIONS[direction][1] + cols) % cols );
};

/*
    This function will update update the lastSeen value for any squares currently
    visible by one of your live ants.

    BE VERY CAREFUL IF YOU ARE GOING TO TRY AND MAKE THIS FUNCTION MORE EFFICIENT,
    THE OBVIOUS WAY OF TRYING TO IMPROVE IT BREAKS USING THE EUCLIDEAN METRIC, FOR
    A CORRECT MORE EFFICIENT IMPLEMENTATION, TAKE A LOOK AT THE GET_VISION FUNCTION
    IN ANTS.PY ON THE CONTESTS GITHUB PAGE.
*/
void State::updateVisionInformation()
{   bug << "State::UpdateVisionInformation" << endl;
    //bug << "Ants: " << myAnts.size() << endl;
    std::vector<Ant>::iterator it=myAnts.begin();
    int n=0;
    do{
        bug << "Ant " << n++ << " at " << *it << endl;
        for(int r(it->row()-viewradius);r<=it->row()+viewradius;r++){
            for(int c=it->col()-viewradius;c<=it->col()+viewradius;c++){
                int rr,cc;
                if(r<0){
                    rr=r+rows;
                }else if(r>=rows){
                    rr=r-rows;
                }else{rr=r;}
                if(c<0){
                    cc=c+cols;
                }else if(c>=cols){
                    cc=c-cols;
                }else{cc=c;}
                //bug << "Creating Location(" << rr << "," << cc << ") based on " << r << ":" << c << endl;
                const Location l(rr,cc);
                //Location l(r<0?r+rows:r,c<0?c+cols:c);
                if(distance(*it, l)<=viewradius){
                    //bug << "setting vision status for " << l.row << ":" << l.col << " (" << l.row*cols+l.col << ")." << endl;
                    grid[l.row*cols+l.col].explored=1;
                    grid[l.row*cols+l.col].visible=1;
                    //bug << "explored " << l.row << ":" << l.col << endl;
                }
            }
        }
        it++;
    }while(it!=myAnts.end());
};

/*
    This is the output function for a state. It will add a char map
    representation of the state to the output stream passed to it.

    For example, you might call "cout << state << endl;"
*/
ostream& operator<<(ostream &os, const State &state)
{
        int nRow(0),nCol(0);
        os << "---";
        for(int col=0; col<state.cols; col++){
            os << nCol++;
            if(nCol==10) nCol=0;
        }
        os << endl;
        for(int row=0; row<state.rows; row++)
        {
            if(nRow<100) os << "0";
            if(nRow<10) os << "0";
            os << nRow++;
            for(int col=0; col<state.cols; col++)
            {
                if(state.grid[CRD].ant){
                    //os << state.grid[CRD].ant;
                    os << 'a';
                /*}else if(state.grid[CRD].closed){
                    os << 'X';
                }else if(state.grid[CRD].open){
                    os << 'O';*/
                }else if(state.grid[CRD].enemy){
                    os << 'A';
                }else if(state.grid[CRD].water){
                    os << '#';
                }else if(state.grid[CRD].food){
                    os << 'f';
                }else if(state.grid[CRD].enemyHill){
                    os << 'H';
                }else if(state.grid[CRD].hill){
                    os << 'h';
                }else if(state.grid[CRD].path){
                    os << ',';
                    //os << state.grid[CRD].path;
                }else if(state.grid[CRD].visible){
                    os << '.';
                    //os << state.grid[CRD].visits;
                }else if(state.grid[CRD].explored){
                    os << ':';
                }else{
                    os << '?';
                }
            }
        os << endl;
    }
    return os;
};

//input function
istream& operator>>(istream &is, State &state)
{
    int row, col, player;
    string inputType, junk;

    //finds out which turn it is
    while(is >> inputType)
    {
        if(inputType == "end")
        {
            state.gameover = 1;
            break;
        }
        else if(inputType == "turn")
        {
            is >> state.turn;
            break;
        }
        else //unknown line
            getline(is, junk);
    }

    if(state.turn == 0)
    {
        //reads game parameters
        while(is >> inputType)
        {
            if(inputType == "loadtime")
                is >> state.loadtime;
            else if(inputType == "turntime"){
                is >> state.turntime;
                state.maxtime=state.turntime*TIMEFACTOR; // Writing debug output takes time. Use 0.8 for debug, 0.9 for aichallenge.org
            }else if(inputType == "rows")
                is >> state.rows;
            else if(inputType == "cols")
                is >> state.cols;
            else if(inputType == "turns")
                is >> state.turns;
            else if(inputType == "viewradius2")
            {
                is >> state.viewradius2;
                state.viewradius = sqrt(state.viewradius2);
            }
            else if(inputType == "attackradius2")
            {
                is >> state.attackradius2;
                state.attackradius = sqrt(state.attackradius2);
            }
            else if(inputType == "spawnradius2")
            {
                is >> state.spawnradius2;
                state.spawnradius = sqrt(state.spawnradius2);
            }
            else if(inputType == "ready") //end of parameter input
            {
                state.timer.start();
                break;
            }
            else    //unknown line
                getline(is, junk);
        }
    }
    else
    {
        //reads information about the current turn
        while(is >> inputType)
        {
            if(inputType == "w") //water square
            {
                is >> row >> col;
                state.grid[CRD].water = 1;
                state.grid[CRD].visible=1;
            }
            else if(inputType == "f") //food square
            {
                is >> row >> col;
                state.grid[CRD].visible=1;
                state.grid[CRD].explored=1; // explored means that the spot has been seen and is not water, so it can be used for pathfinding.
                state.grid[CRD].food=1;
                //state.food.push_back(Location(row, col));
                state.food.insert(Location(row, col));
                /*if(state.food.find(CRD)==state.food.end()){
                    state.food[CRD]=NULL; // add food to list
                }*/
            }
            else if(inputType == "a") //live ant square
            {
                is >> row >> col >> player;
                //state.grid[CRD].visits++;
                /*state.updateVisits(row,col);
                state.grid[CRD].explored=1;
                state.grid[CRD].visible=1;*/
                state.updateVision(row, col);
                if(player == 0){
                    state.bug << "own ant " << row << ":" << col << endl;
                    state.grid[CRD].ant++;
                    if(state.isNewAnt(Location(row,col))){
                        state.bug << "New ant at " << row << ":" << col << " using max " << state.rows << ":" << state.cols <<endl;
                        state.myAnts.push_back(Ant(row, col, state.rows, state.cols, &state.grid));
                        //state.bug << "Ant added (" << state.myAnts.size() << ")" << endl;
                    }
                    state.updateVisits(row,col);
                }
                else{
                    state.enemyAnts.insert(Location(row, col));
                    state.bug << "enemy ant " << row << ":" << col << endl;
                    state.grid[CRD].enemy=1;
                }
            }
            else if(inputType == "d") //dead ant square
            {
                is >> row >> col >> player;
                state.grid[CRD].explored=1;
                state.grid[CRD].visible=1;
                if(player == 0){
                    state.grid[CRD].deadAnt=1;
                }else{
                    state.grid[CRD].deadEnemy=1;
                }
                //state.grid[CRD].deadAnts.push_back(player);
            }
            else if(inputType == "h")
            {
                is >> row >> col >> player;
                state.grid[CRD].explored=1;
                state.grid[CRD].visible=1;
                //state.grid[CRD].hillPlayer = player;
                if(player == 0){
                    state.bug << "Adding hill " << row << ":" << col << endl;
                    state.myHills.insert(Location(row, col));
                    state.bug << "There are now " << state.myHills.size() << " hills in the set." << endl;
                    state.grid[CRD].hill = 1;
                }else{
                    state.enemyHills.insert(Location(row, col));
                    state.grid[CRD].enemyHill = 1;
                }

            }
            else if(inputType == "players") //player information
                is >> state.nPlayers;
            else if(inputType == "scores") //score information
            {
                state.scores = vector<double>(state.nPlayers, 0.0);
                for(int p=0; p<state.nPlayers; p++)
                    is >> state.scores[p];
            }
            else if(inputType == "go") //end of turn input
            {
                if(state.gameover)
                    is.setstate(std::ios::failbit);
                else
                    state.timer.start();
                break;
            }
            else //unknown line
                getline(is, junk);
        }
    }
    double tracker(state.timer.getTime());
    state.clearAnts();
    state.bug << "clearAnts took " << state.timer.getTime()-tracker << endl;
    state.preProcess();
    return is;
};

void State::preProcess(){
    bug << "preProcess" << endl;
    //bug << "Found " << myHills.size() << " hills." << endl;
    myGuards.clear();
    for(std::set<Location>::const_iterator itHill(myHills.begin());itHill!=myHills.end();itHill++){
        myGuards[*itHill]=0;
        //bug << "processing " << *itHill << endl;
        for(std::vector<Ant>::const_iterator itAnt(myAnts.begin());itAnt!=myAnts.end();itAnt++){
            if(itAnt->target==*itHill){
                bug << "adding " << *itHill << endl;
                myGuards[*itHill]++;
            }
        }
        bug << "Hill " << *itHill << " has " << myGuards[*itHill] << "guards." << endl;
    }
}

/** Find the hill with fewest guards.
*   \return Location location of the hill with fewest guards.
**/
Location State::leastGuardedHill(){
    Location l;
    if(myHills.empty()||myGuards.empty()) {bug << "No Hills" << endl; return Location(-1,-1);}
    //std::set<Location>::const_iterator itReturn(0);
    unsigned short guards(-1);
    for(std::set<Location>::const_iterator itHill(myHills.begin());itHill!=myHills.end();itHill++){
        if(myGuards[*itHill]<guards){
            guards=myGuards[*itHill];
            l=*itHill;
        }
    }
    return l;
}

/** return false if Location belongs to an existing ant.
*   If the ant exists its location is updated.
*   \arg location
*   \return true if the ant isn't already known.
**/
bool State::isNewAnt(const Location l){
    std::vector<Ant>::iterator it;
    for(it=myAnts.begin();it!=myAnts.end();it++){
        //if(it->isExpectedLocation(l)){
        if(it->expected==l){
            it->location=l;
            return false;
        }
    }
    return true;
}

/** clear erroneous and duplicate ants, updates location for correct ants
*   this needs to be done after getting input from server to ensure that ant tracking is correct.
*   Othervise unexpected ant behaviour (such as blocked moves) will make us lose track of ants.
*   This in turn will cause the bot to creat erroneous output and fail. **/
void State::clearAnts(){
    bug << "cleaning up ant vector" << endl;
    bug << "myAnts not empty" << endl;
    std::vector<Ant>::iterator it(myAnts.begin());
    //bug << "iterator initiated" << endl;
    while(it!=myAnts.end()){
        bug << "Ant at " << *it << "(" << &(*it) <<")" << endl;
        //bug << "Antcount(grid): " << grid[it->ecrd()].ant << endl;
        if(grid[it->ecrd()].ant==0){ // lost track of ant
            bug << "No ant here (" << *it << ")" << endl;
            if(grid[it->crd()].ant!=0){ // and did not move. reset it.
                bug << "Ant didn't move. Found at old location." << endl;
                it++;
             }else if(grid[it->ecrd()].deadAnt!=0||grid[it->crd()].deadAnt!=0){ // information about dead ants is available even id the dqwuare is invisible. = always known
                 bug << "Ant died.." << endl;
                 it->resetPath();
                 it=myAnts.erase(it);
             }else { // if this happens something is terribly wrong. Try to minimize damage.
                 bug << "Ant abducted by aliens!" << endl;
                 it->resetPath();
                 it=myAnts.erase(it);
             }
        /*}else if(grid[it->ecrd()].ant>1) { // This shouldn't happen
            grid[it->ecrd()].ant--;
            it=myAnts.erase(it);
            bug << "Duplicate ant (" << *it << ")" << endl;*/
        }else if(it->collisionCount()>3){ // try to reset ant if it seems stuck
            it->resetPath();
            it->expected=it->location;
            it->target=Location(-1,-1);
            it->task=Ant::IDLE;
            it->resetCollision();
            it++;
        }else if(it->task==it->FOOD&&grid[it->tcrd()].food==0){
            bug << "Food picked up" << endl;
            grid[it->ecrd()].ant=0;
            grid[it->crd()].ant=1;
            it->expected=it->location;
            it->target=Location(-1,-1);
            it->task=Ant::IDLE;
            it++;
        }else if(it->task==it->DESTROY && (it->path.empty()||grid[it->tcrd()].enemyHill==0) ){
            bug << "Target destroyed" << endl;
            enemyHills.erase(it->target);
            grid[it->ecrd()].ant=0;
            grid[it->crd()].ant=1;
            it->expected=it->location;
            it->target=Location(-1,-1);
            it->task=Ant::IDLE;
            it++;
        /*}else if(it->task==it->GUARD && it->target==it->location){
            grid[calcCrd(it->location)].guarded=true;
            it++;*/
        }else if(it->task==it->EXPLORE&&it->path.empty()){
            it->task=it->IDLE;
            it->target=Location(-1,-1);
        }else{
            it->update();
            it++;
            //bug << "Moving on.." << endl;
        }
    }
}

/** Reset Squares and clear lists.
*   This needs to be done before UpdateVisionInformation is run.
*   Can be run at end of turn.
**/
void State::reset(){
    std::vector<Square>::iterator it;
    for(it=grid.begin();it!=grid.end();it++){
        it->ant=0;
        if(it->visible){  // Only reset objects in visible spots. We want to remember where things are even if we can't see them.
            it->enemy=0;
            it->food=0;
            it->enemyHill=0;
            it->hill=0;
        }
        it->visible=0;
        it->deadAnt=0;
        it->deadEnemy=0;
        it->guarded=0;
    }
    food.clear();
    enemyAnts.clear();
    //enemyHills.clear();
    //myHills.clear();
}

/** Updates the visits counter for a area.
*   \arg row
*   \arg col
**/
inline void State::updateVisits(int row, int col){
    bug << "updateVisits for " << row << ", " << col << endl;
    grid[calcCrd(row,col)].visits++;
}

/** Update what is seen
*   Set squares within viewradius of each ant to visible and explored
*   \arg row
*   \arg col
**/
void State::updateVision(int row, int col){
    Location antLocation(row,col);
    int rr,cc;
    for(int r=row-viewradius;r<=row+viewradius;r++){
        for(int c=col-viewradius;c<=col+viewradius;c++){
            if(r<0){
                rr=r+rows;
            }else if(r>=rows){
                rr=r-rows;
            }else{rr=r;}
            if(c<0){
                cc=c+cols;
            }else if(c>=cols){
                cc=c-cols;
            }else{cc=c;}
            //bug << "Creating Location(" << rr << "," << cc << ") based on " << r << ":" << c << endl;
            Location l(rr,cc);
            //Location l(r<0?r+rows:r,c<0?c+cols:c);
            if(distance(antLocation, l)<=viewradius){
                //bug << "setting vision status for " << l.row << ":" << l.col << " (" << l.row*cols+l.col << ")." << endl;
                grid[l.row*cols+l.col].explored=1;
                grid[l.row*cols+l.col].visible=1;
                //bug << "explored " << l.row << ":" << l.col << endl;
            }
        }
    }
}

/** Calculate the vector position for give coordinates
*   \arg row
*   \arg col
*   \return int vector position
*/
unsigned int State::calcCrd(int r, int c){
    if(r<0) {
        r+=rows;
    }
    else if(r>=rows){
        r-=rows;
    }
    if(c<0) {
        c+=cols;
    }else if(c>=cols) {
        c-=cols;
    }
    return r*cols+c;
}
