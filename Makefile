# ===================================================================
# MAKEFILE - SPACE GAME C (ESTILO SPACE IMPACT)
# ===================================================================

# Compilador e Flags
CC       := gcc
CFLAGS   := -Wall -Wextra -O2
LIBS     := -lncurses -lm

# Nome do Executável Final
TARGET   := space

# Deteção Automática de Ficheiros Fonte e Objetos
SRCS     := main.c space.c levels.c boss.c draw.c menu.c
OBJS     := $(SRCS:.c=.o)

# Regra Principal (Default)
all: $(TARGET)

# Linkagem do Executável Final
$(TARGET): $(OBJS)
	@echo "Linking target: $@"
	$(CC) $(CFLAGS) $(OBJS) -o $(TARGET) $(LIBS)
	@echo "Build complete successfully!"

# Compilação dos Ficheiros Objeto (.c -> .o)
%.o: %.c
	@echo "Compiling: $<"
	$(CC) $(CFLAGS) -c $< -o $@

# Limpeza de Ficheiros Temporários de Compilação
clean:
	@echo "Cleaning object files..."
	rm -f $(OBJS)

# Limpeza Total (Objetos + Executável)
fclean: clean
	@echo "Removing executable..."
	rm -f $(TARGET)

# Rebuild Total
re: fclean all

# Evitar Conflitos com Ficheiros do Mesmo Nome
.PHONY: all clean fclean re