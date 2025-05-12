
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


global String_Note_Kind_Pair c_sharp_name_to_kind_entries[] = {
	{.text = str8_lit("function_def"),      .note_kind = Index_Note_CSharp_Method},
	{.text = str8_lit("typedef.type"),      .note_kind = Index_Note_CSharp_Type},
	{.text = str8_lit("typedef.interface"), .note_kind = Index_Note_CSharp_Interface},
	{.text = str8_lit("typedef.class"),     .note_kind = Index_Note_CSharp_Class},
	{.text = str8_lit("typedef.enum"),      .note_kind = Index_Note_CSharp_Enum},
	{.text = str8_lit("typedef.struct"),    .note_kind = Index_Note_CSharp_Struct},
	{.text = str8_lit("typedef.record"),    .note_kind = Index_Note_CSharp_Record},
	{.text = str8_lit("namespace_def"),     .note_kind = Index_Note_CSharp_Namespace},
	{.text = str8_lit("constructor_def"),   .note_kind = Index_Note_CSharp_Constructor},
};

global String_Note_Kind_Pair c_sharp_note_kind_to_color_name_entries[] = {
	{.text = str8_lit("defcolor_function"),       .note_kind = Index_Note_CSharp_Method},
	{.text = str8_lit("ts_color_prod_type"),      .note_kind = Index_Note_CSharp_Type},
	{.text = str8_lit("ts_color_interface_type"), .note_kind = Index_Note_CSharp_Interface},
	{.text = str8_lit("ts_color_prod_type"),      .note_kind = Index_Note_CSharp_Class},
	{.text = str8_lit("ts_color_prod_type"),      .note_kind = Index_Note_CSharp_Enum},
	{.text = str8_lit("ts_color_prod_type"),      .note_kind = Index_Note_CSharp_Struct},
	{.text = str8_lit("ts_color_prod_type"),      .note_kind = Index_Note_CSharp_Record},
	{.text = str8_lit("ts_color_namespace"),      .note_kind = Index_Note_CSharp_Namespace},
	{.text = str8_lit("ts_color_constructor"),    .note_kind = Index_Note_CSharp_Constructor},
};

global String_Const_u8 c_sharp_extensions[] = {
	str8_lit("cs"),
};

global Language_Description c_sharp_language_description = {
	.name = str8_lit("c_sharp"),
	.name_to_kind_entries_count    = ArrayCount(c_sharp_name_to_kind_entries),
	.name_to_kind_entries          = c_sharp_name_to_kind_entries,
	.note_kind_to_color_name_count = ArrayCount(c_sharp_note_kind_to_color_name_entries),
	.note_kind_to_color_name       = c_sharp_note_kind_to_color_name_entries,
	.extensions_count              = ArrayCount(c_sharp_extensions),
	.extensions                    = c_sharp_extensions,
	.language = {
		.language                  = tree_sitter_c_sharp(),
	},
};