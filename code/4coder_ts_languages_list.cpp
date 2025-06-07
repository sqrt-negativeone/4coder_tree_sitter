
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

function TSQuery *
ts_create_query(Application_Links *app, const TSLanguage *language, String_Const_u8 ts_languages_path_root, String_Const_u8 query_file_name)
{
	TSQuery *result = 0;
	Scratch_Block scratch(app);
	File_Name_Data query_data = dump_file(scratch, push_stringf(scratch, "%.*s/%.*s", string_expand(ts_languages_path_root), string_expand(query_file_name)));
	if (query_data.data.size)
	{
		u32 err_offset = 0;
		TSQueryError err_type;
		result = ts_query_new(language, (char*)query_data.data.str, (u32)query_data.data.size, &err_offset, &err_type);
		if (!result)
		{
			print_message(app, str8_lit("[ERROR]: couldn't create query.\n"));
			InvalidPath;
		}
	}
	
	return result;
}

function void
ts_init_ts_language(Application_Links *app, String_Const_u8 name, TS_Language *language)
{
	Scratch_Block scratch(app);
	
	String_Const_u8 exe_path = string_remove_last_folder(system_get_path(scratch, SystemPath_Binary));
	
	String_Const_u8 ts_languages_path_root = push_stringf(scratch, "%.*slangs/%s", string_expand(exe_path), name);
	
	language->highlight_query = ts_create_query(app, language->language, ts_languages_path_root, str8_lit("highlight.scm"));
	language->index_query     = ts_create_query(app, language->language, ts_languages_path_root, str8_lit("index.scm"));
	language->scope_query     = ts_create_query(app, language->language, ts_languages_path_root, str8_lit("scope.scm"));
}


function void
ts_init_index_map(Application_Links *app, TS_Index_Map *map, Base_Allocator *base_allocator,
									u32 name_to_kind_entries_count, String_Note_Kind_Pair *name_to_kind_entries,
									u32 note_kind_to_color_name_count, String_Note_Kind_Pair *note_kind_to_color_name)
{
	map->name_to_note_kind_table = make_table_Data_u64(base_allocator, 32);
	for (u32 i = 0; i < name_to_kind_entries_count; i += 1)
	{
		String_Note_Kind_Pair entry = name_to_kind_entries[i];
		if (!entry.text.size) continue;
		table_insert(&map->name_to_note_kind_table,  entry.text, entry.note_kind);
	}
	
	map->note_kind_to_color_id_table = make_table_u64_u64(base_allocator, 32);
	for (u32 i = 0; i < note_kind_to_color_name_count; i += 1)
	{
		String_Note_Kind_Pair entry = note_kind_to_color_name[i];
		if (!entry.text.size) continue;
		table_insert(&map->note_kind_to_color_id_table, entry.note_kind,  managed_id_get(app, SCu8("colors"), entry.text));
	}
}

function void
ts_init_languages(Application_Links *app, TS_Index_Context *ts_index)
{
	ts_init_index_map(app, &ts_index->global_index_map, ts_index->arena.base_allocator,
										ArrayCount(global_name_to_kind_entries), global_name_to_kind_entries,
										ArrayCount(global_note_kind_to_color_name), global_note_kind_to_color_name);
	
	for (u32 lang_idx = 0; lang_idx < ArrayCount(g_languages); lang_idx += 1) {
		Language_Description *lang_desc = g_languages[lang_idx];
		
		TS_Language *language = &lang_desc->language;
		
		ts_init_ts_language(app, lang_desc->name, language);
		
		ts_init_index_map(app, &language->index_map, ts_index->arena.base_allocator,
											lang_desc->name_to_kind_entries_count, lang_desc->name_to_kind_entries,
											lang_desc->note_kind_to_color_name_count, lang_desc->note_kind_to_color_name);
		
		for (u32 i = 0; i < lang_desc->extensions_count; i += 1)
			table_insert(&ts_index->ext_to_language_table, lang_desc->extensions[i], make_data_struct(language));
	}
}
