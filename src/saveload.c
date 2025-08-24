#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h> 
#include <sys/types.h> 
#include <string.h>    
#include <errno.h>    
#include <GL/glut.h>
#include <GL/freeglut.h>
#include <sys/time.h>

#include "LDE.h"
#include "structures.h"
#include "textureloader.h"

// Função para escrever no arquivo
void writeFile(ObjectList* lde, const char* filename) {
    // Nome da pasta
    const char* folder = "save";

    // Verifica se a pasta existe
    struct stat st = {0};
    if (stat(folder, &st) == -1) {
        if (mkdir(folder, 0700) != 0) {
            perror("Erro ao criar pasta save");
            return;
        }
    }

    // Monta o caminho completo: save/filename
    char path[256];
    snprintf(path, sizeof(path), "%s/%s", folder, filename);

    FILE* file = fopen(path, "w");
    if (file == NULL) {
        printf("Erro ao abrir arquivo %s\n", path);
        return;
    }

    // Escrever cada objeto na lista
    Object* current = lde->head;
    while (current != NULL) {
        // Escrever os dados do objeto
        switch (current->type) {
            case POINT:
                fprintf(file, "POINT %.2f %.2f %.2f %.2f %.2f\n", 
                    current->objectData.point.x, 
                    current->objectData.point.y,
                    current->color.r, 
                    current->color.g, 
                    current->color.b);
                break;
            case LINE:
                fprintf(file, "LINE %.2f %.2f %.2f %.2f %.2f %.2f %.2f\n", 
                    current->objectData.line.start_line.x, 
                    current->objectData.line.start_line.y, 
                    current->objectData.line.end_line.x, 
                    current->objectData.line.end_line.y,
                    current->color.r, 
                    current->color.g, 
                    current->color.b);
                break;
            case POLYGON:
                fprintf(file, "POLYGON %d", current->objectData.polygon.num_vertices);
                for (int i = 0; i < current->objectData.polygon.num_vertices; ++i) {
                    fprintf(file, " %.2f %.2f", 
                        current->objectData.polygon.vertices[i].x, 
                        current->objectData.polygon.vertices[i].y);
                }
                // Adicionando as componentes da cor
                fprintf(file, " %.2f %.2f %.2f\n", 
                    current->color.r, 
                    current->color.g, 
                    current->color.b);
                break;
            case CIRCLE:
                fprintf(file, "CIRCLE %.2f %.2f %.2f %.2f %.2f %.2f\n",
                    current->objectData.circle.center.x,
                    current->objectData.circle.center.y,
                    current->objectData.circle.radius,
                    current->color.r,
                    current->color.g,
                    current->color.b);
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
        return;
    }

    char type[10];
    while (fscanf(file, "%s", type) != EOF) {
        if (strcmp(type, "POINT") == 0) {
            float x, y;
            Color color; // Cria uma instância de Color
            fscanf(file, "%f %f %f %f %f", &x, &y, &color.r, &color.g, &color.b);
            addPoint(lde, x, y, color); // Chamada para adicionar ponto com cor
        }
        else if (strcmp(type, "LINE") == 0) {
            Point start_line, end_line;
            Color color; // Cria uma instância de Color
            fscanf(file, "%f %f %f %f %f %f %f", 
                   &start_line.x, &start_line.y, 
                   &end_line.x, &end_line.y, 
                   &color.r, &color.g, &color.b);
            addLine(lde, start_line, end_line, color); // Chamada para adicionar linha com cor
        }
        else if (strcmp(type, "POLYGON") == 0) {
            int num_vertices;
            fscanf(file, "%d", &num_vertices);
            Point *vertices = (Point *)malloc(num_vertices * sizeof(Point));

            if (!vertices) {
                printf("Erro de alocação para os vértices do polígono!\n");
                fclose(file);
                return;
            }

            for (int i = 0; i < num_vertices; i++) {
                fscanf(file, "%f %f", &vertices[i].x, &vertices[i].y);
            }

            Color color; // Cria uma instância de Color
            fscanf(file, "%f %f %f", &color.r, &color.g, &color.b); // Lê a cor do polígono
            addPolygon(lde, vertices, num_vertices, color); // Chamada para adicionar polígono com cor
        }
        else if (strcmp(type, "CIRCLE") == 0) {
            Point circle_center;
            float radius;
            Color color; // Cria uma instância de Color
            fscanf(file, "%f %f %f %f %f %f", 
                &circle_center.x, &circle_center.y, &radius, 
                &color.r, &color.g, &color.b);
            addCircle(lde, circle_center, radius, color); // Chamada para adicionar círculo com cor
        }
    }

    fclose(file);
    printf("Objetos carregados com sucesso de %s.\n", filename);
}
