#ifndef LDEP_H
#define LDEP_H
#include "structures.h"

// Inicializa a Lista
void initObjectList(ObjectList *lde);

// Adiciona um ponto à lista
void addPoint(ObjectList *lde, float x, float y, Color color);

// Adiciona uma linha à lista
void addLine(ObjectList *lde, Point start_line, Point end_line, Color color);

// Adiciona um polígono à lista
void addPolygon(ObjectList *lde, Point *vertices, int num_vertices, Color color);

// Remove um objeto da lista
void removeObject(ObjectList *lde, Object *obj);

// Limpa a lista e libera a memória
void clearObjectList(ObjectList *lde);

// Imprimir objetos existentes na lista
void printObjectList(ObjectList *lde);

// Trocar dois objetos
void swapNodes(ObjectList* lde, Object * obj_1, Object* obj_2);

#endif // LDEP_H
