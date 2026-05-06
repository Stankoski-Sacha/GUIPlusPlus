# GUI++ Language Specification

> **Note:** This is a living document. Sections marked `[TODO]` are placeholders to be completed as the language evolves.

---

## 1. Overview

GUI++ is a declarative language for describing graphical user interfaces. It uses a structured, hierarchical syntax inspired by CSS and JSON-like block notation, where UI components are defined as nested blocks with typed sections.

The file extension for GUI++ source files is `.gui`.

---

## 2. File Structure

A GUI++ file is a sequence of one or more **top-level component declarations**. Each component is a named block that may contain one or more **sections**.

```
ComponentName {
    .sectionName {
        key: value;
    }
}
```

Top-level components are not nested inside any other block. Multiple top-level components can exist in the same file.

---

## 3. Syntax

### 3.1 Component Declaration

A component is declared by its **type name** followed by a block enclosed in curly braces `{ }`.

```
Window {
    ...
}
```

Component type names are **PascalCase** (e.g., `Window`, `Button`, `Label`).

### 3.2 Sections

Inside a component block, **sections** group related properties. A section begins with a dot `.` followed by its name, and its body is enclosed in curly braces.

```
Window {
    .data {
        ...
    }

    .content {
        ...
    }
}
```

#### Built-in Sections

| Section     | Purpose                                              |
|-------------|------------------------------------------------------|
| `.data`     | Defines the component's properties (size, position, title, etc.) |
| `.content`  | Defines the component's child components             |

> [TODO] Document any additional built-in or user-defined sections.

### 3.3 Properties

Properties are defined inside sections as `key: value;` pairs. Each property statement ends with a semicolon `;`.

```
.data {
    position: 200, 200;
    dimensions: 400, 400;
    title: "Button Window";
}
```

#### Property Value Types

| Type        | Syntax example          | Description                            |
|-------------|-------------------------|----------------------------------------|
| String      | `"Hello"`               | UTF-8 text enclosed in double quotes   |
| Integer     | `42`                    | Whole number                           |
| Float       | `3.14`                  | Decimal number                         |
| Pair        | `200, 200`              | Two comma-separated numeric values     |

> [TODO] Document additional types (booleans, colors, enums, etc.) if supported.

### 3.4 Nesting (Children)

Child components are declared inside a parent's `.content` section. They follow the same component declaration syntax as top-level components.

```
Window {
    .content {
        Button {
            .data {
                position: 100, 100;
                dimensions: 100, 50;
                text: "Click me";
            }
        }
    }
}
```

Nesting can be arbitrarily deep.

---

## 4. Built-in Components

### 4.1 `Window`

The root container for a GUI application. A `.gui` file typically declares at least one `Window`.

**Properties (`.data` section):**

| Property     | Type   | Description                              |
|--------------|--------|------------------------------------------|
| `position`   | Pair   | X, Y position of the window on screen    |
| `dimensions` | Pair   | Width, height of the window in pixels    |
| `title`      | String | Text shown in the window's title bar     |

**Example:**

```
Window {
    .data {
        position: 200, 200;
        dimensions: 600, 600;
        title: "My Window";
    }
}
```

---

### 4.2 `Button`

A clickable button widget.

**Properties (`.data` section):**

| Property     | Type   | Description                                  |
|--------------|--------|----------------------------------------------|
| `position`   | Pair   | X, Y position relative to the parent widget  |
| `dimensions` | Pair   | Width, height of the button in pixels         |
| `text`       | String | Label text displayed on the button            |

**Example:**

```
Button {
    .data {
        position: 100, 100;
        dimensions: 100, 50;
        text: "Submit";
    }
}
```

> [TODO] Document remaining built-in components (Label, TextInput, Image, Panel, etc.).

---

## 5. Comments

> [TODO] Document comment syntax if supported (e.g., `//`, `/* */`, `#`).

---

## 6. Whitespace and Formatting

- Whitespace (spaces, tabs, newlines) between tokens is ignored.
- Indentation is conventional (one tab per nesting level) but not enforced by the parser.
- Semicolons `;` are required to terminate property declarations.

---

## 7. Complete Examples

### Minimal Window (`temp.gui`)

```
Window {
    .data {
        dimensions: 600, 600;
        position: 200, 200;
        title: "GUI++";
    }
}
```

### Window with a Button (`main.gui`)

```
Window {
    .data {
        position: 200, 200;
        dimensions: 400, 400;
        title: "Button Window";
    }

    .content {
        Button {
            .data {
                position: 100, 100;
                dimensions: 100, 50;
                text: "Button";
            }
        }
    }
}
```

---

## 8. Grammar (EBNF)

```ebnf
program        ::= component+
component      ::= IDENTIFIER "{" section* "}"
section        ::= "." IDENTIFIER "{" property* component* "}"
property       ::= IDENTIFIER ":" value ";"
value          ::= string | pair | number
pair           ::= number "," number
string         ::= '"' [^"]* '"'
number         ::= [0-9]+ ("." [0-9]+)?
IDENTIFIER     ::= [A-Za-z_][A-Za-z0-9_]*
```

> [TODO] Refine grammar as the language grows (events, conditionals, variables, etc.).

---

## 9. Error Handling

> [TODO] Document parse errors, type errors, and how the compiler reports them.

---

## 10. Roadmap / Future Features

> [TODO] List planned features such as:
> - Event handlers (e.g., `onClick`)
> - Variables and expressions
> - Imports / file splitting
> - Theming / style inheritance
> - Layout managers (flex, grid, absolute)
