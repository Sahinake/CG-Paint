#include <GL/glew.h>
#include <stdio.h>
#include <stdlib.h>
#include <GL/glut.h>
#include <GL/freeglut.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>
#include <math.h>

#include "LDE.h"
#include "Structures.h"
#include "texture_loader.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define DOUBLE_CLICK_THRESHOLD 500  //500 milissegundos para detecção do duplo clique
#define TOLERANCY 5.0f              // Raio de Seleção
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define NUM_BUTTONS 7               // O número de ícones

Object *animated_object = NULL;  // Objeto que será animado
float animation_speed = 0.0f;    // Velocidade do movimento
int is_animating = 0;            // Flag para saber se a animação está ocorrendo
float elapsed_time = 0;          // Tempo decorrido da animação
const int animation_duration = 20000;  // Duração total da animação (10 segundos)

// Lista duplamente encadeada que armazena os objetos
ObjectList object_list;

// Estado inicial
Mode current_mode = MODE_CREATE_POINT;

// Variável para armazenar o ponto selecionado
Object *selected_object = NULL;

Button buttons[NUM_BUTTONS];

// Lista temporária de vértices do polígono (max 100 vértices)
Point temp_polygon_vertices[100];
// Contador de vértices
int vertices_count = 0;
// Para armazenar o primeiro ponto ao criar uma linha
Point first_point;
// Armazena a posição atual do mouse
Point current_mouse_position;
// Última posição conhecida do mouse
Point last_mouse_position;
// Estado do modo de rotação
int rotation_mode;

// Flag para saber se estamos no processo de criar uma linha
int creating_line = 0;
// Flag para saber se estamos no processo de criar um polígono
int creating_polygon = 0;
// Flag para saber se o objeto está sendo arrastado
int dragging = 0;
// Flag para saber se o objeto está sendo deformado
int shearing = 0;
// Armazena o tempo do primeiro clique
int last_click_time = 0;

GLuint icons[NUM_BUTTONS];

void loadIcons() {
    icons[0] = loadTexture("selection.png");    // Ícone de Seleção
    icons[1] = loadTexture("pencil.png");       // Ícone de Ponto
    icons[2] = loadTexture("line-segment.png"); // Ícone de Linha
    icons[3] = loadTexture("polygon.png");      // Ícone de Polígono
    icons[4] = loadTexture("shear.png");        // Ícone de Cisalhamento
    icons[5] = loadTexture("reflectX.png");     // Ícone de Reflexão em X
    icons[6] = loadTexture("reflextY.png");     // Ícone de Reflexão em Y
}

int getTimeInMillis() {
    struct timeval time;
    gettimeofday(&time, NULL);
    return (time.tv_sec * 1000) + (time.tv_usec / 1000);
}

// Função para converter coordenadas de janela para coordenadas OpenGL (-1, 1)
Point convertScreenToOpenGL(int x, int y) {
    Point p;
    int window_height = glutGet(GLUT_WINDOW_HEIGHT);

    p.x = x;
    // Inverte o Y, pois o OpenGL tem origem no canto inferior esquerdo
    p.y = window_height - (float)y;
    return p;
}

void selectMode() {
    if(current_mode != MODE_ANIMATE && is_animating == 0){
        current_mode = MODE_SELECT;
        printf("Modo de seleção ativado.\n");
        creating_line = 0;
        creating_polygon = 0;
        glutPostRedisplay();
    }   
}

void createPointMode() {
    if(current_mode != MODE_ANIMATE && is_animating == 0) {
        current_mode = MODE_CREATE_POINT;
        printf("Modo de criação de pontos ativado.\n");
        creating_line = 0;
        creating_polygon = 0;
        glutPostRedisplay();
        }
}

void createLineMode() {
    if(current_mode != MODE_ANIMATE && is_animating == 0) {
        current_mode = MODE_CREATE_LINE;
        printf("Modo de criação de linhas ativado.\n");
        creating_line = 0;
        creating_polygon = 0;
        glutPostRedisplay();
    }
}
void createPolygonMode() {
    if(current_mode != MODE_ANIMATE && is_animating == 0) {
        current_mode = MODE_CREATE_POLYGON;
        printf("Modo de criação de polígonos ativado.\n");
        creating_line = 0;
        creating_polygon = 0;
        glutPostRedisplay();
    }
}

void shearMode() {
    if(current_mode != MODE_ANIMATE && is_animating == 0) {
        current_mode = MODE_SHEAR;
        printf("Modo de cisalhamento ativado.\n");
        creating_line = 0;
        creating_polygon = 0;
        glutPostRedisplay();
    }
}

void reflectX() {
    printf("Reflexão em X\n");
    creating_line = 0;
    creating_polygon = 0;
    if(selected_object != NULL) {
        reflectObject(selected_object, 0, 1);
    }
    glutPostRedisplay();
}

void reflectY() {
    printf("Reflexão em Y\n");
    creating_line = 0;
    creating_polygon = 0;
    if(selected_object != NULL) {
        reflectObject(selected_object, 1, 0);
    }
    glutPostRedisplay();
}

void motion(int x, int y) {
    current_mouse_position = convertScreenToOpenGL(x,y);
    if(creating_polygon == 1 || creating_line == 1 || dragging == 1) {
        glutPostRedisplay();
    }
}

int getTextWidth(const char *text, void *font) {
    int width = 0;
    while(*text) {
        width += glutBitmapWidth(font, *text);
        text++;
    }
    return width;
}

// Função para renderizar o texto do modo atual da tela
void renderModeText(float x, float y, const char* text, void *font) {
    glRasterPos2f(x, y);
    while(*text) {
        glutBitmapCharacter(font, *text);
        text++;
    }
}

const char* getObjectInfo(Object *obj) {
    const char *info;

    if(obj == NULL) {
        return "Nenhum objeto selecionado";
    }

    switch(obj->type) {
        case POINT:
            info = "Selected Object: POINT"; break;
        case LINE:
            info = "Selected Object: LINE"; break;
        case POLYGON:
            info = "Selected Object: POLYGON "; break;
        default:
            info = "Objeto desconhecido"; break;
    }

    return info;
}

void displayInfo() {
    glColor3f(0.0, 0.0, 0.0);
    float window_width = glutGet(GLUT_WINDOW_WIDTH);
    void *font = GLUT_BITMAP_9_BY_15;
    const char *mode_text;

    // Define o texto baseado no modo atual
    switch(current_mode) {
        case MODE_SELECT:
            mode_text = "Select Mode"; break;
        case MODE_CREATE_POINT:
            mode_text = "Point Creation Mode"; break;
        case MODE_CREATE_LINE:
            mode_text = "Line Creation Mode"; break;
        case MODE_CREATE_POLYGON:
            mode_text = "Polygon Creation Mode"; break;
        case MODE_SHEAR:
            mode_text = "Shear Mode"; break;
        case MODE_ANIMATE:
            mode_text = "Animation Mode"; break;
    }

    int text_width = getTextWidth(mode_text, font);
    float x = (window_width - text_width - 10);
    float y = 8;

    renderModeText(x, y, mode_text, font);

    if(selected_object != NULL) {
        x = 10;
        mode_text = getObjectInfo(selected_object);
        renderModeText(x, y, mode_text, font);
        if(rotation_mode) {
            x = 20 + getTextWidth(mode_text, font);
            renderModeText(x, y, "Rotation Mode ON", font);
        }
    }
}

void drawDrawingArea() {
    // Cor da Borda
    glColor3f(1.0, 1.0, 1.0);
    glBegin(GL_QUADS);
        glVertex2f(50, 25);
        glVertex2f(glutGet(GLUT_WINDOW_WIDTH), 25);
        glVertex2f(glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT) - 4);
        glVertex2f(50, glutGet(GLUT_WINDOW_HEIGHT) - 4);
    glEnd();
}

void drawButtons(GLuint texture_ID, int index, float x, float y, float width, float height, int is_selected) {
    if(index != current_mode) {
        glColor3f(0.9f, 0.9f, 0.9f);
    }
    else {
        glColor3f(1.0f, 1.0f, 1.0f);
    }

    // Desenha o quadrado para o ícone
    glBegin(GL_QUADS);
        glVertex2f(x, y);
        glVertex2f(x + width, y);
        glVertex2f(x + width, y - height);
        glVertex2f(x, y - height);
    glEnd();

    if(index != current_mode) {
        glColor3f(0.9f, 0.9f, 0.9f);
    }
    else {
        glColor3f(0.1f, 0.3, 1.0f);
    }

    // Desenha o quadrado para o ícone
    glBegin(GL_LINE_LOOP);
        glVertex2f(x, y);
        glVertex2f(x + width, y);
        glVertex2f(x + width, y - height);
        glVertex2f(x, y - height);
    glEnd();

    // Ativa a textura 2D
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texture_ID);

    // Desenha o quadrado para o ícone
    glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 1.0f); glVertex2f(x, y);
        glTexCoord2f(1.0f, 1.0f); glVertex2f(x + width, y);
        glTexCoord2f(1.0f, 0.0f); glVertex2f(x + width, y - height);
        glTexCoord2f(0.0f, 0.0f); glVertex2f(x, y - height);
    glEnd();

    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_TEXTURE_2D);
}

void drawMenu() {
    float button_width = 40.0f;
    float button_height = 40.0f;
    float x = 5;
    float y = glutGet(GLUT_WINDOW_HEIGHT) - 5;
    int is_selected = 0;
    void (*action[NUM_BUTTONS])() = {selectMode, createPointMode, createLineMode, createPolygonMode, shearMode, reflectX, reflectY};

    // Cor de fundo do menu
    glColor3f(0.9, 0.9, 0.9);
    glBegin(GL_QUADS);
        glVertex2f(0, 0);
        glVertex2f(50, 0);
        glVertex2f(50, glutGet(GLUT_WINDOW_HEIGHT));
        glVertex2f(0, glutGet(GLUT_WINDOW_HEIGHT));
    glEnd();

    glBegin(GL_QUADS);
        glVertex2f(0, 0);
        glVertex2f(glutGet(GLUT_WINDOW_WIDTH), 0);
        glVertex2f(glutGet(GLUT_WINDOW_WIDTH), 25);
        glVertex2f(0, 25);
    glEnd();

    for(int i = 0; i < NUM_BUTTONS; i++) {
        switch (i) {
            case 0:
                if(current_mode == MODE_SELECT) {
                    is_selected = 1;
                }
            case 1: if(current_mode == MODE_CREATE_POINT) {
                    is_selected = 1;
                }
            case 2: if(current_mode == MODE_CREATE_LINE) {
                    is_selected = 1;
                }
            case 3: if(current_mode == MODE_CREATE_POLYGON) {
                    is_selected = 1;
                }
            case 4: if(current_mode == MODE_SHEAR) {
                    is_selected = 1;
                }

        }
        drawButtons(icons[i], i, x, y - i * (button_height + 5.0f), button_width, button_height, is_selected);
        buttons[i] = (Button){x, y - i * (button_height + 5.0f), button_width, button_height, is_selected, action[i]};
    }
}

void initMenu() {
    float button_width = 40, button_height = 40;
    float x = 5.0f, y = glutGet(GLUT_WINDOW_HEIGHT) - 5;
    void (*action[NUM_BUTTONS])() = {selectMode, createPointMode, createLineMode, createPolygonMode, shearMode, reflectX, reflectY};

    for(int i = 0; i < NUM_BUTTONS; i++) {
        buttons[i] = (Button){x, y - i * (button_height + 5.0f), button_width, button_height, 0, action[i]};
    }
}

// Função de teste
void printButtonData() {
    for(int i = 0; i < NUM_BUTTONS; i++) {
        printf("Button %d:\n", i);
        printf("    Position: (%f, %f)\n", buttons[i].x, buttons[i].y);
        printf("    Size: %f x %f\n", buttons[i].width, buttons[i].height);
        printf("    Selected: %s\n", buttons[i].selected ? "Yes" : "No");
        printf("\n");
    }
}

void mouseMotion(int x, int y) {
    if (dragging && selected_object != NULL) {
        // Converte as coordenadas da tela para coordenadas OpenGL
        Point moved_point = convertScreenToOpenGL(x, y);

        // Calcula o deslocamento desde a última posição
        float dx = moved_point.x - last_mouse_position.x;
        float dy = moved_point.y - last_mouse_position.y;

        // Atualiza a posição do objeto selecionado em tempo real
        translateObject(selected_object, dx, dy);

        // Armazena a nova posição como a última posição do mouse
        last_mouse_position = moved_point;

        // Redesenha a tela
        glutPostRedisplay();
    }
}

// Callback para eventos de clique do mouse
void mouse(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON && state == GLUT_UP) {
        Point p = convertScreenToOpenGL(x, y);
        for (int i = 0; i < 5; i++) {
            if (p.x >= buttons[i].x && p.x <= buttons[i].x + buttons[i].width && p.y <= buttons[i].y && p.y >= buttons[i].y - buttons[i].height) {
                rotation_mode = 0;
                buttons[i].action();
                glutPostRedisplay();
                break;
            }
        }
    }

    if (current_mode != MODE_SELECT && current_mode != MODE_SHEAR && button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        Point p = convertScreenToOpenGL(x, y);
        rotation_mode = 0;

        if (p.x > 50 && p.y > 25) {
            if (current_mode == MODE_CREATE_POINT) {
                // Adiciona um ponto à lista
                vertices_count = 0;
                creating_polygon = 0;
                creating_line = 0;
                float rgb[3] = {0.0f, 0.0f, 0.0f};
                addPoint(&object_list, p.x, p.y, rgb);
            } else if (current_mode == MODE_CREATE_LINE) {
                if (creating_line == 0) {
                    // Primeira metade da linha (captura do primeiro ponto)
                    creating_polygon = 0;
                    first_point = p;
                    vertices_count = 0;
                    creating_line = 1;
                } else {
                    // Segunda metade da linha (captura do segundo ponto)
                    float rgb[3] = {1.0f, 0.0f, 1.0f};
                    addLine(&object_list, first_point, p, rgb);
                    creating_line = 0;
                }
            } else if (current_mode == MODE_CREATE_POLYGON) {
                // Adiciona um vértice temporário ao polígono
                creating_line = 0;
                temp_polygon_vertices[vertices_count] = p;
                vertices_count++;
                creating_polygon = 1;

                // Verifica se o usuário clicou no primeiro ponto
                if (vertices_count > 2 && isCloseEnough(p, temp_polygon_vertices[0])) {
                    // Fecha o polígono
                    float rgb[3] = {0.0f, 1.0f, 1.0f};
                    addPolygon(&object_list, temp_polygon_vertices, vertices_count, rgb);
                    vertices_count = 0;
                    creating_polygon = 0;
                }
            }
            glutPostRedisplay();
        }
    }

    if (current_mode == MODE_SHEAR && button == GLUT_LEFT_BUTTON) {
        if (state == GLUT_DOWN) {
            Point clicked_point = convertScreenToOpenGL(x, y);

            if (selected_object != NULL) {
                for (int i = 0; i < NUM_BUTTONS; i++) {
                    if (clicked_point.x >= buttons[i].x && clicked_point.x <= buttons[i].x + buttons[i].width && clicked_point.y <= buttons[i].y && clicked_point.y >= buttons[i].y - buttons[i].height) {
                        rotation_mode = 0;
                        buttons[i].action();
                        glutPostRedisplay();
                        break;
                    }
                }
            }

            Object *current = object_list.head;
            selected_object = NULL; // Resetar a seleção anterior

            while (current != NULL) {
                if (current->type == POINT) {
                    Point p = current->objectData.point;
                    if (pickPoint(p, clicked_point, TOLERANCY)) {
                        selected_object = current;
                        break;
                    }
                } else if (current->type == LINE) {
                    Line line = current->objectData.line;
                    if (pickLine(line, clicked_point, TOLERANCY)) {
                        selected_object = current;
                        shearing = 1;
                        last_mouse_position.x = clicked_point.x;
                        last_mouse_position.y = clicked_point.y;
                        break;
                    }
                } else if (current->type == POLYGON) {
                    Polygon poly = current->objectData.polygon;
                    if (pickPolygon(poly, clicked_point)) {
                        selected_object = current;
                        shearing = 1;
                        last_mouse_position.x = clicked_point.x;
                        last_mouse_position.y = clicked_point.y;
                        break;
                    }
                }
                current = current->next;
            }
            displayInfo();
            glutPostRedisplay();
        } else if (state == GLUT_UP && shearing == 1) {
            shearing = 0;
            Point unclicked_point = convertScreenToOpenGL(x, y);
            float shearx = (unclicked_point.x - last_mouse_position.x) * 0.002;
            float sheary = (unclicked_point.y - last_mouse_position.y) * 0.002;
            shearObject(selected_object, shearx, sheary);
            glutPostRedisplay();
        }
    } else if (current_mode == MODE_SELECT && button == GLUT_LEFT_BUTTON) {
        if (state == GLUT_DOWN) {
            Point clicked_point = convertScreenToOpenGL(x, y);

            if (selected_object != NULL) {
                for (int i = 0; i < NUM_BUTTONS; i++) {
                    if (clicked_point.x >= buttons[i].x && clicked_point.x <= buttons[i].x + buttons[i].width && clicked_point.y <= buttons[i].y && clicked_point.y >= buttons[i].y - buttons[i].height) {
                        rotation_mode = 0;
                        buttons[i].action();
                        glutPostRedisplay();
                        break;
                    }
                }
            }

            if (clicked_point.x > 50 && clicked_point.y > 25) {
                int current_time = getTimeInMillis();

                Object *current = object_list.head;
                selected_object = NULL;

                while (current != NULL) {
                    if (current->type == POINT) {
                        Point p = current->objectData.point;
                        if (pickPoint(p, clicked_point, TOLERANCY)) {
                            if (current_time - last_click_time < DOUBLE_CLICK_THRESHOLD) {
                                rotation_mode = !rotation_mode;
                            }
                            last_click_time = current_time;
                            selected_object = current;
                            dragging = 1;
                            last_mouse_position.x = clicked_point.x;
                            last_mouse_position.y = clicked_point.y;
                            break;
                        }
                    } else if (current->type == LINE) {
                        Line line = current->objectData.line;
                        if (pickLine(line, clicked_point, TOLERANCY)) {
                            if (current_time - last_click_time < DOUBLE_CLICK_THRESHOLD) {
                                rotation_mode = !rotation_mode;
                            }
                            last_click_time = current_time;
                            selected_object = current;
                            dragging = 1;
                            last_mouse_position.x = clicked_point.x;
                            last_mouse_position.y = clicked_point.y;
                            break;
                        }
                    } else if (current->type == POLYGON) {
                        Polygon poly = current->objectData.polygon;
                        if (pickPolygon(poly, clicked_point)) {
                            if (current_time - last_click_time < DOUBLE_CLICK_THRESHOLD) {
                                rotation_mode = !rotation_mode;
                            }
                            last_click_time = current_time;
                            selected_object = current;
                            dragging = 1;
                            last_mouse_position.x = clicked_point.x;
                            last_mouse_position.y = clicked_point.y;
                            break;
                        }
                    }
                    current = current->next;
                }
                displayInfo();
                glutPostRedisplay();
            } else {
                selected_object = 0;
                rotation_mode = 0;
                displayInfo();
                glutPostRedisplay();
            }

        } else if (state == GLUT_UP && selected_object != NULL) {
            dragging = 0;
            Point unclicked_point = convertScreenToOpenGL(x, y);
            if (unclicked_point.x > 50 && unclicked_point.y > 25) {
                float dx = unclicked_point.x - last_mouse_position.x;
                float dy = unclicked_point.y - last_mouse_position.y;
                translateObject(selected_object, dx, dy);
                glutPostRedisplay();
            }
        }
    } else if (creating_polygon == 1 && button == GLUT_RIGHT_BUTTON && state == GLUT_UP) {
        // Fecha o polígono quando o botão direito for clicado
        if (current_mode == MODE_CREATE_POLYGON && vertices_count > 2) {
            float rgb[3] = {0.0f, 1.0f, 1.0f};
            addPolygon(&object_list, temp_polygon_vertices, vertices_count, rgb);
            vertices_count = 0;
            creating_polygon = 0;
            displayInfo();
            glutPostRedisplay();
        }
    } else if (selected_object != NULL && state == GLUT_UP) {
        if (rotation_mode) {
            if (button == 3) {
                rotateObject(selected_object, 10.0f);
            } else if (button == 4) {
                rotateObject(selected_object, -10.0f);
            }
        } else {
            if (button == 3) {
                scaleObject(selected_object, 1.1f);
            } else if (button == 4) {
                scaleObject(selected_object, 0.9f);
            }
        }
        glutPostRedisplay();
    }
}

// Callback para redenrização
void display() {
    glClear(GL_COLOR_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glViewport(0, 0, glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));

    // Itera sobre a lista
    Object *current = object_list.head;
    while(current != NULL) {

        glColor3f(current->color[0], current->color[1], current->color[2]);
        if(current->type == POINT) {
            glBegin(GL_POINTS);
                glVertex2f(current->objectData.point.x, current->objectData.point.y);
            glEnd();
        }
        else if(current->type == LINE) {
            glBegin(GL_LINES);
                glVertex2f(current->objectData.line.start_line.x, current->objectData.line.start_line.y);
                glVertex2f(current->objectData.line.end_line.x, current->objectData.line.end_line.y);
            glEnd();
        }
        else if (current->type == POLYGON) {
            glBegin(GL_POLYGON);
            for(int i = 0; i < current->objectData.polygon.num_vertices; i++) {
                glVertex2f(current->objectData.polygon.vertices[i].x, current->objectData.polygon.vertices[i].y);
            }
            glEnd();
        }

        current = current->next;
    }

    // Desenha o rastro da linha em execução
    if(current_mode == MODE_CREATE_LINE && creating_line == 1) {
        // Cor temporária da linha
        glColor4f(0.0f, 0.0f, 0.0f, 0.4f);
        glBegin(GL_LINES);
            glVertex2f(first_point.x, first_point.y);
            glVertex2f(current_mouse_position.x, current_mouse_position.y);
        glEnd();
    }

    // Desenha o rastro do polígono em execução
    if(current_mode == MODE_CREATE_POLYGON && creating_polygon == 1) {
        // Cor temporária da linha
        glColor4f(0.0f, 0.0f, 0.0f, 0.4f);
        glBegin(GL_LINE_STRIP);
        for(int i = 0; i < vertices_count; i++) {
            glVertex2f(temp_polygon_vertices[i].x, temp_polygon_vertices[i].y);
        }

        glVertex2f(current_mouse_position.x, current_mouse_position.y);
        glEnd();
    }

    glColor3f(0.0, 0.0, 0.0);

    drawMenu();
    initMenu();
    displayInfo();

    // Troca os buffers para exibir o conteúdo
    glutSwapBuffers();
    glFlush();
}


// Função para redimensionar a janela
void reshape(int width, int height) {
    // Previne divisão por 0
    if(height == 0) {
        height = 1;
    }
    // Define a Viewport para cobrir toda a tela
    glViewport(0, 0, width, height);

    // Carrega a matriz de projeção
    glMatrixMode(GL_PROJECTION);
    // Carrega a matriz identidade
    glLoadIdentity();

    // Define a janela de recorte
    gluOrtho2D(0, width, 0, height);
    glEnable(GL_MULTISAMPLE);

    // Selecione a matriz de modelagem
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void init() {
    // Define a cor de fundo
    glClearColor(1.0f, 1.0f, 1.0f, 1.0);
    // Define a cor dos objetos
    glColor3f(0.0, 0.0, 0.0);
    // Tamanho dos pontos
    glPointSize(4.0);
    // Tamanho da linha
    glLineWidth(2.0f);

    loadIcons();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

    // Carrega a matriz de projeção
    glMatrixMode(GL_MODELVIEW);

}

// Função para criar um octágono com pontas afiadas (para dois octágonos menores)
void createSpikyOctagon(ObjectList *list, float cx, float cy, float radius, float sharpness, float rgb[3]) {
    Point vertices[8];
    for (int i = 0; i < 8; i++) {
        float angle = 2.0f * M_PI * i / 8.0f;  // Ângulo para cada vértice
        float spike = (i % 2 == 0) ? radius * sharpness : radius;  // Alterna entre raio e raio modificado para ponta
        vertices[i].x = cx + cos(angle) * spike;
        vertices[i].y = cy + sin(angle) * spike;
    }
    addPolygon(list, vertices, 8, rgb);  // Adiciona o polígono na lista
}

// Função para criar os dois octágonos e o retângulo grande
void createAnimatedPolygonsAndRectangle() {
    int window_width = glutGet(GLUT_WINDOW_WIDTH);
    int window_height = glutGet(GLUT_WINDOW_HEIGHT);

    // Calcula o centro da tela
    float center_x = window_width / 2.0f;
    float center_y = window_height / 2.0f;

    // Parâmetros para os octágonos
    float radius1 = 100.0f;
    float radius2 = 75.0f;
    float sharpness1 = 1.2f;
    float sharpness2 = 1.4f;

    // Cores para os octágonos
    float rgb1[3] = {1.0f, 0.0f, 0.0f};  // Vermelho
    float rgb2[3] = {0.0f, 1.0f, 0.0f};  // Verde

    // Cria os dois octágonos primeiro
    createSpikyOctagon(&object_list, window_width + 100, center_y, radius1, sharpness1, rgb1);  // Primeiro octágono
    createSpikyOctagon(&object_list, window_width + 100, center_y, radius2, sharpness2, rgb2);  // Segundo octágono

    // Após os octágonos, cria o retângulo maior (dentro da função de animação)
    createAnimatedPolygon();
}

// Função para criar o polígono animado (retângulo)
void createAnimatedPolygon() {
    int window_width = glutGet(GLUT_WINDOW_WIDTH);
    int window_height = glutGet(GLUT_WINDOW_HEIGHT);

    // Calcula o centro da tela
    float center_x = window_width / 2.0f;
    float center_y = window_height / 2.0f;

    // Largura e altura do polígono retangular
    float rect_width = window_width * 1.5f;
    float rect_height = window_height;

    // Cria um polígono retangular com o centro alinhado ao centro da tela
    Point vertices[4];
    
    // Vértices do retângulo alinhados com o centro da tela
    vertices[0].x = window_width;     // Ponto inicial à direita da tela (fora da tela)
    vertices[0].y = center_y + rect_height / 2.0f;   // Topo do polígono
    vertices[1].x = window_width + rect_width;      // Extensão para a direita
    vertices[1].y = center_y + rect_height / 2.0f;   // Topo
    vertices[2].x = window_width + rect_width;      // Extensão inferior direita
    vertices[2].y = center_y - rect_height / 2.0f;   // Base inferior
    vertices[3].x = window_width;     // Esquerda inferior
    vertices[3].y = center_y - rect_height / 2.0f;   // Base inferior

    // Cor vermelha para o polígono
    float rgb[3] = {1.0f, 0.0f, 0.0f};

    // Adiciona o polígono na lista e o torna animado
    addPolygon(&object_list, vertices, 4, rgb);

    // O polígono recém-criado será o último da lista (animação)
    animated_object = object_list.tail;

    // Define a velocidade de movimento para a animação (20 segundos)
    animation_speed = (float)window_width / (animation_duration / 1000.0f);
    is_animating = 1;  // Inicia a animação
    elapsed_time = 0;  // Reseta o tempo decorrido

    // Inicia o timer para a animação
    glutTimerFunc(16, animateObjects, 0);  // Atualiza a cada 16 ms (~60 FPS)
}

// Função de animação para todos os objetos
void animateObjects(int value) {
    if (is_animating && animated_object != NULL) {
        // Calcula o deslocamento da animação com base no tempo
        float dx = animation_speed * 16.0f / 1000.0f;  // Movimento por frame (~60 FPS)

        // Atualiza as coordenadas dos objetos animados
        for (int i = 0; i < animated_object->objectData.polygon.num_vertices; i++) {
            animated_object->objectData.polygon.vertices[i].x -= dx;  // Move para a esquerda
        }

        // Atualiza a cor do polígono com base no tempo decorrido
        float t = (float)elapsed_time / (float)animation_duration;  // Valor normalizado entre 0 e 1
        animated_object->color[0] = t;                          // Vermelho aumenta linearmente
        animated_object->color[1] = fabs(sin(t * 3.14159f));    // Verde oscila entre 0 e 1
        animated_object->color[2] = (1.0f - t);                 // Azul diminui linearmente    

        // Redesenha a tela com o polígono atualizado
        glutPostRedisplay();

        // Incrementa o tempo decorrido
        elapsed_time += 16;  // Aproximadamente 16 ms por frame

        // Se a animação ainda não terminou, continua
        if (elapsed_time < animation_duration) {
            glutTimerFunc(16, animateObjects, 0);  // Continua chamando a função a cada 16 ms (~60 FPS)
        } else {
            is_animating = 0;  // Finaliza a animação após 20 segundos
            printf("Animação concluída, limpando objetos.\n");

            // Limpa todos os objetos da lista quando a animação terminar
            clearObjectList(&object_list);
            selected_object = NULL;
            rotation_mode = 0;
            glutPostRedisplay();
            current_mode = MODE_SELECT;
        }
    }
}

// Função para alternar modos (ponto, linha e polígono)
void keyboard(unsigned char key, int x, int y) {
    switch(key) {
        case 's': if(current_mode != MODE_ANIMATE){selectMode();}break;
        case 'p': if(current_mode != MODE_ANIMATE){createPointMode();} break;
        case 'l': if(current_mode != MODE_ANIMATE){createLineMode();} break;
        case 'g': if(current_mode != MODE_ANIMATE){createPolygonMode();} break;
        case 't': printObjectList(&object_list); break;
        case 'r': if(current_mode != MODE_ANIMATE) {shearMode();} break;
        case 8:
            if(selected_object != NULL) {
                removeObject(&object_list, selected_object);
                selected_object = NULL;
                rotation_mode = 0;
                glutPostRedisplay();
            }
            printf("Objeto excluido com sucesso!\n"); break;
        case 127:
            clearObjectList(&object_list);
            selected_object = NULL;
            rotation_mode = 0;
            glutPostRedisplay();
            printf("Todos os objetos foram excluídos com sucesso!\n"); break;
        case 27:
            printf("Saindo do programa...\n");
            exit(0); break;
        case 'a':
            if(current_mode != MODE_ANIMATE){
                current_mode = MODE_ANIMATE;
                printf("Iniciando animação\n");
                createAnimatedPolygon();  // Cria o polígono animado
                is_animating = 1;  // Inicia a animação
                elapsed_time = 0;  // Reinicia o tempo da animação
                glutTimerFunc(16, animateObjects, 0);  // Configura o temporizador para a animação
            }
            break;
    }

}

int main(int argc, char** argv){
    // Inicializa o GLUT
    glutInit(&argc, argv);
    // Configura o modo de display
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB | GLUT_MULTISAMPLE);
    // Configura a largura e a altura da janela de exibição
    glutInitWindowSize(glutGet(GLUT_SCREEN_WIDTH)/2,glutGet(GLUT_SCREEN_HEIGHT)/2);
    //glutInitWindowPosition(200,0);

    // Cria a janela de exibição
    glutCreateWindow("Paint");

    // Executa a função de inicialização
    init();
    // Inicializa a lista duplamente encadeada dos objetos
    initObjectList(&object_list);

    // Estabelece as funções "reshape", "display", "mouse",  "mouseMotion" e "keyboard" como a funções callback
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutMouseFunc(mouse);
    glutMotionFunc(mouseMotion);
    glutKeyboardFunc(keyboard);
    glutPassiveMotionFunc(motion);

    // Mostre tudo e espere
    glutMainLoop();
    return 0;
}
