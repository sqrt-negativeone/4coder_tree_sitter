
enum
{
	Index_Note_GO_None,
	Index_Note_GO_Function,
	Index_Note_GO_Type,
	Index_Note_GO_Interface,
	Index_Note_GO_Struct,
};

external const TSLanguage *tree_sitter_go(void);

global String_Note_Kind_Pair go_name_to_kind_entries[] = {
	{.text = str8_lit("function_def"),       .note_kind = Index_Note_GO_Function},
	{.text = str8_lit("typedef.type"),       .note_kind = Index_Note_GO_Type},
	{.text = str8_lit("typedef.interface"),  .note_kind = Index_Note_GO_Interface},
	{.text = str8_lit("typedef.struct"),     .note_kind = Index_Note_GO_Struct},
};

global String_Note_Kind_Pair go_note_kind_to_color_name_entries[] = {
	{.text = str8_lit("defcolor_function"),        .note_kind = Index_Note_GO_Function},
	{.text = str8_lit("ts_color_prod_type"),       .note_kind = Index_Note_GO_Type},
	{.text = str8_lit("ts_color_interface_type"),  .note_kind = Index_Note_GO_Interface},
	{.text = str8_lit("ts_color_prod_type"),       .note_kind = Index_Note_GO_Struct},
};

global String_Const_u8 go_extensions[] = {
	str8_lit("go"),
};

global Language_Description go_language_description = {
	.name = str8_lit("go"),
	.name_to_kind_entries_count    = ArrayCount(go_name_to_kind_entries),
	.name_to_kind_entries          = go_name_to_kind_entries,
	.note_kind_to_color_name_count = ArrayCount(go_note_kind_to_color_name_entries),
	.note_kind_to_color_name       = go_note_kind_to_color_name_entries,
	.extensions_count              = ArrayCount(go_extensions),
	.extensions                    = go_extensions,
	.language = {
		.language                  = tree_sitter_go(),
	},
};