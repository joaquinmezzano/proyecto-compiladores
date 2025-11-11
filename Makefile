# Makefile para el compilador C-TDS
# Compatible con Linux y macOS

# Usar bash como shell para que funcionen los códigos de escape ANSI
SHELL := /bin/bash

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

# Sistema de colores simplificado y robusto
NO_COLOR := \033[0m
GREEN := \033[0;32m
YELLOW := \033[0;33m
BLUE := \033[0;34m
RED := \033[0;31m
BOLD := \033[1m

# Comandos de colores para uso directo en bash
ECHO_BLUE = bash -c 'echo -e "\033[0;34m$$0\033[0m"'
ECHO_GREEN = bash -c 'echo -e "\033[0;32m$$0\033[0m"'
ECHO_YELLOW = bash -c 'echo -e "\033[0;33m$$0\033[0m"'
ECHO_RED = bash -c 'echo -e "\033[0;31m$$0\033[0m"'
ECHO_SUCCESS = bash -c 'echo -e "\033[0;32m✓ $$0\033[0m"'
ECHO_INFO = bash -c 'echo -e "\033[0;34m==> $$0\033[0m"'
ECHO_WARNING = bash -c 'echo -e "\033[0;33m⚠ $$0\033[0m"'
ECHO_ERROR = bash -c 'echo -e "\033[0;31m✗ $$0\033[0m"'

# Regla por defecto
.PHONY: all
all: banner $(EXECUTABLE)

# Banner de información
.PHONY: banner
banner:
	@bash -c 'echo -e "\033[0;34m================================================\033[0m"'
	@bash -c 'echo -e "\033[0;34m  Compilador C-TDS - Plataforma: $(PLATFORM)\033[0m"'
	@bash -c 'echo -e "\033[0;34m================================================\033[0m"'
	@echo ""

# Compilar el ejecutable principal
$(EXECUTABLE): $(PARSER_OUT) $(LEXER_OUT) $(C_SOURCES) $(HEADERS)
	@$(ECHO_INFO) "Compilando con GCC en $(PLATFORM)..."
	$(CC) $(CFLAGS) -o $@ $(PARSER_OUT) $(LEXER_OUT) $(C_SOURCES)
	@$(ECHO_SUCCESS) "Compilación exitosa. Ejecutable: $@"
	@echo ""

# Generar parser con Bison
$(PARSER_OUT) $(PARSER_HDR) $(PARSER_REPORT): $(PARSER_SRC)
	@$(ECHO_INFO) "Generando parser con Bison..."
	$(BISON) -d -v $<

# Generar lexer con Flex
$(LEXER_OUT): $(LEXER_SRC)
	@$(ECHO_INFO) "Generando lexer con Flex..."
	$(FLEX) --nounput --noyywrap $<

# Verificar archivos fuente
.PHONY: check-sources
check-sources:
	@$(ECHO_INFO) "Verificando archivos fuente..."
	@for file in $(LEXER_SRC) $(PARSER_SRC) $(C_SOURCES) $(HEADERS); do \
		if [ ! -f "$$file" ]; then \
			$(ECHO_ERROR) "el archivo fuente '$$file' no existe."; \
			exit 1; \
		fi \
	done
	@$(ECHO_SUCCESS) "Todos los archivos fuente verificados correctamente."

# Limpiar archivos generados
.PHONY: clean
clean:
	@$(ECHO_INFO) "Limpiando archivos generados..."
	rm -f $(CLEAN_FILES)
	@$(ECHO_SUCCESS) "Limpieza completada."

# Compilar desde cero (limpiar y compilar)
.PHONY: rebuild
rebuild: clean all

# Ejecutar el compilador con un archivo de ejemplo
.PHONY: run
run: $(EXECUTABLE)
	@if [ -z "$(FILE)" ]; then \
		$(ECHO_ERROR) "Uso: make run FILE=examples/<archivo.ctds> [DEBUG=1] [TARGET=<etapa>] [OPTIMIZER=1]"; \
		echo "Ejemplo: make run FILE=examples/example1.ctds"; \
		echo "Con debug: make run FILE=examples/example1.ctds DEBUG=1"; \
		echo "Con target: make run FILE=examples/example1.ctds TARGET=semantic"; \
		echo "Con optimizaciones: make run FILE=examples/example1.ctds OPTIMIZER=1"; \
		echo "Etapas: syntax/semantic, ir, object (default), all"; \
		exit 1; \
	fi
	@if [ ! -f "$(FILE)" ]; then \
		$(ECHO_ERROR) "el archivo '$(FILE)' no existe."; \
		exit 1; \
	fi
	@if [ "$(DEBUG)" = "1" ]; then \
		if [ -n "$(TARGET)" ]; then \
			$(ECHO_INFO) "Ejecutando compilación de $(FILE) (modo debug, target: $(TARGET))..."; \
		else \
			$(ECHO_INFO) "Ejecutando parser con análisis semántico en $(FILE) (modo debug)..."; \
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
			$(ECHO_SUCCESS) "Análisis completado exitosamente."; \
			if [ -f "ast_tree.png" ]; then $(ECHO_SUCCESS) "AST generado: ast_tree.png"; fi; \
			if [ -f "sintaxis.output" ]; then $(ECHO_SUCCESS) "Reporte de parser: sintaxis.output"; fi; \
			if [ "$(TARGET)" = "syntax" ] || [ "$(TARGET)" = "semantic" ]; then \
				: ; \
			elif [ "$(TARGET)" = "ir" ]; then \
				if [ -f "inter.ir" ]; then $(ECHO_SUCCESS) "Código intermedio generado: inter.ir"; fi; \
			else \
				if [ -f "inter.ir" ]; then $(ECHO_SUCCESS) "Código intermedio generado: inter.ir"; fi; \
				if [ -f "output.s" ]; then $(ECHO_SUCCESS) "Código objeto generado: output.s"; fi; \
			fi; \
			$(ECHO_SUCCESS) "Archivo analizado: $(FILE) ($$(wc -l < "$(FILE)" | xargs) líneas)"; \
			$(ECHO_BLUE) "✓ Plataforma: $(PLATFORM)"; \
			echo ""; \
		else \
			echo "----------------------------------------------"; \
			echo ""; \
			$(ECHO_ERROR) "Análisis terminó con errores."; \
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
			$(ECHO_INFO) "Ejecutando compilación de $(FILE) (target: $(TARGET))..."; \
		else \
			$(ECHO_INFO) "Ejecutando compilación de $(FILE)..."; \
		fi; \
		TARGET_ARG=""; \
		OPTIMIZER_ARG=""; \
		if [ -n "$(TARGET)" ]; then TARGET_ARG="-target $(TARGET)"; fi; \
		if [ "$(OPTIMIZER)" = "1" ]; then OPTIMIZER_ARG="-optimizer"; fi; \
		if ./$(EXECUTABLE) $$TARGET_ARG $$OPTIMIZER_ARG < "$(FILE)"; then \
			$(ECHO_SUCCESS) "Compilación exitosa: $(FILE)"; \
			if [ -f "ast_tree.png" ]; then $(ECHO_SUCCESS) "AST generado: ast_tree.png"; fi; \
			if [ "$(TARGET)" = "syntax" ] || [ "$(TARGET)" = "semantic" ]; then \
				: ; \
			elif [ "$(TARGET)" = "ir" ]; then \
				if [ -f "inter.ir" ]; then $(ECHO_SUCCESS) "Código intermedio generado: inter.ir"; fi; \
			else \
				if [ -f "inter.ir" ]; then $(ECHO_SUCCESS) "Código intermedio generado: inter.ir"; fi; \
				if [ -f "output.s" ]; then $(ECHO_SUCCESS) "Código objeto generado: output.s"; fi; \
			fi; \
		else \
			$(ECHO_ERROR) "Error durante la compilación de $(FILE)"; \
			exit 1; \
		fi; \
	fi

# Compilar y enlazar código ensamblador generado (para pruebas)
.PHONY: assemble
assemble:
	@if [ ! -f "output.s" ]; then \
		$(ECHO_ERROR) "no existe output.s. Ejecuta primero: make run FILE=<archivo>"; \
		exit 1; \
	fi
	@$(ECHO_INFO) "Ensamblando output.s en $(PLATFORM)..."
ifeq ($(UNAME_S),Darwin)
	@$(ECHO_WARNING) "Usando sintaxis de ensamblador macOS..."
	$(AS) output.s -o output.o
	@$(ECHO_SUCCESS) "Ensamblado exitoso: output.o"
	@$(ECHO_BLUE) "Para enlazar, usa: ld -o program output.o -lSystem -e _main"
else
	@$(ECHO_WARNING) "Usando sintaxis de ensamblador Linux..."
	$(AS) output.s -o output.o
	$(CC) output.o -o program
	@$(ECHO_SUCCESS) "Programa compilado: ./program"
endif

# Ejecutar todos los ejemplos
.PHONY: test-all
test-all: $(EXECUTABLE)
	@$(ECHO_INFO) "Ejecutando todos los ejemplos..."
	@for example in examples/example*.ctds; do \
		if [ -f "$$example" ]; then \
			echo ""; \
			$(ECHO_INFO) "Probando: $$example"; \
			$(MAKE) run FILE="$$example" || true; \
		fi \
	done

# Ejecutar solo ejemplos que no deberían tener errores
.PHONY: test-good
test-good: $(EXECUTABLE)
	@$(ECHO_INFO) "Ejecutando ejemplos válidos..."
	@for example in examples/example[1-7].ctds; do \
		if [ -f "$$example" ]; then \
			echo ""; \
			$(ECHO_INFO) "Probando: $$example"; \
			$(MAKE) run FILE="$$example" || true; \
		fi \
	done

# Ejecutar ejemplos con errores esperados
.PHONY: test-errors
test-errors: $(EXECUTABLE)
	@$(ECHO_INFO) "Ejecutando ejemplos con errores esperados..."
	@for example in examples/exampleError*.ctds; do \
		if [ -f "$$example" ]; then \
			echo ""; \
			$(ECHO_INFO) "Probando: $$example (se esperan errores)"; \
			$(MAKE) run FILE="$$example" || true; \
		fi \
	done

# Mostrar información del sistema
.PHONY: info
info:
	@bash -c 'echo -e "\033[0;34m================================================\033[0m"'
	@bash -c 'echo -e "\033[0;34m  Información del Sistema\033[0m"'
	@bash -c 'echo -e "\033[0;34m================================================\033[0m"'
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
	@bash -c 'echo -e "\033[0;34m================================================\033[0m"'
	@bash -c 'echo -e "\033[0;34m  Makefile para el compilador C-TDS\033[0m"'
	@bash -c 'echo -e "\033[0;34m  Plataforma: $(PLATFORM)\033[0m"'
	@bash -c 'echo -e "\033[0;34m================================================\033[0m"'
	@echo ""
	@echo "Targets disponibles:"
	@bash -c 'echo -e "  \033[0;32mall\033[0m             - Compilar el ejecutable (default)"'
	@bash -c 'echo -e "  \033[0;32mclean\033[0m           - Limpiar archivos generados"'
	@bash -c 'echo -e "  \033[0;32mrebuild\033[0m         - Limpiar y recompilar desde cero"'
	@bash -c 'echo -e "  \033[0;32mcheck-sources\033[0m   - Verificar que existan todos los archivos fuente"'
	@bash -c 'echo -e "  \033[0;32minfo\033[0m            - Mostrar información del sistema"'
	@echo ""
	@bash -c 'echo -e "  \033[0;32mrun\033[0m FILE=<archivo> [DEBUG=1] [TARGET=<etapa>] [OPTIMIZER=1]"'
	@echo "                  - Ejecutar el compilador"
	@echo "                    Ejemplo: make run FILE=examples/example1.ctds"
	@echo "                    Con debug: make run FILE=examples/example1.ctds DEBUG=1"
	@echo "                    Con target: make run FILE=examples/example1.ctds TARGET=ir"
	@echo "                    Con optimizaciones: make run FILE=examples/example1.ctds OPTIMIZER=1"
	@echo ""
	@bash -c 'echo -e "  \033[0;33mEtapas disponibles (TARGET):\033[0m"'
	@echo "    syntax/semantic - Hasta análisis semántico + AST optimizado"
	@echo "    ir              - Hasta código intermedio + optimizaciones IR"
	@echo "    object/all      - Compilación completa hasta código objeto (default)"
	@echo ""
	@bash -c 'echo -e "  \033[0;32massemble\033[0m        - Ensamblar output.s generado"'
	@bash -c 'echo -e "  \033[0;32mtest-all\033[0m        - Ejecutar todos los ejemplos"'
	@bash -c 'echo -e "  \033[0;32mtest-good\033[0m       - Ejecutar solo ejemplos válidos"'
	@bash -c 'echo -e "  \033[0;32mtest-errors\033[0m     - Ejecutar ejemplos con errores esperados"'
	@echo ""
	@bash -c 'echo -e "  \033[0;32mhelp\033[0m            - Mostrar esta ayuda"'
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