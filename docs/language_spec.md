# MiniTorch Language Spec

MiniTorch accepts a deliberately small subset of Python. The goal is not to
interpret arbitrary Python, but to capture statically shaped tensor programs
that can be lowered to native code.

## Supported Constructs

- Decorated function definitions using `@minitorch.compile`
- Tensor-typed parameters: `name: Tensor[d0, d1, ...]`
- Tensor return annotations: `-> Tensor[d0, d1, ...]`
- Assignments: `name = expr`
- Return statements: `return expr`
- Function calls: `matmul(x, w)`, `relu(x)`, `softmax(x)`
- Binary operators: `+` and `*`
- Identifiers and numeric literals

## Supported Tensor Operations

- `matmul(A, B)`
- `relu(x)`
- `sigmoid(x)`
- `softmax(x)`
- `transpose(x)`
- `reshape(x, shape)`
- `+` for tensor addition

## Not Supported Initially

- `import` statements
- Classes
- Closures
- Lists and dictionaries as general values
- `for` and `while` loops
- Dynamic tensor shapes
- Runtime dispatch based on tensor rank or dtype
- Arbitrary PyTorch APIs

## Grammar Sketch

```text
module      ::= function*
function    ::= decorator? "def" ident "(" params? ")" "->" tensor_type ":" suite
decorator   ::= "@" dotted_name newline
params      ::= param ("," param)*
param       ::= ident ":" tensor_type
tensor_type ::= "Tensor" "[" number ("," number)* "]"
suite       ::= newline indent statement+ dedent
statement   ::= ident "=" expr newline
              | "return" expr newline
expr        ::= term ("+" term)*
term        ::= primary ("*" primary)*
primary     ::= number
              | ident
              | dotted_name "(" args? ")"
              | "(" expr ")"
args        ::= expr ("," expr)*
```

