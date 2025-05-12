
; Function calls

(package_clause "package" (package_identifier) @ts_color_namespace)

(call_expression
  function: (identifier) @ts_color_builtin_func
  (#match? @ts_color_builtin_func "^(append|cap|close|complex|copy|delete|imag|len|make|new|panic|print|println|real|recover)$"))

((type_identifier) @ts_color_builtin_type
(#match? @ts_color_builtin_type "^(bool|byte|complex64|complex128|error|float32|float64|int|int8|int16|int32|int64|rune|string|uint|uint8|uint16|uint32|uint64|uintptr)$"))

; Operators

[
  "--"
  "-"
  "-="
  ":="
  "!"
  "!="
  "..."
  "*"
  "*"
  "*="
  "/"
  "/="
  "&"
  "&&"
  "&="
  "%"
  "%="
  "^"
  "^="
  "+"
  "++"
  "+="
  "<-"
  "<"
  "<<"
  "<<="
  "<="
  "="
  "=="
  ">"
  ">="
  ">>"
  ">>="
  "|"
  "|="
  "||"
  "~"
] @ts_color_operator

 [
":"
"," 
"." 
 ";" 
 "("
 ")"
"["
"]"
"{"
"}"
 ] @ts_color_syntax_crap

; Keywords
[
  "break"
  "case"
  "chan"
  "const"
  "continue"
  "default"
  "defer"
  "else"
  "fallthrough"
  "for"
  "func"
  "go"
  "goto"
  "if"
  "import"
  "interface"
  "map"
  "package"
  "range"
  "return"
  "select"
  "struct"
  "switch"
  "type"
  "var"
] @defcolor_keyword

; Literals

[
  (interpreted_string_literal)
  (raw_string_literal)
  (rune_literal)
(escape_sequence)
] @defcolor_str_constant


[
  (int_literal)
  (float_literal)
  (imaginary_literal)
  (true)
  (false)
  (nil)
  (iota)
] @defcolor_int_constant

(comment) @defcolor_comment

