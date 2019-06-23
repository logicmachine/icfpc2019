
/*
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

void multiDfs( Worker &robot, vector<vector<bool>>& occupied, long long  robot_bit ){
    assert( robot.robot_num < 60 );

    vector<int> members;
    for(int i=0; i<robot.robot_num; ++i ){
        if( robot_bit & (1<<i) ){
            members.push_back( i );
        }
    }
    assert( members.size() == __builtin_popcountll( robot_bit ) );
    assert( members.size() > 0 );
    int leader_robot_num = members[0];
    assert( leader_robot_num < robot.robot_num );

#ifndef NDEBUG
    for( auto rob_num: members ){
        assert( robot.y[leader_robot_num] == robot.y[rob_num] );
        assert( robot.x[leader_robot_num] == robot.x[rob_num] );
    }
#endif

    int dy[] = {0, -1, 0, 1};
    int dx[] = {1, 0, -1, 0};
    string direction[] = {"D", "S", "A", "W"};
    
    int candidate_num = 0;
    vector<int> cand_k;

    for( int k=0; k<4; ++k ){
        int ddy = robot.y[ leader_robot_num ] + dy[k];
        int ddx = robot.x[ leader_robot_num ] + dx[k];
        if( ddy<0 or ddy>=occupied.size() or ddx<0 or ddx>=occupied[0].size() )continue;
        if( occupied[ddy][ddx] == false and robot.field[ddy][ddx] != boardloader::Obstacle ){
            occupied[ddy][ddx] = true;
            candidate_num++;
            cand_k.push_back( k );
        }
    }
    assert( candidate_num == cand_k.size() );
    if( candidate_num == 0 ) return ;

    // Assign task to each robots
    int responsible_robot_num = members.size();    

    vector<int> robot_alloc( 4, 0 );
    assert( candidate_num != 0 );
    int robot_num_of_each_dir = responsible_robot_num / candidate_num;
    for( auto k: cand_k ){
        robot_alloc[k] = robot_num_of_each_dir;
    }

    responsible_robot_num -= robot_num_of_each_dir * candidate_num;
    assert( responsible_robot_num >= 0 );
    assert( responsible_robot_num < candidate_num );

    for( auto k: cand_k){
        if( responsible_robot_num<=0 )break;
        robot_alloc[k]++;
        responsible_robot_num--;
    }

    assert( responsible_robot_num == 0 );

#ifndef NDEBUG
    int alloc_sum = accumulate( robot_alloc.begin(), robot_alloc.end(), 0 );
    assert( alloc_sum == members.size() );
#endif

    // Deside responsible_robot_bit
    vector<int> responsible_robot_bit(4, 0);
    auto idle_robots = robot_bit;
    for( auto k: cand_k ){
        assert( k<4 );
        int ddy = robot.y[ leader_robot_num ] + dy[k];
        int ddx = robot.x[ leader_robot_num ] + dx[k];
        assert( not (ddy<0 or ddy>=occupied.size() or ddx<0 or ddx>=occupied[0].size()) );
        assert( robot.field[ddy][ddx] != boardloader::Obstacle );

        int new_bit = 0;

        for(int i=0; i<robot.robot_num; ++i ){
            if( (idle_robots & (1<<i)) && robot_alloc[k]>0 ){
                idle_robots ^= (1<<i);
                robot_alloc[k]--;
                new_bit |= (1<<i);
            }
        }
        responsible_robot_bit[k] = new_bit;
    }
    
    assert( idle_robots == 0 );

#ifndef NDEBUG
    for( auto elm: robot_alloc ){
        assert( elm == 0 );
    }

    int bit_sum = accumulate( responsible_robot_bit.begin(), responsible_robot_bit.end(), 0 );
    assert( bit_sum = robot_bit );
#endif

    assert( responsible_robot_bit[cand_k[0]] != 0 );

    // Assign adjustment
    if( candidate_num > members.size() ){
        for( int i=1; i<cand_k.size(); ++i ){
            if( responsible_robot_bit[cand_k[i]] == 0 ){
                assert( (i-1)>=0 );
                responsible_robot_bit[cand_k[i]] = responsible_robot_bit[ cand_k[(i-1)/2] ];
            }
        }
    }

    // Execute
    for( auto k: cand_k ){
        assert( k<4 );
        int ddy = robot.y[ leader_robot_num ] + dy[k];
        int ddx = robot.x[ leader_robot_num ] + dx[k];
        assert( not (ddy<0 or ddy>=occupied.size() or ddx<0 or ddx>=occupied[0].size()) );
        assert( robot.field[ddy][ddx] != boardloader::Obstacle );
        vector<Action> actions, rev_actions;
        for(int i=0; i<robot.robot_num; ++i ){
            if( responsible_robot_bit[k] & (1<<i) ){
                actions    .push_back( Action(direction[k]) );
                rev_actions.push_back( Action(direction[(k+2)%4]) );
            }else{
                actions    .push_back( Action() );
                rev_actions.push_back( Action() );
            }
        }

#ifndef NDEBUG
        assert( actions.size() == robot.robot_num );
        for( int i=0; i<robot.robot_num; ++i ){
            if( actions[i] != Action() ){
                assert( robot_bit & (1<<i) );
            }
            if( rev_actions[i] != Action() ){
                assert( robot_bit & (1<<i) );
            }
        }
#endif
        assert( robot.doAction( actions ) );
        assert( responsible_robot_bit[k] != 0 );
        multiDfs( robot, occupied, responsible_robot_bit[k] );

        assert( robot.doAction( rev_actions ) );
    }
}
*/