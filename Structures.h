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
    MODE_SELECT,
    MODE_CREATE_POINT,
    MODE_CREATE_LINE,
    MODE_CREATE_POLYGON,
    MODE_SHEAR,
} Mode;

// Estrutura do nó da lista duplamente encadeada
typedef struct Object {
    ObjectType type;
    float Xmax; // Adicione este campo para armazenar o valor de Xmax

    // Dados do objeto
    union {
        Point point;
        Line line;
        Polygon polygon;
    } objectData;

    float color[3]; // Adicione este campo para armazenar a cor (RGB)

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
int computeRegionCode(float x, float y, float xmin, float ymin, float xmax, float ymax);
int pickPoint(Point p, Point clicked_point, float tolerancy);
int pickLine(Line line, Point clicked_point, float tolerancy);
int pickPolygon(Polygon poly, Point clicked_point);
int pickPolygon(Polygon poly, Point clicked_point);
Point applyTransformation(float matrix[3][3], Point p);
Point getObjectCenter(Object *obj);
void translateObject(Object *obj, float tx, float ty);
void scaleObject(Object *obj, float scale_factor) ;
void rotateObject(Object *obj, float angle);
void shearObject(Object *obj, float shx, float shy);
void reflectObject(Object *obj, int reflectX, int reflectY);

void animateObjects(int value);
void createAnimatedPolygon();

#endif
