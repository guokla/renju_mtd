#include "zobrist.h"
#include "mainwindow.h"
#include "mythread.h"

bool MyThread::lookup(int depth, int alpha, int beta, Pos& ret)
{
    long long hashInfo = hash & (HASH_TABLE_SIZE - 1); // 获取局面在表中位置
    if((H[hashInfo].ref.a2 >= depth) && H[hashInfo].checknum == hash)
    {
        tag++;

        if(H[hashInfo].flag == HASH_EXACT){
            ret = H[hashInfo].ref;
        }else if(H[hashInfo].flag == HASH_ALPHA && H[hashInfo].ref.value >= alpha){
            ret = H[hashInfo].ref;
        }else if(H[hashInfo].flag == HASH_BETA  && H[hashInfo].ref.value >= beta){
            ret = H[hashInfo].ref;
        }else
            return false;
        return true;
    }
    return false;
}

bool MyThread::store(QMutex &m, int hashf, long long hashIndex, const Pos ret)
{
    long long hashInfo;

    m.lock();
    hashInfo = hashIndex & (HASH_TABLE_SIZE - 1);

    if(H[hashInfo].ref.a2 < ret.a2 || H[hashInfo].flag > 2 || H[hashInfo].flag < 0){
        sto++;
        H[hashInfo].checknum = hash;
        H[hashInfo].flag = hashf;
        H[hashInfo].ref   = ret;
    }
    m.unlock();
    return true;
}


