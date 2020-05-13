#ifndef ZOBRIST_H
#define ZOBRIST_H

#include <QVector>
#include <string>
#include "pos.h"

struct HASHITEM{
    long long checknum;
    int flag;
    Pos ref;
    HASHITEM(): checknum(0),flag(0){}
};

#endif // ZOBRIST_H
