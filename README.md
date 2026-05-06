# GUI++ Compiler Internals Specification

This document describes the internal architecture of the GUI++ compiler — how a `.gui` source file is processed, what each stage does, and how the stages connect together.

---

## 1. Overview

GUI++ is a **transpiling compiler**. It does not emit machine code or bytecode directly. Instead it:

1. Reads a `.gui` source file
2. Lexes, parses, and compiles it into an in-memory component tree
3. Generates equivalent C++ source code that uses SDL2 and SDL2_ttf
4. Writes that C++ to a randomly named temporary file
5. Invokes `g++` (or `clang++`) to compile that file into a native executable
6. Deletes the temporary file (unless `-keep-temp` is passed)

```
 .gui source
     │
     ▼
 ┌─────────┐     token stream     ┌─────────┐     token stream     ┌──────────────┐
 │  Lexer  │ ──────────────────▶ │  Parser │ ──────────────────▶  │   Compiler   │
 └─────────┘                     └─────────┘  (validation only)   └──────────────┘
                                                                          │
                                                                  ComponentNode tree
                                                                          │
                                                                          ▼
                                                                  ┌──────────────┐
                                                                  │   Code Gen   │
                                                                  └──────────────┘
                                                                          │
                                                                   C++ source string
                                                                          │
                                                                          ▼
                                                                  ┌──────────────┐
                                                                  │  g++/clang++ │
                                                                  └──────────────┘
                                                                          │
                                                                   native executable
```

---

## 2. CLI — `main.cpp`

The entry point handles argument parsing and orchestrates the pipeline.

### 2.1 Usage

```
GUI++ <source.gui> [-o <output>] [-keep-temp] [-clang++] [--help]
```

### 2.2 Flags

| Flag          | Effect                                                                 |
|---------------|------------------------------------------------------------------------|
| `<file>.gui`  | Source file to compile. Detected by the `.gui` extension.              |
| `-o <name>`   | Name of the output executable. Defaults to `a.out`.                   |
| `-keep-temp`  | Do not delete the intermediate C++ file after compilation.             |
| `-clang++`    | Use `clang++` instead of `g++` to compile the generated C++.          |
| `--help`      | Print usage information and exit.                                      |

### 2.3 Argument Parsing

Arguments are scanned in a single forward loop over `argv`. Detection is done with `std::string::find` on each argument:

- Any argument containing `.gui` → source filename
- `-o` sets a flag; the **next** argument that is not a flag and not a `.gui` file becomes the output name
- `-keep-temp`, `-clang++`, `--help` → boolean flags

If no `.gui` file is found among the arguments, the compiler prints an error and exits with code `1`.

### 2.4 Temporary File Naming

The intermediate C++ file is given a randomly generated name to avoid collisions:

```cpp
mt19937 random_gen(random_device{}());
uniform_int_distribution<int> dist(10, 30);
int size = dist(random_gen);           // length between 10 and 30 chars
// each char: dist(random_gen) % 26 + 'a'  → random lowercase letter
file_name += ".cpp";
```

The file is written to the **current working directory**, compiled, then deleted via `std::filesystem::remove` unless `-keep-temp` was passed.

### 2.5 Compilation Command

```cpp
// g++ (default)
"g++ <tempfile>.cpp -o <output> -lSDL2 -lSDL2_ttf"

// clang++ (with -clang++ flag)
"clang++ <tempfile>.cpp -o <output> -lSDL2 -lSDL2_ttf"
```

The command is executed with `std::system()`.

> **Note:** There is a known bug in the current code — the `if (useClang)` branch assigns `command` but then the line immediately after it unconditionally overwrites `command` with the `g++` version, so `-clang++` has no effect yet. The fix is to add an `else` before the second assignment.

---

## 3. Lexer — `Lexer::Lexer` (`lexer.hpp`)

### 3.1 Responsibility

Converts the raw text of a `.gui` file into a flat `std::vector<Lexer::Token>`.

### 3.2 Token Types (`Lexer::TokenType`)

| Token type       | Example lexeme   | Description                              |
|------------------|------------------|------------------------------------------|
| `IDENTIFIER`     | `Window`, `data` | Alphabetic word (not a keyword per se)   |
| `INT_LITERAL`    | `400`, `200`     | One or more digit characters             |
| `STRING_LITERAL` | `Hello`          | Text between `"` double quotes (quotes stripped) |
| `BOOL_LITERAL`   | `true`, `false`  | The exact strings `true` or `false`      |
| `LEFT_BRACE`     | `{`              | Opening curly brace                      |
| `RIGHT_BRACE`    | `}`              | Closing curly brace                      |
| `COLON`          | `:`              | Property key/value separator             |
| `SEMICOLON`      | `;`              | Property statement terminator            |
| `COMMA`          | `,`              | Pair value separator                     |
| `DOT`            | `.`              | Section prefix (`.data`, `.content`)     |
| `EndOfFile`      | —                | Sentinel, not currently emitted explicitly |
| `WTF`            | `?`              | Any character the lexer does not recognise |

### 3.3 Token Struct

```cpp
struct Token {
    TokenType   type;
    std::string lexeme;  // the raw text of the token
    int         line;    // 1-based source line number
};
```

### 3.4 Lexing Algorithm

The lexer reads the entire file into a `std::string` via `std::getline`, then walks it character by character with an index `i`:

| Character class      | Action                                                                 |
|----------------------|------------------------------------------------------------------------|
| `\n`                 | Increment `current_line`, skip                                         |
| Other whitespace     | Skip                                                                   |
| `isalpha`            | Consume alphanumeric run → `IDENTIFIER` or `BOOL_LITERAL`             |
| `isdigit`            | Consume digit run → `INT_LITERAL`                                      |
| `"`                  | Consume until next `"` → `STRING_LITERAL` (quotes not included in lexeme) |
| `{`                  | `LEFT_BRACE`                                                           |
| `}`                  | `RIGHT_BRACE`                                                          |
| `.`                  | `DOT`                                                                  |
| `:`                  | `COLON`                                                                |
| `,`                  | `COMMA`                                                                |
| `;`                  | `SEMICOLON`                                                            |
| Anything else        | `WTF` (unknown — does not stop lexing)                                 |

**Boolean detection:** after consuming an alphabetic run, if the resulting string equals `"true"` or `"false"` it is emitted as `BOOL_LITERAL` instead of `IDENTIFIER`.

**Identifiers** are defined as a leading `isalpha` character followed by `isalnum` characters. Underscores are not currently supported in identifiers.

---

## 4. Parser — `Parser::Parser` (`parser.hpp`)

### 4.1 Responsibility

Performs a **validation-only** pass over the token stream. It does not build an AST or produce any output data — it either returns `true` (the token stream is valid) or throws `std::runtime_error` with a message that includes the source line number.

### 4.2 Key Methods

| Method              | Description                                                                 |
|---------------------|-----------------------------------------------------------------------------|
| `mainParserLoop()`  | Entry point. Iterates top-level components until EOF.                       |
| `parseComponent()`  | Parses one `ComponentName { ... }` block. Recursive via `.content`.         |
| `parseSectionBlock()` | Parses one `.sectionName { ... }` block; dispatches to data or content.   |
| `parseDataBlock()`  | Validates all `key: value;` pairs inside a `.data` block.                   |
| `parseContentBlock()` | Validates child components inside a `.content` block; calls `parseComponent()` for each. |
| `expect(type)`      | Asserts current token is of the given type; throws if not. Does not advance.|
| `consume(type)`     | `expect` + advance. Returns the consumed token.                             |
| `peek()`            | Returns the token one ahead without advancing.                              |
| `atEnd()`           | True if `pos` is past the end or at `EndOfFile`.                           |

### 4.3 Grammar Enforced

```
program         ::= component+
component       ::= IDENTIFIER '{' section+ '}'
section         ::= '.' IDENTIFIER '{' (property* | component*) '}'
property        ::= IDENTIFIER ':' value ';'
value           ::= INT ',' INT          (pair)
                  | STRING_LITERAL       (string)
                  | BOOL_LITERAL         (bool)
                  | INT ',' INT          (range — same shape as pair)
```

### 4.4 Context-Sensitive Rules Enforced

| Property   | Only valid in          |
|------------|------------------------|
| `title`    | `Window`               |
| `text`     | `Button`, `TextBox`    |
| `editable` | `TextBox`              |
| `range`    | `Slider`               |
| `position` | Any component          |
| `dimensions` | Any component        |
| `.content` | `Window` only          |

### 4.5 Context Tracking

The parser maintains a `Context currentContext` enum that is updated each time a component name (`Window`, `Button`, etc.) is recognised. When parsing nested components inside `.content`, the parent context is saved to a local variable and restored after the child `parseComponent()` call returns, so nesting does not corrupt the parent's context.

### 4.6 Error Format

```
Parse error on line <N>: expected <TokenType> but got <TokenType> ('<lexeme>').
Parse error on line <N>: '<property>' is only valid inside a <Component>.
Parse error on line <N>: unknown section '.<name>'. Expected '.data' or '.content'.
Parse error: component '<name>' on line <N> has no '.data' section.
```

---

## 5. Compiler — `Compiler::compiler` (`compiler.hpp`)

### 5.1 Responsibility

Walks the same flat token stream (not the parser output) and builds a `ComponentNode` — a single struct that holds all the data needed by the code generator.

### 5.2 Data Structures

#### `ComponentNode`

The root output of the compiler stage. Passed directly to the code generator.

```cpp
struct ComponentNode {
    WindowNode            win;
    std::vector<Button>   buttonsCreated;
    std::vector<TextBox>  TextBoxCreated;
    std::vector<CheckBox> CheckBoxCreated;
    std::vector<Slider>   sliderCreated;
};
```

#### Per-component structs

| Struct       | Fields                                      |
|--------------|---------------------------------------------|
| `WindowNode` | `title`, `x`, `y`, `w`, `h`, `flag`        |
| `Button`     | `buttonText`, `x`, `y`, `w`, `h`           |
| `TextBox`    | `defaultText`, `x`, `y`, `w`, `h`, `isEditable` |
| `CheckBox`   | `x`, `y`, `w`, `h`                         |
| `Slider`     | `x`, `y`, `w`, `h`, `minVal`, `maxVal`     |

`WindowNode::flag` defaults to `SDL_WINDOW_SHOWN`. There is currently no `.gui` syntax to override it.

### 5.3 `makeNode()` — Main Loop

Iterates the token vector. When it encounters an `IDENTIFIER` it checks if the lexeme is a component keyword and updates a local `Context` enum. When it then sees the identifier `"data"`, it:

1. Finds the next `LEFT_BRACE` after `"data"` using `std::find_if`
2. Finds the matching `RIGHT_BRACE` after that
3. Extracts the sub-vector of tokens between them
4. Dispatches to the appropriate `make*` method based on the current context
5. Advances `i` past the closing brace

### 5.4 `make*` Methods

Each `make*` method (e.g. `makeWindow`, `makeButton`, `makeSlider`) receives the sub-vector of tokens from inside a `.data` block and scans it for known property identifiers:

**`returnVals(toks)`** — shared helper that extracts the first two `INT_LITERAL` tokens from a sub-slice (used for both `position` and `dimensions`).

Each method advances its local index `i` past the semicolon after processing each property, using a `do { i++ } while (tok != SEMICOLON)` pattern.

| Method          | Properties extracted                              |
|-----------------|---------------------------------------------------|
| `makeWindow`    | `dimensions`, `position`, `title`                 |
| `makeButton`    | `dimensions`, `position`, `text`                  |
| `makeTextBox`   | `dimensions`, `position`, `text`, `BOOL_LITERAL`  |
| `makeCheckBox`  | `dimensions`, `position`                          |
| `makeSlider`    | `dimensions`, `position`, `range`                 |

> **Note:** The compiler stage operates independently of the parser and does its own token scanning. The parser validates structure; the compiler extracts meaning.

---

## 6. Code Generator — `CODEGEN::Code_Gen` (`code_gen.hpp`)

### 6.1 Responsibility

Takes a `ComponentNode` and produces a single `std::string` containing complete, compilable C++ source code that recreates the described UI using SDL2 and SDL2_ttf.

### 6.2 `make_final_code()` — Build Order

The generated C++ file is assembled in this order:

```
1. #include headers          (SDL2, SDL2_ttf, string)
2. Widget class source       (Button.hpp / TextBox.hpp / CheckBox.hpp / Slider.hpp — inlined verbatim)
3. int main(...)  {
4.   SDL_Init / TTF_Init
5.   SDL_CreateWindow / SDL_CreateRenderer
6.   TTF_OpenFont
7.   Widget instantiation    (Button, CheckBox, Slider constructors)
8.   Event loop — part 1     (SDL_PollEvent, SDL_QUIT, SDL_MOUSEBUTTONDOWN open)
9.     isInside checks       (CheckBox hit testing)
10.  Event loop — part 2     (close event block, SDL_RenderClear)
11.   Widget render calls    (.render(ren) for each widget)
12.  Event loop end          (SDL_RenderPresent, SDL_Delay(16))
13.  Cleanup                 (DestroyWindow, DestroyRenderer, CloseFont, Quit)
14. }
```

### 6.3 Private Methods

| Method                        | Output produced                                                  |
|-------------------------------|------------------------------------------------------------------|
| `headerCode()`                | `#include` directives                                            |
| `mainFunctionEntry()`         | `int main(int argc, char** argv) {`                              |
| `initCode()`                  | `SDL_Init` + `TTF_Init`                                          |
| `makeWindowCode()`            | `SDL_CreateWindow`, `SDL_CreateRenderer`, `bool running`, `SDL_Event e` |
| `openFontCode()`              | `TTF_OpenFont("LiberationSans-Regular.ttf", 20)`                 |
| `makeButtonInitCode(id)`      | `Button button<id> = Button(x, y, w, h, "text", font);`         |
| `makeCheckBoxInitCode(id)`    | `CheckBox check<id> = CheckBox(x, y, w, h);`                    |
| `makeSliderInitCode(id)`      | `Slider slider<id> = Slider(x, y, w, h, font, min, max);`       |
| `makeCheckBoxisInsideCode(id)`| `if (check<id>.isInside(mx, my)) { check<id>.switchBox(ren); }` |
| `renderCode(name)`            | `<name>.render(ren);`                                            |
| `renderLoopCodePart1()`       | Event loop open, mouse state, QUIT and MOUSEBUTTONDOWN handlers  |
| `renderLoopCodePart2()`       | Close event blocks, `SDL_RenderClear`                            |
| `renderLoopCodeEnd()`         | `SDL_RenderPresent`, `SDL_Delay(16)`, close loop                 |
| `cleanupCode()`               | SDL and TTF teardown, `return 0;`, close `main`                  |
| `addClassCode(filepath)`      | Reads a widget `.hpp` file from disk and inlines it verbatim     |
| `embedFontCode()`             | Copies `LiberationSans-Regular.ttf` from the project source dir to CWD |

### 6.4 Widget Class Inlining

Widget implementations (`Button.hpp`, `TextBox.hpp`, `CheckBox.hpp`, `Slider.hpp`) are not `#include`d in the normal C++ sense. Instead, `addClassCode()` reads them from disk at compile time using their absolute path (constructed from the `PROJECT_SOURCE_DIR` CMake definition) and pastes their full text into the generated source before `main`. This means the generated `.cpp` is entirely self-contained.

### 6.5 Widget Indexing

Widgets are indexed from `size - 1` down to `0` in all loops. This means they are instantiated and rendered in reverse order relative to how they appear in the `.gui` file. This does not affect correctness for the current feature set but is worth knowing for future z-order handling.

### 6.6 Font

The font is hardcoded to `LiberationSans-Regular.ttf` at size `20`. `embedFontCode()` copies the font file from `PROJECT_SOURCE_DIR/font/` to the current working directory so the compiled executable can find it at runtime. The font path in the generated code is a bare filename (no directory), so the executable must be run from the directory where the font was copied.

### 6.7 Frame Rate

The render loop calls `SDL_Delay(16)` at the end of every frame, targeting approximately 62.5 fps. This is a fixed delay and does not account for frame processing time.

---

## 7. Component Support Matrix

| Component  | Lexed | Parsed | Compiled | Code Generated | Notes                          |
|------------|:-----:|:------:|:--------:|:--------------:|--------------------------------|
| `Window`   | ✅    | ✅     | ✅       | ✅             | One per file expected          |
| `Button`   | ✅    | ✅     | ✅       | ✅             |                                |
| `TextBox`  | ✅    | ✅     | ✅       | ⚠️             | `makeTextBoxisInsideCode` returns `""` — no click handling yet |
| `CheckBox` | ✅    | ✅     | ✅       | ✅             |                                |
| `Slider`   | ✅    | ✅     | ✅       | ✅             |                                |

---

## 8. Known Issues and TODOs

| Location         | Issue                                                                                          |
|------------------|------------------------------------------------------------------------------------------------|
| `main.cpp`       | `-clang++` flag has no effect — the `g++` command unconditionally overwrites it. Add `else`.  |
| `main.cpp`       | `system()` is used to invoke the C++ compiler — no error checking on its return value.        |
| `code_gen.hpp`   | `TextBox` click/input handling is stubbed out (`makeTextBoxisInsideCode` returns `""`).       |
| `code_gen.hpp`   | `embedFontCode()` copies the font but its return value is never used in `make_final_code()`.  |
| `code_gen.hpp`   | Widget render order is reversed (highest index rendered first).                                |
| `code_gen.hpp`   | Font size (20) and frame delay (16 ms) are hardcoded with no `.gui` syntax to override them.  |
| `compiler.hpp`   | `WindowNode::flag` is always `SDL_WINDOW_SHOWN` — no `.gui` syntax for window flags yet.     |
| `parser.hpp`     | The parser and compiler both walk the token stream independently — the parser output is not reused by the compiler. |
