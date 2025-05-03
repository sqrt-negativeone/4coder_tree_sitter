
enum
{
	Index_Note_Odin_None,
	Index_Note_Odin_Function,
	Index_Note_Odin_Product_Type,
	Index_Note_Odin_Sum_Type,
	Index_Note_Odin_Enum_Const,
};

external const TSLanguage *tree_sitter_odin(void);
global TS_Language odin_language;


// TODO(fakhri): add odin index query
global String_Const_u8 odin_index_query_str =
str8_lit
(
 R"scm(

)scm"
 );

global String_Const_u8 odin_highlight_query_str = 
str8_lit
(
 R"scm(

[
"auto_cast"
"bit_set"
 "break"
 "case"
"cast" 
"continue" 
"defer" 
"distinct" 
"do" 
"dynamic" 
"else" 
"enum"
 "for"
 "foreign"
 "if"
 "import"
 "in"
 "map"
 "not_in"
 "or_else"
 "or_return"
 "package"
 "proc"
 "return"
 "struct"
 "switch"
 "transmute"
 "union"
 "using"
 "when"
 "where"
] @defcolor_keyword

[
".."
"..="
"..<"
"::"
 ] @ts_color_operator

(int_literal)   @defcolor_int_constant
(float_literal) @defcolor_int_constant
(rune_literal)  @defcolor_char_constant
(bool_literal) @boolean
(nil) @defcolor_int_constant

 (ERROR) @ts_color_error_annotation

(package_identifier) @ts_color_namespace
(label_identifier)   @ts_color_label

(interpreted_string_literal) @defcolor_str_constant
(raw_string_literal) @defcolor_str_constant
(escape_sequence) @defcolor_str_constant

(comment) @defcolor_comment
; (const_identifier) @constant


(compiler_directive) @ts_color_attribute
(calling_convention) @ts_color_attribute

; (identifier) @variable
(pragma_identifier) @ts_color_attribute
)scm"
 );


function void
ts_init_odin_language(Application_Links *app, TS_Index_Context *ts_index)
{
	odin_language.language = tree_sitter_odin();
	
	u32 err_offset = 0;
	TSQueryError err_type;
	odin_language.highlight_query = ts_query_new(odin_language.language, (char*)odin_highlight_query_str.str, (u32)odin_highlight_query_str.size, &err_offset, &err_type);
	Assert(odin_language.highlight_query);
	if (!odin_language.highlight_query)
	{
		print_message(app, str8_lit("couldn't create highlight query"));
	}
	
	odin_language.index_query = ts_query_new(odin_language.language, (char*)odin_index_query_str.str, (u32)odin_index_query_str.size, &err_offset, &err_type);
	Assert(odin_language.index_query);
	if (!odin_language.index_query)
	{
		print_message(app, str8_lit("couldn't create highlight query"));
	}
	
	odin_language.name_to_note_kind_table = make_table_Data_u64(ts_index->arena.base_allocator, 32);
	table_insert(&odin_language.name_to_note_kind_table, str8_lit("function_def"),     Index_Note_Odin_Function);
	table_insert(&odin_language.name_to_note_kind_table, str8_lit("typedef.type"),     Index_Note_Odin_Product_Type);
	table_insert(&odin_language.name_to_note_kind_table, str8_lit("typedef.struct"),   Index_Note_Odin_Product_Type);
	table_insert(&odin_language.name_to_note_kind_table, str8_lit("typedef.enum"),     Index_Note_Odin_Product_Type);
	table_insert(&odin_language.name_to_note_kind_table, str8_lit("typedef.union"),    Index_Note_Odin_Sum_Type);
	table_insert(&odin_language.name_to_note_kind_table, str8_lit("enum.const"),       Index_Note_Odin_Enum_Const);
	
	odin_language.note_kind_to_color_id = make_table_u64_u64(ts_index->arena.base_allocator, 32);
	table_insert(&odin_language.note_kind_to_color_id, Index_Note_Odin_Function,     managed_id_get(app, SCu8("colors"), str8_lit("defcolor_function")));
	table_insert(&odin_language.note_kind_to_color_id, Index_Note_Odin_Product_Type, managed_id_get(app, SCu8("colors"), str8_lit("ts_color_prod_type")));
	table_insert(&odin_language.note_kind_to_color_id, Index_Note_Odin_Sum_Type,     managed_id_get(app, SCu8("colors"), str8_lit("ts_color_sum_type")));
	table_insert(&odin_language.note_kind_to_color_id, Index_Note_Odin_Enum_Const,   managed_id_get(app, SCu8("colors"), str8_lit("ts_color_constant")));
	
	table_insert(&ts_index->ext_to_language_table, str8_lit("odin"), make_data_struct(&odin_language));
}
