# yowaic
Tiny C-subset compiler that emits x86-64 assembly (educational).

This project reads a very small subset of C, builds an AST, and either:
- prints a simple S-expression style AST for debugging (`-a`), or
- generates AT&T-style x86-64 assembly suitable for Linux and links with `gcc`.

It is a learning/teaching toy, not a complete or production-ready C compiler.

## Quick Start

- Requirements: 64-bit Linux, `gcc`, `make`, `flex`.
- Build:
  - `make yowaic`
- Print AST (S-expr):
  - `./yowaic -a`, then type a function and press CTRL+D:
    ```c
    int f(){return 0;}
    ```
    Output:
    ```
    (int)f(){(return 0);}
    ```
- Generate assembly and run:
  - `./yowaic < Example/fibonacci.c > foo.s`
  - `gcc -o a.out foo.s`
  - `./a.out`

## What It Supports

Core language (intentionally small):
- Functions with up to 6 parameters (System V AMD64 calling convention)
- Statements: empty `;`, expression statements, blocks `{ ... }`,
  `if`/`else`, `for`, `return`
- Types: `char`, `int`, pointers (`*T`), fixed-size arrays (`T name[N]`)
- String literals (`"..."`) as arrays of `char` or pointers to `char`
- Expressions: `+`, `-`, `*`, `/`, assignment `=`, comparisons `<` and `>`
- Address-of `&` and dereference `*` (with basic pointer arithmetic)
- Declarations must have an initializer (e.g. `int x = 2;`)

What it intentionally does not support (non-goals):
- Full C grammar or semantics; no `while`, `do/while`, `switch`,
  `break`/`continue`, structs/unions/enums, typedefs, `sizeof`,
  floating point, `const/volatile`, or preprocessing
- A full optimizer or register allocator; codegen is straightforward

## Usage Examples

AST (debug) mode:
```sh
./yowaic -a
int f(){
  int x = 10;
  x = x + 2;
  return x;
}
```
Output:
```
(int)f(){(decl int x 10);(= x (+ x 2));(return x);}
```

Assembly generation (pipe a file on stdin):
```sh
./yowaic < Example/fibonacci.c > foo.s
gcc -o a.out foo.s
./a.out
```

## Building, Testing, Cleaning

- Build compiler: `make yowaic`
- Run tests (quick functional checks): `make test`
- Clean artifacts: `make clean`

## Project Layout (high level)

- `scanner.l` → tokenization via `flex`
- `token.c`, `token.h` → token utilities
- `parser.c`, `parser.h` → hand-written parser building the AST
- `generator.c`, `generator.h` → x86-64 assembly code generation
- `util.c`, `util.h` → small data structures and helpers
- `yowaic.c` → CLI entrypoint (`-a` for AST, otherwise emits assembly)
- `test.sh` → smoke tests; compiles small snippets and runs them
- `Example/` → sample C code and generated assembly

## Notes

- The emitted assembly uses the System V AMD64 ABI and calls into `libc`
  (e.g., `printf`). It targets Linux and uses AT&T syntax.
- The subset is deliberately tiny and geared toward clarity.
  Expect missing features and rough edges.

## License

See `LICENSE` for details.

