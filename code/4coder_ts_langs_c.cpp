
enum
{
	Index_Note_C_None,
	Index_Note_C_Macro,
	Index_Note_C_Function,
	Index_Note_C_Product_Type,
	Index_Note_C_Sum_Type,
	Index_Note_C_Enum_Const,
};

external const TSLanguage *tree_sitter_c(void);

global String_Const_u8 c_scope_query_str =
str8_lit
(
 R"scm(
[
(_ "{"@scope.open "}" @scope.close)
(_ "("@scope.open ")" @scope.close)
(_ "["@scope.open "]" @scope.close)
] @scope
)scm"
 );

global String_Const_u8 c_index_query_str =
str8_lit
(
 R"scm(

(preproc_function_def name: (identifier) @macro_def)
(preproc_def name: (identifier) @macro_def)

(type_definition
declarator: (function_declarator declarator: (type_identifier) @typedef.function)
)

 ;(function_declarator  (parenthesized_declarator (pointer_declarator [(identifier) (field_identifier)] @function_pointer)))
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

global String_Const_u8 c_highlight_query_str = 
str8_lit
(
 R"scm(
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

[(string_literal)
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
 )scm"
 );

global TS_Language c_language;


function String_Const_u8
ts_get_lister_note_kind_text_c(TS_Index_Note *note, Arena *arena)
{
	String_Const_u8 result = str8_lit("");
	if (!note) return result;
	
	switch (note->kind)
	{
		case Index_Note_C_Macro:
		{
			result = str8_lit("macro");
		} break;
		case Index_Note_C_Function:
		{
			result = str8_lit("function");
		} break;
		case Index_Note_C_Product_Type:
		{
			result = str8_lit("type [product]");
		} break;
		case Index_Note_C_Sum_Type:
		{
			result = str8_lit("type [sum]");
		} break;
		case Index_Note_C_Enum_Const:
		{
			result = str8_lit("constant");
		} break;
	}
	
	return result;
}

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
	
	
	c_language.scope_query = ts_query_new(c_language.language, (char*)c_scope_query_str.str, (u32)c_scope_query_str.size, &err_offset, &err_type);
	Assert(c_language.scope_query);
	if (!c_language.scope_query)
	{
		print_message(app, str8_lit("couldn't create highlight query"));
	}
	
	c_language.name_to_note_kind_table = make_table_Data_u64(ts_index->arena.base_allocator, 32);
	table_insert(&c_language.name_to_note_kind_table, str8_lit("macro_def"),        Index_Note_C_Macro);
	table_insert(&c_language.name_to_note_kind_table, str8_lit("function_def"),     Index_Note_C_Function);
	table_insert(&c_language.name_to_note_kind_table, str8_lit("typedef.type"),     Index_Note_C_Product_Type);
	table_insert(&c_language.name_to_note_kind_table, str8_lit("typedef.function"), Index_Note_C_Product_Type);
	table_insert(&c_language.name_to_note_kind_table, str8_lit("typedef.struct"),   Index_Note_C_Product_Type);
	table_insert(&c_language.name_to_note_kind_table, str8_lit("typedef.enum"),     Index_Note_C_Product_Type);
	table_insert(&c_language.name_to_note_kind_table, str8_lit("typedef.union"),    Index_Note_C_Sum_Type);
	table_insert(&c_language.name_to_note_kind_table, str8_lit("enum.const"),       Index_Note_C_Enum_Const);
	
	c_language.note_kind_to_color_id = make_table_u64_u64(ts_index->arena.base_allocator, 32);
	table_insert(&c_language.note_kind_to_color_id, Index_Note_C_Function,     managed_id_get(app, SCu8("colors"), str8_lit("defcolor_function")));
	table_insert(&c_language.note_kind_to_color_id, Index_Note_C_Product_Type, managed_id_get(app, SCu8("colors"), str8_lit("ts_color_prod_type")));
	table_insert(&c_language.note_kind_to_color_id, Index_Note_C_Sum_Type,     managed_id_get(app, SCu8("colors"), str8_lit("ts_color_sum_type")));
	table_insert(&c_language.note_kind_to_color_id, Index_Note_C_Enum_Const,   managed_id_get(app, SCu8("colors"), str8_lit("ts_color_constant")));
	table_insert(&c_language.note_kind_to_color_id, Index_Note_C_Macro,        managed_id_get(app, SCu8("colors"), str8_lit("ts_color_macro")));
	
	{
		String_Const_u8 extensions[] = {
			str8_lit("c"),
		};
		for (u32 i = 0; i < ArrayCount(extensions); i += 1)
			table_insert(&ts_index->ext_to_language_table, extensions[i], make_data_struct(&c_language));
	}
	
	c_language.get_lister_note_kind_text = ts_get_lister_note_kind_text_c;
}
