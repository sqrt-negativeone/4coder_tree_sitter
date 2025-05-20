
function void
ts_auto_indent_buffer(Application_Links *app, Buffer_ID buffer, Range_i64 range)
{
	Scratch_Block scratch(app);
	Managed_Scope scope = buffer_get_managed_scope(app, buffer);
	TS_Data *ts_data = scope_attachment(app, scope, ts_data_id, TS_Data);
	TS_Index_File *file = ts_data->file;
	if (!file) return;
	
	Range_i64 line_numbers = get_line_range_from_pos_range(app, buffer, range);
	i64 count = line_numbers.max - line_numbers.min + 1;
	i64 *indentations = push_array(scratch, i64, count);
	block_fill_u64(indentations, sizeof(*indentations)*count, (u64)(-1));
	
	for (u32 line = 0; line < count; line += 1)
	{
		i64 offset = get_line_start_pos(app, buffer, line + line_numbers.start);
		TS_Layout_Scope *nest_scope = ts_get_layout_scope_for_offset(file->scopes_root, offset);
		indentations[line] = nest_scope->nest_level;
	}
	
	i32 tab_width = (i32)def_get_config_u64(app, vars_save_string_lit("default_tab_width"));
	tab_width = clamp_bot(1, tab_width);
	u32 flags = 0;
	b32 indent_with_tabs = def_get_config_b32(vars_save_string_lit("indent_with_tabs"));
	if (indent_with_tabs){
		AddFlag(flags, Indent_UseTab);
	}
	set_line_indents(app, scratch, buffer, line_numbers, indentations, flags, tab_width);
}


CUSTOM_COMMAND_SIG(ts_auto_indent_whole_file)
CUSTOM_DOC("Audo-indents the entire current buffer with TreeSitter.")
{
	View_ID view = get_active_view(app, Access_ReadWriteVisible);
	Buffer_ID buffer = view_get_buffer(app, view, Access_ReadWriteVisible);
	i64 buffer_size = buffer_get_size(app, buffer);
	ts_auto_indent_buffer(app, buffer, Ii64(0, buffer_size));
}

CUSTOM_COMMAND_SIG(ts_auto_indent_range)
CUSTOM_DOC("Auto-indents the range between the cursor and the mark with TreeSitter.")
{
	View_ID view = get_active_view(app, Access_ReadWriteVisible);
	Buffer_ID buffer = view_get_buffer(app, view, Access_ReadWriteVisible);
	Range_i64 range = get_view_range(app, view);
	ts_auto_indent_buffer(app, buffer, range);
	move_past_lead_whitespace(app, view, buffer);
}


CUSTOM_COMMAND_SIG(ts_auto_indent_line_at_cursor)
CUSTOM_DOC("Auto-indents the line on which the cursor sits with TreeSitter.")
{
	View_ID view = get_active_view(app, Access_ReadWriteVisible);
	Buffer_ID buffer = view_get_buffer(app, view, Access_ReadWriteVisible);
	i64 pos = view_get_cursor_pos(app, view);
	ts_auto_indent_buffer(app, buffer, Ii64(pos));
	move_past_lead_whitespace(app, view, buffer);
}


CUSTOM_COMMAND_SIG(ts_write_text_and_auto_indent)
CUSTOM_DOC("Inserts text and auto-indents the line on which the cursor sits if any of the text contains 'layout punctuation' such as ;:{}()[]# and new lines.")
{
	ProfileScope(app, "write and auto indent");
	User_Input in = get_current_input(app);
	String_Const_u8 insert = to_writable(&in);
	if (insert.str != 0 && insert.size > 0){
		b32 do_auto_indent = false;
		for (u64 i = 0; !do_auto_indent && i < insert.size; i += 1){
			switch (insert.str[i]){
				case ';': case ':':
				case '{': case '}':
				case '(': case ')':
				case '[': case ']':
				case '#':
				case '\n': case '\t':
				{
					do_auto_indent = true;
				}break;
			}
		}
		if (do_auto_indent){
			View_ID view = get_active_view(app, Access_ReadWriteVisible);
			Buffer_ID buffer = view_get_buffer(app, view, Access_ReadWriteVisible);
			
			Range_i64 pos = {};
			if (view_has_highlighted_range(app, view)){
				pos = get_view_range(app, view);
			}
			else{
				pos.min = pos.max = view_get_cursor_pos(app, view);
			}
			
			write_text_input(app);
			
			i64 end_pos = view_get_cursor_pos(app, view);
			pos.min = Min(pos.min, end_pos);
			pos.max = Max(pos.max, end_pos);
			
			ts_auto_indent_buffer(app, buffer, pos);
			move_past_lead_whitespace(app, view, buffer);
		}
		else{
			write_text_input(app);
		}
	}
}
