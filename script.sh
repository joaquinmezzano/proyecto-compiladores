#!/bin/bash

# Terminar si algún comando falla
set -e

# Archivos fuente
LEXER="lexico.l"
PARSER="sintaxis.y"
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

# Generar lexer
echo "==> Generando lexer con Flex..."
flex "$LEXER"

# Generar parser con Bison y reporte de conflictos
echo "==> Generando parser con Bison..."
bison -d -v "$PARSER"  # -v genera el archivo sintaxis.output útil para debug

# Compilar con GCC
echo "==> Compilando con GCC..."
if [[ "$OSTYPE" == "darwin"* ]]; then
    gcc -o "$OUTPUT" sintaxis.tab.c lex.yy.c
else
    gcc -o "$OUTPUT" sintaxis.tab.c lex.yy.c -lfl
fi

# Ejecutar parser y capturar salida
echo "==> Ejecutando parser con $FILE..."
./"$OUTPUT" < "$FILE" || echo "Parser terminó con errores. Revisá 'sintaxis.output' para detalles."
