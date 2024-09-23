#ifndef LDEP_H
#define LDEP_H
#include "Structures.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

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

// Função para multiplicar uma matriz 3x3 por um ponto
Point applyTransformation(float matrix[3][3], Point p) {
    Point result;
    result.x = matrix[0][0] * p.x + matrix[0][1] * p.y + matrix[0][2] * 1;
    result.y = matrix[1][0] * p.x + matrix[1][1] * p.y + matrix[1][2] * 1;
    return result;
}

Point getObjectCenter(Object *obj) {
    Point center = {0, 0};

    switch(obj->type) {
        case POINT:
            center = obj->objectData.point; // O centro é o próprio ponto;
            break;
        case LINE:
            center.x = (obj->objectData.line.start_line.x + obj->objectData.line.end_line.x) / 2;
            center.y = (obj->objectData.line.start_line.y + obj->objectData.line.end_line.y) / 2;
            break;
        case POLYGON:
            float sum_x = 0, sum_y = 0;
            for(int i = 0; i < obj->objectData.polygon.num_vertices; i++) {
                sum_x += obj->objectData.polygon.vertices[i].x;
                sum_y += obj->objectData.polygon.vertices[i].y;
            }
            center.x = sum_x / obj->objectData.polygon.num_vertices;
            center.y = sum_y / obj->objectData.polygon.num_vertices;
            break;
    }
    return center;
}

void translateObject(Object *obj, float tx, float ty) {
    if(obj == NULL) return;

    float translation_matrix[3][3] = {
        {1, 0, tx},
        {0, 1, ty},
        {0, 0, 1}
    };

    if(obj->type == POINT) {
        obj->objectData.point = applyTransformation(translation_matrix, obj->objectData.point);
    }
    else if(obj->type == LINE) {
        obj->objectData.line.start_line = applyTransformation(translation_matrix, obj->objectData.line.start_line);
        obj->objectData.line.end_line = applyTransformation(translation_matrix, obj->objectData.line.end_line);
    }
    else if(obj->type == POLYGON) {
        for(int i = 0; i < obj->objectData.polygon.num_vertices; i++) {
            obj->objectData.polygon.vertices[i] = applyTransformation(translation_matrix, obj->objectData.polygon.vertices[i]);
        }
    }

}

void scaleObject(Object *obj, float scale_factor) {
    Point center = getObjectCenter(obj);
    // Primeiro, transladar o objeto para a origem
    translateObject(obj, -center.x, -center.y);

    // Escala
    float scale_matrix[3][3] = {
        {scale_factor, 0, 0},
        {0, scale_factor, 0},
        {0, 0, 1}
    };

    if(obj->type == POINT) {
    }
    else if(obj->type == LINE) {
        obj->objectData.line.start_line = applyTransformation(scale_matrix, obj->objectData.line.start_line);
        obj->objectData.line.end_line = applyTransformation(scale_matrix, obj->objectData.line.end_line);
    }
    else if(obj->type == POLYGON) {
        for(int i = 0; i < obj->objectData.polygon.num_vertices; i++) {
            obj->objectData.polygon.vertices[i] = applyTransformation(scale_matrix, obj->objectData.polygon.vertices[i]);
        }
    }

    // Transladar de volta para a posição inicial
    translateObject(obj, center.x, center.y);
}

void rotateObject(Object *obj, float theta, Point center) {
    // Primeiro, transladar o objeto para a origem
    translateObject(obj, -center.x, -center.y);

    // Matriz de rotação
    float rad = theta * (M_PI / 180.0); // Converter para radianos
    float rotation_matrix[3][3] = {
        {cos(rad), -sin(rad), 0},
        {sin(rad), cos(rad), 0},
        {0, 0, 1}
    };

    if(obj->type == POINT) {
        obj->objectData.point = applyTransformation(rotation_matrix, obj->objectData.point);
    }
    else if(obj->type == LINE) {
        obj->objectData.line.start_line = applyTransformation(rotation_matrix, obj->objectData.line.start_line);
        obj->objectData.line.end_line = applyTransformation(rotation_matrix, obj->objectData.line.end_line);
    }
    else if(obj->type == POLYGON) {
        for(int i = 0; i < obj->objectData.polygon.num_vertices; i++) {
            obj->objectData.polygon.vertices[i] = applyTransformation(rotation_matrix, obj->objectData.polygon.vertices[i]);
        }
    }

    // Transladar de volta para a posição inicial
    translateObject(obj, center.x, center.y);
}

void shear(Point *points, int num_points, float shx, float shy) {
    float shear_matrix[3][3] = {
        {1, shx, 0},
        {shy, 1, 0},
        {0, 0, 1}
    };

    for(int i = 0; i < num_points; i++) {
       points[i] = applyTransformation(shear_matrix, points[i]);
    }
}

void reflection(Point *points, int num_points, char axis) {
    float reflection_matrix[3][3];

    if(axis == 'x') {
        reflection_matrix[0][0] = 1;
        reflection_matrix[1][1] = -1;
    }
    else if(axis == 'y') {
        reflection_matrix[0][0] = -1;
        reflection_matrix[1][1] = 1;
    }

    reflection_matrix[0][1] = reflection_matrix[1][0] = reflection_matrix[0][2] = reflection_matrix[1][2] = reflection_matrix[2][0] = reflection_matrix[2][1] = 0;
    reflection_matrix[2][2] = 1;

    for(int i = 0; i < num_points; i++) {
        points[i] = applyTransformation(reflection_matrix, points[i]);
    }
}

#endif
