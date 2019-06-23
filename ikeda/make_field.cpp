#include "bits/stdc++.h"
#include "./ikeda.hpp"
using namespace std;

#define br(y,x) "(" + to_string(x) + "," + to_string(y) + ")"

struct cord{
    int y, x;
    cord( int yy, int xx): y(yy), x(xx) {}
    bool operator < (cord obj) const{
        return make_tuple(y, x) < make_tuple(obj.y, obj.x);
    }
};

void parse_input( const string& filename, int& bNum, int& eNum, int& tSize, int& vMin, int& vMax, int& mNum, int& fNum, int& dNum, int& rNum, int& cNum, int& xNum, set<cord>& iSqs, set<cord>& oSqs ){
    ifstream ifs( filename );
    if( not ifs.is_open() ){
        cerr << "File not found." << endl;
        exit(-1);
    }
    string bNum_str, eNum_str, tSize_str, vMin_str, vMax_str, mNum_str, fNum_str, dNum_str, rNum_str, cNum_str, xNum_str;
    
    getline(ifs, bNum_str , ',');
    getline(ifs, eNum_str , ',');
    getline(ifs, tSize_str, ',');
    getline(ifs, vMin_str , ',');
    getline(ifs, vMax_str , ',');
    getline(ifs, mNum_str , ',');
    getline(ifs, fNum_str , ',');
    getline(ifs, dNum_str , ',');
    getline(ifs, rNum_str , ',');
    getline(ifs, cNum_str , ',');
    getline(ifs, xNum_str , '#');
    
    bNum  = stoi( bNum_str );
    eNum  = stoi( eNum_str );
    tSize = stoi( tSize_str );
    vMin  = stoi( vMin_str );
    vMax  = stoi( vMax_str );
    mNum  = stoi( mNum_str );
    fNum  = stoi( fNum_str );
    dNum  = stoi( dNum_str );
    rNum  = stoi( rNum_str );
    cNum  = stoi( cNum_str );
    xNum  = stoi( xNum_str );
    
    string iSqs_line;
    getline(ifs, iSqs_line, '#' );
    istringstream iss(iSqs_line);
    
    string ix, iy;
    char skip;
    while( iss >> skip ){
        assert( skip == '(' );
        
        getline(iss, ix, ',');
        
        getline(iss, iy, ')');
        
        iss >> skip;
        assert( iss.eof() or skip == ',' );
        
        iSqs.insert( cord(stoi(iy), stoi(ix)) );
        
        if( iss.eof() )break;
    }
    
    string oSqs_line;
    getline(ifs, oSqs_line);
    istringstream oss(oSqs_line);
    
    string ox, oy;
    while( oss >> skip ){
        assert( skip == '(' );
        
        getline(oss, ox, ',');
        
        getline(oss, oy, ')');
        
        oss >> skip;
        assert( oss.eof() or skip == ',' );
        
        oSqs.insert( cord(stoi(oy), stoi(ox)) );
        
        if( oss.eof() )break;
    }
    ifs.close();
}

void dump_field( const vector<string>& field ){
    for( int i=0; i<field.size(); ++i ){
        cout << field[i] << endl;
    }
}


string conv_map( vector<pair<int,int>> vp ){
    string ret = "";
    for( int i=0; i<vp.size(); ++i ){
        if( i!=0 ){
            ret += ",";
        }
        ret += br(vp[i].first, vp[i].second);
    }
    return ret;
}


string toDesc( vector<string>& field ){
    string ret = "";
    vector<vector<pair<int,int>>> points = ikeda::get_tenretsu( field );

    ret += conv_map( points[0] );
    
    ret += '#';

    int H = field.size();
    int W = field[0].size();
    bool did_dump_start = false;
    for( int i=0; i<H; ++i ){
        for( int j=0; j<W; ++j ){
            if( field[i][j] == '.' ){
                ret += br(i, j);
                did_dump_start = true;
            }
            if( did_dump_start == true )break;
        }
        if( did_dump_start == true )break;
    }

    ret += '#';

    for( int i=1; i<points.size(); ++i ){
        if( i!=1 )ret += ";";
        ret += conv_map( points[i] );
    }

    ret += '#';

    bool boosters_dumped = false;
    for( int i=0; i<H; ++i ){
        for( int j=0; j<W; ++j ){
            if( field[i][j] == 'B' or field[i][j] == 'F' or field[i][j] == 'L' or field[i][j] == 'X' or field[i][j] == 'R' or field[i][j] == 'C' ){
                if( boosters_dumped == true ){
                    ret += ';';
                }
                ret += field[i][j];
                ret += br(i, j);
                boosters_dumped = true;
            }else{
                assert( field[i][j] == '#' or field[i][j] == '.' );
            }
        }
    }
    return ret;

}

int main(int argc, char* argv[]){
    assert( argc == 2 );
    string filename = argv[1];
    
    int bNum, eNum, tSize, vMin, vMax, mNum, fNum, dNum, rNum, cNum, xNum;
    set<cord> iSqs, oSqs;
    parse_input( filename, bNum, eNum, tSize, vMin, vMax, mNum, fNum, dNum, rNum, cNum, xNum, iSqs, oSqs );
    
    // 外周をObstacleで埋める
    int H = tSize+1, W = H;
    vector<string> field(H, string(W, '.'));
    for( int j=0; j<W; ++j ){
        field[0][j]   = '#';
        assert( H-1>=0 );
        field[H-1][j] = '#';
    }
    for( int i=0; i<H; ++i ){
        field[i][0]   = '#';
        assert( W-1>=0 );
        field[i][W-1] = '#';
    }
    
    // oSqsの座標をObstacleで埋める
    set<pair<int, int>> stliSqs;
    for (auto i : iSqs) {
        stliSqs.insert({i.y, i.x});
    }
    for( auto elm: oSqs){
        assert( elm.y<H );
        assert( elm.x<W );
        ikeda::connect(field, stliSqs, {elm.y, elm.x});
    }

        while(1){
        bool painted = false;
        for( int i=2; i<H-3; ++i ){
            for( int j=2; j<W-3; ++j ){
                if(
                   field[i][j]     == '.' and
                   field[i][j+1]   == '#' and
                   field[i+1][j]   == '#' and
                   field[i+1][j+1] == '.'
                   ){
                    if( iSqs.count( cord(i, j) )==0 ){
                        field[i][j] = '#';
                    }else if( iSqs.count( cord(i+1, j+1) ) ){
                        field[i+1][j+1] = '#';
                    }else{
                        cerr << "I cant do it..." << endl;
                        exit(1);
                    }
                    painted = true;
                }
            }
        }
        if( painted == false )break;
    }
    
    while(1){
        bool painted = false;
        for( int i=2; i<H-3; ++i ){
            for( int j=2; j<W-3; ++j ){
                if(
                   field[i][j]     == '#' and
                   field[i][j+1]   == '.' and
                   field[i+1][j]   == '.' and
                   field[i+1][j+1] == '#'
                   ){
                    if( iSqs.count( cord(i, j+1) )==0 ){
                        field[i][j+1] = '#';
                    }else if( iSqs.count( cord(i+1, j) ) ){
                        field[i+1][j] = '#';
                    }else{
                        cerr << "I cant do it..." << endl;
                        exit(1);
                    }
                    painted = true;
                }
            }
        }
        if( painted == false )break;
    }

    // vMinを超えるまで外周そばのマスをObstacleで埋める
    vector<vector<pair<int,int>>> res;
    int angle_num;
    do{
        bool painted = false;
        
        res  = ikeda::get_tenretsu( field );

        angle_num = res[0].size();
        
        int need_angle_num = (vMin - angle_num + 3)/4;
        for( int i=2; i<=H-3; i+=2 ){
            if( need_angle_num>0 and iSqs.count( cord(i, 1) ) == 0
               and field[i-1][1] != '#'
               and field[i  ][1] != '#'
               and field[i+1][1] != '#'
               and field[i-1][2] != '#'
               and field[i  ][2] != '#'
               and field[i+1][2] != '#'
               ){
                field[i][1] = '#';
                need_angle_num--;
                painted = true;
            }
            
            if( need_angle_num>0 and field[i][W-2] != '#' and iSqs.count( cord(i, W-2) ) == 0
               and field[i-1][W-2] != '#'
               and field[i  ][W-2] != '#'
               and field[i+1][W-2] != '#'
               and field[i-1][W-3] != '#'
               and field[i  ][W-3] != '#'
               and field[i+1][W-3] != '#'
               ){
                field[i][W-2] = '#';
                need_angle_num--;
                painted = true;
            }
            
            if( need_angle_num <= 0 )break;
        }
        for( int j=2; j<=W-3; j+=2 ){
            if( need_angle_num>0 and iSqs.count( cord(1, j) ) == 0
               and field[1][j-1] != '#'
               and field[1][j  ] != '#'
               and field[1][j+1] != '#'
               and field[2][j-1] != '#'
               and field[2][j  ] != '#'
               and field[2][j+1] != '#'
               ){
                field[1][j] = '#';
                need_angle_num--;
                painted = true;
            }
            
            if( need_angle_num>0 and iSqs.count( cord(H-2, j) ) == 0
               and field[H-2][j-1] != '#'
               and field[H-2][j  ] != '#'
               and field[H-2][j+1] != '#'
               and field[H-3][j-1] != '#'
               and field[H-3][j  ] != '#'
               and field[H-3][j+1] != '#'
               ){
                field[H-2][j] = '#';
                need_angle_num--;
                painted = true;
            }
            if( need_angle_num <= 0 )break;
        }
        if( need_angle_num <= 0 )break;
        else if( painted == false ){
            cerr << "I cant do it..." << endl;
            exit(1);
        }
        cerr << angle_num << endl;
    }while( angle_num < vMin );
    
    
    queue<char> que;
    for( int i=0; i<mNum; ++i ) que.push( 'B' );
    for( int i=0; i<fNum; ++i ) que.push( 'F' );
    for( int i=0; i<dNum; ++i ) que.push( 'L' );
    for( int i=0; i<rNum; ++i ) que.push( 'R' );
    for( int i=0; i<cNum; ++i ) que.push( 'C' );
    for( int i=0; i<xNum; ++i ) que.push( 'X' );
    for( int i=0; i<H; ++i ){
        for( int j=0; j<W; ++j ){
            if( que.size() <= 0 )break;
            if( field[i][j] == '.'){
                field[i][j] = que.front();
                que.pop();
            }
        }
        if( que.size() <= 0 )break;
    }
    assert( que.size()==0 );
    
    string desc_str = toDesc( field );
    cout << desc_str << endl;
    //dump_field( field );
}
