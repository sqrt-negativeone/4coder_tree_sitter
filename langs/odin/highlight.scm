

; Preprocs

[
  (calling_convention)
  (tag)
] @defcolor_preproc

; Includes

[
  "import"
  "package"
] @defcolor_preproc

; Keywords

[
  "foreign"
  "using"
  "struct"
  "enum"
  "union"
  "defer"
  "cast"
  "transmute"
  "auto_cast"
  "map"
  "bit_set"
  "matrix"
  "bit_field"
  "proc"
  "return"
  "or_return"
  "distinct"
  "dynamic"
  "if"
  "else"
  "when"
  "switch"
  "case"
  "where"
  "break"
  (fallthrough_statement)
  "for"
  "do"
  "continue"
] @defcolor_keyword

((ternary_expression
  [
    "?"
    ":"
    "if"
    "else"
    "when"
  ] @ts_color_operator)
  (#set! "priority" 105))

;(type (identifier) @ts_color_builtin_type
;  (#any-of? @ts_color_builtin_type
;    "bool" "byte" "b8" "b16" "b32" "b64"
;    "int" "i8" "i16" "i32" "i64" "i128"
;    "uint" "u8" "u16" "u32" "u64" "u128" "uintptr"
;    "i16le" "i32le" "i64le" "i128le" "u16le" "u32le" "u64le" "u128le"
;    "i16be" "i32be" "i64be" "i128be" "u16be" "u32be" "u64be" "u128be"
;    "float" "double" "f16" "f32" "f64" "f16le" "f32le" "f64le" "f16be" "f32be" "f64be"
;    "complex32" "complex64" "complex128" "complex_float" "complex_double"
;    "quaternion64" "quaternion128" "quaternion256"
;    "rune" "string" "cstring" "rawptr" "typeid" "any"))


; Attributes

(attribute (identifier) @ts_color_attribute "="?)

; Labels

(label_statement (identifier) @ts_color_label ":")

; Literals

[
 (number)
 (float)
 (uninitialized)
 (nil)
 (boolean) @defcolor_int_constant
] @defcolor_int_constant

[
(string)
(character)
(escape_sequence)
] @defcolor_str_constant



;((identifier) @variable.builtin
;  (#any-of? @variable.builtin "context" "self"))

; Operators

[
  ":="
  "="
  "+"
  "-"
  "*"
  "/"
  "%"
  "%%"
  ">"
  ">="
  "<"
  "<="
  "=="
  "!="
  "~="
  "|"
  "~"
  "&"
  "&~"
  "<<"
  ">>"
  "||"
  "&&"
  "!"
  "^"
  ".."
  "+="
  "-="
  "*="
  "/="
  "%="
  "&="
  "|="
  "^="
  "<<="
  ">>="
  "||="
  "&&="
  "&~="
  "..="
  "..<"
  "?"
  "::"
  "->"
] @ts_color_operator

[
  "or_else"
  "in"
  "not_in"
] @defcolor_keyword

; Punctuation

[
 "{"
 "}"

 "("
 ")"

 "["
 "]"

  "."
  ","
  ":"
  ";"

  "@"
  "$"
] @ts_color_syntax_crap

; Comments

[
  (comment)
  (block_comment)
] @defcolor_comment

; Errors

;(ERROR) @ts_color_error_annotation

; Types

;((identifier) @ts_color_builtin_type 
;	(#match? @ts_color_builtin_type "blabla"))
