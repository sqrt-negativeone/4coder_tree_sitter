
enum
{
	Index_Note_Python_None,
	Index_Note_Python_Function,
	Index_Note_Python_Type,
};

external const TSLanguage *tree_sitter_python(void);


global String_Const_u8 python_scope_query_str =
str8_lit
(
 R"scm(
)scm"
 );

global String_Const_u8 python_index_query_str =
str8_lit
(
 R"scm(
(class_definition (identifier) @typedef.class)
(function_definition name: (identifier) @function_def)
)scm"
 );

global String_Const_u8 python_highlight_query_str = 
str8_lit
(
 R"scm(


((call
  function: (identifier) @ts_color_builtin_func)
 (#match?
   @ts_color_builtin_func
   "^(abs|all|any|ascii|bin|bool|breakpoint|bytearray|bytes|callable|chr|classmethod|compile|complex|delattr|dict|dir|divmod|enumerate|eval|exec|filter|float|format|frozenset|getattr|globals|hasattr|hash|help|hex|id|input|int|isinstance|issubclass|iter|len|list|locals|map|max|memoryview|min|next|object|oct|open|ord|pow|print|property|range|repr|reversed|round|set|setattr|slice|sorted|staticmethod|str|sum|super|tuple|type|vars|zip|__import__)$"))


(attribute attribute: (identifier) @ts_color_attribute)

(decorator (identifier) @ts_color_decorator)

[
(none)
  (true)
  (false)
  (integer)
  (float)
] @defcolor_int_constant

(comment) @defcolor_comment
(string) @defcolor_str_constant
(escape_sequence) @defcolor_str_constant


[
  "-"
  "-="
  "!="
  "*"
  "**"
  "**="
  "*="
  "/"
  "//"
  "//="
  "/="
  "&"
  "&="
  "%"
  "%="
  "^"
  "^="
  "+"
  "->"
  "+="
  "<"
  "<<"
  "<<="
  "<="
  "<>"
  "="
  ":="
  ":"
  "=="
  ">"
  ">="
  ">>"
  ">>="
  "|"
  "|="
  "~"
  "@="
  "and"
  "in"
  "is"
  "not"
  "or"
  "is not"
  "not in"
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
  "as"
  "assert"
  "async"
  "await"
  "break"
  "class"
  "continue"
  "def"
  "del"
  "elif"
  "else"
  "except"
  "exec"
  "finally"
  "for"
  "from"
  "global"
  "if"
  "import"
  "lambda"
  "nonlocal"
  "pass"
  "print"
  "raise"
  "return"
  "try"
  "while"
  "with"
  "yield"
  "match"
  "case"
] @defcolor_keyword
)scm"
 );

global TS_Language python_language;



function void
ts_init_python_language(Application_Links *app, TS_Index_Context *ts_index)
{
	python_language.language = tree_sitter_python();
	
	u32 err_offset = 0;
	TSQueryError err_type;
	python_language.highlight_query = ts_query_new(python_language.language, (char*)python_highlight_query_str.str, (u32)python_highlight_query_str.size, &err_offset, &err_type);
	Assert(python_language.highlight_query);
	if (!python_language.highlight_query)
	{
		print_message(app, str8_lit("couldn't create highlight query"));
	}
	
	python_language.index_query = ts_query_new(python_language.language, (char*)python_index_query_str.str, (u32)python_index_query_str.size, &err_offset, &err_type);
	Assert(python_language.index_query);
	if (!python_language.index_query)
	{
		print_message(app, str8_lit("couldn't create highlight query"));
	}
	
	python_language.scope_query = ts_query_new(python_language.language, (char*)python_scope_query_str.str, (u32)python_scope_query_str.size, &err_offset, &err_type);
	Assert(python_language.scope_query);
	if (!python_language.scope_query)
	{
		print_message(app, str8_lit("couldn't create highlight query"));
	}
	
	python_language.name_to_note_kind_table = make_table_Data_u64(ts_index->arena.base_allocator, 32);
	table_insert(&python_language.name_to_note_kind_table, str8_lit("function_def"),     Index_Note_Python_Function);
	table_insert(&python_language.name_to_note_kind_table, str8_lit("typedef.class"),    Index_Note_Python_Type);
	
	python_language.note_kind_to_color_id = make_table_u64_u64(ts_index->arena.base_allocator, 32);
	table_insert(&python_language.note_kind_to_color_id, Index_Note_Python_Function,     managed_id_get(app, SCu8("colors"), str8_lit("defcolor_function")));
	table_insert(&python_language.note_kind_to_color_id, Index_Note_Python_Type, managed_id_get(app, SCu8("colors"), str8_lit("ts_color_prod_type")));
	
	table_insert(&ts_index->ext_to_language_table, str8_lit("py"), make_data_struct(&python_language));
}
