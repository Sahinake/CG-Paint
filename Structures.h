#ifndef STRUCTURES_H
#define STRUCTURES_H

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

// Enum com os tipos de objetos (ponto, linha ou polígono)
typedef enum {
    POINT,
    LINE,
    POLYGON
} ObjectType;

// Enum com os tipos de estados
typedef enum {
    MODE_CREATE_POINT,
    MODE_CREATE_LINE,
    MODE_CREATE_POLYGON,
    MODE_SELECT
} Mode;

// Estrutura do nó da lista duplamente encadeada
typedef struct Object {
    ObjectType type;

    // Dados do objeto
    union {
        Point point;
        Line line;
        Polygon polygon;
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

int isCloseEnough(Point a, Point b);
int computeRegionCode(float x, float y, float xmin, float ymin, float xmax, float ymax);
int pickPoint(Point p, Point clicked_point, float tolerancy);
int pickLine(Line line, Point clicked_point, float tolerancy);
int pickPolygon(Polygon poly, Point clicked_point);
int pickPolygon(Polygon poly, Point clicked_point);

#endif
