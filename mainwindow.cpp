#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setMaximumSize(WIDTH_W,HEIGH_W);
    this->setMinimumSize(WIDTH_W,HEIGH_W);
    memset(chess, 0, sizeof(chess));
    memset(vis, 0, sizeof(vis));
    memset(rx, 0, sizeof(rx));
    memset(ry, 0, sizeof(ry));
    memset(valTab, 0, sizeof(valTab));
    memset(priorTab, 0, sizeof(priorTab));
    memset(isdraw, false, sizeof(isdraw));
//    H[0] = new HASHITEM[HASH_TABLE_SIZE]();
//    H[1] = new HASHITEM[HASH_TABLE_SIZE]();
//    H[2] = new HASHITEM[HASH_TABLE_SIZE]();

    strTab[0] = ' ';
    strTab[1] = 'M';
    strTab[2] = 'O';

    Pix_board = QPixmap("myImage/chessboard.png");
    Pix = Pix_board;

    centraTimer = QObject::startTimer(300);

    this->setWindowTitle(tr("Renju Game : Freestyle"));
    ui->statusBar->showMessage(tr("Welcome to play Renju Game : Freestyle"));

    area = new Area(48, 67, 630, 650);

    qsrand(1234567891011);

    for(int i = 0; i < N; i++)
        for(int j = 0; j < N; j++){
            Z[i][j][1] = rand64();
            Z[i][j][2] = rand64();
        }
}

MainWindow::~MainWindow()
{
    delete H[0];
    delete H[1];
    delete H[2];
    thread.quit();
    thread.wait();
    delete ui;
}

void MainWindow::mousePressEvent(QMouseEvent *e){

    int p1;
    cur_x = e->x();
    cur_y = e->y();

    if (area->Among(cur_x, cur_y) && !runing){

        px = (cur_x - area->Min_x) / area->dx + 0.5;
        py = (cur_y - area->Min_y) / area->dy + 0.5;

        if (chess[px][py] == 0){
            valueChess(px, py, hold, &p1);
            Pos move = {px, py, 0, p1};
            moveQueue.push_back(move);
            powerOperation(px, py, FLAGS_POWER_CONDESE, hold);

            checkWinner(px, py, true);
            showChess();

            hold = EXCHANGE - hold;
            ui->statusBar->showMessage(tr("(%1, %2, %3)").arg(px).arg(py).arg(evaluate(3-hold)), 5000);
        }
    }
    else ui->statusBar->showMessage(tr("超出棋盘区域"), 5000);
}

void MainWindow::timerEvent(QTimerEvent *){

    if (hold == BLACK)
        ui->textEdit_2->setText(tr("轮到黑棋走"));
    else
        ui->textEdit_2->setText(tr("轮到白棋走"));
    if(ui->textEdit->toPlainText() != buffer){
        ui->textEdit->setText(buffer);
        QTextCursor cursor = ui->textEdit->textCursor();
        cursor.movePosition(QTextCursor::End);
        ui->textEdit->setTextCursor(cursor);
    }

    if (change && !stop) {
        Pix = Pix_board;
        memset(isdraw, false, sizeof(isdraw));
        change = false;
    }
    qsrand(QTime(0,0,0).secsTo(QTime::currentTime()));
    drawAll();
}

void MainWindow::drawAll(){
    for (int i = 0; i < N ; i++)
        for (int j = 0; j < N ; j++)
        {
            if (!isdraw[i][j] && chess[i][j] == BLACK){
                drawChess(i, j, QColor(0, 0, 0));
                isdraw[i][j] = true;
            }
            if (!isdraw[i][j] && chess[i][j] == WHITE){
                drawChess(i, j, QColor(255, 255, 255));
                isdraw[i][j] = true;
            }
        }
}

void MainWindow::paintEvent(QPaintEvent *){
    if (!stop){
        centerpainter.begin(this);
        centerpainter.drawPixmap(20, 40, WIDTH_P, HEIGH_P, Pix);
        centerpainter.end();
    }
}

void MainWindow::drawChess(int px, int py, QColor color){
    int dx = int(px) * area->dx + area->Min_x - 20;
    int dy = int(py) * area->dy + area->Min_y - 40;
    QPainter painter(&Pix);
    QBrush brush(color);
    QPen pen(color);
    QRectF rect(QPoint(dx-14, dy-15), QPoint(dx+16, dy+15));
    painter.setPen(pen);
    painter.setBrush(brush);
    painter.drawEllipse(rect);
    painter.end();
    this->repaint();
}

void MainWindow::on_action_Back_triggered()
{
    Pos move;
    if (!moveQueue.isEmpty() && !runing) {
        change = true;
        move = moveQueue.last();
        moveQueue.pop_back();
        powerOperation(move.x, move.y, FLAGS_POWER_RELEASE, EXCHANGE - hold);

        hold = EXCHANGE - hold;
    }
    else ui->statusBar->showMessage(tr("已经是第一手棋！"), 5000);
}

void MainWindow::on_action_Help_triggered()
{
    int x=20, y=20, p1;

    ui->statusBar->showMessage(tr("启用提示"), 5000);
    getPosition(x, y, hold, 1);

    if(inside(x, y) && order < 3 && runing==false){
        valueChess(x, y, hold, &p1);
        Pos move = {x, y, 0, p1};
        moveQueue.push_back(move);
        powerOperation(x, y, FLAGS_POWER_CONDESE, hold);
        checkWinner(x, y, true);
        hold = EXCHANGE - hold;
    }
}

void MainWindow::getPosition(int &x, int &y, int key, int flag)
{
    if(flag > 2){
        int p1, p2;
        QVector<Pos> moves;
        for(int i = 0; i < N; i++)for(int j = 0; j < N; j++){
            if(chess[i][j] == 0 && vis[2][i][j] > 0){
                Pos newMove(i, j);
                powerOperation(i, j, FLAGS_POWER_CONDESE, key);
                newMove.value = 0.5*valueChess(i, j, key, &p1) + 0.5*valueChess(i, j, 3-key, &p2) + evaluate(key);
                powerOperation(i, j, FLAGS_POWER_RELEASE, key);
                moves.push_back(newMove);
            }
        }
        qSort(moves.begin(), moves.end(), greater<Pos>());
        x = moves.front().x;
        y = moves.front().y;
        return;
    }


    if(order == 0){
        x = N / 2;
        y = N / 2;
    }
    else if(order == 1 || order == 2){
        do{
            x = moveQueue.first().x + qrand() % (2*order+1) - order;
            y = moveQueue.first().y + qrand() % (2*order+1) - order;
        }while(x < 0 || x >= 15 || y < 0 || y >= 15 || chess[x][y] != 0);
    }else{
        if(!runing){
            depth = init_depth;
            algoFlag = 1;
            t2.start();
            result.clear();
            distribution(key, limit_kill);
        }else{
            qDebug() << "wrong operation";
        }
    }
}

void MainWindow::powerOperation(int x, int y, int flag, int key)
{
    int i, j, k, dx, dy, p;

    if (flag == FLAGS_POWER_CONDESE){
        order++;
        hash ^= Z[x][y][key];
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
        // 更新权值
        valTab[x][y][key] = valueChess(x, y, key, &p);
        priorTab[x][y][key] = p;
        for(i=0, j=1; i<8; i++, j=1){
            dx=x+vx[i];dy=y+vy[i];
            for(;j<=4 && inside(dx, dy); ++j, dx=x+vx[i]*j, dy=y+vy[i]*j){
                if(chess[dx][dy] > 0){
                    valTab[dx][dy][chess[dx][dy]] = valueChess(dx, dy, chess[dx][dy], &p);
                    priorTab[dx][dy][chess[dx][dy]] = p;
                }
            }
        }
    }
    else{
        order--;
        hash ^= Z[x][y][key];
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
        valTab[x][y][key] = 0;
        priorTab[x][y][key] = 0;
        for(i=0, j=1; i<8; i++, j=1){
            dx=x+vx[i];dy=y+vy[i];
            for(;j<=4 && inside(dx, dy); ++j, dx=x+vx[i]*j, dy=y+vy[i]*j){
                if(chess[dx][dy] > 0){
                    valTab[dx][dy][chess[dx][dy]] = valueChess(dx, dy, chess[dx][dy], &p);
                    priorTab[dx][dy][chess[dx][dy]] = p;
                }
            }
        }
    }
}

bool inline MainWindow::inside(int x, int y){
    if (x < 15 && y < 15 && x >= 0 && y >= 0)
        return true;
    return false;
}

bool inline MainWindow::inside(Pos move){
    if (move.x < 15 && move.y < 15 && move.x >= 0 && move.y >= 0)
        return true;
    return false;
}

int MainWindow::valueChess(int x, int y, int key, int *piority){
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

int MainWindow::evaluate(int key)
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

void MainWindow::showChess()
{
    if(!openlog) return;
    for (int i = 0; i < 15; i++)
       qDebug("%2d %2d %2d %2d %2d %2d %2d %2d %2d %2d %2d %2d %2d %2d %2d",
              valTab[0][i][hold] , valTab[1][i][hold] , valTab[2][i][hold] , valTab[3][i][hold] , valTab[4][i][hold] ,
              valTab[5][i][hold] , valTab[6][i][hold] , valTab[7][i][hold] , valTab[8][i][hold] , valTab[9][i][hold] ,
              valTab[10][i][hold], valTab[11][i][hold], valTab[12][i][hold], valTab[13][i][hold], valTab[14][i][hold]);

//    for (int i = 0; i < 15; i++)
//       qDebug("%2c %2c %2c %2c %2c %2c %2c %2c %2c %2c %2c %2c %2c %2c %2c\n",
//              strTab[chess[0][i]], strTab[chess[1][i]], strTab[chess[2][i]], strTab[chess[3][i]], strTab[chess[4][i]],
//              strTab[chess[5][i]], strTab[chess[6][i]], strTab[chess[7][i]], strTab[chess[8][i]], strTab[chess[9][i]],
//              strTab[chess[10][i]], strTab[chess[11][i]], strTab[chess[12][i]], strTab[chess[13][i]], strTab[chess[14][i]]);
    qDebug("\n");
}

bool MainWindow::distribution(int key, int time)
{
    runing = true;
    MyThread *myt = new MyThread();
    myt->moveToThread(&thread);
    myt->initial(H[lock_algo], Z, hash, chess, vis, key, time, depth, algoFlag, openlog, order, valTab, priorTab);
    myt->connect(&thread, &QThread::finished, myt, &QObject::deleteLater); // Mythread与QThread关联
    myt->connect(myt, &MyThread::resultReady, this, &MainWindow::dealSignal);
    myt->connect(this, &MainWindow::startThread, myt, &MyThread::dowork);
    thread.start();
    emit startThread(QString::number(guess));
    return true;
}

int MainWindow::checkWinner(int x, int y, bool endFlag){

    int judge = NONE;
    int i, j;
    int pawns_black[8];
    int pawns_white[8];
    memset(pawns_black, 0, sizeof(pawns_black));
    memset(pawns_white, 0, sizeof(pawns_white));

    for (i = 0, j = 0; i < 8; i++){
        for(j = 0; j <= 5 && chess[x+vx[i]*j][y+vy[i]*j] == BLACK && inside(x+vx[i]*j, y+vy[i]*j); j++, pawns_black[i]++);
        for(j = 0; j <= 5 && chess[x+vx[i]*j][y+vy[i]*j] == WHITE && inside(x+vx[i]*j, y+vy[i]*j); j++, pawns_white[i]++);
    }
    for (i = 0; i < 4; i++){
        if (pawns_black[i] + pawns_black[i+4] >= 6) judge = BLACK;
        if (pawns_white[i] + pawns_white[i+4] >= 6) judge = WHITE;
    }

    if (judge > NONE && endFlag){
        this->repaint();
        if (judge == BLACK)
            QMessageBox::warning(this, tr("提示"), tr("黑棋已取得胜利！"));
        else if(judge == WHITE)
            QMessageBox::warning(this, tr("提示"), tr("白棋已取得胜利！"));
        qApp->quit();
    }
    return judge;
}

unsigned long long MainWindow::rand64(){
    return qrand() ^ (qrand() << 15) ^ (qrand() << 30) ^ (qrand() << 45);
}

void MainWindow::dealSignal(const QString &str)
{
    int recive[10], i, j = str.section(',',0, 0).toLong();
    thread.quit();
    thread.wait();
    for(i = 0; i < j; i++)
        recive[i]=str.section(',', i+1, i+1).toLong();
    if(openlog){
        qDebug() << str;
        qDebug() << "result=" << result.x << result.y << result.value << result.a1;
    }
    Pos newMove(recive[0], recive[1], recive[2], recive[3]);

    // 简单判定，如果是必应着法直接落子
    valueChess(newMove.x, newMove.y, 3-hold, &newMove.a3);
    valueChess(newMove.x, newMove.y, hold, &newMove.a1);
    if(newMove.a3 >= 10000 || newMove.a1 >= 10000){
        moveQueue.push_back(newMove);
        powerOperation(newMove.x, newMove.y, FLAGS_POWER_CONDESE, hold);
        checkWinner(newMove.x, newMove.y, true);
        hold = EXCHANGE - hold;
        runing = false;
        QString temp;
        temp.sprintf("[绝对先手: %2d,%2d]\n\n", newMove.x, newMove.y);
        buffer += temp;
        return;
    }

    // running标志
    const int judge = recive[4];
    callFunction(newMove, 0, judge);
}

void MainWindow::callFunction(Pos& newMove, int flag, const int& judge){

    if(algoFlag == 1)
    {
        // 算杀
        if(judge == 1 && inside(newMove))
        {
            result = newMove;
            QString temp;
            temp.sprintf("[kill: 深度%d,%2d,%2d] = %3d, time: %.3f s\n", depth, newMove.x, newMove.y, newMove.value, t2.elapsed()/1000.0);
            buffer += temp;
            if(newMove.value < R_INFINTETY){
                depth += delta;
                distribution(hold, limit_kill - t2.elapsed());
            }
        }
        if(judge == 0 || !inside(newMove) || newMove.value >= R_INFINTETY)
        {
            QString temp;
            temp.sprintf("[kill: 搜索结束]\n\n");
            buffer += temp;
            if(inside(result) && result.value >= R_INFINTETY)
            {
                if(depth<=2)
                    getPosition(result.x, result.y, hold, 3);
                moveQueue.push_back(result);
                powerOperation(result.x, result.y, FLAGS_POWER_CONDESE, hold);
                checkWinner(result.x, result.y, true);
                hold = EXCHANGE - hold;
                runing = false;
                result.clear();
                return;
            }else{
                depth = init_depth;
                algoFlag = lock_algo;
                result.clear();
                t2.start();
                runing = true;
                distribution(hold, limit);
            }
        }
    }else if(algoFlag == 0){
        // PVS
        if(judge == 1 && inside(newMove))
        {
            result = newMove;
            QString temp;
            temp.sprintf("[PVS: 深度%d,%2d,%2d] = %3d, time: %.3f s\n", depth, newMove.x, newMove.y, newMove.value, t2.elapsed()/1000.0);
            buffer += temp;
            if(newMove.value < R_INFINTETY){
                depth += delta;
                distribution(hold, limit - t2.elapsed());
            }
        }
        if(judge == 0 || !inside(newMove) || result.value >= R_INFINTETY)
        {
            runing = false;
            if(inside(result))
            {
                if(depth<=2)
                    getPosition(result.x, result.y, hold, 3);
                moveQueue.push_back(result);
                powerOperation(result.x, result.y, FLAGS_POWER_CONDESE, hold);
                checkWinner(result.x, result.y, true);
                hold = EXCHANGE - hold;
                QString temp;
                temp.sprintf("[PVS: 深度%d] = timeout, time: %.3f s\n\n", depth, t2.elapsed()/1000.0);
                buffer += temp;
                result.clear();
                return;
            }else{
                getPosition(result.x, result.y, hold, 3);
                if(inside(result))
                {
                    moveQueue.push_back(result);
                    powerOperation(result.x, result.y, FLAGS_POWER_CONDESE, hold);
                    checkWinner(result.x, result.y, true);
                    hold = EXCHANGE - hold;
                    QString temp;
                    temp.sprintf("[PVS: 结束,%2d,%2d]\n\n", result.x, result.y);
                    buffer += temp;
                }
            }
        }
    }else if(algoFlag == 2){
            // MTD
            if(judge == 1 && inside(newMove))
            {
                result = newMove;
                QString temp;
                temp.sprintf("[MTD: 深度%d,%2d,%2d] = %3d, time: %.3f s\n", depth, newMove.x, newMove.y, newMove.value, t2.elapsed()/1000.0);
                buffer += temp;
                guess = newMove.value;
                if(newMove.value < R_INFINTETY){
                    depth += delta;
                    distribution(hold, limit - t2.elapsed());
                }
            }
            if(judge == 0 || !inside(newMove) || result.value >= R_INFINTETY)
            {
                runing = false;
                if(inside(result))
                {
                    if(depth<=2)
                        getPosition(result.x, result.y, hold, 3);
                    moveQueue.push_back(result);
                    powerOperation(result.x, result.y, FLAGS_POWER_CONDESE, hold);
                    checkWinner(result.x, result.y, true);
                    hold = EXCHANGE - hold;
                    QString temp;
                    temp.sprintf("[MTD: 深度%d] = timeout, time: %.3f s\n\n", depth, t2.elapsed()/1000.0);
                    buffer += temp;
                    result.clear();
                    return;
                }else{
                    getPosition(result.x, result.y, hold, 3);
                    if(inside(result))
                    {
                        moveQueue.push_back(result);
                        powerOperation(result.x, result.y, FLAGS_POWER_CONDESE, hold);
                        checkWinner(result.x, result.y, true);
                        hold = EXCHANGE - hold;
                        QString temp;
                        temp.sprintf("[MTD: 结束,%2d,%2d]\n\n", result.x, result.y);
                        buffer += temp;
                    }
                }
            }
    }
}

void MainWindow::update(QMutex& m, Pos& ret, const Pos ref, int key, int order, int val){
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
    }
    m.unlock();
}

void MainWindow::on_radioButton_clicked()
{
    if(runing == false){
        algoFlag = 0;
        lock_algo = 0;
        ui->radioButton_2->clearFocus();
        ui->statusBar->showMessage(tr("采用PVS算法"), 0);
    }else{
        ui->statusBar->showMessage(tr("运算中"), 0);
        if(lock_algo == 0) ui->radioButton_2->clearFocus();
        if(lock_algo == 2) ui->radioButton->clearFocus();
    }
}

void MainWindow::on_radioButton_2_clicked()
{
    if(runing==false){
        algoFlag = 2;
        lock_algo = 2;
        ui->radioButton->clearFocus();
        ui->statusBar->showMessage(tr("采用MTD-f算法"), 0);
    }else{
        ui->statusBar->showMessage(tr("运算中"), 0);
        if(lock_algo == 0) ui->radioButton_2->clearFocus();
        if(lock_algo == 2) ui->radioButton->clearFocus();
    }
}

void MainWindow::on_radioButton_9_clicked()
{
    if(openlog){
        openlog = false;
        ui->statusBar->showMessage(tr("关闭日志"), 0);
    }else{
        openlog = true;
        ui->statusBar->showMessage(tr("开启日志"), 0);
    }
}

void MainWindow::on_action_Version_triggered()
{
    QMessageBox::information(this, "Renju", "version: 3.1\nanthor: guokla", tr("确定"));
}

void MainWindow::on_pushButton_clicked()
{
    buffer.clear();
}

void MainWindow::on_lineEdit_textChanged(const QString &arg1)
{
    bool changeSuccess;
    limit_kill = arg1.toInt(&changeSuccess);
    if(!changeSuccess){
        limit_kill = 0;
        ui->lineEdit->setText("0");
    }
}

void MainWindow::on_lineEdit_2_textChanged(const QString &arg1)
{
    bool changeSuccess;
    limit = arg1.toInt(&changeSuccess);
    if(!changeSuccess){
        limit = 0;
        ui->lineEdit_2->setText("0");
    }
}
