#include "bits/stdc++.h"

#include "../sayama/board_loader.hpp"

using namespace std;
using boardloader::Cell;
using boardloader::load_board;
using boardloader::print_table;

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
    Right = 0,
    Down  = 1,
    Left  = 2,
    Up    = 3,
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
    vector<Action> action_log;
    
    bool isIntersectLineSeg( double y1, double x1, double y2, double x2, double y3, double x3, double y4, double x4 ){
        double ta=(x1-x2)*(y3-y1)+(y1-y2)*(x1-x3);
        double tb=(x1-x2)*(y4-y1)+(y1-y2)*(x1-x4);
        if( ta*tb<0 ) return true;
        else return false;
    }
    
    bool isIntersect( double y1, double x1, double y2, double x2, double y3, double x3, double y4, double x4 ){
        if( isIntersectLineSeg( y1, x1, y2, x2, y3, x3, y4, x4 ) and isIntersectLineSeg( y3, x3, y4, x4, y1, x1, y2, x2 ) ){
            return true;
        }else{
            return false;
        }
    }
    
    bool isReachable( int src_y, int src_x, int dst_y, int dst_x ){
        int dy[] = {0, 0, 1, 1, 0};
        int dx[] = {0, 1, 1, 0, 0};
        for( int i=min(src_y, dst_y); i<max(src_y, dst_y); ++i ){
            for( int j=min(src_x, dst_x); j<max(src_x, dst_x); ++j ){
                if( field[i][j] != boardloader::Obstacle )continue;
                for( int k=0; k<4; ++k ){
                    if( isIntersect( src_y+0.5, src_x+0.5, dst_y+0.5, dst_x+0.5, i+dy[k], j+dx[k], i+dy[k+1], j+dx[k+1] ) ){
                        return false;
                    }
                }
            }
        }
        return true;
    }
    
    void applyPaint( const int& cur_y, const int& cur_x, const vector<pair<int,int>>& manip ){
        assert( field[cur_y][cur_x] != boardloader::Obstacle );
        field[cur_y][cur_x] = boardloader::Occupied;
        
        for( int i=0; i<manipulators_.size(); ++i ){
            int ty, tx;
            tie(ty, tx) = manipulators_[i];
            if( ty<0 or ty>=H or tx<0 or tx>=W )continue;
            if( field[ty][tx] != boardloader::Obstacle && not isReachable( cur_y, cur_x, ty, tx ) ){
                field[ty][tx] = boardloader::Occupied;
            }
        }
    }
    
    Worker( int yy, int xx, vector<vector<Cell>> &field ): y(yy), x(xx), dir_(Right), field(field){
        H = (int)field.size();
        W = (int)field[0].size();
        
        applyPaint( y, x, manipulators_ );
        speed_ = 1;
        
        manipulators_.push_back( make_pair( y, x ) );
        for( int i=0; i<MANIP_NUM; ++i ){
            manipulators_.push_back( make_pair( y + manip_y[i], x + manip_x[i] ) );
        }
    }
    
    bool doAction( Action act ){
        if( act.opcode == "W" ){ // move_up
            if( y+1 >= H or field[y+1][x]==boardloader::Obstacle ){
                return false;
            }else{
                int y_new = y;
                for( int d=1; d<=speed_; ++d ){
                    int moved_y = y+d;
                    int moved_x = x;
                    
                    if( moved_y<0 or H<=moved_y or moved_x<0 or W<=moved_x or field[moved_y][moved_x]==boardloader::Obstacle )break;
                    if( field[moved_y][moved_x] == boardloader::ManipulatorExtension ) items_[ B ]++;
                    if( field[moved_y][moved_x] == boardloader::FastWheels           ) items_[ F ]++;
                    if( field[moved_y][moved_x] == boardloader::Drill                ) items_[ L ]++;
                    if( field[moved_y][moved_x] == boardloader::Mysterious           ) items_[ X ]++;
                    
                    applyPaint( moved_y, moved_x, manipulators_ );
                    
                    y_new = y+d;
                }
                assert( y_new < H );
                y = y_new;
                
                action_log.push_back( act );
                return true;
            }
            
        }else if( act.opcode == "S" ){ // move_down
            if( y-1 < 0 or field[y-1][x] == boardloader::Obstacle ){
                return false;
            }else{
                int y_new = y-1;
                for( int d=1; d<=speed_; ++d ){
                    int moved_y = y-d;
                    int moved_x = x;
                    
                    if( moved_y<0 or H<=moved_y or moved_x<0 or W<=moved_x or field[moved_y][moved_x]==boardloader::Obstacle )break;
                    if( field[moved_y][moved_x] == boardloader::ManipulatorExtension ) items_[ B ]++;
                    if( field[moved_y][moved_x] == boardloader::FastWheels           ) items_[ F ]++;
                    if( field[moved_y][moved_x] == boardloader::Drill                ) items_[ L ]++;
                    if( field[moved_y][moved_x] == boardloader::Mysterious           ) items_[ X ]++;
                    
                    applyPaint( moved_y, moved_x, manipulators_ );
                    
                    y_new = y-d;
                }
                assert( y_new>=0 );
                y = y_new;
                
                action_log.push_back( act );
                return true;
            }
            
        }else if( act.opcode == "A" ){ // move_left
            if( x-1<0 or field[y][x-1] == boardloader::Obstacle ){
                return false;
            }else{
                int x_new = x-1;
                for( int d=1; d<=speed_; ++d ){
                    int moved_y = y;
                    int moved_x = x-d;
                    
                    if( moved_y<0 or H<=moved_y or moved_x<0 or W<=moved_x or field[moved_y][moved_x]==boardloader::Obstacle )break;
                    if( field[moved_y][moved_x] == boardloader::ManipulatorExtension ) items_[ B ]++;
                    if( field[moved_y][moved_x] == boardloader::FastWheels           ) items_[ F ]++;
                    if( field[moved_y][moved_x] == boardloader::Drill                ) items_[ L ]++;
                    if( field[moved_y][moved_x] == boardloader::Mysterious           ) items_[ X ]++;
                    
                    applyPaint( moved_y, moved_x, manipulators_ );
                    
                    x_new = x-d;
                }
                assert( x_new >= 0 );
                x = x_new;
                
                action_log.push_back( act );
                return true;
            }
            
        }else if( act.opcode == "D" ){ // move_right
            if( x+1>=W or field[y][x+1] == boardloader::Obstacle ){
                return false;
            }else{
                int x_new = x+1;
                for( int d=1; d<=speed_; ++d ){
                    int moved_y = y;
                    int moved_x = x+d;
                    
                    if( moved_y<0 or H<=moved_y or moved_x<0 or W<=moved_x or field[moved_y][moved_x]==boardloader::Obstacle )break;
                    if( field[moved_y][moved_x] == boardloader::ManipulatorExtension ) items_[ B ]++;
                    if( field[moved_y][moved_x] == boardloader::FastWheels           ) items_[ F ]++;
                    if( field[moved_y][moved_x] == boardloader::Drill                ) items_[ L ]++;
                    if( field[moved_y][moved_x] == boardloader::Mysterious           ) items_[ X ]++;
                    
                    applyPaint( moved_y, moved_x, manipulators_ );
                    
                    x_new = x+d;
                }
                assert( x_new < W );
                x = x_new;
                
                action_log.push_back( act );
                return true;
            }
            
        }else if( act.opcode == "Z" ){ // do_nothing
            action_log.push_back( act );
            return true;
            
        }else if( act.opcode == "E" ){ // turn_clockwize
            dir_ = static_cast<Direction>((dir_+1)%4);
            for( int i=0; i<manipulators_.size(); ++i ){
                manipulators_[i] = make_pair( manipulators_[i].second, -manipulators_[i].first );
            }
            
            action_log.push_back( act );
            return true;
            
        }else if( act.opcode == "Q" ){ // turn_counterclockwize
            dir_ = static_cast<Direction>((dir_+3)%4);
            
            for( int i=0; i<manipulators_.size(); ++i ){
                manipulators_[i] = make_pair( -manipulators_[i].second, manipulators_[i].first );
            }
            
            action_log.push_back( act );
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
        
        if( field[y+dy][x+dx] == boardloader::Obstacle ){
            return false;
        }else{
            return true;
        }
    }
    
    
    bool hasItem( Item item ){
        assert( items_[item] >= 0 );
        return (bool)items_[item];
    }
    
    int countItem( Item item ){
        assert( items_[item] >= 0 );
        return items_[item];
    }
    
    void dump_actions( ofstream& ofs ){
        for( auto elm: action_log ){
            if( elm.operand == make_pair(-1, -1) ){
                ofs << elm.opcode;
            }else{
                if( elm.opcode == "B" ){
                    ofs << "B" << "(" << elm.operand.first << "," << elm.operand.second << ")";
                }
            }
        }
    }
    
};


void dfs( Worker &robot, vector<vector<bool>>& occupied ){
    //    cout << robot.y << " " << robot.x << endl;
    
    int dy[] = {0, -1, 0, 1};
    int dx[] = {1, 0, -1, 0};
    string direction[] = {"D", "S", "A", "W"};
    
    for( int k=0; k<4; ++k ){
        int ddy = robot.y + dy[k];
        int ddx = robot.x + dx[k];
        if( ddy<0 or ddy>=occupied.size() or ddx<0 or ddx>=occupied[0].size() )continue;
        if( not occupied[ddy][ddx] and robot.field[ddy][ddx] != boardloader::Obstacle ){
            occupied[ddy][ddx] = true;
            assert( robot.doAction( Action(direction[k]) ) );
            
            //無意味にキョロキョロ
            if( rand()%10 == 0 )robot.doAction( Action("Q") );
            if( rand()%15 == 0 )robot.doAction( Action("E") );

            dfs( robot, occupied );
            
            assert( robot.doAction( Action(direction[(k+2)%4]) ) );
            
        }
    }
}


int main(){
    int start_y, start_x;
    string dirname = "/Users/ashibata/GitHub/icfpc2019/problems/";
    
    for( int i=2; i<=2; ++i ){
        stringstream ss;
        ss << "prob-" << setfill('0') << setw(3) << right << i;
        string filename = ss.str();
        cout << filename <<endl;
        
        vector<vector<Cell>> field = load_board( dirname + filename + ".desc", start_y, start_x );
        Worker robot( start_y, start_x, field );
        
        vector<vector<bool>> occupied( field.size(), vector<bool>(field[0].size(), false) );
        occupied[robot.y][robot.x] = true;
        dfs( robot, occupied );
        
        ofstream sol_fs( filename + ".sol" );
        robot.dump_actions( sol_fs );
        sol_fs.close();
    }
}
