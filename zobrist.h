#ifndef ZOBRIST_H
#define ZOBRIST_H

#include <QVector>
#include <string>
#include "pos.h"

struct HASHITEM{
    long long checknum;
    int depth;
    int flag;
    int val;
    HASHITEM(): checknum(0), depth(-1), flag(-1), val(0) {}
    HASHITEM(long long c, int d, int f, int v): checknum(c), depth(d), flag(f), val(v) {}
};

#endif // ZOBRIST_H
