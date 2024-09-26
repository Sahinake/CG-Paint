
# Projeto de Aplicação Gráfica em OpenGL

Este projeto implementa uma aplicação gráfica interativa utilizando OpenGL em C, permitindo a criação de pontos, linhas e polígonos. Além disso, há funcionalidades para salvar e carregar os objetos desenhados em/de um arquivo, e um menu interativo que facilita a navegação e a manipulação dos objetos gráficos.

## Funcionalidades
* ### Desenho de Objetos:
    **Ponto:** Desenho de pontos na tela
    
    **Linha:** Desenho de linhas conectando dois pontos.
   
   **Polígono:** Desenho de polígonos de qualquer número de vértices.

* ### Menu Interativo:

    O menu pode ser aberto/fechado pressionando a tecla **Esc**. Contém opções de Salvar e Carregar arquivos.

* ### Salvamento e Carregamento:
    Os objetos desenhados podem ser salvos em um arquivo de texto. Arquivos salvos podem ser carregados para recriar a cena.

* ### Transformações Geométricas:
    Translação, Escala, Rotação, Cisalhamento e Refexão são as transformações que podem ser aplicadas nos objetos.

## Como Executar
### Requisitos
* Compilador C (gcc ou equivalente).
* OpenGL e GLUT instalados no sistema.

## Compilação e Execução
* Clone o repositório:

``` 
git clone https://github.com/Sahinake/CG-Paint.git
```
* Compile-o dentro do diretório do projeto. Dpis exemplos de possíveis comandos que podem ser usados para compilar o programa:
```
gcc main.c LDE.c structures.c textureloader.c saveload.c -o paint -lGL -lGLU -lglut -lm -lfreetype
```
Ou
```
gcc main.c LDE.c structures.c textureloader.c saveload.c -o paint -L/lib/x86_64-linux-gnu -lGL -lGLU -lglut -lm -lfreetype
```

* Execute o programa
```
./paint
```

## Controles
* **Esc:** Abre/fecha o menu.
* **Clique do mouse:** Interage com a interface, seleciona modos e desenha objetos.
* **Teclas numéricas:** Alteram o modo de interação (por exemplo, desenho de pontos, linhas, etc.).
    
## Estrutura do Projeto
* **main.c**: Arquivo principal que inicializa o OpenGL e contém o loop principal do programa.
* **saveload.c:** Funções responsáveis por adicionar, salvar e carregar objetos.
* **LDE.c:** Funções relacionadas à estrutura de dados utilizada para armazenar os objetos primários.
* **structures.c:** Funções relacionadas à manipulação dos objetos primários e de todas as estruturas.
* **textureloader.c:** Função auxiliar utilizada para carregar as texturas dos ícones.

## Como Salvar e Carregar Objetos
### Salvamento
Para salvar os objetos desenhados, basta abrir o menu **(tecla Esc)** e clicar no botão "Salvar".
Um arquivo será gerado no diretório de execução do programa com a lista dos objetos.

### Carregamento
Para carregar um arquivo com objetos salvos, clique no botão "Carregar" no menu.
O arquivo deve seguir o formato esperado pelo programa (veja o exemplo abaixo).
#### Formato do Arquivo de Salvamento
Os objetos são salvos em um arquivo de texto com o seguinte formato:
* Ponto
```
POINT <x> <y>
```
* Linha
```
LINE <x1> <y1> <x2> <y2>
```
* Polígono
```
POLYGON <num_vertices> <x1> <y1> <x2> <y2> ... <xn> <yn>
```
**Exemplo de Arquivo salvo**
```
POINT 100.00 200.00
LINE 50.00 50.00 150.00 150.00
POLYGON 3 50.00 50.00 100.00 100.00 150.00 50.00
```



