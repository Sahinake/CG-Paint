#ifndef LDEP_H
#define LDEP_H
#include "Structures.h"

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
int computeRegionCode(float x, float y, float xmin, float ymin, float xmax, float ymax) {
    int code = INSIDE;

    if(x < xmin) {
        code |= LEFT;
    }
    else if(x > xmax) {
        code |= RIGHT;
    }
    if(y < ymin) {
        code |= BOTTOM;
    }
    else if(y > ymax) {
        code |= TOP;
    }
    return code;
}

int pickPoint(Point p, Point clicked_point, float tolerancy) {
    // Definir a área de tolerãncia ao redor do ponto clicado
    float xmin = clicked_point.x - tolerancy;
    float xmax = clicked_point.x + tolerancy;
    float ymin = clicked_point.y - tolerancy;
    float ymax = clicked_point.y + tolerancy;

    if(p.x >= xmin && p.x <= xmax) {
        if(p.y >= ymin && p.y <= ymax) {
            return 1;
        }
    }
    return 0;
}

int pickLine(Line line, Point clicked_point, float tolerancy) {
    // Definir a área de tolerãncia ao redor do ponto clicado
    float xmin = clicked_point.x - tolerancy;
    float xmax = clicked_point.x + tolerancy;
    float ymin = clicked_point.y - tolerancy;
    float ymax = clicked_point.y + tolerancy;

    // Coordenadas da linha
    float x0 = line.start_line.x;
    float y0 = line.start_line.y;
    float x1 = line.end_line.x;
    float y1 = line.end_line.y;

    int outcode0 = computeRegionCode(x0, y0, xmin, ymin, xmax, ymax);
    int outcode1 = computeRegionCode(x1, y1, xmin, ymin, xmax, ymax);

    while(1) {
        if((outcode0 | outcode1) == 0) {
            return 1;
        }
        else if((outcode0 & outcode1) != 0) {
            return 0;
        }
        else {
            float x, y;
            int outcode = outcode0 ? outcode0 : outcode1;

            if(outcode & TOP) {
                x = x0 + (x1 - x0) * (ymax - y0) / (y1 -y0);
                y = ymax;
            }
            else if(outcode & BOTTOM) {
                x = x0 + (x1 - x0) * (ymin - y0) / (y1 - y0);
                y = ymin;
            }
            else if(outcode & RIGHT) {
                y = y0 + (y1 - y0) * (xmax - x0) / (x1 - x0);
                x = xmax;
            }
            else if(outcode & LEFT) {
                y = y0 + (y1 - y0) *(xmin - x0) / (x1 - x0);
                x = xmin;
            }

            if(outcode == outcode0) {
                x0 = x;
                y0 = y;
                outcode0 = computeRegionCode(x0, y0, xmin, ymin, xmax, ymax);
            }
            else {
                x1 = x;
                y1 = y;
                outcode1 = computeRegionCode(x1, y1, xmin, ymin, xmax, ymax);
            }
        }
    }

    return 0;
}

int pickPolygon(Polygon poly, Point clicked_point) {
    int intersections = 0;
    int i, j;

    // Percorre todos os vértices do polígono
    for(i = 0, j = poly.num_vertices - 1; i < poly.num_vertices; j = i++) {
        // Verifica se o tiro horizontal cruza a aresta (poly.vertices[i], poly.vertices[j])
        if(((poly.vertices[i].y > clicked_point.y) != (poly.vertices[j].y > clicked_point.y)) &&
        (clicked_point.x < (poly.vertices[j].x - poly.vertices[i].x) * (clicked_point.y - poly.vertices[i].y) / (poly.vertices[j].y - poly.vertices[i].y) + poly.vertices[i].x)) {
            intersections++;
        }
    }

    // Se o número de interseções for ímpar, o ponto está dentro do polígono
    return (intersections % 2) != 0;
}

#endif
