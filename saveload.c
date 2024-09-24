#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <GL/glut.h>
#include <GL/freeglut.h>
#include <sys/time.h>

#include "LDE.h"
#include "Structures.h"
#include "texture_loader.h"

// Função para escrever no arquivo
void writeFile(ObjectList* lde, const char* filename) {
    FILE* file = fopen(filename, "w");
    if (file == NULL) {
        printf("Erro ao abrir arquivo\n");
        return;
    }

    // Escrever cada objeto na lista
    Object* current = lde->head;
    while (current != NULL) {
        // Escrever os dados do objeto
        switch (current->type) {
            case POINT:
                fprintf(file, "POINT %.2f %.2f\n", current->objectData.point.x, current->objectData.point.y);
                break;
            case LINE:
                fprintf(file, "LINE %.2f %.2f %.2f %.2f\n", current->objectData.line.start_line.x, current->objectData.line.start_line.y, current->objectData.line.end_line.x, current->objectData.line.end_line.y);
                break;
            case POLYGON:
                fprintf(file, "POLYGON %d ", current->objectData.polygon.num_vertices);
                for (int i = 0; i < current->objectData.polygon.num_vertices; i++) {
                    fprintf(file, "%.2f %.2f ", current->objectData.polygon.vertices[i].x, current->objectData.polygon.vertices[i].y);
                }
                fprintf(file, "\n");
                break;
        }

        current = current->next;
    }

    fclose(file);
    printf("Objetos salvos com sucesso em %s.\n", filename);
}

// Função para ler o arquivo
void readFile(ObjectList *lde, const char* filename) {
    FILE* file = fopen(filename, "r");

    if (file == NULL) {
        printf("Erro ao abrir arquivo\n");
        return NULL;
    }

    char type[10];
    while (fscanf(file, "%s", type) != EOF) {
        if(strcmp(type, "POINT") == 0) {
            float x, y;
            fscanf(file, "%f %f", &x, &y);
            addPoint(lde, x, y);
        }
        else if(strcmp(type, "LINE") == 0) {
            Point start_line, end_line;
            fscanf(file, "%f %f %f %f", &start_line.x, &start_line.y, &end_line.x, &end_line.y);
            addLine(lde, start_line, end_line);
        }
        else if(strcmp(type, "POLYGON") == 0) {
            int num_vertices;
            fscanf(file, "%d", &num_vertices);
            Point *vertices = (Point *)malloc(num_vertices * sizeof(Point));

            if(!vertices) {
                printf("Erro de alocação para os vértices do polígono!\n");
                fclose(file);
                return;
            }

            for(int i = 0; i < num_vertices; i++) {
                fscanf(file, "%f %f", &vertices[i].x, &vertices[i].y);
            }

            addPolygon(lde, vertices, num_vertices);
        }
    }
    fclose(file);
    printf("Objetos salvos com sucesso em %s.\n", filename);
}
