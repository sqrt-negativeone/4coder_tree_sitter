
enum
{
	Index_Note_Odin_None = Index_Note_COUNT,
};

external const TSLanguage *tree_sitter_odin(void);


function String_Const_u8
ts_get_lister_note_kind_text_odin(TS_Index_Note *note, Arena *arena)
{
	String_Const_u8 result = str8_lit("");
	if (!note) return result;
	
	switch (note->kind)
	{
		case Index_Note_Function:
		{
			result = str8_lit("function");
		} break;
		case Index_Note_Product_Type:
		{
			result = str8_lit("type [struct]");
		} break;
		case Index_Note_Constant:
		{
			result = str8_lit("constant");
		} break;
	}
	
	return result;
}

global String_Note_Kind_Pair odin_name_to_kind_entries[] = {
	{}
};

global String_Note_Kind_Pair odin_note_kind_to_color_name_entries[] = {
	{}
};

global String_Const_u8 odin_extensions[] = {
	str8_lit("odin"),
};

global Language_Description odin_language_description = {
	/*.name                          =*/ str8_lit("odin"),
	/*.name_to_kind_entries_count    =*/ ArrayCount(odin_name_to_kind_entries),
	/*.name_to_kind_entries          =*/ odin_name_to_kind_entries,
	/*.note_kind_to_color_name_count =*/ ArrayCount(odin_note_kind_to_color_name_entries),
	/*.note_kind_to_color_name       =*/ odin_note_kind_to_color_name_entries,
	/*.extensions_count              =*/ ArrayCount(odin_extensions),
	/*.extensions                    =*/ odin_extensions,
	/*.language =*/ {
		/*.language                  =*/ tree_sitter_odin(),
		/*.get_lister_note_kind_text =*/ ts_get_lister_note_kind_text_odin,
	},
};