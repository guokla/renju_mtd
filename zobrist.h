#ifndef ZOBRIST_H
#define ZOBRIST_H

#include <QVector>
#include <string>
#include "pos.h"

struct HASHITEM{
    long long checknum;
    int flag;
    Pos ref;
    HASHITEM(): checknum(0),flag(-1){}
    HASHITEM(long long c, int d, int f, int v): checknum(c), flag(f){}
};

#endif // ZOBRIST_H
