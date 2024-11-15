
# Nyan Paint: Projeto de Aplicação Gráfica em OpenGL

Este projeto implementa uma aplicação gráfica interativa utilizando OpenGL em C, permitindo a criação de pontos, linhas e polígonos. Além disso, há funcionalidades para salvar e carregar os objetos desenhados em/de um arquivo, e um menu interativo que facilita a navegação e a manipulação dos objetos gráficos.

## Funcionalidades
* ### Desenho de Objetos:
  **Ponto:** Desenho de pontos na tela.
    
  **Linha:** Desenho de linhas conectando dois pontos.
   
  **Polígono:** Desenho de polígonos de qualquer número de vértices.
  
  **Círculo:** Desenho de círculos na tela.

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
* Você pode utilizar o Makefile, compilando e executando o programa utilizando os seguintes comandos:
```
make clean
make
make run
```
   Utilize o ```make clean``` para limpar arquivos temporários!
   
* Ou compile-o manualmente. Dois exemplos de possíveis comandos que podem ser usados para compilar o programa:
```
gcc src/main.c src/LDE.c src/structures.c src/textureloader.c src/saveload.c -o build/nyan -Iinclude -Ilib -lGL -lGLU -lglut -lm
```
```
gcc src/main.c src/LDE.c src/structures.c src/textureloader.c src/saveload.c -o build/nyan -L/lib/x86_64-linux-gnu -lGL -lGLU -lglut -lm
```

   Para executar o programa:
```
./build/nyan
```

## Controles
### Teclado
* **Esc:** Abre/fecha o menu.
* **s:** Ativa o Modo de Seleção de Objetos.
* **p:** Ativa o Modo de Criação de Pontos.
* **l:** Ativa o Modo de Criação de linhas.
* **g:** Ativa o Modo de Criação de Polígonos.
* **c:** Ativa o Modo de Criação de Círculos.
* **r:** Ativa o Modo de Cisalhamento.
* **d:** Aplica o algoritmo de Varredura de Graham.
* **a:** Ativa/Desativa a Animação Nyan.
* **Backspace:** Exclusão do objeto selecionado
* **Delete:** Limpa a tela.
### Mouse
* **Clique Direito do Mouse:** Interage com a interface e com os objetos, seleciona os botões, desenha objetos.
* **Clique Esquerdo do Mouse:** Uma vez no modo de Criação de Polígonos, é utilizado para finalizá-lo (vértices > 2).
* **Double Click:** Double Click em um objeto ativa/desativa o modo de Rotação.
* **Scroll do Mouse:** Em um objeto selecionado, ele aplica as transformações de Escala ou Rotação.

## Estrutura de diretórios
* **src/:** Contém os arquivos .c (e.g. main.c, LDE.c, etc.).
* **include/:** Contém os arquivos .h (cabeçalhos do projeto).
* **lib/:** Contém as bibliotecas externas (e.g. stb_image.h).
* **build/:** Diretório onde o executável será gerado.
* **assets/:** Armazena as texturas ou qualquer outro arquivo gráfico usado no projeto.
* **Backup:** Arquivo onde os objetos criados no Nyan são salvos.
* **Makefile:** Arquivo que facilita a compilação e gerenciamento do projeto.
* **.gitignore:** Para garantir que os arquivos desnecessários não sejam versionados no Git.

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
POINT <x> <y> <r> <g> <b>
```
* Linha
```
LINE <x1> <y1> <x2> <y2> <r> <g> <b>
```
* Polígono
```
POLYGON <num_vertices> <x1> <y1> <x2> <y2> ... <xn> <yn> <r> <g> <b>
```
* Círculo
```
CIRCLE <center.x> <center.y> <radius> <r> <g> <b>
```
**Exemplo de Arquivo salvo**
```
POINT 100.00 200.00 0.00 0.00 0.00
LINE 50.00 50.00 150.00 150.00 0.50 0.00 0.50
POLYGON 3 50.00 50.00 100.00 100.00 150.00 50.00 1.00 0.00 0.00
CIRCLE 294.00 539.00 24.35 1.00 0.75 0.80
```

## Possíveis Problemas de Compilação ou Execução
* O programa ainda não foi testado no Windows, então possivelmente tenha muitos problemas e ele não consiga executar corretamente.
* Se você está utilizando WSL e tiver problemas com o som, tente instalar como alternativa o Pulseaudio.
```
sudo apt-get install pulseaudio
```
* Caso o Nyan não abra no WSL2, tente usar esses comandos para liberar áudio e imagem:
```
export PULSE_SERVER=unix:/mnt/wslg/PulseServer
export DISPLAY=:0
```



