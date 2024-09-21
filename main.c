#include <stdio.h>
#include <stdlib.h>
#include <GL/glut.h>
#include <GL/freeglut.h>
#include "LDE.h"
#include "Structures.h"

#define TOLERANCY 5.0f // Raio de Seleção
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

// Lista duplamente encadeada que armazena os objetos
ObjectList object_list;

// Estado inicial
Mode current_mode = MODE_CREATE_POINT;

// Variável para armazenar o ponto selecionado
Object *selected_object = NULL;

// Lista temporária de vértices do polígono (max 100 vértices)
Point temp_polygon_vertices[100];
// Contador de vértices
int vertices_count = 0;
// Para armazenar o primeiro ponto ao criar uma linha
Point first_point;
// Armazena a posição atual do mouse
Point current_mouse_position;

// Flag para saber se estamos no processo de criar uma linha
int creating_line = 0;
// Flag para saber se estamos no processo de criar um polígono
int creating_polygon = 0;

// Função para converter coordenadas de janela para coordenadas OpenGL (-1, 1)
Point convertScreenToOpenGL(int x, int y) {
    Point p;
    int window_height = glutGet(GLUT_WINDOW_HEIGHT);

    p.x = x;
    // Inverte o Y, pois o OpenGL tem origem no canto inferior esquerdo
    p.y = window_height - (float)y;
    return p;
}

// Callback para eventos de clique do mouse
void mouse(int button, int state, int x, int y) {
    if(current_mode != MODE_SELECT && button == GLUT_LEFT_BUTTON && state ==  GLUT_UP) {
        Point p = convertScreenToOpenGL(x, y);
        printf("Coordenadas do Mouse; (%f, %f)\n", p.x, p.y);

        if(p.x > 50 && p.y < glutGet(GLUT_WINDOW_HEIGHT) - 5) {
            if(current_mode == MODE_CREATE_POINT) {
                // Adiciona um ponto à lista
                vertices_count = 0;
                creating_polygon = 0;
                creating_line = 0;
                addPoint(&object_list, p.x, p.y);
            }
            else if(current_mode == MODE_CREATE_LINE) {
                if(creating_line == 0) {
                    // Primeira metade da linha (captura do primeiro ponto)
                    creating_polygon = 0;
                    first_point = p;
                    vertices_count = 0;
                    creating_line = 1;
                }
                else {
                    // Segunda metade da linha (captura do segundo ponto)
                    printf("First_point: (%f, %f), p: (%f, %f).\n", first_point.x, first_point.y, p.x, p.y);
                    addLine(&object_list, first_point, p);
                    creating_line = 0;
                }

            }
            else if(current_mode == MODE_CREATE_POLYGON) {
                // Adiciona um vértice temporário ao polígono
                creating_line = 0;
                temp_polygon_vertices[vertices_count] = p;
                vertices_count++;
                creating_polygon = 1;

                // Verifica se o usuário clicou no primeiro ponto
                if(vertices_count > 2 && isCloseEnough(p, temp_polygon_vertices[0])) {
                    addPolygon(&object_list, temp_polygon_vertices, vertices_count);
                    vertices_count = 0;
                    creating_polygon = 0;
                }
            }

            glutPostRedisplay();
        }
    }
    else if(current_mode == MODE_SELECT && button == GLUT_LEFT_BUTTON && state ==  GLUT_DOWN) {
        Point clicked_point = convertScreenToOpenGL(x, y);
        printf("Coordenadas do Mouse; (%f, %f)\n", clicked_point.x, clicked_point.y);
        Object *current = object_list.head;
        selected_object = NULL; // Resetar a seleção anterior
        while(current != NULL) {
            if(current->type == POINT) {
                Point p = current->objectData.point;
                // Usa a função pickPoint para verificar a seleção
                if(pickPoint(p, clicked_point, TOLERANCY)) {
                    selected_object = current;
                    printf("Ponto selecionado: (%f, %f)\n", current->objectData.point.x, current->objectData.point.y);
                    break;
                }
            }
            else if(current->type == LINE) {
                Line line = current->objectData.line;
                // Usa a função pickLine para verificar a seleção
                if(pickLine(line, clicked_point, TOLERANCY)) {
                    selected_object = current;
                    printf("Linha selecionada: (%f, %f) a (%f, %f)\n", current->objectData.line.start_line.x, current->objectData.line.start_line.y, current->objectData.line.end_line.x, current->objectData.line.end_line.y);
                    break;
                }
            }
            else if(current->type == POLYGON) {
                Polygon poly = current->objectData.polygon;
                // Usa a função pickPolygon para verificar a seleção
                if(pickPolygon(poly, clicked_point)) {
                    selected_object = current;
                    printf("Polígono selecionado com %d vértices\n", current->objectData.polygon.num_vertices);
                    break;
                }
            }
            current = current->next;
        }
    }
    else if(current_mode == MODE_ERASER && button == GLUT_LEFT_BUTTON && state ==  GLUT_DOWN) {
        Point clicked_point = convertScreenToOpenGL(x, y);
        printf("Coordenadas do Mouse; (%f, %f)\n", clicked_point.x, clicked_point.y);
        Object *current = object_list.head;
        selected_object = NULL; // Resetar a seleção anterior
        while(current != NULL) {
            if(current->type == POINT) {
                Point p = current->objectData.point;
                // Usa a função pickPoint para verificar a seleção
                if(pickPoint(p, clicked_point, TOLERANCY)) {
                    removeObject(&object_list, current);
                    selected_object = NULL;
                    break;
                }
            }
            else if(current->type == LINE) {
                Line line = current->objectData.line;
                // Usa a função pickLine para verificar a seleção
                if(pickLine(line, clicked_point, TOLERANCY)) {
                    removeObject(&object_list, current);
                    selected_object = NULL;
                    break;
                }
            }
            else if(current->type == POLYGON) {
                Polygon poly = current->objectData.polygon;
                // Usa a função pickPolygon para verificar a seleção
                if(pickPolygon(poly, clicked_point)) {
                    removeObject(&object_list, current);
                    selected_object = NULL;
                    break;
                }
            }
            current = current->next;
        }
            // Redesenha a tela
            glutPostRedisplay();
    }
    else if(creating_polygon == 1 && button == GLUT_RIGHT_BUTTON && state ==  GLUT_UP) {
        // Fecha o polígono quando o botão direito for clicado
        if(current_mode == MODE_CREATE_POLYGON && vertices_count > 2) {
            addPolygon(&object_list, temp_polygon_vertices, vertices_count);

            vertices_count = 0;
            creating_polygon = 0;

            // Redesenha a tela
            glutPostRedisplay();
        }
    }
}

void motion(int x, int y) {
    current_mouse_position = convertScreenToOpenGL(x,y);
    if(creating_polygon == 1 || creating_line == 1) {
        glutPostRedisplay();
    }
}

// Função para renderizar o texto do modo atual da tela
void renderModeText() {
    char *mode_text;
    switch(current_mode) {
        case MODE_CREATE_POINT: mode_text = "Modo de Criação de Pontos"; break;
        case MODE_CREATE_LINE: mode_text = "Modo de Criação de Linhas"; break;
        case MODE_CREATE_POLYGON: mode_text = "Modo de Criação de Polígonos"; break;
        case MODE_SELECT: mode_text = "Modo de Seleção"; break;
    }
}

void drawDrawingArea() {
    // Cor da Borda
    glColor3f(1.0, 1.0, 1.0);
    glBegin(GL_QUADS);
        glVertex2f(50, 0);
        glVertex2f(glutGet(GLUT_WINDOW_WIDTH), 0);
        glVertex2f(glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT) - 4);
        glVertex2f(50, glutGet(GLUT_WINDOW_HEIGHT) - 4);
    glEnd();
}

void drawMenu() {
    // Cor de fundo do menu
    glColor3f(0.9, 0.9, 0.9);
    glBegin(GL_QUADS);
        glVertex2f(0, 0);
        glVertex2f(50, 0);
        glVertex2f(50, glutGet(GLUT_WINDOW_HEIGHT));
        glVertex2f(0, glutGet(GLUT_WINDOW_HEIGHT));
    glEnd();
}

// Callback para redenrização
void display() {
    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();

    drawDrawingArea();
    drawMenu();
    glColor3f(0.0, 0.0, 0.0);

    // Itera sobre a lista
    Object *current = object_list.head;
    while(current != NULL) {
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

    renderModeText();

    // Troca os buffers para exibir o conteúdo
    glutSwapBuffers();
    glFlush();
}

void reshape(int width, int height) {
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

int init() {
    // Define a cor de fundo
    glClearColor(0.9, 0.9, 0.9, 1.0);
    // Define a cor dos objetos
    glColor3f(0.0, 0.0, 0.0);
    // Tamanho dos pontos
    glPointSize(4.0);
    // Tamanho da linha
    glLineWidth(2.0f);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glViewport(0, 0, glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, glutGet(GLUT_WINDOW_WIDTH), 0, glutGet(GLUT_WINDOW_HEIGHT));

    // Carrega a matriz de projeção
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

}

// Função para alternar modos (ponto, linha e polígono)
void keyboard(unsigned char key, int x, int y) {
    if(key == 'p') {
        current_mode = MODE_CREATE_POINT;
        printf("Modo de criação de pontos ativado.\n");
        creating_line = 0;
    }
    else if(key == 'l') {
        current_mode = MODE_CREATE_LINE;
        printf("Modo de criação de linhas ativado.\n");
    }
    else if(key == 'g') {
        current_mode = MODE_CREATE_POLYGON;
        printf("Modo de criação de polígonos ativado.\n");
    }
    // Para testes da lista
    else if(key == 't') {
        printObjectList(&object_list);
    }
    // Para testes da lista
    else if(key == 's') {
        current_mode = MODE_SELECT;
        printf("Modo de seleção ativado.\n");
    }
    else if(key == 'e') {
        current_mode = MODE_ERASER;
        printf("Modo de exclusão ativado.\n");
    }
}

int main(int argc, char** argv){
    // Inicializa o GLUT
    glutInit(&argc, argv);
    // Configura o modo de display
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB | GLUT_MULTISAMPLE);
    // Configura a largura e a altura da janela de exibição
    glutInitWindowSize(800,600);
    // glutInitWindowPosition(200,0);

    // Cria a janela de exibição
    glutCreateWindow("Paint");

    // Executa a função de inicialização
    init();
    // Inicializa a lista duplamente encadeada dos objetos
    initObjectList(&object_list);


    // Estabelece as funções "reshape", "display", "mouse" e "keyboard" como a funções callback
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutMouseFunc(mouse);
    glutKeyboardFunc(keyboard);
    glutPassiveMotionFunc(motion);

    // Mostre tudo e espere
    glutMainLoop();
    return 0;
}
