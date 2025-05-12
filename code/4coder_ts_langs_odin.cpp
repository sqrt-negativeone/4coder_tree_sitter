
enum
{
	Index_Note_Odin_None,
	Index_Note_Odin_Function,
	Index_Note_Odin_Struct,
	Index_Note_Odin_Enum,
	Index_Note_Odin_Enum_Const,
};

external const TSLanguage *tree_sitter_odin(void);


function String_Const_u8
ts_get_lister_note_kind_text_odin(TS_Index_Note *note, Arena *arena)
{
	String_Const_u8 result = str8_lit("");
	if (!note) return result;
	
	switch (note->kind)
	{
		case Index_Note_Odin_Function:
		{
			result = str8_lit("function");
		} break;
		case Index_Note_Odin_Struct:
		{
			result = str8_lit("type [struct]");
		} break;
		case Index_Note_Odin_Enum:
		{
			result = str8_lit("type [enum]");
		} break;
		case Index_Note_Odin_Enum_Const:
		{
			result = str8_lit("constant");
		} break;
	}
	
	return result;
}

global String_Note_Kind_Pair odin_name_to_kind_entries[] = {
	{.text = str8_lit("function_def"),   .note_kind = Index_Note_Odin_Function},
	{.text = str8_lit("typedef.struct"), .note_kind = Index_Note_Odin_Struct},
	{.text = str8_lit("typedef.enum"),   .note_kind = Index_Note_Odin_Enum},
	{.text = str8_lit("enum.const"),     .note_kind = Index_Note_Odin_Enum_Const},
};

global String_Note_Kind_Pair odin_note_kind_to_color_name_entries[] = {
	{.text = str8_lit("defcolor_function"),  .note_kind = Index_Note_Odin_Function},
	{.text = str8_lit("ts_color_prod_type"), .note_kind = Index_Note_Odin_Struct},
	{.text = str8_lit("ts_color_prod_type"), .note_kind = Index_Note_Odin_Enum},
	{.text = str8_lit("enum.const"),         .note_kind = Index_Note_Odin_Enum_Const},
};

global String_Const_u8 odin_extensions[] = {
	str8_lit("odin"),
};

global Language_Description odin_language_description = {
	.name = str8_lit("odin"),
	.name_to_kind_entries_count    = ArrayCount(odin_name_to_kind_entries),
	.name_to_kind_entries          = odin_name_to_kind_entries,
	.note_kind_to_color_name_count = ArrayCount(odin_note_kind_to_color_name_entries),
	.note_kind_to_color_name       = odin_note_kind_to_color_name_entries,
	.extensions_count              = ArrayCount(odin_extensions),
	.extensions                    = odin_extensions,
	.language = {
		.language                  = tree_sitter_odin(),
		.get_lister_note_kind_text = ts_get_lister_note_kind_text_odin,
	},
};