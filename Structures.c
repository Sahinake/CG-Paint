#ifndef LDEP_H
#define LDEP_H
#include "Structures.h"

int isCloseEnough(Point a, Point b) {
    // Limite de proximidade
    float threshold = 0.05;
    float dx = a.x - b.x;
    float dy = a.y - b.y;
    return (dx * dx + dy * dy) <= (threshold * threshold);
}

#endif
