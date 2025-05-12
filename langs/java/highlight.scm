(super) @ts_color_builtin_func

(annotation name: (identifier) @ts_color_attribute)
(marker_annotation name: (identifier) @ts_color_attribute)
"@" @ts_color_operator

[
  (boolean_type)
  (integral_type)
  (floating_point_type)
  (floating_point_type)
  (void_type)
] @ts_color_builtin_type

(this) @defcolor_keyword

[
  (hex_integer_literal)
  (decimal_integer_literal)
  (octal_integer_literal)
  (decimal_floating_point_literal)
  (hex_floating_point_literal)
] @defcolor_int_constant

[
  (character_literal)
  (string_literal)
] @defcolor_str_constant
(escape_sequence) @defcolor_str_constant

[
  (true)
  (false)
  (null_literal)
] @defcolor_int_constant

[
  (line_comment)
  (block_comment)
] @defcolor_comment

[
"->"
  "+" "-" "*" "/" "%"               ;; Arithmetic
  "=" "+=" "-=" "*=" "/=" "%=" "&=" "|=" "^=" "<<=" ">>=" ">>>=" ;; Assignment
  "==" "!=" "<" "<=" ">" ">="       ;; Comparison
  "&&" "||" "!"                     ;; Logical
  "&" "|" "^" "~" "<<" ">>" ">>>"   ;; Bitwise
  "++" "--"                         ;; Increment/Decrement
  "?" ":"                           ;; Ternary
] @ts_color_operator

 [
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

[
  "abstract"
  "assert"
  "break"
  "case"
  "catch"
  "class"
  "continue"
  "default"
  "do"
  "else"
  "enum"
  "exports"
  "extends"
  "final"
  "finally"
  "for"
  "if"
  "implements"
  "import"
  "instanceof"
  "interface"
  "module"
  "native"
  "new"
  "non-sealed"
  "open"
  "opens"
  "package"
  "permits"
  "private"
  "protected"
  "provides"
  "public"
  "requires"
  "record"
  "return"
  "sealed"
  "static"
  "strictfp"
  "switch"
  "synchronized"
  "throw"
  "throws"
  "to"
  "transient"
  "transitive"
  "try"
  "uses"
  "volatile"
  "when"
  "while"
  "with"
  "yield"
] @defcolor_keyword
