#include <stdio.h>
#include <stdlib.h>
#include "LDE.h"
#include "structures.h"

// Inicializa a lista
void initObjectList(ObjectList *lde) {
    lde->head = NULL;
    lde->tail = NULL;
}

// Adiciona um ponto à lista
void addPoint(ObjectList *lde, float x, float y, Color color) {
    Object *new_object = (Object *)malloc(sizeof(Object));
    if(new_object == NULL) {
        return;
    }

    new_object->type = POINT;
    new_object->color = color;
    new_object->objectData.point.x = x;
    new_object->objectData.point.y = y;
    new_object->prev = NULL;
    new_object->next = NULL;

    // Se a lista estiver vazia
    if(lde->head == NULL) {
        lde->head = new_object;
        lde->tail = new_object;
    }
    else {
        new_object->prev = lde->tail;
        lde->tail->next = new_object;
        lde->tail = new_object;
    }
}

// Adiciona uma linha à lista
void addLine(ObjectList *lde, Point start_line, Point end_line, Color color) {
    Object *new_object = (Object *)malloc(sizeof(Object));
    if(new_object == NULL) {
        return;
    }

    new_object->type = LINE;
    new_object->color = color;
    new_object->objectData.line.start_line = start_line;
    new_object->objectData.line.end_line = end_line;
    new_object->prev = NULL;
    new_object->next = NULL;

    // Se a lista estiver vazia
    if(lde->head == NULL) {
        lde->head = new_object;
        lde->tail = new_object;
    }
    else {
        new_object->prev = lde->tail;
        lde->tail->next = new_object;
        lde->tail = new_object;
    }
}

// Adiciona um polígono à lista
void addPolygon(ObjectList *lde, Point *vertices, int num_vertices, Color color) {
    Object *new_object = (Object *)malloc(sizeof(Object));
    if(new_object == NULL) {
        return;
    }

    new_object->type = POLYGON;
    new_object->color = color;
    new_object->objectData.polygon.vertices = (Point *)malloc(sizeof(Point) * num_vertices);

    if(new_object->objectData.polygon.vertices == NULL) {
        printf("Erro: Falha ao alocar memória para os vértices do polígono.\n");
        exit(1);
    }

    for(int i = 0; i < num_vertices; i++) {
        new_object->objectData.polygon.vertices[i] = vertices[i];
    }

    new_object->objectData.polygon.num_vertices = num_vertices;
    new_object->prev = NULL;
    new_object->next = NULL;

    // Se a lista estiver vazia
    if(lde->head == NULL) {
        lde->head = new_object;
        lde->tail = new_object;
    }
    else {
        new_object->prev = lde->tail;
        lde->tail->next = new_object;
        lde->tail = new_object;
    }
}

// Remove um objeto da lista
void removeObject(ObjectList *lde, Object *obj) {
    if(obj == NULL || lde == NULL) return;

    if(obj->prev != NULL) {
        obj->prev->next = obj->next;
    }
    else {
        lde->head = obj->next;
    }

    if(obj->next != NULL) {
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

// Imprimir objetos existentes na lista
void printObjectList(ObjectList *lde) {
    Object *current = lde->head;
    while(current != NULL) {
        switch(current->type) {
            case POINT:
                printf("Object Type: POINT.\n");
                printf("Coordenadas: (%f, %f)\n", current->objectData.point.x, current->objectData.point.y);
                break;
            case LINE:
                printf("Object Type: LINE.\n");
                printf("Start Point: (%f, %f)\n", current->objectData.line.start_line.x, current->objectData.line.start_line.y);
                printf("End Point: (%f, %f)\n", current->objectData.line.end_line.x, current->objectData.line.end_line.y);
                break;
            case POLYGON:
                printf("Object Type: POLYGON.\n");
                printf("Vertices: \n");
                for(int i = 0; i < current->objectData.polygon.num_vertices; i++) {
                    printf("Vertice %d: (%f, %f)\n", i, current->objectData.polygon.vertices[i].x, current->objectData.polygon.vertices[i].y);
                }
                break;
            default:
                printf("Tipo de Objeto Desconhecido.\n");
                break;
        }
        current = current->next;
    }
}

void swapNodes(ObjectList* lde, Object * obj_1, Object* obj_2) {
    if (obj_1 == NULL || obj_2 == NULL) return;

    Object* prev_1 = obj_1->prev;
    Object* next_2 = obj_2->next;

    // Ajusta o anterior de node1
    if (prev_1 != NULL) {
        prev_1->next = obj_2;
    } else {
        lde->head = obj_2;  // Se node1 for o head, atualiza o head
    }

    // Ajusta o próximo de node2
    if (next_2 != NULL) {
        next_2->prev = obj_1;
    } else {
        lde->tail = obj_1;  // Se node2 for o tail, atualiza o tail
    }

    // Ajusta os ponteiros internos entre node1 e node2
    obj_2->prev = prev_1;
    obj_1->next = next_2;
    obj_2->next = obj_1;
    obj_1->prev = obj_2;
}

