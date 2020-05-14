#include "mythread.h"
#include <QThread>
#include <QDebug>
#include <QMessageBox>

extern QVector<Pos> rec;
extern uint32_t rec_pos;

MyThread::MyThread(QObject *parent) : QObject(parent)
{
    isStop = false;
}

void MyThread::initial(HASHITEM *_H, unsigned long _Z[20][20][3], long long _hash,
                        int _chess[20][20], int _vis[3][20][20], int key, int _limit,
                        int _depth, int _algoFlag, bool _openlog, int _order)
{
    hash = _hash;
    hold = key;
    for(int i = 0; i < 15; i++){
        for(int j = 0; j < 15; j++){
            chess[i][j] = _chess[i][j];
            vis[1][i][j] = _vis[1][i][j];
            vis[2][i][j] = _vis[2][i][j];
            Z[i][j][1] = _Z[i][j][1];
            Z[i][j][2] = _Z[i][j][2];
            minX = Min(minX, i);
            minY = Min(minY, j);
        }
    }
    limit = _limit;
    depth = _depth;
    algoFlag = _algoFlag;
    openlog = _openlog;
    order = _order;
    H = new HASHITEM[HASH_TABLE_SIZE]();
    if(H == nullptr){
        qDebug("failed to create hashtable");
    }
}

MyThread::~MyThread(){
     delete H;
}

void MyThread::dowork(const QString& str)
{
    Pos ret(20, 20);
    QVector<Pos> path;
    if(isStop == false){
        if(limit > 0 && depth < 30)
        {
            t2.start();
            topFlag = true;
            if(algoFlag == 1){
                deepSearch(ret, hold, hold, depth, depth, -R_INFINTETY, R_INFINTETY, path);
            }else if(algoFlag == 0){
                killSearch(ret, hold, depth, -R_INFINTETY,R_INFINTETY, path);
            }else if(algoFlag == 2){
                int guess =str.toLong();
                MTD(ret, hold, guess, depth);
            }
        }
        QString temp;
        temp.sprintf("%d,%d,%d,%d,%d,",5 ,ret.x, ret.y, ret.value, 0);
        if(runing && limit > 0) temp += "1";
        else temp += "0";
        emit resultReady(temp);
        isStop = true;
    }
    else{
        QThread::sleep(1);
    }
}

void MyThread::setFlag(bool flag){
    isStop = flag;
}

int MyThread::valueChess(int x, int y, int key, int *piority){
    int i, j;

    int p[8];                   // p对应方向的子力
    int b[8];                   // blank对应方向的子力，之后的空位
    int bp[8];                  // bp对应方向的子力，之后的空位，之后的子力
    int bpb[8];                 // bpb对应方向的子力，之后的空位，之后的子力，之后的空位
    int two=0, three=0, jump=0, four=0, five=0;
    int sleep_three=0, sleep_two=0, sleep_jump=0;

    memset(p, 0, sizeof(p));
    memset(b, 0, sizeof(b));
    memset(bp, 0, sizeof(bp));
    memset(bpb, 0, sizeof(bpb));

    // 方向从上方开始，顺时针寻找
    for (i = 0, j = 1; i < 8; i++, j = 1){
        for(; j <= 5 && chess[x+vx[i]*j][y+vy[i]*j] == key && inside(x+vx[i]*j, y+vy[i]*j); j++, p[i]++);
        for(; j <= 5 && chess[x+vx[i]*j][y+vy[i]*j] == 0   && inside(x+vx[i]*j, y+vy[i]*j); j++, b[i]++);
        for(; j <= 5 && chess[x+vx[i]*j][y+vy[i]*j] == key && inside(x+vx[i]*j, y+vy[i]*j); j++, bp[i]++);
        for(; j <= 5 && chess[x+vx[i]*j][y+vy[i]*j] == 0   && inside(x+vx[i]*j, y+vy[i]*j); j++, bpb[i]++);
    }

    for (i = 0; i < 4; i++){

        if(p[i] + p[i+4] >= 4) {
            // OOOOO
            five++;
        }

        if(p[i] + p[i+4] == 3){
            // +OOOO+
            if(b[i] >= 1 && b[i+4] >= 1 )       { four += 2;}      // 四连
            // +OOOO
            if(b[i] == 0 && b[i+4] >= 1 )       { four++;}      // 冲四
            if(b[i] >= 1 && b[i+4] == 0 )       { four++;}      // 冲四
        }

        if(p[i] + p[i+4] == 2){
            // OOO+O
            if(b[i]   == 1 && bp[i]   >= 1 )    { four++;} // 单跳四
            if(b[i+4] == 1 && bp[i+4] >= 1 )    { four++;}
            // ++OOO+
            if     (b[i] >= 2 && b[i+4] >= 1 && b[i] + b[i+4] >= 3 && bp[i+4] == 0)   { three++;}   // 活三
            else if(b[i] >= 1 && b[i+4] >= 2 && b[i] + b[i+4] >= 3 && bp[i]   == 0)   { three++;}   // 活三
            // OOO++ // +OOO+
            if(b[i] == 1 && b[i+4] == 1)           { sleep_three++;}    // 眠三
            else if(b[i] == 0 && b[i+4] >= 2)           { sleep_three++;}    // 眠三
            else if(b[i+4] == 0 && b[i] >= 2)           { sleep_three++;}    // 眠三
        }

        if(p[i] + p[i+4] == 1){
            // OO+OO
            if(b[i]   == 1 && bp[i]   >= 2 )    { four++;}   // 单跳四
            if(b[i+4] == 1 && bp[i+4] >= 2 )    { four++;}
            // +OO+O+
            if     (b[i]   == 1 && bp[i]   == 1 && bpb[i] >= 1 && b[i+4] >= 1 && b[i+4] - bp[i+4] > 0)   { jump++;} // 跳三
            else if(b[i+4] == 1 && bp[i+4] == 1 && b[i] >= 1 && bpb[i+4] >= 1 && b[i] - bp[i] > 0)   { jump++;} // 跳三

            // OO+O+ or +OO+O
            if     (b[i]   == 1 && bp[i]   == 1 && bpb[i] + b[i+4] == 1 )   { sleep_jump++;}       // 眠三
            else if(b[i+4] == 1 && bp[i+4] == 1 && b[i] + bpb[i+4] == 1 )   { sleep_jump++;}
            // OO++O
            if     (b[i]   == 2 && bp[i]   >= 1 )                   { sleep_jump++;}
            else if(b[i+4] == 2 && bp[i+4] >= 1 )                   { sleep_jump++;}
            // +++OO++ && ++OO+++
            if (b[i] >= 1 && b[i+4] >= 1 && b[i] + b[i+4] >= 5)  { two++; }       // 活二
            else if (b[i] + b[i+4] <= 5)  { sleep_two++; }       // 眠二
            else if (b[i] == 0 && b[i+4] >= 5)  { sleep_two++; }       // 眠二
            else if (b[i+4] == 0 && b[i] >= 5)  { sleep_two++; }       // 眠二

        }

        if(p[i] + p[i+4] == 0){
            // O+OOO
            if(b[i]   == 1 && bp[i]   >= 3 )    { four++;}
            if(b[i+4] == 1 && bp[i+4] >= 3 )    { four++;}
            // +O+OO+
            if     (b[i]   == 1 && bp[i]   == 2 && bpb[i]   >= 1 && b[i+4] >= 1)   { jump++;}
            else if(b[i+4] == 1 && bp[i+4] == 2 && bpb[i+4] >= 1 && b[i]   >= 1)   { jump++;}

            // O+OO+ && +O+OO
            if((b[i] == 1 && bp[i] == 2 && (bpb[i] >= 1 ||  b[i+4] >= 1)))           { sleep_jump++;}
            else if((b[i+4] == 1 && bp[i+4] == 2 && (bpb[i+4] >= 1 ||  b[i] >= 1)))  { sleep_jump++;}

            // +O+O++
            if(b[i]   >= 2 && b[i+4] == 1 && bp[i+4] == 1 &&  bpb[i+4] >= 1)    { two++;}
            if(b[i+4] >= 2 && b[i]   == 1 && bp[i]   == 1 &&  bpb[i]   >= 1)    { two++;}

            // +O++O+
            if(b[i]   >= 1 && b[i+4] == 2 && bp[i+4] == 1 &&  bpb[i+4] >= 1)    { two++;}
            if(b[i+4] >= 1 && b[i]   == 2 && bp[i]   == 1 &&  bpb[i]   >= 1)    { two++;}

            // O+O++ or ++O+O
            if(b[i]   >= 2 && b[i+4] == 1 && bp[i+4] == 1 &&  bpb[i+4] == 0)    { sleep_two++;}
            if(b[i+4] >= 2 && b[i]   == 1 && bp[i]   == 1 &&  bpb[i]   == 1)    { sleep_two++;}
            if(b[i]   == 0 && b[i+4] == 1 && bp[i+4] == 1 &&  bpb[i+4] >= 2)    { sleep_two++;}
            if(b[i+4] == 0 && b[i]   == 1 && bp[i]   == 1 &&  bpb[i]   >= 2)    { sleep_two++;}
        }
    }

     *piority = jump + 2*three + 100*four + 10000*five;

    if (five >= 1)
        return chessValue[10];

    if (four >= 2)
        return chessValue[9];

    if (four >= 1 && jump+three >= 1)
        return chessValue[8];

    if (jump+three >= 2)
        return chessValue[7];

    return chessValue[0]*sleep_two + chessValue[1]*sleep_three +
           chessValue[2]*sleep_jump + chessValue[3]*two + chessValue[4]*jump +
           chessValue[5]*three + chessValue[6]*four + add[x][y];
}

bool inline MyThread::inside(int x, int y){
    if (x < 15 && y < 15 && x >= 0 && y >= 0)
        return true;
    return false;
}

bool inline MyThread::inside(Pos move){
    if (move.x < 15 && move.y < 15 && move.x >= 0 && move.y >= 0)
        return true;
    return false;
}

int MyThread::deepSearch(Pos& ret, int origin, int key, int deep, int rec, int alpha, int beta, QVector<Pos>& path)
{
    int i, j, p1, p2, k;
    QVector<Pos> attackQueue, vec_moves;

    if(t2.elapsed() > limit){
        if(runing) runing = false;
        return alpha;
    }

    // 检查游戏是否结束
    if(rec > deep && path.last().a1 >= 10000)
        return -R_INFINTETY;

    for (i = 0; i < 15; i++){
        for (j = 0; j < 15; j++){
            if(vis[2][i][j] >= 2 && chess[i][j] == 0){
                // 算杀的结点选择
                // 进攻方：活三、冲四、活四、五连、防守对方的冲四
                // 防守方：防守对方的活三、冲四，自身的冲四
                k = Max(1.5*valueChess(i, j, key, &p1), valueChess(i, j, 3-key, &p2));
                if(origin == key){
                    // 进攻方选点，己方的进攻棋，和对方的冲四棋
                    if(p1 > 0 || p2 >= 10000){
                        attackQueue.push_back(Pos(i, j, k, p1, rec-deep, p2));
                    }
                }else{
                    // 防守方选点，己方的冲四棋，和对方的进攻棋
                    if(p2 > 0 || p1 >= 100){
                        attackQueue.push_back(Pos(i, j, k, p1, rec-deep, p2));
                    }
                }
            }
        }
    }

    // 进攻方无棋
    if(origin == key && attackQueue.isEmpty())
        return -R_INFINTETY;
    // 防守方无棋
    if(origin == 3-key && attackQueue.isEmpty())
        return -R_INFINTETY;

    qSort(attackQueue.begin(), attackQueue.end(), [](Pos& a, Pos &b){
        return a.value > b.value;
    });

    //五连棋型
    if (attackQueue[0].value >= 2000){
        for(i = 0; i < attackQueue.size() && attackQueue[i].value >= 2000; i++)
            vec_moves.push_back(attackQueue[i]);
    }
    //己方四连
    else if (attackQueue[0].value >= 1500){
        for(i = 0; i < attackQueue.size() && attackQueue[i].value >= 1500; i++)
            vec_moves.push_back(attackQueue[i]);
    }
    //对方四连
    else if (attackQueue[0].value >= 1000){  

        for(i = 0; i < attackQueue.size() && attackQueue[i].value >= 1000; i++)
            vec_moves.push_back(attackQueue[i]);

        for(;i < attackQueue.size(); i++){
            if (attackQueue[i].a1 + attackQueue[i].a3 >= 100)
                vec_moves.push_back(attackQueue[i]);
        }
    }
    // 普通情况
    else
        vec_moves = attackQueue;

    // 如果进攻方没找到结点，情况如下：
    // 1.局面没有进攻结点了，表明进攻失败，应该返回alpha。
    // 2.进攻过程中遭到反击，如反活三，说明进攻不能成功，应该返回alpha。
    // 如果防守方没找到结点，情况如下：
    // 1.无法阻挡攻势，应该返回-INF。

    // 进攻方无棋
    if(origin == key && vec_moves.isEmpty())
        return -R_INFINTETY;
    // 防守方无棋
    if(origin == 3-key && vec_moves.isEmpty())
        return -R_INFINTETY;



    qSort(vec_moves.begin(), vec_moves.end(), [](Pos &a, Pos &b){
        return a.value > b.value;
    });

    for(Pos& move: vec_moves){

        powerOperation(move.x, move.y, FLAGS_POWER_CONDESE, key);
        path.push_back(move);

        if(deep > 1)
            move.value = - deepSearch(ret, origin, 3-key, deep-1, rec, -beta, -alpha, path);
        else{
            move.value = evaluate(key);
        }

//        QString tmp, out;
//        for(Pos &a: path){
//            tmp.sprintf("[%d,%d],", a.x, a.y);
//            out += tmp;
//        }
//        tmp.sprintf(" = %d\n", move.value);
//        out += tmp;
//        qDebug(out.toLatin1());

        path.pop_back();
        powerOperation(move.x, move.y, FLAGS_POWER_RELEASE, key);

        if(move.value > alpha){
            alpha = move.value;
            update(mutex, ret, move);
        }

        if (move.value >= beta){
            ABcut++;
            return beta;
        }
    }
    return alpha;

}

int MyThread::killSearch(Pos& ret, int key, int deep, int alpha, int beta, QVector<Pos>& path)
{
    int i, j, p1, p2, k;
    int hashf = HASH_ALPHA;
    long long hashIndex=0, hashBest=0;
    Pos newMove(i, j, 0, 0, depth-deep), bestMove;
    QVector<Pos> attackQueue, vec_moves;

    if(t2.elapsed() > limit){
        if(runing) runing = false;
        return alpha;
    }

    if(depth > deep && path.last().a1 >= 10000)
        return -R_INFINTETY;

    if(lookup(deep, alpha, beta, newMove)){
        update(mutex, ret, newMove);
        return newMove.value;
    }

    for (i = 0; i < 15; i++){
        for (j = 0; j < 15; j++){
            if(chess[i][j] == 0 && vis[2][i][j] >= 2){
                k = Max(1.5*valueChess(i, j, key, &p1), valueChess(i, j, 3-key, &p2));
                attackQueue.push_back(Pos{i, j, k, p1, depth-deep, Max(p1, p2)});
            }
        }
    }

    qSort(attackQueue.begin(), attackQueue.end(), [](Pos& a, Pos &b){
        return a.value > b.value;
    });

    //五连棋型
    if (attackQueue[0].value >= 10000){
        for(i=0; attackQueue[i].value >= 11000 && i < attackQueue.size(); i++)
            vec_moves.push_back(attackQueue[i]);
    }
    //四连棋型
    else if (attackQueue[0].value >= 1100){
        for(i=0; attackQueue[i].value >= 1100 && i < attackQueue.size(); i++)
            vec_moves.push_back(attackQueue[i]);
    }
    else if (attackQueue[0].value >= 1000){
        for(i=0; attackQueue[i].value >= 1000 && i < attackQueue.size(); i++)
            vec_moves.push_back(attackQueue[i]);
        for(; i < attackQueue.size(); i++){
            valueChess(attackQueue[i].x, attackQueue[i].y, key, &p1);
            if (p1 >= 100) vec_moves.push_back(attackQueue[i]);
        }
    }
    // 普通情况
    else
        vec_moves.swap(attackQueue);

    if(vec_moves.isEmpty())
        return alpha;

    qSort(vec_moves.begin(), vec_moves.end(), [](Pos &a, Pos &b){
        return a.value > b.value;
    });

    int cur = -R_INFINTETY;
    for(Pos& move: vec_moves){

        powerOperation(move.x, move.y, FLAGS_POWER_CONDESE, key);
        path.push_back(move);

        if(deep > 1){
            if(move == vec_moves[0]){
                move.value = - killSearch(ret, EXCHANGE - key, deep - 1, -beta, -alpha, path);
            }
            else{
                move.value = - killSearch(ret, EXCHANGE - key, deep - 1, -alpha-1, -alpha, path);
                if(alpha < move.value && move.value < beta){
                    move.value = - killSearch(ret, EXCHANGE - key, deep - 1, -beta, -alpha, path);
                }
            }
        }else{
            move.value = evaluate(key);
            store(mutex, HASH_EXACT, hash, move, deep);
        }

        hashIndex = hash;
        path.pop_back();
        powerOperation(move.x, move.y, FLAGS_POWER_RELEASE, key);

        if(cur < move.value){
            cur = move.value;
            if(alpha < move.value){
                if(beta <= move.value){
                    ABcut++;
                    if(runing) store(mutex, HASH_BETA, hashIndex, move, deep);
                    return move.value;
                }
                alpha = move.value;
                hashf = HASH_EXACT;
                hashBest = hashIndex;
                bestMove = move;
                update(mutex, ret, move);
            }
        }
    }
    if(runing) store(mutex, hashf, hashBest, bestMove, deep);
    return cur;
}

void MyThread::MTD(Pos& bestmove, int origin, int f, int deep)
{
    int alpha, beta, best_value, test, speed[2]={0, 0};
    Pos newMove = Pos(20, 20);
    QVector<Pos> path;

    test  = f;
    alpha = -R_INFINTETY;
    beta  = +R_INFINTETY;

    do{
        best_value = MT(newMove, origin, deep, test-1, test, path);
        if(best_value < test){
            // alpha结点，找自己最差的结点
            beta = test = best_value;
        }else{
            // beta结点，找自己最好的结点
            alpha = best_value;
            if(runing){
                bestmove = newMove;
                bestmove.value = alpha;
            }
            test = best_value+1;
        }

    }while(alpha < beta);
}

int MyThread::MT(Pos& ret, int key, int deep, int alpha, int beta, QVector<Pos>& path)
{
    int i, j, p1, p2, cur=-R_INFINTETY, k;
    int hashf = HASH_ALPHA;
    long long hashIndex=0, hashBest;
    Pos newMove, bestMove;
    QVector<Pos> vec_moves, attackQueue, tmp;

    if(t2.elapsed() > limit){
        if(runing) runing = false;
        return alpha;
    }

    // 检查游戏是否结束
    if(depth > deep && path.last().a1 >= 10000)
        return -R_INFINTETY;

    // 查找哈希表
    if(lookup(deep, alpha, beta, newMove)){

        newMove.a2 = depth - deep;
        valueChess(newMove.x, newMove.y, key, &newMove.a1);

        update(mutex, ret, newMove);
        return newMove.value;
    }

    if(rec_pos > 0 && depth == deep){

        // 读取记录
        vec_moves = rec;

    }else{
        // 生成合适着法
        for (i = 0; i < 15; i++){
            for (j = 0; j < 15; j++){
                if (vis[2][i][j] >= 2 && chess[i][j] == 0){
                    k = Max(1.5*valueChess(i, j, key, &p1), valueChess(i, j, 3-key, &p2));
                    attackQueue.push_back(Pos(i, j, k, p1, depth-deep, p2));
                }
            }
        }

        qSort(attackQueue.begin(), attackQueue.end(), [](Pos& a, Pos &b){
            return a.value > b.value;
        });

        //五连棋型
        if (attackQueue[0].value >= 2000){
            for(i = 0; i < attackQueue.size() && attackQueue[i].value >= 2000; i++)
                vec_moves.push_back(attackQueue[i]);
        }
        //己方四连
        else if (attackQueue[0].value >= 1500){
            for(i = 0;i < attackQueue.size() && attackQueue[i].value >= 1500; i++)
                vec_moves.push_back(attackQueue[i]);
        }
        //对方四连
        else if (attackQueue[0].value >= 1000){

            if(depth == deep && rec_pos == 0){

                // 首发剪枝
                for(i = 0; i < attackQueue.size() && attackQueue[i].value >= 1000; i++)
                    tmp.push_back(attackQueue[i]);

                for(;i < attackQueue.size(); i++)
                    if (attackQueue[i].a1 + attackQueue[i].a3 >= 100)
                        tmp.push_back(attackQueue[i]);

                cutTreeNode(tmp, vec_moves, path, key);
                rec = vec_moves;
                rec_pos = depth;

            }else{

                for(i = 0; i < attackQueue.size() && attackQueue[i].value >= 1000; i++)
                    vec_moves.push_back(attackQueue[i]);

                for(;i < attackQueue.size(); i++)
                    if (attackQueue[i].a1 + attackQueue[i].a3 >= 100)
                        vec_moves.push_back(attackQueue[i]);
            }

        }else{
            if(depth == deep && rec_pos == 0){
                // 首发剪枝
                cutTreeNode(attackQueue, vec_moves, path, key);
                rec = vec_moves;
                rec_pos = depth;
            }else{
                // 普通情况
                vec_moves = attackQueue;
            }
        }
    }

    // 遍历搜索树
    int cnt = 0;
    for(Pos& move: vec_moves){

        if(cnt++ >= rangenum) break;

        hashIndex = hash;
        powerOperation(move.x, move.y, FLAGS_POWER_CONDESE, key);
        path.push_back(move);

        if(deep > 1)
            move.value = - MT(ret, 3-key, deep-1, -beta, -alpha, path);
        else{
            move.value = evaluate(key);
            store(mutex, HASH_EXACT, hash, move, deep);
        }

        powerOperation(move.x, move.y, FLAGS_POWER_RELEASE, key);
        path.pop_back();

        if(move.value > cur){
            cur = move.value;

            if(move.value > alpha){
                alpha = move.value;
                hashf = HASH_EXACT;
                hashBest = hashIndex;
                bestMove = move;
                update(mutex, ret, move);
            }
            if(move.value >= beta){
                ABcut++;
                if(runing) store(mutex, HASH_BETA, hashIndex, move, deep);
                return move.value;
            }
        }
    }
    if(runing) store(mutex, hashf, hashBest, bestMove, deep);
    return cur;
}

void MyThread::cutTreeNode(QVector<Pos>& queue_move, QVector<Pos>& vec_moves, QVector<Pos>& path, int key){

    if(queue_move.isEmpty())
        return;

    Pos newMove;
    int MaxDeep = (queue_move.size() <= rangenum/2) ? (12) : (10);

    for(Pos &move: queue_move)
    {
        powerOperation(move.x, move.y, FLAGS_POWER_CONDESE, key);
        path.push_back(move);
        newMove.value = evaluate(3-key);
        for(int d = 2; d <= MaxDeep && newMove.value < chessValue[10]; d += 2){
            newMove.value = deepSearch(newMove, 3-key, 3-key, d, d, -R_INFINTETY, R_INFINTETY, path);
            move.a3 = d;
        }
        path.pop_back();
        powerOperation(move.x, move.y, FLAGS_POWER_RELEASE, key);

        if(newMove.value < chessValue[10]){
            vec_moves.push_back(move);
        }
    }

    // 当全为必败着法时，保存最长的结果
    if(vec_moves.isEmpty()){

        if(openlog){
            qDebug("No enough move, find longest move");
        }

        qSort(queue_move.begin(), queue_move.end(), [](Pos &a, Pos &b){
            return a.a3 > b.a3;
        });

        vec_moves.push_back(queue_move[0]);
        for(int i=1; i < queue_move.size() && queue_move[i].a3 >= vec_moves[0].a3; i++){
            vec_moves.push_back(queue_move[i]);
        }
    }
    if(openlog){
        qDebug("[%d -> %d]", queue_move.size(), vec_moves.size());
    }
}

void MyThread::update(QMutex& m, Pos& ret, const Pos ref)
{
    QMutexLocker locker(&m);
    if(inside(ref) && ref.a2 == ret.a2 && (ref.value > ret.value || !inside(ret))){
//        if(openlog) qDebug("(%d,%d,%d)->(%d,%d,%d) alpha", ret.x, ret.y, ret.value, ref.x, ref.y, ref.value);
        ret.x = ref.x;
        ret.y = ref.y;
        ret.value = ref.value;
        ret.a3++;
    }
}

int MyThread::evaluate(int key)
{
    int i, j, p;
    int o_prior=0, d_prior=0, o_val=0, d_val=0;
    for (i = minX; i < 15; i++)
        for (j = minY; j < 15; j++){
            if(chess[i][j] == 3-key)
            {
                o_val = Max(o_val, valueChess(i, j, 3-key, &p));
                o_prior =  Max(o_prior, p);

            }
            if(chess[i][j] == key){
                d_val = Max(d_val,valueChess(i, j, key, &p));
                d_prior = Max(d_prior, p);
            }
        }
    // 后手方五连
    if (d_prior >= 10000)
        return R_INFINTETY;
    // 先手方五连
    if (o_prior >= 10000)
        return -R_INFINTETY;
    // 先手方有先手，后手方无更高级先手
    if(o_prior > 0 && o_prior < 100 && d_prior < 100)
        return -R_INFINTETY;
    // 冲四
    if(o_prior >= 100 && o_prior < 10000 && d_prior < 10000)
        return -R_INFINTETY;
    // 五连
    if(o_prior >= 10000 && d_prior >= 10000)
        return -R_INFINTETY;

    if(d_prior>=1)  d_val *= 1.5;
    if(o_prior>=1)  o_val *= 1.5;
    return d_val - o_val;
}

void MyThread::powerOperation(int x, int y, int flag, int key)
{
    int i, j, dx, dy, k, p;

    if (flag == FLAGS_POWER_CONDESE){
        order++;
        hash ^= Z[x][y][key];
        chess[x][y] = key;
        minX = Min(minX, x);
        minY = Min(minY, y);
        FF(0, 8, 1, 3){
            dx = x + vx[i]*j;
            dy = y + vy[i]*j;
            if(inside(dx, dy)){
                vis[2][dx][dy]++;
            }
        }
    }
    else{
        order--;
        hash ^= Z[x][y][key];
        chess[x][y] &= 0;
        FF(0, 8, 1, 3){
            dx = x + vx[i]*j;
            dy = y + vy[i]*j;
            if(inside(dx, dy)){
                vis[2][dx][dy]--;
            }
        }
    }
}
