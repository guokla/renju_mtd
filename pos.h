#ifndef POS_H
#define POS_H

struct Pos
{
    int x, y;
    int value, a1, a2, a3;
    bool operator< (const Pos &a)const{
        return a.value > value;
    }
    bool operator> (const Pos &a)const{
        return a.value < value;
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

    Pos(int _x=20, int _y=20, int _value=0, int _a1=0, int _a2=0, int _a3=0)
        :x(_x), y(_y), value(_value), a1(_a1), a2(_a2), a3(_a3){}

};

#endif // POS_H
