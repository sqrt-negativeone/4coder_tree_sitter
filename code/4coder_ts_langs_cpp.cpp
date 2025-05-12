
enum
{
	Index_Note_CPP_None,
	Index_Note_CPP_Macro,
	Index_Note_CPP_Namespace,
	Index_Note_CPP_Function,
	Index_Note_CPP_Product_Type,
	Index_Note_CPP_Sum_Type,
	Index_Note_CPP_Enum_Const,
};



external const TSLanguage *tree_sitter_cpp(void);

function String_Const_u8
ts_get_lister_note_kind_text_cpp(TS_Index_Note *note, Arena *arena)
{
	String_Const_u8 result = str8_lit("");
	if (!note) return result;
	
	switch (note->kind)
	{
		case Index_Note_CPP_Macro:
		{
			result = str8_lit("macro");
		} break;
		case Index_Note_CPP_Function:
		{
			result = str8_lit("function");
		} break;
		case Index_Note_CPP_Product_Type:
		{
			result = str8_lit("type [product]");
		} break;
		case Index_Note_CPP_Sum_Type:
		{
			result = str8_lit("type [sum]");
		} break;
		case Index_Note_CPP_Enum_Const:
		{
			result = str8_lit("constant");
		} break;
		
		case Index_Note_CPP_Namespace:
		{
			result = str8_lit("namespace");
		} break;
		
	}
	
	return result;
}

global String_Note_Kind_Pair cpp_name_to_kind_entries[] = {
	{.text = str8_lit("namespace_def"),    .note_kind = Index_Note_CPP_Namespace},
	{.text = str8_lit("macro_def"),        .note_kind = Index_Note_CPP_Macro},
	{.text = str8_lit("function_def"),     .note_kind = Index_Note_CPP_Function},
	{.text = str8_lit("typedef.type"),     .note_kind = Index_Note_CPP_Product_Type},
	{.text = str8_lit("typedef.class"),    .note_kind = Index_Note_CPP_Product_Type},
	{.text = str8_lit("typedef.function"), .note_kind = Index_Note_CPP_Product_Type},
	{.text = str8_lit("typedef.struct"),   .note_kind = Index_Note_CPP_Product_Type},
	{.text = str8_lit("typedef.enum"),     .note_kind = Index_Note_CPP_Product_Type},
	{.text = str8_lit("typedef.union"),    .note_kind = Index_Note_CPP_Sum_Type},
	{.text = str8_lit("enum.const"),       .note_kind = Index_Note_CPP_Enum_Const},
};

global String_Note_Kind_Pair cpp_note_kind_to_color_name_entries[] = {
	{.text = str8_lit("defcolor_function"),  .note_kind = Index_Note_CPP_Function},
	{.text = str8_lit("ts_color_prod_type"), .note_kind = Index_Note_CPP_Product_Type},
	{.text = str8_lit("ts_color_sum_type"),  .note_kind = Index_Note_CPP_Sum_Type},
	{.text = str8_lit("ts_color_constant"),  .note_kind = Index_Note_CPP_Enum_Const},
	{.text = str8_lit("ts_color_macro"),     .note_kind = Index_Note_CPP_Macro},
	{.text = str8_lit("ts_color_namespace"), .note_kind = Index_Note_CPP_Namespace},
};

global String_Const_u8 cpp_extensions[] = {
	str8_lit("cpp"),
	str8_lit("cc"),
	str8_lit("h"),
	str8_lit("hh"),
};

global Language_Description cpp_language_description = {
	.name = str8_lit("cpp"),
	.name_to_kind_entries_count    = ArrayCount(cpp_name_to_kind_entries),
	.name_to_kind_entries          = cpp_name_to_kind_entries,
	.note_kind_to_color_name_count = ArrayCount(cpp_note_kind_to_color_name_entries),
	.note_kind_to_color_name       = cpp_note_kind_to_color_name_entries,
	.extensions_count              = ArrayCount(cpp_extensions),
	.extensions                    = cpp_extensions,
	.language = {
		.language                  = tree_sitter_cpp(),
		.get_lister_note_kind_text = ts_get_lister_note_kind_text_cpp,
	},
};
