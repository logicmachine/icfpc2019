#include "bits/stdc++.h"
#include "unistd.h"

#include "../sayama/board_loader.hpp"
#include "../ikeda/ikeda.hpp"
#include "../futatsugi/ccl.hpp"

#include "Worker.hpp"
#include "dfs.hpp"
#include "getLabeledPoints.hpp"

using namespace std;
using boardloader::Cell;
using boardloader::load_board;
using boardloader::print_table;


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
    if( items_pos.size()>15 ){
        warning( "To match items" + to_string(items_pos.size()) );
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


int main(){
    int start_y, start_x;
    string dirname = "/Users/ashibata/GitHub/icfpc2019/problems/";
    
    for( int i=1; i<=1; ++i ){
        string filename = getFilename( i );
        
        vector<vector<Cell>> field = load_board( dirname + filename + ".desc", start_y, start_x );
        Worker robot( start_y, start_x, field );
        
        vector<vector<boardloader::Point>> labeled_points;
        getLabeledPoints( robot.field, labeled_points );
        for( auto vec: labeled_points ){
            for( auto elm: vec ){
                cerr << elm << " ";
            }
            cerr << endl;
        }
        
        // Find target Cell position
        Cell target_cell = boardloader::Cloning;
        vector<pair<int,int>> items_pos;
        for( int i=0; i<robot.field.size(); ++i ){
            for( int j=0; j<robot.field[0].size(); ++ j){
                if( robot.field[i][j] == target_cell ){
                    items_pos.push_back( make_pair(i, j) );
                }
            }
        }
        
        assert( correct_items( start_y, start_x, items_pos, robot ) );
        cerr << "End correcting items" << endl;
        assert( moveToTargetCell( robot, robot.y[0], robot.x[0], boardloader::Mysterious ) );
        cerr << "End moving to target cell" << endl;
        assert( useAllClone( robot ) );
        cerr << "End using" << endl;

        /*
        vector<vector<bool>> occupied( field.size(), vector<bool>(field[0].size(), false) );
        occupied[robot.y[0]][robot.x[0]] = true;
        //dfs( robot, occupied );
        multiDfs( robot, occupied, (1<<robot.robot_num)-1 );
        */

        ofstream sol_fs( filename + ".sol" );
        robot.dump_actions( sol_fs );
        sol_fs.close();
    }
}
