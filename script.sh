#!/bin/bash

# Terminar si algún comando falla
set -e

# Archivos fuente
LEXER="lexico.l"
PARSER="sintaxis.y"
AST="ast.c"
SYMTAB="symtab.c"
SEMANTICS="semantics.c"
OUTPUT="c-tds"

# Confirmar que se pasó un archivo
if [ $# -eq 0 ]; then
    echo "Uso: $0 Ejemplos/<archivo.ctds>"
    exit 1
fi

FILE=$1

# Verificar que el archivo existe
if [ ! -f "$FILE" ]; then
    echo "Error: el archivo '$FILE' no existe."
    exit 1
fi

# Verificar que todos los archivos fuente existen
for archivo in "$LEXER" "$PARSER" "$AST" "$SYMTAB" "$SEMANTICS"; do
    if [ ! -f "$archivo" ]; then
        echo "Error: el archivo fuente '$archivo' no existe."
        exit 1
    fi
done

# Generar lexer
echo "==> Generando lexer con Flex..."
flex --nounput --noyywrap "$LEXER"

# Generar parser con Bison y reporte de conflictos
echo "==> Generando parser con Bison..."
bison -d -v "$PARSER"  # -v genera el archivo sintaxis.output útil para debug

# Compilar con GCC
echo "==> Compilando con GCC..."
gcc -Wall -Wextra -std=c99 -D_POSIX_C_SOURCE=200809L -g -I. \
    -Wno-sign-compare -Wno-unused-function \
    -o "$OUTPUT" sintaxis.tab.c lex.yy.c "$AST" "$SYMTAB" "$SEMANTICS"

echo "==> Compilación exitosa. Ejecutable: $OUTPUT"

# Ejecutar parser y capturar salida
echo "==> Ejecutando parser con análisis semántico en $FILE..."
echo "=================================================="

if ./"$OUTPUT" < "$FILE"; then
    echo "=============================================="
    echo "✓ Análisis completado exitosamente."
    
    # Mostrar archivos generados si existen
    [ -f "ast_tree.png" ] && echo "✓ AST generado: ast_tree.png"
    [ -f "sintaxis.output" ] && echo "✓ Reporte de parser: sintaxis.output"
else
    echo "=============================================="
    echo "✗ Análisis terminó con errores."
    echo ""
    echo "Para debug, revisar:"
    echo "- sintaxis.output: conflictos del parser"
    echo "- Mensajes de error mostrados arriba"
    exit 1
fi
