#ifndef LDEP_H
#define LDEP_H
#include "structures.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <GL/glut.h>
#include <GL/glu.h>

#define INSIDE 0    //0000
#define LEFT 1      //0001
#define RIGHT 2     //0010
#define BOTTOM 4    //0100
#define TOP 8       //1000
#define M_PI 3.14159265358979323846

int isCloseEnough(Point a, Point b) {
    // Limite de proximidade
    float threshold = 3.0;
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

// Função para calcular a distância entre dois pontos
float calculateDistance(Point p1, Point p2) {
    return sqrt(pow(p2.x - p1.x, 2) + pow(p2.y - p1.y, 2));
}

// Função para selecionar um círculo
int pickCircle(Circle circle, Point clicked_point) {
    float distance = calculateDistance(circle.center, clicked_point);
    if(distance <= circle.radius) {
        return 1;
    }
    return 0;
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
        case CIRCLE:
            center = obj->objectData.circle.center;
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
    else if(obj->type == CIRCLE) {
        obj->objectData.circle.center = applyTransformation(translation_matrix, obj->objectData.circle.center);
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
    else if(obj->type == CIRCLE) {
        // Escala o raio do círculo pelo fator de escala
        obj->objectData.circle.radius *= scale_factor;
    }

    // Transladar de volta para a posição inicial
    translateObject(obj, center.x, center.y);
}

void rotateObject(Object *obj, float angle) {
    // Matriz de rotação
    float rad = angle * (M_PI / 180.0); // Converter para radianos
    float rotation_matrix[3][3] = {
        {cos(rad), -sin(rad), 0},
        {sin(rad), cos(rad), 0},
        {0, 0, 1}
    };

    if(obj->type == POINT) {
        // Rotação do ponto em torno do centro da tela
        float windows_center_x = glutGet(GLUT_WINDOW_WIDTH)/2;
        float windows_center_y = glutGet(GLUT_WINDOW_HEIGHT)/2;
        translateObject(obj, -windows_center_x, -windows_center_y);

        obj->objectData.point = applyTransformation(rotation_matrix, obj->objectData.point);

        translateObject(obj, windows_center_x, windows_center_y);
        return;
    }
    //if(obj->type == POINT) {
    //    obj->objectData.point = applyTransformation(rotation_matrix, obj->objectData.point);
    //    return;
    //}

    Point center = getObjectCenter(obj);
    // Primeiro, transladar o objeto para a origem
    translateObject(obj, -center.x, -center.y);

    if(obj->type == LINE) {
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

void shearObject(Object *obj, float shx, float shy) {
    Point center = getObjectCenter(obj);

    float shear_matrix[3][3] = {
        {1, shx, 0},
        {shy, 1, 0},
        {0, 0, 1}
    };

    // Primeiro, transladar o objeto para a origem
    translateObject(obj, -center.x, -center.y);

    // Aplica a matriz de Cisalhamento
    if(obj->type == POINT) {
    }
    else if(obj->type == LINE) {
        obj->objectData.line.start_line = applyTransformation(shear_matrix, obj->objectData.line.start_line);
        obj->objectData.line.end_line = applyTransformation(shear_matrix, obj->objectData.line.end_line);
    }
    else if(obj->type == POLYGON) {
        for(int i = 0; i < obj->objectData.polygon.num_vertices; i++) {
            obj->objectData.polygon.vertices[i] = applyTransformation(shear_matrix, obj->objectData.polygon.vertices[i]);
        }
    }

    // Transladar de volta para a posição inicial
    translateObject(obj, center.x, center.y);
}

void reflectObject(Object *obj, int reflectX, int reflectY) {
    Point center = getObjectCenter(obj);

    // Matriz de Reflexão
    float reflection_matrix[3][3] = {
        {reflectX ? -1 : 1, 0, 0},
        {0, reflectY ? -1 : 1, 0},
        {0, 0, 1}
    };

    // Primeiro, transladar o objeto para a origem
    translateObject(obj, -center.x, -center.y);

    // Aplica a matriz de reflexão
    if(obj->type == POINT) {
    }
    else if(obj->type == LINE) {
        obj->objectData.line.start_line = applyTransformation(reflection_matrix, obj->objectData.line.start_line);
        obj->objectData.line.end_line = applyTransformation(reflection_matrix, obj->objectData.line.end_line);
    }
    else if(obj->type == POLYGON) {
        for(int i = 0; i < obj->objectData.polygon.num_vertices; i++) {
            obj->objectData.polygon.vertices[i] = applyTransformation(reflection_matrix, obj->objectData.polygon.vertices[i]);
        }
    }

    // Transladar de volta para a posição inicial
    translateObject(obj, center.x, center.y);
}

// Função para trocar dois pontos
void swap(Point *p1, Point *p2) {
    Point temp = *p1;
    *p1 = *p2;
    *p2 = temp;
}

// Função para calcular o produto vetorial de três pontos
float crossProduct(Point p0, Point p1, Point p2) {
    return (p1.x - p0.x) * (p2.y - p0.y) - (p1.y - p0.y) * (p2.x - p0.x);
}

// Função para calcular a distância ao quadrado entre dois pontos
float distanceSquared(Point p1, Point p2) {
    return (p1.x - p2.x) * (p1.x - p2.x) + (p1.y - p2.y) * (p1.y - p2.y);
}

// Função de comparação para ordenação angular
int compare(const void *a, const void *b, void *p0) {
    Point *p1 = (Point *)a;
    Point *p2 = (Point *)b;
    Point *p0_ = (Point *)p0;

    float cp = crossProduct(*p0_, *p1, *p2);
    if (cp == 0) {
        // Se colineares, o mais próximo do ponto p0 vem primeiro
        return distanceSquared(*p0_, *p1) < distanceSquared(*p0_, *p2) ? -1 : 1;
    }
    return (cp > 0) ? -1 : 1;
}

// Função para encontrar o fecho convexo usando a Varredura de Graham
void grahamScan(Object *obj) {
    int n = obj->objectData.polygon.num_vertices;
    Point *points = obj->objectData.polygon.vertices;

    if (n < 3) {
        printf("O fecho convexo não pode ser formado com menos de 3 pontos.\n");
        return;
    }

    // Encontre o ponto mais abaixo (com menor y e menor x em caso de empate)
    int min_idx = 0;
    for (int i = 1; i < n; i++) {
        if ((points[i].y < points[min_idx].y) ||
            (points[i].y == points[min_idx].y && points[i].x < points[min_idx].x)) {
            min_idx = i;
        }
    }

    // Troque o ponto mais abaixo com o primeiro ponto
    swap(&points[0], &points[min_idx]);

    // Ordene os pontos com base no ângulo polar em relação ao ponto mais abaixo
    qsort_r(points + 1, n - 1, sizeof(Point), compare, &points[0]);

    // Varredura para encontrar o fecho convexo
    Point hull[n];
    int hull_size = 0;

    // Empilha os primeiros dois pontos
    hull[hull_size++] = points[0];
    hull[hull_size++] = points[1];

    for (int i = 2; i < n; i++) {
        // Enquanto o ângulo formado pelos três últimos pontos for à direita, remova o penúltimo
        while (hull_size >= 2 && crossProduct(hull[hull_size - 2], hull[hull_size - 1], points[i]) <= 0) {
            hull_size--;
        }
        hull[hull_size++] = points[i];
    }

    // Exibir o fecho convexo
    printf("Fecho convexo:\n");
    for (int i = 0; i < hull_size; i++) {
        printf("(%.2f, %.2f)\n", hull[i].x, hull[i].y);
    }

    // Atualizar o polígono original com os pontos do fecho convexo
    obj->objectData.polygon.num_vertices = hull_size;
    for (int i = 0; i < hull_size; i++) {
        obj->objectData.polygon.vertices[i] = hull[i];
    }
}

// Callback para quando o tessellator encontra um vértice
void tessVertexCallback(void* vertex) {
    const GLdouble* pointer = (GLdouble*)vertex;
    glVertex2dv(pointer);
}

// Callback para quando o tessellator inicia um polígono
void tessBeginCallback(GLenum type) {
    glBegin(type);
}

// Callback para quando o tessellator termina o polígono
void tessEndCallback() {
    glEnd();
}

// Função para desenhar um polígono concavo ou convexo usando tessellator
void drawConcavePolygon(Object *obj) {
    if (obj->objectData.polygon.num_vertices < 3) return;  // Verifica se tem ao menos 3 vértices

    GLUtesselator* tess = gluNewTess();  // Cria o tessellator

    // Define os callbacks para o tessellator
    gluTessCallback(tess, GLU_TESS_VERTEX, (void (*)(void))tessVertexCallback);
    gluTessCallback(tess, GLU_TESS_BEGIN, (void (*)(void))tessBeginCallback);
    gluTessCallback(tess, GLU_TESS_END, (void (*)(void))tessEndCallback);

    // Converte os pontos para GLdouble (usado pelo tessellator)
    GLdouble (*vertices)[3] = malloc(obj->objectData.polygon.num_vertices * sizeof(GLdouble[3]));

    // Preenche os vértices
    for (int i = 0; i < obj->objectData.polygon.num_vertices; i++) {
        vertices[i][0] = obj->objectData.polygon.vertices[i].x;
        vertices[i][1] = obj->objectData.polygon.vertices[i].y;
        vertices[i][2] = 0.0;  // Define Z como 0 (plano 2D)
    }

    // Inicia o processo de tesselação
    gluTessBeginPolygon(tess, NULL);
    gluTessBeginContour(tess);

    // Adiciona os vértices ao tessellator
    for (int i = 0; i < obj->objectData.polygon.num_vertices; i++) {
        gluTessVertex(tess, vertices[i], vertices[i]);
    }

    gluTessEndContour(tess);
    gluTessEndPolygon(tess);

    // Limpa os recursos
    gluDeleteTess(tess);
    free(vertices);
}

#endif
