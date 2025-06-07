
enum
{
	Index_Note_CSharp_None = Index_Note_COUNT,
	Index_Note_CSharp_Interface,
	Index_Note_CSharp_Namespace,
	Index_Note_CSharp_Constructor,
};

external const TSLanguage *tree_sitter_c_sharp(void);


global String_Note_Kind_Pair c_sharp_name_to_kind_entries[] = {
	{/*.text =*/ str8_lit("typedef.interface"), /*.note_kind =*/ Index_Note_CSharp_Interface},
	{/*.text =*/ str8_lit("namespace_def"),     /*.note_kind =*/ Index_Note_CSharp_Namespace},
	{/*.text =*/ str8_lit("constructor_def"),   /*.note_kind =*/ Index_Note_CSharp_Constructor},
};

global String_Note_Kind_Pair c_sharp_note_kind_to_color_name_entries[] = {
	{/*.text =*/ str8_lit("ts_color_interface_type"), /*.note_kind =*/ Index_Note_CSharp_Interface},
	{/*.text =*/ str8_lit("ts_color_namespace"),      /*.note_kind =*/ Index_Note_CSharp_Namespace},
	{/*.text =*/ str8_lit("ts_color_constructor"),    /*.note_kind =*/ Index_Note_CSharp_Constructor},
};

global String_Const_u8 c_sharp_extensions[] = {
	str8_lit("cs"),
};

global Language_Description c_sharp_language_description = {
	/*.name                          =*/ str8_lit("c_sharp"),
	/*.name_to_kind_entries_count    =*/ ArrayCount(c_sharp_name_to_kind_entries),
	/*.name_to_kind_entries          =*/ c_sharp_name_to_kind_entries,
	/*.note_kind_to_color_name_count =*/ ArrayCount(c_sharp_note_kind_to_color_name_entries),
	/*.note_kind_to_color_name       =*/ c_sharp_note_kind_to_color_name_entries,
	/*.extensions_count              =*/ ArrayCount(c_sharp_extensions),
	/*.extensions                    =*/ c_sharp_extensions,
	/*.language =*/ {
		/*.language                  =*/ tree_sitter_c_sharp(),
	},
};