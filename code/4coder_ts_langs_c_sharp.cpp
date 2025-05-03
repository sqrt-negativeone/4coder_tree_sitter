
enum
{
	Index_Note_CSharp_None,
	Index_Note_CSharp_Method,
	Index_Note_CSharp_Type,
	Index_Note_CSharp_Interface,
	Index_Note_CSharp_Class,
	Index_Note_CSharp_Enum,
	Index_Note_CSharp_Struct,
	Index_Note_CSharp_Record,
	Index_Note_CSharp_Namespace,
	Index_Note_CSharp_Constructor,
	Index_Note_CSharp_Enum_Const,
};

external const TSLanguage *tree_sitter_c_sharp(void);

// TODO(fakhri): load queries from disk

global String_Const_u8 csharp_index_query_str =
str8_lit
(
 R"scm(

(method_declaration name: (identifier) @function_def)
(local_function_statement name: (identifier) @function_def)

(interface_declaration name: (identifier) @typedef.interface)
(class_declaration name: (identifier) @typedef.class)
(enum_declaration name: (identifier) @typedef.enum)
(struct_declaration (identifier) @typedef.struct)
(record_declaration (identifier) @typedef.record)

(namespace_declaration name: (identifier) @namespace_def)

(generic_name (identifier) @typedef.type)
(type_parameter (identifier) @typedef.type)
(parameter type: (identifier) @tyepdef.type)
(type_argument_list (identifier) @typedef.type)
(as_expression right: (identifier) @typedef.type)
(is_expression right: (identifier) @typedef.type)

(constructor_declaration name: (identifier) @constructor_def)
(destructor_declaration name: (identifier) @constructor_def)

(base_list (identifier) @typedef.type)
(_ type: (identifier) @typedef.type)

(enum_member_declaration (identifier) @enum.const)
)scm"
 );

global String_Const_u8 csharp_highlight_query_str = 
str8_lit
(
 R"scm(

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

)scm"
 );

global TS_Language csharp_language;

function void
ts_init_csharp_language(Application_Links *app, TS_Index_Context *ts_index)
{
	csharp_language.language = tree_sitter_c_sharp();
	
	u32 err_offset = 0;
	TSQueryError err_type;
	csharp_language.highlight_query = ts_query_new(csharp_language.language, (char*)csharp_highlight_query_str.str, (u32)csharp_highlight_query_str.size, &err_offset, &err_type);
	Assert(csharp_language.highlight_query);
	if (!csharp_language.highlight_query)
	{
		print_message(app, str8_lit("couldn't create highlight query"));
	}
	
	csharp_language.index_query = ts_query_new(csharp_language.language, (char*)csharp_index_query_str.str, (u32)csharp_index_query_str.size, &err_offset, &err_type);
	Assert(csharp_language.index_query);
	if (!csharp_language.index_query)
	{
		print_message(app, str8_lit("couldn't create highlight query"));
	}
	
	csharp_language.name_to_note_kind_table = make_table_Data_u64(ts_index->arena.base_allocator, 32);
	table_insert(&csharp_language.name_to_note_kind_table, str8_lit("function_def"),      Index_Note_CSharp_Method);
	table_insert(&csharp_language.name_to_note_kind_table, str8_lit("typedef.type"),      Index_Note_CSharp_Type);
	table_insert(&csharp_language.name_to_note_kind_table, str8_lit("typedef.interface"), Index_Note_CSharp_Interface);
	table_insert(&csharp_language.name_to_note_kind_table, str8_lit("typedef.class"),     Index_Note_CSharp_Class);
	table_insert(&csharp_language.name_to_note_kind_table, str8_lit("typedef.enum"),      Index_Note_CSharp_Enum);
	table_insert(&csharp_language.name_to_note_kind_table, str8_lit("typedef.struct"),    Index_Note_CSharp_Struct);
	table_insert(&csharp_language.name_to_note_kind_table, str8_lit("typedef.record"),    Index_Note_CSharp_Record);
	table_insert(&csharp_language.name_to_note_kind_table, str8_lit("namespace_def"),     Index_Note_CSharp_Namespace);
	table_insert(&csharp_language.name_to_note_kind_table, str8_lit("constructor_def"),   Index_Note_CSharp_Constructor);
	
	csharp_language.note_kind_to_color_id = make_table_u64_u64(ts_index->arena.base_allocator, 32);
	table_insert(&csharp_language.note_kind_to_color_id, Index_Note_CSharp_Method,      managed_id_get(app, SCu8("colors"), str8_lit("defcolor_function")));
	table_insert(&csharp_language.note_kind_to_color_id, Index_Note_CSharp_Type,        managed_id_get(app, SCu8("colors"), str8_lit("ts_color_prod_type")));
	table_insert(&csharp_language.note_kind_to_color_id, Index_Note_CSharp_Interface,   managed_id_get(app, SCu8("colors"), str8_lit("ts_color_interface_type")));
	table_insert(&csharp_language.note_kind_to_color_id, Index_Note_CSharp_Class,       managed_id_get(app, SCu8("colors"), str8_lit("ts_color_prod_type")));
	table_insert(&csharp_language.note_kind_to_color_id, Index_Note_CSharp_Enum,        managed_id_get(app, SCu8("colors"), str8_lit("ts_color_prod_type")));
	table_insert(&csharp_language.note_kind_to_color_id, Index_Note_CSharp_Struct,      managed_id_get(app, SCu8("colors"), str8_lit("ts_color_prod_type")));
	table_insert(&csharp_language.note_kind_to_color_id, Index_Note_CSharp_Record,      managed_id_get(app, SCu8("colors"), str8_lit("ts_color_prod_type")));
	table_insert(&csharp_language.note_kind_to_color_id, Index_Note_CSharp_Namespace,   managed_id_get(app, SCu8("colors"), str8_lit("ts_color_namespace")));
	table_insert(&csharp_language.note_kind_to_color_id, Index_Note_CSharp_Constructor, managed_id_get(app, SCu8("colors"), str8_lit("ts_color_constructor")));
	
	String_Const_u8 extensions[] = {
		str8_lit("cs"),
	};
	
	for (u32 i = 0; i < ArrayCount(extensions); i += 1)
		table_insert(&ts_index->ext_to_language_table, extensions[i], make_data_struct(&csharp_language));
	
}
