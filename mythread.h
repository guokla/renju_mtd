#ifndef MYTHREAD_H
#define MYTHREAD_H

#include <QObject>
#include "pos.h"
#include <mainwindow.h>
#include "zobrist.h"

class MyThread : public QObject
{
    Q_OBJECT

public:
    explicit MyThread(QObject *parent = nullptr);
    ~MyThread();

signals:
    void resultReady(const QString &str);

public slots:
    void dowork(const QString &str);

private:
    volatile bool isStop;
    HASHITEM *H = nullptr;          // 哈希表
    QMutex mutex;
    uint64_t Z[20][20][3];     // 评分置换表
    uint64_t hash = 0;             // 哈希值
    int chess[20][20];              // 棋盘数组
    int vis[3][20][20];             // 棋子能量分布
    int Kernel = 2;                 // 能量分布
    int hold = 1;
    int order = 0;
    int limit = 20000;
    int rangenum = 45;
    bool topFlag = true;
    bool runing = true;
    bool openlog = false;
    QTime t2;
    int depth;
    int algoFlag = 0;

    const int vx[8] = { 0, 1, 1, 1, 0,-1,-1,-1};
    const int vy[8] = {-1,-1, 0, 1, 1, 1, 0,-1};
    const int add[15][15] = {
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 0, 0},
        {0, 0, 2, 3, 4, 4, 4, 4, 4, 4, 4, 3, 2, 0, 0},
        {0, 0, 2, 4, 5, 5, 5, 5, 5, 5, 5, 4, 2, 0, 0},
        {0, 0, 2, 4, 5, 5, 5, 5, 5, 5, 5, 4, 2, 0, 0},
        {0, 0, 2, 4, 5, 5, 6, 6, 6, 5, 5, 4, 2, 0, 0},
        {0, 0, 2, 4, 5, 5, 6, 7, 6, 5, 5, 4, 2, 0, 0},
        {0, 0, 2, 4, 5, 5, 6, 6, 6, 5, 5, 4, 2, 0, 0},
        {0, 0, 2, 4, 5, 5, 5, 5, 5, 5, 5, 4, 2, 0, 0},
        {0, 0, 2, 4, 5, 5, 5, 5, 5, 5, 5, 4, 2, 0, 0},
        {0, 0, 2, 3, 4, 4, 4, 4, 4, 4, 4, 3, 2, 0, 0},
        {0, 0, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    };

public:

    void setFlag(bool flag = true);
    void initial(HASHITEM *_H, uint64_t _Z[20][20][3], uint64_t _hash, int _chess[20][20],
                    int _vis[3][20][20], int key, int _limit, int _depth, int _algoFlag,
                    bool _openlog, int _order);

    // 局面计算函数
    int valueChess(int x, int y, int key, int *piority);
    int evaluate(int key);
    void getPosition(Pos& ret, int key);
    void powerOperation(int x, int y, int flag, int key);
    void cutTreeNode(QVector<Pos>& queue_move, QVector<Pos>& vec_moves, QVector<Pos>& path, int key);

    // PVS搜索
    void update(QMutex& m, Pos& ret, const Pos ref);
    int deepSearch(Pos& ret, int origin, int key, int deep, int rec, int alpha, int beta, QVector<Pos>& path);
    int checkSearch(Pos& ret, int origin, int key, int deep, int rec, int alpha, int beta, QVector<Pos>& path);
    int PVS(Pos& ret, int key, int deep, int alpha, int beta, QVector<Pos>& path);


    // MTDF搜索
    int deepening(int origin, int& x, int& y);
    void MTD(Pos& bestmove, int origin, int f, int deep);
    int MT(Pos& ret, int key, int deep, int alpha, int beta, QVector<Pos>& path);
    int alphabeta(Pos& ret, int key, int deep, int alpha, int beta, QVector<Pos>& path);

    // 越界检查函数
    bool inline inside(int x, int y);
    bool inline inside(Pos move);

    // 哈希表操作
    bool lookup(int depth, int alpha, int beta, Pos& ret);
    bool store(QMutex &m, int hashf, long long hashIndex, const Pos ret, int deep);
};

#endif // MYTHREAD_H
