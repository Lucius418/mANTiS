#include "AStar.h"
/** Destructor. Free allocated memory and close logfile **/
AStar::~AStar(){
    bug << "Destructor called" << endl;
    freeMem();
    bug << "cleanup completed" << endl;
    //bug.close();
}

/** AStar constructor
*   \arg pointer to the map used.
*   \arg rows in map (map is just a vector, we must know how to use it).
*   \arg columns in map.
*/
AStar::AStar(std::vector<Square> *grid, const int rows,const int cols)
    : mvp_grid(grid), m_rows(rows), m_cols(cols){
    //bug.open("./aStar.txt");
    bug << "AStar logging started" << endl;
    m_minRow=-1*m_rows; m_maxRow=2*m_rows;
    m_minCol=-1*m_cols; m_maxCol=2*m_cols;
}

/** Find shortest path to a square.
*   If the path can't be found (not possible if called with a valid ant/goal combination) or if the time runs out
*   the function will return false and no changes are made to the ant. If a path is found it's saved to ant.path
*   and ant.target is set to the goal location.
*   \arg The ant who needs to find its way
*   \arg Where the ant should go
*   \arg maximum time that can be used to find the path
*   \return true if path is found, false if not.
*/
bool AStar::findPath(Ant &ant, const Location &goal,const double timeleft) {
    bug << "==findPath== " << ant << " - " << goal << endl;
    if(ant.location==goal){ant.target=goal; return true;} // do a quick exit if we're trying to find a path to the current location.
    if(goal.row<0||goal.row>=m_rows||goal.col<0||goal.col>=m_cols) return false;
    m_goal=goal;
    m_time=timeleft;
    m_timer.start();
    unsigned int goalCrd(calcCrd(m_goal));
    addFirstToOpen(ant.location);
    m_atGoal=false;
    std::multiset<aLocation*>::iterator it;
    bug << m_atGoal << " - times " << m_timer.getTime() << ":" << m_time << " - open nodes " << mv_Open.size() << endl;
    while(!m_atGoal && m_timer.getTime()<m_time && !mv_Open.empty()){
        bug << "==Head of main loop==" << endl;
        it=mv_Open.begin();
        bug << "it " << *it << endl;
        addToOpen(*it);
        //bug << "Adding to closed." << endl;
        mv_Closed.push_back(*it);
        if(goalCrd==calcCrd((**it))) { // ToDo:check for goal area instead of single square
            bug << "Goal found!" << endl;
            m_atGoal=true;
        }
        //bug << "Erasing it " << *it << endl;
        mv_Open.erase(it);
    }
    if(m_atGoal/*||!mv_Open.empty()*/){
        bug << "Calling savePath" << endl;
        savePath(ant, *it);
        return true;
    }else{
        bug << "No path to save" << endl;
        freeMem();
        return false;
    }
}

/** trace path back to beginning and save it to ants path.
*   \arg Ant ant to update
*   \arg loccation Goal location
*/
void AStar::savePath(Ant &a, const aLocation *al){
    bug << "savePath. Tracing path from " << *al << endl;
    aLocation const *cur=al, *parent;
    while(cur->parent!=0){
        parent=cur;
        cur=cur->parent;
        bug << "Pushing " << *cur << ", " << cur << "(";
        (*mvp_grid)[calcCrd(cur->row, cur->col)].path++;
        if(cur->col<parent->col){
            a.path.push(1); // E
            bug << "1";
        }else if(cur->col>parent->col){
            a.path.push(3); // W
            bug << "3";
        }else if(cur->row<parent->row){
            a.path.push(2); // S
            bug << "2";
        }else if(cur->row>parent->row){
            a.path.push(0); // N
            bug << "0";
        }
        bug << ") to path" << endl;
    }
    a.target=*al;   // Update ants target so that we know where it's going.
                    // This is used when clearing ants (and possibly other occations).
                    // For instance we can check if an ant is trying to gather food someone else already took.
    if(a.target.row<0) {a.target.row+=m_rows;} else if(a.target.row>=m_rows){a.target.row-=m_rows;}
    if(a.target.col<0) {a.target.col+=m_cols;} else if(a.target.col>=m_cols){a.target.col-=m_cols;}
    bug << "Path saved." << endl;

    // Free memory when path is saved.
    freeMem();
}

/** free allocated memory **/
void AStar::freeMem(){
    for(std::multiset<aLocation*>::iterator it(mv_Open.begin());it!=mv_Open.end();it++){
        delete *it;
    }
    for(std::vector<aLocation*>::iterator it(mv_Closed.begin());it!=mv_Closed.end();it++){
        delete *it;
    }
    mv_Open.clear();
    mv_Closed.clear();
    bug << "cleanup completed" << endl;
}

/** Check if node is in closed list
*   \return bool true if node is in closed list
*   \param location location to find
*/
inline bool AStar::closed(const Location &l){
    return (find(mv_Closed.begin(), mv_Closed.end(), l)==mv_Closed.end()) ? false : true;
}

/** Check if node is in open list
*   \return bool true if node is in open list
*   \param location location to find
*/
inline bool AStar::open(const Location &l){
    return (find(mv_Open.begin(), mv_Open.end(), l)==mv_Open.end()) ? false : true;
}

/** Check if node is in open or closed list
*   \return bool true if node is in open or closed list
*   \param location location to find
*/
inline bool AStar::used(const Location &l){
    return (closed(l)||open(l)) ? true : false;
}

/** Add start node to open list
*   \param start location
*/
void AStar::addFirstToOpen(Location &l){
    bug << "addFirstToOpen. Adding start location." << endl;
    aLocation *al=new aLocation(l);
    al->g=1;
    al->h=manhattanDist(l, m_goal);
    al->parent=0;
    addToOpen(al);
    mv_Closed.push_back(al);
}

/** Allowing coordinats outside the real map allows for pathfinding on wrapping map.
*   Coordinates outside map are translated and checked agains the real map for terrain.
*   Since the path (in savePath and Ant.path) only uses directions this works perfectly.
*   \arg aLocation location from wich to find new nodes. New nodes are added to mv_Open
*/
void AStar::addToOpen(const aLocation *al){
    bug << "addToopen. Adding " << *al << ", " << al << endl;
    int rr, cc, c;
    cc=al->col;
    for(rr=al->row-1;rr<=al->row+1;rr+=2){
        if(rr>m_minRow&&rr<m_maxRow){
            Location l(rr,cc);
            c=cost(rr,cc);
            if(c!=0&&!used(l)){
                aLocation *a=new aLocation(rr,cc);
                a->g=al->g+c;
                a->h=manhattanDist(*a, m_goal);
                a->parent=al;
                bug << "addToOpen. Inserting " << *a << ", " << a->g << ":"  << a->h << ":" << a->f() << " with parent " << *a->parent << endl;
                mv_Open.insert(a);
            }
        }
    }
    rr=al->row;
    for(cc=al->col-1;cc<=al->col+1;cc+=2){
        if(cc>m_minCol&&cc<m_maxCol){
            Location l(rr,cc);
            c=cost(rr,cc);
            if(c!=0&&!used(l)){
                aLocation *a=new aLocation(rr,cc);
                a->g=al->g+c;
                a->h=manhattanDist(*a, m_goal);
                a->parent=al;
                bug << "addToOpen. Inserting " << *a << ", " << a->g << ":"  << a->h << ":" << a->f() << " with parent " << *a->parent << endl;
                mv_Open.insert(a);
            }
        }
    }
    bug << "Done adding to open" << endl;
}

/** Return vector position for coordinates
*   The function allows for "virtual" coordinates outside the real map
*   and transforms them to coordinates inside the map.
*   \arg row
*   \arg col
*   \return id item id in grid vector
*/
inline unsigned int AStar::calcCrd(int r, int c){
    if(r<0) {
        r+=m_rows;
    }else if(r>=m_rows){
        r-=m_rows;
    }
    if(c<0) {
        c+=m_cols;
    }else if(c>=m_cols) {
        c-=m_cols;
    }
    return r*m_cols+c;
}

/** see calcCrd(int, int) **/
inline unsigned int AStar::calcCrd(const aLocation &l){
    return calcCrd(l.row, l.col);
}

/** Check the cost for a square. returns 0 if square is unusable.
*   \return short cost for square. 0=not usable, 1=normal, 3=already used path, 20=unknown
*   \param int row
*   \param int column
**/
inline short AStar::cost(int r, int c){
    unsigned int crd(calcCrd(r,c));
    if((*mvp_grid)[crd].water||(*mvp_grid)[crd].guarded) return 0;
    if((*mvp_grid)[crd].path>0) return 2; //adding a cost for treading someone elses path make collisions less likely
    if((*mvp_grid)[crd].explored) return 1;
    return 5; // a high value for unknown terrain will search large areas to find a way around and that takes time.
}

/** calculate manhattan distance between 2 locations.
*   \return distance
*   \param location 1
*   \param location 2
*/
unsigned int AStar::manhattanDist(const Location &l1, const Location &l2){
    using namespace std;
    int r=abs(l1.row-l2.row); // distance between points
    int c=abs(l1.col-l2.col);
    int dr=abs(min(r, m_rows-r)); // distance with wrapping
    int dc=abs(min(c, m_cols-c));
    return (dr+dc)*3; // return distance in squares
    // pessimistic heuristic makes aStar work faster but less likely to find best path.
}
