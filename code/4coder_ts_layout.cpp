function TS_Layout_Scope *
ts_get_layout_scope_for_offset(TS_Layout_Scope *root, i64 offset)
{
	TS_Layout_Scope *node = root;
	for (b32 stop = false; !stop;)
	{
		stop = true;
		for (TS_Layout_Scope *child = node->first_child; child; child = child->next)
		{
			if (range_contains(child->range, offset))
			{
				node = child;
				stop = false;
				break;
			}
		}
	}
	return node;
}

function Layout_Item_List
ts_layout__inner(Application_Links *app, Arena *arena, Buffer_ID buffer, Range_i64 range, Face_ID face, f32 width, TS_Index_File *file, Layout_Wrap_Kind kind)
{
	Layout_Item_List list = get_empty_item_list(range);
	Scratch_Block scratch(app);
	String_Const_u8 text = push_buffer_range(app, scratch, buffer, range);
	
	Face_Advance_Map advance_map = get_face_advance_map(app, face);
	Face_Metrics metrics = get_face_metrics(app, face);
	f32 tab_width = (f32)def_get_config_u64(app, vars_save_string_lit("default_tab_width"));
	tab_width = clamp_bot(1, tab_width);
	LefRig_TopBot_Layout_Vars pos_vars = get_lr_tb_layout_vars(&advance_map, &metrics, tab_width, width);
	
	u64 vw_indent = def_get_config_u64(app, vars_save_string_lit("virtual_whitespace_regular_indent"));
	f32 regular_indent = metrics.space_advance*vw_indent;
	f32 wrap_align_x = width - metrics.normal_advance;
	
	if (text.size == 0){
		lr_tb_write_blank(&pos_vars, face, arena, &list, range.first);
	}
	else{
		b32 first_of_the_line = true;
		Newline_Layout_Vars newline_vars = get_newline_layout_vars();
		
		u8 *ptr = text.str;
		u8 *end_ptr = ptr + text.size;
		u8 *word_ptr = ptr;
		
		u8 *pending_wrap_ptr = ptr;
		f32 pending_wrap_x = 0.f;
#if 0
		i32 pending_wrap_paren_nest_count = 0;
#endif
		i32 pending_wrap_token_score = 0;
		f32 pending_wrap_accumulated_w = 0.f;
		
		start:
		if (ptr == end_ptr){
			i64 index = layout_index_from_ptr(ptr, text.str, range.first);
			TS_Layout_Scope *layout_scope = ts_get_layout_scope_for_offset(file->scopes_root, index);
			f32 shift = layout_scope->nest_level * regular_indent;
			lr_tb_advance_x_without_item(&pos_vars, shift);
			goto finish;
		}
		
		if (!character_is_whitespace(*ptr)){
			i64 index = layout_index_from_ptr(ptr, text.str, range.first);
			TS_Layout_Scope *layout_scope = ts_get_layout_scope_for_offset(file->scopes_root, index);
			f32 shift = layout_scope->nest_level * regular_indent;
			lr_tb_advance_x_without_item(&pos_vars, shift);
			goto consuming_non_whitespace;
		}
		
		{
			for (;ptr < end_ptr; ptr += 1){
				if (!character_is_whitespace(*ptr)){
					pending_wrap_ptr = ptr;
					word_ptr = ptr;
					i64 index = layout_index_from_ptr(ptr, text.str, range.first);
					TS_Layout_Scope *layout_scope = ts_get_layout_scope_for_offset(file->scopes_root, index);
					f32 shift = layout_scope->nest_level * regular_indent;
					lr_tb_advance_x_without_item(&pos_vars, shift);
					goto consuming_non_whitespace;
				}
				if (*ptr == '\r'){
					i64 index = layout_index_from_ptr(ptr, text.str, range.first);
					newline_layout_consume_CR(&newline_vars, index);
				}
				else if (*ptr == '\n'){
					pending_wrap_ptr = ptr;
					i64 index = layout_index_from_ptr(ptr, text.str, range.first);
					TS_Layout_Scope *layout_scope = ts_get_layout_scope_for_offset(file->scopes_root, index);
					f32 shift = layout_scope->nest_level * regular_indent;
					lr_tb_advance_x_without_item(&pos_vars, shift);
					goto consuming_normal_whitespace;
				}
			}
			
			if (ptr == end_ptr){
				pending_wrap_ptr = ptr;
				i64 index = layout_index_from_ptr(ptr - 1, text.str, range.first);
				TS_Layout_Scope *layout_scope = ts_get_layout_scope_for_offset(file->scopes_root, index);
				f32 shift = layout_scope->nest_level * regular_indent;
				lr_tb_advance_x_without_item(&pos_vars, shift);
				goto finish;
			}
		}
		
		consuming_non_whitespace:
		{
			for (;ptr <= end_ptr; ptr += 1){
				if (ptr == end_ptr || character_is_whitespace(*ptr)){
					break;
				}
			}
			
			// NOTE(allen): measure this word
			newline_layout_consume_default(&newline_vars);
			String_Const_u8 word = SCu8(word_ptr, ptr);
			u8 *word_end = ptr;
			{
				f32 word_advance = 0.f;
				ptr = word.str;
				for (;ptr < word_end;){
					Character_Consume_Result consume = utf8_consume(ptr, (u64)(word_end - ptr));
					if (consume.codepoint != max_u32){
						word_advance += lr_tb_advance(&pos_vars, face, consume.codepoint);
					}
					else{
						word_advance += lr_tb_advance_byte(&pos_vars);
					}
					ptr += consume.inc;
				}
				pending_wrap_accumulated_w += word_advance;
			}
			
			if (!first_of_the_line && (kind == Layout_Wrapped) && lr_tb_crosses_width(&pos_vars, pending_wrap_accumulated_w)){
				i64 index = layout_index_from_ptr(pending_wrap_ptr, text.str, range.first);
				lr_tb_align_rightward(&pos_vars, wrap_align_x);
				lr_tb_write_ghost(&pos_vars, face, arena, &list, index, '\\');
				
				lr_tb_next_line(&pos_vars);
#if 0
				f32 shift = layout_index_x_shift(app, &reflex, file, index, regular_indent);
				lr_tb_advance_x_without_item(&pos_vars, shift);
#endif
				
				ptr = pending_wrap_ptr;
				pending_wrap_accumulated_w = 0.f;
				first_of_the_line = true;
				goto start;
			}
		}
		
		consuming_normal_whitespace:
		for (; ptr < end_ptr; ptr += 1){
			if (!character_is_whitespace(*ptr)){
				u8 *new_wrap_ptr = ptr;
				
				i64 index = layout_index_from_ptr(new_wrap_ptr, text.str, range.first);
				TS_Layout_Scope *layout_scope = ts_get_layout_scope_for_offset(file->scopes_root, index);
				f32 new_wrap_x = layout_scope->nest_level * regular_indent;
				
#if 0
				b32 invalid_wrap_x = false;
				Code_Index_Nest *new_wrap_nest = code_index_get_nest(file, index);
				f32 new_wrap_x = layout_index_x_shift(app, &reflex, new_wrap_nest, index, regular_indent, &invalid_wrap_x);
				if (invalid_wrap_x){
					new_wrap_x = max_f32;
				}
				
				i32 new_wrap_paren_nest_count = 0;
				for (Code_Index_Nest *nest = new_wrap_nest;
						 nest != 0;
						 nest = nest->parent){
					if (nest->kind == CodeIndexNest_Paren){
						new_wrap_paren_nest_count += 1;
					}
				}
#endif
				
				// TODO(allen): pull out the token scoring part and make it replacable for other
				// language's token based wrap scoring needs.
				i32 token_score = 0;
#if 0
				Token_Pair new_wrap_token_pair = layout_token_pair(tokens_ptr, index);
				if (new_wrap_token_pair.a.kind == TokenBaseKind_Keyword){
					if (new_wrap_token_pair.b.kind == TokenBaseKind_ParentheticalOpen ||
							new_wrap_token_pair.b.kind == TokenBaseKind_Keyword){
						token_score -= 2;
					}
				}
				token_score += layout_token_score_wrap_token(&new_wrap_token_pair, TokenCppKind_Eq);
				token_score += layout_token_score_wrap_token(&new_wrap_token_pair, TokenCppKind_PlusEq);
				token_score += layout_token_score_wrap_token(&new_wrap_token_pair, TokenCppKind_MinusEq);
				token_score += layout_token_score_wrap_token(&new_wrap_token_pair, TokenCppKind_StarEq);
				token_score += layout_token_score_wrap_token(&new_wrap_token_pair, TokenCppKind_DivEq);
				token_score += layout_token_score_wrap_token(&new_wrap_token_pair, TokenCppKind_ModEq);
				token_score += layout_token_score_wrap_token(&new_wrap_token_pair, TokenCppKind_LeftLeftEq);
				token_score += layout_token_score_wrap_token(&new_wrap_token_pair, TokenCppKind_RightRightEq);
				token_score += layout_token_score_wrap_token(&new_wrap_token_pair, TokenCppKind_Comma);
				token_score += layout_token_score_wrap_token(&new_wrap_token_pair, TokenCppKind_AndAnd);
				token_score += layout_token_score_wrap_token(&new_wrap_token_pair, TokenCppKind_OrOr);
				token_score += layout_token_score_wrap_token(&new_wrap_token_pair, TokenCppKind_Ternary);
				token_score += layout_token_score_wrap_token(&new_wrap_token_pair, TokenCppKind_Colon);
				token_score += layout_token_score_wrap_token(&new_wrap_token_pair, TokenCppKind_Semicolon);
#endif
				
				i32 new_wrap_token_score = token_score;
				
				b32 new_wrap_ptr_is_better = false;
				if (first_of_the_line){
					new_wrap_ptr_is_better = true;
				}
				else{
					if (new_wrap_token_score > pending_wrap_token_score){
						new_wrap_ptr_is_better = true;
					}
					else if (new_wrap_token_score == pending_wrap_token_score){
						f32 new_score = new_wrap_x;
						f32 old_score = pending_wrap_x + metrics.normal_advance*4.f + pending_wrap_accumulated_w*0.5f;
						
						if (new_score < old_score){
							new_wrap_ptr_is_better = true;
						}
					}
				}
				
				if (new_wrap_ptr_is_better){
					layout_index__emit_chunk(&pos_vars, face, arena, text.str, range.first, pending_wrap_ptr, new_wrap_ptr, &list);
					first_of_the_line = false;
					
					pending_wrap_ptr = new_wrap_ptr;
					pending_wrap_x = new_wrap_x;
#if 0
					pending_wrap_x =  layout_index_x_shift(app, &reflex, new_wrap_nest, index, regular_indent);
					pending_wrap_paren_nest_count = new_wrap_paren_nest_count;
					pending_wrap_paren_nest_count = new_wrap_paren_nest_count;
#endif
					pending_wrap_token_score = new_wrap_token_score;
					pending_wrap_accumulated_w = 0.f;
				}
				
				word_ptr = ptr;
				goto consuming_non_whitespace;
			}
			
			i64 index = layout_index_from_ptr(ptr, text.str, range.first);
			switch (*ptr){
				default:
				{
					newline_layout_consume_default(&newline_vars);
					pending_wrap_accumulated_w += lr_tb_advance(&pos_vars, face, *ptr);
				}break;
				
				case '\r':
				{
					newline_layout_consume_CR(&newline_vars, index);
				}break;
				
				case '\n':
				{
					layout_index__emit_chunk(&pos_vars, face, arena, text.str, range.first, pending_wrap_ptr, ptr, &list);
					pending_wrap_ptr = ptr + 1;
					pending_wrap_accumulated_w = 0.f;
					
					u64 newline_index = newline_layout_consume_LF(&newline_vars, index);
					lr_tb_write_blank(&pos_vars, face, arena, &list, newline_index);
					lr_tb_next_line(&pos_vars);
					first_of_the_line = true;
					ptr += 1;
					goto start;
				}break;
			}
		}
		
		finish:
		if (newline_layout_consume_finish(&newline_vars)){
			layout_index__emit_chunk(&pos_vars, face, arena, text.str, range.first, pending_wrap_ptr, ptr, &list);
			i64 index = layout_index_from_ptr(ptr, text.str, range.first);
			lr_tb_write_blank(&pos_vars, face, arena, &list, index);
		}
	}
	
	layout_item_list_finish(&list, -pos_vars.line_to_text_shift);
	
	return list;
}

function Layout_Item_List
ts_layout(Application_Links *app, Arena *arena, Buffer_ID buffer, Range_i64 range, Face_ID face, f32 width)
{
	Layout_Item_List list = get_empty_item_list(range);
	Managed_Scope scope = buffer_get_managed_scope(app, buffer);
	TS_Data *ts_data = scope_attachment(app, scope, ts_data_id, TS_Data);
	TS_Index_File *file = ts_data->file;
	if (file)
	{
		list = ts_layout__inner(app, arena, buffer, range, face, width, file, Layout_Unwrapped);
	}
	else {
		list = layout_virt_indent_literal(app, arena, buffer, range, face, width, Layout_Unwrapped);
	}
	
	return(list);
}

