
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


typedef String_Const_u8 TS_Get_Lister_Note_Kind_Text_Proc(TS_Index_Note *note, Arena *arena);

struct TS_Language
{
	const TSLanguage *language;
	TSQuery *highlight_query;
	TSQuery *index_query;
	
	// TODO(fakhri): note_kind_to_color_id can just be an array, and then we index with note kind
	Table_u64_u64   note_kind_to_color_id;
	Table_Data_u64  name_to_note_kind_table;
	
	TS_Get_Lister_Note_Kind_Text_Proc *get_lister_note_kind_text;
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



function String_Const_u8
ts_push_word_under_pos(Application_Links *app, Arena *arena, Buffer_ID buffer, u64 pos){
	Managed_Scope scope = buffer_get_managed_scope(app, buffer);
	TS_Data *ts_data = scope_attachment(app, scope, ts_data_id, TS_Data);
	TSNode root = ts_tree_root_node(ts_data->tree);
	TSNode node = ts_node_descendant_for_byte_range(root, (i32)pos, (i32)pos);
	Range_i64 node_range = tree_sitter_get_range(node);
	String_Const_u8 result = push_buffer_range(app, arena, buffer, node_range);
	return(result);
}

function String_Const_u8
ts_push_word_under_active_cursor(Application_Links *app, Arena *arena, View_ID view, Buffer_ID buffer){
	i64 pos = view_get_cursor_pos(app, view);
	return(ts_push_word_under_pos(app, arena, buffer, pos));
}

function void
ts_goto_definition(Application_Links *app, TS_Index_Note *note, b32 same_panel)
{
	if (note != 0)
	{
		View_ID view = get_active_view(app, Access_Always);
		Rect_f32 region = view_get_buffer_region(app, view);
		f32 view_height = rect_height(region);
		Buffer_ID buffer = note->buffer_id;
		if(!same_panel)
		{
			view = get_next_view_looped_primary_panels(app, view, Access_Always);
		}
		point_stack_push_view_cursor(app, view);
		view_set_buffer(app, view, buffer, 0);
		i64 line_number = get_line_number_from_pos(app, buffer, note->range.min);
		Buffer_Scroll scroll = view_get_buffer_scroll(app, view);
		scroll.position.line_number = line_number;
		scroll.target.line_number = line_number;
		scroll.position.pixel_shift.y = scroll.target.pixel_shift.y = -view_height*0.5f;
		view_set_buffer_scroll(app, view, scroll, SetBufferScroll_SnapCursorIntoView);
		view_set_cursor(app, view, seek_pos(note->range.min));
		view_set_mark(app, view, seek_pos(note->range.min));
	}
}

CUSTOM_COMMAND_SIG(ts_goto_definition)
CUSTOM_DOC("Goes to the definition of the identifier under the cursor in the same panel.")
{
	View_ID view = get_active_view(app, Access_Always);
	Buffer_ID buffer = view_get_buffer(app, view, Access_Always);
	Scratch_Block scratch(app);
	String_Const_u8 string = ts_push_word_under_active_cursor(app, scratch, view, buffer);
	
	TS_Index_Note *note = ts_code_index_note_from_string(string);
	ts_goto_definition(app, note, 0);
}

CUSTOM_COMMAND_SIG(ts_goto_definition_same_panel)
CUSTOM_DOC("Goes to the definition of the identifier under the cursor in the same panel.")
{
	View_ID view = get_active_view(app, Access_Always);
	Buffer_ID buffer = view_get_buffer(app, view, Access_Always);
	Scratch_Block scratch(app);
	String_Const_u8 string = ts_push_word_under_active_cursor(app, scratch, view, buffer);
	
	TS_Index_Note *note = ts_code_index_note_from_string(string);
	ts_goto_definition(app, note, 1);
}

internal void
_ts_push_lister_option_for_note(Application_Links *app, Arena *arena, Lister *lister, TS_Language *language, TS_Index_Note *note)
{
	if(note)
	{
		Buffer_ID buffer = note->buffer_id;
		Managed_Scope scope = buffer_get_managed_scope(app, buffer);
		TS_Data *ts_data = scope_attachment(app, scope, ts_data_id, TS_Data);
		TS_Language *language = ts_data->language;
		
		Tiny_Jump *jump = push_array(arena, Tiny_Jump, 1);
		jump->buffer = buffer;
		jump->pos = note->range.first;
		
		String_Const_u8 buffer_name = push_buffer_unique_name(app, arena, buffer);
		String_Const_u8 name = push_stringf(arena, "[%.*s] %.*s", string_expand(buffer_name), string_expand(note->text));
		String_Const_u8 sort = str8_lit("");
		
		if (language->get_lister_note_kind_text)
		{
			sort = language->get_lister_note_kind_text(note, arena);
		}
		
		lister_add_item(lister, name, sort, jump, 0);
	}
}

internal void
ts_jump_to_location(Application_Links *app, View_ID view, Buffer_ID buffer, i64 pos)
{
	// NOTE(rjf): This function was ripped from 4coder's jump_to_location. It was copied
	// and modified so that jumping to a location didn't cause a selection in notepad-like
	// mode.
	
	view_set_active(app, view);
	Buffer_Seek seek = seek_pos(pos);
	set_view_to_location(app, view, buffer, seek);
	
	if (auto_center_after_jumps)
	{
		center_view(app);
	}
	view_set_cursor(app, view, seek);
	view_set_mark(app, view, seek);
}

function void
_ts_list_definitions_in_buffer(Application_Links *app, Buffer_ID buffer, Arena *arena, Lister *lister)
{
	Managed_Scope scope = buffer_get_managed_scope(app, buffer);
	TS_Data *ts_data = scope_attachment(app, scope, ts_data_id, TS_Data);
	
	TS_Index_File *file = ts_data->file;
	if (file != 0)
	{
		for(TS_Index_Note *note = file->notes.first; note; note = note->next)
		{
			_ts_push_lister_option_for_note(app, arena, lister, ts_data->language, note);
		}
	}
}

CUSTOM_UI_COMMAND_SIG(ts_search_for_definition__project_wide)
CUSTOM_DOC("List all definitions in the index and jump to the one selected by the user.")
{
	char *query = "Index (Project):";
	
	Scratch_Block scratch(app);
	Lister_Block lister(app, scratch);
	lister_set_query(lister, query);
	lister_set_default_handlers(lister);
	
	ts_code_index_lock();
	{
		for (Buffer_ID buffer = get_buffer_next(app, 0, Access_Always);
				 buffer != 0; buffer = get_buffer_next(app, buffer, Access_Always))
		{
			_ts_list_definitions_in_buffer(app, buffer, scratch, lister);
		}
	}
	ts_code_index_unlock();
	
	Lister_Result l_result = run_lister(app, lister);
	Tiny_Jump result = {};
	if (!l_result.canceled && l_result.user_data != 0){
		block_copy_struct(&result, (Tiny_Jump*)l_result.user_data);
	}
	
	if (result.buffer != 0)
	{
		View_ID view = get_this_ctx_view(app, Access_Always);
		point_stack_push_view_cursor(app, view);
		ts_jump_to_location(app, view, result.buffer, result.pos);
	}
}

CUSTOM_UI_COMMAND_SIG(ts_search_for_definition__current_file)
CUSTOM_DOC("List all definitions in the current file and jump to the one selected by the user.")
{
	char *query = "Index (File):";
	
	View_ID view = get_active_view(app, Access_Always);
	Buffer_ID buffer = view_get_buffer(app, view, Access_Always);
	
	Scratch_Block scratch(app);
	Lister_Block lister(app, scratch);
	lister_set_query(lister, query);
	lister_set_default_handlers(lister);
	
	ts_code_index_lock();
	_ts_list_definitions_in_buffer(app, buffer, scratch, lister);
	ts_code_index_unlock();
	
	Lister_Result l_result = run_lister(app, lister);
	Tiny_Jump result = {};
	if (!l_result.canceled && l_result.user_data != 0){
		block_copy_struct(&result, (Tiny_Jump*)l_result.user_data);
	}
	
	if (result.buffer != 0)
	{
		View_ID view_id = get_this_ctx_view(app, Access_Always);
		point_stack_push_view_cursor(app, view_id);
		ts_jump_to_location(app, view_id, result.buffer, result.pos);
	}
}

