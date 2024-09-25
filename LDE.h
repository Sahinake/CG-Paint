#ifndef LDEP_H
#define LDEP_H
#include "Structures.h"

// Inicializa a Lista
void initObjectList(ObjectList *lde);

// Adiciona um ponto à lista
void addPoint(ObjectList *lde, float x, float y, float rgb[3]);

// Adiciona uma linha à lista
void addLine(ObjectList *lde, Point start_line, Point end_line, float rgb[3]);

// Adiciona um polígono à lista
void addPolygon(ObjectList *lde, Point *vertices, int num_vertices, float rgb[3]);

// Remove um objeto da lista
void removeObject(ObjectList *lde, Object *obj);

// Remove o ultimo objeto da lista
void removeFirstObject(ObjectList *lde);

// Limpa a lista e libera a memória
void clearObjectList(ObjectList *lde);

// Imprimir objetos existentes na lista
void printObjectList(ObjectList *lde);

// Insere o objeto na lista com base no valor de Xmax
void insertInOrder(ObjectList *lde, Object *new_object);

#endif // LDEP_H
