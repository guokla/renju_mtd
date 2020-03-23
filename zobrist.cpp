#include "zobrist.h"
#include "mainwindow.h"
#include "mythread.h"

bool MainWindow::lookup(int depth, int alpha, int beta, int& val, int flag)
{
    long long hashInfo = hash & (HASH_TABLE_SIZE - 1); // 获取局面在表中位置
    if((H[flag][hashInfo].depth >= depth) && H[flag][hashInfo].checknum == hash)
    {
        tag++;

        if(H[flag][hashInfo].flag == HASH_EXACT){
            val = H[flag][hashInfo].val;
        }else if(H[flag][hashInfo].flag == HASH_ALPHA && H[flag][hashInfo].val >= alpha){
            val = H[flag][hashInfo].val;
        }else if(H[flag][hashInfo].flag == HASH_BETA  && H[flag][hashInfo].val >= beta){
            val = H[flag][hashInfo].val;
        }else{
            return false;
        }
        return true;
    }
    return false;
}

bool MainWindow::store(QMutex &m, int depth, int val, int hashf, long long hashIndex, int flag)
{
    long long hashInfo;

    m.lock();
    hashInfo = hashIndex & (HASH_TABLE_SIZE - 1);

    if(H[flag][hashInfo].depth < depth || H[flag][hashInfo].flag > 2 || H[flag][hashInfo].flag < 0){
        sto++;
        H[flag][hashInfo].checknum = hash;
        H[flag][hashInfo].flag = hashf;
        H[flag][hashInfo].depth = depth;
        H[flag][hashInfo].val   = val;
    }
    m.unlock();
    return true;
}

bool MyThread::lookup(int depth, int alpha, int beta, int& val)
{
    long long hashInfo = hash & (HASH_TABLE_SIZE - 1); // 获取局面在表中位置
    if((H[hashInfo].depth >= depth) && H[hashInfo].checknum == hash)
    {
        tag++;

        if(H[hashInfo].flag == HASH_EXACT){
            val = H[hashInfo].val;
        }else if(H[hashInfo].flag == HASH_ALPHA && H[hashInfo].val >= alpha){
            val = H[hashInfo].val;
        }else if(H[hashInfo].flag == HASH_BETA  && H[hashInfo].val >= beta){
            val = H[hashInfo].val;
        }else
            return false;
        return true;
    }
    return false;
}

bool MyThread::store(QMutex &m, int depth, int val, int hashf, long long hashIndex)
{
    long long hashInfo;

    m.lock();
    hashInfo = hashIndex & (HASH_TABLE_SIZE - 1);

    if(H[hashInfo].depth < depth || H[hashInfo].flag > 2 || H[hashInfo].flag < 0){
        sto++;
        H[hashInfo].checknum = hash;
        H[hashInfo].flag = hashf;
        H[hashInfo].depth = depth;
        H[hashInfo].val   = val;
    }
    m.unlock();
    return true;
}


