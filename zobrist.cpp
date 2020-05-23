#include "zobrist.h"
#include "mainwindow.h"
#include "mythread.h"

extern int count, ABcut, tag, sto;

bool MyThread::lookup(int depth, int alpha, int beta, Pos& ret)
{
    uint64_t hashInfo = hash & (HASH_TABLE_SIZE - 1); // 获取局面在表中位置
    if((H[hashInfo].ref.a2 >= depth) && H[hashInfo].checknum == hash)
    {
        if(H[hashInfo].flag == HASH_EXACT){
            ret = H[hashInfo].ref;
        }else if(H[hashInfo].flag == HASH_ALPHA && H[hashInfo].ref.value <= alpha){
            ret = H[hashInfo].ref;
            ret.value = alpha;
        }else if(H[hashInfo].flag == HASH_BETA  && H[hashInfo].ref.value >=  beta){
            ret = H[hashInfo].ref;
            ret.value = beta;
        }else
            return false;
        tag++;
        return true;
    }
    return false;
}

bool MyThread::store(QMutex &m, int hashf, long long hashIndex, const Pos ret, int deep)
{
    uint64_t hashInfo;

    QMutexLocker locker(&m);
    hashInfo = hashIndex & (HASH_TABLE_SIZE - 1);

    if(H[hashInfo].flag > HASH_BETA || H[hashInfo].flag < HASH_EXACT){
        sto++;
        H[hashInfo].checknum = hash;
        H[hashInfo].flag     = hashf;
        H[hashInfo].ref      = ret;
        H[hashInfo].ref.a2   = deep;
    }
    return true;
}


