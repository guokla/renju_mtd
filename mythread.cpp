#include "mythread.h"
#include <QThread>
#include <QDebug>
#include <QMessageBox>

MyThread::MyThread(QObject *parent) : QObject(parent)
{
    isStop = false;
}

void MyThread::initial(HASHITEM *_H, unsigned long _Z[20][20][3], long long _hash, int _chess[20][20],
                        int _vis[3][20][20], int key, int _limit, int _depth, int _algoFlag)
{
    hash = _hash;
    H = _H;
    hold = key;
    for(int i = 0; i < 15; i++){
        for(int j = 0; j < 15; j++){
            chess[i][j] = _chess[i][j];
            vis[1][i][j] = _vis[1][i][j];
            vis[2][i][j] = _vis[2][i][j];
            Z[i][j][1] = _Z[i][j][1];
            Z[i][j][2] = _Z[i][j][2];
        }
    }
    limit = _limit;
    depth = _depth;
    algoFlag = _algoFlag;
    strTab[0] = ' ';
    strTab[1] = 'M';
    strTab[2] = 'O';
}

void MyThread::dowork()
{
    Pos ret = Pos(20, 20, 0, 1+order);
    QVector<Pos> path;
    if(isStop == false){
        if(limit > 0)
        {
            t2.start();
            if(algoFlag == 0){
                killSearch(ret, hold, depth, -2*R_INFINTETY,2*R_INFINTETY, path);
            }else if(algoFlag == 1){
                deepSearch(ret, hold, hold, depth, -2*R_INFINTETY, 2*R_INFINTETY, path);
            }
        }
        QString temp;
        temp.sprintf("%d,%d,%d,%d,%d,",5 ,ret.x, ret.y, ret.value, ret.a3);
        if(runing) temp += "1";
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

void MyThread::showChess()
{
    if(!openlog) return;
    for (int i = 0; i < 15; i++)
       qDebug("%2c %2c %2c %2c %2c %2c %2c %2c %2c %2c %2c %2c %2c %2c %2c",
              strTab[chess[0][i]], strTab[chess[1][i]], strTab[chess[2][i]], strTab[chess[3][i]], strTab[chess[4][i]],
              strTab[chess[5][i]], strTab[chess[6][i]], strTab[chess[7][i]], strTab[chess[8][i]], strTab[chess[9][i]],
              strTab[chess[10][i]], strTab[chess[11][i]], strTab[chess[12][i]], strTab[chess[13][i]], strTab[chess[14][i]]);
    qDebug();
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
            if(b[i] + b[i+4] == 2 )           { sleep_three++;}    // 眠三
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
            if (b[i] + b[i+4] <= 1 && b[i] + b[i+4] >= 5)  { sleep_two++; }       // 眠二

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
        score = Max(score, 100);

    if (four >= 1 && jump+three >= 1)
        score = Max(score, 80);

    if (jump+three >= 2)
        score = Max(score, 50);

    score = Max(score, sleep_three + sleep_jump + 2*jump + 2*two + 2*three);
    score = Max(score, 3/(1+abs(x-7)+abs(y-7)));

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
    int i, j, val, p1, p2, k, three=0;
    int hashf = HASH_ALPHA;
    long long hashIndex, hashBest;
    Pos newMove;
    QVector<Pos> attackQueue, vec_moves;

    if(t2.elapsed() > limit){
        if(runing) runing = false;
        return alpha;
    }

    if(lookup(deep+order, alpha, beta, val))
        return val;

    if (deep <= 0){
        val = -evaluate(3-key);
        store(mutex, 1+order, val, HASH_EXACT, hash);
        return val;
    }

    for (i = 0; i < 15; i++){
        for (j = 0; j < 15; j++){
            if(chess[i][j] == 0 && vis[2][i][j] > 0){
                // 算杀的结点选择
                // 进攻方：活三、冲四、活四、五连、防守对方的冲四
                // 防守方：防守对方的活三、冲四，自身的冲四
                k = 0.2*valueChess(i, j, key, &p1) + 0.1*valueChess(i, j, 3-key, &p2);
                if(origin == key){
                    // 进攻方选点
                    if(p1 > 0 || p2 >= 10000){
                        attackQueue.push_back(Pos{i, j, p1, p1});
                    }
                }else{
                    // 防守方选点
                    if(p2 > 0 || p1 >= 100){
                        attackQueue.push_back(Pos{i, j, p2, p1});
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

        if (k <= -R_INFINTETY) continue;
        newMove.value += k;

        vec_moves.push_back(newMove);
    }

    // 如果进攻方没找到结点，情况如下：
    // 1.局面没有进攻结点了，表明进攻失败，应该返回alpha。
    // 2.进攻过程中遭到反击，如反活三，说明进攻不能成功，应该返回alpha。
    // 如果防守方没找到结点，情况如下：
    // 1.无法阻挡攻势，应该返回-INF。

    // 第一层搜索无活三
    if(origin == key && three == 0 && 1+order == ret.a1)
        return alpha;
    // 进攻方无棋
    if(origin == key && vec_moves.isEmpty())
        return alpha;
    // 防守方无棋
    if(origin == 3-key && vec_moves.isEmpty())
        return -R_INFINTETY;

    qSort(vec_moves.begin(), vec_moves.end(), greater<Pos>());

    if(origin == key && vec_moves[0].a1 >= 10000){
        update(mutex, ret, vec_moves[0], key, ret.a1, R_INFINTETY);
        return R_INFINTETY;
    }

    for(auto move: vec_moves){

        powerOperation(move.x, move.y, FLAGS_POWER_CONDESE, key);
        path.push_back(move);

        if(move == vec_moves[0]){
            val = - deepSearch(ret, origin, EXCHANGE - key, deep - 1, -beta, -alpha, path);
        }
        else{
            val = - deepSearch(ret, origin, EXCHANGE - key, deep - 1, -alpha-1, -alpha, path);
            if(alpha < val && val < beta){
                val = - deepSearch(ret, origin, EXCHANGE - key, deep - 1, -beta, -alpha, path);
            }
        }

        hashIndex = hash;
        path.pop_back();
        powerOperation(move.x, move.y, FLAGS_POWER_RELEASE, key);

        // 剪枝
        if (val >= beta){
            ABcut++;
            store(mutex, deep+order, beta, HASH_BETA, hashIndex);
            return beta;
        }

        if(val > alpha){
            alpha = val;
            hashf = HASH_EXACT;
            hashBest = hashIndex;
            update(mutex, ret, move, key, 1+order, val);
        }


    }
    store(mutex, deep+order, alpha, hashf, hashBest);
    return alpha;

}

int MyThread::killSearch(Pos& ret, int key, int deep, int alpha, int beta, QVector<Pos>& path)
{
    int i, j, val, p1, p2, k;
    int hashf = HASH_ALPHA;
    long long hashIndex, hashBest;
    Pos newMove;
    QVector<Pos> attackQueue, vec_moves;

    if(t2.elapsed() > limit){
        if(runing) runing = false;
        return alpha;
    }

    if(lookup(deep+order, alpha, beta, val))
        return val;

    if (deep <= 0){
        val = -evaluate(3-key);
        store(mutex, 1+order, val, HASH_EXACT, hash);
        return val;
    }

    for (i = 0; i < 15; i++){
        for (j = 0; j < 15; j++){
            if(chess[i][j] == 0 && vis[2][i][j] >= 1){
                k = 0.2*valueChess(i, j, key, &p1) + 0.1*valueChess(i, j, 3-key, &p2);
                if(p1 + p2 > 0){
                    attackQueue.push_back(Pos{i, j, p1, p1});
                }else{
                    attackQueue.push_front(Pos{i, j, p1, p1});
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
        powerOperation(newMove.x, newMove.y, FLAGS_POWER_CONDESE, key);
        k = evaluate(key);
        powerOperation(newMove.x, newMove.y, FLAGS_POWER_RELEASE, key);

        if (k <= -R_INFINTETY) continue;
        newMove.value += k;
        if(vec_moves.size() > rangenum)
            vec_moves.pop_back();
        vec_moves.push_back(newMove);
        qSort(vec_moves.begin(), vec_moves.end(), greater<Pos>());
    }

    if(vec_moves.isEmpty())
        return alpha;

//    qSort(vec_moves.begin(), vec_moves.end(), greater<Pos>());

    for(auto move: vec_moves){

        powerOperation(move.x, move.y, FLAGS_POWER_CONDESE, key);
        path.push_back(move);

        if(move == vec_moves[0]){
            val = - killSearch(ret, EXCHANGE - key, deep - 1, -beta, -alpha, path);
        }
        else{
            val = - killSearch(ret, EXCHANGE - key, deep - 1, -alpha-1, -alpha, path);
            if(alpha < val && val < beta){
                val = - killSearch(ret, EXCHANGE - key, deep - 1, -beta, -alpha, path);
            }
        }


        hashIndex = hash;
        path.pop_back();
        powerOperation(move.x, move.y, FLAGS_POWER_RELEASE, key);

        // 剪枝

        if (val >= beta){
            ABcut++;
            store(mutex, deep+order, beta, HASH_BETA, hashIndex);
            return beta;
        }

        if(val > alpha){
            alpha = val;
            hashf = HASH_EXACT;
            hashBest = hashIndex;
            update(mutex, ret, move, key, 1+order, val);
        }


    }
    store(mutex, deep+order, alpha, hashf, hashBest);
    return alpha;
}

void MyThread::update(QMutex& m, Pos& ret, const Pos ref, int key, int order, int val){
    m.lock();
    if(order == ret.a1 && (val >= ret.value || ret.a2 == 0)){
        if(openlog){
            qDebug("(%d,%d,%d)->(%d,%d,%d) alpha", ret.x, ret.y, ret.value, ref.x, ref.y, val);
            powerOperation(ref.x, ref.y, FLAGS_POWER_CONDESE, key);
            showChess();
            powerOperation(ref.x, ref.y, FLAGS_POWER_RELEASE, key);
        }
        ret.x = ref.x;
        ret.y = ref.y;
        ret.value = val;
        ret.a2++;
        ret.a3 = ref.a3;
    }
    m.unlock();
}

int MyThread::evaluate(int key)
{
    int i, j, p;
    int o_prior=0, d_prior=0, o_val=0, d_val=0;
    for (i = 0; i < 15; i++)
        for (j = 0; j < 15; j++){
            if(chess[i][j] == 3-key)
            {
                o_val += valueChess(i, j, 3-key, &p);
                o_prior += p;

            }
            if(chess[i][j] == key){
                d_val += valueChess(i, j, key, &p);
                d_prior += p;
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

    if(d_prior>=1)  d_val *=1.5;
    if(o_prior>=1)  o_val *=1.5;
    return d_val - 1.5*o_val;
}

void MyThread::powerOperation(int x, int y, int flag, int key)
{
    int i, j, k = key, dx, dy;

    if (flag == FLAGS_POWER_CONDESE){
        order++;
        hash ^= Z[x][y][k];
        chess[x][y] = key;
        for(auto dis = 1; dis <= Kernel; dis++)for(i = 0; i < 8; i++){
            for(j = 1; j <= dis; j++){
                dx = x+vx[i]*j;
                dy = y+vy[i]*j;
                if(inside(dx, dy)){
                    vis[dis][dx][dy]++;
                }
            }
        }
    }
    else{
        order--;
        hash ^= Z[x][y][k];
        chess[x][y] = 0;
        for(auto dis = 1; dis <= Kernel; dis++)for(i = 0; i < 8; i++){
            for(j = 1; j <= dis; j++){
                dx = x+vx[i]*j;
                dy = y+vy[i]*j;
                if(inside(dx, dy)){
                    vis[dis][dx][dy]--;
                }
            }
        }
    }
}
