#include "bits/stdc++.h"
#include "unistd.h"

#include "../sayama/board_loader.hpp"
#include "../ikeda/ikeda.hpp"
#include "../futatsugi/ccl.hpp"

#include "Worker.hpp"
#include "dfs.hpp"
#include "getLabeledPoints.hpp"

using namespace std;
using namespace boardloader;


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
    if( items_pos.size()>25 ){
        warning( "To much items" + to_string(items_pos.size()) );
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
        
        string move_string = "";
        for( int i=0; i<order.size()-1; ++i ){
            move_string += ikeda::move( robot.field, {order[i].first, order[i].second}, {order[i+1].first, order[i+1].second} );
        }
        
        for( auto elm: move_string ){
            assert( robot.doAction( {Action( string(1, elm) )} ) );
        }
        return true;
    }
}

bool moveToTargetCell( Worker& robot, const int& y, const int& x, const Cell& type ){
    int val = 1e9;
    pair<int,int> nearest_myst = make_pair(-1, -1);
    
    int H = robot.field.size();
    int W = robot.field[0].size();
    for( int i=0; i<H; ++i ){
        for( int j=0; j<W; ++ j){
            if( robot.field[i][j] == type ){
                int res = ikeda::calc_distance( robot.field, {y, x}, {i, j} );
                assert( res != 1010001000 );
                if( res<val ){
                    val = res;
                    nearest_myst = make_pair( i, j );
                }
            }
        }
    }
    if( val == 1e9 )return false;

    string move_string  = ikeda::move( robot.field, {y, x}, {nearest_myst.first, nearest_myst.second} );

    for( auto elm: move_string ){
        robot.doAction( {Action( string(1, elm) )} );
    }
    return true;
}

string moveToNearestFullfillPos( Worker& robot, const int& cur_y, const int& cur_x ){
    int H = robot.field.size();
    int W = robot.field[0].size();

    queue<pair<int,int>> que;
    que.push( make_pair( cur_y, cur_x) );
    int dy[] = {0, -1, 0, 1};
    int dx[] = {1, 0, -1, 0};

    vector<vector<bool>> dist(H, vector<bool>(W, false));
    dist[cur_y][cur_x] = true;

    pair<int,int> target_pos = make_pair( -1, -1 );
    while( que.size() ){
        int y, x;
        tie(y,x) = que.front();

        for( int k=0; k<4; ++k ){
            int ddy = y+dy[k];
            int ddx = x+dx[k];
            if( ddy<0 or ddy>=H or ddx<0 or ddx>=W )continue;
            if( robot.field[ddy][ddx] == Obstacle )continue;
            if( dist[ddy][ddx] == true )continue;
            que.push( make_pair( ddy, ddx ) );
            dist[ddy][ddx] = true;
            if( robot.field[ddy][ddx] != Obstacle and robot.field[ddy][ddx] != Occupied ){
                target_pos = make_pair( ddy, ddx );
            }
        }
        if( target_pos != pair<int,int>(-1, -1) )break;
        que.pop();
    }

    if( target_pos == make_pair(-1, -1) )return "";

    string move_string  = ikeda::move( robot.field, {cur_y, cur_x}, { target_pos.first, target_pos.second } );

    return move_string;
}

bool useAllClone( Worker& robot ){
    if( not robot.hasItem( C ) ){
        return false;
    }

    while( robot.hasItem( C ) ){
        vector<Action> actions(robot.robot_num, Action("Z"));
        actions[0] = Action("C", 0);
        assert( robot.doAction( actions ) );
    }
    return true;
}

/*
bool useAllManipEx( Worker& robot ){
    int items_num = robot.countItem( B );
    int each_num = items_num/robot.robot_num;
    items_num -= each_num*robot.robot_num;

    vector<int> alloced_items(robot.robot_num, each_num);

    assert( items_num>=0 );
    assert( items_num<robot.robot_num );
    for( int i=0; i<robot.robot_num; ++i ){
        if( items_num <=0 )break;
        alloced_items[i]++;
        items_num--;
    }

    for( int i=0; i<alloced_items[0]; ++i ){
        vector<Action> actions;
        for( int j=0; j<robot.robot_num; ++j ){
            if( alloced_items[j]>0 ){
                alloced_items[j]--;
                set<pair<int,int>> manip
                for( auto elm: manipulators_[j] ){
                    manip.insert( elm );
                }
                pair<int,int> pos = make_pair( robot.y[j], robot.x[j] );


                actions.push_back( Action( "B", pos, j ) );
            }else{
                actions.push_back( Action( "Z" ) );
            }                
        }
        robot.doAction( actions );
    }
    return true;
}
*/
/*
bool useAllClone( Worker& robot ){
    if( not robot.hasItem( C ) ){
        return false;
    }

    while( robot.hasItem( C ) ){
        
        int items_num = robot.countItem( C );

        vector<Action> clone_actions;
        int idle_robot_num = robot.robot_num;

        for(int i=0; i<idle_robot_num; ++i ){
            if( items_num>0 ){
                clone_actions.push_back( Action( "C", i ) );
                items_num--;
            }else{
                break;
            }
        }
        assert( robot.doAction( clone_actions ) );
    }
    return true;
}

*/
/*
bool useAllClone( Worker& robot ){
    if( not robot.hasItem( C ) ){
        return false;
    }

    while( robot.hasItem( C ) ){
        int items_num = robot.countItem( C );

        vector<Action> clone_actions;
        int idle_robot_num = robot.robot_num;

        for(int i=0; i<idle_robot_num; ++i ){
            if( items_num>0 ){
                clone_actions.push_back( Action( "C", i ) );
                items_num--;
            }else{
                break;
            }
        }
        assert( robot.doAction( clone_actions ) );
    }
    return true;
}
*/

bool k_means( vector<vector<Cell>>& field, const int& num, vector<vector<Point>>& clusters, vector<Point>& leader_points ){
    //onst int REPEAT_TIMES = 3;
    const int REPEAT_TIMES = 50;

    int H = field.size();
    int W = field[0].size();
    vector<Point> candidates;
    for( int i=0; i<H; ++i ){
        for( int j=0; j<W; ++j ){
            if( field[i][j] != Obstacle and field[i][j] != Occupied ){
                candidates.push_back( Point(i, j) );
            }
        }
    }
    assert( num > 0 );

    leader_points.resize(num);
    clusters.resize(num);
    int width = candidates.size()/num;
    for(int i=0; i<num; ++i ){
        assert( width*i < candidates.size() );
        leader_points[i] = candidates[width*i];
    }

    for( int i=0; i<REPEAT_TIMES; ++i ){
        // for( auto elm: leader_points){
        //     cerr << elm.y << " " << elm.x << "__";
        // }
        // cerr << endl;

        map<pair<Point,Point>,int> dist;
        for( int j=0; j<leader_points.size(); ++j ){
            Point from = {leader_points[j].y, leader_points[j].x};
            int dxy[5] = {0, 1, 0, -1, 0};
            vector<vector<int>> cost(field.size(), vector<int>(field[0].size(), 1010001000));
            cost[from.y][from.x] = 0;
            deque<pair<int, boardloader::Point>> que;
            que.push_back({0, from});
            while (!que.empty()) {
                auto pos = que.front().second;
                auto cs = que.front().first;
                que.pop_front();
                if (cs != cost[pos.y][pos.x]) continue;
                for (int i = 0; i < 4; i++) {
                    int nx = pos.y + dxy[i], ny = pos.x + dxy[i+1];
                    if (0 <= nx && nx < field.size() &&
                            0 <= ny && ny < field[nx].size() &&
                            field[nx][ny] != boardloader::Cell::Obstacle && 
                            cost[nx][ny] > cs+1) {
                        cost[nx][ny] = cs+1;
                        que.push_back({cs+1, boardloader::Point(nx, ny)});
                    }
                }
            }
            for( int j=0; j<H; ++j ){
                for( int k=0; k<W; ++k ){
                    dist[ pair<Point,Point>({j,k}, {from.y, from.x}) ] = cost[j][k];
                    //cerr << j << "-" << k << "-" << from.y << "-" << from.x << endl;
                }
            }
        }

        for( auto p: candidates ){
            int diff = 1e9;
            int cluster_num = -1;
            for( int j=0; j<leader_points.size(); ++j ){
//                    cerr << "***" << p.y << "-" << p.x << "-" << leader_points[j].y << "-" << leader_points[j].x << endl;
//                    if( p==leader_points[j] )dist[make_pair( p, leader_points[j] )] = 0;
//                   if( dist.count( make_pair( p, leader_points[j] ) ) != 0 ){cerr << j << endl;cerr << "***" << p.y << "-" << p.x << "-" << leader_points[j].y << "-" << leader_points[j].x << endl;}
                assert( dist.count( make_pair( p, leader_points[j] ) ) != 0 );

                int tmpdiff = dist[ make_pair( p, leader_points[j] ) ];
                if( tmpdiff<diff ){
                    diff = tmpdiff;
                    cluster_num = j;
                }
            }
            assert( cluster_num != -1 );
            assert( cluster_num < clusters.size() );
            clusters[cluster_num].push_back( p );
        }

        for( int j=0; j<num; ++j ){
            sort( clusters[j].begin(), clusters[j].end(), [](Point& a, Point& b){ return make_tuple(a.y, a.x) < make_tuple(b.y, b.x); } );
            int center_idx = clusters[j].size()/2;
            leader_points[j] = clusters[j][ center_idx ];
        }
    }
    return true;

}
    

bool dissolutionRobots( Worker& robot, const vector<Point>& leader_points ){
    vector<string> move_str(robot.robot_num);
    for( int i=0; i<robot.robot_num; ++i ){
        move_str[i] = ikeda::move( robot.field, {robot.y[i], robot.x[i]}, leader_points[i] );
    }

    int max_move = -1;
    for( auto elm: move_str ){
        max_move = max<int>( max_move, elm.size() );
    }

    for( int i=0; i<max_move; ++i ){
        vector<Action> actions;
        for(int j=0; j<robot.robot_num; ++j ){
            if( i<move_str[j].size() ){
                actions.push_back( Action(string(1, move_str[j][i])) );
            }else{
                actions.push_back( Action("Z") );
            }
        }
        robot.doAction( actions );
    }
    return true;
}


bool autoMultiMove( Worker& robot ){
    int H = robot.field.size();
    int W = robot.field[0].size();
    int dy[] = {0, -1, 0, 1};
    int dx[] = {1, 0, -1, 0};
    string direction[] = {"D", "S", "A", "W"};

    vector<bool> next_empty_flg(robot.robot_num,false);
    vector<bool> hlt_flg(robot.robot_num,false);
    int hlt_num = 0;
    vector<queue<char>> move_str_queue(robot.robot_num);

    while(1){
        if( hlt_num == robot.robot_num )break;

        set<Point> locked;
        vector<Action> actions;
        for( int rb=0; rb<robot.robot_num; rb++ ){
            if( hlt_flg[rb] == true ){
                actions.push_back( Action() );
                continue;
            }

            int y = robot.y[rb];
            int x = robot.x[rb];
            bool will_move = false;

            if( next_empty_flg[rb] == false ){
                for( int k=0; k<4; k++ ){
                    int ddy = y+dy[k];
                    int ddx = x+dx[k];
                    if( ddy<0 or ddy>=H or ddx<0 or ddx>=W )continue;
                    if( robot.field[ddy][ddx] == Obstacle )continue;
                    if( robot.field[ddy][ddx] == Occupied )continue;
                    if( locked.count( Point(ddy, ddx) ) != 0 )continue;

                    actions.push_back( Action( direction[k] ) );
                    assert( actions.size() == rb+1 );
                    locked.insert( Point(ddy, ddx) );
                    will_move = true;
                    break;
                }
            }
            if( will_move == false ){
                if( next_empty_flg[rb] == false ){
                    next_empty_flg[rb] = true;

                    string move_str = moveToNearestFullfillPos( robot, robot.y[rb], robot.x[rb] );
                    if( move_str == "" ){
                        hlt_flg[rb] = true;
                        hlt_num++;
                        actions.push_back( Action() );
                        continue;
                    }
                    for( auto ch: move_str ){
                        move_str_queue[rb].push( ch );
                    }
                }
                assert( move_str_queue[rb].size() >0 );
                actions.push_back( Action( string(1,move_str_queue[rb].front()) ) );
                move_str_queue[rb].pop();

                if( move_str_queue[rb].size() == 0 ){
                    next_empty_flg[rb] = false;
                }
            }
        }

        // cerr << actions.size() << " " << robot.robot_num << " " << hlt_num << endl;
        // if( actions.size() == 3 ){
        //     for( auto act: actions){
        //         cerr << act.opcode << " ";
        //     }
        //     cerr << endl;
        // }
        assert( actions.size() == robot.robot_num );
        robot.doAction( actions );
    }
    return true;
}


string loadItemData( string filename ){
    ifstream ifs( filename );
    string ret;
    ifs >> ret;
    return ret;
}

int main(int argc, char* argv[]){
    string l = "250";
    string r = "251";
    if( argc == 3 ){
        l = argv[1];
        r = argv[2];
    }

    int start_y, start_x;
    string dirname = "../problems/";
    
    for( int i=stoi(l); i<stoi(r); ++i ){
        string filename = getFilename( i );

        
        vector<vector<Cell>> field = load_board( dirname + filename + ".desc", start_y, start_x );
        Worker robot( start_y, start_x, field );

        string items_data = loadItemData( dirname + filename + ".buy" );
        for( auto ch: items_data ){
            if( ch == 'B' )robot.items_[B]++;
            if( ch == 'F' )robot.items_[F]++;
            if( ch == 'L' )robot.items_[L]++;
            if( ch == 'R' )robot.items_[R]++;
            if( ch == 'C' )robot.items_[C]++;
        }

        vector<vector<boardloader::Point>> labeled_points;
        getLabeledPoints( robot.field, labeled_points );
        
        // Find target Cell position
        set<Cell> target_cell;
        target_cell.insert( Cloning );
        //target_cell.insert( ManipulatorExtension );

        vector<pair<int,int>> items_pos;
        for( int i=0; i<robot.field.size(); ++i ){
            for( int j=0; j<robot.field[0].size(); ++ j){
                if( target_cell.count( robot.field[i][j] ) != 0 ){
                    items_pos.push_back( make_pair(i, j) );
                }
            }
        }
        
        if( not correct_items( start_y, start_x, items_pos, robot ) ){
            cerr << "There is no items" << endl;
        }
        cerr << "End correcting items" << endl;
        if( not moveToTargetCell( robot, robot.y[0], robot.x[0], Mysterious ) ){
            cerr << "There is no Myst cell" << endl;
        }
        cerr << "End moving to target cell" << endl;
        if( not useAllClone( robot ) ){
            cerr << "Has no Clone" << endl;
        }
        cerr << "End using clone " << endl;
        //assert( useAllManipEx( robot ) );
        cerr << "End using B" << endl;

        vector<vector<Point>> clusters;
        vector<Point> leader_points;

        assert( k_means( robot.field, robot.robot_num, clusters, leader_points ) );

        assert( dissolutionRobots( robot, leader_points ) );

        assert( autoMultiMove( robot ) );


        // auto f = [](const int& y, const int& x, const vector<vector<Cell>>& field){
        //     for( int i=0; i<8; ++i ){
        //         //周りを見て壁的なところか判定
        //     }

        //     return /* true or false */;
        // }
        // for( int i=0; i<robot.robot_num; ++i ){
        //     string move_str = moveToNearestFullfillPos( robot.field, robot.y[i], robot.x[i], f );
        // }
        

        /*
        vector<vector<bool>> occupied( field.size(), vector<bool>(field[0].size(), false) );
        occupied[robot.y[0]][robot.x[0]] = true;
        //dfs( robot, occupied );
        multiDfs( robot, occupied, (1<<robot.robot_num)-1 );
        */

        ofstream sol_fs( filename + ".sol" );
        robot.dump_actions( sol_fs );
        sol_fs.close();


        //print_table( robot.field, robot.y[0], robot.x[0] );
    }
}
