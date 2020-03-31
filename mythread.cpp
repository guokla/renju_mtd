#include "mythread.h"
#include <QThread>
#include <QDebug>
#include <QMessageBox>

MyThread::MyThread(QObject *parent) : QObject(parent)
{
    isStop = false;
}

void MyThread::initial(HASHITEM *_H,
                       unsigned long _Z[20][20][3],
                        long long _hash,
                        int _chess[20][20],
                        int _vis[3][20][20],
                        int key,
                        int _limit,
                        int _depth,
                        int _algoFlag,
                        bool _openlog,
                        int _order,
                        int _valTab[20][20][3],
                        int _priorTab[20][20][3])
{
    hash = _hash;
    hold = key;
    memset(sumTab, 0, sizeof(sumTab));
    for(int i = 0; i < 15; i++){
        for(int j = 0; j < 15; j++){
            chess[i][j] = _chess[i][j];
            vis[1][i][j] = _vis[1][i][j];
            vis[2][i][j] = _vis[2][i][j];
            Z[i][j][1] = _Z[i][j][1];
            Z[i][j][2] = _Z[i][j][2];
            valTab[i][j][1] = _valTab[i][j][1];
            valTab[i][j][2] = _valTab[i][j][2];
            priorTab[i][j][1] = _priorTab[i][j][1];
            priorTab[i][j][2] = _priorTab[i][j][2];
            sumTab[0][1] += _valTab[i][j][1];
            sumTab[0][2] += _valTab[i][j][2];
            sumTab[1][1] += _priorTab[i][j][1];
            sumTab[1][2] += _priorTab[i][j][2];
        }
    }
    limit = _limit;
    depth = _depth;
    algoFlag = _algoFlag;
    openlog = _openlog;
    init_order = order = _order;
    strTab[0] = ' ';
    strTab[1] = 'M';
    strTab[2] = 'O';
    H = new HASHITEM[HASH_TABLE_SIZE]();
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
                deepSearch(ret, hold, hold, depth, -R_INFINTETY, R_INFINTETY, path);
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
        if(openlog) qDebug("[深度%2d],cnt=%d, tag=%d, sto=%d\n",depth, count, tag, sto);
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
    int score = 0;

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

    score = 0;

    if (five >= 1)
        score = Max(score, 200);

    if (four >= 2)
        score = Max(score,  120);

    if (four >= 1 && jump+three >= 1)
        score = Max(score, 80);

    if (jump+three >= 2)
        score = Max(score, 50);

<<<<<<< HEAD
    score += (sleep_two + 2*sleep_three + 2*sleep_jump + 2*jump + 3*two + 5*three + 4*four);
=======
    score += (sleep_two + 2*sleep_three + 2*sleep_jump + 2*jump + 3*two + 5*three);
    score += 5/(1+abs(x-7)+abs(y-7));
>>>>>>> aac07a3ed3c2424844a906d68368d89b82712647

    *piority = jump + 2*three + 100*four + 10000*five;

    return score;
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

int MyThread::deepSearch(Pos& ret, int origin, int key, int deep, int alpha, int beta, QVector<Pos>& path)
{
    int i, j, p1, p2, k, three=0;
    int hashf = HASH_ALPHA;
    long long hashIndex=0, hashBest=0;
    Pos newMove, bestMove;
    QVector<Pos> attackQueue, vec_moves;

    if(t2.elapsed() > limit){
        if(runing) runing = false;
        return alpha;
    }

    if(lookup(deep, alpha, beta, newMove)){
        update(mutex, ret, newMove);
        return newMove.value;
    }

    for (i = 0; i < 15; i++){
        for (j = 0; j < 15; j++){
            if(chess[i][j] == 0 && vis[2][i][j] >= 2){
                // 算杀的结点选择
                // 进攻方：活三、冲四、活四、五连、防守对方的冲四
                // 防守方：防守对方的活三、冲四，自身的冲四
                k = 2*valueChess(i, j, key, &p1) + valueChess(i, j, 3-key, &p2);
                if(origin == key){
                    // 进攻方选点
                    if(p1 > 0 || p2 >= 10000){
                        attackQueue.push_back(Pos(i, j, k, p1, depth-deep));
                    }
                }else{
                    // 防守方选点
                    if(p2 > 0 || p1 >= 100){
                        attackQueue.push_back(Pos(i, j, k, p1, depth-deep));
                    }
                }
            }
        }
    }

    while(!attackQueue.isEmpty()){

        // 取出尾结点，并从队列中删除
        newMove = attackQueue.last();
        attackQueue.pop_back();
        count++;

        // 执行试探测试
        if(newMove.a1 > 0) three++;
        powerOperation(newMove.x, newMove.y, FLAGS_POWER_CONDESE, key);
        k = evaluate(key);
        powerOperation(newMove.x, newMove.y, FLAGS_POWER_RELEASE, key);

        if (k <= -0.5*R_INFINTETY) continue;
        newMove.value = k;

        vec_moves.push_back(newMove);
    }

    // 如果进攻方没找到结点，情况如下：
    // 1.局面没有进攻结点了，表明进攻失败，应该返回alpha。
    // 2.进攻过程中遭到反击，如反活三，说明进攻不能成功，应该返回alpha。
    // 如果防守方没找到结点，情况如下：
    // 1.无法阻挡攻势，应该返回-INF。

    // 第一层搜索无活三
    if(origin == key && three == 0 && topFlag)
        return alpha;
    // 进攻方无棋
    if(origin == key && vec_moves.isEmpty())
        return alpha;
    // 防守方无棋
    if(origin == 3-key && vec_moves.isEmpty())
        return -R_INFINTETY;

    qSort(vec_moves.begin(), vec_moves.end(), greater<Pos>());

    if(origin == key && vec_moves[0].a1 >= 10000){
        update(mutex, ret, vec_moves[0]);
        return beta;
    }

    for(Pos& move: vec_moves){

        powerOperation(move.x, move.y, FLAGS_POWER_CONDESE, key);
        path.push_back(move);

        if(deep > 1)
            move.value = - deepSearch(ret, origin, 3-key, deep - 1, -beta, -alpha, path);
        else{
            move.value = evaluate(key);
            if(runing) store(mutex, HASH_EXACT, hash, move, deep);
        }

        hashIndex = hash;
        path.pop_back();
        powerOperation(move.x, move.y, FLAGS_POWER_RELEASE, key);

        if(move.value > alpha){
            alpha = move.value;
            hashf = HASH_EXACT;
            hashBest = hashIndex;
            bestMove = move;
            update(mutex, ret, move);
        }

        if (move.value >= beta){
            ABcut++;
            if(runing) store(mutex, HASH_BETA, hashIndex, move, deep);
            return move.value;
        }
    }
    if(runing) store(mutex, hashf, hashBest, bestMove, deep);
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

    if(path.last().a1 >= 10000) return -R_INFINTETY;

    if(lookup(deep, alpha, beta, newMove)){
        update(mutex, ret, newMove);
        return newMove.value;
    }

    for (i = 0; i < 15; i++){
        for (j = 0; j < 15; j++){
            if(chess[i][j] == 0 && vis[2][i][j] >= 2){
                k = 2*valueChess(i, j, key, &p1) + valueChess(i, j, 3-key, &p2);
                attackQueue.push_back(Pos{i, j, k, p1, depth-deep, Max(p1, p2)});
            }
        }
    }

    qSort(attackQueue.begin(), attackQueue.end(), [](const Pos a, const Pos b){return a.a3>b.a3;});

    if(attackQueue[0].a3 >= 10000){
        attackQueue[0].value = R_INFINTETY;
        vec_moves.push_front(attackQueue[0]);
    }else if(attackQueue[0].a3 >= 200){
        for(i = 0; i < attackQueue.size(); i++){
            if(attackQueue[i].a3 >= 200)
                vec_moves.push_front(attackQueue[i]);
            else break;
        }
        // 找双方的冲四点
        for(i = vec_moves.size(); i < attackQueue.size(); i++){
            valueChess(attackQueue[i].x, attackQueue[i].y, key, &p1);
            valueChess(attackQueue[i].x, attackQueue[i].y, 3-key, &p2);
            if(p1+p2 >= 100)
                vec_moves.push_front(attackQueue[i]);
            else break;
        }
    }else{
        for(Pos temp: attackQueue){
            if(vec_moves.size() > rangenum) break;
            count++;
            powerOperation(temp.x, temp.y, FLAGS_POWER_CONDESE, key);
            temp.value = evaluate(key);
            powerOperation(temp.x, temp.y, FLAGS_POWER_RELEASE, key);

            if (temp.value <= -0.5*R_INFINTETY) continue;
            vec_moves.push_back(temp);
        }
    }

    if(vec_moves.isEmpty())
        return alpha;

    qSort(vec_moves.begin(), vec_moves.end(), greater<Pos>());

    if(topFlag) topFlag = false;
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
            beta = best_value;
            speed[0]++;speed[1]=0;
            if(speed[0] > 2)
                test = (alpha+beta)>>1;
             else
                test = best_value;
        }else{
            // beta结点，找自己最好的结点
            alpha = best_value;
            if(runing) bestmove = newMove;
            speed[1]++;speed[0]=0;
            if(speed[1] > 2)
                test = 1+((alpha+beta)>>1);
             else
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
    QVector<Pos> vec_moves, attackQueue;

    if(t2.elapsed() > limit){
        if(runing) runing = false;
        return alpha;
    }

    if(path.last().a1 >= 10000) return -R_INFINTETY;

    // 查找哈希表
    if(lookup(deep, alpha, beta, newMove)){
        update(mutex, ret, newMove);
        return newMove.value;
    }

    // 生成合适着法
    for (i = 0; i < 15; i++){
        for (j = 0; j < 15; j++){
            if (chess[i][j] == 0 && vis[2][i][j] >= 2){
                k = 2*valueChess(i, j, key, &p1) + valueChess(i, j, 3-key, &p2);
                attackQueue.push_back(Pos(i, j, k, p1, depth-deep, Max(p1, p2)));
            }
        }
    }

    qSort(attackQueue.begin(), attackQueue.end(), [](const Pos &a, const Pos &b){return a.a3 >b.a3;});

    if(attackQueue[0].a3 >= 10000){
        attackQueue[0].value = R_INFINTETY;
        vec_moves.push_back(attackQueue[0]);
    }else if(attackQueue[0].a3 >= 200){
        for(i = 0; i < attackQueue.size(); i++){
            if(attackQueue[i].a3 >= 200){
                vec_moves.push_back(attackQueue[i]);
            }else break;
        }
        // 找双方的冲四点
        for(i = vec_moves.size(); i < attackQueue.size(); i++){
            valueChess(attackQueue[i].x, attackQueue[i].y, key, &p1);
            valueChess(attackQueue[i].x, attackQueue[i].y, 3-key, &p2);
            if(p1+p2 >= 100)
                vec_moves.push_back(attackQueue[i]);
            else break;
        }
    }else{
        for(Pos& move: attackQueue){
            if(vec_moves.size() > rangenum) break;
            count++;
            powerOperation(move.x, move.y, FLAGS_POWER_CONDESE, key);
            move.value = evaluate(key);
            powerOperation(move.x, move.y, FLAGS_POWER_RELEASE, key);

            if (move.value <= -0.5*R_INFINTETY)
                continue;

            vec_moves.push_back(move);
        }
    }


    qSort(vec_moves.begin(), vec_moves.end(), [](const Pos &a, const Pos &b){return a.value >b.value;});

    // 遍历搜索树
    if(topFlag) topFlag = false;
    for(Pos& move: vec_moves){

        hashIndex = hash;
        powerOperation(move.x, move.y, FLAGS_POWER_CONDESE, key);

        if(deep > 1)
            move.value = - MT(ret, 3-key, deep-1, -beta, -alpha, path);
        else{
            move.value = evaluate(key);
            store(mutex, HASH_EXACT, hash, move, deep);
        }

        powerOperation(move.x, move.y, FLAGS_POWER_RELEASE, key);

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

void MyThread::update(QMutex& m, Pos& ret, const Pos ref)
{
    QMutexLocker locker(&m);
    if(ref.a2 == ret.a2 && (ref.value > ret.value || ret.a3 == 0)){
        if(openlog) qDebug("(%d,%d,%d)->(%d,%d,%d) alpha", ret.x, ret.y, ret.value, ref.x, ref.y, ref.value);
        ret.x = ref.x;
        ret.y = ref.y;
        ret.value = ref.value;
        ret.a3++;
    }
}

int MyThread::evaluate(int key)
{
    int o_prior=sumTab[1][3-key], d_prior=sumTab[1][key];
    int o_val=sumTab[0][3-key], d_val=sumTab[0][key];

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

    if(d_prior>=1)  d_val *=1.5;
    if(o_prior>=1)  o_val *=1.5;
    return d_val - o_val;
}

void MyThread::powerOperation(int x, int y, int flag, int key)
{
    int i, j, dx, dy, k, p;

    if (flag == FLAGS_POWER_CONDESE){
        order++;
        hash ^= Z[x][y][key];
        chess[x][y] = key;
        for(auto dis = 2; dis <= Kernel; dis++)for(i = 0; i < 8; i++){
            for(j = 1; j <= dis; j++){
                dx = x+vx[i]*j;
                dy = y+vy[i]*j;
                if(inside(dx, dy)){
                    vis[dis][dx][dy]++;
                }
            }
        }
        // 更新权值
        sumTab[0][key] -= valTab[x][y][key];
        sumTab[1][key] -= priorTab[x][y][key];

        valTab[x][y][key] = valueChess(x, y, key, &p);
        priorTab[x][y][key] = p;

        sumTab[0][key] += valTab[x][y][key];
        sumTab[1][key] += priorTab[x][y][key];
        for(i=0, j=1; i<8; i++, j=1){
            dx=x+vx[i];dy=y+vy[i];
            for(;j<=5 && inside(dx, dy); ++j, dx=x+vx[i]*j, dy=y+vy[i]*j){
                if(chess[dx][dy] > 0){
                    k = chess[dx][dy];
                    sumTab[0][k] -= valTab[dx][dy][k];
                    sumTab[1][k] -= priorTab[dx][dy][k];
                    valTab[dx][dy][k] = valueChess(dx, dy, k, &p);
                    priorTab[dx][dy][k] = p;
                    sumTab[0][k] += valTab[dx][dy][k];
                    sumTab[1][k] += priorTab[dx][dy][k];
                }
            }
        }
    }
    else{
        order--;
        hash ^= Z[x][y][key];
        chess[x][y] = 0;
        for(auto dis = 2; dis <= Kernel; dis++)for(i = 0; i < 8; i++){
            for(j = 1; j <= dis; j++){
                dx = x+vx[i]*j;
                dy = y+vy[i]*j;
                if(inside(dx, dy)){
                    vis[dis][dx][dy]--;
                }
            }
        }
        // 更新权值
        sumTab[0][key] -= valTab[x][y][key];
        sumTab[1][key] -= priorTab[x][y][key];

        valTab[x][y][key] = 0;
        priorTab[x][y][key] = 0;

        sumTab[0][key] += valTab[x][y][key];
        sumTab[1][key] += priorTab[x][y][key];
        for(i=0, j=1; i<8; i++, j=1){
            dx=x+vx[i];dy=y+vy[i];
            for(;j<=5 && inside(dx, dy); ++j, dx=x+vx[i]*j, dy=y+vy[i]*j){
                if(chess[dx][dy] > 0){
                    k = chess[dx][dy];
                    sumTab[0][k] -= valTab[dx][dy][k];
                    sumTab[1][k] -= priorTab[dx][dy][k];
                    valTab[dx][dy][k] = valueChess(dx, dy, k, &p);
                    priorTab[dx][dy][k] = p;
                    sumTab[0][k] += valTab[dx][dy][k];
                    sumTab[1][k] += priorTab[dx][dy][k];
                }
            }
        }
    }
}

MyThread::~MyThread(){
     delete H;
}
