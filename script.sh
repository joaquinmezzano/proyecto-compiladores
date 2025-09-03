#!/bin/bash

# Si algún comando falla, el script termina
set -e

# Archivos fuente
LEXER="lexico.l"
PARSER="sintaxis.y"
OUTPUT="program"

# Confirmación de pasar archivo
if [ $# -eq 0 ]; then
    echo "Uso: $0 Ejemplos/<archivo.ctds>"
    exit 1
fi

FILE=$1

# Ejecución
echo "==> Generando lexer con Flex..."
flex $LEXER

echo "==> Generando parser con Bison..."
bison -d $PARSER

echo "==> Compilando con GCC..."
gcc -o $OUTPUT sintaxis.tab.c lex.yy.c -lfl

echo "==> Ejecutando parser con $FILE..."
./$OUTPUT < "$FILE"