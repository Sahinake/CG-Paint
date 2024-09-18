#include <stdio.h>
#include <stdlib.h>
#include <GL/glut.h>
#include <GL/freeglut.h>
#include "LDE.h"

// Lista duplamente encadeada que armazena os objetos
ObjectList object_list;

// Para armazenar o primeiro ponto ao criar uma linha
Point first_point;
// Flag para saber se estamos no processo de criar uma linha
int first_point_line = 1;
// Modo atual: POINT, LINE ou POLYGON
int mode = POINT;

// Função para converter coordenadas de janela para coordenadas OpenGL (-1, 1)
Point convertScreenToOpenGL(int x, int y) {
    Point p;
    int window_width = glutGet(GLUT_WINDOW_WIDTH);
    int window_height = glutGet(GLUT_WINDOW_HEIGHT);

    p.x = (2.0f * x) / window_width - 1.0f;
    p.y = 1.0f - (2.0f * y) / window_height;
    return p;
}

// Callback para eventos de clique do mouse
void mouse(int button, int state, int x, int y) {
    if(button == GLUT_LEFT_BUTTON && state ==  GLUT_UP) {
        Point p = convertScreenToOpenGL(x, y);
        printf("Coordenadas do Mouse; (%f, %f)\n", p.x, p.y);

        if(mode == POINT) {
        // Adiciona um ponto à lista
        addPoint(&object_list, p.x, p.y);
        }
        else if(mode == LINE) {
            if(first_point_line) {
                // Primeira metade da linha (captura do primeiro ponto)
                first_point = p;
                first_point_line = 0;
            }
            else {
                // Segunda metade da linha (captura do segundo ponto)
                printf("First_point: (%f, %f), p: (%f, %f).\n", first_point.x, first_point.y, p.x, p.y);
                addLine(&object_list, first_point, p);
                first_point_line = 1;
            }

        }
        else if(mode == POLYGON) {
            //
        }
    }

    // Redesenha a tela
    glutPostRedisplay();
}

// Callback para redenrização
void display() {
    glClear(GL_COLOR_BUFFER_BIT);

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
            //
        }

        current = current->next;
    }
    glFlush();

    // Troca os buffers para exibir o conteúdo
    glutSwapBuffers();
}

void setupProjection() {
    // Carrega a matriz de projeção
    glMatrixMode(GL_PROJECTION);
    // Carrega a matriz identidade
    glLoadIdentity();

    // Define a janela de recorte
    gluOrtho2D(-1.0, 1.0, -1.0, 1.0);
}

void reshape(int width, int height) {
    // Define a Viewport para cobrir toda a tela
    glViewport(0, 0, width, height);

    // Carrega a matriz de projeção
    glMatrixMode(GL_PROJECTION);
    // Carrega a matriz identidade
    glLoadIdentity();

    // Define a janela de recorte
    gluOrtho2D(-1.0, 1.0, -1.0, 1.0);

    // Selecione a matriz de modelagem
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

int init() {
    // Define a cor de fundo
    glClearColor(1.0, 1.0, 1.0, 0.0);

    // Define a cor dos objetos
    glColor3f(0.0, 0.0, 0.0);

    // Tamanho dos pontos
    glPointSize(5.0);

    // Carrega a matriz de projeção
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();



}

// Função para alternar modos (ponto, linha e polígono)
void keyboard(unsigned char key, int x, int y) {
    if(key == 'p') {
        mode = POINT;
        first_point_line = 0;
    }
    else if(key == 'l') {
        mode = LINE;
    }
    else if(key == 'g') {
        mode = POLYGON;
    }
}

int main(int argc, char** argv){
    // Inicializa o GLUT
    glutInit(&argc, argv);
    // Configura o modo de display
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
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
    glutReshapeFunc(reshape);
    glutDisplayFunc(display);
    glutMouseFunc(mouse);
    glutKeyboardFunc(keyboard);

    setupProjection();

    // Mostre tudo e espere
    glutMainLoop();
    return 0;
}
