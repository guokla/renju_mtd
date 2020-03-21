#ifndef AREA_H
#define AREA_H

#include <QPoint>

class Area{

public:
    int Min_x;
    int Min_y;
    int Max_x;
    int Max_y;
    float dx, dy;
    Area(int x1=0, int y1=0, int x2=0, int y2=0);
    ~Area();
    bool Among(int px, int py);
};



#endif // AREA_H
