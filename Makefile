# Makefile para el compilador C-TDS
# Compatible con Linux y macOS

# Detectar el sistema operativo
UNAME_S := $(shell uname -s)

# Variables de configuración base
CC = gcc
FLEX = flex
BISON = bison

# Configuración específica de la plataforma
ifeq ($(UNAME_S),Darwin)
    # macOS
    PLATFORM = macOS
    CFLAGS = -Wall -Wextra -std=c99 -D_POSIX_C_SOURCE=200809L -g -I./src -Wno-sign-compare -Wno-unused-function -Wno-unused-parameter
    AS = as
    LD = ld
else ifeq ($(UNAME_S),Linux)
    # Linux
    PLATFORM = Linux
    CFLAGS = -Wall -Wextra -std=c99 -D_POSIX_C_SOURCE=200809L -g -I./src -Wno-sign-compare -Wno-unused-function -Wno-unused-parameter
    AS = as
    LD = ld
else
    $(error Sistema operativo no soportado: $(UNAME_S))
endif

# Archivos fuente
LEXER_SRC = src/lexico.l
PARSER_SRC = src/sintaxis.y
C_SOURCES = src/ast.c src/symtab.c src/semantics.c src/intermediate.c src/object.c src/optimizer.c
HEADERS = src/ast.h src/symtab.h src/semantics.h src/intermediate.h src/object.h src/optimizer.h

# Archivos generados
LEXER_OUT = lex.yy.c
PARSER_OUT = sintaxis.tab.c
PARSER_HDR = sintaxis.tab.h
PARSER_REPORT = sintaxis.output
EXECUTABLE = c-tds

# Archivos de salida generados por el compilador
OUTPUT_FILES = ast.dot ast_tree.png inter.ir output.s

# Archivos temporales y generados para limpiar
CLEAN_FILES = $(LEXER_OUT) $(PARSER_OUT) $(PARSER_HDR) $(PARSER_REPORT) $(EXECUTABLE) $(OUTPUT_FILES)

# Colores para mensajes (si el terminal lo soporta)
NO_COLOR=\033[0m
GREEN=\033[0;32m
YELLOW=\033[0;33m
BLUE=\033[0;34m
RED=\033[0;31m

# Regla por defecto
.PHONY: all
all: banner $(EXECUTABLE)

# Banner de información
.PHONY: banner
banner:
	@echo "$(BLUE)================================================$(NO_COLOR)"
	@echo "$(BLUE)  Compilador C-TDS - Plataforma: $(PLATFORM)$(NO_COLOR)"
	@echo "$(BLUE)================================================$(NO_COLOR)"
	@echo ""

# Compilar el ejecutable principal
$(EXECUTABLE): $(PARSER_OUT) $(LEXER_OUT) $(C_SOURCES) $(HEADERS)
	@echo "$(GREEN)==> Compilando con GCC en $(PLATFORM)...$(NO_COLOR)"
	$(CC) $(CFLAGS) -o $@ $(PARSER_OUT) $(LEXER_OUT) $(C_SOURCES)
	@echo "$(GREEN)==> Compilación exitosa. Ejecutable: $@$(NO_COLOR)"
	@echo ""

# Generar parser con Bison
$(PARSER_OUT) $(PARSER_HDR) $(PARSER_REPORT): $(PARSER_SRC)
	@echo "$(YELLOW)==> Generando parser con Bison...$(NO_COLOR)"
	$(BISON) -d -v $<

# Generar lexer con Flex
$(LEXER_OUT): $(LEXER_SRC)
	@echo "$(YELLOW)==> Generando lexer con Flex...$(NO_COLOR)"
	$(FLEX) --nounput --noyywrap $<

# Verificar archivos fuente
.PHONY: check-sources
check-sources:
	@echo "$(BLUE)==> Verificando archivos fuente...$(NO_COLOR)"
	@for file in $(LEXER_SRC) $(PARSER_SRC) $(C_SOURCES) $(HEADERS); do \
		if [ ! -f "$$file" ]; then \
			echo "$(RED)Error: el archivo fuente '$$file' no existe.$(NO_COLOR)"; \
			exit 1; \
		fi \
	done
	@echo "$(GREEN)✓ Todos los archivos fuente verificados correctamente.$(NO_COLOR)"

# Limpiar archivos generados
.PHONY: clean
clean:
	@echo "$(YELLOW)==> Limpiando archivos generados...$(NO_COLOR)"
	rm -f $(CLEAN_FILES)
	@echo "$(GREEN)✓ Limpieza completada.$(NO_COLOR)"

# Compilar desde cero (limpiar y compilar)
.PHONY: rebuild
rebuild: clean all

# Ejecutar el compilador con un archivo de ejemplo
.PHONY: run
run: $(EXECUTABLE)
	@if [ -z "$(FILE)" ]; then \
		echo "$(RED)Uso: make run FILE=examples/<archivo.ctds> [DEBUG=1] [TARGET=<etapa>] [OPTIMIZER=1]$(NO_COLOR)"; \
		echo "Ejemplo: make run FILE=examples/example1.ctds"; \
		echo "Con debug: make run FILE=examples/example1.ctds DEBUG=1"; \
		echo "Con target: make run FILE=examples/example1.ctds TARGET=semantic"; \
		echo "Con optimizaciones: make run FILE=examples/example1.ctds OPTIMIZER=1"; \
		echo "Etapas: syntax/semantic, ir, object (default), all"; \
		exit 1; \
	fi
	@if [ ! -f "$(FILE)" ]; then \
		echo "$(RED)Error: el archivo '$(FILE)' no existe.$(NO_COLOR)"; \
		exit 1; \
	fi
	@if [ "$(DEBUG)" = "1" ]; then \
		if [ -n "$(TARGET)" ]; then \
			echo "$(BLUE)==> Ejecutando compilación de $(FILE) (modo debug, target: $(TARGET))...$(NO_COLOR)"; \
		else \
			echo "$(BLUE)==> Ejecutando parser con análisis semántico en $(FILE) (modo debug)...$(NO_COLOR)"; \
		fi; \
		echo ""; \
		echo "--------------------------------------------------"; \
		echo ""; \
		TARGET_ARG=""; \
		OPTIMIZER_ARG=""; \
		if [ -n "$(TARGET)" ]; then TARGET_ARG="-target $(TARGET)"; fi; \
		if [ "$(OPTIMIZER)" = "1" ]; then OPTIMIZER_ARG="-optimizer"; fi; \
		if ./$(EXECUTABLE) -debug $$TARGET_ARG $$OPTIMIZER_ARG < "$(FILE)"; then \
			echo " --------------------------- "; \
			echo "| Reporte final del programa |"; \
			echo " --------------------------- "; \
			echo ""; \
			echo "$(GREEN)✓ Análisis completado exitosamente.$(NO_COLOR)"; \
			if [ -f "ast_tree.png" ]; then echo "$(GREEN)✓ AST generado: ast_tree.png$(NO_COLOR)"; fi; \
			if [ -f "sintaxis.output" ]; then echo "$(GREEN)✓ Reporte de parser: sintaxis.output$(NO_COLOR)"; fi; \
			if [ "$(TARGET)" = "syntax" ] || [ "$(TARGET)" = "semantic" ]; then \
				: ; \
			elif [ "$(TARGET)" = "ir" ]; then \
				if [ -f "inter.ir" ]; then echo "$(GREEN)✓ Código intermedio generado: inter.ir$(NO_COLOR)"; fi; \
			else \
				if [ -f "inter.ir" ]; then echo "$(GREEN)✓ Código intermedio generado: inter.ir$(NO_COLOR)"; fi; \
				if [ -f "output.s" ]; then echo "$(GREEN)✓ Código objeto generado: output.s$(NO_COLOR)"; fi; \
			fi; \
			echo "$(GREEN)✓ Archivo analizado: $(FILE) ($$(wc -l < "$(FILE)" | xargs) líneas)$(NO_COLOR)"; \
			echo "$(BLUE)✓ Plataforma: $(PLATFORM)$(NO_COLOR)"; \
			echo ""; \
		else \
			echo "----------------------------------------------"; \
			echo ""; \
			echo "$(RED)✗ Análisis terminó con errores.$(NO_COLOR)"; \
			echo ""; \
			echo "Para debug, revisar:"; \
			echo "- sintaxis.output: conflictos del parser"; \
			echo "- Mensajes de error mostrados arriba"; \
			echo "- Verificar sintaxis en $(FILE)"; \
			echo ""; \
			exit 1; \
		fi; \
	else \
		if [ -n "$(TARGET)" ]; then \
			echo "$(BLUE)==> Ejecutando compilación de $(FILE) (target: $(TARGET))...$(NO_COLOR)"; \
		else \
			echo "$(BLUE)==> Ejecutando compilación de $(FILE)...$(NO_COLOR)"; \
		fi; \
		TARGET_ARG=""; \
		OPTIMIZER_ARG=""; \
		if [ -n "$(TARGET)" ]; then TARGET_ARG="-target $(TARGET)"; fi; \
		if [ "$(OPTIMIZER)" = "1" ]; then OPTIMIZER_ARG="-optimizer"; fi; \
		if ./$(EXECUTABLE) $$TARGET_ARG $$OPTIMIZER_ARG < "$(FILE)"; then \
			echo "$(GREEN)✓ Compilación exitosa: $(FILE)$(NO_COLOR)"; \
			if [ -f "ast_tree.png" ]; then echo "$(GREEN)✓ AST generado: ast_tree.png$(NO_COLOR)"; fi; \
			if [ "$(TARGET)" = "syntax" ] || [ "$(TARGET)" = "semantic" ]; then \
				: ; \
			elif [ "$(TARGET)" = "ir" ]; then \
				if [ -f "inter.ir" ]; then echo "$(GREEN)✓ Código intermedio generado: inter.ir$(NO_COLOR)"; fi; \
			else \
				if [ -f "inter.ir" ]; then echo "$(GREEN)✓ Código intermedio generado: inter.ir$(NO_COLOR)"; fi; \
				if [ -f "output.s" ]; then echo "$(GREEN)✓ Código objeto generado: output.s$(NO_COLOR)"; fi; \
			fi; \
		else \
			echo "$(RED)✗ Error durante la compilación de $(FILE)$(NO_COLOR)"; \
			exit 1; \
		fi; \
	fi

# Compilar y enlazar código ensamblador generado (para pruebas)
.PHONY: assemble
assemble:
	@if [ ! -f "output.s" ]; then \
		echo "$(RED)Error: no existe output.s. Ejecuta primero: make run FILE=<archivo>$(NO_COLOR)"; \
		exit 1; \
	fi
	@echo "$(BLUE)==> Ensamblando output.s en $(PLATFORM)...$(NO_COLOR)"
ifeq ($(UNAME_S),Darwin)
	@echo "$(YELLOW)==> Usando sintaxis de ensamblador macOS...$(NO_COLOR)"
	$(AS) output.s -o output.o
	@echo "$(GREEN)✓ Ensamblado exitoso: output.o$(NO_COLOR)"
	@echo "$(BLUE)==> Para enlazar, usa: ld -o program output.o -lSystem -e _main$(NO_COLOR)"
else
	@echo "$(YELLOW)==> Usando sintaxis de ensamblador Linux...$(NO_COLOR)"
	$(AS) output.s -o output.o
	$(CC) output.o -o program
	@echo "$(GREEN)✓ Programa compilado: ./program$(NO_COLOR)"
endif

# Ejecutar todos los ejemplos
.PHONY: test-all
test-all: $(EXECUTABLE)
	@echo "$(BLUE)==> Ejecutando todos los ejemplos...$(NO_COLOR)"
	@for example in examples/example*.ctds; do \
		if [ -f "$$example" ]; then \
			echo ""; \
			echo "$(BLUE)==> Probando: $$example$(NO_COLOR)"; \
			$(MAKE) run FILE="$$example" || true; \
		fi \
	done

# Ejecutar solo ejemplos que no deberían tener errores
.PHONY: test-good
test-good: $(EXECUTABLE)
	@echo "$(BLUE)==> Ejecutando ejemplos válidos...$(NO_COLOR)"
	@for example in examples/example[1-7].ctds; do \
		if [ -f "$$example" ]; then \
			echo ""; \
			echo "$(BLUE)==> Probando: $$example$(NO_COLOR)"; \
			$(MAKE) run FILE="$$example" || true; \
		fi \
	done

# Ejecutar ejemplos con errores esperados
.PHONY: test-errors
test-errors: $(EXECUTABLE)
	@echo "$(BLUE)==> Ejecutando ejemplos con errores esperados...$(NO_COLOR)"
	@for example in examples/exampleError*.ctds; do \
		if [ -f "$$example" ]; then \
			echo ""; \
			echo "$(BLUE)==> Probando: $$example (se esperan errores)$(NO_COLOR)"; \
			$(MAKE) run FILE="$$example" || true; \
		fi \
	done

# Mostrar información del sistema
.PHONY: info
info:
	@echo "$(BLUE)================================================$(NO_COLOR)"
	@echo "$(BLUE)  Información del Sistema$(NO_COLOR)"
	@echo "$(BLUE)================================================$(NO_COLOR)"
	@echo "Sistema Operativo: $(UNAME_S)"
	@echo "Plataforma: $(PLATFORM)"
	@echo "Compilador C: $(CC)"
	@echo "Flex: $(shell which $(FLEX))"
	@echo "Bison: $(shell which $(BISON))"
	@echo "Ensamblador: $(AS)"
	@echo ""
	@echo "Versiones:"
	@$(CC) --version | head -n 1
	@$(FLEX) --version
	@$(BISON) --version | head -n 1
	@echo ""

# Mostrar ayuda
.PHONY: help
help:
	@echo ""
	@echo "$(BLUE)================================================$(NO_COLOR)"
	@echo "$(BLUE)  Makefile para el compilador C-TDS$(NO_COLOR)"
	@echo "$(BLUE)  Plataforma: $(PLATFORM)$(NO_COLOR)"
	@echo "$(BLUE)================================================$(NO_COLOR)"
	@echo ""
	@echo "Targets disponibles:"
	@echo "  $(GREEN)all$(NO_COLOR)             - Compilar el ejecutable (default)"
	@echo "  $(GREEN)clean$(NO_COLOR)           - Limpiar archivos generados"
	@echo "  $(GREEN)rebuild$(NO_COLOR)         - Limpiar y recompilar desde cero"
	@echo "  $(GREEN)check-sources$(NO_COLOR)   - Verificar que existan todos los archivos fuente"
	@echo "  $(GREEN)info$(NO_COLOR)            - Mostrar información del sistema"
	@echo ""
	@echo "  $(GREEN)run$(NO_COLOR) FILE=<archivo> [DEBUG=1] [TARGET=<etapa>] [OPTIMIZER=1]"
	@echo "                  - Ejecutar el compilador"
	@echo "                    Ejemplo: make run FILE=examples/example1.ctds"
	@echo "                    Con debug: make run FILE=examples/example1.ctds DEBUG=1"
	@echo "                    Con target: make run FILE=examples/example1.ctds TARGET=ir"
	@echo "                    Con optimizaciones: make run FILE=examples/example1.ctds OPTIMIZER=1"
	@echo ""
	@echo "  $(YELLOW)Etapas disponibles (TARGET):$(NO_COLOR)"
	@echo "    syntax/semantic - Hasta análisis semántico + AST optimizado"
	@echo "    ir              - Hasta código intermedio + optimizaciones IR"
	@echo "    object/all      - Compilación completa hasta código objeto (default)"
	@echo ""
	@echo "  $(GREEN)assemble$(NO_COLOR)        - Ensamblar output.s generado"
	@echo "  $(GREEN)test-all$(NO_COLOR)        - Ejecutar todos los ejemplos"
	@echo "  $(GREEN)test-good$(NO_COLOR)       - Ejecutar solo ejemplos válidos"
	@echo "  $(GREEN)test-errors$(NO_COLOR)     - Ejecutar ejemplos con errores esperados"
	@echo ""
	@echo "  $(GREEN)help$(NO_COLOR)            - Mostrar esta ayuda"
	@echo ""
	@echo "Estructura esperada:"
	@echo "  src/            - Código fuente del compilador"
	@echo "  examples/       - Archivos de ejemplo .ctds"
	@echo "  docs/           - Documentación"
	@echo ""
	@echo "Archivos generados:"
	@echo "  c-tds           - Ejecutable del compilador"
	@echo "  ast_tree.png    - Visualización del AST (requiere Graphviz)"
	@echo "  sintaxis.output - Reporte detallado del parser"
	@echo "  inter.ir        - Código intermedio"
	@echo "  output.s        - Código ensamblador"
	@echo ""

# Dependencias explícitas para el parser (incluye headers)
$(PARSER_OUT): $(HEADERS)

# Dependencias para archivos objeto generados
$(C_SOURCES): $(HEADERS)