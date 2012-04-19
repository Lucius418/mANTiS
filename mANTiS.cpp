/** mANTiS - A aichallenge.org ants bot
    Copyright (C) 2011  Anders Honkamaa (www.expressit.se - anders@expressit.se)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "mANTiS.h"

mANTiS::mANTiS()
{

}

mANTiS::~mANTiS()
{
    delete m_pAS;
}

/** Main game loop **/
void mANTiS::playGame(){
    //reads the game parameters and sets up
    cin >> state;
    state.setup();
    m_pAS = new AStar(&state.grid, state.rows, state.cols);
    state.bug << "Turntime: " << state.turntime << endl;
    state.bug << "ViewRadius: " << state.viewradius << endl;
    state.bug << "SpawnRadius: " << state.spawnradius << endl;
    state.bug << "AttackRadius: " << state.attackradius << endl;
    state.bug << "Rows: " << state.rows << endl;
    state.bug << "Cols: " << state.cols << endl;
    //state.bug << state << endl;
    endTurn();
    state.bug << "Init completed" << endl;
    //continues making moves while the game is not over
    //bool firstRun(true);
    while(cin >> state)
    {
        double tracker(state.timer.getTime());
        //Get input and do maintenance
        //state.updateVisionInformation();
        // Assign ants to tasks in order of importance
        //std::vector<Ant*> aggrAnts;
        //int nAggr(freeAnts(aggrAnts, true));
        destroyHill(); // destroyHills will interrupt exploring and food gathering ants
        state.bug << "destroyHills took " << state.timer.getTime()-tracker << endl;
        tracker=state.timer.getTime();
        // Thats why we don't call findFreeAnts until after it has run.
        std::vector<Ant*> availAnts;
        int nAnts(freeAnts(availAnts));
        state.bug << "freeAnts took " << state.timer.getTime()-tracker << endl;
        if(nAnts>0&&state.timer.getTime()<state.maxtime-50)guardHills(nAnts, availAnts); // Must be run before any aStar pathfinding is done.
        state.bug << "guardHill took " << state.timer.getTime()-tracker << endl;
        tracker=state.timer.getTime();
        if(nAnts>0&&state.timer.getTime()<state.maxtime-50)findFood(nAnts, availAnts); //AStar takes time, so skip these if we're low on time.
        state.bug << "findFood took " << state.timer.getTime()-tracker << endl;
        tracker=state.timer.getTime();
        // At this point there should be some time left to let remaining ants explore the area.
        if(nAnts>0&&state.timer.getTime()<state.maxtime)explore(nAnts, availAnts);
        state.bug << "explore took " << state.timer.getTime()-tracker << endl;
        tracker=state.timer.getTime();
        makeMoves();
        state.bug << "makeMoves took " << state.timer.getTime()-tracker << endl;
        endTurn();
    }
}

/** Send out ants to collect food.
*   Find food items which aren't already targeted by any ant. Then find the closest ant in the list and send it to get the food.
*   \arg number of ants
*   \arg vector with available ants (created by findFreeAnts)
**/
void mANTiS::findFood(int &nAnts, std::vector<Ant*> &availAnts){
    double tracker(0);
    state.bug << "findFood" << endl;
    if(state.food.size()==0) {state.bug << "No food " << endl; return;}
    if(nAnts<=0) { state.bug << "no free ants." << endl; return;} // abort if there are no free ants.
    state.bug << "Free ants: " << nAnts << endl;
    Ant *ant=0;
    for(std::set<Location>::iterator itFood(state.food.begin());!(itFood==state.food.end());itFood++){
        if(!targeted(*itFood)){
            tracker=state.timer.getTime();
            ant=closestFreeAnt(availAnts, *itFood);
            state.bug << "  closest Free Ant took " << state.timer.getTime()-tracker << endl;
            if(ant==0) break;
            tracker=state.timer.getTime();
            if(!createAStarPath(ant, *itFood)){
               ant->task=Ant::IDLE;
               ant->target=Location(-1,-1);
            }else{
                ant->task=Ant::FOOD;
            }
            state.bug << " createAStarPath took " << state.timer.getTime()-tracker << endl;
            if(--nAnts==0) break;  //closestFreeAnt removes the returned ant

        }
        if(state.timer.getTime()>state.maxtime-ASTARTIME) break; // Abort if we are running out of time.
    }
}

/** check if any ant is targeting the location
*   \arg location
*   \return true if location is targeted
*/
bool mANTiS::targeted(const Location &l) const{
    for(std::vector<Ant>::const_iterator it(state.myAnts.begin());it!=state.myAnts.end();it++){
        if(l==it->target) return true;
    }
    return false;
}

/** return false if another ant is at the location or going there the next turn
*   This is just a wrapper for safe(Location)
*   \arg row
*   \arg col
*   \return true if it's safe to go to the location
*/
inline bool mANTiS::safe(const int r, const int c) const {
    return safe(Location(r,c));
}

/** return false if another ant is at the location or going there the next turn
*   \arg location
*   \return true if it's safe to go to the location
*/
bool mANTiS::safe(const Location &l) const{
    for(std::vector<Ant>::const_iterator it(state.myAnts.begin());it!=state.myAnts.end();it++){
        if(it->location==l||it->expected==l) return false;
    }
    return true;
}

/** Send out ants to explore the map.
*   Move ants in list to the least walked neighbour square.
*   \arg number of ants
*   \arg vector of ants which should explore
*/
void mANTiS::explore(int &nAnts, std::vector<Ant*> &availAnts){
    state.bug << "explore" << endl;
    std::vector<Ant*>::iterator ait;
    if(nAnts<=0) { state.bug << "no free ants." << endl; return;}
    state.bug << "Free ants: " << nAnts << endl;
    ait=availAnts.begin();
    while(ait!=availAnts.end()){
        goToLeastVisited(*ait);
        (*ait)->task=Ant::EXPLORE;
        nAnts--;
        ait=availAnts.erase(ait);
        //ait++;
        if(state.timer.getTime()>state.maxtime) break;
    }
}

/** Let some ants guard own hills.
*   This function will flag squares as guardes once a ant reaches its position.
*   The guard flag is used by aStar pathfinding. Running aStar before running guardHills will make aStar unaware of any squares blocked by guards.
*   \arg number of ants
*   \arg NOT USED. vector with ants NOT USED
*/
void mANTiS::guardHills(int &nAnts, std::vector<Ant*> &availAnts){
    state.bug << "guardHill" << endl;
    const unsigned short MAX_GUARDS=3; // max guards for each hill.
    const unsigned short MAX_SHARE=3; // ants/n
    const unsigned short THRESHOLD=(int)state.myHills.size(); // min number of ants that must exist before any guards can be set.
    if(nAnts<THRESHOLD) { state.bug << "no free ants." << endl; return;}
    Ant *ant=0;
    state.bug << "Free ants: " << nAnts << endl;
    unsigned int totalGuards(guards());
    unsigned short maxGuards(min((int)state.myHills.size()*MAX_GUARDS, (nAnts-THRESHOLD)/MAX_SHARE));
    // Assign new guards if needed
    state.bug << "maxGuards=" << maxGuards << "(hills=" << state.myHills.size() << ", ants=" << state.myAnts.size() << "), totalGuards=" << totalGuards << endl;
    if((int)((int)state.myAnts.size()/(totalGuards>0 ? totalGuards : 1))>MAX_SHARE){
        state.bug << "Assigning new guards " << endl;
        while(totalGuards<maxGuards){
            Location l(state.leastGuardedHill());
            ant=closestFreeAnt(availAnts, l);
            state.bug << "Assigning " << *ant << " to " << l << endl;
            ant->resetPath();
            ant->task=Ant::GUARD;
            ant->target=l;
            totalGuards++;
            nAnts--;
            if(nAnts==0) break;
            if(state.timer.getTime()>state.maxtime) break;
        }
    }else{
        state.bug << "guard quote full." << endl;
    } // end if
    // Handle guards
    state.bug << "Handling guards" << endl;
    if(totalGuards==0) return;
    for(std::vector<Ant>::iterator itAnt(state.myAnts.begin());itAnt!=state.myAnts.end();itAnt++){
        state.bug << "Handling " << *itAnt << " with path " << itAnt->path.size() << endl;
        if(itAnt->task==Ant::GUARD&&itAnt->path.empty()){
            state.bug << "GUARD ant" << endl;
            int dist(state.manhattan(itAnt->location, itAnt->target));
            if(dist>3){
                state.bug << "using aStar" << endl;
                createAStarPath(&(*itAnt), itAnt->target);
            }else if(dist<2){
                state.bug << "using simple path" << endl;
                int cc, rr, currentdir, direction(-1);
                unsigned short best(-1), cost;
                cc=itAnt->col();
                for(rr=itAnt->row()-1;rr<=itAnt->row()+1;rr+=2){
                    rr<itAnt->row() ? currentdir=0 : currentdir=2;
                    int crd(state.calcCrd(rr,cc));
                    if(!state.grid[crd].water&&!state.grid[crd].guarded&&safe(rr,cc)){
                        state.bug << "walkable square" << endl;
                        cost=5-state.manhattan(itAnt->target, Location(rr,cc));
                        if(cost<best){
                            state.bug << "new best square found" << endl;
                            best=cost;
                            direction=currentdir;
                        }
                    }else{
                        state.bug << "non walkable square" << endl;
                    }
                }
                rr=itAnt->row();
                for(cc=itAnt->col()-1;cc<=itAnt->col()+1;cc+=2){
                    cc<itAnt->col() ? currentdir=3 : currentdir=1;
                    int crd(state.calcCrd(rr,cc));
                    if(!state.grid[crd].water&&!state.grid[crd].guarded&&safe(rr,cc)){
                         state.bug << "walkable square" << endl;
                        cost=5-state.manhattan(itAnt->target, Location(rr,cc));
                        if(cost<best){
                            state.bug << "new best square found" << endl;
                            best=cost;
                            direction=currentdir;
                        }
                    }else{
                        state.bug << "non walkable square" << endl;
                    }
                }
                if(direction>=0){
                    state.bug << "Pushing path" << endl;
                    itAnt->path.push(direction);
                }else{
                    state.bug << "Nowhere to move" << endl;
                }
            }else{
                state.grid[state.calcCrd(itAnt->location)].guarded=true;
                state.bug << "not moving ant" << endl;
            }
        } // end if
        else if(itAnt->task==Ant::GUARD){
            state.bug << "GUARD moving to " << itAnt->target << endl;
        }else{
            state.bug << "non GUARD,skipping" << endl;
        }
    }
}

/** Find number of guards for a location
*   \arg location (normally location of a hill)
*   \return number of guard ants targeting the location
*/
unsigned int mANTiS::guards(const Location &l) const {
    int n(0);
    for(std::vector<Ant>::const_iterator it(state.myAnts.begin());it!=state.myAnts.end();it++){
        if(it->task==Ant::GUARD&&it->target==l) n++;
    }
    return n;
}

/** find total number of guard ants
*   \return toatl number of ants guarding something
*/
unsigned int mANTiS::guards() const {
    int n(0);
    for(std::vector<Ant>::const_iterator it(state.myAnts.begin());it!=state.myAnts.end();it++){
        if(it->task==Ant::GUARD) n++;
    }
    return n;
}

/** send ants to destroy enemy hill.
*/
void mANTiS::destroyHill(){
    double tracker(0);
    state.bug << "destroyHill" << endl;
    if(state.enemyHills.empty()) {state.bug << "No enemy hills " << endl; return;}else{state.bug << "Enemy hills" << endl;}
    if(state.myAnts.empty()) {state.bug << "No Ants" << endl; return;}else{state.bug << "Ants" << endl;}
    state.bug << "Initializing ants" << endl;
    //int nAnts(state.myAnts.size());
    int nAnts(min(state.myAnts.size()-5,state.myAnts.size()/5));
    if(nAnts<10) {state.bug << "Not enough ants" << endl; return;}
    state.bug << "Initializing iterators" << endl;
    std::set<Location>::iterator it(state.enemyHills.begin());
    std::vector<Ant>::iterator ant;
    state.bug << "Entering loop" << endl;
    while(it!=state.enemyHills.end()&&nAnts>=10){
        state.bug << "while loop" << endl;
        for(int n=destroyers(*it);n<5;n++){
            tracker=state.timer.getTime();
            ant=closestAnt(*it);
            state.bug << "  closest Free Ant took " << state.timer.getTime()-tracker << endl;
            if(ant==state.myAnts.end()) break;
            tracker=state.timer.getTime();
            createAStarPath(&(*ant), *it);
            state.bug << "  createAStarPath took " << state.timer.getTime()-tracker << endl;
            ant->task=Ant::DESTROY;
            nAnts--;
            if(state.timer.getTime()>state.maxtime) break; // Abort if we are running out of time.
        }
        it++;
        if(state.timer.getTime()>state.maxtime-ASTARTIME) break;
    }
}

/** number of ants who are currently targeted to destroy an enemy hill
*   \arg location
*   \return number to destroyer ants targeting the location
**/
unsigned int mANTiS::destroyers(const Location &l) const{
    int n(0);
    for(std::vector<Ant>::const_iterator it(state.myAnts.begin());it!=state.myAnts.end();it++){
        if(it->task==Ant::DESTROY&&it->target==l) n++;
    }
    return n;
}

/** creates a path for the ant
*   \arg ant
*   \arg target location
*   \arg (optional, defaults to false) true if the path should be appended to the current path.
*       False will erase any existing path even if pathfinding fails, so you can be left with an ant without path.
*   \return true if a path is found
**/
bool mANTiS::createAStarPath(Ant *ant, const Location l, bool append){
    state.bug << "Calling AStar with ant at " << ant->location << " targeting " << l << endl;
    if(!append) ant->resetPath();
    return m_pAS->findPath(*ant, l, state.maxtime-state.timer.getTime()-ASTARTIME);
}

/** adds a single step to ants path to keep it moving in the current direction if possible.
*   If that's not possible it will move to the least visited neighbour square.
*   Ants are assumed to have a empty path.
*   \arg Ant
**/
void mANTiS::goToLeastVisited(Ant *ant){
    state.bug << "goToLeastVisited" << endl;
    state.bug << "using: " << *ant << endl;
    int direction(-1), currentdir;
    unsigned int visits(-1); // setting an unsigned to -1 sets it to its max value
    bool useLast(false);
    state.bug << "variables set (visits=" << visits << ")" << endl;
    switch (ant->lastDir){
        case 0:{
            int crd(state.calcCrd(ant->row()-1, ant->col()));
            if(!state.grid[crd].water&&!state.grid[crd].guarded&&safe(ant->row()-1, ant->col())) useLast=true;
            break;
        }case 1:{
            int crd(state.calcCrd(ant->row(), ant->col()+1));
            if(!state.grid[crd].water&&!state.grid[crd].guarded&&safe(ant->row(), ant->col()+1)) useLast=true;
            break;
        }case 2:{
            int crd(state.calcCrd(ant->row()+1, ant->col()));
            if(!state.grid[crd].water&&!state.grid[crd].guarded&&safe(ant->row()+1, ant->col())) useLast=true;
            break;
        }case 3:{
            int crd(state.calcCrd(ant->row(), ant->col()-1));
            if(!state.grid[crd].water&&!state.grid[crd].guarded&&safe(ant->row(), ant->col()-1)) useLast=true;
            break;
        }
    }
    if(useLast){
        ant->path.push(ant->lastDir);
        return;
    }
    int rr, cc(ant->col());
    for(rr=ant->row()-1;rr<=ant->row()+1;rr+=2){
        rr<ant->row() ? currentdir=0 : currentdir=2;
        int crd(state.calcCrd(rr,cc));
        if(!state.grid[crd].water&&!state.grid[crd].guarded&&safe(rr,cc)){
            state.bug << "walkable square" << endl;
            if(state.grid[crd].visits<visits){
                visits=state.grid[crd].visits;
                state.bug << "new best square found (" << visits << ")" << endl;
                direction=currentdir;
            }
        }else{
            state.bug << "non walkable square" << endl;
        }
    }
    rr=ant->row();
    for(cc=ant->col()-1;cc<=ant->col()+1;cc+=2){
        cc<ant->col() ? currentdir=3 : currentdir=1;
        int crd(state.calcCrd(rr,cc));
        if(!state.grid[crd].water&&!state.grid[crd].guarded&&safe(rr,cc)){
            state.bug << "walkable square" << endl;
            if(state.grid[crd].visits<visits){
                state.bug << "new best square found" << endl;
                visits=state.grid[crd].visits;
                direction=currentdir;
            }
        }
    }
    if(direction>=0){
        state.bug << "Pushing path" << endl;
        ant->path.push(direction);
        ant->lastDir=direction;
    }else{
        state.bug << "Unable to move" << endl;
    }
}

/** return pointer to closest free ant
*   the returned ant is also removed from vector.
*   The returned ant will keep all its settings, like path and target. Make sure you handle this.
*   \arg ants to check (these are considered to be free for work)
*   \arg target location
*   \return closest ant
*/
Ant* mANTiS::closestFreeAnt(std::vector<Ant*> &a, const Location &l, bool ignorePath){
    //state.bug << "==closestFreeAnt==" << endl;
    if(a.empty()) return 0;
    std::vector<Ant*>::iterator it, ptr;
    Ant* ant(0);
    unsigned int dist, d(-1);
    for(it=a.begin();it!=a.end();it++){
        if(ignorePath){
            dist=state.manhattan((*it)->location,l);
        }else{
            dist=state.manhattan((*it)->target,l)+(*it)->path.size();
        }
        if(dist<d){
            d=dist;
            ptr=it;
            ant=*it;
        }
        //state.bug << "Trying " << *(*it) << " - " << dist << endl;
        if(state.timer.getTime()>state.maxtime) break; // Abort if we are running out of time.
    }
    a.erase(ptr); // remove ant from list so it's not used again.
    //state.bug << "closest " << *ant << " with dist " << d << endl;
    return ant;
}

/** find ant closest to Location, ignoring destroyer and guarding ants.
*   This function was not made to be general and is only called from destroyHill
*   \arg Location
*   \return ant closest to location
*/
std::vector<Ant>::iterator mANTiS::closestAnt(const Location &l){
    double tracker(state.timer.getTime());
    state.bug << "closestAnt" << endl;
    if(state.myAnts.empty()) return state.myAnts.end();
    //state.bug << "Ants found" << endl;
    std::vector<Ant>::iterator ptr(state.myAnts.end()); //
    unsigned int dist, d(-1);
    for(std::vector<Ant>::iterator it(state.myAnts.begin());it!=state.myAnts.end();it++){
        //state.bug << "task " << it->task << endl;
        if(it->task!=Ant::DESTROY&&it->task!=Ant::GUARD){
            dist=state.manhattan(it->location,l);
            if(dist<d){
                //state.bug << "new dist " << dist << endl;
                d=dist;
                ptr=it;
            }
        }else{
            state.bug << "destroyer ant" << endl;
        }
        if(state.timer.getTime()>state.maxtime){ state.bug << "Aborting.." << endl; break;} // Abort if we are running out of time.
    }
    state.bug << "closest ant " << *ptr << endl;
    state.bug << "closestAnt took " << state.timer.getTime()-tracker << endl;
    return ptr;
}


/** returns number of ants which are either idle or exploring
*   also detects which ants have reached their target
*   consider exploring as secondary to other gathering food and fighting
*   store pointers to available ants in vector
*   \arg vector where pointers to free ants will be stored.
*   \arg if true GUARD ants will also be added to list. Normally only IDLE and EXPLORING ants are added.
*   \return number of ants added.
**/
int mANTiS::freeAnts(std::vector<Ant*> &a,  bool bAggressive){
    int n=0;
    if(!a.empty()) a.clear();
    std::vector<Ant>::iterator it;
    for(it=state.myAnts.begin();it!=state.myAnts.end();it++){
        if(bAggressive){
            if(it->task!=it->GUARD){
                n++;
                a.push_back(&(*it));
                state.bug << "free ant(aggr): " << *it << endl;
            }
        }else{
            if(it->task==it->IDLE||it->task==it->EXPLORE){
                n++;
                a.push_back(&(*it));
                state.bug << "free ant: " << *it << endl;
            }
        }
    }
    return n;
}

/** returns number of ants which are either idle or exploring
*   @return number of ants **/
int mANTiS::freeAnts(){
    int n=0;
    std::vector<Ant>::const_iterator it;
    for(it=state.myAnts.begin();it!=state.myAnts.end();it++){
        if(it->task==it->IDLE||it->task==it->EXPLORE) n++;
    }
    return n;
}

/** find idle and exploring ants **/
int mANTiS::collectAnts(std::vector<Ant*> &a){
    int n=0;
    std::vector<Ant>::iterator it;
    for(it=state.myAnts.begin();it!=state.myAnts.end();it++){
        if(it->task==it->IDLE||it->task==it->EXPLORE){
            n++;
            a.push_back(&(*it));
            state.bug << "free ant: " << *it << endl;
        }
    }
    return n;
}

/** move ants
*   ants with defined paths will be moved. **/
void mANTiS::makeMoves(){
    state.bug << "MakeMoves for " << state.myAnts.size() << " ants." << endl;
    std::vector<Ant>::iterator it(state.myAnts.begin());
    while(it!=state.myAnts.end()){
        state.bug << "Path size for " << *it << " is " << it->path.size() << endl;
        if(!it->path.empty()){
            if(!walkable(it)){
                it->resetPath();
                it->expected=it->location;
                it->target=Location(-1,-1);
                it->task=Ant::IDLE;
            }else if(!collision(it)){ // check if ant will collide with ant or end up on another ants expected location. If so just wait a turn.
                state.bug << "try to move " << *it << endl;
                state.makeMove(it->location, it->movePath());
                state.bug << "Ant moved: " << *it << endl;
            }
        }
        it++;
    }
    state.bug << state << endl;
}

/** return true if move is ok and false if ant will collide with other ant **/
bool mANTiS::collision(std::vector<Ant>::iterator itCurrent){
    state.bug << "Collision check for" << *itCurrent << endl;
    Location lExpected(itCurrent->tryMovePath());
    state.bug << "Expected location: " << lExpected << endl;
    std::vector<Ant>::iterator it;
    for(it=state.myAnts.begin();it!=state.myAnts.end();it++){
        if( (lExpected==it->expected||lExpected==it->location) && it->location!=itCurrent->location ){
            state.bug << "Colliding with: " << it->expected << " at " << it->location << endl;
            itCurrent->addCollision();
            return true;
        }
    }
    state.bug << "No collision" << endl;
    return false;
}

bool mANTiS::walkable(std::vector<Ant>::iterator itCurrent){
    state.bug << "Terrain check for" << *itCurrent << endl;
    Location lExpected(itCurrent->tryMovePath());
    state.bug << "Expected location: " << lExpected << endl;
    if(state.grid[state.calcCrd(lExpected.row, lExpected.col)].water||state.grid[state.calcCrd(lExpected.row,lExpected.col)].guarded)
        return false;
    return true;
}

void mANTiS::endTurn(){
    if(state.turn>0) state.reset();
    state.turn++;
    cout << "go" << endl;
    state.bug << "==== Endturn " << state.turn << ". Time: " << state.timer.getTime() << " ====" << endl;
}
