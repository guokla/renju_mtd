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

signals:
    void resultReady(const QString &str);

public slots:
    void dowork();

private:
    volatile bool isStop;
    HASHITEM *H;                    // 哈希表
    QMutex mutex;
    unsigned long Z[20][20][3];     // 评分置换表
    long long hash = 0;             // 哈希值
    int chess[20][20];              // 棋盘数组
    int vis[3][20][20];             // 棋子能量分布
    char strTab[3];                 // 字符映射表
    int Kernel = 2;                 // 能量分布算子大小
    int hold = 0;
    int order = 0;
    int limit = 20000;
    int rangenum = 11;
    bool runing = true, openlog = false;
    QTime t1, t2;
    int count=0, ABcut=0, delta=0;  // 节点数、剪枝数
    int tag=0,sto=0,ref=0;          // 命中数、存储数、冲突数
    int time_limit=0;               // 思考时间
    int depth;
    int algoFlag = 0;

    const int vx[8] = { 0, 1, 1, 1, 0,-1,-1,-1};
    const int vy[8] = {-1,-1, 0, 1, 1, 1, 0,-1};

public:

    void setFlag(bool flag = true);
    void initial(HASHITEM *_H, unsigned long _Z[20][20][3], long long _hash, int _chess[20][20],
                 int _vis[3][20][20], int key, int _limit, int _depth, int _algoFlag);

    // 局面计算函数
    void showChess();
    int valueChess(int x, int y, int key, int *piority);
    int evaluate(int key);
    void getPosition(int &x,int &y, int key, int flag);
    void powerOperation(int x, int y, int flag, int key);

    // PVS搜索
    void update(QMutex& m, Pos& ret, const Pos ref, int key, int order, int val);
    int deepSearch(Pos& ret, int origin, int key, int deep, int alpha, int beta, QVector<Pos>& path);
    int killSearch(Pos& ret, int key, int deep, int alpha, int beta, QVector<Pos>& path);

    // MTDF搜索
    int deepening(int origin, int& x, int& y);
    void mtdf(Pos& bestmove, int origin, int f, int deep);
    int alpha_beta(Pos& ret, int key, int deep, int alpha, int beta, QVector<Pos>& path, Pos& killer);

    // 越界检查函数
    bool inline inside(int x, int y);
    bool inline inside(Pos move);

    // 哈希表操作
    bool lookup(int depth, int alpha, int beta, int &val);
    bool store(QMutex &m, int depth, int val, int hashf, long long hashIndex);
};

#endif // MYTHREAD_H
