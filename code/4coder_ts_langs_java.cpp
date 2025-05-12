
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


global String_Note_Kind_Pair java_name_to_kind_entries[] = {
	{.text = str8_lit("package_def"),       .note_kind = Index_Note_Java_Package},
	{.text = str8_lit("typedef.interface"), .note_kind = Index_Note_Java_Interface},
	{.text = str8_lit("typedef.class"),     .note_kind = Index_Note_Java_Class},
	{.text = str8_lit("typedef.enum"),      .note_kind = Index_Note_Java_Enum},
	{.text = str8_lit("function_def"),      .note_kind = Index_Note_Java_Method},
};

global String_Note_Kind_Pair java_note_kind_to_color_name_entries[] = {
	{.text = str8_lit("ts_color_namespace"),      .note_kind = Index_Note_Java_Package},
	{.text = str8_lit("ts_color_interface_type"), .note_kind = Index_Note_Java_Interface},
	{.text = str8_lit("ts_color_prod_type"),      .note_kind = Index_Note_Java_Class},
	{.text = str8_lit("ts_color_prod_type"),      .note_kind = Index_Note_Java_Enum},
	{.text = str8_lit("defcolor_function"),       .note_kind = Index_Note_Java_Method},
};

global String_Const_u8 java_extensions[] = {
	str8_lit("java"),
};

global Language_Description java_language_description = {
	.name = str8_lit("java"),
	.name_to_kind_entries_count    = ArrayCount(java_name_to_kind_entries),
	.name_to_kind_entries          = java_name_to_kind_entries,
	.note_kind_to_color_name_count = ArrayCount(java_note_kind_to_color_name_entries),
	.note_kind_to_color_name       = java_note_kind_to_color_name_entries,
	.extensions_count              = ArrayCount(java_extensions),
	.extensions                    = java_extensions,
	.language = {
		.language                  = tree_sitter_java(),
	},
};