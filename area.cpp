#include <mainwindow.h>

Area::Area(int x1, int y1, int x2, int y2){
    Min_x = Min(x1, x2);
    Max_x = Max(x1, x2);
    Min_y = Min(y1, y2);
    Max_y = Max(y1, y2);
    dx = float(Max_x - Min_x) / (N - 1);
    dy = float(Max_y - Min_y) / (N - 1);
}

Area::~Area(){}

bool Area::Among(const int px, const int py){
    if (px <= Max_x + SIZE && px >= Min_x - SIZE && py <= Max_y + SIZE && py >= Min_y - SIZE)
        return true;
    return false;
}

