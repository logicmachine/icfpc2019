#include "bits/stdc++.h"
using namespace std;

const int MANIP_NUM = 3;
const int manip_y[] = {1, 0, -1};
const int manip_x[] = {1, 1, 1};


enum Item{
    B,
    F,  //Fast
    L,  //Drill
    X,
};

enum Direction{
    Right,
    Down,
    Left,
    Up,
};

enum Cell
{
    Empty,
    Obstacle,
    ManipulatorExtension,
    FastWheels,
    Drill,
    Mysterious,
    Occupied,
};

void warning( string msg ){ cerr << "Warning : " << msg << endl; }
void error  ( string msg ){ cerr << "Error   : " << msg << endl; }


class Action{
public:
    string opcode;
    pair<int,int> operand;
    string additional;
    
    Action( string opc ): opcode(opc) { operand = make_pair( -1, -1 ); }
    Action( string opc, pair<int,int> oper ): opcode(opc), operand(oper) {}
    Action( string opc, pair<int,int> oper, string addit ): opcode(opc), operand(oper), additional(addit) {}
    
};


class Worker{
    int H, W;
    Direction dir_;
    vector<pair<int,int>> manipulators_;
    map<Item,int> items_;
    int speed_;
    
public:
    int y, x;
    vector<vector<Cell>> field;
    
    Worker( int yy, int xx, vector<vector<Cell>> &field ): y(yy), x(xx), dir_(Right), field(field){
        H = (int)field.size();
        W = (int)field[0].size();
        
        speed_ = 1;
        
        manipulators_.push_back( make_pair( y, x ) );
        for( int i=0; i<MANIP_NUM; ++i ){
            manipulators_.push_back( make_pair( y + manip_y[i], x + manip_x[i] ) );
        }
    }
    
    bool do_action( Action act ){
        if( act.opcode == "W" ){ // move_up
            if( y+1 >= H or field[y+1][x]==Obstacle ){
                return false;
            }else{
                int y_new = y;
                for( int d=1; d<=speed_; ++d ){
                    int moved_y = y+d;
                    int moved_x = x;
                    if( moved_y<0 or H<=moved_y or moved_x<0 or W<=moved_x or field[moved_y][moved_x]==Obstacle )break;
                    if( field[moved_y][moved_x] == ManipulatorExtension ) items_[ B ]++;
                    if( field[moved_y][moved_x] == FastWheels           ) items_[ F ]++;
                    if( field[moved_y][moved_x] == Drill                ) items_[ L ]++;
                    if( field[moved_y][moved_x] == Mysterious           ) items_[ X ]++;
                    
                    y_new = y+d;
                }
                assert( y_new < H );
                y = y_new;
                return true;
            }
            
        }else if( act.opcode == "S" ){ // move_down
            if( y-1 < 0 or field[y-1][x] == Obstacle ){
                return false;
            }else{
                int y_new = y-1;
                for( int d=1; d<=speed_; ++d ){
                    int moved_y = y+d;
                    int moved_x = x;
                    if( moved_y<0 or H<=moved_y or moved_x<0 or W<=moved_x or field[moved_y][moved_x]==Obstacle )break;
                    if( field[moved_y][moved_x] == ManipulatorExtension ) items_[ B ]++;
                    if( field[moved_y][moved_x] == FastWheels           ) items_[ F ]++;
                    if( field[moved_y][moved_x] == Drill                ) items_[ L ]++;
                    if( field[moved_y][moved_x] == Mysterious           ) items_[ X ]++;

                    y_new = y-d;
                }
                assert( y_new>=0 );
                y = y_new;
                return true;
            }
            
        }else if( act.opcode == "A" ){ // move_left
            if( x-1<0 or field[y][x-1] == Obstacle ){
                return false;
            }else{
                int x_new = x-1;
                for( int d=1; d<=speed_; ++d ){
                    int moved_y = y+d;
                    int moved_x = x;
                    if( moved_y<0 or H<=moved_y or moved_x<0 or W<=moved_x or field[moved_y][moved_x]==Obstacle )break;
                    if( field[moved_y][moved_x] == ManipulatorExtension ) items_[ B ]++;
                    if( field[moved_y][moved_x] == FastWheels           ) items_[ F ]++;
                    if( field[moved_y][moved_x] == Drill                ) items_[ L ]++;
                    if( field[moved_y][moved_x] == Mysterious           ) items_[ X ]++;

                    x_new = x-d;
                }
                assert( x_new >= 0 );
                x = x_new;
                return true;
            }
            
        }else if( act.opcode == "D" ){ // move_right
            if( x+1>=W or field[y][x+1] == Obstacle ){
                return false;
            }else{
                int x_new = x+1;
                for( int d=1; d<=speed_; ++d ){
                    int moved_y = y+d;
                    int moved_x = x;
                    if( moved_y<0 or H<=moved_y or moved_x<0 or W<=moved_x or field[moved_y][moved_x]==Obstacle )break;
                    if( field[moved_y][moved_x] == ManipulatorExtension ) items_[ B ]++;
                    if( field[moved_y][moved_x] == FastWheels           ) items_[ F ]++;
                    if( field[moved_y][moved_x] == Drill                ) items_[ L ]++;
                    if( field[moved_y][moved_x] == Mysterious           ) items_[ X ]++;

                    x_new = x+d;
                }
                assert( x_new < W );
                x = x_new;
                return true;
            }
            
        }else if( act.opcode == "Z" ){ // do_nothing
            return true;
            
        }else if( act.opcode == "E" ){ // turn_clockwize
            if( dir_ == Right){
                dir_ = Down;
            }else if( dir_ == Down ){
                dir_ = Left;
            }else if( dir_ == Left ){
                dir_ = Up;
            }else if( dir_ == Up ){
                dir_ = Right;
            }
            
            for( int i=0; i<manipulators_.size(); ++i ){
                manipulators_[i] = make_pair( manipulators_[i].second, -manipulators_[i].first );
            }

            return true;
            
        }else if( act.opcode == "Q" ){ // turn_counterclockwize
            if( dir_ == Right){
                dir_ = Up;
            }else if( dir_ == Up ){
                dir_ = Left;
            }else if( dir_ == Left ){
                dir_ = Down;
            }else if( dir_ == Down ){
                dir_ = Right;
            }
            
            for( int i=0; i<manipulators_.size(); ++i ){
                manipulators_[i] = make_pair( -manipulators_[i].second, manipulators_[i].first );
            }
            
            return true;
            
        }else if( act.opcode == "B" ){ // attach B
            assert( act.operand.first  == -1 );
            assert( act.operand.second == -1 );
            warning( "Not implemented" );
            return false;
            
        }else if( act.opcode == "F" ){ // attach Fast
            warning( "Not implemented" );
            return false;
            
        }else{
            error( "The action " + act.opcode + " is not defined" );
            return false;
        }
    }
    
    bool isMovable( int dy, int dx ){
        assert( abs(dy+dx) == 1 and (dy==0 or dx==0) );
        if( y+dy<0 or y+dy>=H or x+dx<0 or x+dx>=W )return false;
        
        if( field[y+dy][x+dx] == Obstacle ){
            return false;
        }else{
            return true;
        }
    }
    
    bool isThereNeedsToPaint( int dy, int dx ){
        if( y+dy<0 or y+dy>=H or x+dx<0 or x+dx>=W )return false;
        
        if( field[y+dy][x+dx] == Occupied ){
            return false;
        }else{
            return true;
        }
    }
    
    vector<pair<int,int>> paintable_cells(){
        warning( "Not implemented" );
        return vector<pair<int,int>>();
    }
    
    vector<pair<int,int>> additional_painted_cells(){
        warning( "Not implemented" );
        return vector<pair<int,int>>();
    }
    
    bool has( Item item ){
        assert( items_[item] >= 0 );
        return (bool)items_[item];
    }
    
    int count( Item item ){
        assert( items_[item] >= 0 );
        return items_[item];
    }
};


void dfs( Worker &robot, vector<Action>& actions ){
    robot.field[ robot.y ][ robot.x ] = Occupied;
    
    
    int dy[] = {0, -1, 0, 1};
    int dx[] = {1, 0, -1, 0};
    string direction[] = {"D", "S", "A", "W"};
    
    for( int k=0; k<4; ++k ){
        if( robot.isThereNeedsToPaint( dy[k], dx[k] ) ){
            robot.do_action( Action(direction[k]) );
            actions.push_back( Action(direction[k]) );
            
            dfs( robot, actions );
            
            robot.do_action( Action(direction[(k+2)%4]) );
            actions.push_back( Action(direction[(k+2)%4]) );
            
        }
    }
}

vector<vector<Cell>> dummy_load( int& start_y, int& start_x ){
    vector<vector<Cell>> ret(10, vector<Cell>(10, Empty));
    start_y = 0;
    start_x = 0;
    return ret;
}


int main(){
    int start_y, start_x;
    //    vector<vector<Cell>> field = sayama_function( "example-01.desc", start_y, start_x );
    vector<vector<Cell>> field = dummy_load( start_y, start_x );
    Worker robot( start_y, start_x, field );
    
    vector<Action> actions;
    dfs( robot, actions );
    
    for( auto elm: actions ){
        if( elm.operand == make_pair(-1, -1) ){
            cout << elm.opcode;
        }else{
            if( elm.opcode == "B" ){
                cout << "B" << "(" << elm.operand.first << "," << elm.operand.second << ")";
            }
        }
    }
    cout << endl;
}
