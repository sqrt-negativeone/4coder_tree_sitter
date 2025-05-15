
enum
{
	Index_Note_C_None = Index_Note_COUNT,
	Index_Note_C_Macro,
};

external const TSLanguage *tree_sitter_c(void);

function String_Const_u8
ts_get_lister_note_kind_text_c(TS_Index_Note *note, Arena *arena)
{
	String_Const_u8 result = str8_lit("");
	if (!note) return result;
	
	switch (note->kind)
	{
		case Index_Note_C_Macro:
		{
			result = str8_lit("macro");
		} break;
		case Index_Note_Function:
		{
			result = str8_lit("function");
		} break;
		case Index_Note_Product_Type:
		{
			result = str8_lit("type [product]");
		} break;
		case Index_Note_Sum_Type:
		{
			result = str8_lit("type [sum]");
		} break;
		case Index_Note_Constant:
		{
			result = str8_lit("constant");
		} break;
	}
	
	return result;
}

global String_Note_Kind_Pair c_name_to_kind_entries[] = {
	{.text = str8_lit("macro_def"),        .note_kind = Index_Note_C_Macro},
};

global String_Note_Kind_Pair c_note_kind_to_color_name_entries[] = {
	{.text = str8_lit("ts_color_macro"),     .note_kind = Index_Note_C_Macro},
};

global String_Const_u8 c_extensions[] = {
	str8_lit("c"),
};

global Language_Description c_language_description = {
	.name = str8_lit("c"),
	.name_to_kind_entries_count    = ArrayCount(c_name_to_kind_entries),
	.name_to_kind_entries          = c_name_to_kind_entries,
	.note_kind_to_color_name_count = ArrayCount(c_note_kind_to_color_name_entries),
	.note_kind_to_color_name       = c_note_kind_to_color_name_entries,
	.extensions_count              = ArrayCount(c_extensions),
	.extensions                    = c_extensions,
	.language = {
		.language                  = tree_sitter_c(),
		.get_lister_note_kind_text = ts_get_lister_note_kind_text_c,
	},
};