# Variáveis
CC = gcc
CFLAGS = -Wall -g -Iinclude -Ilib   # Incluir os diretórios include e lib
LDFLAGS = -lGL -lGLU -lglut -lm   	# Bibliotecas necessárias
SRCDIR = src
BUILDDIR = build
TARGET = $(BUILDDIR)/nyan

# Coleta todos os arquivos .c em src/
SRC = $(wildcard $(SRCDIR)/*.c)

# Converte os arquivos .c para .o
OBJ = $(SRC:$(SRCDIR)/%.c=$(BUILDDIR)/%.o)

# Regra padrão - compilar o executável
all: $(TARGET)

# Regra para criar o executável
$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $(TARGET) $(LDFLAGS)

# Regra para compilar os arquivos .c em .o
$(BUILDDIR)/%.o: $(SRCDIR)/%.c
	mkdir -p $(BUILDDIR)    # Cria o diretório build, se não existir
	$(CC) -c $< -o $@ $(CFLAGS)

# Limpa os arquivos gerados (útil para recomeçar a compilação)
clean:
	rm -rf $(BUILDDIR)/*

# Executa o programa
run: $(TARGET)
	./$(TARGET)
