
#include "4coder_ts_langs_c.cpp"
#include "4coder_ts_langs_cpp.cpp"
#include "4coder_ts_langs_go.cpp"
#include "4coder_ts_langs_python.cpp"
#include "4coder_ts_langs_java.cpp"
#include "4coder_ts_langs_c_sharp.cpp"
#include "4coder_ts_langs_odin.cpp"

global Language_Description *g_languages[] = {
	&cpp_language_description,
	&c_language_description,
	&go_language_description,
	&python_language_description,
	&java_language_description,
	&c_sharp_language_description,
	&odin_language_description,
};


function void
ts_init_ts_language(Application_Links *app, String_Const_u8 name, TS_Language *language)
{
	Scratch_Block scratch(app);
	
	String_Const_u8 exe_path = string_remove_last_folder(system_get_path(scratch, SystemPath_Binary));
	
	
	String_Const_u8 ts_languages_path_root = push_stringf(scratch, "%.*slangs/%s", string_expand(exe_path), name);
	
	File_Name_Data highlight_query_data = dump_file(scratch, push_stringf(scratch, "%.*s/highlight.scm", string_expand(ts_languages_path_root)));
	if (highlight_query_data.data.size)
	{
		u32 err_offset = 0;
		TSQueryError err_type;
		language->highlight_query = ts_query_new(language->language, (char*)highlight_query_data.data.str, (u32)highlight_query_data.data.size, &err_offset, &err_type);
		if (!language->highlight_query)
		{
			print_message(app, str8_lit("[ERROR]: couldn't create highlight query.\n"));
			InvalidPath;
		}
	}
	
	
	
	File_Name_Data index_query_data = dump_file(scratch, push_stringf(scratch, "%.*s/index.scm", string_expand(ts_languages_path_root)));
	if (index_query_data.data.size)
	{
		u32 err_offset = 0;
		TSQueryError err_type;
		language->index_query = ts_query_new(language->language, (char*)index_query_data.data.str, (u32)index_query_data.data.size, &err_offset, &err_type);
		if (!language->index_query)
		{
			print_message(app, str8_lit("[ERROR]: couldn't create index query.\n"));
			InvalidPath;
		}
	}
	
	
	File_Name_Data scope_query_data = dump_file(scratch, push_stringf(scratch, "%.*s/scope.scm", string_expand(ts_languages_path_root)));
	if (scope_query_data.data.size)
	{
		u32 err_offset = 0;
		TSQueryError err_type;
		language->scope_query = ts_query_new(language->language, (char*)scope_query_data.data.str, (u32)scope_query_data.data.size, &err_offset, &err_type);
		if (!language->scope_query)
		{
			print_message(app, str8_lit("[ERROR]: couldn't create scope query.\n"));
			InvalidPath;
		}
	}
}

function void
ts_init_languages(Application_Links *app, TS_Index_Context *ts_index)
{
	for (u32 lang_idx = 0; lang_idx < ArrayCount(g_languages); lang_idx += 1) {
		Language_Description *lang_desc = g_languages[lang_idx];
		
		TS_Language *language = &lang_desc->language;
		
		ts_init_ts_language(app, lang_desc->name, language);
		
		language->name_to_note_kind_table = make_table_Data_u64(ts_index->arena.base_allocator, 32);
		for (u32 i = 0; i < lang_desc->name_to_kind_entries_count; i += 1)
		{
			String_Note_Kind_Pair entry = lang_desc->name_to_kind_entries[i];
			table_insert(&language->name_to_note_kind_table,  entry.text, entry.note_kind);
		}
		
		language->note_kind_to_color_id = make_table_u64_u64(ts_index->arena.base_allocator, 32);
		for (u32 i = 0; i < lang_desc->note_kind_to_color_name_count; i += 1)
		{
			String_Note_Kind_Pair entry = lang_desc->note_kind_to_color_name[i];
			table_insert(&language->note_kind_to_color_id, entry.note_kind,  managed_id_get(app, SCu8("colors"), entry.text));
		}
		
		for (u32 i = 0; i < lang_desc->extensions_count; i += 1)
			table_insert(&ts_index->ext_to_language_table, lang_desc->extensions[i], make_data_struct(language));
	}
}
