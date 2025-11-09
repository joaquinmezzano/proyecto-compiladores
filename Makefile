# Makefile para el compilador C-TDS
# Compilador de lenguaje C-TDS

# Variables de configuración
CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -D_POSIX_C_SOURCE=200809L -g -I./src -Wno-sign-compare -Wno-unused-function -Wno-unused-parameter
FLEX = flex
BISON = bison

# Archivos fuente
LEXER_SRC = src/lexico.l
PARSER_SRC = src/sintaxis.y
C_SOURCES = src/ast.c src/symtab.c src/semantics.c src/intermediate.c src/object.c src/optimizer.c
HEADERS = src/ast.h src/symtab.h src/semantics.h src/intermediate.h src/object.h src/optimizer.o

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

# Regla por defecto
.PHONY: all
all: $(EXECUTABLE)

# Compilar el ejecutable principal
$(EXECUTABLE): $(PARSER_OUT) $(LEXER_OUT) $(C_SOURCES) $(HEADERS)
	@echo "==> Compilando con GCC..."
	$(CC) $(CFLAGS) -o $@ $(PARSER_OUT) $(LEXER_OUT) $(C_SOURCES)
	@echo "==> Compilación exitosa. Ejecutable: $@"

# Generar parser con Bison
$(PARSER_OUT) $(PARSER_HDR) $(PARSER_REPORT): $(PARSER_SRC)
	@echo "==> Generando parser con Bison..."
	$(BISON) -d -v $<

# Generar lexer con Flex
$(LEXER_OUT): $(LEXER_SRC)
	@echo "==> Generando lexer con Flex..."
	$(FLEX) --nounput --noyywrap $<

# Verificar archivos fuente
.PHONY: check-sources
check-sources:
	@echo "==> Verificando archivos fuente..."
	@for file in $(LEXER_SRC) $(PARSER_SRC) $(C_SOURCES) $(HEADERS); do \
		if [ ! -f "$$file" ]; then \
			echo "Error: el archivo fuente '$$file' no existe."; \
			exit 1; \
		fi \
	done
	@echo "✓ Todos los archivos fuente verificados correctamente."

# Limpiar archivos generados
.PHONY: clean
clean:
	@echo "==> Limpiando archivos generados..."
	rm -f $(CLEAN_FILES)
	@echo "✓ Limpieza completada."

# Compilar desde cero (limpiar y compilar)
.PHONY: rebuild
rebuild: clean all

# Ejecutar el compilador con un archivo de ejemplo
.PHONY: run
run: $(EXECUTABLE)
	@if [ -z "$(FILE)" ]; then \
		echo "Uso: make run FILE=examples/<archivo.ctds> [DEBUG=1] [TARGET=<etapa>] [OPTIMIZER=1]"; \
		echo "Ejemplo: make run FILE=examples/example1.ctds"; \
		echo "Con debug: make run FILE=examples/example1.ctds DEBUG=1"; \
		echo "Con target: make run FILE=examples/example1.ctds TARGET=semantic"; \
		echo "Con optimizaciones: make run FILE=examples/example1.ctds OPTIMIZER=1"; \
		echo "Etapas: syntax/semantic, ir, object (default), all"; \
		exit 1; \
	fi
	@if [ ! -f "$(FILE)" ]; then \
		echo "Error: el archivo '$(FILE)' no existe."; \
		exit 1; \
	fi
	@if [ "$(DEBUG)" = "1" ]; then \
		if [ -n "$(TARGET)" ]; then \
			echo "==> Ejecutando compilación de $(FILE) (modo debug, target: $(TARGET))..."; \
		else \
			echo "==> Ejecutando parser con análisis semántico en $(FILE) (modo debug)..."; \
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
			echo "✓ Análisis completado exitosamente."; \
			if [ -f "ast_tree.png" ]; then echo "✓ AST generado: ast_tree.png"; fi; \
			if [ -f "sintaxis.output" ]; then echo "✓ Reporte de parser: sintaxis.output"; fi; \
			if [ "$(TARGET)" = "syntax" ] || [ "$(TARGET)" = "semantic" ]; then \
				: ; \
			elif [ "$(TARGET)" = "ir" ]; then \
				if [ -f "inter.ir" ]; then echo "✓ Código intermedio generado: inter.ir"; fi; \
			else \
				if [ -f "inter.ir" ]; then echo "✓ Código intermedio generado: inter.ir"; fi; \
				if [ -f "output.s" ]; then echo "✓ Código objeto generado: output.s"; fi; \
			fi; \
			echo "✓ Archivo analizado: $(FILE) ($$(wc -l < "$(FILE)" | xargs) líneas)"; \
			echo ""; \
		else \
			echo "----------------------------------------------"; \
			echo ""; \
			echo "✗ Análisis terminó con errores."; \
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
			echo "==> Ejecutando compilación de $(FILE) (target: $(TARGET))..."; \
		else \
			echo "==> Ejecutando compilación de $(FILE)..."; \
		fi; \
		TARGET_ARG=""; \
		OPTIMIZER_ARG=""; \
		if [ -n "$(TARGET)" ]; then TARGET_ARG="-target $(TARGET)"; fi; \
		if [ "$(OPTIMIZER)" = "1" ]; then OPTIMIZER_ARG="-optimizer"; fi; \
		if ./$(EXECUTABLE) $$TARGET_ARG $$OPTIMIZER_ARG < "$(FILE)"; then \
			echo "✓ Compilación exitosa: $(FILE)"; \
			if [ -f "ast_tree.png" ]; then echo "✓ AST generado: ast_tree.png"; fi; \
			if [ "$(TARGET)" = "syntax" ] || [ "$(TARGET)" = "semantic" ]; then \
				: ; \
			elif [ "$(TARGET)" = "ir" ]; then \
				if [ -f "inter.ir" ]; then echo "✓ Código intermedio generado: inter.ir"; fi; \
			else \
				if [ -f "inter.ir" ]; then echo "✓ Código intermedio generado: inter.ir"; fi; \
				if [ -f "output.s" ]; then echo "✓ Código objeto generado: output.s"; fi; \
			fi; \
		else \
			echo "✗ Error durante la compilación de $(FILE)"; \
			exit 1; \
		fi; \
	fi

# Ejecutar todos los ejemplos
.PHONY: test-all
test-all: $(EXECUTABLE)
	@echo "==> Ejecutando todos los ejemplos..."
	@for example in examples/example*.ctds; do \
		if [ -f "$$example" ]; then \
			echo ""; \
			echo "==> Probando: $$example"; \
			$(MAKE) run FILE="$$example" || true; \
		fi \
	done

# Ejecutar solo ejemplos que no deberían tener errores
.PHONY: test-good
test-good: $(EXECUTABLE)
	@echo "==> Ejecutando ejemplos válidos..."
	@for example in examples/example[1-7].ctds; do \
		if [ -f "$$example" ]; then \
			echo ""; \
			echo "==> Probando: $$example"; \
			$(MAKE) run FILE="$$example" || true; \
		fi \
	done

# Ejecutar ejemplos con errores esperados
.PHONY: test-errors
test-errors: $(EXECUTABLE)
	@echo "==> Ejecutando ejemplos con errores esperados..."
	@for example in examples/exampleError*.ctds; do \
		if [ -f "$$example" ]; then \
			echo ""; \
			echo "==> Probando: $$example (se esperan errores)"; \
			$(MAKE) run FILE="$$example" || true; \
		fi \
	done

# Mostrar ayuda
.PHONY: help
help:
	@echo ""
	@echo "Makefile para el compilador C-TDS"
	@echo ""
	@echo "Targets disponibles:"
	@echo "  all             - Compilar el ejecutable (default)"
	@echo "  clean           - Limpiar archivos generados"
	@echo "  rebuild         - Limpiar y recompilar desde cero"
	@echo "  check-sources   - Verificar que existan todos los archivos fuente"
	@echo ""
	@echo "  run FILE=<archivo> [DEBUG=1] [TARGET=<etapa>] [OPTIMIZER=1] - Ejecutar el compilador"
	@echo "                                Ejemplo: make run FILE=examples/example1.ctds"
	@echo "                                Con debug: make run FILE=examples/example1.ctds DEBUG=1"
	@echo "                                Con target: make run FILE=examples/example1.ctds TARGET=ir"
	@echo "                                Con optimizaciones: make run FILE=examples/example1.ctds OPTIMIZER=1"
	@echo ""
	@echo "  Etapas disponibles (TARGET):"
	@echo "    syntax/semantic - Hasta análisis semántico + AST optimizado"
	@echo "    ir              - Hasta código intermedio + optimizaciones IR"
	@echo "    object/all      - Compilación completa hasta código objeto (default)"
	@echo ""
	@echo "  test-all        - Ejecutar todos los ejemplos"
	@echo "  test-good       - Ejecutar solo ejemplos válidos"
	@echo "  test-errors     - Ejecutar ejemplos con errores esperados"
	@echo ""
	@echo "  help            - Mostrar esta ayuda"
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