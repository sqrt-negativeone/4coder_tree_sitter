

(predefined_type) @ts_color_builtin_type

;; Literals

[
  (real_literal)
  (integer_literal)
  (boolean_literal)
  (null_literal)
] @defcolor_int_constant

[
  (character_literal)
  (string_literal)
  (raw_string_literal)
  (verbatim_string_literal)
  (interpolated_string_expression)
  (interpolation_start)
  (interpolation_quote)
 (escape_sequence)
] @defcolor_str_constant

;; Comments

(comment) @defcolor_comment

;; Tokens

[
  ";"
  "."
  ","
  "("
  ")"
  "["
  "]"
  "{"
  "}"
  (interpolation_brace)
] @ts_color_syntax_crap

[
  "--"
  "-"
  "-="
  "&"
  "&="
  "&&"
  "+"
  "++"
  "+="
  "<"
  "<="
  "<<"
  "<<="
  "="
  "=="
  "!"
  "!="
  "=>"
  ">"
  ">="
  ">>"
  ">>="
  ">>>"
  ">>>="
  "|"
  "|="
  "||"
  "?"
  "??"
  "??="
  "^"
  "^="
  "~"
  "*"
  "*="
  "/"
  "/="
  "%"
  "%="
  ":"
] @ts_color_operator

;; Keywords

[
    (modifier)
  "this"
  (implicit_type)
"add"
  "alias"
  "as"
  "base"
  "break"
  "case"
  "catch"
  "checked"
  "class"
  "continue"
  "default"
  "delegate"
  "do"
  "else"
  "enum"
  "event"
  "explicit"
  "extern"
  "finally"
  "for"
  "foreach"
  "global"
  "goto"
  "if"
  "implicit"
  "interface"
  "is"
  "lock"
  "namespace"
  "notnull"
  "operator"
  "params"
  "return"
  "remove"
  "sizeof"
  "stackalloc"
  "static"
  "struct"
  "switch"
  "throw"
  "try"
  "typeof"
  "unchecked"
  "using"
  "while"
  "new"
  "await"
  "in"
  "yield"
  "get"
  "set"
  "when"
  "out"
  "ref"
  "from"
  "where"
  "select"
  "record"
  "init"
  "with"
  "let"
] @defcolor_keyword

;; Attribute

(attribute name: (identifier) @ts_color_attribute)

