#!/bin/bash
# Terminar si algún comando falla
set -e

# Archivos fuente
LEXER="src/lexico.l"
PARSER="src/sintaxis.y"
AST_SOURCE="src/ast.c"
SYMTAB_SOURCE="src/symtab.c"
SEMANTICS_SOURCE="src/semantics.c"
INTERMEDIATE_SOURCE="src/intermediate.c"
AST_HEADER="src/ast.h"
SYMTAB_HEADER="src/symtab.h"
SEMANTICS_HEADER="src/semantics.h"
INTERMEDIATE_HEADER="src/intermediate.h"
OUTPUT="c-tds"

# Función de ayuda si se llama con --help
if [ "$1" = "--help" ] || [ "$1" = "-h" ]; then
    echo ""
    echo "Uso: $0 examples/<archivo.ctds>"
    echo ""
    echo "Estructura esperada:"
    echo "  src/            - Código fuente del compilador"
    echo "  examples/       - Archivos de ejemplo .ctds"
    echo "  docs/           - Documentación"
    echo ""
    echo "Archivos generados:"
    echo "  c-tds           - Ejecutable del compilador"
    echo "  ast_tree.png    - Visualización del AST (requiere Graphviz)"
    echo "  sintaxis.output - Reporte detallado del parser"
    echo ""
    exit 0
fi

# Confirmar que se pasó un archivo
if [ $# -eq 0 ]; then
    echo "Uso: $0 examples/<archivo.ctds>"
    exit 1
fi

FILE=$1

# Verificar que el archivo de entrada existe
if [ ! -f "$FILE" ]; then
    echo "Error: el archivo '$FILE' no existe."
    exit 1
fi

# Verificar que todos los archivos fuente existen
echo "==> Verificando archivos fuente..."
for archivo in "$LEXER" "$PARSER" "$AST_SOURCE" "$SYMTAB_SOURCE" "$SEMANTICS_SOURCE" "$INTERMEDIATE_SOURCE"; do
    if [ ! -f "$archivo" ]; then
        echo "Error: el archivo fuente '$archivo' no existe."
        exit 1
    fi
done

# Verificar que todos los archivos header existen
for archivo in "$AST_HEADER" "$SYMTAB_HEADER" "$SEMANTICS_HEADER" "$INTERMEDIATE_HEADER"; do
    if [ ! -f "$archivo" ]; then
        echo "Error: el archivo header '$archivo' no existe."
        exit 1
    fi
done

# Limpiar archivos generados anteriores
echo "==> Limpiando archivos generados anteriores..."
rm -f lex.yy.c sintaxis.tab.c sintaxis.tab.h sintaxis.output "$OUTPUT" ast.dot ast_tree.png inter.s

# Generar lexer
echo "==> Generando lexer con Flex..."
flex --nounput --noyywrap "$LEXER"

# Generar parser con Bison y reporte de conflictos
echo "==> Generando parser con Bison..."
bison -d -v "$PARSER" # -v genera el archivo sintaxis.output útil para debug

# Compilar con GCC
echo "==> Compilando con GCC..."
gcc -Wall -Wextra -std=c99 -D_POSIX_C_SOURCE=200809L -g \
    -I./src \
    -Wno-sign-compare -Wno-unused-function -Wno-unused-parameter \
    -o "$OUTPUT" \
    sintaxis.tab.c lex.yy.c \
    "$AST_SOURCE" "$SYMTAB_SOURCE" "$SEMANTICS_SOURCE" "$INTERMEDIATE_SOURCE"

echo "==> Compilación exitosa. Ejecutable: $OUTPUT"

# Ejecutar parser y capturar salida
echo "==> Ejecutando parser con análisis semántico en $FILE..."
echo ""
echo "--------------------------------------------------"
echo ""

if ./"$OUTPUT" < "$FILE"; then
    echo "----------------------------------------------"
    echo ""
    echo "✓ Análisis completado exitosamente."
    
    # Mostrar archivos generados si existen
    [ -f "ast_tree.png" ] && echo "✓ AST generado: ast_tree.png"
    [ -f "sintaxis.output" ] && echo "✓ Reporte de parser: sintaxis.output"
    [ -f "inter.s" ] && echo "✓ Código intermedio generado: inter.s"
    
    # Opcional: mostrar estadísticas del archivo analizado
    echo "✓ Archivo analizado: $FILE ($(wc -l < "$FILE") líneas)"
    echo ""
    
else
    echo "----------------------------------------------"
    echo ""
    echo "✗ Análisis terminó con errores."
    echo ""
    echo "Para debug, revisar:"
    echo "- sintaxis.output: conflictos del parser"
    echo "- Mensajes de error mostrados arriba"
    echo "- Verificar sintaxis en $FILE"
    echo ""
    exit 1
fi
