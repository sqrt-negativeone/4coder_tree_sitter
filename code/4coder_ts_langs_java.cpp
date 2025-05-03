
enum
{
	Index_Note_Java_None,
	Index_Note_Java_Method,
	Index_Note_Java_Enum,
	Index_Note_Java_Class,
	Index_Note_Java_Interface,
	Index_Note_Java_Package,
};

external const TSLanguage *tree_sitter_java(void);

// TODO(fakhri): load queries from disk

global String_Const_u8 java_index_query_str =
str8_lit
(
 R"scm(

(method_declaration name: (identifier) @function_def)

(interface_declaration name: (identifier) @typedef.interface)
(class_declaration name: (identifier) @typedef.class)
(enum_declaration name: (identifier) @typedef.enum)

(package_declaration (scoped_identifier (identifier) @package_def))
(package_declaration (identifier) @package_def)

)scm"
 );

global String_Const_u8 java_highlight_query_str = 
str8_lit
(
 R"scm(

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
)scm"
 );

global TS_Language java_language;

function void
ts_init_java_language(Application_Links *app, TS_Index_Context *ts_index)
{
	java_language.language = tree_sitter_java();
	
	u32 err_offset = 0;
	TSQueryError err_type;
	java_language.highlight_query = ts_query_new(java_language.language, (char*)java_highlight_query_str.str, (u32)java_highlight_query_str.size, &err_offset, &err_type);
	Assert(java_language.highlight_query);
	if (!java_language.highlight_query)
	{
		print_message(app, str8_lit("couldn't create highlight query"));
	}
	
	java_language.index_query = ts_query_new(java_language.language, (char*)java_index_query_str.str, (u32)java_index_query_str.size, &err_offset, &err_type);
	Assert(java_language.index_query);
	if (!java_language.index_query)
	{
		print_message(app, str8_lit("couldn't create highlight query"));
	}
	
	java_language.name_to_note_kind_table = make_table_Data_u64(ts_index->arena.base_allocator, 32);
	table_insert(&java_language.name_to_note_kind_table, str8_lit("package_def"),       Index_Note_Java_Package);
	table_insert(&java_language.name_to_note_kind_table, str8_lit("typedef.interface"), Index_Note_Java_Interface);
	table_insert(&java_language.name_to_note_kind_table, str8_lit("typedef.class"), Index_Note_Java_Class);
	table_insert(&java_language.name_to_note_kind_table, str8_lit("typedef.enum"), Index_Note_Java_Enum);
	table_insert(&java_language.name_to_note_kind_table, str8_lit("function_def"), Index_Note_Java_Method);
	
	java_language.note_kind_to_color_id = make_table_u64_u64(ts_index->arena.base_allocator, 32);
	table_insert(&java_language.note_kind_to_color_id, Index_Note_Java_Method,    managed_id_get(app, SCu8("colors"), str8_lit("defcolor_function")));
	table_insert(&java_language.note_kind_to_color_id, Index_Note_Java_Enum,      managed_id_get(app, SCu8("colors"), str8_lit("ts_color_prod_type")));
	table_insert(&java_language.note_kind_to_color_id, Index_Note_Java_Class,     managed_id_get(app, SCu8("colors"), str8_lit("ts_color_prod_type")));
	table_insert(&java_language.note_kind_to_color_id, Index_Note_Java_Interface, managed_id_get(app, SCu8("colors"), str8_lit("ts_color_interface_type")));
	table_insert(&java_language.note_kind_to_color_id, Index_Note_Java_Package,   managed_id_get(app, SCu8("colors"), str8_lit("ts_color_namespace")));
	
	String_Const_u8 extensions[] = {
		str8_lit("java"),
	};
	
	for (u32 i = 0; i < ArrayCount(extensions); i += 1)
		table_insert(&ts_index->ext_to_language_table, extensions[i], make_data_struct(&java_language));
	
}
