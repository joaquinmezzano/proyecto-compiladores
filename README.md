# Proyecto Compiladores

**Universidad Nacional de Río Cuarto**  
**Facultad de Ciencias Exactas, Físico-Químicas y Naturales**  
**Departamento de Computación**  
**Taller de Diseño de Software (Cod. 3306)**

**Año: 2025**

## Descripción del Proyecto

Este proyecto tiene como objetivo el desarrollo de un compilador, aplicando los conceptos y prácticas aprendidas durante el Taller de Diseño de Software.

## Estrategia de ramas y main para seguimiento

| Rama / Etapa                       | Inicio     | Entrega    |
| ---------------------------------- | ---------- | ---------- |
| ~~`preproyecto`~~                  | -          | -          |
| ~~`analizador-lexico-sintactico`~~ | 03/09/2025 | 15/09/2025 |
| ~~`ts-ast`~~                       | 15/09/2025 | 24/09/2025 |
| ~~`analizador-semantico`~~         | 15/09/2025 | 01/10/2025 |
| ~~`codigo-intermedio`~~            | 01/10/2025 | 08/10/2025 |
| ~~`codigo-objeto`~~                | 08/10/2025 | 27/10/2025 |
| ~~`optimizador`~~                  | 27/10/2025 | 12/11/2025 |
| `entrega-final`                    | -          | 15/11/2025 |

**Comentario:** La rama `main` contiene la versión estable del proyecto. Se actualiza con merges de cada etapa.

## Integrantes

- **Joaquín Mezzano**
- **Tomás Rodeghiero**

## Uso del proyecto

### Compilación

    make [target]

| Target    | Descripción                                | Ejemplo             |
| --------- | ------------------------------------------ | ------------------- |
| `all`     | Compila el ejecutable (opción por defecto) | `make` o `make all` |
| `clean`   | Limpia archivos generados                  | `make clean`        |
| `rebuild` | Limpia y recompila desde cero              | `make rebuild`      |
| `help`    | Muestra información y uso del programa     | `make help`         |

### Ejecución

    make run FILE=<archivo.ctds> [DEBUG=1] [TARGET=<etapa>] [OPTIMIZER=1]

| Comando                   | Descripción                            | Ejemplo                                |
| ------------------------- | -------------------------------------- | -------------------------------------- |
| `make run FILE=<archivo>` | Ejecuta el compilador con un archivo   | `make run FILE=examples/example1.ctds` |
| `make run FILE=<archivo> DEBUG=1` | Ejecuta en modo debug (output detallado) | `make run FILE=examples/example1.ctds DEBUG=1` |
| `make run FILE=<archivo> TARGET=<etapa>` | Compila hasta etapa específica | `make run FILE=examples/example1.ctds TARGET=ir` |
| `make run FILE=<archivo> OPTIMIZER=1` | Habilita optimizaciones | `make run FILE=examples/example1.ctds OPTIMIZER=1` |
| `make test-all`           | Ejecuta todos los ejemplos disponibles | `make test-all`                        |
| `make help`               | Muestra ayuda completa                  | `make help`                            |

## Etapas de compilación (TARGET)

| Etapa | Hasta donde compila | Archivos generados |
|-------|--------------------|--------------------|
| `syntax`/`semantic` | Análisis semántico + AST (optimizado con `-optimizer`) | `ast_tree.png` |
| `ir` | Código intermedio (optimizado con `-optimizer`) | `ast_tree.png`, `inter.ir` |
| `object`/`all` | Compilación completa (default) | `ast_tree.png`, `inter.ir`, `output.s` |

## Optimizaciones

Las optimizaciones están **deshabilitadas por defecto**. Para habilitarlas, use el flag `OPTIMIZER=1`:

```bash
make run FILE=examples/example1.ctds OPTIMIZER=1
```

Las optimizaciones incluyen:
- **AST**: Constant folding, algebraic simplification
- **IR**: Constant folding, algebraic simplification, constant propagation, dead code elimination