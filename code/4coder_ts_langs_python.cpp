
enum
{
	Index_Note_Python_None,
	Index_Note_Python_Function,
	Index_Note_Python_Type,
};

external const TSLanguage *tree_sitter_python(void);

global String_Note_Kind_Pair python_name_to_kind_entries[] = {
	{.text = str8_lit("function_def"),       .note_kind = Index_Note_Python_Function},
	{.text = str8_lit("typedef.type"),       .note_kind = Index_Note_Python_Type},
};

global String_Note_Kind_Pair python_note_kind_to_color_name_entries[] = {
	{.text = str8_lit("defcolor_function"),        .note_kind = Index_Note_Python_Function},
	{.text = str8_lit("ts_color_prod_type"),       .note_kind = Index_Note_Python_Type},
};

global String_Const_u8 python_extensions[] = {
	str8_lit("py"),
};

global Language_Description python_language_description = {
	.name = str8_lit("python"),
	.name_to_kind_entries_count    = ArrayCount(python_name_to_kind_entries),
	.name_to_kind_entries          = python_name_to_kind_entries,
	.note_kind_to_color_name_count = ArrayCount(python_note_kind_to_color_name_entries),
	.note_kind_to_color_name       = python_note_kind_to_color_name_entries,
	.extensions_count              = ArrayCount(python_extensions),
	.extensions                    = python_extensions,
	.language = {
		.language                  = tree_sitter_python(),
	},
};