#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "LDE.h"

// Inicializa a lista
void initObjectList(ObjectList *lde) {
    lde->head = NULL;
    lde->tail = NULL;
}

// Insere o objeto na lista com base no valor de Xmax
void insertInOrder(ObjectList *lde, Object *new_object) {
    Object *current = lde->head;

    // Se a lista estiver vazia, adiciona como o único elemento
    if (lde->head == NULL) {
        lde->head = new_object;
        lde->tail = new_object;
    } else {
        // Percorre a lista até encontrar a posição correta para inserir
        while (current != NULL && current->Xmax < new_object->Xmax) {
            current = current->next;
        }

        // Se current for NULL, significa que o novo objeto tem o maior Xmax
        if (current == NULL) {
            new_object->prev = lde->tail;
            lde->tail->next = new_object;
            lde->tail = new_object;
        } 
        // Se current não for NULL, insere antes do nó atual
        else {
            new_object->next = current;
            new_object->prev = current->prev;

            if (current->prev != NULL) {
                current->prev->next = new_object;
            } else {
                // Se o novo nó for inserido no início
                lde->head = new_object;
            }

            current->prev = new_object;
        }
    }
}

// Adiciona um ponto à lista
void addPoint(ObjectList *lde, float x, float y, float rgb[3]) {
    Object *new_object = (Object *)malloc(sizeof(Object));
    if (new_object == NULL) {
        return;
    }

    new_object->type = POINT;
    new_object->objectData.point.x = x;
    new_object->objectData.point.y = y;

    new_object->color[0] = rgb[0];
    new_object->color[1] = rgb[1];
    new_object->color[2] = rgb[2];

    new_object->Xmax = x; // O valor de Xmax para um ponto é o valor de x
    new_object->prev = NULL;
    new_object->next = NULL;

    // Insere o novo objeto na lista ordenada pelo valor de Xmax
    insertInOrder(lde, new_object);
}

// Adiciona uma linha à lista
void addLine(ObjectList *lde, Point start_line, Point end_line, float rgb[3]) {
    Object *new_object = (Object *)malloc(sizeof(Object));
    if (new_object == NULL) {
        return;
    }

    new_object->type = LINE;
    new_object->objectData.line.start_line = start_line;
    new_object->objectData.line.end_line = end_line;

    new_object->color[0] = rgb[0];
    new_object->color[1] = rgb[1];
    new_object->color[2] = rgb[2];

    new_object->Xmax = (start_line.x > end_line.x) ? start_line.x : end_line.x; // O Xmax é o maior x entre os dois pontos
    new_object->prev = NULL;
    new_object->next = NULL;

    // Insere o novo objeto na lista ordenada pelo valor de Xmax
    insertInOrder(lde, new_object);
}

// Adiciona um polígono à lista
void addPolygon(ObjectList *lde, Point *vertices, int num_vertices, float rgb[3]) {
    Object *new_object = (Object *)malloc(sizeof(Object));
    if (new_object == NULL) {
        return;
    }

    new_object->type = POLYGON;
    new_object->objectData.polygon.vertices = (Point *)malloc(sizeof(Point) * num_vertices);
    if (new_object->objectData.polygon.vertices == NULL) {
        printf("Erro: Falha ao alocar memória para os vértices do polígono.\n");
        exit(1);
    }

    // Copia os vértices
    for (int i = 0; i < num_vertices; i++) {
        new_object->objectData.polygon.vertices[i] = vertices[i];
    }

    new_object->objectData.polygon.num_vertices = num_vertices;

    new_object->color[0] = rgb[0];
    new_object->color[1] = rgb[1];
    new_object->color[2] = rgb[2];

    // Calcula o Xmax para o polígono (o maior valor de x entre todos os vértices)
    new_object->Xmax = vertices[0].x;
    for (int i = 1; i < num_vertices; i++) {
        if (vertices[i].x > new_object->Xmax) {
            new_object->Xmax = vertices[i].x;
        }
    }

    new_object->prev = NULL;
    new_object->next = NULL;

    // Insere o novo objeto na lista ordenada pelo valor de Xmax
    insertInOrder(lde, new_object);
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

// Remove o primeiro objeto da lista
void removeFirstObject(ObjectList *lde) {
    if(lde->head == NULL) return;

    Object *first = lde->head;
    lde->head = lde->head->next;

    // Libera a memória se for um polígono (pois contém um array dinâmico)
    if(first->type == POLYGON) {
        free(first->objectData.polygon.vertices);
    }
    free(first);
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
