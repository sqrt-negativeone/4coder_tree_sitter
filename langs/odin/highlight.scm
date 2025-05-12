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

; Repeats

; Variables

;(identifier) @variable

; Namespaces

; (package_declaration (identifier) @namespace)

; (import_declaration alias: (identifier) @namespace)

; (foreign_block (identifier) @namespace)

; (using_statement (identifier) @namespace)

; Parameters

;(parameter (identifier) @parameter ":" "="? (identifier)? @constant)

;(default_parameter (identifier) @parameter ":=")

;(named_type (identifier) @parameter)

;(call_expression argument: (identifier) @parameter "=")

; Functions

;(procedure_declaration (identifier) @type)

;(procedure_declaration (identifier) @function (procedure (block)))

;(procedure_declaration (identifier) @function (procedure (uninitialized)))

;(overloaded_procedure_declaration (identifier) @function)

;(call_expression function: (identifier) @function.call)

; Types

;(type (identifier) @type)

((type (identifier) @ts_color_builtin_type)
  (#any-of? @ts_color_builtin_type
    "bool" "byte" "b8" "b16" "b32" "b64"
    "int" "i8" "i16" "i32" "i64" "i128"
    "uint" "u8" "u16" "u32" "u64" "u128" "uintptr"
    "i16le" "i32le" "i64le" "i128le" "u16le" "u32le" "u64le" "u128le"
    "i16be" "i32be" "i64be" "i128be" "u16be" "u32be" "u64be" "u128be"
    "float" "double" "f16" "f32" "f64" "f16le" "f32le" "f64le" "f16be" "f32be" "f64be"
    "complex32" "complex64" "complex128" "complex_float" "complex_double"
    "quaternion64" "quaternion128" "quaternion256"
    "rune" "string" "cstring" "rawptr" "typeid" "any"))

"..." @ts_color_builtin_type

;(struct_declaration (identifier) @type "::")

;(enum_declaration (identifier) @type "::")

;(union_declaration (identifier) @type "::")

;(bit_field_declaration (identifier) @type "::")

;(const_declaration (identifier) @type "::" [(array_type) (distinct_type) (bit_set_type) (pointer_type)])

;(struct . (identifier) @type)

;(field_type . (identifier) @namespace "." (identifier) @type)

;(bit_set_type (identifier) @type ";")

;(procedure_type (parameters (parameter (identifier) @type)))

;(polymorphic_parameters (identifier) @type)

;((identifier) @type
;  (#lua-match? @type "^[A-Z][a-zA-Z0-9]*$")
;  (#not-has-parent? @type parameter procedure_declaration call_expression))

; Fields

;(member_expression "." (identifier) @field)

;(struct_type "{" (identifier) @field)

;(struct_field (identifier) @field "="?)

;(field (identifier) @field)

; Constants

;((identifier) @constant
;  (#lua-match? @constant "^_*[A-Z][A-Z0-9_]*$")
;  (#not-has-parent? @constant type parameter))

;(member_expression . "." (identifier) @constant)

;(enum_declaration "{" (identifier) @constant)

; Macros

;((call_expression function: (identifier) @function.macro)
;  (#lua-match? @function.macro "^_*[A-Z][A-Z0-9_]*$"))

; Attributes

(attribute (identifier) @ts_color_attribute "="?)

; Labels

(label_statement (identifier) @ts_color_label ":")

; Literals

(number) @defcolor_int_constant

(float) @defcolor_int_constant

(string) @defcolor_str_constant

(character) @defcolor_str_constant

(escape_sequence) @defcolor_str_constant

(boolean) @defcolor_int_constant

[
  (uninitialized)
  (nil)
] @defcolor_int_constant

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

(ERROR) @ts_color_error_annotation