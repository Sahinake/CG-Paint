#ifndef LDEP_H
#define LDEP_H
#include "Structures.h"

#define SELECTION_RADIUS 0.1f // Raio de Seleção
#define INSIDE 0    //0000
#define LEFT 1      //0001
#define RIGHT 2     //0010
#define BOTTOM 4    //0100
#define TOP 8       //1000

int isCloseEnough(Point a, Point b) {
    // Limite de proximidade
    float threshold = 0.05;
    float dx = a.x - b.x;
    float dy = a.y - b.y;
    return (dx * dx + dy * dy) <= (threshold * threshold);
}

// Função que determina a região
int computeRegionCode(float x, float y, float mx, float my) {
    int code = INSIDE;

    if(x < mx - SELECTION_RADIUS) {
        code |= LEFT;
    }
    else if(x > mx + SELECTION_RADIUS) {
        code |= RIGHT;
    }
    if(y < my - SELECTION_RADIUS) {
        code |= BOTTOM;
    }
    else if(y > my + SELECTION_RADIUS) {
        code |= TOP;
    }
    return code;
}

int pickPoint(float px, float py, float mx, float my, int t) {
    if(mx <= px + t && mx >= px - t) {
        if(my <= py + t && my >= py - t) {
            return 1;
        }
    }
    return 0;
}

int pickLine(float x1, float y1, float x2, float y2, float mx, float my) {
    int code1 = computeRegionCode(x1, y1, mx, my);
    int code2 = computeRegionCode(x2, y2, mx, my);

    while(1) {
        if(!(code1 | code2)) {
            // Os dois pontos estão dentro da área de seleção
            return 1;
        }
        else if(code1 & code2) {
            // Os dois pontos estão fora da área de seleção
            return 0;
        }
        else {
            // Um dos pontos estão fora da área de seleção
            int code_out = code1 ? code1 : code2;
            float x, y;

            if(code_out & TOP) {
                x = x1 + (x2 - x1) * (my + SELECTION_RADIUS - y1) / (y2 - y1);
                y = my + SELECTION_RADIUS;
            }
            else if(code_out & BOTTOM) {
                x = x1 + (x2 - x1) * (my - SELECTION_RADIUS - y1) / (y2 - y1);
                y = my - SELECTION_RADIUS;
            }
            else if(code_out & RIGHT) {
                y = y1 + (y2 - y1) * (mx + SELECTION_RADIUS - x1) / (x2 - x1);
                x = mx + SELECTION_RADIUS;
            }
            else if(code_out & LEFT) {
                y = y1 + (y2 - y1) * (mx - SELECTION_RADIUS - x1) / (x2 - x1);
                x = mx - SELECTION_RADIUS;
            }

            // Substituir o ponto fora da linha
            if(code_out == code1) {
                x1 = x;
                y1 = y;
                code1 = computeRegionCode(x1, y1, mx, my);
            }
            else {
                x2 = x;
                y2 = x;
                code2 = computeRegionCode(x2, y2, mx, my);
            }
        }
    }
}

#endif
