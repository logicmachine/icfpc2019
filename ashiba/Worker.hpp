#include "util.hpp"

using boardloader::Cell;

const int DEFAULT_MANIP_NUM = 3;
const int manip_y[] = {1, 0, -1};
const int manip_x[] = {1, 1, 1};


class Worker{
    int H, W;
    vector<vector<pair<int,int>>> manipulators_;
    vector<Direction> dir_;
    vector<int> speed_;
    
    map<Item,int> items_;    
    
public:
    int robot_num;
    vector<int> y, x;
    vector<vector<Action>> action_log;
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
                        //cout << "field[" << i << "][" << j << "] = " << field[i][j] << " " << "intersect : " << src_y+0.5 << "_" << src_x+0.5 << " " << dst_y+0.5 << "_" << dst_x+0.5 << " " << i+dy[k] << "_" << j+dx[k] << " " << i+dy[k+1] << "_" << j+dx[k+1] << endl;
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
        
        for( int rb=0; rb<act_list.size(); ++rb ){
            Action act = act_list[rb];
            //cerr << "act : " << act.opcode << endl;
            if( act == Action() )continue;
            
            if( act.opcode == "W" ){ // move_up
                if( y[rb]+1 >= H ){
                    cerr << "range_error" << endl;
                    return false;
                }else if( field[y[rb]+1][x[rb]]==boardloader::Obstacle ){
                    cerr << "Go on Obstacle" << endl;
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
                        if( field[moved_y][moved_x] == boardloader::Teleport             ) items_[ R ]++;
                        if( field[moved_y][moved_x] == boardloader::Cloning              ) items_[ C ]++;
                        
                        applyPaint( moved_y, moved_x, manipulators_[rb] );
                        
                        y_new = y[rb]+d;
                    }
                    assert( y_new < H );
                    y[rb] = y_new;
                    
                }
                
            }else if( act.opcode == "S" ){ // move_down
                if( y[rb]-1 < 0){
                    cerr << "range_error" << endl;
                    return false;
                }else if( field[y[rb]-1][x[rb]] == boardloader::Obstacle ){
                    cerr << "Go on Obstacle" << endl;
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
                        if( field[moved_y][moved_x] == boardloader::Teleport             ) items_[ R ]++;
                        if( field[moved_y][moved_x] == boardloader::Cloning              ) items_[ C ]++;

                        
                        applyPaint( moved_y, moved_x, manipulators_[rb] );
                        
                        y_new = y[rb]-d;
                    }
                    assert( y_new>=0 );
                    y[rb] = y_new;
                    
                }
                
            }else if( act.opcode == "A" ){ // move_left
                if( x[rb]-1<0){
                    cerr << "range_error" << endl;
                    return false;
                }else if( field[y[rb]][x[rb]-1] == boardloader::Obstacle ){
                    cerr << "Go on Obstacle" << endl;
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
                        if( field[moved_y][moved_x] == boardloader::Teleport             ) items_[ R ]++;
                        if( field[moved_y][moved_x] == boardloader::Cloning              ) items_[ C ]++;

                        
                        applyPaint( moved_y, moved_x, manipulators_[rb] );
                        
                        x_new = x[rb]-d;
                    }
                    assert( x_new >= 0 );
                    x[rb] = x_new;
                    
                }
                
            }else if( act.opcode == "D" ){ // move_right
                if( x[rb]+1>=W ){
                    cerr << "range_error" << endl;
                    return false;
                }else if( field[y[rb]][x[rb]+1] == boardloader::Obstacle ){
                    cerr << "Go on Obstacle" << endl;
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
                        if( field[moved_y][moved_x] == boardloader::Teleport             ) items_[ R ]++;
                        if( field[moved_y][moved_x] == boardloader::Cloning              ) items_[ C ]++;

                        
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
                if( act.user_num<0 or act.user_num>=robot_num ){
                    warning( "Invalid user_name" + to_string(act.user_num) );
                    return false;
                }
                if( not hasItem( C ) ){
                    return false;
                }
                robot_num++;
                assert( items_[C]>0 );
                items_[C]--;

                action_log   .push_back( vector<Action>() );
                manipulators_.push_back( manipulators_[ act.user_num ] );
                dir_         .push_back( dir_[act.user_num] );
                speed_       .push_back( speed_[act.user_num] );
                y            .push_back( y[act.user_num] ); 
                x            .push_back( x[act.user_num] );
                
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