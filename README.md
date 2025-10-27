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
| `optimizador`                      | 27/10/2025 | 12/11/2025 |
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

    make run FILE=<archivo.ctds>

| Comando                   | Descripción                            | Ejemplo                                |
| ------------------------- | -------------------------------------- | -------------------------------------- |
| `make run FILE=<archivo>` | Ejecuta el compilador con un archivo   | `make run FILE=examples/example1.ctds` |
| `make test-all`           | Ejecuta todos los ejemplos disponibles | `make test-all`                        |
| `make test-good`          | Ejecuta solo ejemplos válidos          | `make test-good`                       |
| `make test-errors`        | Ejecuta ejemplos con errores esperados | `make test-errors`                     |
