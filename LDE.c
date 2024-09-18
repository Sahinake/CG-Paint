#include <stdio.h>
#include <stdlib.h>
#include "LDE.h"

// Inicializa a lista
void initObjectList(ObjectList *lde) {
    lde->head = NULL;
    lde->tail = NULL;
}

// Cria um novo nó
Object* createObject(ObjectType type) {
    Object *new_object = (Object *)malloc(sizeof(Object));

    if(new_object == NULL) {
        printf("Erro: Falha ao alocar memória para o objeto.\n");
        exit(1);
    }

    new_object->type = type;
    new_object->prev = NULL;
    new_object->next = NULL;
    return new_object;
}

// Adiciona um ponto à lista
void addPoint(ObjectList *lde, float x, float y) {
    Object *new_object = createObject(POINT);
    new_object->objectData.point.x = x;
    new_object->objectData.point.y = y;

    // Se a lista estiver vazia
    if(lde->head == NULL) {
        lde->head = new_object;
        lde->tail = new_object;
    }
    else {
        new_object->prev = lde->head;
        lde->tail->next = new_object;
        lde->tail = new_object;
    }
}

// Adiciona uma linha à lista
void addLine(ObjectList *lde, Point start_line, Point end_line) {
    Object *new_object = createObject(LINE);
    new_object->objectData.line.start_line = start_line;
    new_object->objectData.line.end_line = end_line;

    // Se a lista estiver vazia
    if(lde->head == NULL) {
        lde->head = new_object;
        lde->tail = new_object;
    }
    else {
        new_object->prev = lde->head;
        lde->tail->next = new_object;
        lde->tail = new_object;
    }
}

// Adiciona um polígono à lista
void addPolygon(ObjectList *lde, Point *vertices, int num_vertices) {
    Object *new_object = createObject(POLYGON);
    new_object->objectData.polygon.vertices = (Point *)malloc(sizeof(Point) * num_vertices);

    if(new_object->objectData.polygon.vertices == NULL) {
        printf("Erro: Falha ao alocar memória para os vértices do polígono.\n");
        exit(1);
    }

    for(int i = 0; i < num_vertices; i++) {
        new_object->objectData.polygon.vertices[i] = vertices[i];
    }

    new_object->objectData.polygon.num_vertices = num_vertices;

    // Se a lista estiver vazia
    if(lde->head == NULL) {
        lde->head = new_object;
        lde->tail = new_object;
    }
    else {
        new_object->prev = lde->head;
        lde->tail->next = new_object;
        lde->tail = new_object;
    }
}

// Remove um objeto da lista
void removeObject(ObjectList *lde, Object *obj) {
    if(obj == NULL) return;

    if(obj->prev) {
        obj->prev->next = obj->next;
    }
    else {
        lde->head = obj->next;
    }

    if(obj->next) {
        obj->next->prev = obj->prev;
    }
    else {
        lde->tail = obj->prev;
    }

    // Libera a memória se for um polígono (pois contém um array dinâmico)
    if(obj->type == POLYGON) {
        free(obj->objectData.polygon.vertices);
    }
    free(obj);
}

// Limpa a lista e libera a memória
void clearObjectList(ObjectList *lde) {
    Object *current = lde->head;
    Object *next;

    while(current != NULL) {
        next = current->next;

        // Libera a memória se for um polígono
        if(current->type == POLYGON) {
            free(current->objectData.polygon.vertices);
        }

        free(current);
        current = next;
    }
    lde->head = NULL;
    lde->tail = NULL;
}
