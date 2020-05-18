#ifndef POS_H
#define POS_H

struct Pos
{
    int x, y;   // 着法坐标
    int value, a1, a2, a3;  // 估值，先手权值，次序，保留
    bool operator< (const Pos &a)const{
        return value < a.value;
    }
    bool operator> (const Pos &a)const{
        return value > a.value;
    }
    bool operator==(const Pos &a) const{
        if(a.x == x && a.y == y)
            return true;
        return false;
    }
    void clear(){
        x=y=20;
        value=a1=a2=a3=0;
    }

    Pos(int _x=-1, int _y=-1, int _value=0, int _a1=0, int _a2=0, int _a3=0)
        :x(_x), y(_y), value(_value), a1(_a1), a2(_a2), a3(_a3){}

};

#endif // POS_H
