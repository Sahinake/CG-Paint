#ifndef LDEP_H
#define LDEP_H
#include "structures.h"

// Inicializa a Lista
void initObjectList(ObjectList *lde);

// Adiciona um ponto à lista
void addPoint(ObjectList *lde, float x, float y);

// Adiciona uma linha à lista
void addLine(ObjectList *lde, Point start_line, Point end_line);

// Adiciona um polígono à lista
void addPolygon(ObjectList *lde, Point *vertices, int num_vertices);

// Remove um objeto da lista
void removeObject(ObjectList *lde, Object *obj);

// Limpa a lista e libera a memória
void clearObjectList(ObjectList *lde);

// Imprimir objetos existentes na lista
void printObjectList(ObjectList *lde);

#endif // LDEP_H
