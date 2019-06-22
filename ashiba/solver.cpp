#include "bits/stdc++.h"
#include "unistd.h"

#include "../sayama/board_loader.hpp"
#include "../ikeda/ikeda.hpp"

using namespace std;
using boardloader::Cell;
using boardloader::load_board;
using boardloader::print_table;

const int DEFAULT_MANIP_NUM = 3;
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
    vector<vector<pair<int,int>>> manipulators_;
    vector<vector<Action>> action_log;
    vector<Direction> dir_;
    vector<int> speed_;
    
    map<Item,int> items_;
    int robot_num;
    
    
public:
    vector<int> y, x;
    vector<vector<Cell>> field;
    
    Worker( int yy, int xx, vector<vector<Cell>> &field ): field(field){
        H = (int)field.size();
        W = (int)field[0].size();
        
        robot_num = 1;
        
        manipulators_.resize( robot_num );
        action_log   .resize( robot_num );
        dir_         .resize( robot_num );
        speed_       .resize( robot_num );
        y            .resize( robot_num );
        x            .resize( robot_num );
        
        y[0]      = yy;
        x[0]      = xx;
        dir_[0]   = Right;
        speed_[0] = 1;
        
        manipulators_[0].push_back( make_pair( 0, 0 ) );
        for( int i=0; i<DEFAULT_MANIP_NUM; ++i ){
            manipulators_[0].push_back( make_pair( manip_y[i], manip_x[i] ) );
        }
        
        applyPaint( y[0], x[0], manipulators_[0] );
        
        assert( items_.size() == 0 );
        assert( robot_num == 1 );
    }
    
    
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
                for( int k=0; k<4; ++k ){
                    if( field[i][j] == boardloader::Obstacle and isIntersect( src_y+0.5, src_x+0.5, dst_y+0.5, dst_x+0.5, i+dy[k], j+dx[k], i+dy[k+1], j+dx[k+1] ) ){
                        cout << "field[" << i << "][" << j << "] = " << field[i][j] << " " << "intersect : " << src_y+0.5 << "_" << src_x+0.5 << " " << dst_y+0.5 << "_" << dst_x+0.5 << " " << i+dy[k] << "_" << j+dx[k] << " " << i+dy[k+1] << "_" << j+dx[k+1] << endl;
                        return false;
                    }
                }
            }
        }
        return true;
    }
    
    void applyPaint( const int& cur_y, const int& cur_x, const vector<pair<int,int>>& manip ){
        assert( field[cur_y][cur_x] != boardloader::Obstacle );
        
        for( int i=0; i<manip.size(); ++i ){
            int ty, tx;
            tie(ty, tx) = manip[i];
            ty += cur_y;
            tx += cur_x;
            
            if( ty<0 or ty>=H or tx<0 or tx>=W )continue;
            
            if( field[ty][tx] == boardloader::Obstacle ){
                /* cout << "There is Obstacle " << ty << " " << tx << endl; */;
                
            }else if( not isReachable( cur_y, cur_x, ty, tx ) ){
                /* cout << "Not reachable " << ty << " " << tx << endl; */;
            }else{
                field[ty][tx] = boardloader::Occupied;
            }
        }
    }
    
    
    bool doAction( vector<Action> act_list ){
        assert( act_list.size() <= robot_num );
        
        for( int rb=0; rb<robot_num; ++rb ){
            Action act = act_list[rb];
            
            if( act.opcode == "W" ){ // move_up
                if( y[rb]+1 >= H or field[y[rb]+1][x[rb]]==boardloader::Obstacle ){
                    return false;
                }else{
                    int y_new = y[rb];
                    for( int d=1; d<=speed_[rb]; ++d ){
                        int moved_y = y[rb]+d;
                        int moved_x = x[rb];
                        
                        if( moved_y<0 or H<=moved_y or moved_x<0 or W<=moved_x or field[moved_y][moved_x]==boardloader::Obstacle )break;
                        if( field[moved_y][moved_x] == boardloader::ManipulatorExtension ) items_[ B ]++;
                        if( field[moved_y][moved_x] == boardloader::FastWheels           ) items_[ F ]++;
                        if( field[moved_y][moved_x] == boardloader::Drill                ) items_[ L ]++;
                        if( field[moved_y][moved_x] == boardloader::Mysterious           ) items_[ X ]++;
                        
                        applyPaint( moved_y, moved_x, manipulators_[rb] );
                        
                        y_new = y[rb]+d;
                    }
                    assert( y_new < H );
                    y[rb] = y_new;
                    
                }
                
            }else if( act.opcode == "S" ){ // move_down
                if( y[rb]-1 < 0 or field[y[rb]-1][x[rb]] == boardloader::Obstacle ){
                    return false;
                }else{
                    int y_new = y[rb]-1;
                    for( int d=1; d<=speed_[rb]; ++d ){
                        int moved_y = y[rb]-d;
                        int moved_x = x[rb];
                        
                        if( moved_y<0 or H<=moved_y or moved_x<0 or W<=moved_x or field[moved_y][moved_x]==boardloader::Obstacle )break;
                        if( field[moved_y][moved_x] == boardloader::ManipulatorExtension ) items_[ B ]++;
                        if( field[moved_y][moved_x] == boardloader::FastWheels           ) items_[ F ]++;
                        if( field[moved_y][moved_x] == boardloader::Drill                ) items_[ L ]++;
                        if( field[moved_y][moved_x] == boardloader::Mysterious           ) items_[ X ]++;
                        
                        applyPaint( moved_y, moved_x, manipulators_[rb] );
                        
                        y_new = y[rb]-d;
                    }
                    assert( y_new>=0 );
                    y[rb] = y_new;
                    
                }
                
            }else if( act.opcode == "A" ){ // move_left
                if( x[rb]-1<0 or field[y[rb]][x[rb]-1] == boardloader::Obstacle ){
                    return false;
                }else{
                    int x_new = x[rb]-1;
                    for( int d=1; d<=speed_[rb]; ++d ){
                        int moved_y = y[rb];
                        int moved_x = x[rb]-d;
                        
                        if( moved_y<0 or H<=moved_y or moved_x<0 or W<=moved_x or field[moved_y][moved_x]==boardloader::Obstacle )break;
                        if( field[moved_y][moved_x] == boardloader::ManipulatorExtension ) items_[ B ]++;
                        if( field[moved_y][moved_x] == boardloader::FastWheels           ) items_[ F ]++;
                        if( field[moved_y][moved_x] == boardloader::Drill                ) items_[ L ]++;
                        if( field[moved_y][moved_x] == boardloader::Mysterious           ) items_[ X ]++;
                        
                        applyPaint( moved_y, moved_x, manipulators_[rb] );
                        
                        x_new = x[rb]-d;
                    }
                    assert( x_new >= 0 );
                    x[rb] = x_new;
                    
                }
                
            }else if( act.opcode == "D" ){ // move_right
                if( x[rb]+1>=W or field[y[rb]][x[rb]+1] == boardloader::Obstacle ){
                    return false;
                }else{
                    int x_new = x[rb]+1;
                    for( int d=1; d<=speed_[rb]; ++d ){
                        int moved_y = y[rb];
                        int moved_x = x[rb]+d;
                        
                        if( moved_y<0 or H<=moved_y or moved_x<0 or W<=moved_x or field[moved_y][moved_x]==boardloader::Obstacle )break;
                        if( field[moved_y][moved_x] == boardloader::ManipulatorExtension ) items_[ B ]++;
                        if( field[moved_y][moved_x] == boardloader::FastWheels           ) items_[ F ]++;
                        if( field[moved_y][moved_x] == boardloader::Drill                ) items_[ L ]++;
                        if( field[moved_y][moved_x] == boardloader::Mysterious           ) items_[ X ]++;
                        
                        applyPaint( moved_y, moved_x, manipulators_[rb] );
                        
                        x_new = x[rb]+d;
                    }
                    assert( x_new < W );
                    x[rb] = x_new;
                    
                }
                
            }else if( act.opcode == "Z" ){ // do_nothing
                /* */;
                
            }else if( act.opcode == "E" ){ // turn_clockwize
                dir_[rb] = static_cast<Direction>((dir_[rb]+1)%4);
                for( int i=0; i<manipulators_.size(); ++i ){
                    manipulators_[rb][i] = make_pair( manipulators_[rb][i].second, -manipulators_[rb][i].first );
                }
                
            }else if( act.opcode == "Q" ){ // turn_counterclockwize
                dir_[rb] = static_cast<Direction>((dir_[rb]+3)%4);
                
                for( int i=0; i<manipulators_[rb].size(); ++i ){
                    manipulators_[rb][i] = make_pair( -manipulators_[rb][i].second, manipulators_[rb][i].first );
                }
                
            }else if( act.opcode == "B" ){ // attach B
                assert( act.operand.first  == -1 );
                assert( act.operand.second == -1 );
                warning( "Not implemented" );
                return false;
                
            }else if( act.opcode == "F" ){ // attach Fast
                warning( "Not implemented" );
                return false;
                
            }else if( act.opcode == "R" ){ // attach Fast
                warning( "Not implemented" );
                return false;
                
            }else if( act.opcode == "C" ){ // attach Fast
                warning( "Not implemented" );
                return false;
                
            }else{
                error( "The action " + act.opcode + " is not defined" );
                return false;
            }
            
            applyPaint( y[rb], x[rb], manipulators_[rb] );
            
            //boardloader::print_table( field, y[rb], x[rb] );
            //cout << endl;
            action_log[rb].push_back( act );
        }
        return true;
    }
    
    /*
     bool isMovable( int dy, int dx ){
     assert( abs(dy+dx) == 1 and (dy==0 or dx==0) );
     if( y+dy<0 or y+dy>=H or x+dx<0 or x+dx>=W )return false;
     
     if( field[y+dy][x+dx] == boardloader::Obstacle ){
     return false;
     }else{
     return true;
     }
     }
     */
    
    
    bool hasItem( Item item ){
        assert( items_[item] >= 0 );
        return (bool)items_[item];
    }
    
    int countItem( Item item ){
        assert( items_[item] >= 0 );
        return items_[item];
    }
    
    void dump_actions( ofstream& ofs ){
        assert( robot_num == action_log.size() );
        for( int rb=0; rb<robot_num; ++rb ){
            if( rb != 0 ){
                ofs << "#";
            }
            
            for( auto elm: action_log[rb] ){
                if( elm.operand == make_pair(-1, -1) ){
                    ofs << elm.opcode;
                }else{
                    if( elm.opcode == "B" ){
                        ofs << "B" << "(" << elm.operand.first << "," << elm.operand.second << ")";
                    }
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
        int ddy = robot.y[0] + dy[k];
        int ddx = robot.x[0] + dx[k];
        if( ddy<0 or ddy>=occupied.size() or ddx<0 or ddx>=occupied[0].size() )continue;
        if( not occupied[ddy][ddx] and robot.field[ddy][ddx] != boardloader::Obstacle ){
            occupied[ddy][ddx] = true;
            assert( robot.doAction( {Action(direction[k])} ) );
            
            dfs( robot, occupied );
            
            assert( robot.doAction( {Action(direction[(k+2)%4])} ) );
            
        }
    }
}

string getFilename( int n ){
    stringstream ss;
    ss << "prob-" << setfill('0') << setw(3) << right << n;
    return ss.str();
}



bool correct_items( int cur_y, int cur_x, vector<pair<int,int>>& items_pos, Worker& robot ){
    if( items_pos.size() == 0 ){
        warning( "There is no the item" );
        return false;
    }
    if( items_pos.size()>10 ){
        warning( "To match items" );
        return false;
    }else{
        items_pos.push_back( make_pair( cur_y, cur_x ) );
        int items_num = items_pos.size()-1;

        vector<vector<int>> dist( items_num+1, vector<int>(items_num+1, 1e9) );
        for( int i=0; i<items_num+1; ++i ){
            for( int j=i; j<items_num+1; ++j ){
                if( i==j ){
                    dist[i][j] = 0;
                }else{
                    dist[i][j] = dist[j][i] = ikeda::calc_distance( robot.field, {items_pos[i].first, items_pos[i].second}, {items_pos[j].first, items_pos[j].second} );
                }
            }
        }

        for( int i=0; i<items_num+1; ++i ){
            for( int j=i; j<items_num+1; ++j ){
                assert( dist[i][j] != 1e9 );
            }
        }

        vector<vector<int>> dp( items_num, vector<int>(1<<items_num, 1e9) );
        for( int i=0; i<items_num; ++i ){
            dp[i][1<<i] = dist[items_num][i];
        }
        
        for( int visit=0; visit<items_num; visit++ ){
            for( int last=0; last<items_num; ++last ){
                for( int bit=0; bit<(1<<items_num); ++bit ){
                    if( (bit & (1<<visit)) )continue;
                    dp[visit][bit | (1<<visit)] = min( dp[visit][bit | (1<<visit)], dp[last][bit] + dist[last][visit] );
                }
            }
        }
        
        
        vector<pair<int,int>> order;
        int goal = -1;
        int val = 1e9;
        for(int i=0; i<items_num; ++i ){
            if( dp[i][(1<<items_num)-1]<val ){
                val = dp[i][(1<<items_num)-1];
                goal = i;
            }
        }

        assert( goal != -1 );
        order.push_back( items_pos[goal] );
        int bit_status = ( (1<<items_num)-1 ) - (1<<goal);

        while( order.size()<items_num ){
            assert( bit_status != 0 );
            int via = -1;
            val = 1e9;
            for( int i=0; i<items_num; ++i ){
                if( ( bit_status & (1<<i) ) == 0 )continue;
                if( dp[i][bit_status] < val ){
                    val = dp[i][bit_status];
                    via = i;
                }
            }
            assert( via != -1 );
            bit_status -= (1<<via);
            order.push_back( items_pos[via] );
        }
        assert( bit_status == 0 );
        
        order.insert( order.begin(), items_pos.back() );
        
        val = 1e9;
        pair<int,int> nearest_myst = make_pair(-1, -1);
        for( int i=0; i<robot.field.size(); ++i ){
            for( int j=0; j<robot.field[0].size(); ++ j){
                if( robot.field[i][j] == boardloader::Mysterious ){
                    int res = ikeda::calc_distance( robot.field, {order.back().first, order.back().second}, {i, j} );
                    if( res<val ){
                        val = res;
                        nearest_myst = make_pair( i, j );
                    }
                }
            }
        }
        
        order.push_back( nearest_myst );
        
        string move_string = "";
        for( int i=0; i<order.size()-1; ++i ){
            cout << order[i].first << " " << order[i].second << "---" << order[i+1].first << " " << order[i+1].second << endl;
            move_string += ikeda::move( robot.field, {order[i].first, order[i].second}, {order[i+1].first, order[i+1].second} );
        }
        
        for( auto elm: move_string ){
            assert( robot.doAction( {Action( string(1, elm) )} ) );
        }

        return true;
    }
}

int main(){
    int start_y, start_x;
    string dirname = "/Users/ashibata/GitHub/icfpc2019/problems/";
    
    for( int i=300; i<=300; ++i ){
        string filename = getFilename( i );
        
        vector<vector<Cell>> field = load_board( dirname + filename + ".desc", start_y, start_x );
        Worker robot( start_y, start_x, field );
        
        vector<pair<int,int>> items_pos;
        for( int i=0; i<robot.field.size(); ++i ){
            for( int j=0; j<robot.field[0].size(); ++ j){
                if( robot.field[i][j] == boardloader::Cloning ){
                    items_pos.push_back( make_pair(i, j) );
                }
            }
        }
        
        assert( correct_items( start_y, start_x, items_pos, robot ) );
        
        
        vector<vector<bool>> occupied( field.size(), vector<bool>(field[0].size(), false) );
        occupied[robot.y[0]][robot.x[0]] = true;
        dfs( robot, occupied );
        
        ofstream sol_fs( filename + ".sol" );
        robot.dump_actions( sol_fs );
        sol_fs.close();
    }
}
