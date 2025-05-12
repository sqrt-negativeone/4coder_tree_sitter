 (identifier) @defcolor_text_default
 
 ((identifier) @defcolor_text_default
	(#match? @defcolor_text_default "^[A-Z][A-Z\\d_]*$"))
 
 [
"break"
 "case" 
 "const" 
 "continue"
 "default"
 "do" 
 "else" 
 "enum" 
 "extern"
 "for" 
 "if" 
 "inline" 
 "return" 
 "sizeof" 
 "static" 
 "struct" 
 "switch" 
 "typedef"
 "union" 
 "volatile"
 "while"

"catch"
 "class"
 "co_await"
 "co_return"
 "co_yield"
 "constexpr"
 "constinit"
 "consteval"
 "delete"
 "explicit"
 "final"
 "friend"
 "mutable"
 "namespace"
 "noexcept"
 "new"
 "override"
 "private"
 "protected"
 "public"
 "template"
 "throw"
 "try"
 "typename"
 "using"
 "concept"
 "requires"
 "virtual"
 "import"
 "export"
 "module"
] @defcolor_keyword
 
[
"#define" 
 "#elif" 
 "#else" 
 "#endif"
 "#if" 
 "#ifdef" 
 "#ifndef" 
 "#include"
 (preproc_defined)
 (preproc_directive)
] @defcolor_preproc
 
[
"..."
"::"
"++"
"--"
"."
"->"
"+"
"-"
"!"
"~"
"*"
"&"
"/"
"%"
"<<"
">>"
"<"
"<="
">"
">="
"=="
"!="
"^"
"|"
"&&"
"||"
"?"
"="
"+="
"-="
"*="
"/="
"%="
"<<="
">>="
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
(string_literal)
(raw_string_literal)
(system_lib_string)
] @defcolor_str_constant

(null) @defcolor_int_constant
(number_literal) @defcolor_int_constant
(char_literal) @defcolor_char_constant

(field_identifier) @defcolor_text_default
(statement_identifier) @defcolor_text_default

[
(sized_type_specifier)
(primitive_type)
] @defcolor_type

(comment) @defcolor_comment


 