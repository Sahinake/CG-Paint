#include <stdio.h>
#include <stdlib.h>
#inclide <GL/glut.h>
#include <GL/freeglut.h>

int init(void) {
    // Define a cor de fundo
    glClearColor(1.0, 1.0, 1.0, 0.0);

    // Carrega a matriz de projeção
    glMatrixMode(GL_PROJECTION);
    // Define a projeção ortogonal 2D que mapeia os objetos da coordenada do mundo para coordenadas da tela
    gluOrtho2D(0.0, 200.0, 0.0, 150.0);
}

void display(void) {
    // Desenha o fundo (limpa a janela)
    glClear(GL_COLOR_BUFFER_BIT);

    // Carrega a matriz de modelo
    glMatrixMode(GL_MODELVIEW);


    // Desenha os comandos não executados
    glFlush();
}

int main(int argc, char** argv){
    // Inicializa o GLUT
    glutInit(&argc, argv);
    // Configura o modo de display
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
    // Configura a largura e a altura da janela de exibição
    glutInitWindowSize(400,300);
    glutInitWindowPosition(200,0);

    // Cria a janela de exibição
    glutCreateWindow("Paint");

    // Executa a função de inicialização
    init();
    // Estabelece a função "display" como a função callback de exibição
    glutDisplayFunc(display);
    // Mostre tudo e espere
    glutMainLoop();
    return 0;
}
