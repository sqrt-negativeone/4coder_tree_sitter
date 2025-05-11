
#include "tree_sitter/api.h"

#include "4coder_default_include.cpp"

// NOTE(allen): Users can declare their own managed IDs here.

CUSTOM_ID(attachment, ts_data_id);


CUSTOM_ID(colors, ts_color_attributes);
CUSTOM_ID(colors, ts_color_namespace);
CUSTOM_ID(colors, ts_color_label);
CUSTOM_ID(colors, ts_color_operator);
CUSTOM_ID(colors, ts_color_syntax_crap);
CUSTOM_ID(colors, ts_color_sum_type);
CUSTOM_ID(colors, ts_color_prod_type);
CUSTOM_ID(colors, ts_color_macro);
CUSTOM_ID(colors, ts_color_constant);
CUSTOM_ID(colors, ts_color_comment_tag);
CUSTOM_ID(colors, ts_color_global_decl);
CUSTOM_ID(colors, ts_color_token_highlight);
CUSTOM_ID(colors, ts_color_token_minor_highlight);
CUSTOM_ID(colors, ts_color_error_annotation);
CUSTOM_ID(colors, ts_color_comment_user_name);
CUSTOM_ID(colors, ts_color_builtin_func);
CUSTOM_ID(colors, ts_color_decorator);
CUSTOM_ID(colors, ts_color_builtin_type);
CUSTOM_ID(colors, ts_color_interface_type);
CUSTOM_ID(colors, ts_color_constructor);

CUSTOM_ID(colors, defcolor_function);
CUSTOM_ID(colors, defcolor_type);
CUSTOM_ID(colors, defcolor_macro);
CUSTOM_ID(colors, defcolor_enum);
CUSTOM_ID(colors, defcolor_global);
CUSTOM_ID(colors, defcolor_primitive);
CUSTOM_ID(colors, defcolor_struct);
CUSTOM_ID(colors, defcolor_control);

#if !defined(META_PASS)
#include "generated/managed_id_metadata.cpp"
#endif


function Range_i64
tree_sitter_get_range(TSNode node)
{
	Range_i64 range = {ts_node_start_byte(node), ts_node_end_byte(node)};
	return range;
}



#include "4coder_ts_index.cpp"
#include "4coder_ts_commands.cpp"
#include "4coder_ts_layout.cpp"

BUFFER_EDIT_RANGE_SIG(tree_sitter_buffer_edit_range){
	// buffer_id, new_range, original_size
	ProfileScope(app, "default edit range");
	
	Range_i64 old_range = Ii64(old_cursor_range.min.pos, old_cursor_range.max.pos);
	
	buffer_shift_fade_ranges(buffer_id, old_range.max, (new_range.max - old_range.max));
	
	Scratch_Block scratch(app);
	Managed_Scope scope = buffer_get_managed_scope(app, buffer_id);
	
	TS_Data *ts_data = scope_attachment(app, scope, ts_data_id, TS_Data);
	if (ts_data->parser)
	{
		Buffer_Cursor new_end_point = buffer_compute_cursor(app, buffer_id, seek_pos(new_range.end));
		
		if (ts_data->tree)
		{
			TSInputEdit edit = {};
			edit.start_byte = (u32)old_cursor_range.start.pos;
			edit.old_end_byte = (u32)old_cursor_range.end.pos;
			edit.new_end_byte = (u32)new_range.max;
			edit.start_point = {(u32)old_cursor_range.start.line, (u32)old_cursor_range.start.col};
			edit.old_end_point = {(u32)old_cursor_range.start.line, (u32)old_cursor_range.start.col};
			edit.new_end_point = {(u32)new_end_point.line, (u32)new_end_point.col};
			
			ts_tree_edit(ts_data->tree, &edit);
		}
		
		// TODO(fakhri): async
		String_Const_u8 content = push_whole_buffer(app, scratch, buffer_id);
		ts_data->tree = ts_parser_parse_string(ts_data->parser, ts_data->tree, (char*)content.str, (u32)content.size);
		buffer_mark_as_modified(buffer_id);
	}
	
	{
		code_index_lock();
		Code_Index_File *file = code_index_get_file(buffer_id);
		if (file != 0){
			code_index_shift(file, old_range, range_size(new_range));
		}
		code_index_unlock();
	}
	
	
#if 0
	i64 insert_size = range_size(new_range);
	i64 text_shift = replace_range_shift(old_range, insert_size);
	Async_Task *lex_task_ptr = scope_attachment(app, scope, buffer_lex_task, Async_Task);
	
	Base_Allocator *allocator = managed_scope_allocator(app, scope);
	b32 do_full_relex = false;
	
	if (async_task_is_running_or_pending(&global_async_system, *lex_task_ptr)){
		async_task_cancel(app, &global_async_system, *lex_task_ptr);
		buffer_unmark_as_modified(buffer_id);
		do_full_relex = true;
		*lex_task_ptr = 0;
	}
	
	Token_Array *ptr = scope_attachment(app, scope, attachment_tokens, Token_Array);
	if (ptr != 0 && ptr->tokens != 0){
		ProfileBlockNamed(app, "attempt resync", profile_attempt_resync);
		
		i64 token_index_first = token_relex_first(ptr, old_range.first, 1);
		i64 token_index_resync_guess =
			token_relex_resync(ptr, old_range.one_past_last, 16);
		
		if (token_index_resync_guess - token_index_first >= 4000){
			do_full_relex = true;
		}
		else{
			Token *token_first = ptr->tokens + token_index_first;
			Token *token_resync = ptr->tokens + token_index_resync_guess;
			
			Range_i64 relex_range = Ii64(token_first->pos, token_resync->pos + token_resync->size + text_shift);
			String_Const_u8 partial_text = push_buffer_range(app, scratch, buffer_id, relex_range);
			
			Token_List relex_list = lex_full_input_cpp(scratch, partial_text);
			if (relex_range.one_past_last < buffer_get_size(app, buffer_id)){
				token_drop_eof(&relex_list);
			}
			
			Token_Relex relex = token_relex(relex_list, relex_range.first - text_shift, ptr->tokens, token_index_first, token_index_resync_guess);
			
			ProfileCloseNow(profile_attempt_resync);
			
			if (!relex.successful_resync){
				do_full_relex = true;
			}
			else{
				ProfileBlock(app, "apply resync");
				
				i64 token_index_resync = relex.first_resync_index;
				
				Range_i64 head = Ii64(0, token_index_first);
				Range_i64 replaced = Ii64(token_index_first, token_index_resync);
				Range_i64 tail = Ii64(token_index_resync, ptr->count);
				i64 resynced_count = (token_index_resync_guess + 1) - token_index_resync;
				i64 relexed_count = relex_list.total_count - resynced_count;
				i64 tail_shift = relexed_count - (token_index_resync - token_index_first);
				
				i64 new_tokens_count = ptr->count + tail_shift;
				Token *new_tokens = base_array(allocator, Token, new_tokens_count);
				
				Token *old_tokens = ptr->tokens;
				block_copy_array_shift(new_tokens, old_tokens, head, 0);
				token_fill_memory_from_list(new_tokens + replaced.first, &relex_list, relexed_count);
				for (i64 i = 0, index = replaced.first; i < relexed_count; i += 1, index += 1){
					new_tokens[index].pos += relex_range.first;
				}
				for (i64 i = tail.first; i < tail.one_past_last; i += 1){
					old_tokens[i].pos += text_shift;
				}
				block_copy_array_shift(new_tokens, ptr->tokens, tail, tail_shift);
				
				base_free(allocator, ptr->tokens);
				
				ptr->tokens = new_tokens;
				ptr->count = new_tokens_count;
				ptr->max = new_tokens_count;
				
				buffer_mark_as_modified(buffer_id);
			}
		}
	}
	
	if (do_full_relex){
		*lex_task_ptr = async_task_no_dep(&global_async_system, do_full_lex_async,
																			make_data_struct(&buffer_id));
	}
#endif
	
	// no meaning for return
	return(0);
}

BUFFER_HOOK_SIG(tree_sitter_begin_buffer){
	ProfileScope(app, "begin buffer");
	
	Scratch_Block scratch(app);
	
	Managed_Scope scope = buffer_get_managed_scope(app, buffer_id);
	
	b32 treat_as_code = ts_begin_buffer(app, buffer_id, scope);
#if 0
	String_Const_u8 file_name = push_buffer_file_name(app, scratch, buffer_id);
	if (file_name.size > 0){
		String_Const_u8 treat_as_code_string = def_get_config_string(scratch, vars_save_string_lit("treat_as_code"));
		String_Const_u8_Array extensions = parse_extension_line_to_extension_list(app, scratch, treat_as_code_string);
		String_Const_u8 ext = string_file_extension(file_name);
		for (i32 i = 0; i < extensions.count; ++i)
		{
			if (string_match(ext, extensions.strings[i])){
				
				if (
						string_match(ext, string_u8_litexpr("h"))
						|| string_match(ext, string_u8_litexpr("c"))
#if 0
						|| string_match(ext, string_u8_litexpr("cpp"))
						|| string_match(ext, string_u8_litexpr("hpp"))
						|| string_match(ext, string_u8_litexpr("cc"))
#endif
						)
				{
					treat_as_code = true;
					
				}
				break;
			}
		}
	}
#endif
	
	String_ID file_map_id = vars_save_string_lit("keys_file");
	String_ID code_map_id = vars_save_string_lit("keys_code");
	
	Command_Map_ID map_id = (treat_as_code)?(code_map_id):(file_map_id);
	Command_Map_ID *map_id_ptr = scope_attachment(app, scope, buffer_map_id, Command_Map_ID);
	*map_id_ptr = map_id;
	
	Line_Ending_Kind setting = guess_line_ending_kind_from_buffer(app, buffer_id);
	Line_Ending_Kind *eol_setting = scope_attachment(app, scope, buffer_eol_setting, Line_Ending_Kind);
	*eol_setting = setting;
	
	// NOTE(allen): Decide buffer settings
	b32 wrap_lines = true;
#if 0
	b32 use_lexer = false;
#endif
	if (treat_as_code){
		wrap_lines = def_get_config_b32(vars_save_string_lit("enable_code_wrapping"));
#if 0
		use_lexer = true;
#endif
	}
	
	String_Const_u8 buffer_name = push_buffer_base_name(app, scratch, buffer_id);
	if (buffer_name.size > 0 && buffer_name.str[0] == '*' && buffer_name.str[buffer_name.size - 1] == '*'){
		wrap_lines = def_get_config_b32(vars_save_string_lit("enable_output_wrapping"));
	}
	
#if 0
	if (use_lexer){
		ProfileBlock(app, "begin buffer kick off lexer");
		Async_Task *lex_task_ptr = scope_attachment(app, scope, buffer_lex_task, Async_Task);
		*lex_task_ptr = async_task_no_dep(&global_async_system, do_full_lex_async, make_data_struct(&buffer_id));
	}
#endif
	
	{
		b32 *wrap_lines_ptr = scope_attachment(app, scope, buffer_wrap_lines, b32);
		*wrap_lines_ptr = wrap_lines;
	}
	
#if 0
	if (use_lexer){
		buffer_set_layout(app, buffer_id, layout_virt_indent_index_generic);
	}
	else{
		if (treat_as_code){
			buffer_set_layout(app, buffer_id, layout_virt_indent_literal_generic);
		}
		else{
			buffer_set_layout(app, buffer_id, layout_generic);
		}
	}
#endif
	
	// no meaning for return
	return(0);
}


function void
tree_sitter_tick(Application_Links *app, Frame_Info frame_info){
	////////////////////////////////
	// NOTE(allen): Update code index
	
	ts_code_index_update_tick(app);
	
#if 0
	code_index_update_tick(app);
#endif
	
	////////////////////////////////
	// NOTE(allen): Update fade ranges
	
	if (tick_all_fade_ranges(app, frame_info.animation_dt)){
		animate_in_n_milliseconds(app, 0);
	}
	
	////////////////////////////////
	// NOTE(allen): Clear layouts if virtual whitespace setting changed.
	
	{
		b32 enable_virtual_whitespace = def_get_config_b32(vars_save_string_lit("enable_virtual_whitespace"));
		if (enable_virtual_whitespace != def_enable_virtual_whitespace){
			def_enable_virtual_whitespace = enable_virtual_whitespace;
			clear_all_layouts(app);
		}
	}
}



function b32
tree_sitter_draw_text_highlight_colors(Application_Links *app, Text_Layout_ID text_layout_id, Buffer_ID buffer_id, Range_i64 visible_range)
{
	Scratch_Block scratch(app);
	paint_text_color_fcolor(app, text_layout_id, visible_range, fcolor_id(defcolor_text_default));
	
	b32 result = false;
	Managed_Scope scope = buffer_get_managed_scope(app, buffer_id);
	TS_Data *ts_data = scope_attachment(app, scope, ts_data_id, TS_Data);
	if (ts_data->tree)
	{
		result = true;
		TS_Language *language = ts_data->language;
		TSQueryCursor *highlight_cursor = ts_query_cursor_new();
		TSNode root = ts_tree_root_node(ts_data->tree);
		TSNode visible_nodes = ts_node_descendant_for_byte_range(root, (u32)visible_range.start, (u32)visible_range.end);
		
		ts_query_cursor_set_byte_range(highlight_cursor, (u32)visible_range.start, (u32)visible_range.end);
		ts_query_cursor_exec(highlight_cursor, language->highlight_query, visible_nodes);
		TSQueryMatch match = {};
		u32 capture_index = 0;
		while (ts_query_cursor_next_capture(highlight_cursor, &match, &capture_index))
		{
			const TSQueryCapture *capture = match.captures + capture_index;
			
			Range_i64 node_range = tree_sitter_get_range(capture->node);
			if (node_range.start >= visible_range.end) break;
			u32 capture_name_len = 0;
			const char *capture_name_str = ts_query_capture_name_for_id(language->highlight_query, capture->index, &capture_name_len);
			String_Const_u8 capture_name = SCu8((u8*)capture_name_str, capture_name_len);
			Managed_ID color_id = managed_id_get(app, SCu8("colors"), capture_name);
			
			if (color_id) {
				paint_text_color_fcolor(app, text_layout_id, node_range, fcolor_id(color_id));
			}
		}
		ts_query_cursor_delete(highlight_cursor);
		
		TSTreeCursor tree_cursor = ts_tree_cursor_new(visible_nodes);
		ts_tree_cursor_goto_first_child_for_byte(&tree_cursor, (u32)visible_range.start);
		b32 ok = true;
		for (;ok;)
		{
			Temp_Memory_Block temp(scratch);
			TSNode node = ts_tree_cursor_current_node(&tree_cursor);
			Range_i64 node_range = tree_sitter_get_range(node);
			if (node_range.start >= visible_range.end) break;
			
			String_Const_u8 lexeme = push_buffer_range(app, scratch, buffer_id, node_range);
			TS_Index_Note *note = ts_code_index_note_from_string(lexeme);
			if (note != 0)
			{
				Managed_ID color_id = ts_code_index_color_from_note(app, language, note);
				if (color_id)
					paint_text_color_fcolor(app, text_layout_id, node_range, fcolor_id(color_id));
			}
			
			if (ts_tree_cursor_goto_first_child(&tree_cursor))
			{
				continue;
			}
			
			if (ts_tree_cursor_goto_next_sibling(&tree_cursor))
			{
				continue;
			}
			
			// NOTE(fakhri): go up the tree until we find a valid next sibling or we reach the root
			for (;ok;) {
				if (!ts_tree_cursor_goto_parent(&tree_cursor))
				{
					// NOTE(fakhri): reached the root
					ok = false;
					break;
				}
				if (ts_tree_cursor_goto_next_sibling(&tree_cursor))
				{
					break;
				}
			}
		}
		ts_tree_cursor_delete(&tree_cursor);
	}
	
	
	return result;
}

function void
tree_sitter_render_buffer(Application_Links *app, View_ID view_id, Face_ID face_id,
													Buffer_ID buffer, Text_Layout_ID text_layout_id,
													Rect_f32 rect){
	ProfileScope(app, "render buffer");
	
	View_ID active_view = get_active_view(app, Access_Always);
	b32 is_active_view = (active_view == view_id);
	Rect_f32 prev_clip = draw_set_clip(app, rect);
	
	Range_i64 visible_range = text_layout_get_visible_range(app, text_layout_id);
	
	// NOTE(allen): Cursor shape
	Face_Metrics metrics = get_face_metrics(app, face_id);
	u64 cursor_roundness_100 = def_get_config_u64(app, vars_save_string_lit("cursor_roundness"));
	f32 cursor_roundness = metrics.normal_advance*cursor_roundness_100*0.01f;
	f32 mark_thickness = (f32)def_get_config_u64(app, vars_save_string_lit("mark_thickness"));
	
#if 0
	// NOTE(allen): Token colorizing
	Token_Array token_array = get_token_array_from_buffer(app, buffer);
	if (token_array.tokens != 0){
		draw_cpp_token_colors(app, text_layout_id, &token_array);
		
		// NOTE(allen): Scan for TODOs and NOTEs
		b32 use_comment_keyword = def_get_config_b32(vars_save_string_lit("use_comment_keyword"));
		if (use_comment_keyword){
			Comment_Highlight_Pair pairs[] = {
				{string_u8_litexpr("NOTE"), finalize_color(defcolor_comment_pop, 0)},
				{string_u8_litexpr("TODO"), finalize_color(defcolor_comment_pop, 1)},
			};
			draw_comment_highlights(app, buffer, text_layout_id, &token_array, pairs, ArrayCount(pairs));
		}
		
#if 0
		// TODO(allen): Put in 4coder_draw.cpp
		// NOTE(allen): Color functions
		
		Scratch_Block scratch(app);
		ARGB_Color argb = 0xFFFF00FF;
		
		Token_Iterator_Array it = token_iterator_pos(0, &token_array, visible_range.first);
		for (;;){
			if (!token_it_inc_non_whitespace(&it)){
				break;
			}
			Token *token = token_it_read(&it);
			String_Const_u8 lexeme = push_token_lexeme(app, scratch, buffer, token);
			Code_Index_Note *note = code_index_note_from_string(lexeme);
			if (note != 0 && note->note_kind == CodeIndexNote_Function){
				paint_text_color(app, text_layout_id, Ii64_size(token->pos, token->size), argb);
			}
		}
#endif
	}
	else{
		paint_text_color_fcolor(app, text_layout_id, visible_range, fcolor_id(defcolor_text_default));
	}
#else
	tree_sitter_draw_text_highlight_colors(app, text_layout_id, buffer, visible_range);
#endif
	
	i64 cursor_pos = view_correct_cursor(app, view_id);
	view_correct_mark(app, view_id);
	
	// NOTE(allen): Scope highlight
	b32 use_scope_highlight = def_get_config_b32(vars_save_string_lit("use_scope_highlight"));
	if (use_scope_highlight){
		Color_Array colors = finalize_color_array(defcolor_back_cycle);
		draw_scope_highlight(app, buffer, text_layout_id, cursor_pos, colors.vals, colors.count);
	}
	
	b32 use_error_highlight = def_get_config_b32(vars_save_string_lit("use_error_highlight"));
	b32 use_jump_highlight = def_get_config_b32(vars_save_string_lit("use_jump_highlight"));
	if (use_error_highlight || use_jump_highlight){
		// NOTE(allen): Error highlight
		String_Const_u8 name = string_u8_litexpr("*compilation*");
		Buffer_ID compilation_buffer = get_buffer_by_name(app, name, Access_Always);
		if (use_error_highlight){
			draw_jump_highlights(app, buffer, text_layout_id, compilation_buffer,
													 fcolor_id(defcolor_highlight_junk));
		}
		
		// NOTE(allen): Search highlight
		if (use_jump_highlight){
			Buffer_ID jump_buffer = get_locked_jump_buffer(app);
			if (jump_buffer != compilation_buffer){
				draw_jump_highlights(app, buffer, text_layout_id, jump_buffer,
														 fcolor_id(defcolor_highlight_white));
			}
		}
	}
	
	// NOTE(allen): Color parens
	b32 use_paren_helper = def_get_config_b32(vars_save_string_lit("use_paren_helper"));
	if (use_paren_helper){
		Color_Array colors = finalize_color_array(defcolor_text_cycle);
		draw_paren_highlight(app, buffer, text_layout_id, cursor_pos, colors.vals, colors.count);
	}
	
	// NOTE(allen): Line highlight
	b32 highlight_line_at_cursor = def_get_config_b32(vars_save_string_lit("highlight_line_at_cursor"));
	if (highlight_line_at_cursor && is_active_view){
		i64 line_number = get_line_number_from_pos(app, buffer, cursor_pos);
		draw_line_highlight(app, text_layout_id, line_number, fcolor_id(defcolor_highlight_cursor_line));
	}
	
#if 0
	// NOTE(allen): Whitespace highlight
	b64 show_whitespace = false;
	view_get_setting(app, view_id, ViewSetting_ShowWhitespace, &show_whitespace);
	if (show_whitespace){
		if (token_array.tokens == 0){
			draw_whitespace_highlight(app, buffer, text_layout_id, cursor_roundness);
		}
		else{
			draw_whitespace_highlight(app, text_layout_id, &token_array, cursor_roundness);
		}
	}
#endif
	
	// NOTE(allen): Cursor
	switch (fcoder_mode){
		case FCoderMode_Original:
		{
			draw_original_4coder_style_cursor_mark_highlight(app, view_id, is_active_view, buffer, text_layout_id, cursor_roundness, mark_thickness);
		}break;
		case FCoderMode_NotepadLike:
		{
			draw_notepad_style_cursor_highlight(app, view_id, buffer, text_layout_id, cursor_roundness);
		}break;
	}
	
	// NOTE(allen): Fade ranges
	paint_fade_ranges(app, text_layout_id, buffer);
	
	// NOTE(allen): put the actual text on the actual screen
	draw_text_layout_default(app, text_layout_id);
	
	draw_set_clip(app, prev_clip);
}

function void
tree_sitter_render_caller(Application_Links *app, Frame_Info frame_info, View_ID view_id){
	ProfileScope(app, "default render caller");
	View_ID active_view = get_active_view(app, Access_Always);
	b32 is_active_view = (active_view == view_id);
	
	Rect_f32 region = draw_background_and_margin(app, view_id, is_active_view);
	Rect_f32 prev_clip = draw_set_clip(app, region);
	
	Buffer_ID buffer = view_get_buffer(app, view_id, Access_Always);
	Face_ID face_id = get_face_id(app, buffer);
	Face_Metrics face_metrics = get_face_metrics(app, face_id);
	f32 line_height = face_metrics.line_height;
	f32 digit_advance = face_metrics.decimal_digit_advance;
	
	// NOTE(allen): file bar
	b64 showing_file_bar = false;
	if (view_get_setting(app, view_id, ViewSetting_ShowFileBar, &showing_file_bar) && showing_file_bar){
		Rect_f32_Pair pair = layout_file_bar_on_top(region, line_height);
		draw_file_bar(app, view_id, buffer, face_id, pair.min);
		region = pair.max;
	}
	
	Buffer_Scroll scroll = view_get_buffer_scroll(app, view_id);
	
	Buffer_Point_Delta_Result delta = delta_apply(app, view_id,
																								frame_info.animation_dt, scroll);
	if (!block_match_struct(&scroll.position, &delta.point)){
		block_copy_struct(&scroll.position, &delta.point);
		view_set_buffer_scroll(app, view_id, scroll, SetBufferScroll_NoCursorChange);
	}
	if (delta.still_animating){
		animate_in_n_milliseconds(app, 0);
	}
	
	// NOTE(allen): query bars
	region = default_draw_query_bars(app, region, view_id, face_id);
	
	// NOTE(allen): FPS hud
	if (show_fps_hud){
		Rect_f32_Pair pair = layout_fps_hud_on_bottom(region, line_height);
		draw_fps_hud(app, frame_info, face_id, pair.max);
		region = pair.min;
		animate_in_n_milliseconds(app, 1000);
	}
	
	// NOTE(allen): layout line numbers
	b32 show_line_number_margins = def_get_config_b32(vars_save_string_lit("show_line_number_margins"));
	Rect_f32 line_number_rect = {};
	if (show_line_number_margins){
		Rect_f32_Pair pair = layout_line_number_margin(app, buffer, region, digit_advance);
		line_number_rect = pair.min;
		region = pair.max;
	}
	
	// NOTE(allen): begin buffer render
	Buffer_Point buffer_point = scroll.position;
	Text_Layout_ID text_layout_id = text_layout_create(app, buffer, region, buffer_point);
	
	// NOTE(allen): draw line numbers
	if (show_line_number_margins){
		draw_line_number_margin(app, view_id, buffer, face_id, text_layout_id, line_number_rect);
	}
	
	// NOTE(allen): draw the buffer
	tree_sitter_render_buffer(app, view_id, face_id, buffer, text_layout_id, region);
	
	text_layout_free(app, text_layout_id);
	draw_set_clip(app, prev_clip);
}

BUFFER_HOOK_SIG(ts_end_buffer){
	end_buffer_close_jump_list(app, buffer_id);
	
	Managed_Scope scope = buffer_get_managed_scope(app, buffer_id);
	TS_Data *ts_data = scope_attachment(app, scope, ts_data_id, TS_Data);
	
	ts_code_index_lock();
	ts_clear_index_file(ts_data);
	ts_code_index_unlock();
	
	ts_parser_delete(ts_data->parser);
	
	return(0);
}

void
custom_layer_init(Application_Links *app){
	Thread_Context *tctx = get_thread_context(app);
	
	// NOTE(allen): setup for default framework
	default_framework_init(app);
	
	ts_code_index_init(app);
	
	// NOTE(allen): default hooks and command maps
	set_all_default_hooks(app);
	
	set_custom_hook(app, HookID_BeginBuffer,     tree_sitter_begin_buffer);
	set_custom_hook(app, HookID_EndBuffer,       ts_end_buffer);
	set_custom_hook(app, HookID_Tick,            tree_sitter_tick);
	set_custom_hook(app, HookID_RenderCaller,    tree_sitter_render_caller);
	set_custom_hook(app, HookID_BufferEditRange, tree_sitter_buffer_edit_range);
	set_custom_hook(app, HookID_Layout, ts_layout);
	
	mapping_init(tctx, &framework_mapping);
	String_ID global_map_id = vars_save_string_lit("keys_global");
	String_ID file_map_id = vars_save_string_lit("keys_file");
	String_ID code_map_id = vars_save_string_lit("keys_code");
#if OS_MAC
	setup_mac_mapping(&framework_mapping, global_map_id, file_map_id, code_map_id);
#else
	setup_default_mapping(&framework_mapping, global_map_id, file_map_id, code_map_id);
#endif
	setup_essential_mapping(&framework_mapping, global_map_id, file_map_id, code_map_id);
	
	
	// NOTE(fakhri): create ts tree buffer
	{
		Buffer_ID buffer = create_buffer(app, string_u8_litexpr("*ts_tree*"),
																		 BufferCreate_NeverAttachToFile |
																		 BufferCreate_AlwaysNew);
		buffer_set_setting(app, buffer, BufferSetting_Unimportant, true);
		buffer_set_setting(app, buffer, BufferSetting_ReadOnly, true);
	}
	
	
	// NOTE(fakhri): create nests buffer
	{
		Buffer_ID buffer = create_buffer(app, string_u8_litexpr("*nests*"),
																		 BufferCreate_NeverAttachToFile |
																		 BufferCreate_AlwaysNew);
		buffer_set_setting(app, buffer, BufferSetting_Unimportant, true);
		buffer_set_setting(app, buffer, BufferSetting_ReadOnly, true);
	}
}

