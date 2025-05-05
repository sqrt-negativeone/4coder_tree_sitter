
typedef u64 TS_Index_Note_Kind;

struct TS_Index_Note
{
	TS_Index_Note *next;
	TS_Index_Note *next_hash;
	TS_Index_Note *prev_hash;
	
	u64 hash;
	TS_Index_Note_Kind kind;
	Buffer_ID buffer_id;
	String_Const_u8 text;
	Range_i64 range;
};

struct TS_Index_Note_List
{
	TS_Index_Note *first;
	TS_Index_Note *last;
	u32 count;
};

struct TS_Index_File
{
	Arena arena;
	TS_Index_Note_List notes;
};

struct TS_Language
{
	const TSLanguage *language;
	TSQuery *highlight_query;
	TSQuery *index_query;
	
	// TODO(fakhri): note_kind_to_color_id can just be an array, and then we index with note kind
	Table_u64_u64   note_kind_to_color_id;
	Table_Data_u64  name_to_note_kind_table;
};

struct TS_Data
{
	TSParser *parser;
	TSTree   *tree;
	TS_Index_File *file;
	TS_Language *language;
};



struct TS_Index_Context
{
	Arena arena;
	System_Mutex mutex;
	TS_Index_Note *free_notes;
	TS_Index_Note_List name_to_note_table[4096];
	
	Table_Data_Data ext_to_language_table;
};

global TS_Index_Context g_index_ctx;

#include "4coder_ts_langs_c.cpp"
#include "4coder_ts_langs_cpp.cpp"
#include "4coder_ts_langs_odin.cpp"
#include "4coder_ts_langs_python.cpp"
#include "4coder_ts_langs_java.cpp"
#include "4coder_ts_langs_c_sharp.cpp"
#include "4coder_ts_langs_go.cpp"

function void
ts_code_index_lock(void){
	system_mutex_acquire(g_index_ctx.mutex);
}

function void
ts_code_index_unlock(void){
	system_mutex_release(g_index_ctx.mutex);
}

function b32
ts_begin_buffer(Application_Links *app, Buffer_ID buffer_id, Managed_Scope scope)
{
	b32 treat_as_code = false;
	Scratch_Block scratch(app);
	TS_Index_Context *ts_index = &g_index_ctx;
	String_Const_u8 file_name = push_buffer_file_name(app, scratch, buffer_id);
	String_Const_u8 ext = string_file_extension(file_name);
	
	String_Const_u8 lang_out;
	if (table_read(&ts_index->ext_to_language_table, ext, &lang_out))
	{
		treat_as_code = true;
		Assert(lang_out.size == sizeof(TS_Language));
		TS_Language *ts_language = (TS_Language*)lang_out.str;
		
		TS_Data *ts_data = scope_attachment(app, scope, ts_data_id, TS_Data);
		ts_data->parser = ts_parser_new();
		ts_parser_set_language(ts_data->parser, ts_language->language);
		ts_data->language = ts_language;
		
		String_Const_u8 content = push_whole_buffer(app, scratch, buffer_id);
		ts_data->tree = ts_parser_parse_string(ts_data->parser, 0, (char*)content.str, (u32)content.size);
		buffer_mark_as_modified(buffer_id);
		
	}
	
	return treat_as_code;
}


function TS_Index_Note *
ts_make_index_note(TS_Index_File *file, Buffer_ID buffer_id,
									 TS_Index_Note_Kind note_kind, Range_i64 note_range, String_Const_u8 text)
{
	TS_Index_Note *note = push_array(&file->arena, TS_Index_Note, 1);
	sll_queue_push(file->notes.first, file->notes.last, note);
	file->notes.count += 1;
	
	note->kind = note_kind;
	note->range = note_range;
	note->buffer_id = buffer_id;
	note->text = push_string_copy(&file->arena, text);
	note->hash = table_hash_u8(text.str, text.size);
	return note;
}


function void
ts_code_index_init(Application_Links *app)
{
	TS_Index_Context *ts_index = &g_index_ctx;
	block_zero_struct(ts_index);
	ts_index->arena = make_arena_system(KB(512));
	ts_index->mutex = system_mutex_make();
	
	
	ts_index->ext_to_language_table = make_table_Data_Data(ts_index->arena.base_allocator, 32);
	
	ts_init_c_language(app, ts_index);
	ts_init_cpp_language(app, ts_index);
	ts_init_python_language(app, ts_index);
	ts_init_java_language(app, ts_index);
	ts_init_csharp_language(app, ts_index);
	ts_init_go_language(app, ts_index);
	// ts_init_odin_language(app, ts_index);
}

function TS_Index_Note *
ts_code_index_note_from_string(String_Const_u8 string)
{
	TS_Index_Note *result = 0;
	TS_Index_Context *ts_index = &g_index_ctx;
	
	u64 hash = table_hash_u8(string.str, string.size);
	u64 slot_index = hash % ArrayCount(ts_index->name_to_note_table);
	for (TS_Index_Note *note = ts_index->name_to_note_table[slot_index].first;
			 note; note = note->next_hash)
	{
		if (note->hash == hash && string_match(note->text, string))
		{
			result = note;
			break;
		}
	}
	return result;
}

function TS_Index_Note_Kind 
ts_node_kind_from_string(TS_Language *lang, String_Const_u8 capture_name)
{
	TS_Index_Note_Kind note_kind = 0;
	table_read(&lang->name_to_note_kind_table, capture_name, &note_kind);
	return note_kind;
}

function Managed_ID
ts_code_index_color_from_note(Application_Links *app, TS_Language *lang, TS_Index_Note *note)
{
	Managed_ID color_id = 0;
	table_read(&lang->note_kind_to_color_id, note->kind, &color_id);
	return color_id;
}

function void
ts_clear_index_file(TS_Data *ts_data)
{
	TS_Index_File *old_index_file = ts_data->file;
	if (old_index_file)
	{
		// NOTE(fakhri): delete the old index file
		for (TS_Index_Note *note = old_index_file->notes.first;
				 note; note = note->next)
		{
			u64 slot_index = note->hash % ArrayCount(g_index_ctx.name_to_note_table);
			
			TS_Index_Note_List *list = g_index_ctx.name_to_note_table + slot_index;
			zdll_remove_NP_(list->first, list->last, note, next_hash, prev_hash);
			list->count -= 1;
		}
		
		Arena arena = old_index_file->arena;
		linalloc_clear(&arena);
		ts_data->file = 0;
	}
}

function void
ts_update_index_file(TS_Data *ts_data, TS_Index_File *new_index_file)
{
	ts_clear_index_file(ts_data);
	
	if (new_index_file)
	{
		ts_data->file = new_index_file;
		
		for (TS_Index_Note *note = new_index_file->notes.first;
				 note; note = note->next)
		{
			u64 slot_index = note->hash % ArrayCount(g_index_ctx.name_to_note_table);
			
			TS_Index_Note_List *list = g_index_ctx.name_to_note_table + slot_index;
			zdll_push_back_NP_(list->first, list->last, note, next_hash, prev_hash);
			list->count += 1;
		}
	}
}

function void
ts_code_index_update_tick(Application_Links *app)
{
	Scratch_Block scratch(app);
	for (Buffer_Modified_Node *node = global_buffer_modified_set.first;
			 node != 0;
			 node = node->next)
	{
		Temp_Memory_Block temp(scratch);
		Buffer_ID buffer_id = node->buffer;
		
		Managed_Scope scope = buffer_get_managed_scope(app, buffer_id);
		TS_Data *ts_data = scope_attachment(app, scope, ts_data_id, TS_Data);
		if (!ts_data->tree) continue;
		TS_Language *language = ts_data->language;
		
		String_Const_u8 contents = push_whole_buffer(app, scratch, buffer_id);
		Arena arena = make_arena_system(KB(16));
		
		TS_Index_File *file = push_array_zero(&arena, TS_Index_File, 1);
		file->arena = arena;
		
		TSQueryCursor *index_cursor = ts_query_cursor_new();
		if (index_cursor)
		{
			ProfileBlock(app, "TreeSitter Indexing");
			TSNode root = ts_tree_root_node(ts_data->tree);
			
#if 0
			ts_query_cursor_set_max_start_depth(index_cursor, 8);
#endif
			ts_query_cursor_exec(index_cursor, language->index_query, root);
			
			TSQueryMatch match = {};
			u32 capture_index = 0;
			while (ts_query_cursor_next_capture(index_cursor, &match, &capture_index))
			{
				const TSQueryCapture *capture = match.captures + capture_index;
				
				Range_i64 node_range = tree_sitter_get_range(capture->node);
				u32 capture_name_len = 0;
				const char *capture_name_str = ts_query_capture_name_for_id(language->index_query, capture->index, &capture_name_len);
				String_Const_u8 capture_name = SCu8((u8*)capture_name_str, capture_name_len);
				
				ts_make_index_note(file, buffer_id, 
													 ts_node_kind_from_string(language, capture_name),
													 node_range, string_substring(contents, node_range));
			}
			
			ts_query_cursor_delete(index_cursor);
		}
		
		ts_code_index_lock();
		ts_update_index_file(ts_data, file);
		ts_code_index_unlock();
		buffer_clear_layout_cache(app, buffer_id);
	}
	
	buffer_modified_set_clear();
}
