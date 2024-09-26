#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <GL/glut.h>
#include <GL/freeglut.h>
#include <sys/time.h>
#include <math.h>

#include "LDE.h"
#include "structures.h"
#include "textureloader.h"
#include "saveload.h"

#define DOUBLE_CLICK_THRESHOLD 500      //500 milissegundos para detecção do duplo clique
#define TOLERANCY 5.0f                  // Raio de Seleção
#define WINDOW_WIDTH 800                // Tamanho inicial da janela do OpenGL
#define WINDOW_HEIGHT 600
#define NUM_BUTTONS 17                  // O número de botões
#define M_PI 3.14159265358979323846

// Lista duplamente encadeada que armazena os objetos
ObjectList object_list;
// Estado inicial
Mode current_mode = MODE_CREATE_POINT;
// Variável para armazenar o ponto selecionado
Object *selected_object = NULL;
// Botões
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
// Flag para saber se o menu está aberto ou não
int menu_open = 1;
// Flag para saber se o objeto está sendo arrastado
int dragging = 0;
// Flag para saber se o objeto está sendo deformado
int shearing = 0;
// Armazena o tempo do primeiro clique
int last_click_time = 0;
// Índice que armazena a cor atualmente selecionada
int selected_color_index;
// Flag para controlar a animação
int is_animating = 0;          

GLuint icons[10];

Color colors[] = {
    {1.0f, 0.0f, 0.0f}, // Vermelho
    {0.0f, 1.0f, 0.0f}, // Verde
    {0.0f, 0.0f, 1.0f}, // Azul
    {1.0f, 1.0f, 0.0f}, // Amarelo
    {1.0f, 0.5f, 0.0f}, // Laranja
    {0.5f, 0.0f, 0.5f}, // Roxo
};

void loadIcons() {
    icons[0] = loadTexture("selection.png");    // Ícone de Seleção
    icons[1] = loadTexture("pencil.png");       // Ícone de Ponto
    icons[2] = loadTexture("line-segment.png"); // Ícone de Linha
    icons[3] = loadTexture("polygon.png");      // Ícone de Polígono
    icons[4] = loadTexture("shear.png");        // Ícone de Cisalhamento
    icons[5] = loadTexture("reflectX.png");     // Ícone de Reflexão em X
    icons[6] = loadTexture("reflextY.png");     // Ícone de Reflexão em Y
    icons[7] = loadTexture("broom.png");        // Ícone de Limpar a tela
    icons[8] = loadTexture("stickman.png");     // Ícone da animação
    icons[9] = loadTexture("nyan_cat.png");     // Gato da animação
}

// Variáveis para controlar o movimento e estado do botão
float imageSpeed = 0.0f;  // Velocidade de movimento da imagem
float imageWidth = 108.0f;   // Largura da imagem em unidades OpenGL
float imagePosX = -108.0f;  // Posição inicial fora da tela (à esquerda)
float imageHeight = 66.0f;  // Altura da imagem em unidades OpenGL
Object *animated_object = NULL;  // Objeto que será animado
float animation_speed = 0.0f;    // Velocidade do movimento
float elapsed_time = 0;          // Tempo decorrido da animação
const int animation_duration = 10000;  // Duração total da animação (10 segundos)

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

// Função para renderizar o texto do modo atual da tela
void renderModeText(float x, float y, const char* text, void *font) {
    glRasterPos2f(x, y);
    while(*text) {
        glutBitmapCharacter(font, *text);
        text++;
    }
}

void update(int value) {
    if (is_animating == 1) {
        // Obter a largura da janela
        int windowWidth = glutGet(GLUT_WINDOW_WIDTH);

        // Modificar a velocidade do gatinho baseado na largura da janela
        // Aqui, você pode ajustar o fator multiplicativo como preferir
        imageSpeed = windowWidth / 625.0f;  // Exemplo: a velocidade é proporcional à largura da janela

        imagePosX += imageSpeed;  // Mover a imagem para a direita

        // Se a imagem sair completamente da tela à direita, parar a animação
        if (imagePosX > windowWidth + imageWidth) {
            is_animating = 0;  // Parar a animação quando a imagem desaparecer
        }

        glutPostRedisplay();  // Redesenhar a tela com a nova posição
    }

    // Continuar chamando a função de atualização
    glutTimerFunc(16, update, 0);  // Atualiza a cada 60 FPS aproximadamente
}

void drawImage() {
    // Ativar texturas
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, icons[9]);  // Usa a textura carregada
    
    // Desenhar a imagem como um quadrado com a textura
    glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_QUADS);
        // Corrigir a inversão trocando as coordenadas Y da textura
        glTexCoord2f(0.0f, 1.0f); glVertex2f(imagePosX, glutGet(GLUT_WINDOW_HEIGHT)/2 - imageHeight / 2);  // Inferior esquerdo
        glTexCoord2f(1.0f, 1.0f); glVertex2f(imagePosX + imageWidth, glutGet(GLUT_WINDOW_HEIGHT)/2 - imageHeight / 2);  // Inferior direito
        glTexCoord2f(1.0f, 0.0f); glVertex2f(imagePosX + imageWidth, glutGet(GLUT_WINDOW_HEIGHT)/2  + imageHeight / 2);  // Superior direito
        glTexCoord2f(0.0f, 0.0f); glVertex2f(imagePosX, glutGet(GLUT_WINDOW_HEIGHT)/2 + imageHeight / 2);  // Superior esquerdo
    glEnd();
    
    glDisable(GL_TEXTURE_2D);
}


void saveProject() {
    writeFile(&object_list, "Backup");
    printf("Dados salvos!");
}

void loadProject() {
    clearObjectList(&object_list);
    readFile(&object_list, "Backup");
    menu_open = 0;
    glutPostRedisplay();
}

void setRedColor() {
    selected_color_index = 0;
    printf("Set Red Color\n");
}
void setGreenColor() {
    selected_color_index = 1;
    printf("Set Green Color\n");
}
void setBlueColor() {
    selected_color_index = 2;
    printf("Set Blue Color\n");
}
void setYellowColor() {
    selected_color_index = 3;
    printf("Set Yellow Color\n");
}
void setOrangeColor() {
    selected_color_index = 4;
    printf("Set Orange Color\n");
}
void setPurpleColor() {
    selected_color_index = 5;
    printf("Set Purple Color\n");
}

void drawColorButtons() {
    int numColors = sizeof(colors) / sizeof(colors[0]);
    void (*action[6])() = {setRedColor, setGreenColor, setBlueColor, setYellowColor, setOrangeColor, setPurpleColor};
    float button_size = 17.5f;
    float spacing = 5.0f;
    float menu_width = 50.0f;
    float starY = 40.0f;
    int count = 0;

    for (int i = 0; i < numColors; i++) {
        // Define a posição do botão
        float x = 5.0f + (button_size + spacing) * count;
        float y = starY + (button_size + spacing) * (i / 2);

        // Desenha o quadrado de cor
        glColor3f(colors[i].r, colors[i].g, colors[i].b);
        glBegin(GL_QUADS);
            glVertex2f(x, y);
            glVertex2f(x + button_size, y);
            glVertex2f(x + button_size,y + button_size);
            glVertex2f(x, y + button_size);
        glEnd();

        // Desenha o contorno
        if (i == selected_color_index) {
            glColor3f(0.2f, 0.2f, 0.2f); // Contorno
            glBegin(GL_LINE_LOOP);
                glVertex2f(x, y);
                glVertex2f(x + button_size, y);
                glVertex2f(x + button_size, y + button_size);
                glVertex2f(x, y + button_size);
            glEnd();
        }
        count++;
        if(x >= 27.5f) count = 0;
    }
    starY = starY + button_size;
    buttons[11] = (Button){5.0, starY, button_size, button_size, 0, action[0]};
    buttons[12] = (Button){5.0 + button_size + spacing, starY, button_size, button_size, 0, action[1]};
    buttons[13] = (Button){5.0 , starY + button_size + spacing, button_size, button_size, 0, action[2]};
    buttons[14] = (Button){5.0 + button_size + spacing, starY + button_size + spacing, button_size, button_size, 0, action[3]};
    buttons[15] = (Button){5.0 , starY + 2 * (button_size + spacing), button_size, button_size, 0, action[4]};
    buttons[16] = (Button){5.0 + button_size + spacing, starY + 2 * (button_size + spacing), button_size, button_size, 0, action[5]};
}


// Função para desenhar os botões de Salvar/Carregar arquivo
void drawSaveLoadButton(float x, float y, float width, float height, const char* label) {
    float text_x = x + (width / 2) - (glutBitmapLength(GLUT_BITMAP_9_BY_15, (const unsigned char*)label) / 2);
    float text_y = y - (height / 2) + 5;
    void *font = GLUT_BITMAP_9_BY_15;
    // Desenha o quadrado para o botão
    glColor3f(0.9f, 0.9f, 0.9f);
    glBegin(GL_QUADS);
        glVertex2f(x, y);
        glVertex2f(x + width, y);
        glVertex2f(x + width, y - height);
        glVertex2f(x, y - height);
    glEnd();

    // Desenha o contorno do botão
    glColor3f(0.2f, 0.2f, 0.2f);
    glBegin(GL_LINE_LOOP);
        glVertex2f(x, y);
        glVertex2f(x + width, y);
        glVertex2f(x + width, y - height);
        glVertex2f(x, y - height);
    glEnd();

    // Desenha o texto do botão
    glColor3f(0.0f, 0.0f, 0.0f);
    renderModeText(text_x, text_y, label, font);
}

// Função para criar o menu
void drawSaveLoadMenu() {
    if(menu_open == 1) {
        float button_width = 200.0f;
        float button_height = 50.0f;
        float button_spacing = 20.0f;
        void *font = GLUT_BITMAP_9_BY_15;
        void (*action[2])() = {saveProject, loadProject};

        // Calcular a posição centralizada
        float menu_x = (glutGet(GLUT_WINDOW_WIDTH) / 2) - (button_width / 2);
        float menu_y = (glutGet(GLUT_WINDOW_HEIGHT) / 2) + (button_height / 2) + 30;

        // Dimensões do fundo
        float background_width = button_width + 40;
        float background_height = (2 * button_height) + button_spacing + 60;

        // Coordenadas do fundo centralizado
        float background_x = (glutGet(GLUT_WINDOW_WIDTH) / 2) - (background_width / 2);
        float background_y = (glutGet(GLUT_WINDOW_HEIGHT) / 2) + (background_height / 2);

        // Desenha o quadrado para o menu
        glColor3f(0.6f, 0.6f, 0.6f);
        glBegin(GL_QUADS);
            glVertex2f(background_x, background_y);
            glVertex2f(background_x + background_width, background_y);
            glVertex2f(background_x + background_width, background_y - background_height);
            glVertex2f(background_x, background_y - background_height);
        glEnd();

        // Desenha o contorno para o menu
        glColor3f(0.2f, 0.2f, 0.2f);
        glBegin(GL_LINE_LOOP);
            glVertex2f(background_x, background_y);
            glVertex2f(background_x + background_width, background_y);
            glVertex2f(background_x + background_width, background_y - background_height);
            glVertex2f(background_x, background_y - background_height);
        glEnd();

        // Desenha o título do menu
        float title_x = (glutGet(GLUT_WINDOW_WIDTH) / 2) - (glutBitmapLength(GLUT_BITMAP_9_BY_15, (const unsigned char*)"Menu") / 2);
        float title_y = background_y - 20;
        glColor3f(1.0f, 1.0f, 1.0f);
        renderModeText(title_x, title_y, "Menu", font);

        drawSaveLoadButton(menu_x, menu_y, button_width, button_height, "Save Project");
        drawSaveLoadButton(menu_x, menu_y - button_height - 20, button_width, button_height, "Load Project");
        buttons[9] = (Button){menu_x, menu_y, button_width, button_height, 0, action[0]};
        buttons[10] = (Button){menu_x, menu_y - button_height - 20, button_width, button_height, 0, action[1]};
    }
}

void selectMode() {
    current_mode = MODE_SELECT;
    printf("Modo de seleção ativado.\n");
    creating_line = 0;
    creating_polygon = 0;
    glutPostRedisplay();
}

void createPointMode() {
    current_mode = MODE_CREATE_POINT;
    printf("Modo de criação de pontos ativado.\n");
    creating_line = 0;
    creating_polygon = 0;
    glutPostRedisplay();
}

void createLineMode() {
    current_mode = MODE_CREATE_LINE;
    printf("Modo de criação de linhas ativado.\n");
    creating_line = 0;
    creating_polygon = 0;
    glutPostRedisplay();
}
void createPolygonMode() {
    current_mode = MODE_CREATE_POLYGON;
    printf("Modo de criação de polígonos ativado.\n");
    creating_line = 0;
    creating_polygon = 0;
    glutPostRedisplay();
}

void shearMode() {
    current_mode = MODE_SHEAR;
    printf("Modo de cisalhamento ativado.\n");
    creating_line = 0;
    creating_polygon = 0;
    glutPostRedisplay();
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

void cleanDrawView() {
    printf("Limpar a tela\n");
    clearObjectList(&object_list);
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

void callSaveLoadMenu() {
    selected_object = NULL;
    menu_open = (menu_open == 1) ? 0 : 1;
    glutPostRedisplay();
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

    if(menu_open) {
        x = 10;
        mode_text = "Objects will be saved in the Backup file!";
        renderModeText(x, y, mode_text, font);
    }
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
        glTexCoord2f(0.0f, 0.0f); glVertex2f(x, y);
        glTexCoord2f(1.0f, 0.0f); glVertex2f(x + width, y);
        glTexCoord2f(1.0f, 1.0f); glVertex2f(x + width, y - height);
        glTexCoord2f(0.0f, 1.0f); glVertex2f(x, y - height);
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
    void (*action[NUM_BUTTONS - 2])() = {selectMode, createPointMode, createLineMode, createPolygonMode, shearMode, reflectX, reflectY, cleanDrawView};

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

    for(int i = 0; i < NUM_BUTTONS - 8; i++) {
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
    drawColorButtons();
}

void initMenu() {
    float button_width = 40, button_height = 40;
    float x = 5.0f, y = glutGet(GLUT_WINDOW_HEIGHT) - 5;
    void (*action[NUM_BUTTONS - 2])() = {selectMode, createPointMode, createLineMode, createPolygonMode, shearMode, reflectX, reflectY, cleanDrawView};

    for(int i = 0; i < NUM_BUTTONS - 8; i++) {
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
    if(dragging && selected_object != NULL) {
        // Converte as coordenadas da tela para coordenadas OpenGL
        Point moved_point = convertScreenToOpenGL(x, y);

        // Calcula o deslocamento desde a última posição
        float dx = moved_point.x - last_mouse_position.x;
        float dy = moved_point.y - last_mouse_position.y;

        // Atualiza a posição do objeto selecionado em tempo real
        translateObject(selected_object, dx, dy);

        // Armazena a última posição como a última posição do mouse
        last_mouse_position = moved_point;

        //Redesenha a tela
        glutPostRedisplay();
    }
}

// Função de animação para todos os objetos
void animateObjects(int value) {
    if (is_animating && animated_object != NULL) {
        // Calcula o deslocamento da animação com base no tempo
        float dx = animation_speed * 16.0f / 1000.0f;  // Movimento por frame (~60 FPS)

        // Atualiza as coordenadas dos objetos animados
        for (int i = 0; i < animated_object->objectData.polygon.num_vertices; i++) {
            animated_object->objectData.polygon.vertices[i].x += dx;  // Move para a esquerda
        }

        // Atualiza a cor do polígono com base no tempo decorrido
        float t = (float)elapsed_time / (float)animation_duration;  // Valor normalizado entre 0 e 1
        animated_object->color.r = t;  // Vermelho aumenta linearmente
        animated_object->color.g = fabs(sin(t * 3.14159f)); // Verde oscila entre 0 e 1
        animated_object->color.b = (1.0f - t); // Azul diminui linearmente  

        // Redesenha a tela com o polígono atualizado
        glutPostRedisplay();

        // Incrementa o tempo decorrido
        elapsed_time += 16;  // Aproximadamente 16 ms por frame

        glutTimerFunc(16, animateObjects, 0);  // Continua chamando a função a cada 16 ms (~60 FPS)
    }
    else {
        is_animating = 0;  // Finaliza a animação após 20 segundos
        printf("Animação concluída, limpando objetos.\n");

        // Limpa todos os objetos da lista quando a animação terminar
        clearObjectList(&object_list);
        selected_object = NULL;
        rotation_mode = 0;
        current_mode = MODE_SELECT;
        glutPostRedisplay();
    }
        
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
    vertices[0].x = -rect_width;     // Ponto inicial à esquerda da tela (fora da tela)
    vertices[0].y = center_y + rect_height / 2.0f;   // Topo do polígono
    vertices[1].x = 0;                // Extensão para a direita
    vertices[1].y = center_y + rect_height / 2.0f;   // Topo
    vertices[2].x = 0;                // Extensão inferior direita
    vertices[2].y = center_y - rect_height / 2.0f;   // Base inferior
    vertices[3].x = -rect_width;     // Esquerda inferior
    vertices[3].y = center_y - rect_height / 2.0f;   // Base inferior


    // Adiciona o polígono na lista e o torna animado
    addPolygon(&object_list, vertices, 4, colors[0]);

    // O polígono recém-criado será o último da lista (animação)
    animated_object = object_list.tail;

    // Define a velocidade de movimento para a animação (20 segundos)
    animation_speed = (float)window_width / (animation_duration / 1000.0f);
    is_animating = 1;  // Inicia a animação
    elapsed_time = 0;  // Reseta o tempo decorrido

    glutTimerFunc(16, animateObjects, 0);
}

// Callback para eventos de clique do mouse
void mouse(int button, int state, int x, int y) {
    if(menu_open) {
        if(button == GLUT_LEFT_BUTTON && state == GLUT_UP) {
            Point p = convertScreenToOpenGL(x, y);

            if(p.x >= buttons[9].x && p.x <= buttons[9].x + buttons[9].width && p.y <= buttons[9].y && p.y >= buttons[9].y - buttons[9].height) {
                rotation_mode = 0;
                buttons[9].action();
                glutPostRedisplay();
            }
            else if(p.x >= buttons[10].x && p.x <= buttons[10].x + buttons[10].width && p.y <= buttons[10].y && p.y >= buttons[10].y - buttons[10].height) {
                rotation_mode = 0;
                buttons[10].action();
                glutPostRedisplay();
            }
        }
    }
    else {
        if(button == GLUT_LEFT_BUTTON && state == GLUT_UP) {
            Point p = convertScreenToOpenGL(x, y);
            for(int i = 0; i < NUM_BUTTONS - 8; i++) {
                if(p.x >= buttons[i].x && p.x <= buttons[i].x + buttons[i].width && p.y <= buttons[i].y && p.y >= buttons[i].y - buttons[i].height) {
                    rotation_mode = 0;
                    buttons[i].action();
                    glutPostRedisplay();
                    break;
                }
            }
            for(int i = 11; i < NUM_BUTTONS; i++) {
                if(p.x >= buttons[i].x && p.x <= buttons[i].x + buttons[i].width && p.y <= buttons[i].y && p.y >= buttons[i].y - buttons[i].height) {
                    rotation_mode = 0;
                    buttons[i].action();
                    glutPostRedisplay();
                    break;
                }
            }
        }

        if(current_mode != MODE_SELECT && current_mode != MODE_SHEAR && button == GLUT_LEFT_BUTTON && state ==  GLUT_DOWN) {
            Point p = convertScreenToOpenGL(x, y);
            rotation_mode = 0;
            //printf("Coordenadas do Mouse; (%f, %f)\n", p.x, p.y);

            if(p.x > 50 && p.y > 25) {
                if(current_mode == MODE_CREATE_POINT) {
                    // Adiciona um ponto à lista
                    vertices_count = 0;
                    creating_polygon = 0;
                    creating_line = 0;
                    addPoint(&object_list, p.x, p.y, colors[selected_color_index]);
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
                        //printf("First_point: (%f, %f), p: (%f, %f).\n", first_point.x, first_point.y, p.x, p.y);
                        addLine(&object_list, first_point, p, colors[selected_color_index]);
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
                        addPolygon(&object_list, temp_polygon_vertices, vertices_count, colors[selected_color_index]);
                        vertices_count = 0;
                        creating_polygon = 0;
                    }
                }
            }
        }

        if(current_mode == MODE_SHEAR && button == GLUT_LEFT_BUTTON) {
            if(state == GLUT_DOWN) {
                Point clicked_point = convertScreenToOpenGL(x, y);
                int current_time = getTimeInMillis();

                if(selected_object != NULL) {
                    for(int i = 0; i < NUM_BUTTONS - 8; i++) {
                        if(clicked_point.x >= buttons[i].x && clicked_point.x <= buttons[i].x + buttons[i].width && clicked_point.y <= buttons[i].y && clicked_point.y >= buttons[i].y - buttons[i].height) {
                            rotation_mode = 0;
                            buttons[i].action();
                            glutPostRedisplay();
                            break;
                        }
                    }


                }

                Object *current = object_list.head;
                selected_object = NULL; // Resetar a seleção anterior
                rotation_mode = 0;

                while(current != NULL) {
                    if(current->type == POINT) {
                        Point p = current->objectData.point;
                        // Usa a função pickPoint para verificar a seleção
                        if(pickPoint(p, clicked_point, TOLERANCY)) {
                            if(current_time - last_click_time < DOUBLE_CLICK_THRESHOLD) {
                                rotation_mode = !rotation_mode;   // Alterna o modo de rotação
                                printf("Double click detected. Rotation mode: %s\n", rotation_mode ? "ON" : "OFF");
                            }
                            last_click_time = current_time;
                            selected_object = current;
                            printf("Ponto selecionado: (%f, %f)\n", current->objectData.point.x, current->objectData.point.y);
                            //printf("Last mouse position (%f, %f)\n", last_mouse_position.x, last_mouse_position.y);
                            break;
                        }
                    }
                    else if(current->type == LINE) {
                        Line line = current->objectData.line;
                        // Usa a função pickLine para verificar a seleção
                        if(pickLine(line, clicked_point, TOLERANCY)) {
                            if(current_time - last_click_time < DOUBLE_CLICK_THRESHOLD) {
                                rotation_mode = !rotation_mode;   // Alterna o modo de rotação
                                printf("Double click detected. Rotation mode: %s\n", rotation_mode ? "ON" : "OFF");
                            }
                            last_click_time = current_time;
                            selected_object = current;
                            printf("Linha selecionada: (%f, %f) a (%f, %f)\n", current->objectData.line.start_line.x, current->objectData.line.start_line.y, current->objectData.line.end_line.x, current->objectData.line.end_line.y);
                            shearing = 1;
                            last_mouse_position.x = clicked_point.x;
                            last_mouse_position.y = clicked_point.y;
                            break;
                        }
                    }
                    else if(current->type == POLYGON) {
                        Polygon poly = current->objectData.polygon;
                        // Usa a função pickPolygon para verificar a seleção
                        if(pickPolygon(poly, clicked_point)) {
                            if(current_time - last_click_time < DOUBLE_CLICK_THRESHOLD) {
                                rotation_mode = !rotation_mode;   // Alterna o modo de rotação
                                printf("Double click detected. Rotation mode: %s\n", rotation_mode ? "ON" : "OFF");
                            }
                            last_click_time = current_time;
                            selected_object = current;
                            printf("Polígono selecionado com %d vértices\n", current->objectData.polygon.num_vertices);
                            shearing = 1;
                            last_mouse_position.x = clicked_point.x;
                            last_mouse_position.y = clicked_point.y;
                            break;
                        }
                    }
                    current = current->next;
                }
            }
            else if(state == GLUT_UP && shearing == 1) {
                shearing = 0;
                Point unclicked_point = convertScreenToOpenGL(x, y);
                float shearx = (unclicked_point.x - last_mouse_position.x) * 0.002, sheary = (unclicked_point.y - last_mouse_position.y) * 0.002;
                printf("Cisalhamento de shx: %f e shy: %f\n", shearx, sheary);

                shearObject(selected_object, shearx, sheary);
            }
        }
        else if(current_mode == MODE_SELECT && button == GLUT_LEFT_BUTTON) {
            if(state == GLUT_DOWN) {
                Point clicked_point = convertScreenToOpenGL(x, y);
                printf("Coordenadas do Mouse; (%f, %f)\n", clicked_point.x, clicked_point.y);

                 if(selected_object != NULL) {
                    for(int i = 0; i < NUM_BUTTONS - 8; i++) {
                        if(clicked_point.x >= buttons[i].x && clicked_point.x <= buttons[i].x + buttons[i].width && clicked_point.y <= buttons[i].y && clicked_point.y >= buttons[i].y - buttons[i].height) {
                            rotation_mode = 0;
                            buttons[i].action();
                            glutPostRedisplay();
                            break;
                        }
                    }
                    for(int i = 11; i < NUM_BUTTONS; i++) {
                        if(clicked_point.x >= buttons[i].x && clicked_point.x <= buttons[i].x + buttons[i].width && clicked_point.y <= buttons[i].y && clicked_point.y >= buttons[i].y - buttons[i].height) {
                            rotation_mode = 0;
                            buttons[i].action();
                            glutPostRedisplay();
                            break;
                        }
                    }
                }

                if(clicked_point.x > 50 && clicked_point.y > 25) {
                    int current_time = getTimeInMillis();

                    Object *current = object_list.head;
                    selected_object = NULL; // Resetar a seleção anterior
                    rotation_mode = 0;

                    while(current != NULL) {
                        if(current->type == POINT) {
                            Point p = current->objectData.point;
                            // Usa a função pickPoint para verificar a seleção
                            if(pickPoint(p, clicked_point, TOLERANCY)) {
                                if(current_time - last_click_time < DOUBLE_CLICK_THRESHOLD) {
                                    rotation_mode = !rotation_mode;   // Alterna o modo de rotação
                                    printf("Double click detected. Rotation mode: %s\n", rotation_mode ? "ON" : "OFF");
                                }
                                last_click_time = current_time;
                                selected_object = current;
                                printf("Ponto selecionado: (%f, %f)\n", current->objectData.point.x, current->objectData.point.y);
                                dragging = 1;
                                last_mouse_position.x = clicked_point.x;
                                last_mouse_position.y = clicked_point.y;
                                //printf("Last mouse position (%f, %f)\n", last_mouse_position.x, last_mouse_position.y);
                                break;
                            }
                        }
                        else if(current->type == LINE) {
                            Line line = current->objectData.line;
                            // Usa a função pickLine para verificar a seleção
                            if(pickLine(line, clicked_point, TOLERANCY)) {
                                if(current_time - last_click_time < DOUBLE_CLICK_THRESHOLD) {
                                    rotation_mode = !rotation_mode;   // Alterna o modo de rotação
                                    printf("Double click detected. Rotation mode: %s\n", rotation_mode ? "ON" : "OFF");
                                }
                                last_click_time = current_time;
                                selected_object = current;
                                printf("Linha selecionada: (%f, %f) a (%f, %f)\n", current->objectData.line.start_line.x, current->objectData.line.start_line.y, current->objectData.line.end_line.x, current->objectData.line.end_line.y);
                                dragging = 1;
                                last_mouse_position.x = clicked_point.x;
                                last_mouse_position.y = clicked_point.y;
                                break;
                            }
                        }
                        else if(current->type == POLYGON) {
                            Polygon poly = current->objectData.polygon;
                            // Usa a função pickPolygon para verificar a seleção
                            if(pickPolygon(poly, clicked_point)) {
                                if(current_time - last_click_time < DOUBLE_CLICK_THRESHOLD) {
                                    rotation_mode = !rotation_mode;   // Alterna o modo de rotação
                                    printf("Double click detected. Rotation mode: %s\n", rotation_mode ? "ON" : "OFF");
                                }
                                last_click_time = current_time;
                                selected_object = current;
                                printf("Polígono selecionado com %d vértices\n", current->objectData.polygon.num_vertices);
                                dragging = 1;
                                last_mouse_position.x = clicked_point.x;
                                last_mouse_position.y = clicked_point.y;
                                break;
                            }
                        }
                        current = current->next;
                    }
                }
                else {
                    selected_object = NULL;
                    rotation_mode = 0;
                }

            }
            else if(state == GLUT_UP && selected_object != NULL) {
                Point clicked_point = convertScreenToOpenGL(x, y);
                if(clicked_point.x > 50 && clicked_point.y > 25) {
                    dragging = 0;
                    Point unclicked_point = convertScreenToOpenGL(x, y);
                    if(unclicked_point.x > 50 && unclicked_point.y > 25) {
                        float dx = unclicked_point.x - last_mouse_position.x, dy = unclicked_point.y - last_mouse_position.y;
                        //printf("Dx e Dy (%f, %f)\n", dx, dy);

                        translateObject(selected_object, dx, dy);
                    }
                }
            }
        }
        else if(creating_polygon == 1 && button == GLUT_RIGHT_BUTTON && state ==  GLUT_UP) {
            // Fecha o polígono quando o botão direito for clicado
            if(current_mode == MODE_CREATE_POLYGON && vertices_count > 2) {
                addPolygon(&object_list, temp_polygon_vertices, vertices_count, colors[selected_color_index]);

                vertices_count = 0;
                creating_polygon = 0;
            }
        }
        else if(selected_object != NULL && state == GLUT_UP) {
            if(rotation_mode) {
                if(button == 3) {
                    rotateObject(selected_object, 10.0f);
                }
                else if(button == 4) {
                    rotateObject(selected_object, -10.0f);
                }
            }
            else {
                if(button == 3) {
                    scaleObject(selected_object, 1.1f);
                }
                else if(button == 4) {
                    scaleObject(selected_object, 0.9f);
                }
            }
        }
    }

    displayInfo();
    glutPostRedisplay();
}

// Callback para redenrização
void display() {
    glClear(GL_COLOR_BUFFER_BIT);

    glColor3f(colors[selected_color_index].r, colors[selected_color_index].g, colors[selected_color_index].b);

    // Itera sobre a lista
    Object *current = object_list.head;
    while(current != NULL) {
        // Configurar a cor atual com base na cor do objeto
        glColor3f(current->color.r, current->color.g, current->color.b);

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
    drawSaveLoadMenu();
    drawColorButtons();
    if(is_animating == 1) {
        drawImage();
    }

    // Troca os buffers para exibir o conteúdo
    glutSwapBuffers();
    glFlush();
}

void reshape(int width, int height) {
    // Previne divisão por 0
    if(height == 0) {
        height = 1;
    }

    // Carrega a matriz de projeção
    glMatrixMode(GL_PROJECTION);
    // Carrega a matriz identidade
    glLoadIdentity();

    // Define a janela de recorte
    gluOrtho2D(0, width, 0, height);

    // Define a Viewport para cobrir toda a tela
    glViewport(0, 0, width, height);

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

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

    // Carrega a matriz de projeção
    glMatrixMode(GL_MODELVIEW);
}

// Função para alternar modos (ponto, linha e polígono)
void keyboard(unsigned char key, int x, int y) {
    switch(key) {
        case 's': selectMode(); is_animating = 0; break;
        case 'p': createPointMode(); is_animating = 0; break; 
        case 'l': createLineMode(); is_animating = 0; break; 
        case 'g': createPolygonMode(); is_animating = 0;  break; 
        case 'r': shearMode(); is_animating = 0; break; 
        case 't': printObjectList(&object_list); break;
        case 'a':
                if(menu_open == 0) {
                    is_animating = (is_animating == 1) ? 0 : 1;
                    cleanDrawView();
                    animated_object = NULL;
                    if(is_animating == 1) {
                        imagePosX = -1.0f;  // Posicionar o GIF no lado esquerdo
                        printf("Iniciando animação\n");
                        elapsed_time = 0;
                        createAnimatedPolygon();  // Cria o polígono animado
                    }
                }
                break;
        case 27: callSaveLoadMenu(); break;
        case 127: 
            cleanDrawView(); 
            selected_object = NULL;
            rotation_mode = 0;
            printf("Todos os objetos foram excluídos com sucesso!\n"); 
            break;
        case 8:
            if(selected_object != NULL) {
                removeObject(&object_list, selected_object);
                selected_object = NULL;
                rotation_mode = 0;
                glutPostRedisplay();
            }
            printf("Objeto excluido com sucesso!\n");
            break;
    }

}

int main(int argc, char** argv){
    // Inicializa o GLUT
    glutInit(&argc, argv);
    // Configura o modo de display
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB | GLUT_MULTISAMPLE);
    // Configura a largura e a altura da janela de exibição
    glutInitWindowSize(WINDOW_WIDTH,WINDOW_HEIGHT);
    //glutInitWindowPosition(200,0);

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
    glutMotionFunc(mouseMotion);
    glutKeyboardFunc(keyboard);
    glutPassiveMotionFunc(motion);
    glutTimerFunc(16, update, 0);      // Chamar a função de atualização periodicamente

    // Mostre tudo e espere
    glutMainLoop();
    return 0;
}
