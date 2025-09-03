# Preproyecto - Taller de diseño de software

Este proyecto implementa un compilador para un lenguaje simple extendido con características como:

- Declaración de variables de tipo `int` y `bool`.
- Asignaciones y expresiones aritméticas y lógicas.
- Sentencias de retorno de valores de tipo `int` o `bool`.
- Soporte para operaciones aritméticas (`+`, `*`, etc.) y lógicas (`&&`, `||`, etc.).

El compilador se implementa utilizando herramientas como **Lex** para el análisis léxico, **Bison** para el análisis sintáctico, y genera un **Árbol Sintáctico Abstracto (AST)**. Además, el proyecto incluye un **intérprete** para evaluar las expresiones y un generador de **pseudo-assembly** para simular la ejecución de los programas.

## Participantes

Este proyecto fue realizado por un grupo de estudiantes de la **Universidad Nacional de Rio Cuarto (UNRC)** en la materia **Taller de Diseño de Software**. El grupo esta compuesto por:

- Mezzano, Joaquin
- Rodeghiero, Tomás

## Estructura del Proyecto

El proyecto consta de los siguientes archivos principales:

- **`calc-lexico.l`**: Archivo fuente de Lex que define las expresiones regulares para el análisis léxico del lenguaje.
- **`calc-sintaxis.y`**: Archivo fuente de Bison que define la gramática del lenguaje y genera el parser.
- **`ast.h`**: Definiciones de las estructuras de datos del **AST** y funciones relacionadas.
- **`ast.c`**: Implementación de las funciones para manejar el AST, incluidas las funciones de creación y liberación de nodos.
- **`calc-sintaxis.tab.h`**: Archivo generado por Bison que contiene las definiciones de los tokens utilizados en el parser.
- **`programa.sasm`**: Archivo de salida con el código **pseudo-assembly** generado por el compilador.
- **`input.txt`**: Archivo con ejemplos de código en el lenguaje soportado por el compilador.
- **`script`**: Script para ejecutar el compilador e interpretar el código generado.
- **`preproyecto.pdf`**: Documento que describe el proyecto y los requisitos del mismo.

## Funcionalidades

1. **Análisis Léxico**: Utiliza **Lex** para escanear el código fuente y generar tokens.
2. **Análisis Sintáctico**: Utiliza **Bison** para procesar los tokens y generar un árbol sintáctico (AST).
3. **Evaluador de Expresiones**: Evalúa expresiones aritméticas y lógicas en tiempo de ejecución.
4. **Generación de Pseudo-Assembly**: Genera un archivo de pseudo-assembly que simula las instrucciones básicas de un procesador.

## Requisitos

Para ejecutar este proyecto en un sistema **Ubuntu** o **Mac**, necesitarás tener instaladas las siguientes herramientas:

- **Flex** (para el análisis léxico).
- **Bison** (para el análisis sintáctico).
- **gcc** (compilador de C).
- **Graphviz** (para visualizar el AST).

### Instalación en Ubuntu / Mac

1. **Instalar Flex y Bison**:
   En Ubuntu:
   ```
   bash
   sudo apt-get update
   sudo apt-get install flex bison
   ```
   En Mac (usando Homebrew):
   ```
   brew install flex bison
   ```
2. **Instalar Graphviz (Para la visualización del AST)**
   En Ubuntu;
   ```
   sudo apt-get install graphviz
   ```
   En Mac:
   ```
   brew install graphviz
   ```

### Como usar el proyecto

1. **Escribe un programa**
   ```
   Modifica el archivo script.txt
   ```
2. **Ejecutar el script que compila y ejecuta el proyecto**
   ```
   ./script
   ```
3. **¿Quieres ver el AST?**
   ```
   Ver el archivo ast_tree.png
   ```

## Licencia

Este proyecto está disponible bajo la licencia MIT.
