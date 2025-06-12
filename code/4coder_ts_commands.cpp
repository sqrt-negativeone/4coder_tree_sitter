
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
	
	Managed_Scope scope = buffer_get_managed_scope(app, buffer);
	TS_Data *ts_data = scope_attachment(app, scope, ts_data_id, TS_Data);
	if (ts_data->language)
	{
		TS_Index_Note *note = ts_code_index_note_from_string(ts_data->language, string);
		ts_goto_definition(app, note, 0);
	}
}

CUSTOM_COMMAND_SIG(ts_goto_definition_same_panel)
CUSTOM_DOC("Goes to the definition of the identifier under the cursor in the same panel.")
{
	View_ID view = get_active_view(app, Access_Always);
	Buffer_ID buffer = view_get_buffer(app, view, Access_Always);
	Scratch_Block scratch(app);
	String_Const_u8 string = ts_push_word_under_active_cursor(app, scratch, view, buffer);
	
	Managed_Scope scope = buffer_get_managed_scope(app, buffer);
	TS_Data *ts_data = scope_attachment(app, scope, ts_data_id, TS_Data);
	if (ts_data->language)
	{
		TS_Index_Note *note = ts_code_index_note_from_string(ts_data->language, string);
		ts_goto_definition(app, note, 1);
	}
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

CUSTOM_COMMAND_SIG(ts_search_for_definition__project_wide)
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

CUSTOM_COMMAND_SIG(ts_search_for_definition__current_file)
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


function void
ts_write_tree_to_buffer(Application_Links *app, Arena *arena, Buffer_ID buffer_id,
												TSNode cur_node, i32 indent = 0, const char *node_name = "")
{
	char prefix_buffer[1024] = { 0 };
	for (int i = 0; i < indent; i+=1)
	{
		prefix_buffer[i] = '\t';
	}
	TSPoint start = ts_node_start_point(cur_node);
	TSPoint end = ts_node_end_point(cur_node);
	
	String_Const_u8 string = push_stringf(arena, "%s%s%s %s [%d, %d] - [%d, %d]\n",
																				prefix_buffer, node_name, (node_name[0]? ":": ""), ts_node_type(cur_node),
																				start.row + 1, start.column + 1,
																				end.row + 1, end.column + 1);
	
	buffer_replace_range(app, buffer_id, Ii64(buffer_get_size(app, buffer_id)), string);
	
	u32 child_count = ts_node_child_count(cur_node);
	for (u32 i = 0; i < child_count; ++i)
	{
		TSNode child = ts_node_child(cur_node, i);
		if (ts_node_is_named(child))
		{
			node_name = ts_node_field_name_for_child(cur_node, i);
			if (!node_name) {
				node_name = "";
			}
			ts_write_tree_to_buffer(app, arena, buffer_id, child, indent + 1, node_name);
		}
	}
}


CUSTOM_COMMAND_SIG(ts_write_buffer_tree)
CUSTOM_DOC("output tree sitter AST to *ts_tree* buffer")
{
	Scratch_Block scratch(app);
	Buffer_ID out_buffer = get_buffer_by_name(app, string_u8_litexpr("*ts_tree*"), Access_Always);
	buffer_replace_range(app, out_buffer, buffer_range(app, out_buffer), str8_lit(""));
	
	View_ID view = get_active_view(app, Access_Always);
	Buffer_ID buffer = view_get_buffer(app, view, Access_Always);
	Managed_Scope scope = buffer_get_managed_scope(app, buffer);
	TS_Data *ts_data = scope_attachment(app, scope, ts_data_id, TS_Data);
	
	if (ts_data->tree)
	{
		TSNode node = ts_tree_root_node(ts_data->tree);
		ts_write_tree_to_buffer(app, scratch, out_buffer, node);
	}
}

CUSTOM_COMMAND_SIG(ts_invalidate_surrounding_node)
CUSTOM_DOC("Invalidate the active surrounding TS node")
{
	View_ID view = get_active_view(app, Access_Always);
	Buffer_ID buffer = view_get_buffer(app, view, Access_Always);
	Managed_Scope scope = buffer_get_managed_scope(app, buffer);
	TS_Data *ts_data = scope_attachment(app, scope, ts_data_id, TS_Data);
	if (ts_data)
	{
		block_zero_struct(&ts_data->current_surrounding_node);
	}
}

internal void
ts_select_node(Application_Links *app, View_ID view, TSNode node)
{
	Range_i64 node_range = tree_sitter_get_range(node);
	view_set_cursor_and_preferred_x(app, view, seek_pos(node_range.start));
	view_set_mark(app, view, seek_pos(node_range.end));
}

CUSTOM_COMMAND_SIG(ts_select_parent_syntax_node)
CUSTOM_DOC("Selects one parent element in TS AST")
{
	Scratch_Block scratch(app);
	
	View_ID view = get_active_view(app, Access_Always);
	Buffer_ID buffer = view_get_buffer(app, view, Access_Always);
	Managed_Scope scope = buffer_get_managed_scope(app, buffer);
	TS_Data *ts_data = scope_attachment(app, scope, ts_data_id, TS_Data);
	if (ts_data)
	{
		i64 pos = view_get_cursor_pos(app, view);
		
		if (ts_node_is_null(ts_data->current_surrounding_node) || 
				(!range_contains(tree_sitter_get_range(ts_data->current_surrounding_node), pos)))
		{
			TSNode root = ts_tree_root_node(ts_data->tree);
			ts_data->current_surrounding_node = ts_node_descendant_for_byte_range(root, (i32)pos, (i32)pos);
		}
		else
		{
			TSNode parent = ts_node_parent(ts_data->current_surrounding_node);
			if (!ts_node_is_null(parent))
			{
				ts_data->current_surrounding_node = parent;
			}
		}
		ts_select_node(app, view, ts_data->current_surrounding_node);
	}
}


CUSTOM_COMMAND_SIG(ts_select_first_child_syntax_node)
CUSTOM_DOC("Selects first child element in TS AST")
{
	Scratch_Block scratch(app);
	
	View_ID view = get_active_view(app, Access_Always);
	Buffer_ID buffer = view_get_buffer(app, view, Access_Always);
	Managed_Scope scope = buffer_get_managed_scope(app, buffer);
	TS_Data *ts_data = scope_attachment(app, scope, ts_data_id, TS_Data);
	if (ts_data)
	{
		i64 pos = view_get_cursor_pos(app, view);
		
		if (ts_node_is_null(ts_data->current_surrounding_node) || 
				(!range_contains(tree_sitter_get_range(ts_data->current_surrounding_node), pos)))
		{
			TSNode root = ts_tree_root_node(ts_data->tree);
			ts_data->current_surrounding_node = ts_node_descendant_for_byte_range(root, (i32)pos, (i32)pos);
		}
		else
		{
			TSNode child = ts_node_child(ts_data->current_surrounding_node, 0);
			if (!ts_node_is_null(child))
			{
				ts_data->current_surrounding_node = child;
			}
		}
		ts_select_node(app, view, ts_data->current_surrounding_node);
	}
}


CUSTOM_COMMAND_SIG(ts_select_next_syntax_node)
CUSTOM_DOC("Selects one next sibling in TS AST")
{
	Scratch_Block scratch(app);
	
	View_ID view = get_active_view(app, Access_Always);
	Buffer_ID buffer = view_get_buffer(app, view, Access_Always);
	Managed_Scope scope = buffer_get_managed_scope(app, buffer);
	TS_Data *ts_data = scope_attachment(app, scope, ts_data_id, TS_Data);
	if (ts_data)
	{
		i64 pos = view_get_cursor_pos(app, view);
		
		if (ts_node_is_null(ts_data->current_surrounding_node) || 
				(!range_contains(tree_sitter_get_range(ts_data->current_surrounding_node), pos)))
		{
			TSNode root = ts_tree_root_node(ts_data->tree);
			ts_data->current_surrounding_node = ts_node_descendant_for_byte_range(root, (i32)pos, (i32)pos);
		}
		else
		{
			TSNode sib = ts_node_next_sibling(ts_data->current_surrounding_node);
			if (!ts_node_is_null(sib))
			{
				ts_data->current_surrounding_node = sib;
			}
		}
		ts_select_node(app, view, ts_data->current_surrounding_node);
	}
}

CUSTOM_COMMAND_SIG(ts_select_prev_syntax_node)
CUSTOM_DOC("Selects prev sibling in TS AST")
{
	Scratch_Block scratch(app);
	
	View_ID view = get_active_view(app, Access_Always);
	Buffer_ID buffer = view_get_buffer(app, view, Access_Always);
	Managed_Scope scope = buffer_get_managed_scope(app, buffer);
	TS_Data *ts_data = scope_attachment(app, scope, ts_data_id, TS_Data);
	if (ts_data)
	{
		i64 pos = view_get_cursor_pos(app, view);
		
		if (ts_node_is_null(ts_data->current_surrounding_node) || 
				(!range_contains(tree_sitter_get_range(ts_data->current_surrounding_node), pos)))
		{
			TSNode root = ts_tree_root_node(ts_data->tree);
			ts_data->current_surrounding_node = ts_node_descendant_for_byte_range(root, (i32)pos, (i32)pos);
		}
		else
		{
			TSNode sib = ts_node_prev_sibling(ts_data->current_surrounding_node);
			if (!ts_node_is_null(sib))
			{
				ts_data->current_surrounding_node = sib;
			}
		}
		
		ts_select_node(app, view, ts_data->current_surrounding_node);
	}
}


internal b32
ts_is_node_function_type(String_Const_u8 type)
{
	local_persist String_Const_u8 function_types[] = {
		str8_lit("function_definition"),
		str8_lit("method_declaration"),
	};
	
	b32 result = false;
	for (u32 i = 0; i < ArrayCount(function_types); i += 1)
	{
		if (string_match(function_types[i], type))
		{
			result = true;
			break;
		}
	}
	return result;
}

internal TSNode
ts_get_first_surrounding_function(TSTree *tree, i64 pos)
{
	TSNode result = {};
	
	TSNode root = ts_tree_root_node(tree);
	TSNode surrounding_node = ts_node_descendant_for_byte_range(root, (i32)pos, (i32)pos);
	b32 valid = false;
	for (;;)
	{
		String_Const_u8 node_type = SCu8((u8*)ts_node_type(surrounding_node));
		if (ts_is_node_function_type(node_type))
		{
			valid = true;
			break;
		}
		
		TSNode node = ts_node_parent(surrounding_node);
		if (ts_node_is_null(node))
		{
			break;
		}
		surrounding_node = node;
	}
	
	if (valid)
		result = surrounding_node;
	return result;
}

CUSTOM_COMMAND_SIG(ts_select_surrounding_function)
CUSTOM_DOC("Selects surrounding function")
{
	Scratch_Block scratch(app);
	
	View_ID view = get_active_view(app, Access_Always);
	Buffer_ID buffer = view_get_buffer(app, view, Access_Always);
	Managed_Scope scope = buffer_get_managed_scope(app, buffer);
	TS_Data *ts_data = scope_attachment(app, scope, ts_data_id, TS_Data);
	if (ts_data)
	{
		i64 pos = view_get_cursor_pos(app, view);
		TSNode surrounding_node = ts_get_first_surrounding_function(ts_data->tree, pos);
		if (!ts_node_is_null(surrounding_node))
		{
			ts_data->current_surrounding_node = surrounding_node;
			ts_select_node(app, view, surrounding_node);
		}
	}
}

internal b32
ts_advance_next_node(TSTreeCursor *tree_cursor)
{
	b32 result = false;
	
	if (ts_tree_cursor_goto_next_sibling(tree_cursor))
	{
		TSNode node = ts_tree_cursor_current_node(tree_cursor);
		ts_tree_cursor_goto_first_child_for_byte(tree_cursor, ts_node_start_byte(node));
		result = true;
	}
	else if (ts_tree_cursor_goto_parent(tree_cursor))
	{
		result = true;
	}
	return result;
}

internal b32
ts_advance_prev_node(TSTreeCursor *tree_cursor)
{
	b32 result = false;
	
	if (ts_tree_cursor_goto_previous_sibling(tree_cursor))
	{
		TSNode node = ts_tree_cursor_current_node(tree_cursor);
		ts_tree_cursor_goto_first_child_for_byte(tree_cursor, ts_node_end_byte(node) - 1);
		result = true;
	}
	else if (ts_tree_cursor_goto_parent(tree_cursor))
	{
		result = true;
	}
	return result;
}


CUSTOM_COMMAND_SIG(ts_select_next_function)
CUSTOM_DOC("Selects next function")
{
	Scratch_Block scratch(app);
	
	View_ID view = get_active_view(app, Access_Always);
	Buffer_ID buffer = view_get_buffer(app, view, Access_Always);
	Managed_Scope scope = buffer_get_managed_scope(app, buffer);
	TS_Data *ts_data = scope_attachment(app, scope, ts_data_id, TS_Data);
	if (ts_data)
	{
		TSNode next_function_node = {}; 
		
		TSNode root = ts_tree_root_node(ts_data->tree);
		i64 pos = view_get_cursor_pos(app, view);
		
		TSTreeCursor tree_cursor = ts_tree_cursor_new(root);
		for (;ts_tree_cursor_goto_first_child_for_byte(&tree_cursor, (u32)pos) >= 0;);
		
		for (;ts_advance_next_node(&tree_cursor);)
		{
			TSNode node = ts_tree_cursor_current_node(&tree_cursor);
			
			String_Const_u8 node_type = SCu8((u8*)ts_node_type(node));
			if (ts_is_node_function_type(node_type) && (!ts_node_eq(node, ts_data->current_surrounding_node)))
			{
				next_function_node = node;
				break;
			}
		}
		ts_tree_cursor_delete(&tree_cursor);
		
		if (!ts_node_is_null(next_function_node))
		{
			ts_data->current_surrounding_node = next_function_node;
			ts_select_node(app, view, next_function_node);
		}
	}
}

CUSTOM_COMMAND_SIG(ts_select_prev_function)
CUSTOM_DOC("Selects previous function")
{
	Scratch_Block scratch(app);
	
	View_ID view = get_active_view(app, Access_Always);
	Buffer_ID buffer = view_get_buffer(app, view, Access_Always);
	Managed_Scope scope = buffer_get_managed_scope(app, buffer);
	TS_Data *ts_data = scope_attachment(app, scope, ts_data_id, TS_Data);
	if (ts_data)
	{
		TSNode next_function_node = {}; 
		
		TSNode root = ts_tree_root_node(ts_data->tree);
		i64 pos = view_get_cursor_pos(app, view);
		
		TSTreeCursor tree_cursor = ts_tree_cursor_new(root);
		for (;ts_tree_cursor_goto_first_child_for_byte(&tree_cursor, (u32)pos) >= 0;);
		
		for (;ts_advance_prev_node(&tree_cursor);)
		{
			TSNode node = ts_tree_cursor_current_node(&tree_cursor);
			
			String_Const_u8 node_type = SCu8((u8*)ts_node_type(node));
			if (ts_is_node_function_type(node_type) && (!ts_node_eq(node, ts_data->current_surrounding_node)))
			{
				next_function_node = node;
				break;
			}
		}
		ts_tree_cursor_delete(&tree_cursor);
		
		if (!ts_node_is_null(next_function_node))
		{
			ts_data->current_surrounding_node = next_function_node;
			ts_select_node(app, view, next_function_node);
		}
	}
}


