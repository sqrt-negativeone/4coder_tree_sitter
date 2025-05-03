
enum
{
	Index_Note_GO_None,
	Index_Note_GO_Function,
	Index_Note_GO_Type,
	Index_Note_GO_Interface,
	Index_Note_GO_Struct,
};

external const TSLanguage *tree_sitter_go(void);

// TODO(fakhri): load queries from disk

global String_Const_u8 go_index_query_str =
str8_lit
(
 R"scm(

(function_declaration name: (identifier) @function_def)
(method_declaration name: (field_identifier) @function_def)

(type_spec name: (type_identifier) @typedef.type)
(type_declaration (type_spec name: (type_identifier) @tyepdef.interface type: (interface_type)))
(type_declaration (type_spec name: (type_identifier) @typedef.struct type: (struct_type)))

)scm"
 );

global String_Const_u8 go_highlight_query_str = 
str8_lit
(
 R"scm(
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

)scm"
 );

global TS_Language go_language;

function void
ts_init_go_language(Application_Links *app, TS_Index_Context *ts_index)
{
	go_language.language = tree_sitter_go();
	
	u32 err_offset = 0;
	TSQueryError err_type;
	go_language.highlight_query = ts_query_new(go_language.language, (char*)go_highlight_query_str.str, (u32)go_highlight_query_str.size, &err_offset, &err_type);
	Assert(go_language.highlight_query);
	if (!go_language.highlight_query)
	{
		print_message(app, str8_lit("couldn't create highlight query"));
	}
	
	go_language.index_query = ts_query_new(go_language.language, (char*)go_index_query_str.str, (u32)go_index_query_str.size, &err_offset, &err_type);
	Assert(go_language.index_query);
	if (!go_language.index_query)
	{
		print_message(app, str8_lit("couldn't create highlight query"));
	}
	
	go_language.name_to_note_kind_table = make_table_Data_u64(ts_index->arena.base_allocator, 32);
	table_insert(&go_language.name_to_note_kind_table, str8_lit("function_def"),     Index_Note_GO_Function);
	table_insert(&go_language.name_to_note_kind_table, str8_lit("typedef.type"),     Index_Note_GO_Type);
	table_insert(&go_language.name_to_note_kind_table, str8_lit("typedef.interface"),Index_Note_GO_Interface);
	table_insert(&go_language.name_to_note_kind_table, str8_lit("typedef.struct"),   Index_Note_GO_Struct);
	
	go_language.note_kind_to_color_id = make_table_u64_u64(ts_index->arena.base_allocator, 32);
	table_insert(&go_language.note_kind_to_color_id, Index_Note_GO_Function,     managed_id_get(app, SCu8("colors"), str8_lit("defcolor_function")));
	table_insert(&go_language.note_kind_to_color_id, Index_Note_GO_Type,         managed_id_get(app, SCu8("colors"), str8_lit("ts_color_prod_type")));
	table_insert(&go_language.note_kind_to_color_id, Index_Note_GO_Interface,    managed_id_get(app, SCu8("colors"), str8_lit("ts_color_interface_type")));
	table_insert(&go_language.note_kind_to_color_id, Index_Note_GO_Struct,       managed_id_get(app, SCu8("colors"), str8_lit("ts_color_prod_type")));
	
	
	table_insert(&ts_index->ext_to_language_table, str8_lit("go"), make_data_struct(&go_language));
}
