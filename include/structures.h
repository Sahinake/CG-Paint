#ifndef STRUCTURES_H
#define STRUCTURES_H
#include <stdbool.h>

// Estrutura do ponto
typedef struct {
    float x, y;
} Point;

// Estrutura da linha
typedef struct {
    Point start_line;
    Point end_line;
} Line;

// Estrutura do polígono (array de vértices)
typedef struct {
    Point *vertices;
    int num_vertices;
} Polygon;

// Estrutura do círculo
typedef struct {
    Point center;
    float radius;
} Circle;

// Enum com os tipos de objetos (ponto, linha ou polígono)
typedef enum {
    POINT,
    LINE,
    POLYGON,
    CIRCLE
} ObjectType;

// Enum com os tipos de estados
typedef enum {
    MODE_SELECT,
    MODE_CREATE_POINT,
    MODE_CREATE_LINE,
    MODE_CREATE_POLYGON,
    MODE_CREATE_CIRCLE,
    MODE_SHEAR,
} Mode;

typedef struct {
    float r, g, b;
} Color;

// Estrutura do nó da lista duplamente encadeada
typedef struct Object {
    ObjectType type;
    Color color;

    // Dados do objeto
    union {
        Point point;
        Line line;
        Polygon polygon;
        Circle circle;
    } objectData;

    struct Object *prev;
    struct Object *next;
} Object;

// Estrutura da lista duplamente encadeada
typedef struct {
    // Ponteiro para o primeiro nó
    Object *head;
    // Ponteiro para o último nó
    Object *tail;
} ObjectList;

typedef struct Button {
    float x, y;
    float width, height;
    int selected;
    void (*action)();
} Button;

int isCloseEnough(Point a, Point b);
float calculateDistance(Point p1, Point p2);
int computeRegionCode(float x, float y, float xmin, float ymin, float xmax, float ymax);
int pickPoint(Point p, Point clicked_point, float tolerancy);
int pickLine(Line line, Point clicked_point, float tolerancy);
int pickPolygon(Polygon poly, Point clicked_point);
int pickPolygon(Polygon poly, Point clicked_point);
int pickCircle(Circle circle, Point clicked_point);
Point applyTransformation(float matrix[3][3], Point p);
Point getObjectCenter(Object *obj);
void translateObject(Object *obj, float tx, float ty);
void scaleObject(Object *obj, float scale_factor) ;
void rotateObject(Object *obj, float angle);
void shearObject(Object *obj, float shx, float shy);
void reflectObject(Object *obj, int reflectX, int reflectY);
void grahamScan(Object *obj);
void drawConcavePolygon(Object *obj);
#endif
