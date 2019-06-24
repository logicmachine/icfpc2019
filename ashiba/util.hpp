
enum Item{
    B,
    F,  //Fast
    L,  //Drill
    R,
    C,
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
    int user_num;
    
    bool operator == (Action obj) const{
        if( opcode != obj.opcode )         return false;
        else if( operand != obj.operand )  return false;
        else if( user_num != obj.user_num )return false;
        else return true;
    }
    bool operator != (Action obj) const{
        return not (*this == obj);
    }

    Action(){ opcode = ""; operand = make_pair( -1, -1 ); user_num = -1; } 
    Action( string opc ): opcode(opc) { operand = make_pair( -1, -1 ); }
    Action( string opc, pair<int,int> oper ): opcode(opc), operand(oper) {}
    Action( string opc, int num ): opcode(opc), user_num(num) { operand = make_pair( -1, -1 ); }
    Action( string opc, pair<int,int> oper, int num ): opcode(opc), operand(oper), user_num(num) {}
    
};
