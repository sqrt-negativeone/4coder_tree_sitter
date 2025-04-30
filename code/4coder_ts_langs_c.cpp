
external const TSLanguage *tree_sitter_c(void);

String_Const_u8 c_index_query_str =
str8_lit
(
 R"scm(

(preproc_function_def name: (identifier) @macro_def)
(preproc_def name: (identifier) @macro_def)

(type_definition
declarator: (function_declarator declarator: (type_identifier) @typedef.function)
)

(function_declarator declarator: (identifier) @function_def)
(function_definition (parenthesized_declarator (identifier) @function_def))

(struct_specifier
name: (type_identifier) @typedef.struct)

(enum_specifier
name: (type_identifier) @typedef.enum)

(union_specifier
name: (type_identifier) @typedef.union)

; (type_identifier) @typedef.type

(enumerator name: (identifier) @enum.const)

(type_definition
	declarator: (parenthesized_declarator
    	(type_identifier) @typedef.type))
        
; declared types
(_declarator declarator: (type_identifier) @typedef.type)

; capture function paramaters
(function_declarator
parameters: (parameter_list
	(parameter_declaration
    	declarator: (identifier) @func_param_name)
    )+
)

)scm"
 );

String_Const_u8 c_highlight_query_str = 
str8_lit
(
 R"scm(
 (identifier) @defcolor_text_default
 
 ((identifier) @defcolor_text_default
	(#match? @defcolor_text_default "^[A-Z][A-Z\\d_]*$"))
 
 ["break"
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
] @defcolor_keyword
 
 ["#define" 
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
 ] @fleury_color_operators
 
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
 ] @fleury_color_syntax_crap

[(string_literal)
(system_lib_string)
] @defcolor_str_constant

(null) @defcolor_int_constant
(number_literal) @defcolor_int_constant
(char_literal) @defcolor_char_constant

(field_identifier) @defcolor_text_default
(statement_identifier) @defcolor_text_default

(primitive_type) @defcolor_type

(comment) @defcolor_comment
 )scm"
 );

global TS_Language c_language;

function void
ts_init_c_language(Application_Links *app, TS_Index_Context *ts_index)
{
	c_language.language = tree_sitter_c();
	
	u32 err_offset = 0;
	TSQueryError err_type;
	c_language.highlight_query = ts_query_new(c_language.language, (char*)c_highlight_query_str.str, (u32)c_highlight_query_str.size, &err_offset, &err_type);
	Assert(c_language.highlight_query);
	if (!c_language.highlight_query)
	{
		print_message(app, str8_lit("couldn't create highlight query"));
	}
	
	c_language.index_query = ts_query_new(c_language.language, (char*)c_index_query_str.str, (u32)c_index_query_str.size, &err_offset, &err_type);
	Assert(c_language.index_query);
	if (!c_language.index_query)
	{
		print_message(app, str8_lit("couldn't create highlight query"));
	}
	
	String_Const_u8 extensions[] = {
		str8_lit("cpp"),
		str8_lit("c"),
		str8_lit("h"),
	};
	
	for (u32 i = 0; i < ArrayCount(extensions); i += 1)
		table_insert(&ts_index->ext_to_language_table, extensions[i], make_data_struct(&c_language));
}
