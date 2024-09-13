// Copyright (c) 2024 Epic Games Tools
// Licensed under the MIT license (https://opensource.org/license/mit/)

#undef MARKUP_LAYER_COLOR
#define MARKUP_LAYER_COLOR 0.10f, 0.20f, 0.25f

////////////////////////////////
//~ rjf: Generated Code

#include "generated/dbg_frontend.meta.c"

////////////////////////////////
//~ rjf: Registers Type Functions

internal void
df_regs_copy_contents(Arena *arena, DF_Regs *dst, DF_Regs *src)
{
  MemoryCopyStruct(dst, src);
  dst->entity_list = d_handle_list_copy(arena, src->entity_list);
  dst->file_path   = push_str8_copy(arena, src->file_path);
  dst->lines       = d_line_list_copy(arena, &src->lines);
  dst->dbgi_key    = di_key_copy(arena, &src->dbgi_key);
  dst->string      = push_str8_copy(arena, src->string);
  dst->params_tree = md_tree_copy(arena, src->params_tree);
  if(dst->entity_list.count == 0 && !d_handle_match(d_handle_zero(), dst->entity))
  {
    d_handle_list_push(arena, &dst->entity_list, dst->entity);
  }
}

internal DF_Regs *
df_regs_copy(Arena *arena, DF_Regs *src)
{
  DF_Regs *dst = push_array(arena, DF_Regs, 1);
  df_regs_copy_contents(arena, dst, src);
  return dst;
}

////////////////////////////////
//~ rjf: Commands Type Functions

internal void
df_cmd_list_push_new(Arena *arena, DF_CmdList *cmds, String8 name, DF_Regs *regs)
{
  DF_CmdNode *n = push_array(arena, DF_CmdNode, 1);
  n->cmd.name = push_str8_copy(arena, name);
  n->cmd.regs = df_regs_copy(arena, regs);
  DLLPushBack(cmds->first, cmds->last, n);
  cmds->count += 1;
}

////////////////////////////////
//~ rjf: Entity Functions

//- rjf: nil

internal B32
df_entity_is_nil(DF_Entity *entity)
{
  return (entity == 0 || entity == &d_nil_entity);
}

//- rjf: handle <-> entity conversions

internal U64
df_index_from_entity(DF_Entity *entity)
{
  return (U64)(entity - df_state->entities_base);
}

internal D_Handle
df_handle_from_entity(DF_Entity *entity)
{
  D_Handle handle = d_handle_zero();
  if(!df_entity_is_nil(entity))
  {
    handle.u64[0] = df_index_from_entity(entity);
    handle.u64[1] = entity->gen;
  }
  return handle;
}

internal DF_Entity *
df_entity_from_handle(D_Handle handle)
{
  DF_Entity *result = df_state->entities_base + handle.u64[0];
  if(handle.u64[0] >= df_state->entities_count || result->gen != handle.u64[1])
  {
    result = &d_nil_entity;
  }
  return result;
}

internal D_HandleList
df_handle_list_from_entity_list(Arena *arena, DF_EntityList entities)
{
  D_HandleList result = {0};
  for(DF_EntityNode *n = entities.first; n != 0; n = n->next)
  {
    D_Handle handle = df_handle_from_entity(n->entity);
    d_handle_list_push(arena, &result, handle);
  }
  return result;
}

//- rjf: entity recursion iterators

internal DF_EntityRec
df_entity_rec_depth_first(DF_Entity *entity, DF_Entity *subtree_root, U64 sib_off, U64 child_off)
{
  DF_EntityRec result = {0};
  if(!df_entity_is_nil(*MemberFromOffset(DF_Entity **, entity, child_off)))
  {
    result.next = *MemberFromOffset(DF_Entity **, entity, child_off);
    result.push_count = 1;
  }
  else for(DF_Entity *parent = entity; parent != subtree_root && !df_entity_is_nil(parent); parent = parent->parent)
  {
    if(parent != subtree_root && !df_entity_is_nil(*MemberFromOffset(DF_Entity **, parent, sib_off)))
    {
      result.next = *MemberFromOffset(DF_Entity **, parent, sib_off);
      break;
    }
    result.pop_count += 1;
  }
  return result;
}

//- rjf: ancestor/child introspection

internal DF_Entity *
df_entity_child_from_kind(DF_Entity *entity, DF_EntityKind kind)
{
  DF_Entity *result = &d_nil_entity;
  for(DF_Entity *child = entity->first; !df_entity_is_nil(child); child = child->next)
  {
    if(child->kind == kind)
    {
      result = child;
      break;
    }
  }
  return result;
}

internal DF_Entity *
df_entity_ancestor_from_kind(DF_Entity *entity, DF_EntityKind kind)
{
  DF_Entity *result = &d_nil_entity;
  for(DF_Entity *p = entity->parent; !df_entity_is_nil(p); p = p->parent)
  {
    if(p->kind == kind)
    {
      result = p;
      break;
    }
  }
  return result;
}

internal DF_EntityList
df_push_entity_child_list_with_kind(Arena *arena, DF_Entity *entity, DF_EntityKind kind)
{
  DF_EntityList result = {0};
  for(DF_Entity *child = entity->first; !df_entity_is_nil(child); child = child->next)
  {
    if(child->kind == kind)
    {
      df_entity_list_push(arena, &result, child);
    }
  }
  return result;
}

internal DF_Entity *
df_entity_child_from_string_and_kind(DF_Entity *parent, String8 string, DF_EntityKind kind)
{
  DF_Entity *result = &d_nil_entity;
  for(DF_Entity *child = parent->first; !df_entity_is_nil(child); child = child->next)
  {
    if(str8_match(child->string, string, 0) && child->kind == kind)
    {
      result = child;
      break;
    }
  }
  return result;
}

//- rjf: entity list building

internal void
df_entity_list_push(Arena *arena, DF_EntityList *list, DF_Entity *entity)
{
  DF_EntityNode *n = push_array(arena, DF_EntityNode, 1);
  n->entity = entity;
  SLLQueuePush(list->first, list->last, n);
  list->count += 1;
}

internal DF_EntityArray
df_entity_array_from_list(Arena *arena, DF_EntityList *list)
{
  DF_EntityArray result = {0};
  result.count = list->count;
  result.v = push_array(arena, DF_Entity *, result.count);
  U64 idx = 0;
  for(DF_EntityNode *n = list->first; n != 0; n = n->next, idx += 1)
  {
    result.v[idx] = n->entity;
  }
  return result;
}

//- rjf: entity fuzzy list building

internal DF_EntityFuzzyItemArray
df_entity_fuzzy_item_array_from_entity_list_needle(Arena *arena, DF_EntityList *list, String8 needle)
{
  Temp scratch = scratch_begin(&arena, 1);
  DF_EntityArray array = df_entity_array_from_list(scratch.arena, list);
  DF_EntityFuzzyItemArray result = df_entity_fuzzy_item_array_from_entity_array_needle(arena, &array, needle);
  return result;
}

internal DF_EntityFuzzyItemArray
df_entity_fuzzy_item_array_from_entity_array_needle(Arena *arena, DF_EntityArray *array, String8 needle)
{
  Temp scratch = scratch_begin(&arena, 1);
  DF_EntityFuzzyItemArray result = {0};
  result.count = array->count;
  result.v = push_array(arena, DF_EntityFuzzyItem, result.count);
  U64 result_idx = 0;
  for(U64 src_idx = 0; src_idx < array->count; src_idx += 1)
  {
    DF_Entity *entity = array->v[src_idx];
    String8 display_string = df_display_string_from_entity(scratch.arena, entity);
    FuzzyMatchRangeList matches = fuzzy_match_find(arena, needle, display_string);
    if(matches.count >= matches.needle_part_count)
    {
      result.v[result_idx].entity = entity;
      result.v[result_idx].matches = matches;
      result_idx += 1;
    }
    else
    {
      String8 search_tags = df_search_tags_from_entity(scratch.arena, entity);
      if(search_tags.size != 0)
      {
        FuzzyMatchRangeList tag_matches = fuzzy_match_find(scratch.arena, needle, search_tags);
        if(tag_matches.count >= tag_matches.needle_part_count)
        {
          result.v[result_idx].entity = entity;
          result.v[result_idx].matches = matches;
          result_idx += 1;
        }
      }
    }
  }
  result.count = result_idx;
  scratch_end(scratch);
  return result;
}

//- rjf: full path building, from file/folder entities

internal String8
df_full_path_from_entity(Arena *arena, DF_Entity *entity)
{
  String8 string = {0};
  {
    Temp scratch = scratch_begin(&arena, 1);
    String8List strs = {0};
    for(DF_Entity *e = entity; !df_entity_is_nil(e); e = e->parent)
    {
      if(e->kind == DF_EntityKind_File)
      {
        str8_list_push_front(scratch.arena, &strs, e->string);
      }
    }
    StringJoin join = {0};
    join.sep = str8_lit("/");
    string = str8_list_join(arena, &strs, &join);
    scratch_end(scratch);
  }
  return string;
}

//- rjf: display string entities, for referencing entities in ui

internal String8
df_display_string_from_entity(Arena *arena, DF_Entity *entity)
{
  String8 result = {0};
  switch(entity->kind)
  {
    default:
    {
      if(entity->string.size != 0)
      {
        result = push_str8_copy(arena, entity->string);
      }
      else
      {
        String8 kind_string = d_entity_kind_display_string_table[entity->kind];
        result = push_str8f(arena, "%S $%I64u", kind_string, entity->id);
      }
    }break;
    
    case DF_EntityKind_Target:
    {
      if(entity->string.size != 0)
      {
        result = push_str8_copy(arena, entity->string);
      }
      else
      {
        DF_Entity *exe = df_entity_child_from_kind(entity, DF_EntityKind_Executable);
        result = push_str8_copy(arena, exe->string);
      }
    }break;
    
    case DF_EntityKind_Breakpoint:
    {
      if(entity->string.size != 0)
      {
        result = push_str8_copy(arena, entity->string);
      }
      else
      {
        DF_Entity *loc = df_entity_child_from_kind(entity, DF_EntityKind_Location);
        if(loc->flags & DF_EntityFlag_HasTextPoint)
        {
          result = push_str8f(arena, "%S:%I64d:%I64d", str8_skip_last_slash(loc->string), loc->text_point.line, loc->text_point.column);
        }
        else if(loc->flags & DF_EntityFlag_HasVAddr)
        {
          result = str8_from_u64(arena, loc->vaddr, 16, 16, 0);
        }
        else if(loc->string.size != 0)
        {
          result = push_str8_copy(arena, loc->string);
        }
      }
    }break;
    
    case DF_EntityKind_Process:
    {
      DF_Entity *main_mod_child = df_entity_child_from_kind(entity, DF_EntityKind_Module);
      String8 main_mod_name = str8_skip_last_slash(main_mod_child->string);
      result = push_str8f(arena, "%S%s%sPID: %i%s",
                          main_mod_name,
                          main_mod_name.size != 0 ? " " : "",
                          main_mod_name.size != 0 ? "(" : "",
                          entity->ctrl_id,
                          main_mod_name.size != 0 ? ")" : "");
    }break;
    
    case DF_EntityKind_Thread:
    {
      String8 name = entity->string;
      if(name.size == 0)
      {
        DF_Entity *process = df_entity_ancestor_from_kind(entity, DF_EntityKind_Process);
        DF_Entity *first_thread = df_entity_child_from_kind(process, DF_EntityKind_Thread);
        if(first_thread == entity)
        {
          name = str8_lit("Main Thread");
        }
      }
      result = push_str8f(arena, "%S%s%sTID: %i%s",
                          name,
                          name.size != 0 ? " " : "",
                          name.size != 0 ? "(" : "",
                          entity->ctrl_id,
                          name.size != 0 ? ")" : "");
    }break;
    
    case DF_EntityKind_Module:
    {
      result = push_str8_copy(arena, str8_skip_last_slash(entity->string));
    }break;
    
    case DF_EntityKind_RecentProject:
    {
      result = push_str8_copy(arena, str8_skip_last_slash(entity->string));
    }break;
  }
  return result;
}

//- rjf: extra search tag strings for fuzzy filtering entities

internal String8
df_search_tags_from_entity(Arena *arena, DF_Entity *entity)
{
  String8 result = {0};
  if(entity->kind == DF_EntityKind_Thread)
  {
    Temp scratch = scratch_begin(&arena, 1);
    CTRL_Entity *entity_ctrl = ctrl_entity_from_handle(d_state->ctrl_entity_store, entity->ctrl_handle);
    CTRL_Entity *process = ctrl_entity_ancestor_from_kind(entity_ctrl, DF_EntityKind_Process);
    CTRL_Unwind unwind = d_query_cached_unwind_from_thread(entity_ctrl);
    String8List strings = {0};
    for(U64 frame_num = unwind.frames.count; frame_num > 0; frame_num -= 1)
    {
      CTRL_UnwindFrame *f = &unwind.frames.v[frame_num-1];
      U64 rip_vaddr = regs_rip_from_arch_block(entity->arch, f->regs);
      CTRL_Entity *module = ctrl_module_from_process_vaddr(process, rip_vaddr);
      U64 rip_voff = ctrl_voff_from_vaddr(module, rip_vaddr);
      DI_Key dbgi_key = ctrl_dbgi_key_from_module(module);
      String8 procedure_name = d_symbol_name_from_dbgi_key_voff(scratch.arena, &dbgi_key, rip_voff, 0);
      if(procedure_name.size != 0)
      {
        str8_list_push(scratch.arena, &strings, procedure_name);
      }
    }
    StringJoin join = {0};
    join.sep = str8_lit(",");
    result = str8_list_join(arena, &strings, &join);
    scratch_end(scratch);
  }
  return result;
}

//- rjf: entity -> color operations

internal Vec4F32
df_hsva_from_entity(DF_Entity *entity)
{
  Vec4F32 result = {0};
  if(entity->flags & DF_EntityFlag_HasColor)
  {
    result = entity->color_hsva;
  }
  return result;
}

internal Vec4F32
df_rgba_from_entity(DF_Entity *entity)
{
  Vec4F32 result = {0};
  if(entity->flags & DF_EntityFlag_HasColor)
  {
    Vec3F32 hsv = v3f32(entity->color_hsva.x, entity->color_hsva.y, entity->color_hsva.z);
    Vec3F32 rgb = rgb_from_hsv(hsv);
    result = v4f32(rgb.x, rgb.y, rgb.z, entity->color_hsva.w);
  }
  return result;
}

//- rjf: entity -> expansion tree keys

internal EV_Key
df_ev_key_from_entity(DF_Entity *entity)
{
  EV_Key parent_key = df_parent_ev_key_from_entity(entity);
  EV_Key key = ev_key_make(ev_hash_from_key(parent_key), (U64)entity);
  return key;
}

internal EV_Key
df_parent_ev_key_from_entity(DF_Entity *entity)
{
  EV_Key parent_key = ev_key_make(5381, (U64)entity);
  return parent_key;
}

//- rjf: entity -> evaluation

internal DF_EntityEval *
df_eval_from_entity(Arena *arena, DF_Entity *entity)
{
  DF_EntityEval *eval = push_array(arena, DF_EntityEval, 1);
  {
    DF_Entity *loc = df_entity_child_from_kind(entity, DF_EntityKind_Location);
    DF_Entity *cnd = df_entity_child_from_kind(entity, DF_EntityKind_Condition);
    String8 label_string = push_str8_copy(arena, entity->string);
    String8 loc_string = {0};
    if(loc->flags & DF_EntityFlag_HasTextPoint)
    {
      loc_string = push_str8f(arena, "%S:%I64u:%I64u", loc->string, loc->text_point.line, loc->text_point.column);
    }
    else if(loc->flags & DF_EntityFlag_HasVAddr)
    {
      loc_string = push_str8f(arena, "0x%I64x", loc->vaddr);
    }
    String8 cnd_string = push_str8_copy(arena, cnd->string);
    eval->enabled      = !entity->disabled;
    eval->hit_count    = entity->u64;
    eval->label_off    = (U64)((U8 *)label_string.str - (U8 *)eval);
    eval->location_off = (U64)((U8 *)loc_string.str - (U8 *)eval);
    eval->condition_off= (U64)((U8 *)cnd_string.str - (U8 *)eval);
  }
  return eval;
}

////////////////////////////////
//~ rjf: View Type Functions

internal B32
df_view_is_nil(DF_View *view)
{
  return (view == 0 || view == &df_nil_view);
}

internal B32
df_view_is_project_filtered(DF_View *view)
{
  B32 result = 0;
  String8 view_project = view->project_path;
  if(view_project.size != 0)
  {
    String8 current_project = df_cfg_path_from_src(D_CfgSrc_Project);
    result = !path_match_normalized(view_project, current_project);
  }
  return result;
}

internal D_Handle
df_handle_from_view(DF_View *view)
{
  D_Handle handle = d_handle_zero();
  if(!df_view_is_nil(view))
  {
    handle.u64[0] = (U64)view;
    handle.u64[1] = view->generation;
  }
  return handle;
}

internal DF_View *
df_view_from_handle(D_Handle handle)
{
  DF_View *result = (DF_View *)handle.u64[0];
  if(df_view_is_nil(result) || result->generation != handle.u64[1])
  {
    result = &df_nil_view;
  }
  return result;
}

////////////////////////////////
//~ rjf: View Spec Type Functions

internal DF_ViewKind
df_view_kind_from_string(String8 string)
{
  DF_ViewKind result = DF_ViewKind_Null;
  for(U64 idx = 0; idx < ArrayCount(df_g_gfx_view_kind_spec_info_table); idx += 1)
  {
    if(str8_match(string, df_g_gfx_view_kind_spec_info_table[idx].name, StringMatchFlag_CaseInsensitive))
    {
      result = (DF_ViewKind)idx;
      break;
    }
  }
  return result;
}

////////////////////////////////
//~ rjf: Panel Type Functions

//- rjf: basic type functions

internal B32
df_panel_is_nil(DF_Panel *panel)
{
  return panel == 0 || panel == &df_nil_panel;
}

internal D_Handle
df_handle_from_panel(DF_Panel *panel)
{
  D_Handle h = {0};
  h.u64[0] = (U64)panel;
  h.u64[1] = panel->generation;
  return h;
}

internal DF_Panel *
df_panel_from_handle(D_Handle handle)
{
  DF_Panel *panel = (DF_Panel *)handle.u64[0];
  if(panel == 0 || panel->generation != handle.u64[1])
  {
    panel = &df_nil_panel;
  }
  return panel;
}

internal UI_Key
df_ui_key_from_panel(DF_Panel *panel)
{
  UI_Key panel_key = ui_key_from_stringf(ui_key_zero(), "panel_window_%p", panel);
  return panel_key;
}

//- rjf: tree construction

internal void
df_panel_insert(DF_Panel *parent, DF_Panel *prev_child, DF_Panel *new_child)
{
  DLLInsert_NPZ(&df_nil_panel, parent->first, parent->last, prev_child, new_child, next, prev);
  parent->child_count += 1;
  new_child->parent = parent;
}

internal void
df_panel_remove(DF_Panel *parent, DF_Panel *child)
{
  DLLRemove_NPZ(&df_nil_panel, parent->first, parent->last, child, next, prev);
  child->next = child->prev = child->parent = &df_nil_panel;
  parent->child_count -= 1;
}

//- rjf: tree walk

internal DF_PanelRec
df_panel_rec_df(DF_Panel *panel, U64 sib_off, U64 child_off)
{
  DF_PanelRec rec = {0};
  if(!df_panel_is_nil(*MemberFromOffset(DF_Panel **, panel, child_off)))
  {
    rec.next = *MemberFromOffset(DF_Panel **, panel, child_off);
    rec.push_count = 1;
  }
  else if(!df_panel_is_nil(*MemberFromOffset(DF_Panel **, panel, sib_off)))
  {
    rec.next = *MemberFromOffset(DF_Panel **, panel, sib_off);
  }
  else
  {
    DF_Panel *uncle = &df_nil_panel;
    for(DF_Panel *p = panel->parent; !df_panel_is_nil(p); p = p->parent)
    {
      rec.pop_count += 1;
      if(!df_panel_is_nil(*MemberFromOffset(DF_Panel **, p, sib_off)))
      {
        uncle = *MemberFromOffset(DF_Panel **, p, sib_off);
        break;
      }
    }
    rec.next = uncle;
  }
  return rec;
}

//- rjf: panel -> rect calculations

internal Rng2F32
df_target_rect_from_panel_child(Rng2F32 parent_rect, DF_Panel *parent, DF_Panel *panel)
{
  Rng2F32 rect = parent_rect;
  if(!df_panel_is_nil(parent))
  {
    Vec2F32 parent_rect_size = dim_2f32(parent_rect);
    Axis2 axis = parent->split_axis;
    rect.p1.v[axis] = rect.p0.v[axis];
    for(DF_Panel *child = parent->first; !df_panel_is_nil(child); child = child->next)
    {
      rect.p1.v[axis] += parent_rect_size.v[axis] * child->pct_of_parent;
      if(child == panel)
      {
        break;
      }
      rect.p0.v[axis] = rect.p1.v[axis];
    }
    //rect.p0.v[axis] += parent_rect_size.v[axis] * panel->off_pct_of_parent.v[axis];
    //rect.p0.v[axis2_flip(axis)] += parent_rect_size.v[axis2_flip(axis)] * panel->off_pct_of_parent.v[axis2_flip(axis)];
  }
  rect.x0 = round_f32(rect.x0);
  rect.x1 = round_f32(rect.x1);
  rect.y0 = round_f32(rect.y0);
  rect.y1 = round_f32(rect.y1);
  return rect;
}

internal Rng2F32
df_target_rect_from_panel(Rng2F32 root_rect, DF_Panel *root, DF_Panel *panel)
{
  Temp scratch = scratch_begin(0, 0);
  
  // rjf: count ancestors
  U64 ancestor_count = 0;
  for(DF_Panel *p = panel->parent; !df_panel_is_nil(p); p = p->parent)
  {
    ancestor_count += 1;
  }
  
  // rjf: gather ancestors
  DF_Panel **ancestors = push_array(scratch.arena, DF_Panel *, ancestor_count);
  {
    U64 ancestor_idx = 0;
    for(DF_Panel *p = panel->parent; !df_panel_is_nil(p); p = p->parent)
    {
      ancestors[ancestor_idx] = p;
      ancestor_idx += 1;
    }
  }
  
  // rjf: go from highest ancestor => panel and calculate rect
  Rng2F32 parent_rect = root_rect;
  for(S64 ancestor_idx = (S64)ancestor_count-1;
      0 <= ancestor_idx && ancestor_idx < ancestor_count;
      ancestor_idx -= 1)
  {
    DF_Panel *ancestor = ancestors[ancestor_idx];
    DF_Panel *parent = ancestor->parent;
    if(!df_panel_is_nil(parent))
    {
      parent_rect = df_target_rect_from_panel_child(parent_rect, parent, ancestor);
    }
  }
  
  // rjf: calculate final rect
  Rng2F32 rect = df_target_rect_from_panel_child(parent_rect, panel->parent, panel);
  
  scratch_end(scratch);
  return rect;
}

//- rjf: view ownership insertion/removal

internal void
df_panel_insert_tab_view(DF_Panel *panel, DF_View *prev_view, DF_View *view)
{
  DLLInsert_NPZ(&df_nil_view, panel->first_tab_view, panel->last_tab_view, prev_view, view, order_next, order_prev);
  panel->tab_view_count += 1;
  if(!df_view_is_project_filtered(view))
  {
    panel->selected_tab_view = df_handle_from_view(view);
  }
}

internal void
df_panel_remove_tab_view(DF_Panel *panel, DF_View *view)
{
  if(df_view_from_handle(panel->selected_tab_view) == view)
  {
    panel->selected_tab_view = d_handle_zero();
    if(d_handle_match(d_handle_zero(), panel->selected_tab_view))
    {
      for(DF_View *v = view->order_next; !df_view_is_nil(v); v = v->order_next)
      {
        if(!df_view_is_project_filtered(v))
        {
          panel->selected_tab_view = df_handle_from_view(v);
          break;
        }
      }
    }
    if(d_handle_match(d_handle_zero(), panel->selected_tab_view))
    {
      for(DF_View *v = view->order_prev; !df_view_is_nil(v); v = v->order_prev)
      {
        if(!df_view_is_project_filtered(v))
        {
          panel->selected_tab_view = df_handle_from_view(v);
          break;
        }
      }
    }
  }
  DLLRemove_NPZ(&df_nil_view, panel->first_tab_view, panel->last_tab_view, view, order_next, order_prev);
  panel->tab_view_count -= 1;
}

internal DF_View *
df_selected_tab_from_panel(DF_Panel *panel)
{
  DF_View *view = df_view_from_handle(panel->selected_tab_view);
  if(df_view_is_project_filtered(view))
  {
    view = &df_nil_view;
  }
  return view;
}

//- rjf: icons & display strings

internal DF_IconKind
df_icon_kind_from_view(DF_View *view)
{
  DF_IconKind result = view->spec->info.icon_kind;
  return result;
}

internal DR_FancyStringList
df_title_fstrs_from_view(Arena *arena, DF_View *view, Vec4F32 primary_color, Vec4F32 secondary_color, F32 size)
{
  DR_FancyStringList result = {0};
  Temp scratch = scratch_begin(&arena, 1);
  String8 query = str8(view->query_buffer, view->query_string_size);
  String8 file_path = d_file_path_from_eval_string(scratch.arena, query);
  if(file_path.size != 0)
  {
    DR_FancyString fstr =
    {
      df_font_from_slot(DF_FontSlot_Main),
      push_str8_copy(arena, str8_skip_last_slash(file_path)),
      primary_color,
      size,
    };
    dr_fancy_string_list_push(arena, &result, &fstr);
  }
  else
  {
    DR_FancyString fstr1 =
    {
      df_font_from_slot(DF_FontSlot_Main),
      view->spec->info.display_string,
      primary_color,
      size,
    };
    dr_fancy_string_list_push(arena, &result, &fstr1);
    if(query.size != 0)
    {
      DR_FancyString fstr2 =
      {
        df_font_from_slot(DF_FontSlot_Code),
        str8_lit(" "),
        primary_color,
        size,
      };
      dr_fancy_string_list_push(arena, &result, &fstr2);
      DR_FancyString fstr3 =
      {
        df_font_from_slot(DF_FontSlot_Code),
        push_str8_copy(arena, query),
        secondary_color,
        size*0.8f,
      };
      dr_fancy_string_list_push(arena, &result, &fstr3);
    }
  }
  scratch_end(scratch);
  return result;
}

////////////////////////////////
//~ rjf: Window Type Functions

internal D_Handle
df_handle_from_window(DF_Window *window)
{
  D_Handle handle = {0};
  if(window != 0)
  {
    handle.u64[0] = (U64)window;
    handle.u64[1] = window->gen;
  }
  return handle;
}

internal DF_Window *
df_window_from_handle(D_Handle handle)
{
  DF_Window *window = (DF_Window *)handle.u64[0];
  if(window != 0 && window->gen != handle.u64[1])
  {
    window = 0;
  }
  return window;
}

////////////////////////////////
//~ rjf: Command Parameters From Context

internal B32
df_prefer_dasm_from_window(DF_Window *window)
{
  DF_Panel *panel = window->focused_panel;
  DF_View *view = df_selected_tab_from_panel(panel);
  DF_ViewKind view_kind = df_view_kind_from_string(view->spec->info.name);
  B32 result = 0;
  if(view_kind == DF_ViewKind_Disasm)
  {
    result = 1;
  }
  else if(view_kind == DF_ViewKind_Text)
  {
    result = 0;
  }
  else
  {
    B32 has_src = 0;
    B32 has_dasm = 0;
    for(DF_Panel *p = window->root_panel; !df_panel_is_nil(p); p = df_panel_rec_df_pre(p).next)
    {
      DF_View *p_view = df_selected_tab_from_panel(p);
      DF_ViewKind p_view_kind = df_view_kind_from_string(p_view->spec->info.name);
      if(p_view_kind == DF_ViewKind_Text)
      {
        has_src = 1;
      }
      if(p_view_kind == DF_ViewKind_Disasm)
      {
        has_dasm = 1;
      }
    }
    if(has_src && !has_dasm) {result = 0;}
    if(has_dasm && !has_src) {result = 1;}
  }
  return result;
}

#if 0 // TODO(rjf): @msgs

internal D_CmdParams
df_cmd_params_from_window(DF_Window *window)
{
  D_CmdParams p = d_cmd_params_zero();
  p.window = df_handle_from_window(window);
  p.panel  = df_handle_from_panel(window->focused_panel);
  p.view   = df_handle_from_view(df_selected_tab_from_panel(window->focused_panel));
  p.prefer_dasm = df_prefer_dasm_from_window(window);
  p.entity = df_regs()->thread;
  p.unwind_index = df_regs()->unwind_count;
  p.inline_depth = df_regs()->inline_depth;
  return p;
}

internal D_CmdParams
df_cmd_params_from_panel(DF_Window *window, DF_Panel *panel)
{
  D_CmdParams p = d_cmd_params_zero();
  p.window = df_handle_from_window(window);
  p.panel  = df_handle_from_panel(panel);
  p.view   = df_handle_from_view(df_selected_tab_from_panel(panel));
  p.prefer_dasm = df_prefer_dasm_from_window(window);
  p.entity = df_regs()->thread;
  p.unwind_index = df_regs()->unwind_count;
  p.inline_depth = df_regs()->inline_depth;
  return p;
}

internal D_CmdParams
df_cmd_params_from_view(DF_Window *window, DF_Panel *panel, DF_View *view)
{
  D_CmdParams p = d_cmd_params_zero();
  p.window = df_handle_from_window(window);
  p.panel  = df_handle_from_panel(panel);
  p.view   = df_handle_from_view(view);
  p.prefer_dasm = df_prefer_dasm_from_window(window);
  p.entity = df_regs()->thread;
  p.unwind_index = df_regs()->unwind_count;
  p.inline_depth = df_regs()->inline_depth;
  return p;
}

#endif

////////////////////////////////
//~ rjf: Global Cross-Window UI Interaction State Functions

internal B32
df_drag_is_active(void)
{
  return ((df_state->drag_drop_state == DF_DragDropState_Dragging) ||
          (df_state->drag_drop_state == DF_DragDropState_Dropping));
}

internal void
df_drag_begin(DF_DragDropPayload *payload)
{
  if(!df_drag_is_active())
  {
    df_state->drag_drop_state = DF_DragDropState_Dragging;
    MemoryCopyStruct(&df_drag_drop_payload, payload);
  }
}

internal B32
df_drag_drop(DF_DragDropPayload *out_payload)
{
  B32 result = 0;
  if(df_state->drag_drop_state == DF_DragDropState_Dropping)
  {
    result = 1;
    df_state->drag_drop_state = DF_DragDropState_Null;
    MemoryCopyStruct(out_payload, &df_drag_drop_payload);
    MemoryZeroStruct(&df_drag_drop_payload);
  }
  return result;
}

internal void
df_drag_kill(void)
{
  df_state->drag_drop_state = DF_DragDropState_Null;
  MemoryZeroStruct(&df_drag_drop_payload);
}

internal void
df_queue_drag_drop(void)
{
  df_state->drag_drop_state = DF_DragDropState_Dropping;
}

internal void
df_set_hover_regs(void)
{
  df_state->next_hover_regs = df_regs_copy(df_frame_arena(), df_regs());
}

internal DF_Regs *
df_get_hover_regs(void)
{
  return df_state->hover_regs;
}

////////////////////////////////
//~ rjf: Name Allocation

internal U64
df_name_bucket_idx_from_string_size(U64 size)
{
  U64 size_rounded = u64_up_to_pow2(size+1);
  size_rounded = ClampBot((1<<4), size_rounded);
  U64 bucket_idx = 0;
  switch(size_rounded)
  {
    case 1<<4: {bucket_idx = 0;}break;
    case 1<<5: {bucket_idx = 1;}break;
    case 1<<6: {bucket_idx = 2;}break;
    case 1<<7: {bucket_idx = 3;}break;
    case 1<<8: {bucket_idx = 4;}break;
    case 1<<9: {bucket_idx = 5;}break;
    case 1<<10:{bucket_idx = 6;}break;
    default:{bucket_idx = ArrayCount(df_state->free_name_chunks)-1;}break;
  }
  return bucket_idx;
}

internal String8
df_name_alloc(String8 string)
{
  if(string.size == 0) {return str8_zero();}
  U64 bucket_idx = df_name_bucket_idx_from_string_size(string.size);
  
  // rjf: loop -> find node, allocate if not there
  //
  // (we do a loop here so that all allocation logic goes through
  // the same path, such that we *always* pull off a free list,
  // rather than just using what was pushed onto an arena directly,
  // which is not undoable; the free lists we control, and are thus
  // trivially undoable)
  //
  D_NameChunkNode *node = 0;
  for(;node == 0;)
  {
    node = df_state->free_name_chunks[bucket_idx];
    
    // rjf: pull from bucket free list
    if(node != 0)
    {
      if(bucket_idx == ArrayCount(df_state->free_name_chunks)-1)
      {
        node = 0;
        D_NameChunkNode *prev = 0;
        for(D_NameChunkNode *n = df_state->free_name_chunks[bucket_idx];
            n != 0;
            prev = n, n = n->next)
        {
          if(n->size >= string.size+1)
          {
            if(prev == 0)
            {
              df_state->free_name_chunks[bucket_idx] = n->next;
            }
            else
            {
              prev->next = n->next;
            }
            node = n;
            break;
          }
        }
      }
      else
      {
        SLLStackPop(df_state->free_name_chunks[bucket_idx]);
      }
    }
    
    // rjf: no found node -> allocate new, push onto associated free list
    if(node == 0)
    {
      U64 chunk_size = 0;
      if(bucket_idx < ArrayCount(df_state->free_name_chunks)-1)
      {
        chunk_size = 1<<(bucket_idx+4);
      }
      else
      {
        chunk_size = u64_up_to_pow2(string.size);
      }
      U8 *chunk_memory = push_array(df_state->arena, U8, chunk_size);
      D_NameChunkNode *chunk = (D_NameChunkNode *)chunk_memory;
      SLLStackPush(df_state->free_name_chunks[bucket_idx], chunk);
    }
  }
  
  // rjf: fill string & return
  String8 allocated_string = str8((U8 *)node, string.size);
  MemoryCopy((U8 *)node, string.str, string.size);
  return allocated_string;
}

internal void
df_name_release(String8 string)
{
  if(string.size == 0) {return;}
  U64 bucket_idx = df_name_bucket_idx_from_string_size(string.size);
  D_NameChunkNode *node = (D_NameChunkNode *)string.str;
  node->size = u64_up_to_pow2(string.size);
  SLLStackPush(df_state->free_name_chunks[bucket_idx], node);
}

////////////////////////////////
//~ rjf: Entity State Functions

//- rjf: entity allocation + tree forming

internal DF_Entity *
df_entity_alloc(DF_Entity *parent, DF_EntityKind kind)
{
  B32 user_defined_lifetime = !!(d_entity_kind_flags_table[kind] & DF_EntityKindFlag_UserDefinedLifetime);
  U64 free_list_idx = !!user_defined_lifetime;
  if(df_entity_is_nil(parent)) { parent = df_state->entities_root; }
  
  // rjf: empty free list -> push new
  if(!df_state->entities_free[free_list_idx])
  {
    DF_Entity *entity = push_array(df_state->entities_arena, DF_Entity, 1);
    df_state->entities_count += 1;
    df_state->entities_free_count += 1;
    SLLStackPush(df_state->entities_free[free_list_idx], entity);
  }
  
  // rjf: pop new entity off free-list
  DF_Entity *entity = df_state->entities_free[free_list_idx];
  SLLStackPop(df_state->entities_free[free_list_idx]);
  df_state->entities_free_count -= 1;
  df_state->entities_active_count += 1;
  
  // rjf: zero entity
  {
    U64 gen = entity->gen;
    MemoryZeroStruct(entity);
    entity->gen = gen;
  }
  
  // rjf: set up alloc'd entity links
  entity->first = entity->last = entity->next = entity->prev = entity->parent = &d_nil_entity;
  entity->parent = parent;
  
  // rjf: stitch up parent links
  if(df_entity_is_nil(parent))
  {
    df_state->entities_root = entity;
  }
  else
  {
    DLLPushBack_NPZ(&d_nil_entity, parent->first, parent->last, entity, next, prev);
  }
  
  // rjf: fill out metadata
  entity->kind = kind;
  df_state->entities_id_gen += 1;
  entity->id = df_state->entities_id_gen;
  entity->gen += 1;
  entity->alloc_time_us = os_now_microseconds();
  entity->params_root = &md_nil_node;
  
  // rjf: initialize to deleted, record history, then "undelete" if this allocation can be undone
  if(user_defined_lifetime)
  {
    // TODO(rjf)
  }
  
  // rjf: dirtify caches
  df_state->kind_alloc_gens[kind] += 1;
  
  // rjf: log
  LogInfoNamedBlockF("new_entity")
  {
    log_infof("kind: \"%S\"\n", d_entity_kind_display_string_table[kind]);
    log_infof("id: $0x%I64x\n", entity->id);
  }
  
  return entity;
}

internal void
df_entity_mark_for_deletion(DF_Entity *entity)
{
  if(!df_entity_is_nil(entity))
  {
    entity->flags |= DF_EntityFlag_MarkedForDeletion;
  }
}

internal void
df_entity_release(DF_Entity *entity)
{
  Temp scratch = scratch_begin(0, 0);
  
  // rjf: unpack
  U64 free_list_idx = !!(d_entity_kind_flags_table[entity->kind] & DF_EntityKindFlag_UserDefinedLifetime);
  
  // rjf: release whole tree
  typedef struct Task Task;
  struct Task
  {
    Task *next;
    DF_Entity *e;
  };
  Task start_task = {0, entity};
  Task *first_task = &start_task;
  Task *last_task = &start_task;
  for(Task *task = first_task; task != 0; task = task->next)
  {
    for(DF_Entity *child = task->e->first; !df_entity_is_nil(child); child = child->next)
    {
      Task *t = push_array(scratch.arena, Task, 1);
      t->e = child;
      SLLQueuePush(first_task, last_task, t);
    }
    LogInfoNamedBlockF("end_entity")
    {
      String8 name = df_display_string_from_entity(scratch.arena, task->e);
      log_infof("kind: \"%S\"\n", d_entity_kind_display_string_table[task->e->kind]);
      log_infof("id: $0x%I64x\n", task->e->id);
      log_infof("display_string: \"%S\"\n", name);
    }
    SLLStackPush(df_state->entities_free[free_list_idx], task->e);
    df_state->entities_free_count += 1;
    df_state->entities_active_count -= 1;
    task->e->gen += 1;
    if(task->e->string.size != 0)
    {
      df_name_release(task->e->string);
    }
    if(task->e->params_arena != 0)
    {
      arena_release(task->e->params_arena);
    }
    df_state->kind_alloc_gens[task->e->kind] += 1;
  }
  
  scratch_end(scratch);
}

internal void
df_entity_change_parent(DF_Entity *entity, DF_Entity *old_parent, DF_Entity *new_parent, DF_Entity *prev_child)
{
  Assert(entity->parent == old_parent);
  Assert(prev_child->parent == old_parent || df_entity_is_nil(prev_child));
  
  // rjf: fix up links
  if(!df_entity_is_nil(old_parent))
  {
    DLLRemove_NPZ(&d_nil_entity, old_parent->first, old_parent->last, entity, next, prev);
  }
  if(!df_entity_is_nil(new_parent))
  {
    DLLInsert_NPZ(&d_nil_entity, new_parent->first, new_parent->last, prev_child, entity, next, prev);
  }
  entity->parent = new_parent;
  
  // rjf: notify
  df_state->kind_alloc_gens[entity->kind] += 1;
}

//- rjf: entity simple equipment

internal void
df_entity_equip_txt_pt(DF_Entity *entity, TxtPt point)
{
  df_require_entity_nonnil(entity, return);
  entity->text_point = point;
  entity->flags |= DF_EntityFlag_HasTextPoint;
}

internal void
df_entity_equip_entity_handle(DF_Entity *entity, D_Handle handle)
{
  df_require_entity_nonnil(entity, return);
  entity->entity_handle = handle;
  entity->flags |= DF_EntityFlag_HasEntityHandle;
}

internal void
df_entity_equip_disabled(DF_Entity *entity, B32 value)
{
  df_require_entity_nonnil(entity, return);
  entity->disabled = value;
}

internal void
df_entity_equip_u64(DF_Entity *entity, U64 u64)
{
  df_require_entity_nonnil(entity, return);
  entity->u64 = u64;
  entity->flags |= DF_EntityFlag_HasU64;
}

internal void
df_entity_equip_color_rgba(DF_Entity *entity, Vec4F32 rgba)
{
  df_require_entity_nonnil(entity, return);
  Vec3F32 rgb = v3f32(rgba.x, rgba.y, rgba.z);
  Vec3F32 hsv = hsv_from_rgb(rgb);
  Vec4F32 hsva = v4f32(hsv.x, hsv.y, hsv.z, rgba.w);
  df_entity_equip_color_hsva(entity, hsva);
}

internal void
df_entity_equip_color_hsva(DF_Entity *entity, Vec4F32 hsva)
{
  df_require_entity_nonnil(entity, return);
  entity->color_hsva = hsva;
  entity->flags |= DF_EntityFlag_HasColor;
}

internal void
df_entity_equip_cfg_src(DF_Entity *entity, D_CfgSrc cfg_src)
{
  df_require_entity_nonnil(entity, return);
  entity->cfg_src = cfg_src;
}

internal void
df_entity_equip_timestamp(DF_Entity *entity, U64 timestamp)
{
  df_require_entity_nonnil(entity, return);
  entity->timestamp = timestamp;
}

//- rjf: control layer correllation equipment

internal void
df_entity_equip_ctrl_handle(DF_Entity *entity, CTRL_Handle handle)
{
  df_require_entity_nonnil(entity, return);
  entity->ctrl_handle = handle;
  entity->flags |= DF_EntityFlag_HasCtrlHandle;
}

internal void
df_entity_equip_arch(DF_Entity *entity, Arch arch)
{
  df_require_entity_nonnil(entity, return);
  entity->arch = arch;
  entity->flags |= DF_EntityFlag_HasArch;
}

internal void
df_entity_equip_ctrl_id(DF_Entity *entity, U32 id)
{
  df_require_entity_nonnil(entity, return);
  entity->ctrl_id = id;
  entity->flags |= DF_EntityFlag_HasCtrlID;
}

internal void
df_entity_equip_stack_base(DF_Entity *entity, U64 stack_base)
{
  df_require_entity_nonnil(entity, return);
  entity->stack_base = stack_base;
  entity->flags |= DF_EntityFlag_HasStackBase;
}

internal void
df_entity_equip_vaddr_rng(DF_Entity *entity, Rng1U64 range)
{
  df_require_entity_nonnil(entity, return);
  entity->vaddr_rng = range;
  entity->flags |= DF_EntityFlag_HasVAddrRng;
}

internal void
df_entity_equip_vaddr(DF_Entity *entity, U64 vaddr)
{
  df_require_entity_nonnil(entity, return);
  entity->vaddr = vaddr;
  entity->flags |= DF_EntityFlag_HasVAddr;
}

//- rjf: name equipment

internal void
df_entity_equip_name(DF_Entity *entity, String8 name)
{
  df_require_entity_nonnil(entity, return);
  if(entity->string.size != 0)
  {
    df_name_release(entity->string);
  }
  if(name.size != 0)
  {
    entity->string = df_name_alloc(name);
  }
  else
  {
    entity->string = str8_zero();
  }
}

//- rjf: file path map override lookups

internal String8List
d_possible_overrides_from_file_path(Arena *arena, String8 file_path)
{
  // NOTE(rjf): This path, given some target file path, scans all file path map
  // overrides, and collects the set of file paths which could've redirected
  // to the target file path given the set of file path maps.
  //
  // For example, if I have a rule saying D:/devel/ maps to C:/devel/, and I
  // feed in C:/devel/foo/bar.txt, then this path will construct
  // D:/devel/foo/bar.txt, as a possible option.
  //
  // It will also preserve C:/devel/foo/bar.txt in the resultant list, so that
  // overrideless files still work through this path, and both redirected
  // files and non-redirected files can go through the same path.
  //
  String8List result = {0};
  str8_list_push(arena, &result, file_path);
  Temp scratch = scratch_begin(&arena, 1);
  PathStyle pth_style = PathStyle_Relative;
  String8List pth_parts = path_normalized_list_from_string(scratch.arena, file_path, &pth_style);
  {
    DF_EntityList links = d_query_cached_entity_list_with_kind(DF_EntityKind_FilePathMap);
    for(DF_EntityNode *n = links.first; n != 0; n = n->next)
    {
      //- rjf: unpack link
      DF_Entity *link = n->entity;
      DF_Entity *src = df_entity_child_from_kind(link, DF_EntityKind_Source);
      DF_Entity *dst = df_entity_child_from_kind(link, DF_EntityKind_Dest);
      PathStyle src_style = PathStyle_Relative;
      PathStyle dst_style = PathStyle_Relative;
      String8List src_parts = path_normalized_list_from_string(scratch.arena, src->string, &src_style);
      String8List dst_parts = path_normalized_list_from_string(scratch.arena, dst->string, &dst_style);
      
      //- rjf: determine if this link can possibly redirect to the target file path
      B32 dst_redirects_to_pth = 0;
      String8Node *non_redirected_pth_first = 0;
      if(dst_style == pth_style && dst_parts.first != 0 && pth_parts.first != 0)
      {
        dst_redirects_to_pth = 1;
        String8Node *dst_n = dst_parts.first;
        String8Node *pth_n = pth_parts.first;
        for(;dst_n != 0 && pth_n != 0; dst_n = dst_n->next, pth_n = pth_n->next)
        {
          if(!str8_match(dst_n->string, pth_n->string, StringMatchFlag_CaseInsensitive))
          {
            dst_redirects_to_pth = 0;
            break;
          }
          non_redirected_pth_first = pth_n->next;
        }
      }
      
      //- rjf: if this link can redirect to this path via `src` -> `dst`, compute
      // possible full source path, by taking `src` and appending non-redirected
      // suffix (which did not show up in `dst`)
      if(dst_redirects_to_pth)
      {
        String8List candidate_parts = src_parts;
        for(String8Node *p = non_redirected_pth_first; p != 0; p = p->next)
        {
          str8_list_push(scratch.arena, &candidate_parts, p->string);
        }
        StringJoin join = {0};
        join.sep = str8_lit("/");
        String8 candidate_path = str8_list_join(arena, &candidate_parts, 0);
        str8_list_push(arena, &result, candidate_path);
      }
    }
  }
  scratch_end(scratch);
  return result;
}

//- rjf: top-level state queries

internal DF_Entity *
df_entity_root(void)
{
  return df_state->entities_root;
}

internal DF_EntityList
df_push_entity_list_with_kind(Arena *arena, DF_EntityKind kind)
{
  ProfBeginFunction();
  DF_EntityList result = {0};
  for(DF_Entity *entity = df_state->entities_root;
      !df_entity_is_nil(entity);
      entity = df_entity_rec_depth_first_pre(entity, &d_nil_entity).next)
  {
    if(entity->kind == kind)
    {
      df_entity_list_push(arena, &result, entity);
    }
  }
  ProfEnd();
  return result;
}

internal DF_Entity *
df_entity_from_id(DF_EntityID id)
{
  DF_Entity *result = &d_nil_entity;
  for(DF_Entity *e = df_entity_root();
      !df_entity_is_nil(e);
      e = df_entity_rec_depth_first_pre(e, &d_nil_entity).next)
  {
    if(e->id == id)
    {
      result = e;
      break;
    }
  }
  return result;
}

internal DF_Entity *
df_machine_entity_from_machine_id(CTRL_MachineID machine_id)
{
  DF_Entity *result = &d_nil_entity;
  for(DF_Entity *e = df_entity_root();
      !df_entity_is_nil(e);
      e = df_entity_rec_depth_first_pre(e, &d_nil_entity).next)
  {
    if(e->kind == DF_EntityKind_Machine && e->ctrl_handle.machine_id == machine_id)
    {
      result = e;
      break;
    }
  }
  if(df_entity_is_nil(result))
  {
    result = df_entity_alloc(df_entity_root(), DF_EntityKind_Machine);
    df_entity_equip_ctrl_handle(result, ctrl_handle_make(machine_id, dmn_handle_zero()));
  }
  return result;
}

internal DF_Entity *
df_entity_from_ctrl_handle(CTRL_Handle handle)
{
  DF_Entity *result = &d_nil_entity;
  if(handle.machine_id != 0 || handle.dmn_handle.u64[0] != 0)
  {
    for(DF_Entity *e = df_entity_root();
        !df_entity_is_nil(e);
        e = df_entity_rec_depth_first_pre(e, &d_nil_entity).next)
    {
      if(e->flags & DF_EntityFlag_HasCtrlHandle &&
         ctrl_handle_match(e->ctrl_handle, handle))
      {
        result = e;
        break;
      }
    }
  }
  return result;
}

internal DF_Entity *
df_entity_from_ctrl_id(CTRL_MachineID machine_id, U32 id)
{
  DF_Entity *result = &d_nil_entity;
  if(id != 0)
  {
    for(DF_Entity *e = df_entity_root();
        !df_entity_is_nil(e);
        e = df_entity_rec_depth_first_pre(e, &d_nil_entity).next)
    {
      if(e->flags & DF_EntityFlag_HasCtrlHandle &&
         e->flags & DF_EntityFlag_HasCtrlID &&
         e->ctrl_handle.machine_id == machine_id &&
         e->ctrl_id == id)
      {
        result = e;
        break;
      }
    }
  }
  return result;
}

internal DF_Entity *
df_entity_from_name_and_kind(String8 string, DF_EntityKind kind)
{
  DF_Entity *result = &d_nil_entity;
  DF_EntityList all_of_this_kind = d_query_cached_entity_list_with_kind(kind);
  for(DF_EntityNode *n = all_of_this_kind.first; n != 0; n = n->next)
  {
    if(str8_match(n->entity->string, string, 0))
    {
      result = n->entity;
      break;
    }
  }
  return result;
}

////////////////////////////////
//~ rjf: Evaluation Spaces

//- rjf: ctrl entity <-> eval space

internal CTRL_Entity *
d_ctrl_entity_from_eval_space(E_Space space)
{
  CTRL_Entity *entity = &ctrl_entity_nil;
  // TODO(rjf): @msgs
  return entity;
}

internal E_Space
d_eval_space_from_ctrl_entity(CTRL_Entity *entity)
{
  E_Space space = {0};
  // TODO(rjf): @msgs
  return space;
}

//- rjf: entity <-> eval space

internal DF_Entity *
d_entity_from_eval_space(E_Space space)
{
  DF_Entity *entity = &d_nil_entity;
  if(space.u64_0 != 0)
  {
    entity = (DF_Entity *)space.u64_0;
  }
  return entity;
}

internal E_Space
d_eval_space_from_entity(DF_Entity *entity)
{
  E_Space space = {0};
  space.u64_0 = (U64)entity;
  return space;
}

//- rjf: eval space reads/writes

internal B32
d_eval_space_read(void *u, E_Space space, void *out, Rng1U64 range)
{
  B32 result = 0;
  switch(space.kind)
  {
    case E_SpaceKind_FileSystem:
    {
      
    }break;
    case DF_EvalSpaceKind_CtrlEntity:
    {
      
    }break;
    case DF_EvalSpaceKind_CfgEntity:
    {
      
    }break;
  }
  
#if 0 // TODO(rjf): @msgs
  DF_Entity *entity = d_entity_from_eval_space(space);
  switch(entity->kind)
  {
    //- rjf: nil-space -> fall back to file system
    case DF_EntityKind_Nil:
    {
      U128 key = space.u128;
      U128 hash = hs_hash_from_key(key, 0);
      HS_Scope *scope = hs_scope_open();
      {
        String8 data = hs_data_from_hash(scope, hash);
        Rng1U64 legal_range = r1u64(0, data.size);
        Rng1U64 read_range = intersect_1u64(range, legal_range);
        if(read_range.min < read_range.max)
        {
          result = 1;
          MemoryCopy(out, data.str + read_range.min, dim_1u64(read_range));
        }
      }
      hs_scope_close(scope);
    }break;
    
    //- rjf: default -> evaluating a debugger entity; read from entity POD evaluation
    default:
    {
      Temp scratch = scratch_begin(0, 0);
      arena_push(scratch.arena, 0, 64);
      U64 pos_min = arena_pos(scratch.arena);
      DF_EntityEval *eval = df_eval_from_entity(scratch.arena, entity);
      U64 pos_opl = arena_pos(scratch.arena);
      Rng1U64 legal_range = r1u64(0, pos_opl-pos_min);
      if(contains_1u64(legal_range, range.min))
      {
        result = 1;
        U64 range_dim = dim_1u64(range);
        U64 bytes_to_read = Min(range_dim, (legal_range.max - range.min));
        MemoryCopy(out, ((U8 *)eval) + range.min, bytes_to_read);
        if(bytes_to_read < range_dim)
        {
          MemoryZero((U8 *)out + bytes_to_read, range_dim - bytes_to_read);
        }
      }
      scratch_end(scratch);
    }break;
    
    //- rjf: process -> reading process memory
    case DF_EntityKind_Process:
    {
      Temp scratch = scratch_begin(0, 0);
      CTRL_ProcessMemorySlice slice = ctrl_query_cached_data_from_process_vaddr_range(scratch.arena, entity->ctrl_handle, range, d_state->frame_eval_memread_endt_us);
      String8 data = slice.data;
      if(data.size == dim_1u64(range))
      {
        result = 1;
        MemoryCopy(out, data.str, data.size);
      }
      scratch_end(scratch);
    }break;
    
    //- rjf: thread -> reading from thread register block
    case DF_EntityKind_Thread:
    {
      Temp scratch = scratch_begin(0, 0);
      CTRL_Unwind unwind = d_query_cached_unwind_from_thread(entity);
      U64 frame_idx = e_interpret_ctx->reg_unwind_count;
      if(frame_idx < unwind.frames.count)
      {
        CTRL_UnwindFrame *f = &unwind.frames.v[frame_idx];
        U64 regs_size = regs_block_size_from_arch(e_interpret_ctx->reg_arch);
        Rng1U64 legal_range = r1u64(0, regs_size);
        Rng1U64 read_range = intersect_1u64(legal_range, range);
        U64 read_size = dim_1u64(read_range);
        MemoryCopy(out, (U8 *)f->regs + read_range.min, read_size);
        result = (read_size == dim_1u64(range));
      }
      scratch_end(scratch);
    }break;
  }
#endif
  return result;
}

internal B32
d_eval_space_write(void *u, E_Space space, void *in, Rng1U64 range)
{
  B32 result = 0;
#if 0 // TODO(rjf): @msgs
  DF_Entity *entity = d_entity_from_eval_space(space);
  switch(entity->kind)
  {
    //- rjf: default -> making commits to entity evaluation
    default:
    {
      Temp scratch = scratch_begin(0, 0);
      DF_EntityEval *eval = df_eval_from_entity(scratch.arena, entity);
      U64 range_dim = dim_1u64(range);
      if(range.min == OffsetOf(DF_EntityEval, enabled) &&
         range_dim >= 1)
      {
        result = 1;
        B32 new_enabled = !!((U8 *)in)[0];
        df_entity_equip_disabled(entity, !new_enabled);
      }
      else if(range.min == eval->label_off &&
              range_dim >= 1)
      {
        result = 1;
        String8 new_name = str8_cstring_capped((U8 *)in, (U8 *)in+range_dim);
        df_entity_equip_name(entity, new_name);
      }
      else if(range.min == eval->condition_off &&
              range_dim >= 1)
      {
        result = 1;
        DF_Entity *condition = df_entity_child_from_kind(entity, DF_EntityKind_Condition);
        if(df_entity_is_nil(condition))
        {
          condition = df_entity_alloc(entity, DF_EntityKind_Condition);
        }
        String8 new_name = str8_cstring_capped((U8 *)in, (U8 *)in+range_dim);
        df_entity_equip_name(condition, new_name);
      }
      scratch_end(scratch);
    }break;
    
    //- rjf: process -> commit to process memory
    case DF_EntityKind_Process:
    {
      result = ctrl_process_write(entity->ctrl_handle, range, in);
    }break;
    
    //- rjf: thread -> commit to thread's register block
    case DF_EntityKind_Thread:
    {
      CTRL_Unwind unwind = d_query_cached_unwind_from_thread(entity);
      U64 frame_idx = 0;
      if(frame_idx < unwind.frames.count)
      {
        Temp scratch = scratch_begin(0, 0);
        U64 regs_size = regs_block_size_from_arch(d_arch_from_entity(entity));
        Rng1U64 legal_range = r1u64(0, regs_size);
        Rng1U64 write_range = intersect_1u64(legal_range, range);
        U64 write_size = dim_1u64(write_range);
        CTRL_UnwindFrame *f = &unwind.frames.v[frame_idx];
        void *new_regs = push_array(scratch.arena, U8, regs_size);
        MemoryCopy(new_regs, f->regs, regs_size);
        MemoryCopy((U8 *)new_regs + write_range.min, in, write_size);
        result = ctrl_thread_write_reg_block(entity->ctrl_handle, new_regs);
        scratch_end(scratch);
      }
    }break;
  }
#endif
  return result;
}

//- rjf: asynchronous streamed reads -> hashes from spaces

internal U128
d_key_from_eval_space_range(E_Space space, Rng1U64 range, B32 zero_terminated)
{
  U128 result = {0};
  DF_Entity *entity = d_entity_from_eval_space(space);
  switch(entity->kind)
  {
    default:{}break;
    
    //- rjf: nil space -> filesystem key encoded inside of `space`
    case DF_EntityKind_Nil:
    {
      result = space.u128;
    }break;
    
    //- rjf: process space -> query 
    case DF_EntityKind_Process:
    {
      result = ctrl_hash_store_key_from_process_vaddr_range(entity->ctrl_handle, range, zero_terminated);
    }break;
  }
  return result;
}

//- rjf: space -> entire range

internal Rng1U64
d_whole_range_from_eval_space(E_Space space)
{
  // TODO(rjf): @msgs
  Rng1U64 result = r1u64(0, 0);
#if 0
  DF_Entity *entity = d_entity_from_eval_space(space);
  switch(entity->kind)
  {
    default:{}break;
    
    //- rjf: nil space -> filesystem key encoded inside of `space`
    case DF_EntityKind_Nil:
    {
      HS_Scope *scope = hs_scope_open();
      U128 hash = {0};
      for(U64 idx = 0; idx < 2; idx += 1)
      {
        hash = hs_hash_from_key(space.u128, idx);
        if(!u128_match(hash, u128_zero()))
        {
          break;
        }
      }
      String8 data = hs_data_from_hash(scope, hash);
      result = r1u64(0, data.size);
      hs_scope_close(scope);
    }break;
    case DF_EntityKind_Process:
    {
      result = r1u64(0, 0x7FFFFFFFFFFFull);
    }break;
  }
#endif
  return result;
}

////////////////////////////////
//~ rjf: Evaluation View Visualization & Interaction

//- rjf: writing values back to child processes

internal B32
d_commit_eval_value_string(E_Eval dst_eval, String8 string)
{
  B32 result = 0;
  if(dst_eval.mode == E_Mode_Offset)
  {
    Temp scratch = scratch_begin(0, 0);
    E_TypeKey type_key = e_type_unwrap(dst_eval.type_key);
    E_TypeKind type_kind = e_type_kind_from_key(type_key);
    E_TypeKey direct_type_key = e_type_unwrap(e_type_direct_from_key(e_type_unwrap(dst_eval.type_key)));
    E_TypeKind direct_type_kind = e_type_kind_from_key(direct_type_key);
    String8 commit_data = {0};
    B32 commit_at_ptr_dest = 0;
    if(E_TypeKind_FirstBasic <= type_kind && type_kind <= E_TypeKind_LastBasic)
    {
      E_Eval src_eval = e_eval_from_string(scratch.arena, string);
      commit_data = push_str8_copy(scratch.arena, str8_struct(&src_eval.value));
      commit_data.size = Min(commit_data.size, e_type_byte_size_from_key(type_key));
    }
    else if(type_kind == E_TypeKind_Ptr || type_kind == E_TypeKind_Array)
    {
      E_Eval src_eval = e_eval_from_string(scratch.arena, string);
      E_Eval src_eval_value = e_value_eval_from_eval(src_eval);
      E_TypeKind src_eval_value_type_kind = e_type_kind_from_key(src_eval_value.type_key);
      if(type_kind == E_TypeKind_Ptr &&
         (e_type_kind_is_pointer_or_ref(src_eval_value_type_kind) ||
          e_type_kind_is_integer(src_eval_value_type_kind)) &&
         src_eval_value.value.u64 != 0 && src_eval_value.mode == E_Mode_Value)
      {
        commit_data = push_str8_copy(scratch.arena, str8_struct(&src_eval.value));
        commit_data.size = Min(commit_data.size, e_type_byte_size_from_key(type_key));
      }
      else if(direct_type_kind == E_TypeKind_Char8 ||
              direct_type_kind == E_TypeKind_UChar8 ||
              e_type_kind_is_integer(direct_type_kind))
      {
        if(string.size >= 1 && string.str[0] == '"')
        {
          string = str8_skip(string, 1);
        }
        if(string.size >= 1 && string.str[string.size-1] == '"')
        {
          string = str8_chop(string, 1);
        }
        commit_data = e_raw_from_escaped_string(scratch.arena, string);
        commit_data.size += 1;
        if(type_kind == E_TypeKind_Ptr)
        {
          commit_at_ptr_dest = 1;
        }
      }
    }
    if(commit_data.size != 0 && e_type_byte_size_from_key(type_key) != 0)
    {
      U64 dst_offset = dst_eval.value.u64;
      if(dst_eval.mode == E_Mode_Offset && commit_at_ptr_dest)
      {
        E_Eval dst_value_eval = e_value_eval_from_eval(dst_eval);
        dst_offset = dst_value_eval.value.u64;
      }
      result = e_space_write(dst_eval.space, commit_data.str, r1u64(dst_offset, dst_offset + commit_data.size));
    }
    scratch_end(scratch);
  }
  return result;
}

//- rjf: view rule config tree info extraction

internal U64
d_base_offset_from_eval(E_Eval eval)
{
  if(e_type_kind_is_pointer_or_ref(e_type_kind_from_key(eval.type_key)))
  {
    eval = e_value_eval_from_eval(eval);
  }
  return eval.value.u64;
}

internal E_Value
d_value_from_params_key(MD_Node *params, String8 key)
{
  Temp scratch = scratch_begin(0, 0);
  MD_Node *key_node = md_child_from_string(params, key, 0);
  String8 expr = md_string_from_children(scratch.arena, key_node);
  E_Eval eval = e_eval_from_string(scratch.arena, expr);
  E_Eval value_eval = e_value_eval_from_eval(eval);
  scratch_end(scratch);
  return value_eval.value;
}

internal Rng1U64
d_range_from_eval_params(E_Eval eval, MD_Node *params)
{
  Temp scratch = scratch_begin(0, 0);
  U64 size = d_value_from_params_key(params, str8_lit("size")).u64;
  E_TypeKey type_key = e_type_unwrap(eval.type_key);
  E_TypeKind type_kind = e_type_kind_from_key(type_key);
  E_TypeKey direct_type_key = e_type_unwrap(e_type_direct_from_key(eval.type_key));
  E_TypeKind direct_type_kind = e_type_kind_from_key(direct_type_key);
  if(size == 0 && e_type_kind_is_pointer_or_ref(type_kind) && (direct_type_kind == E_TypeKind_Struct ||
                                                               direct_type_kind == E_TypeKind_Union ||
                                                               direct_type_kind == E_TypeKind_Class ||
                                                               direct_type_kind == E_TypeKind_Array))
  {
    size = e_type_byte_size_from_key(e_type_direct_from_key(e_type_unwrap(eval.type_key)));
  }
  if(size == 0 && eval.mode == E_Mode_Offset && (type_kind == E_TypeKind_Struct ||
                                                 type_kind == E_TypeKind_Union ||
                                                 type_kind == E_TypeKind_Class ||
                                                 type_kind == E_TypeKind_Array))
  {
    size = e_type_byte_size_from_key(e_type_unwrap(eval.type_key));
  }
  if(size == 0)
  {
    size = 16384;
  }
  Rng1U64 result = {0};
  result.min = d_base_offset_from_eval(eval);
  result.max = result.min + size;
  scratch_end(scratch);
  return result;
}

internal TXT_LangKind
d_lang_kind_from_eval_params(E_Eval eval, MD_Node *params)
{
  TXT_LangKind lang_kind = TXT_LangKind_Null;
  if(eval.expr->kind == E_ExprKind_LeafFilePath)
  {
    lang_kind = txt_lang_kind_from_extension(str8_skip_last_dot(eval.expr->string));
  }
  else
  {
    MD_Node *lang_node = md_child_from_string(params, str8_lit("lang"), 0);
    String8 lang_kind_string = lang_node->first->string;
    lang_kind = txt_lang_kind_from_extension(lang_kind_string);
  }
  return lang_kind;
}

internal Arch
d_arch_from_eval_params(E_Eval eval, MD_Node *params)
{
  Arch arch = Arch_Null;
  MD_Node *arch_node = md_child_from_string(params, str8_lit("arch"), 0);
  String8 arch_kind_string = arch_node->first->string;
  if(str8_match(arch_kind_string, str8_lit("x64"), StringMatchFlag_CaseInsensitive))
  {
    arch = Arch_x64;
  }
  return arch;
}

internal Vec2S32
d_dim2s32_from_eval_params(E_Eval eval, MD_Node *params)
{
  Vec2S32 dim = v2s32(1, 1);
  {
    dim.x = d_value_from_params_key(params, str8_lit("w")).s32;
    dim.y = d_value_from_params_key(params, str8_lit("h")).s32;
  }
  return dim;
}

internal R_Tex2DFormat
d_tex2dformat_from_eval_params(E_Eval eval, MD_Node *params)
{
  R_Tex2DFormat result = R_Tex2DFormat_RGBA8;
  {
    MD_Node *fmt_node = md_child_from_string(params, str8_lit("fmt"), 0);
    for(EachNonZeroEnumVal(R_Tex2DFormat, fmt))
    {
      if(str8_match(r_tex2d_format_display_string_table[fmt], fmt_node->first->string, StringMatchFlag_CaseInsensitive))
      {
        result = fmt;
        break;
      }
    }
  }
  return result;
}

//- rjf: eval -> entity

internal DF_Entity *
d_entity_from_eval_string(String8 string)
{
  DF_Entity *entity = &d_nil_entity;
  {
    Temp scratch = scratch_begin(0, 0);
    E_Eval eval = e_eval_from_string(scratch.arena, string);
    entity = d_entity_from_eval_space(eval.space);
    scratch_end(scratch);
  }
  return entity;
}

internal String8
d_eval_string_from_entity(Arena *arena, DF_Entity *entity)
{
  String8 eval_string = push_str8f(arena, "macro:`$%I64u`", entity->id);
  return eval_string;
}

//- rjf: eval <-> file path

internal String8
d_file_path_from_eval_string(Arena *arena, String8 string)
{
  String8 result = {0};
  {
    Temp scratch = scratch_begin(&arena, 1);
    E_Eval eval = e_eval_from_string(scratch.arena, string);
    if(eval.expr->kind == E_ExprKind_LeafFilePath)
    {
      result = d_cfg_raw_from_escaped_string(arena, eval.expr->string);
    }
    scratch_end(scratch);
  }
  return result;
}

internal String8
d_eval_string_from_file_path(Arena *arena, String8 string)
{
  Temp scratch = scratch_begin(&arena, 1);
  String8 string_escaped = d_cfg_escaped_from_raw_string(scratch.arena, string);
  String8 result = push_str8f(arena, "file:\"%S\"", string_escaped);
  scratch_end(scratch);
  return result;
}

////////////////////////////////
//~ rjf: View Spec State Functions

internal void
df_register_view_specs(DF_ViewSpecInfoArray specs)
{
  for(U64 idx = 0; idx < specs.count; idx += 1)
  {
    DF_ViewSpecInfo *src_info = &specs.v[idx];
    U64 hash = d_hash_from_string(src_info->name);
    U64 slot_idx = hash%df_state->view_spec_table_size;
    DF_ViewSpec *spec = push_array(df_state->arena, DF_ViewSpec, 1);
    SLLStackPush_N(df_state->view_spec_table[slot_idx], spec, hash_next);
    MemoryCopyStruct(&spec->info, src_info);
    spec->info.name = push_str8_copy(df_state->arena, spec->info.name);
    spec->info.display_string = push_str8_copy(df_state->arena, spec->info.display_string);
  }
}

internal DF_ViewSpec *
df_view_spec_from_string(String8 string)
{
  DF_ViewSpec *spec = &df_nil_view_spec;
  U64 hash = d_hash_from_string(string);
  U64 slot_idx = hash%df_state->view_spec_table_size;
  for(DF_ViewSpec *s = df_state->view_spec_table[slot_idx];
      s != 0;
      s = s->hash_next)
  {
    if(str8_match(s->info.name, string, 0))
    {
      spec = s;
      break;
    }
  }
  return spec;
}

internal DF_ViewSpec *
df_view_spec_from_kind(DF_ViewKind kind)
{
  DF_ViewSpec *spec = df_view_spec_from_string(df_g_gfx_view_kind_spec_info_table[kind].name);
  return spec;
}

////////////////////////////////
//~ rjf: View Rule Spec State Functions

internal void
df_register_view_rule_specs(DF_ViewRuleSpecInfoArray specs)
{
  for(U64 idx = 0; idx < specs.count; idx += 1)
  {
    // rjf: extract info from array slot
    DF_ViewRuleSpecInfo *info = &specs.v[idx];
    
    // rjf: skip empties
    if(info->string.size == 0)
    {
      continue;
    }
    
    // rjf: determine hash/slot
    U64 hash = d_hash_from_string(info->string);
    U64 slot_idx = hash%df_state->view_rule_spec_table_size;
    
    // rjf: allocate node & push
    DF_ViewRuleSpec *spec = push_array(df_state->arena, DF_ViewRuleSpec, 1);
    SLLStackPush_N(df_state->view_rule_spec_table[slot_idx], spec, hash_next);
    
    // rjf: fill node
    DF_ViewRuleSpecInfo *info_copy = &spec->info;
    MemoryCopyStruct(info_copy, info);
    info_copy->string         = push_str8_copy(df_state->arena, info->string);
  }
}

internal DF_ViewRuleSpec *
df_view_rule_spec_from_string(String8 string)
{
  DF_ViewRuleSpec *spec = &df_nil_view_rule_spec;
  {
    U64 hash = d_hash_from_string(string);
    U64 slot_idx = hash%df_state->view_rule_spec_table_size;
    for(DF_ViewRuleSpec *s = df_state->view_rule_spec_table[slot_idx]; s != 0; s = s->hash_next)
    {
      if(str8_match(string, s->info.string, 0))
      {
        spec = s;
        break;
      }
    }
  }
  return spec;
}

////////////////////////////////
//~ rjf: View State Functions

//- rjf: allocation/releasing

internal DF_View *
df_view_alloc(void)
{
  // rjf: allocate
  DF_View *view = df_state->free_view;
  {
    if(!df_view_is_nil(view))
    {
      df_state->free_view_count -= 1;
      SLLStackPop_N(df_state->free_view, alloc_next);
      U64 generation = view->generation;
      MemoryZeroStruct(view);
      view->generation = generation;
    }
    else
    {
      view = push_array(df_state->arena, DF_View, 1);
    }
    view->generation += 1;
  }
  
  // rjf: initialize
  view->arena = arena_alloc();
  view->spec = &df_nil_view_spec;
  view->project_path_arena = arena_alloc();
  view->project_path = str8_zero();
  for(U64 idx = 0; idx < ArrayCount(view->params_arenas); idx += 1)
  {
    view->params_arenas[idx] = arena_alloc();
    view->params_roots[idx] = &md_nil_node;
  }
  view->query_cursor = view->query_mark = txt_pt(1, 1);
  view->query_string_size = 0;
  df_state->allocated_view_count += 1;
  DLLPushBack_NPZ(&df_nil_view, df_state->first_view, df_state->last_view, view, alloc_next, alloc_prev);
  return view;
}

internal void
df_view_release(DF_View *view)
{
  DLLRemove_NPZ(&df_nil_view, df_state->first_view, df_state->last_view, view, alloc_next, alloc_prev);
  SLLStackPush_N(df_state->free_view, view, alloc_next);
  for(DF_View *tchild = view->first_transient, *next = 0; !df_view_is_nil(tchild); tchild = next)
  {
    next = tchild->order_next;
    df_view_release(tchild);
  }
  view->first_transient = view->last_transient = &df_nil_view;
  view->transient_view_slots_count = 0;
  view->transient_view_slots = 0;
  for(DF_ArenaExt *ext = view->first_arena_ext; ext != 0; ext = ext->next)
  {
    arena_release(ext->arena);
  }
  view->first_arena_ext = view->last_arena_ext = 0;
  arena_release(view->project_path_arena);
  for(U64 idx = 0; idx < ArrayCount(view->params_arenas); idx += 1)
  {
    arena_release(view->params_arenas[idx]);
  }
  arena_release(view->arena);
  view->generation += 1;
  df_state->allocated_view_count -= 1;
  df_state->free_view_count += 1;
}

//- rjf: equipment

internal void
df_view_equip_spec(DF_View *view, DF_ViewSpec *spec, String8 query, MD_Node *params)
{
  // rjf: fill params tree
  for(U64 idx = 0; idx < ArrayCount(view->params_arenas); idx += 1)
  {
    arena_clear(view->params_arenas[idx]);
  }
  view->params_roots[0] = md_tree_copy(view->params_arenas[0], params);
  view->params_write_gen = view->params_read_gen = 0;
  
  // rjf: fill query buffer
  df_view_equip_query(view, query);
  
  // rjf: initialize state for new view spec
  DF_ViewSetupFunctionType *view_setup = spec->info.setup_hook;
  {
    for(DF_ArenaExt *ext = view->first_arena_ext; ext != 0; ext = ext->next)
    {
      arena_release(ext->arena);
    }
    for(DF_View *tchild = view->first_transient, *next = 0; !df_view_is_nil(tchild); tchild = next)
    {
      next = tchild->order_next;
      df_view_release(tchild);
    }
    view->first_transient = view->last_transient = &df_nil_view;
    view->first_arena_ext = view->last_arena_ext = 0;
    view->transient_view_slots_count = 0;
    view->transient_view_slots = 0;
    arena_clear(view->arena);
    view->user_data = 0;
  }
  MemoryZeroStruct(&view->scroll_pos);
  view->spec = spec;
  if(spec->info.flags & DF_ViewSpecFlag_ProjectSpecific)
  {
    arena_clear(view->project_path_arena);
    view->project_path = push_str8_copy(view->project_path_arena, df_cfg_path_from_src(D_CfgSrc_Project));
  }
  else
  {
    MemoryZeroStruct(&view->project_path);
  }
  view->is_filtering = 0;
  view->is_filtering_t = 0;
  view_setup(view, view->params_roots[view->params_read_gen%ArrayCount(view->params_roots)], str8(view->query_buffer, view->query_string_size));
}

internal void
df_view_equip_query(DF_View *view, String8 query)
{
  view->query_string_size = Min(sizeof(view->query_buffer), query.size);
  MemoryCopy(view->query_buffer, query.str, view->query_string_size);
  view->query_cursor = view->query_mark = txt_pt(1, query.size+1);
}

internal void
df_view_equip_loading_info(DF_View *view, B32 is_loading, U64 progress_v, U64 progress_target)
{
  view->loading_t_target = (F32)!!is_loading;
  view->loading_progress_v = progress_v;
  view->loading_progress_v_target = progress_target;
}

//- rjf: user state extensions

internal void *
df_view_get_or_push_user_state(DF_View *view, U64 size)
{
  void *result = view->user_data;
  if(result == 0)
  {
    view->user_data = result = push_array(view->arena, U8, size);
  }
  return result;
}

internal Arena *
df_view_push_arena_ext(DF_View *view)
{
  DF_ArenaExt *ext = push_array(view->arena, DF_ArenaExt, 1);
  ext->arena = arena_alloc();
  SLLQueuePush(view->first_arena_ext, view->last_arena_ext, ext);
  return ext->arena;
}

//- rjf: param saving

internal void
df_view_store_param(DF_View *view, String8 key, String8 value)
{
  B32 new_copy = 0;
  if(view->params_write_gen == view->params_read_gen)
  {
    view->params_write_gen += 1;
    new_copy = 1;
  }
  Arena *new_params_arena = view->params_arenas[view->params_write_gen%ArrayCount(view->params_arenas)];
  if(new_copy)
  {
    arena_clear(new_params_arena);
    view->params_roots[view->params_write_gen%ArrayCount(view->params_arenas)] = md_tree_copy(new_params_arena, view->params_roots[view->params_read_gen%ArrayCount(view->params_arenas)]);
  }
  MD_Node *new_params_root = view->params_roots[view->params_write_gen%ArrayCount(view->params_arenas)];
  if(md_node_is_nil(new_params_root))
  {
    new_params_root = view->params_roots[view->params_write_gen%ArrayCount(view->params_arenas)] = md_push_node(new_params_arena, MD_NodeKind_Main, 0, str8_zero(), str8_zero(), 0);
  }
  MD_Node *key_node = md_child_from_string(new_params_root, key, 0);
  if(md_node_is_nil(key_node))
  {
    String8 key_copy = push_str8_copy(new_params_arena, key);
    key_node = md_push_node(new_params_arena, MD_NodeKind_Main, MD_NodeFlag_Identifier, key_copy, key_copy, 0);
    md_node_push_child(new_params_root, key_node);
  }
  key_node->first = key_node->last = &md_nil_node;
  String8 value_copy = push_str8_copy(new_params_arena, value);
  MD_TokenizeResult value_tokenize = md_tokenize_from_text(new_params_arena, value_copy);
  MD_ParseResult value_parse = md_parse_from_text_tokens(new_params_arena, str8_zero(), value_copy, value_tokenize.tokens);
  for(MD_EachNode(child, value_parse.root->first))
  {
    child->parent = key_node;
  }
  key_node->first = value_parse.root->first;
  key_node->last = value_parse.root->last;
}

internal void
df_view_store_paramf(DF_View *view, String8 key, char *fmt, ...)
{
  Temp scratch = scratch_begin(0, 0);
  va_list args;
  va_start(args, fmt);
  String8 string = push_str8fv(scratch.arena, fmt, args);
  df_view_store_param(view, key, string);
  va_end(args);
  scratch_end(scratch);
}

////////////////////////////////
//~ rjf: Expand-Keyed Transient View Functions

internal DF_TransientViewNode *
df_transient_view_node_from_ev_key(DF_View *owner_view, EV_Key key)
{
  if(owner_view->transient_view_slots_count == 0)
  {
    owner_view->transient_view_slots_count = 256;
    owner_view->transient_view_slots = push_array(owner_view->arena, DF_TransientViewSlot, owner_view->transient_view_slots_count);
  }
  U64 hash = ev_hash_from_key(key);
  U64 slot_idx = hash%owner_view->transient_view_slots_count;
  DF_TransientViewSlot *slot = &owner_view->transient_view_slots[slot_idx];
  DF_TransientViewNode *node = 0;
  for(DF_TransientViewNode *n = slot->first; n != 0; n = n->next)
  {
    if(ev_key_match(n->key, key))
    {
      node = n;
      n->last_frame_index_touched = df_state->frame_index;
      break;
    }
  }
  if(node == 0)
  {
    if(!owner_view->free_transient_view_node)
    {
      owner_view->free_transient_view_node = push_array(df_state->arena, DF_TransientViewNode, 1);
    }
    node = owner_view->free_transient_view_node;
    SLLStackPop(owner_view->free_transient_view_node);
    DLLPushBack(slot->first, slot->last, node);
    node->key = key;
    node->view = df_view_alloc();
    node->initial_params_arena = arena_alloc();
    node->first_frame_index_touched = node->last_frame_index_touched = df_state->frame_index;
    DLLPushBack_NPZ(&df_nil_view, owner_view->first_transient, owner_view->last_transient, node->view, order_next, order_prev);
  }
  return node;
}

////////////////////////////////
//~ rjf: Panel State Functions

internal DF_Panel *
df_panel_alloc(DF_Window *ws)
{
  DF_Panel *panel = ws->free_panel;
  if(!df_panel_is_nil(panel))
  {
    SLLStackPop(ws->free_panel);
    U64 generation = panel->generation;
    MemoryZeroStruct(panel);
    panel->generation = generation;
  }
  else
  {
    panel = push_array(ws->arena, DF_Panel, 1);
  }
  panel->first = panel->last = panel->next = panel->prev = panel->parent = &df_nil_panel;
  panel->first_tab_view = panel->last_tab_view = &df_nil_view;
  panel->generation += 1;
  MemoryZeroStruct(&panel->animated_rect_pct);
  return panel;
}

internal void
df_panel_release(DF_Window *ws, DF_Panel *panel)
{
  df_panel_release_all_views(panel);
  SLLStackPush(ws->free_panel, panel);
  panel->generation += 1;
}

internal void
df_panel_release_all_views(DF_Panel *panel)
{
  for(DF_View *view = panel->first_tab_view, *next = 0; !df_view_is_nil(view); view = next)
  {
    next = view->order_next;
    df_view_release(view);
  }
  panel->first_tab_view = panel->last_tab_view = &df_nil_view;
  panel->selected_tab_view = d_handle_zero();
  panel->tab_view_count = 0;
}

////////////////////////////////
//~ rjf: Window State Functions

internal DF_Window *
df_window_open(Vec2F32 size, OS_Handle preferred_monitor, D_CfgSrc cfg_src)
{
  DF_Window *window = df_state->free_window;
  if(window != 0)
  {
    SLLStackPop(df_state->free_window);
    U64 gen = window->gen;
    MemoryZeroStruct(window);
    window->gen = gen;
  }
  else
  {
    window = push_array(df_state->arena, DF_Window, 1);
  }
  window->gen += 1;
  window->frames_alive = 0;
  window->cfg_src = cfg_src;
  window->arena = arena_alloc();
  {
    String8 title = str8_lit_comp(BUILD_TITLE_STRING_LITERAL);
    window->os = os_window_open(size, OS_WindowFlag_CustomBorder, title);
  }
  window->r = r_window_equip(window->os);
  window->ui = ui_state_alloc();
  window->code_ctx_menu_arena = arena_alloc();
  window->hover_eval_arena = arena_alloc();
  window->autocomp_lister_params_arena = arena_alloc();
  window->free_panel = &df_nil_panel;
  window->root_panel = df_panel_alloc(window);
  window->focused_panel = window->root_panel;
  window->query_cmd_arena = arena_alloc();
  window->query_view_stack_top = &df_nil_view;
  window->last_dpi = os_dpi_from_window(window->os);
  for(EachEnumVal(DF_SettingCode, code))
  {
    if(df_g_setting_code_default_is_per_window_table[code])
    {
      window->setting_vals[code] = df_g_setting_code_default_val_table[code];
    }
  }
  OS_Handle zero_monitor = {0};
  if(!os_handle_match(zero_monitor, preferred_monitor))
  {
    os_window_set_monitor(window->os, preferred_monitor);
  }
  if(df_state->first_window == 0) DF_RegsScope(.window = df_handle_from_window(window))
  {
    DF_FontSlot english_font_slots[] = {DF_FontSlot_Main, DF_FontSlot_Code};
    DF_FontSlot icon_font_slot = DF_FontSlot_Icons;
    for(U64 idx = 0; idx < ArrayCount(english_font_slots); idx += 1)
    {
      Temp scratch = scratch_begin(0, 0);
      DF_FontSlot slot = english_font_slots[idx];
      String8 sample_text = str8_lit("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz1234567890~!@#$%^&*()-_+=[{]}\\|;:'\",<.>/?");
      fnt_push_run_from_string(scratch.arena,
                               df_font_from_slot(slot),
                               df_font_size_from_slot(DF_FontSlot_Code),
                               0, 0, 0,
                               sample_text);
      fnt_push_run_from_string(scratch.arena,
                               df_font_from_slot(slot),
                               df_font_size_from_slot(DF_FontSlot_Main),
                               0, 0, 0,
                               sample_text);
      scratch_end(scratch);
    }
    for(DF_IconKind icon_kind = DF_IconKind_Null; icon_kind < DF_IconKind_COUNT; icon_kind = (DF_IconKind)(icon_kind+1))
    {
      Temp scratch = scratch_begin(0, 0);
      fnt_push_run_from_string(scratch.arena,
                               df_font_from_slot(icon_font_slot),
                               df_font_size_from_slot(icon_font_slot),
                               0, 0, FNT_RasterFlag_Smooth,
                               df_g_icon_kind_text_table[icon_kind]);
      fnt_push_run_from_string(scratch.arena,
                               df_font_from_slot(icon_font_slot),
                               df_font_size_from_slot(DF_FontSlot_Main),
                               0, 0, FNT_RasterFlag_Smooth,
                               df_g_icon_kind_text_table[icon_kind]);
      fnt_push_run_from_string(scratch.arena,
                               df_font_from_slot(icon_font_slot),
                               df_font_size_from_slot(DF_FontSlot_Code),
                               0, 0, FNT_RasterFlag_Smooth,
                               df_g_icon_kind_text_table[icon_kind]);
      scratch_end(scratch);
    }
  }
  DLLPushBack(df_state->first_window, df_state->last_window, window);
  return window;
}

internal DF_Window *
df_window_from_os_handle(OS_Handle os)
{
  DF_Window *result = 0;
  for(DF_Window *w = df_state->first_window; w != 0; w = w->next)
  {
    if(os_handle_match(w->os, os))
    {
      result = w;
      break;
    }
  }
  return result;
}

#if COMPILER_MSVC && !BUILD_DEBUG
#pragma optimize("", off)
#endif

internal void
df_window_frame(DF_Window *ws)
{
  ProfBeginFunction();
  
  //////////////////////////////
  //- rjf: unpack context
  //
  B32 window_is_focused = os_window_is_focused(ws->os) || ws->window_temporarily_focused_ipc;
  B32 confirm_open = df_state->confirm_active;
  B32 query_is_open = !df_view_is_nil(ws->query_view_stack_top);
  B32 hover_eval_is_open = (!confirm_open &&
                            ws->hover_eval_string.size != 0 &&
                            ws->hover_eval_first_frame_idx+20 < ws->hover_eval_last_frame_idx &&
                            df_state->frame_index-ws->hover_eval_last_frame_idx < 20);
  if(!window_is_focused || confirm_open)
  {
    ws->menu_bar_key_held = 0;
  }
  ws->window_temporarily_focused_ipc = 0;
  ui_select_state(ws->ui);
  
  //////////////////////////////
  //- rjf: auto-close tabs which have parameter entities that've been deleted
  //
  for(DF_Panel *panel = ws->root_panel;
      !df_panel_is_nil(panel);
      panel = df_panel_rec_df_pre(panel).next)
  {
    for(DF_View *view = panel->first_tab_view;
        !df_view_is_nil(view);
        view = view->order_next)
    {
      DF_Entity *entity = d_entity_from_eval_string(str8(view->query_buffer, view->query_string_size));
      if(entity->flags & DF_EntityFlag_MarkedForDeletion ||
         (df_entity_is_nil(entity) && view->spec->info.flags & DF_ViewSpecFlag_ParameterizedByEntity))
      {
        df_cmd(DF_CmdKind_CloseTab,
               .panel = df_handle_from_panel(panel),
               .view = df_handle_from_view(view));
      }
    }
  }
  
  //////////////////////////////
  //- rjf: panels with no selected tabs? -> select.
  // panels with selected tabs? -> ensure they have active tabs.
  //
  for(DF_Panel *panel = ws->root_panel;
      !df_panel_is_nil(panel);
      panel = df_panel_rec_df_pre(panel).next)
  {
    if(!df_panel_is_nil(panel->first))
    {
      continue;
    }
    DF_View *view = df_selected_tab_from_panel(panel);
    if(df_view_is_nil(view))
    {
      for(DF_View *tab = panel->first_tab_view; !df_view_is_nil(tab); tab = tab->order_next)
      {
        if(!df_view_is_project_filtered(tab))
        {
          panel->selected_tab_view = df_handle_from_view(tab);
          break;
        }
      }
    }
    if(!df_view_is_nil(view))
    {
      B32 found = 0;
      for(DF_View *tab = panel->first_tab_view; !df_view_is_nil(tab); tab = tab->order_next)
      {
        if(df_view_is_project_filtered(tab)) {continue;}
        if(tab == view)
        {
          found = 1;
        }
      }
      if(!found)
      {
        panel->selected_tab_view = d_handle_zero();
      }
    }
  }
  
  //////////////////////////////
  //- rjf: fill panel/view interaction registers
  //
  df_regs()->panel  = df_handle_from_panel(ws->focused_panel);
  df_regs()->view   = ws->focused_panel->selected_tab_view;
  
  //////////////////////////////
  //- rjf: process view-level commands on leaf panels
  //
  ProfScope("dispatch view-level commands")
  {
    for(DF_Panel *panel = ws->root_panel;
        !df_panel_is_nil(panel);
        panel = df_panel_rec_df_pre(panel).next)
    {
      if(!df_panel_is_nil(panel->first))
      {
        continue;
      }
      DF_View *view = df_selected_tab_from_panel(panel);
      if(!df_view_is_nil(view))
      {
        df_push_regs();
        df_regs()->panel = df_handle_from_panel(panel);
        df_regs()->view  = df_handle_from_view(view);
        DF_ViewCmdFunctionType *do_view_cmds_function = view->spec->info.cmd_hook;
        do_view_cmds_function(view, view->params_roots[view->params_read_gen%ArrayCount(view->params_roots)], str8(view->query_buffer, view->query_string_size));
        DF_Regs *view_regs = df_pop_regs();
        if(panel == ws->focused_panel)
        {
          MemoryCopyStruct(df_regs(), view_regs);
        }
      }
    }
  }
  
  //////////////////////////////
  //- rjf: compute ui palettes from theme
  //
  {
    DF_Theme *current = &df_state->cfg_theme;
    for(EachEnumVal(DF_PaletteCode, code))
    {
      ws->cfg_palettes[code].null       = v4f32(1, 0, 1, 1);
      ws->cfg_palettes[code].cursor     = current->colors[DF_ThemeColor_Cursor];
      ws->cfg_palettes[code].selection  = current->colors[DF_ThemeColor_SelectionOverlay];
    }
    ws->cfg_palettes[DF_PaletteCode_Base].background = current->colors[DF_ThemeColor_BaseBackground];
    ws->cfg_palettes[DF_PaletteCode_Base].text       = current->colors[DF_ThemeColor_Text];
    ws->cfg_palettes[DF_PaletteCode_Base].text_weak  = current->colors[DF_ThemeColor_TextWeak];
    ws->cfg_palettes[DF_PaletteCode_Base].border     = current->colors[DF_ThemeColor_BaseBorder];
    ws->cfg_palettes[DF_PaletteCode_MenuBar].background = current->colors[DF_ThemeColor_MenuBarBackground];
    ws->cfg_palettes[DF_PaletteCode_MenuBar].text       = current->colors[DF_ThemeColor_Text];
    ws->cfg_palettes[DF_PaletteCode_MenuBar].text_weak  = current->colors[DF_ThemeColor_TextWeak];
    ws->cfg_palettes[DF_PaletteCode_MenuBar].border     = current->colors[DF_ThemeColor_MenuBarBorder];
    ws->cfg_palettes[DF_PaletteCode_Floating].background = current->colors[DF_ThemeColor_FloatingBackground];
    ws->cfg_palettes[DF_PaletteCode_Floating].text       = current->colors[DF_ThemeColor_Text];
    ws->cfg_palettes[DF_PaletteCode_Floating].text_weak  = current->colors[DF_ThemeColor_TextWeak];
    ws->cfg_palettes[DF_PaletteCode_Floating].border     = current->colors[DF_ThemeColor_FloatingBorder];
    ws->cfg_palettes[DF_PaletteCode_ImplicitButton].background = current->colors[DF_ThemeColor_ImplicitButtonBackground];
    ws->cfg_palettes[DF_PaletteCode_ImplicitButton].text       = current->colors[DF_ThemeColor_Text];
    ws->cfg_palettes[DF_PaletteCode_ImplicitButton].text_weak  = current->colors[DF_ThemeColor_TextWeak];
    ws->cfg_palettes[DF_PaletteCode_ImplicitButton].border     = current->colors[DF_ThemeColor_ImplicitButtonBorder];
    ws->cfg_palettes[DF_PaletteCode_PlainButton].background = current->colors[DF_ThemeColor_PlainButtonBackground];
    ws->cfg_palettes[DF_PaletteCode_PlainButton].text       = current->colors[DF_ThemeColor_Text];
    ws->cfg_palettes[DF_PaletteCode_PlainButton].text_weak  = current->colors[DF_ThemeColor_TextWeak];
    ws->cfg_palettes[DF_PaletteCode_PlainButton].border     = current->colors[DF_ThemeColor_PlainButtonBorder];
    ws->cfg_palettes[DF_PaletteCode_PositivePopButton].background = current->colors[DF_ThemeColor_PositivePopButtonBackground];
    ws->cfg_palettes[DF_PaletteCode_PositivePopButton].text       = current->colors[DF_ThemeColor_Text];
    ws->cfg_palettes[DF_PaletteCode_PositivePopButton].text_weak  = current->colors[DF_ThemeColor_TextWeak];
    ws->cfg_palettes[DF_PaletteCode_PositivePopButton].border     = current->colors[DF_ThemeColor_PositivePopButtonBorder];
    ws->cfg_palettes[DF_PaletteCode_NegativePopButton].background = current->colors[DF_ThemeColor_NegativePopButtonBackground];
    ws->cfg_palettes[DF_PaletteCode_NegativePopButton].text       = current->colors[DF_ThemeColor_Text];
    ws->cfg_palettes[DF_PaletteCode_NegativePopButton].text_weak  = current->colors[DF_ThemeColor_TextWeak];
    ws->cfg_palettes[DF_PaletteCode_NegativePopButton].border     = current->colors[DF_ThemeColor_NegativePopButtonBorder];
    ws->cfg_palettes[DF_PaletteCode_NeutralPopButton].background = current->colors[DF_ThemeColor_NeutralPopButtonBackground];
    ws->cfg_palettes[DF_PaletteCode_NeutralPopButton].text       = current->colors[DF_ThemeColor_Text];
    ws->cfg_palettes[DF_PaletteCode_NeutralPopButton].text_weak  = current->colors[DF_ThemeColor_TextWeak];
    ws->cfg_palettes[DF_PaletteCode_NeutralPopButton].border     = current->colors[DF_ThemeColor_NeutralPopButtonBorder];
    ws->cfg_palettes[DF_PaletteCode_ScrollBarButton].background = current->colors[DF_ThemeColor_ScrollBarButtonBackground];
    ws->cfg_palettes[DF_PaletteCode_ScrollBarButton].text       = current->colors[DF_ThemeColor_Text];
    ws->cfg_palettes[DF_PaletteCode_ScrollBarButton].text_weak  = current->colors[DF_ThemeColor_TextWeak];
    ws->cfg_palettes[DF_PaletteCode_ScrollBarButton].border     = current->colors[DF_ThemeColor_ScrollBarButtonBorder];
    ws->cfg_palettes[DF_PaletteCode_Tab].background = current->colors[DF_ThemeColor_TabBackground];
    ws->cfg_palettes[DF_PaletteCode_Tab].text       = current->colors[DF_ThemeColor_Text];
    ws->cfg_palettes[DF_PaletteCode_Tab].text_weak  = current->colors[DF_ThemeColor_TextWeak];
    ws->cfg_palettes[DF_PaletteCode_Tab].border     = current->colors[DF_ThemeColor_TabBorder];
    ws->cfg_palettes[DF_PaletteCode_TabInactive].background = current->colors[DF_ThemeColor_TabBackgroundInactive];
    ws->cfg_palettes[DF_PaletteCode_TabInactive].text       = current->colors[DF_ThemeColor_Text];
    ws->cfg_palettes[DF_PaletteCode_TabInactive].text_weak  = current->colors[DF_ThemeColor_TextWeak];
    ws->cfg_palettes[DF_PaletteCode_TabInactive].border     = current->colors[DF_ThemeColor_TabBorderInactive];
    ws->cfg_palettes[DF_PaletteCode_DropSiteOverlay].background = current->colors[DF_ThemeColor_DropSiteOverlay];
    ws->cfg_palettes[DF_PaletteCode_DropSiteOverlay].text       = current->colors[DF_ThemeColor_DropSiteOverlay];
    ws->cfg_palettes[DF_PaletteCode_DropSiteOverlay].text_weak  = current->colors[DF_ThemeColor_DropSiteOverlay];
    ws->cfg_palettes[DF_PaletteCode_DropSiteOverlay].border     = current->colors[DF_ThemeColor_DropSiteOverlay];
    if(df_setting_val_from_code(DF_SettingCode_OpaqueBackgrounds).s32)
    {
      for(EachEnumVal(DF_PaletteCode, code))
      {
        if(ws->cfg_palettes[code].background.x != 0 ||
           ws->cfg_palettes[code].background.y != 0 ||
           ws->cfg_palettes[code].background.z != 0)
        {
          ws->cfg_palettes[code].background.w = 1;
        }
      }
    }
  }
  
  //////////////////////////////
  //- rjf: build UI
  //
  UI_Box *autocomp_box = &ui_g_nil_box;
  UI_Box *hover_eval_box = &ui_g_nil_box;
  ProfScope("build UI")
  {
    ////////////////////////////
    //- rjf: set up
    //
    {
      // rjf: gather font info
      FNT_Tag main_font = df_font_from_slot(DF_FontSlot_Main);
      F32 main_font_size = df_font_size_from_slot(DF_FontSlot_Main);
      FNT_Tag icon_font = df_font_from_slot(DF_FontSlot_Icons);
      
      // rjf: build icon info
      UI_IconInfo icon_info = {0};
      {
        icon_info.icon_font = icon_font;
        icon_info.icon_kind_text_map[UI_IconKind_RightArrow]     = df_g_icon_kind_text_table[DF_IconKind_RightScroll];
        icon_info.icon_kind_text_map[UI_IconKind_DownArrow]      = df_g_icon_kind_text_table[DF_IconKind_DownScroll];
        icon_info.icon_kind_text_map[UI_IconKind_LeftArrow]      = df_g_icon_kind_text_table[DF_IconKind_LeftScroll];
        icon_info.icon_kind_text_map[UI_IconKind_UpArrow]        = df_g_icon_kind_text_table[DF_IconKind_UpScroll];
        icon_info.icon_kind_text_map[UI_IconKind_RightCaret]     = df_g_icon_kind_text_table[DF_IconKind_RightCaret];
        icon_info.icon_kind_text_map[UI_IconKind_DownCaret]      = df_g_icon_kind_text_table[DF_IconKind_DownCaret];
        icon_info.icon_kind_text_map[UI_IconKind_LeftCaret]      = df_g_icon_kind_text_table[DF_IconKind_LeftCaret];
        icon_info.icon_kind_text_map[UI_IconKind_UpCaret]        = df_g_icon_kind_text_table[DF_IconKind_UpCaret];
        icon_info.icon_kind_text_map[UI_IconKind_CheckHollow]    = df_g_icon_kind_text_table[DF_IconKind_CheckHollow];
        icon_info.icon_kind_text_map[UI_IconKind_CheckFilled]    = df_g_icon_kind_text_table[DF_IconKind_CheckFilled];
      }
      
      // rjf: build widget palette info
      UI_WidgetPaletteInfo widget_palette_info = {0};
      {
        widget_palette_info.tooltip_palette   = df_palette_from_code(DF_PaletteCode_Floating);
        widget_palette_info.ctx_menu_palette  = df_palette_from_code(DF_PaletteCode_Floating);
        widget_palette_info.scrollbar_palette = df_palette_from_code(DF_PaletteCode_ScrollBarButton);
      }
      
      // rjf: build animation info
      UI_AnimationInfo animation_info = {0};
      {
        if(df_setting_val_from_code(DF_SettingCode_HoverAnimations).s32)       {animation_info.flags |= UI_AnimationInfoFlag_HotAnimations;}
        if(df_setting_val_from_code(DF_SettingCode_PressAnimations).s32)       {animation_info.flags |= UI_AnimationInfoFlag_ActiveAnimations;}
        if(df_setting_val_from_code(DF_SettingCode_FocusAnimations).s32)       {animation_info.flags |= UI_AnimationInfoFlag_FocusAnimations;}
        if(df_setting_val_from_code(DF_SettingCode_TooltipAnimations).s32)     {animation_info.flags |= UI_AnimationInfoFlag_TooltipAnimations;}
        if(df_setting_val_from_code(DF_SettingCode_MenuAnimations).s32)        {animation_info.flags |= UI_AnimationInfoFlag_ContextMenuAnimations;}
        if(df_setting_val_from_code(DF_SettingCode_ScrollingAnimations).s32)   {animation_info.flags |= UI_AnimationInfoFlag_ScrollingAnimations;}
      }
      
      // rjf: begin & push initial stack values
      ui_begin_build(ws->os, &ws->ui_events, &icon_info, &widget_palette_info, &animation_info, df_state->frame_dt, df_state->frame_dt);
      ui_push_font(main_font);
      ui_push_font_size(main_font_size);
      ui_push_text_padding(main_font_size*0.3f);
      ui_push_pref_width(ui_em(20.f, 1));
      ui_push_pref_height(ui_em(2.75f, 1.f));
      ui_push_palette(df_palette_from_code(DF_PaletteCode_Base));
      ui_push_blur_size(10.f);
      FNT_RasterFlags text_raster_flags = 0;
      if(df_setting_val_from_code(DF_SettingCode_SmoothUIText).s32) {text_raster_flags |= FNT_RasterFlag_Smooth;}
      if(df_setting_val_from_code(DF_SettingCode_HintUIText).s32) {text_raster_flags |= FNT_RasterFlag_Hinted;}
      ui_push_text_raster_flags(text_raster_flags);
    }
    
    ////////////////////////////
    //- rjf: calculate top-level rectangles
    //
    Rng2F32 window_rect = os_client_rect_from_window(ws->os);
    Vec2F32 window_rect_dim = dim_2f32(window_rect);
    Rng2F32 top_bar_rect = r2f32p(window_rect.x0, window_rect.y0, window_rect.x0+window_rect_dim.x+1, window_rect.y0+ui_top_pref_height().value);
    Rng2F32 bottom_bar_rect = r2f32p(window_rect.x0, window_rect_dim.y - ui_top_pref_height().value, window_rect.x0+window_rect_dim.x, window_rect.y0+window_rect_dim.y);
    Rng2F32 content_rect = r2f32p(window_rect.x0, top_bar_rect.y1, window_rect.x0+window_rect_dim.x, bottom_bar_rect.y0);
    F32 window_edge_px = os_dpi_from_window(ws->os)*0.035f;
    content_rect = pad_2f32(content_rect, -window_edge_px);
    
    ////////////////////////////
    //- rjf: truncated string hover
    //
    if(ui_string_hover_active()) UI_Tooltip
    {
      Temp scratch = scratch_begin(0, 0);
      String8 string = ui_string_hover_string(scratch.arena);
      DR_FancyRunList runs = ui_string_hover_runs(scratch.arena);
      UI_Box *box = ui_build_box_from_key(UI_BoxFlag_DrawText, ui_key_zero());
      ui_box_equip_display_string_fancy_runs(box, string, &runs);
      scratch_end(scratch);
    }
    
    ////////////////////////////
    //- rjf: drag/drop visualization tooltips
    //
    B32 drag_active = df_drag_is_active();
    if(drag_active && window_is_focused)
    {
      Temp scratch = scratch_begin(0, 0);
      DF_DragDropPayload *payload = &df_drag_drop_payload;
      DF_Panel *panel = df_panel_from_handle(payload->panel);
      DF_Entity *entity = df_entity_from_handle(payload->entity);
      DF_View *view = df_view_from_handle(payload->view);
      {
        //- rjf: tab dragging
        if(!df_view_is_nil(view))
        {
          UI_Size main_width = ui_top_pref_width();
          UI_Size main_height = ui_top_pref_height();
          UI_TextAlign main_text_align = ui_top_text_alignment();
          DF_Palette(DF_PaletteCode_Tab)
            UI_Tooltip
            UI_PrefWidth(main_width)
            UI_PrefHeight(main_height)
            UI_TextAlignment(main_text_align)
          {
            ui_set_next_pref_width(ui_em(60.f, 1.f));
            ui_set_next_pref_height(ui_em(40.f, 1.f));
            ui_set_next_child_layout_axis(Axis2_Y);
            UI_Box *container = ui_build_box_from_key(0, ui_key_zero());
            UI_Parent(container)
            {
              UI_Row
              {
                DF_IconKind icon_kind = df_icon_kind_from_view(view);
                DR_FancyStringList fstrs = df_title_fstrs_from_view(scratch.arena, view, ui_top_palette()->text, ui_top_palette()->text_weak, ui_top_font_size());
                DF_Font(DF_FontSlot_Icons)
                  UI_FontSize(df_font_size_from_slot(DF_FontSlot_Icons))
                  UI_PrefWidth(ui_em(2.5f, 1.f))
                  UI_FlagsAdd(UI_BoxFlag_DrawTextWeak)
                  ui_label(df_g_icon_kind_text_table[icon_kind]);
                UI_PrefWidth(ui_text_dim(10, 1))
                {
                  UI_Box *name_box = ui_build_box_from_key(UI_BoxFlag_DrawText, ui_key_zero());
                  ui_box_equip_display_fancy_strings(name_box, &fstrs);
                }
              }
              ui_set_next_pref_width(ui_pct(1, 0));
              ui_set_next_pref_height(ui_pct(1, 0));
              ui_set_next_child_layout_axis(Axis2_Y);
              UI_Box *view_preview_container = ui_build_box_from_stringf(UI_BoxFlag_DrawBorder|UI_BoxFlag_DrawBackground|UI_BoxFlag_Clip, "###view_preview_container");
              UI_Parent(view_preview_container) UI_Focus(UI_FocusKind_Off) UI_WidthFill
              {
                DF_ViewSpec *view_spec = view->spec;
                DF_ViewUIFunctionType *build_view_ui_function = view_spec->info.ui_hook;
                build_view_ui_function(view, view->params_roots[view->params_read_gen%ArrayCount(view->params_roots)], str8(view->query_buffer, view->query_string_size), view_preview_container->rect);
              }
            }
          }
        }
        
        //- rjf: entity dragging
        else if(!df_entity_is_nil(entity)) UI_Tooltip
        {
          ui_set_next_pref_width(ui_children_sum(1));
          UI_Row UI_HeightFill
          {
            String8 display_name = df_display_string_from_entity(scratch.arena, entity);
            DF_IconKind icon_kind = df_entity_kind_icon_kind_table[entity->kind];
            DF_Font(DF_FontSlot_Icons)
              UI_FontSize(df_font_size_from_slot(DF_FontSlot_Icons))
              UI_FlagsAdd(UI_BoxFlag_DrawTextWeak)
              ui_label(df_g_icon_kind_text_table[icon_kind]);
            ui_label(display_name);
          }
        }
      }
      scratch_end(scratch);
    }
    
    ////////////////////////////
    //- rjf: developer menu
    //
    if(ws->dev_menu_is_open) DF_Font(DF_FontSlot_Code)
    {
      ui_set_next_flags(UI_BoxFlag_ViewScrollY|UI_BoxFlag_AllowOverflowY|UI_BoxFlag_ViewClamp);
      UI_PaneF(r2f32p(30, 30, 30+ui_top_font_size()*100, ui_top_font_size()*150), "###dev_ctx_menu")
      {
        //- rjf: toggles
        for(U64 idx = 0; idx < ArrayCount(DEV_toggle_table); idx += 1)
        {
          if(ui_clicked(df_icon_button(*DEV_toggle_table[idx].value_ptr ? DF_IconKind_CheckFilled : DF_IconKind_CheckHollow, 0, DEV_toggle_table[idx].name)))
          {
            *DEV_toggle_table[idx].value_ptr ^= 1;
          }
        }
        
        ui_divider(ui_em(1.f, 1.f));
        
        //- rjf: draw current interaction regs
        {
          DF_Regs *regs = df_regs();
#define Handle(name) ui_labelf("%s: [0x%I64x, 0x%I64x]", #name, (regs->name).u64[0], (regs->name).u64[1])
          Handle(window);
          Handle(panel);
          Handle(view);
#undef Handle
          ui_labelf("file_path: \"%S\"", regs->file_path);
          ui_labelf("cursor: (L:%I64d, C:%I64d)", regs->cursor.line, regs->cursor.column);
          ui_labelf("mark: (L:%I64d, C:%I64d)", regs->mark.line, regs->mark.column);
          ui_labelf("unwind_count: %I64u", regs->unwind_count);
          ui_labelf("inline_depth: %I64u", regs->inline_depth);
          ui_labelf("text_key: [0x%I64x, 0x%I64x]", regs->text_key.u64[0], regs->text_key.u64[1]);
          ui_labelf("lang_kind: '%S'", txt_extension_from_lang_kind(regs->lang_kind));
          ui_labelf("vaddr_range: [0x%I64x, 0x%I64x)", regs->vaddr_range.min, regs->vaddr_range.max);
          ui_labelf("voff_range: [0x%I64x, 0x%I64x)", regs->voff_range.min, regs->voff_range.max);
        }
        
        ui_divider(ui_em(1.f, 1.f));
        
        //- rjf: draw per-window stats
        for(DF_Window *window = df_state->first_window; window != 0; window = window->next)
        {
          // rjf: calc ui hash chain length
          F64 avg_ui_hash_chain_length = 0;
          {
            F64 chain_count = 0;
            F64 chain_length_sum = 0;
            for(U64 idx = 0; idx < ws->ui->box_table_size; idx += 1)
            {
              F64 chain_length = 0;
              for(UI_Box *b = ws->ui->box_table[idx].hash_first; !ui_box_is_nil(b); b = b->hash_next)
              {
                chain_length += 1;
              }
              if(chain_length > 0)
              {
                chain_length_sum += chain_length;
                chain_count += 1;
              }
            }
            avg_ui_hash_chain_length = chain_length_sum / chain_count;
          }
          ui_labelf("Target Hz: %.2f", 1.f/df_state->frame_dt);
          ui_labelf("Ctrl Run Index: %I64u", ctrl_run_gen());
          ui_labelf("Ctrl Mem Gen Index: %I64u", ctrl_mem_gen());
          ui_labelf("Window %p", window);
          ui_set_next_pref_width(ui_children_sum(1));
          ui_set_next_pref_height(ui_children_sum(1));
          UI_Row
          {
            ui_spacer(ui_em(2.f, 1.f));
            ui_labelf("Box Count: %I64u", window->ui->last_build_box_count);
          }
          ui_set_next_pref_width(ui_children_sum(1));
          ui_set_next_pref_height(ui_children_sum(1));
          UI_Row
          {
            ui_spacer(ui_em(2.f, 1.f));
            ui_labelf("Average UI Hash Chain Length: %f", avg_ui_hash_chain_length);
          }
        }
        
        ui_divider(ui_em(1.f, 1.f));
        
        //- rjf: draw entity tree
        DF_EntityRec rec = {0};
        S32 indent = 0;
        UI_PrefWidth(ui_text_dim(10, 1)) ui_labelf("Entity Tree:");
        for(DF_Entity *e = df_entity_root(); !df_entity_is_nil(e); e = rec.next)
        {
          ui_set_next_pref_width(ui_children_sum(1));
          ui_set_next_pref_height(ui_children_sum(1));
          UI_Row
          {
            ui_spacer(ui_em(2.f*indent, 1.f));
            DF_Entity *dst = df_entity_from_handle(e->entity_handle);
            if(!df_entity_is_nil(dst))
            {
              ui_labelf("[link] %S -> %S", e->string, dst->string);
            }
            else
            {
              ui_labelf("%S: %S", d_entity_kind_display_string_table[e->kind], e->string);
            }
          }
          rec = df_entity_rec_depth_first_pre(e, df_entity_root());
          indent += rec.push_count;
          indent -= rec.pop_count;
        }
      }
    }
    
    ////////////////////////////
    //- rjf: universal ctx menus
    //
    DF_Palette(DF_PaletteCode_Floating)
    {
      Temp scratch = scratch_begin(0, 0);
      
      //- rjf: auto-close entity ctx menu
      if(ui_ctx_menu_is_open(df_state->entity_ctx_menu_key))
      {
        DF_Entity *entity = df_entity_from_handle(ws->entity_ctx_menu_entity);
        if(df_entity_is_nil(entity))
        {
          ui_ctx_menu_close();
        }
      }
      
      //- rjf: code ctx menu
      UI_CtxMenu(df_state->code_ctx_menu_key)
        UI_PrefWidth(ui_em(40.f, 1.f))
        DF_Palette(DF_PaletteCode_ImplicitButton)
      {
        TXT_Scope *txt_scope = txt_scope_open();
        HS_Scope *hs_scope = hs_scope_open();
        TxtRng range = ws->code_ctx_menu_range;
        D_LineList lines = ws->code_ctx_menu_lines;
        if(!txt_pt_match(range.min, range.max) && ui_clicked(df_cmd_spec_button(df_cmd_kind_info_table[DF_CmdKind_Copy].string)))
        {
          U128 hash = {0};
          TXT_TextInfo info = txt_text_info_from_key_lang(txt_scope, ws->code_ctx_menu_text_key, ws->code_ctx_menu_lang_kind, &hash);
          String8 data = hs_data_from_hash(hs_scope, hash);
          String8 copy_data = txt_string_from_info_data_txt_rng(&info, data, ws->code_ctx_menu_range);
          os_set_clipboard_text(copy_data);
          ui_ctx_menu_close();
        }
        if(range.min.line == range.max.line && ui_clicked(df_icon_buttonf(DF_IconKind_RightArrow, 0, "Set Next Statement")))
        {
          CTRL_Entity *thread = ctrl_entity_from_handle(d_state->ctrl_entity_store, df_regs()->thread);
          U64 new_rip_vaddr = ws->code_ctx_menu_vaddr;
          if(ws->code_ctx_menu_file_path.size != 0)
          {
            for(D_LineNode *n = lines.first; n != 0; n = n->next)
            {
              CTRL_EntityList modules = ctrl_modules_from_dbgi_key(scratch.arena, d_state->ctrl_entity_store, &n->v.dbgi_key);
              CTRL_Entity *module = ctrl_module_from_thread_candidates(d_state->ctrl_entity_store, thread, &modules);
              if(module != &ctrl_entity_nil)
              {
                new_rip_vaddr = ctrl_vaddr_from_voff(module, n->v.voff_range.min);
                break;
              }
            }
          }
          df_cmd(DF_CmdKind_SetThreadIP, .vaddr = new_rip_vaddr);
          ui_ctx_menu_close();
        }
        if(range.min.line == range.max.line && ui_clicked(df_icon_buttonf(DF_IconKind_Play, 0, "Run To Line")))
        {
          if(ws->code_ctx_menu_file_path.size != 0)
          {
            df_cmd(DF_CmdKind_RunToLine, .file_path = ws->code_ctx_menu_file_path, .cursor = range.min);
          }
          else
          {
            df_cmd(DF_CmdKind_RunToAddress, .vaddr = ws->code_ctx_menu_vaddr);
          }
          ui_ctx_menu_close();
        }
        if(range.min.line == range.max.line && ui_clicked(df_icon_buttonf(DF_IconKind_Null, 0, "Go To Name")))
        {
          U128 hash = {0};
          TXT_TextInfo info = txt_text_info_from_key_lang(txt_scope, ws->code_ctx_menu_text_key, ws->code_ctx_menu_lang_kind, &hash);
          String8 data = hs_data_from_hash(hs_scope, hash);
          Rng1U64 expr_off_range = {0};
          if(range.min.column != range.max.column)
          {
            expr_off_range = r1u64(txt_off_from_info_pt(&info, range.min), txt_off_from_info_pt(&info, range.max));
          }
          else
          {
            expr_off_range = txt_expr_off_range_from_info_data_pt(&info, data, range.min);
          }
          String8 expr = str8_substr(data, expr_off_range);
          df_cmd(DF_CmdKind_GoToName, .string = expr);
          ui_ctx_menu_close();
        }
        if(range.min.line == range.max.line && ui_clicked(df_icon_buttonf(DF_IconKind_CircleFilled, 0, "Toggle Breakpoint")))
        {
          df_cmd(DF_CmdKind_ToggleBreakpoint,
                 .file_path = ws->code_ctx_menu_file_path,
                 .cursor    = range.min,
                 .vaddr     = ws->code_ctx_menu_vaddr);
          ui_ctx_menu_close();
        }
        if(range.min.line == range.max.line && ui_clicked(df_icon_buttonf(DF_IconKind_Binoculars, 0, "Toggle Watch Expression")))
        {
          U128 hash = {0};
          TXT_TextInfo info = txt_text_info_from_key_lang(txt_scope, ws->code_ctx_menu_text_key, ws->code_ctx_menu_lang_kind, &hash);
          String8 data = hs_data_from_hash(hs_scope, hash);
          Rng1U64 expr_off_range = {0};
          if(range.min.column != range.max.column)
          {
            expr_off_range = r1u64(txt_off_from_info_pt(&info, range.min), txt_off_from_info_pt(&info, range.max));
          }
          else
          {
            expr_off_range = txt_expr_off_range_from_info_data_pt(&info, data, range.min);
          }
          String8 expr = str8_substr(data, expr_off_range);
          df_cmd(DF_CmdKind_ToggleWatchExpression, .string = expr);
          ui_ctx_menu_close();
        }
        if(ws->code_ctx_menu_file_path.size == 0 && range.min.line == range.max.line && ui_clicked(df_icon_buttonf(DF_IconKind_FileOutline, 0, "Go To Source")))
        {
          if(lines.first != 0)
          {
            df_cmd(DF_CmdKind_FindCodeLocation,
                   .file_path = lines.first->v.file_path,
                   .cursor    = lines.first->v.pt);
          }
          ui_ctx_menu_close();
        }
        if(ws->code_ctx_menu_file_path.size != 0 && range.min.line == range.max.line && ui_clicked(df_icon_buttonf(DF_IconKind_FileOutline, 0, "Go To Disassembly")))
        {
          CTRL_Entity *thread = ctrl_entity_from_handle(d_state->ctrl_entity_store, df_regs()->thread);
          U64 vaddr = 0;
          for(D_LineNode *n = lines.first; n != 0; n = n->next)
          {
            CTRL_EntityList modules = ctrl_modules_from_dbgi_key(scratch.arena, d_state->ctrl_entity_store, &n->v.dbgi_key);
            CTRL_Entity *module = ctrl_module_from_thread_candidates(d_state->ctrl_entity_store, thread, &modules);
            if(module != &ctrl_entity_nil)
            {
              vaddr = ctrl_vaddr_from_voff(module, n->v.voff_range.min);
              break;
            }
          }
          df_cmd(DF_CmdKind_FindCodeLocation, .vaddr = vaddr);
          ui_ctx_menu_close();
        }
        hs_scope_close(hs_scope);
        txt_scope_close(txt_scope);
      }
      
      //- rjf: entity menu
      UI_CtxMenu(df_state->entity_ctx_menu_key)
        UI_PrefWidth(ui_em(40.f, 1.f))
        DF_Palette(DF_PaletteCode_ImplicitButton)
      {
        DF_Entity *entity = df_entity_from_handle(ws->entity_ctx_menu_entity);
        CTRL_Entity *entity_ctrl = ctrl_entity_from_handle(d_state->ctrl_entity_store, entity->ctrl_handle);
        DF_IconKind entity_icon = df_entity_kind_icon_kind_table[entity->kind];
        DF_EntityKindFlags kind_flags = d_entity_kind_flags_table[entity->kind];
        String8 display_name = df_display_string_from_entity(scratch.arena, entity);
        
        // rjf: title
        UI_Row
        {
          ui_spacer(ui_em(1.f, 1.f));
          DF_Font(DF_FontSlot_Icons)
            UI_FontSize(df_font_size_from_slot(DF_FontSlot_Icons))
            UI_PrefWidth(ui_em(2.f, 1.f))
            UI_PrefHeight(ui_pct(1, 0))
            UI_TextAlignment(UI_TextAlign_Center)
            UI_Flags(UI_BoxFlag_DrawTextWeak)
            ui_label(df_g_icon_kind_text_table[entity_icon]);
          UI_PrefWidth(ui_text_dim(10, 1))
            UI_Flags(UI_BoxFlag_DrawTextWeak)
            ui_label(d_entity_kind_display_string_table[entity->kind]);
          {
            UI_Palette *palette = ui_top_palette();
            if(entity->flags & DF_EntityFlag_HasColor)
            {
              palette = ui_build_palette(ui_top_palette(), .text = df_rgba_from_entity(entity));
            }
            UI_Palette(palette)
              UI_PrefWidth(ui_text_dim(10, 1))
              DF_Font((kind_flags & DF_EntityKindFlag_NameIsCode) ? DF_FontSlot_Code : DF_FontSlot_Main)
              ui_label(display_name);
          }
        }
        
        DF_Palette(DF_PaletteCode_Floating) ui_divider(ui_em(1.f, 1.f));
        
        // rjf: name editor
        if(kind_flags & DF_EntityKindFlag_CanRename) UI_TextPadding(ui_top_font_size()*1.5f)
        {
          UI_Signal sig = df_line_editf(DF_LineEditFlag_Border, 0, 0, &ws->entity_ctx_menu_input_cursor, &ws->entity_ctx_menu_input_mark, ws->entity_ctx_menu_input_buffer, sizeof(ws->entity_ctx_menu_input_buffer), &ws->entity_ctx_menu_input_size, 0, entity->string, "%S###entity_name_edit_%p", d_entity_kind_name_label_table[entity->kind], entity);
          if(ui_committed(sig))
          {
            df_cmd(DF_CmdKind_NameEntity,
                   .entity = df_handle_from_entity(entity),
                   .string = str8(ws->entity_ctx_menu_input_buffer, ws->entity_ctx_menu_input_size));
          }
        }
        
        // rjf: condition editor
        if(kind_flags & DF_EntityKindFlag_CanCondition)
          DF_Font(DF_FontSlot_Code)
          UI_TextPadding(ui_top_font_size()*1.5f)
        {
          DF_Entity *condition = df_entity_child_from_kind(entity, DF_EntityKind_Condition);
          UI_Signal sig = df_line_editf(DF_LineEditFlag_Border|DF_LineEditFlag_CodeContents, 0, 0, &ws->entity_ctx_menu_input_cursor, &ws->entity_ctx_menu_input_mark, ws->entity_ctx_menu_input_buffer, sizeof(ws->entity_ctx_menu_input_buffer), &ws->entity_ctx_menu_input_size, 0, condition->string, "Condition###entity_cond_edit_%p", entity);
          if(ui_committed(sig))
          {
            String8 new_string = str8(ws->entity_ctx_menu_input_buffer, ws->entity_ctx_menu_input_size);
            if(new_string.size != 0)
            {
              if(df_entity_is_nil(condition))
              {
                condition = df_entity_alloc(entity, DF_EntityKind_Condition);
              }
              df_cmd(DF_CmdKind_NameEntity, .entity = df_handle_from_entity(condition), .string = new_string);
            }
            else if(!df_entity_is_nil(condition))
            {
              df_entity_mark_for_deletion(condition);
            }
          }
        }
        
        // rjf: exe editor
        if(entity->kind == DF_EntityKind_Target) UI_TextPadding(ui_top_font_size()*1.5f)
        {
          DF_Entity *exe = df_entity_child_from_kind(entity, DF_EntityKind_Executable);
          UI_Signal sig = df_line_editf(DF_LineEditFlag_Border, 0, 0, &ws->entity_ctx_menu_input_cursor, &ws->entity_ctx_menu_input_mark, ws->entity_ctx_menu_input_buffer, sizeof(ws->entity_ctx_menu_input_buffer), &ws->entity_ctx_menu_input_size, 0, exe->string, "Executable###entity_exe_edit_%p", entity);
          if(ui_committed(sig))
          {
            String8 new_string = str8(ws->entity_ctx_menu_input_buffer, ws->entity_ctx_menu_input_size);
            if(new_string.size != 0)
            {
              if(df_entity_is_nil(exe))
              {
                exe = df_entity_alloc(entity, DF_EntityKind_Executable);
              }
              df_cmd(DF_CmdKind_NameEntity, .entity = df_handle_from_entity(exe), .string = new_string);
            }
            else if(!df_entity_is_nil(exe))
            {
              df_entity_mark_for_deletion(exe);
            }
          }
        }
        
        // rjf: arguments editors
        if(entity->kind == DF_EntityKind_Target) UI_TextPadding(ui_top_font_size()*1.5f)
        {
          DF_Entity *args = df_entity_child_from_kind(entity, DF_EntityKind_Arguments);
          UI_Signal sig = df_line_editf(DF_LineEditFlag_Border, 0, 0, &ws->entity_ctx_menu_input_cursor, &ws->entity_ctx_menu_input_mark, ws->entity_ctx_menu_input_buffer, sizeof(ws->entity_ctx_menu_input_buffer), &ws->entity_ctx_menu_input_size, 0, args->string, "Arguments###entity_args_edit_%p", entity);
          if(ui_committed(sig))
          {
            String8 new_string = str8(ws->entity_ctx_menu_input_buffer, ws->entity_ctx_menu_input_size);
            if(new_string.size != 0)
            {
              if(df_entity_is_nil(args))
              {
                args = df_entity_alloc(entity, DF_EntityKind_Arguments);
              }
              df_cmd(DF_CmdKind_NameEntity, .entity = df_handle_from_entity(args), .string = new_string);
            }
            else if(!df_entity_is_nil(args))
            {
              df_entity_mark_for_deletion(args);
            }
          }
        }
        
        // rjf: copy name
        if(ui_clicked(df_icon_buttonf(DF_IconKind_Clipboard, 0, "Copy Name")))
        {
          os_set_clipboard_text(display_name);
          ui_ctx_menu_close();
        }
        
        // rjf: is command line only? -> make permanent
        if(entity->cfg_src == D_CfgSrc_CommandLine && ui_clicked(df_icon_buttonf(DF_IconKind_Save, 0, "Save To Project")))
        {
          df_entity_equip_cfg_src(entity, D_CfgSrc_Project);
        }
        
        // rjf: duplicate
        if(kind_flags & DF_EntityKindFlag_CanDuplicate && ui_clicked(df_icon_buttonf(DF_IconKind_XSplit, 0, "Duplicate")))
        {
          df_cmd(DF_CmdKind_DuplicateEntity, .entity = df_handle_from_entity(entity));
          ui_ctx_menu_close();
        }
        
        // rjf: edit
        if(kind_flags & DF_EntityKindFlag_CanEdit && ui_clicked(df_icon_buttonf(DF_IconKind_Pencil, 0, "Edit")))
        {
          df_cmd(DF_CmdKind_EditEntity, .entity = df_handle_from_entity(entity));
          ui_ctx_menu_close();
        }
        
        // rjf: deletion
        if(kind_flags & DF_EntityKindFlag_CanDelete && ui_clicked(df_icon_buttonf(DF_IconKind_Trash, 0, "Delete")))
        {
          df_cmd(DF_CmdKind_RemoveEntity, .entity = df_handle_from_entity(entity));
          ui_ctx_menu_close();
        }
        
        // rjf: enabling
        if(kind_flags & DF_EntityKindFlag_CanEnable)
        {
          B32 is_enabled = !entity->disabled;
          if(!is_enabled && ui_clicked(df_icon_buttonf(DF_IconKind_CheckHollow, 0, "Enable###enabler")))
          {
            df_cmd(DF_CmdKind_EnableEntity, .entity = df_handle_from_entity(entity));
          }
          if(is_enabled && ui_clicked(df_icon_buttonf(DF_IconKind_CheckFilled, 0, "Disable###enabler")))
          {
            df_cmd(DF_CmdKind_DisableEntity, .entity = df_handle_from_entity(entity));
          }
        }
        
        // rjf: freezing
        if(kind_flags & DF_EntityKindFlag_CanFreeze)
        {
          B32 is_frozen = ctrl_entity_tree_is_frozen(entity_ctrl);
          ui_set_next_palette(df_palette_from_code(is_frozen ? DF_PaletteCode_NegativePopButton : DF_PaletteCode_PositivePopButton));
          if(is_frozen && ui_clicked(df_icon_buttonf(DF_IconKind_Locked, 0, "Thaw###freeze_thaw")))
          {
            df_cmd(DF_CmdKind_ThawEntity, .entity = df_handle_from_entity(entity));
          }
          if(!is_frozen && ui_clicked(df_icon_buttonf(DF_IconKind_Unlocked, 0, "Freeze###freeze_thaw")))
          {
            df_cmd(DF_CmdKind_FreezeEntity, .entity = df_handle_from_entity(entity));
          }
        }
        
        // rjf: go-to-location
        {
          DF_Entity *loc = df_entity_child_from_kind(entity, DF_EntityKind_Location);
          if(!df_entity_is_nil(loc) && ui_clicked(df_icon_buttonf(DF_IconKind_FileOutline, 0, "Go To Location")))
          {
            df_cmd(DF_CmdKind_FindCodeLocation,
                   .file_path  = loc->string,
                   .cursor     = loc->text_point,
                   .vaddr      = loc->vaddr);
            ui_ctx_menu_close();
          }
        }
        
        // rjf: entity-kind-specific options
        switch(entity->kind)
        {
          default:
          {
          }break;
          
          case DF_EntityKind_Process:
          case DF_EntityKind_Thread:
          {
            if(entity->kind == DF_EntityKind_Thread)
            {
              CTRL_Entity *entity_ctrl = ctrl_entity_from_handle(d_state->ctrl_entity_store, entity->ctrl_handle);
              B32 is_selected = ctrl_handle_match(df_base_regs()->thread, entity_ctrl->handle);
              if(is_selected)
              {
                df_icon_buttonf(DF_IconKind_Thread, 0, "[Selected]###select_entity");
              }
              else if(ui_clicked(df_icon_buttonf(DF_IconKind_Thread, 0, "Select###select_entity")))
              {
                df_cmd(DF_CmdKind_SelectThread, .thread = entity_ctrl->handle);
                ui_ctx_menu_close();
              }
            }
            
            if(ui_clicked(df_icon_buttonf(DF_IconKind_Clipboard, 0, "Copy ID")))
            {
              U32 ctrl_id = entity->ctrl_id;
              String8 string = push_str8f(scratch.arena, "%i", (int)ctrl_id);
              os_set_clipboard_text(string);
              ui_ctx_menu_close();
            }
            
            if(entity->kind == DF_EntityKind_Thread)
            {
              if(ui_clicked(df_icon_buttonf(DF_IconKind_Clipboard, 0, "Copy Instruction Pointer Address")))
              {
                CTRL_Entity *entity_ctrl = ctrl_entity_from_handle(d_state->ctrl_entity_store, entity->ctrl_handle);
                U64 rip = d_query_cached_rip_from_thread(entity_ctrl);
                String8 string = push_str8f(scratch.arena, "0x%I64x", rip);
                os_set_clipboard_text(string);
                ui_ctx_menu_close();
              }
            }
            
            if(entity->kind == DF_EntityKind_Thread)
            {
              if(ui_clicked(df_icon_buttonf(DF_IconKind_Clipboard, 0, "Copy Call Stack")))
              {
                DI_Scope *di_scope = di_scope_open();
                CTRL_Entity *entity_ctrl = ctrl_entity_from_handle(d_state->ctrl_entity_store, entity->ctrl_handle);
                CTRL_Entity *process = ctrl_entity_ancestor_from_kind(entity_ctrl, CTRL_EntityKind_Process);
                CTRL_Unwind base_unwind = d_query_cached_unwind_from_thread(entity_ctrl);
                D_Unwind rich_unwind = d_unwind_from_ctrl_unwind(scratch.arena, di_scope, process, &base_unwind);
                String8List lines = {0};
                for(U64 frame_idx = 0; frame_idx < rich_unwind.frames.concrete_frame_count; frame_idx += 1)
                {
                  D_UnwindFrame *concrete_frame = &rich_unwind.frames.v[frame_idx];
                  U64 rip_vaddr = regs_rip_from_arch_block(entity->arch, concrete_frame->regs);
                  CTRL_Entity *module = ctrl_module_from_process_vaddr(process, rip_vaddr);
                  RDI_Parsed *rdi = concrete_frame->rdi;
                  RDI_Procedure *procedure = concrete_frame->procedure;
                  for(D_UnwindInlineFrame *inline_frame = concrete_frame->last_inline_frame;
                      inline_frame != 0;
                      inline_frame = inline_frame->prev)
                  {
                    RDI_InlineSite *inline_site = inline_frame->inline_site;
                    String8 name = {0};
                    name.str = rdi_string_from_idx(rdi, inline_site->name_string_idx, &name.size);
                    str8_list_pushf(scratch.arena, &lines, "0x%I64x: [inlined] \"%S\"%s%S", rip_vaddr, name, module == &ctrl_entity_nil ? "" : " in ", module->string);
                  }
                  if(procedure != 0)
                  {
                    String8 name = {0};
                    name.str = rdi_name_from_procedure(rdi, procedure, &name.size);
                    str8_list_pushf(scratch.arena, &lines, "0x%I64x: \"%S\"%s%S", rip_vaddr, name, module == &ctrl_entity_nil ? "" : " in ", module->string);
                  }
                  else if(module != &ctrl_entity_nil)
                  {
                    str8_list_pushf(scratch.arena, &lines, "0x%I64x: [??? in %S]", rip_vaddr, module->string);
                  }
                  else
                  {
                    str8_list_pushf(scratch.arena, &lines, "0x%I64x: [??? in ???]", rip_vaddr);
                  }
                }
                StringJoin join = {0};
                join.sep = join.post = str8_lit("\n");
                String8 text = str8_list_join(scratch.arena, &lines, &join);
                os_set_clipboard_text(text);
                ui_ctx_menu_close();
                di_scope_close(di_scope);
              }
            }
            
            if(entity->kind == DF_EntityKind_Thread)
            {
              if(ui_clicked(df_icon_buttonf(DF_IconKind_FileOutline, 0, "Find")))
              {
                df_cmd(DF_CmdKind_FindThread, .entity = df_handle_from_entity(entity));
                ui_ctx_menu_close();
              }
            }
          }break;
          
          case DF_EntityKind_Module:
          {
            UI_Signal copy_full_path_sig = df_icon_buttonf(DF_IconKind_Clipboard, 0, "Copy Full Path");
            if(ui_clicked(copy_full_path_sig))
            {
              String8 string = entity->string;
              os_set_clipboard_text(string);
              ui_ctx_menu_close();
            }
            if(ui_hovering(copy_full_path_sig)) UI_Tooltip
            {
              String8 string = entity->string;
              ui_label(string);
            }
            if(ui_clicked(df_icon_buttonf(DF_IconKind_Clipboard, 0, "Copy Base Address")))
            {
              Rng1U64 vaddr_rng = entity->vaddr_rng;
              String8 string = push_str8f(scratch.arena, "0x%I64x", vaddr_rng.min);
              os_set_clipboard_text(string);
              ui_ctx_menu_close();
            }
            if(ui_clicked(df_icon_buttonf(DF_IconKind_Clipboard, 0, "Copy Address Range Size")))
            {
              Rng1U64 vaddr_rng = entity->vaddr_rng;
              String8 string = push_str8f(scratch.arena, "0x%I64x", dim_1u64(vaddr_rng));
              os_set_clipboard_text(string);
              ui_ctx_menu_close();
            }
          }break;
          
          case DF_EntityKind_Target:
          {
            if(ui_clicked(df_icon_buttonf(DF_IconKind_Play, 0, "Launch And Run")))
            {
              df_cmd(DF_CmdKind_LaunchAndRun, .entity = df_handle_from_entity(entity));
              ui_ctx_menu_close();
            }
            if(ui_clicked(df_icon_buttonf(DF_IconKind_PlayStepForward, 0, "Launch And Initialize")))
            {
              df_cmd(DF_CmdKind_LaunchAndInit, .entity = df_handle_from_entity(entity));
              ui_ctx_menu_close();
            }
          }break;
        }
        
        DF_Palette(DF_PaletteCode_Floating) ui_divider(ui_em(1.f, 1.f));
        
        // rjf: color editor
        {
          B32 entity_has_color = entity->flags & DF_EntityFlag_HasColor;
          if(entity_has_color)
          {
            UI_Padding(ui_em(1.5f, 1.f))
            {
              ui_set_next_pref_height(ui_em(9.f, 1.f));
              UI_Row UI_Padding(ui_pct(1, 0))
              {
                UI_PrefWidth(ui_em(1.5f, 1.f)) UI_PrefHeight(ui_em(9.f, 1.f)) UI_Column UI_PrefHeight(ui_em(1.5f, 0.f))
                {
                  Vec4F32 presets[] =
                  {
                    v4f32(1.0f, 0.2f, 0.1f, 1.0f),
                    v4f32(1.0f, 0.8f, 0.2f, 1.0f),
                    v4f32(0.3f, 0.8f, 0.2f, 1.0f),
                    v4f32(0.1f, 0.8f, 0.4f, 1.0f),
                    v4f32(0.1f, 0.6f, 0.8f, 1.0f),
                    v4f32(0.5f, 0.3f, 0.8f, 1.0f),
                    v4f32(0.8f, 0.3f, 0.5f, 1.0f),
                  };
                  UI_CornerRadius(ui_em(0.3f, 1.f).value)
                    for(U64 preset_idx = 0; preset_idx < ArrayCount(presets); preset_idx += 1)
                  {
                    ui_set_next_hover_cursor(OS_Cursor_HandPoint);
                    ui_set_next_palette(ui_build_palette(ui_top_palette(), .background = presets[preset_idx]));
                    UI_Box *box = ui_build_box_from_stringf(UI_BoxFlag_DrawBackground|
                                                            UI_BoxFlag_DrawBorder|
                                                            UI_BoxFlag_Clickable|
                                                            UI_BoxFlag_DrawHotEffects|
                                                            UI_BoxFlag_DrawActiveEffects,
                                                            "###color_preset_%i", (int)preset_idx);
                    UI_Signal sig = ui_signal_from_box(box);
                    if(ui_clicked(sig))
                    {
                      Vec3F32 hsv = hsv_from_rgb(v3f32(presets[preset_idx].x, presets[preset_idx].y, presets[preset_idx].z));
                      Vec4F32 hsva = v4f32(hsv.x, hsv.y, hsv.z, 1);
                      entity->color_hsva = hsva;
                    }
                    ui_spacer(ui_em(0.3f, 1.f));
                  }
                }
                
                ui_spacer(ui_em(0.75f, 1.f));
                
                UI_PrefWidth(ui_em(9.f, 1.f)) UI_PrefHeight(ui_em(9.f, 1.f))
                {
                  ui_sat_val_pickerf(entity->color_hsva.x, &entity->color_hsva.y, &entity->color_hsva.z, "###ent_satval_picker");
                }
                
                ui_spacer(ui_em(0.75f, 1.f));
                
                UI_PrefWidth(ui_em(1.5f, 1.f)) UI_PrefHeight(ui_em(9.f, 1.f))
                  ui_hue_pickerf(&entity->color_hsva.x, entity->color_hsva.y, entity->color_hsva.z, "###ent_hue_picker");
              }
            }
            
            UI_Row UI_Padding(ui_pct(1, 0)) UI_PrefWidth(ui_em(16.f, 1.f)) UI_CornerRadius(8.f) UI_TextAlignment(UI_TextAlign_Center)
              DF_Palette(DF_PaletteCode_Floating)
            {
              if(ui_clicked(df_icon_buttonf(DF_IconKind_Trash, 0, "Remove Color###color_toggle")))
              {
                entity->flags &= ~DF_EntityFlag_HasColor;
              }
            }
            
            ui_spacer(ui_em(1.5f, 1.f));
          }
          if(!entity_has_color && ui_clicked(df_icon_buttonf(DF_IconKind_Palette, 0, "Apply Color###color_toggle")))
          {
            df_entity_equip_color_rgba(entity, v4f32(1, 1, 1, 1));
          }
        }
      }
      
      //- rjf: auto-close tab ctx menu
      if(ui_ctx_menu_is_open(df_state->tab_ctx_menu_key))
      {
        DF_View *tab = df_view_from_handle(ws->tab_ctx_menu_view);
        if(df_view_is_nil(tab))
        {
          ui_ctx_menu_close();
        }
      }
      
      //- rjf: tab menu
      UI_CtxMenu(df_state->tab_ctx_menu_key) UI_PrefWidth(ui_em(40.f, 1.f)) UI_CornerRadius(0)
        DF_Palette(DF_PaletteCode_ImplicitButton)
      {
        DF_Panel *panel = df_panel_from_handle(ws->tab_ctx_menu_panel);
        DF_View *view = df_view_from_handle(ws->tab_ctx_menu_view);
        DF_IconKind view_icon = df_icon_kind_from_view(view);
        DR_FancyStringList fstrs = df_title_fstrs_from_view(scratch.arena, view, ui_top_palette()->text, ui_top_palette()->text_weak, ui_top_font_size());
        String8 file_path = d_file_path_from_eval_string(scratch.arena, str8(view->query_buffer, view->query_string_size));
        
        // rjf: title
        UI_Row
        {
          ui_spacer(ui_em(1.f, 1.f));
          DF_Font(DF_FontSlot_Icons)
            UI_FontSize(df_font_size_from_slot(DF_FontSlot_Icons))
            UI_PrefWidth(ui_em(2.f, 1.f))
            UI_PrefHeight(ui_pct(1, 0))
            UI_TextAlignment(UI_TextAlign_Center)
            UI_FlagsAdd(UI_BoxFlag_DrawTextWeak)
            ui_label(df_g_icon_kind_text_table[view_icon]);
          UI_PrefWidth(ui_text_dim(10, 1))
          {
            UI_Box *name_box = ui_build_box_from_key(UI_BoxFlag_DrawText, ui_key_zero());
            ui_box_equip_display_fancy_strings(name_box, &fstrs);
          }
        }
        
        DF_Palette(DF_PaletteCode_Floating) ui_divider(ui_em(1.f, 1.f));
        
        // rjf: copy name
        if(ui_clicked(df_icon_buttonf(DF_IconKind_Clipboard, 0, "Copy Name")))
        {
          os_set_clipboard_text(dr_string_from_fancy_string_list(scratch.arena, &fstrs));
          ui_ctx_menu_close();
        }
        
        // rjf: copy full path
        if(file_path.size != 0)
        {
          UI_Signal copy_full_path_sig = df_icon_buttonf(DF_IconKind_Clipboard, 0, "Copy Full Path");
          String8 full_path = path_normalized_from_string(scratch.arena, file_path);
          if(ui_clicked(copy_full_path_sig))
          {
            os_set_clipboard_text(full_path);
            ui_ctx_menu_close();
          }
          if(ui_hovering(copy_full_path_sig)) UI_Tooltip
          {
            ui_label(full_path);
          }
        }
        
        // rjf: show in explorer
        if(file_path.size != 0)
        {
          UI_Signal sig = df_icon_buttonf(DF_IconKind_FolderClosedFilled, 0, "Show In Explorer");
          if(ui_clicked(sig))
          {
            String8 full_path = path_normalized_from_string(scratch.arena, file_path);
            os_show_in_filesystem_ui(full_path);
            ui_ctx_menu_close();
          }
        }
        
        // rjf: filter controls
        if(view->spec->info.flags & DF_ViewSpecFlag_CanFilter)
        {
          if(ui_clicked(df_cmd_spec_button(df_cmd_kind_info_table[DF_CmdKind_Filter].display_name)))
          {
            df_cmd(DF_CmdKind_Filter, .view = df_handle_from_view(view));
            ui_ctx_menu_close();
          }
          if(ui_clicked(df_cmd_spec_button(df_cmd_kind_info_table[DF_CmdKind_ClearFilter].display_name)))
          {
            df_cmd(DF_CmdKind_ClearFilter, .view = df_handle_from_view(view));
            ui_ctx_menu_close();
          }
        }
        
        // rjf: close tab
        if(ui_clicked(df_icon_buttonf(DF_IconKind_X, 0, "Close Tab")))
        {
          df_cmd(DF_CmdKind_CloseTab, .panel = df_handle_from_panel(panel), .view = df_handle_from_view(view));
          ui_ctx_menu_close();
        }
        
        // rjf: param tree editing
        UI_TextPadding(ui_top_font_size()*1.5f) DF_Font(DF_FontSlot_Code)
        {
          Temp scratch = scratch_begin(0, 0);
          D_ViewRuleSpec *core_vr_spec = d_view_rule_spec_from_string(view->spec->info.name);
          String8 schema_string = core_vr_spec->info.schema;
          MD_TokenizeResult schema_tokenize = md_tokenize_from_text(scratch.arena, schema_string);
          MD_ParseResult schema_parse = md_parse_from_text_tokens(scratch.arena, str8_zero(), schema_string, schema_tokenize.tokens);
          MD_Node *schema_root = schema_parse.root->first;
          if(!md_node_is_nil(schema_root))
          {
            if(!md_node_is_nil(schema_root->first))
            {
              DF_Palette(DF_PaletteCode_Floating) ui_divider(ui_em(1.f, 1.f));
            }
            for(MD_EachNode(key, schema_root->first))
            {
              UI_Row
              {
                MD_Node *params = view->params_roots[view->params_write_gen%ArrayCount(view->params_roots)];
                MD_Node *param_tree = md_child_from_string(params, key->string, 0);
                String8 pre_edit_value = md_string_from_children(scratch.arena, param_tree);
                UI_PrefWidth(ui_em(10.f, 1.f)) ui_label(key->string);
                UI_Signal sig = df_line_editf(DF_LineEditFlag_Border|DF_LineEditFlag_CodeContents, 0, 0, &ws->tab_ctx_menu_input_cursor, &ws->tab_ctx_menu_input_mark, ws->tab_ctx_menu_input_buffer, sizeof(ws->tab_ctx_menu_input_buffer), &ws->tab_ctx_menu_input_size, 0, pre_edit_value, "%S##view_param", key->string);
                if(ui_committed(sig))
                {
                  String8 new_string = str8(ws->tab_ctx_menu_input_buffer, ws->tab_ctx_menu_input_size);
                  df_view_store_param(view, key->string, new_string);
                }
              }
            }
          }
          scratch_end(scratch);
        }
        
      }
      
      scratch_end(scratch);
    }
    
    ////////////////////////////
    //- rjf: confirmation popup
    //
    {
      if(df_state->confirm_t > 0.005f) UI_TextAlignment(UI_TextAlign_Center) UI_Focus(df_state->confirm_active ? UI_FocusKind_Root : UI_FocusKind_Off)
      {
        Vec2F32 window_dim = dim_2f32(window_rect);
        UI_Box *bg_box = &ui_g_nil_box;
        UI_Palette *palette = ui_build_palette(df_palette_from_code(DF_PaletteCode_Floating));
        palette->background.w *= df_state->confirm_t;
        UI_Rect(window_rect)
          UI_ChildLayoutAxis(Axis2_X)
          UI_Focus(UI_FocusKind_On)
          UI_BlurSize(10*df_state->confirm_t)
          UI_Palette(palette)
        {
          bg_box = ui_build_box_from_stringf(UI_BoxFlag_FixedSize|
                                             UI_BoxFlag_Floating|
                                             UI_BoxFlag_Clickable|
                                             UI_BoxFlag_Scroll|
                                             UI_BoxFlag_DefaultFocusNav|
                                             UI_BoxFlag_DisableFocusOverlay|
                                             UI_BoxFlag_DrawBackgroundBlur|
                                             UI_BoxFlag_DrawBackground, "###confirm_popup_%p", ws);
        }
        if(df_state->confirm_active) UI_Parent(bg_box) UI_Transparency(1-df_state->confirm_t)
        {
          ui_ctx_menu_close();
          UI_WidthFill UI_PrefHeight(ui_children_sum(1.f)) UI_Column UI_Padding(ui_pct(1, 0))
          {
            UI_TextRasterFlags(df_raster_flags_from_slot(DF_FontSlot_Main)) UI_FontSize(ui_top_font_size()*2.f) UI_PrefHeight(ui_em(3.f, 1.f)) ui_label(df_state->confirm_title);
            UI_PrefHeight(ui_em(3.f, 1.f)) UI_FlagsAdd(UI_BoxFlag_DrawTextWeak) ui_label(df_state->confirm_desc);
            ui_spacer(ui_em(1.5f, 1.f));
            UI_Row UI_Padding(ui_pct(1.f, 0.f)) UI_WidthFill UI_PrefHeight(ui_em(5.f, 1.f))
            {
              UI_CornerRadius00(ui_top_font_size()*0.25f)
                UI_CornerRadius01(ui_top_font_size()*0.25f)
                DF_Palette(DF_PaletteCode_NeutralPopButton)
                if(ui_clicked(ui_buttonf("OK")) || (ui_key_match(bg_box->default_nav_focus_hot_key, ui_key_zero()) && ui_slot_press(UI_EventActionSlot_Accept)))
              {
                df_cmd(DF_CmdKind_ConfirmAccept);
              }
              UI_CornerRadius10(ui_top_font_size()*0.25f)
                UI_CornerRadius11(ui_top_font_size()*0.25f)
                if(ui_clicked(ui_buttonf("Cancel")) || ui_slot_press(UI_EventActionSlot_Cancel))
              {
                df_cmd(DF_CmdKind_ConfirmCancel);
              }
            }
            ui_spacer(ui_em(3.f, 1.f));
          }
        }
        ui_signal_from_box(bg_box);
      }
    }
    
    ////////////////////////////
    //- rjf: build auto-complete lister
    //
    ProfScope("build autocomplete lister")
      if(!ws->autocomp_force_closed && !ui_key_match(ws->autocomp_root_key, ui_key_zero()) && ws->autocomp_last_frame_idx+1 >= df_state->frame_index)
    {
      String8 query = str8(ws->autocomp_lister_query_buffer, ws->autocomp_lister_query_size);
      UI_Box *autocomp_root_box = ui_box_from_key(ws->autocomp_root_key);
      if(!ui_box_is_nil(autocomp_root_box))
      {
        Temp scratch = scratch_begin(0, 0);
        
        //- rjf: unpack lister params
        CTRL_Entity *thread = ctrl_entity_from_handle(d_state->ctrl_entity_store, df_base_regs()->thread);
        U64 thread_rip_vaddr = d_query_cached_rip_from_thread_unwind(thread, df_base_regs()->unwind_count);
        CTRL_Entity *process = ctrl_entity_ancestor_from_kind(thread, CTRL_EntityKind_Process);
        CTRL_Entity *module = ctrl_module_from_process_vaddr(process, thread_rip_vaddr);
        U64 thread_rip_voff = ctrl_voff_from_vaddr(module, thread_rip_vaddr);
        DI_Key dbgi_key = ctrl_dbgi_key_from_module(module);
        
        //- rjf: gather lister items
        DF_AutoCompListerItemChunkList item_list = {0};
        {
          //- rjf: gather locals
          if(ws->autocomp_lister_params.flags & DF_AutoCompListerFlag_Locals)
          {
            E_String2NumMap *locals_map = d_query_cached_locals_map_from_dbgi_key_voff(&dbgi_key, thread_rip_voff);
            E_String2NumMap *member_map = d_query_cached_member_map_from_dbgi_key_voff(&dbgi_key, thread_rip_voff);
            for(E_String2NumMapNode *n = locals_map->first; n != 0; n = n->order_next)
            {
              DF_AutoCompListerItem item = {0};
              {
                item.string      = n->string;
                item.kind_string = str8_lit("Local");
                item.matches     = fuzzy_match_find(scratch.arena, query, n->string);
              }
              if(query.size == 0 || item.matches.count != 0)
              {
                df_autocomp_lister_item_chunk_list_push(scratch.arena, &item_list, 256, &item);
              }
            }
            for(E_String2NumMapNode *n = member_map->first; n != 0; n = n->order_next)
            {
              DF_AutoCompListerItem item = {0};
              {
                item.string      = n->string;
                item.kind_string = str8_lit("Local (Member)");
                item.matches     = fuzzy_match_find(scratch.arena, query, n->string);
              }
              if(query.size == 0 || item.matches.count != 0)
              {
                df_autocomp_lister_item_chunk_list_push(scratch.arena, &item_list, 256, &item);
              }
            }
          }
          
          //- rjf: gather registers
          if(ws->autocomp_lister_params.flags & DF_AutoCompListerFlag_Registers)
          {
            Arch arch = thread->arch;
            U64 reg_names_count = regs_reg_code_count_from_arch(arch);
            U64 alias_names_count = regs_alias_code_count_from_arch(arch);
            String8 *reg_names = regs_reg_code_string_table_from_arch(arch);
            String8 *alias_names = regs_alias_code_string_table_from_arch(arch);
            for(U64 idx = 0; idx < reg_names_count; idx += 1)
            {
              if(reg_names[idx].size != 0)
              {
                DF_AutoCompListerItem item = {0};
                {
                  item.string      = reg_names[idx];
                  item.kind_string = str8_lit("Register");
                  item.matches     = fuzzy_match_find(scratch.arena, query, reg_names[idx]);
                }
                if(query.size == 0 || item.matches.count != 0)
                {
                  df_autocomp_lister_item_chunk_list_push(scratch.arena, &item_list, 256, &item);
                }
              }
            }
            for(U64 idx = 0; idx < alias_names_count; idx += 1)
            {
              if(alias_names[idx].size != 0)
              {
                DF_AutoCompListerItem item = {0};
                {
                  item.string      = alias_names[idx];
                  item.kind_string = str8_lit("Reg. Alias");
                  item.matches     = fuzzy_match_find(scratch.arena, query, alias_names[idx]);
                }
                if(query.size == 0 || item.matches.count != 0)
                {
                  df_autocomp_lister_item_chunk_list_push(scratch.arena, &item_list, 256, &item);
                }
              }
            }
          }
          
          //- rjf: gather view rules
          if(ws->autocomp_lister_params.flags & DF_AutoCompListerFlag_ViewRules)
          {
            for(U64 slot_idx = 0; slot_idx < d_state->view_rule_spec_table_size; slot_idx += 1)
            {
              for(D_ViewRuleSpec *spec = d_state->view_rule_spec_table[slot_idx]; spec != 0 && spec != &d_nil_core_view_rule_spec; spec = spec->hash_next)
              {
                DF_AutoCompListerItem item = {0};
                {
                  item.string      = spec->info.string;
                  item.kind_string = str8_lit("View Rule");
                  item.matches     = fuzzy_match_find(scratch.arena, query, spec->info.string);
                }
                if(query.size == 0 || item.matches.count != 0)
                {
                  df_autocomp_lister_item_chunk_list_push(scratch.arena, &item_list, 256, &item);
                }
              }
            }
          }
          
          //- rjf: gather members
          if(ws->autocomp_lister_params.flags & DF_AutoCompListerFlag_Members)
          {
            
          }
          
          //- rjf: gather languages
          if(ws->autocomp_lister_params.flags & DF_AutoCompListerFlag_Languages)
          {
            for(EachNonZeroEnumVal(TXT_LangKind, lang))
            {
              DF_AutoCompListerItem item = {0};
              {
                item.string      = txt_extension_from_lang_kind(lang);
                item.kind_string = str8_lit("Language");
                item.matches     = fuzzy_match_find(scratch.arena, query, item.string);
              }
              if(item.string.size != 0 && (query.size == 0 || item.matches.count != 0))
              {
                df_autocomp_lister_item_chunk_list_push(scratch.arena, &item_list, 256, &item);
              }
            }
          }
          
          //- rjf: gather architectures
          if(ws->autocomp_lister_params.flags & DF_AutoCompListerFlag_Architectures)
          {
            for(EachNonZeroEnumVal(Arch, arch))
            {
              DF_AutoCompListerItem item = {0};
              {
                item.string      = string_from_arch(arch);
                item.kind_string = str8_lit("Arch");
                item.matches     = fuzzy_match_find(scratch.arena, query, item.string);
              }
              if(query.size == 0 || item.matches.count != 0)
              {
                df_autocomp_lister_item_chunk_list_push(scratch.arena, &item_list, 256, &item);
              }
            }
          }
          
          //- rjf: gather tex2dformats
          if(ws->autocomp_lister_params.flags & DF_AutoCompListerFlag_Tex2DFormats)
          {
            for(EachEnumVal(R_Tex2DFormat, fmt))
            {
              DF_AutoCompListerItem item = {0};
              {
                item.string      = lower_from_str8(scratch.arena, r_tex2d_format_display_string_table[fmt]);
                item.kind_string = str8_lit("Format");
                item.matches     = fuzzy_match_find(scratch.arena, query, item.string);
              }
              if(query.size == 0 || item.matches.count != 0)
              {
                df_autocomp_lister_item_chunk_list_push(scratch.arena, &item_list, 256, &item);
              }
            }
          }
          
          //- rjf: gather view rule params
          if(ws->autocomp_lister_params.flags & DF_AutoCompListerFlag_ViewRuleParams)
          {
            for(String8Node *n = ws->autocomp_lister_params.strings.first; n != 0; n = n->next)
            {
              String8 string = n->string;
              DF_AutoCompListerItem item = {0};
              {
                item.string      = string;
                item.kind_string = str8_lit("Parameter");
                item.matches     = fuzzy_match_find(scratch.arena, query, item.string);
              }
              if(query.size == 0 || item.matches.count != 0)
              {
                df_autocomp_lister_item_chunk_list_push(scratch.arena, &item_list, 256, &item);
              }
            }
          }
        }
        
        //- rjf: lister item list -> sorted array
        DF_AutoCompListerItemArray item_array = df_autocomp_lister_item_array_from_chunk_list(scratch.arena, &item_list);
        df_autocomp_lister_item_array_sort__in_place(&item_array);
        
        //- rjf: animate
        {
          // rjf: animate target # of rows
          {
            F32 rate = df_setting_val_from_code(DF_SettingCode_MenuAnimations).s32 ? (1 - pow_f32(2, (-60.f * df_state->frame_dt))) : 1.f;
            F32 target = Min((F32)item_array.count, 16.f);
            if(abs_f32(target - ws->autocomp_num_visible_rows_t) > 0.01f)
            {
              df_request_frame();
            }
            ws->autocomp_num_visible_rows_t += (target - ws->autocomp_num_visible_rows_t) * rate;
            if(abs_f32(target - ws->autocomp_num_visible_rows_t) <= 0.02f)
            {
              ws->autocomp_num_visible_rows_t = target;
            }
          }
          
          // rjf: animate open
          {
            F32 rate = df_setting_val_from_code(DF_SettingCode_MenuAnimations).s32 ? 1 - pow_f32(2, (-60.f * df_state->frame_dt)) : 1.f;
            F32 diff = 1.f-ws->autocomp_open_t;
            ws->autocomp_open_t += diff*rate;
            if(abs_f32(diff) < 0.05f)
            {
              ws->autocomp_open_t = 1.f;
            }
            else
            {
              df_request_frame();
            }
          }
        }
        
        //- rjf: build
        if(item_array.count != 0)
        {
          F32 row_height_px = floor_f32(ui_top_font_size()*2.5f);
          ui_set_next_fixed_x(autocomp_root_box->rect.x0);
          ui_set_next_fixed_y(autocomp_root_box->rect.y1);
          ui_set_next_pref_width(ui_em(30.f, 1.f));
          ui_set_next_pref_height(ui_px(row_height_px*ws->autocomp_num_visible_rows_t + ui_top_font_size()*2.f, 1.f));
          ui_set_next_child_layout_axis(Axis2_Y);
          ui_set_next_corner_radius_01(ui_top_font_size()*0.25f);
          ui_set_next_corner_radius_11(ui_top_font_size()*0.25f);
          ui_set_next_corner_radius_10(ui_top_font_size()*0.25f);
          UI_Focus(UI_FocusKind_On)
            UI_Squish(0.25f-0.25f*ws->autocomp_open_t)
            UI_Transparency(1.f-ws->autocomp_open_t)
            DF_Palette(DF_PaletteCode_Floating)
          {
            autocomp_box = ui_build_box_from_stringf(UI_BoxFlag_DefaultFocusNavY|
                                                     UI_BoxFlag_Clickable|
                                                     UI_BoxFlag_Clip|
                                                     UI_BoxFlag_RoundChildrenByParent|
                                                     UI_BoxFlag_DisableFocusOverlay|
                                                     UI_BoxFlag_DrawBorder|
                                                     UI_BoxFlag_DrawBackgroundBlur|
                                                     UI_BoxFlag_DrawDropShadow|
                                                     UI_BoxFlag_DrawBackground,
                                                     "autocomp_box");
            if(ws->autocomp_query_dirty)
            {
              ws->autocomp_query_dirty = 0;
              autocomp_box->default_nav_focus_hot_key = autocomp_box->default_nav_focus_active_key = autocomp_box->default_nav_focus_next_hot_key = autocomp_box->default_nav_focus_next_active_key = ui_key_zero();
            }
          }
          UI_Parent(autocomp_box)
            UI_WidthFill
            UI_PrefHeight(ui_px(row_height_px, 1.f))
            DF_Font(DF_FontSlot_Code)
            UI_HoverCursor(OS_Cursor_HandPoint)
            UI_Focus(UI_FocusKind_Null)
            DF_Palette(DF_PaletteCode_ImplicitButton)
            UI_Padding(ui_em(1.f, 1.f))
          {
            for(U64 idx = 0; idx < item_array.count; idx += 1)
            {
              DF_AutoCompListerItem *item = &item_array.v[idx];
              UI_Box *item_box = ui_build_box_from_stringf(UI_BoxFlag_DrawBorder|UI_BoxFlag_DrawBackground|UI_BoxFlag_DrawHotEffects|UI_BoxFlag_DrawActiveEffects|UI_BoxFlag_MouseClickable, "autocomp_%I64x", idx);
              UI_Parent(item_box) UI_Padding(ui_em(1.f, 1.f))
              {
                UI_WidthFill
                {
                  UI_Box *box = ui_label(item->string).box;
                  ui_box_equip_fuzzy_match_ranges(box, &item->matches);
                }
                DF_Font(DF_FontSlot_Main)
                  UI_PrefWidth(ui_text_dim(10, 1))
                  UI_FlagsAdd(UI_BoxFlag_DrawTextWeak)
                  ui_label(item->kind_string);
              }
              UI_Signal item_sig = ui_signal_from_box(item_box);
              if(ui_clicked(item_sig))
              {
                UI_Event move_back_evt = zero_struct;
                move_back_evt.kind = UI_EventKind_Navigate;
                move_back_evt.flags = UI_EventFlag_KeepMark;
                move_back_evt.delta_2s32.x = -(S32)query.size;
                ui_event_list_push(ui_build_arena(), &ws->ui_events, &move_back_evt);
                UI_Event paste_evt = zero_struct;
                paste_evt.kind = UI_EventKind_Text;
                paste_evt.string = item->string;
                ui_event_list_push(ui_build_arena(), &ws->ui_events, &paste_evt);
                autocomp_box->default_nav_focus_hot_key = autocomp_box->default_nav_focus_active_key = autocomp_box->default_nav_focus_next_hot_key = autocomp_box->default_nav_focus_next_active_key = ui_key_zero();
              }
              else if(item_box->flags & UI_BoxFlag_FocusHot && !(item_box->flags & UI_BoxFlag_FocusHotDisabled))
              {
                UI_Event evt = zero_struct;
                evt.kind   = UI_EventKind_AutocompleteHint;
                evt.string = item->string;
                ui_event_list_push(ui_build_arena(), &ws->ui_events, &evt);
              }
            }
          }
        }
        
        scratch_end(scratch);
      }
    }
    
    ////////////////////////////
    //- rjf: top bar
    //
    ProfScope("build top bar")
    {
      os_window_clear_custom_border_data(ws->os);
      os_window_push_custom_edges(ws->os, window_edge_px);
      os_window_push_custom_title_bar(ws->os, dim_2f32(top_bar_rect).y);
      ui_set_next_flags(UI_BoxFlag_DefaultFocusNav|UI_BoxFlag_DisableFocusOverlay);
      DF_Palette(DF_PaletteCode_MenuBar)
        UI_Focus((ws->menu_bar_focused && window_is_focused && !ui_any_ctx_menu_is_open() && !ws->hover_eval_focused) ? UI_FocusKind_On : UI_FocusKind_Null)
        UI_Pane(top_bar_rect, str8_lit("###top_bar"))
        UI_WidthFill UI_Row
        UI_Focus(UI_FocusKind_Null)
      {
        UI_Key menu_bar_group_key = ui_key_from_string(ui_key_zero(), str8_lit("###top_bar_group"));
        MemoryZeroArray(ui_top_parent()->parent->corner_radii);
        
        //- rjf: left column
        ui_set_next_flags(UI_BoxFlag_Clip|UI_BoxFlag_ViewScrollX|UI_BoxFlag_ViewClamp);
        UI_WidthFill UI_NamedRow(str8_lit("###menu_bar"))
        {
          //- rjf: icon
          UI_Padding(ui_em(0.5f, 1.f))
          {
            UI_PrefWidth(ui_px(dim_2f32(top_bar_rect).y - ui_top_font_size()*0.8f, 1.f))
              UI_Column
              UI_Padding(ui_em(0.4f, 1.f))
              UI_HeightFill
            {
              R_Handle texture = df_state->icon_texture;
              Vec2S32 texture_dim = r_size_from_tex2d(texture);
              ui_image(texture, R_Tex2DSampleKind_Linear, r2f32p(0, 0, texture_dim.x, texture_dim.y), v4f32(1, 1, 1, 1), 0, str8_lit(""));
            }
          }
          
          //- rjf: menu items
          ui_set_next_flags(UI_BoxFlag_DrawBackground);
          UI_PrefWidth(ui_children_sum(1)) UI_Row UI_PrefWidth(ui_text_dim(20, 1)) UI_GroupKey(menu_bar_group_key)
          {
            // rjf: file menu
            UI_Key file_menu_key = ui_key_from_string(ui_key_zero(), str8_lit("_file_menu_key_"));
            DF_Palette(DF_PaletteCode_Floating)
              UI_CtxMenu(file_menu_key)
              UI_PrefWidth(ui_em(50.f, 1.f))
              DF_Palette(DF_PaletteCode_ImplicitButton)
            {
              String8 cmds[] =
              {
                df_cmd_kind_info_table[DF_CmdKind_Open].string,
                df_cmd_kind_info_table[DF_CmdKind_OpenUser].string,
                df_cmd_kind_info_table[DF_CmdKind_OpenProject].string,
                df_cmd_kind_info_table[DF_CmdKind_OpenRecentProject].string,
                df_cmd_kind_info_table[DF_CmdKind_Exit].string,
              };
              U32 codepoints[] =
              {
                'o',
                'u',
                'p',
                'r',
                'x',
              };
              Assert(ArrayCount(codepoints) == ArrayCount(cmds));
              df_cmd_list_menu_buttons(ArrayCount(cmds), cmds, codepoints);
            }
            
            // rjf: window menu
            UI_Key window_menu_key = ui_key_from_string(ui_key_zero(), str8_lit("_window_menu_key_"));
            DF_Palette(DF_PaletteCode_Floating)
              UI_CtxMenu(window_menu_key)
              UI_PrefWidth(ui_em(50.f, 1.f))
              DF_Palette(DF_PaletteCode_ImplicitButton)
            {
              String8 cmds[] =
              {
                df_cmd_kind_info_table[DF_CmdKind_OpenWindow].string,
                df_cmd_kind_info_table[DF_CmdKind_CloseWindow].string,
                df_cmd_kind_info_table[DF_CmdKind_ToggleFullscreen].string,
              };
              U32 codepoints[] =
              {
                'w',
                'c',
                'f',
              };
              Assert(ArrayCount(codepoints) == ArrayCount(cmds));
              df_cmd_list_menu_buttons(ArrayCount(cmds), cmds, codepoints);
            }
            
            // rjf: panel menu
            UI_Key panel_menu_key = ui_key_from_string(ui_key_zero(), str8_lit("_panel_menu_key_"));
            DF_Palette(DF_PaletteCode_Floating)
              UI_CtxMenu(panel_menu_key)
              UI_PrefWidth(ui_em(50.f, 1.f))
              DF_Palette(DF_PaletteCode_ImplicitButton)
            {
              String8 cmds[] =
              {
                df_cmd_kind_info_table[DF_CmdKind_NewPanelRight].string,
                df_cmd_kind_info_table[DF_CmdKind_NewPanelDown].string,
                df_cmd_kind_info_table[DF_CmdKind_ClosePanel].string,
                df_cmd_kind_info_table[DF_CmdKind_RotatePanelColumns].string,
                df_cmd_kind_info_table[DF_CmdKind_NextPanel].string,
                df_cmd_kind_info_table[DF_CmdKind_PrevPanel].string,
                df_cmd_kind_info_table[DF_CmdKind_CloseTab].string,
                df_cmd_kind_info_table[DF_CmdKind_NextTab].string,
                df_cmd_kind_info_table[DF_CmdKind_PrevTab].string,
                df_cmd_kind_info_table[DF_CmdKind_TabBarTop].string,
                df_cmd_kind_info_table[DF_CmdKind_TabBarBottom].string,
                df_cmd_kind_info_table[DF_CmdKind_ResetToDefaultPanels].string,
                df_cmd_kind_info_table[DF_CmdKind_ResetToCompactPanels].string,
              };
              U32 codepoints[] =
              {
                'r',
                'd',
                'x',
                'c',
                'n',
                'p',
                't',
                'b',
                'v',
                0,
                0,
                0,
                0,
              };
              Assert(ArrayCount(codepoints) == ArrayCount(cmds));
              df_cmd_list_menu_buttons(ArrayCount(cmds), cmds, codepoints);
            }
            
            // rjf: view menu
            UI_Key view_menu_key = ui_key_from_string(ui_key_zero(), str8_lit("_view_menu_key_"));
            DF_Palette(DF_PaletteCode_Floating)
              UI_CtxMenu(view_menu_key)
              UI_PrefWidth(ui_em(50.f, 1.f))
              DF_Palette(DF_PaletteCode_ImplicitButton)
            {
              String8 cmds[] =
              {
                df_cmd_kind_info_table[DF_CmdKind_Targets].string,
                df_cmd_kind_info_table[DF_CmdKind_Scheduler].string,
                df_cmd_kind_info_table[DF_CmdKind_CallStack].string,
                df_cmd_kind_info_table[DF_CmdKind_Modules].string,
                df_cmd_kind_info_table[DF_CmdKind_Output].string,
                df_cmd_kind_info_table[DF_CmdKind_Memory].string,
                df_cmd_kind_info_table[DF_CmdKind_Disassembly].string,
                df_cmd_kind_info_table[DF_CmdKind_Watch].string,
                df_cmd_kind_info_table[DF_CmdKind_Locals].string,
                df_cmd_kind_info_table[DF_CmdKind_Registers].string,
                df_cmd_kind_info_table[DF_CmdKind_Globals].string,
                df_cmd_kind_info_table[DF_CmdKind_ThreadLocals].string,
                df_cmd_kind_info_table[DF_CmdKind_Types].string,
                df_cmd_kind_info_table[DF_CmdKind_Procedures].string,
                df_cmd_kind_info_table[DF_CmdKind_Breakpoints].string,
                df_cmd_kind_info_table[DF_CmdKind_WatchPins].string,
                df_cmd_kind_info_table[DF_CmdKind_FilePathMap].string,
                df_cmd_kind_info_table[DF_CmdKind_Settings].string,
                df_cmd_kind_info_table[DF_CmdKind_ExceptionFilters].string,
                df_cmd_kind_info_table[DF_CmdKind_GettingStarted].string,
              };
              U32 codepoints[] =
              {
                't',
                's',
                'k',
                'd',
                'o',
                'm',
                'y',
                'w',
                'l',
                'r',
                0,
                0,
                0,
                0,
                'b',
                'h',
                'p',
                'e',
                'g',
                0,
              };
              Assert(ArrayCount(codepoints) == ArrayCount(cmds));
              df_cmd_list_menu_buttons(ArrayCount(cmds), cmds, codepoints);
            }
            
            // rjf: targets menu
            UI_Key targets_menu_key = ui_key_from_string(ui_key_zero(), str8_lit("_targets_menu_key_"));
            DF_Palette(DF_PaletteCode_Floating)
              UI_CtxMenu(targets_menu_key)
              UI_PrefWidth(ui_em(50.f, 1.f))
              DF_Palette(DF_PaletteCode_ImplicitButton)
            {
              Temp scratch = scratch_begin(0, 0);
              String8 cmds[] =
              {
                df_cmd_kind_info_table[DF_CmdKind_AddTarget].string,
                df_cmd_kind_info_table[DF_CmdKind_EditTarget].string,
                df_cmd_kind_info_table[DF_CmdKind_RemoveTarget].string,
              };
              U32 codepoints[] =
              {
                'a',
                'e',
                'r',
              };
              Assert(ArrayCount(codepoints) == ArrayCount(cmds));
              df_cmd_list_menu_buttons(ArrayCount(cmds), cmds, codepoints);
              DF_Palette(DF_PaletteCode_Floating) ui_divider(ui_em(1.f, 1.f));
              DF_EntityList targets_list = d_query_cached_entity_list_with_kind(DF_EntityKind_Target);
              for(DF_EntityNode *n = targets_list.first; n != 0; n = n->next)
              {
                DF_Entity *target = n->entity;
                UI_Palette *palette = ui_top_palette();
                if(target->flags & DF_EntityFlag_HasColor)
                {
                  palette = ui_build_palette(ui_top_palette(), .text = df_rgba_from_entity(target));
                }
                String8 target_name = df_display_string_from_entity(scratch.arena, target);
                UI_Signal sig = {0};
                UI_Palette(palette) sig = df_icon_buttonf(DF_IconKind_Target, 0, "%S##%p", target_name, target);
                if(ui_clicked(sig))
                {
                  df_cmd(DF_CmdKind_EditTarget, .entity = df_handle_from_entity(target));
                  ui_ctx_menu_close();
                  ws->menu_bar_focused = 0;
                }
              }
              scratch_end(scratch);
            }
            
            // rjf: ctrl menu
            UI_Key ctrl_menu_key = ui_key_from_string(ui_key_zero(), str8_lit("_ctrl_menu_key_"));
            DF_Palette(DF_PaletteCode_Floating)
              UI_CtxMenu(ctrl_menu_key)
              UI_PrefWidth(ui_em(50.f, 1.f))
              DF_Palette(DF_PaletteCode_ImplicitButton)
            {
              String8 cmds[] =
              {
                df_cmd_kind_info_table[D_CmdKind_Run].string,
                df_cmd_kind_info_table[D_CmdKind_KillAll].string,
                df_cmd_kind_info_table[D_CmdKind_Restart].string,
                df_cmd_kind_info_table[D_CmdKind_Halt].string,
                df_cmd_kind_info_table[D_CmdKind_SoftHaltRefresh].string,
                df_cmd_kind_info_table[D_CmdKind_StepInto].string,
                df_cmd_kind_info_table[D_CmdKind_StepOver].string,
                df_cmd_kind_info_table[D_CmdKind_StepOut].string,
                df_cmd_kind_info_table[D_CmdKind_Attach].string,
              };
              U32 codepoints[] =
              {
                'r',
                'k',
                's',
                'h',
                'f',
                'i',
                'o',
                't',
                'a',
              };
              Assert(ArrayCount(codepoints) == ArrayCount(cmds));
              df_cmd_list_menu_buttons(ArrayCount(cmds), cmds, codepoints);
            }
            
            // rjf: help menu
            UI_Key help_menu_key = ui_key_from_string(ui_key_zero(), str8_lit("_help_menu_key_"));
            DF_Palette(DF_PaletteCode_Floating)
              UI_CtxMenu(help_menu_key)
              UI_PrefWidth(ui_em(50.f, 1.f))
              DF_Palette(DF_PaletteCode_ImplicitButton)
            {
              UI_Row UI_TextAlignment(UI_TextAlign_Center) UI_FlagsAdd(UI_BoxFlag_DrawTextWeak)
                ui_label(str8_lit(BUILD_TITLE_STRING_LITERAL));
              UI_PrefHeight(ui_children_sum(1)) UI_Row UI_Padding(ui_pct(1, 0))
              {
                R_Handle texture = df_state->icon_texture;
                Vec2S32 texture_dim = r_size_from_tex2d(texture);
                UI_PrefWidth(ui_px(ui_top_font_size()*10.f, 1.f))
                  UI_PrefHeight(ui_px(ui_top_font_size()*10.f, 1.f))
                  ui_image(texture, R_Tex2DSampleKind_Linear, r2f32p(0, 0, texture_dim.x, texture_dim.y), v4f32(1, 1, 1, 1), 0, str8_lit(""));
              }
              ui_spacer(ui_em(0.25f, 1.f));
              UI_Row
                UI_PrefWidth(ui_text_dim(10, 1))
                UI_TextAlignment(UI_TextAlign_Center)
                UI_Padding(ui_pct(1, 0))
              {
                ui_labelf("Search for commands by pressing ");
                UI_Flags(UI_BoxFlag_DrawBorder)
                  UI_TextAlignment(UI_TextAlign_Center)
                  df_cmd_binding_buttons(df_cmd_kind_info_table[DF_CmdKind_RunCommand].string);
              }
              ui_spacer(ui_em(0.25f, 1.f));
              UI_Row UI_TextAlignment(UI_TextAlign_Center) ui_label(str8_lit("Submit issues to the GitHub at:"));
              UI_TextAlignment(UI_TextAlign_Center)
              {
                UI_Signal url_sig = ui_buttonf("github.com/EpicGames/raddebugger");
                if(ui_hovering(url_sig)) UI_Tooltip
                {
                  ui_labelf("Copy To Clipboard");
                }
                if(ui_clicked(url_sig))
                {
                  os_set_clipboard_text(str8_lit("https://github.com/EpicGames/raddebugger"));
                }
              }
            }
            
            // rjf: buttons
            UI_TextAlignment(UI_TextAlign_Center) UI_HeightFill
            {
              // rjf: set up table
              struct
              {
                String8 name;
                U32 codepoint;
                OS_Key key;
                UI_Key menu_key;
              }
              items[] =
              {
                {str8_lit("File"),     'f', OS_Key_F, file_menu_key},
                {str8_lit("Window"),   'w', OS_Key_W, window_menu_key},
                {str8_lit("Panel"),    'p', OS_Key_P, panel_menu_key},
                {str8_lit("View"),     'v', OS_Key_V, view_menu_key},
                {str8_lit("Targets"),  't', OS_Key_T, targets_menu_key},
                {str8_lit("Control"),  'c', OS_Key_C, ctrl_menu_key},
                {str8_lit("Help"),     'h', OS_Key_H, help_menu_key},
              };
              
              // rjf: determine if one of the menus is already open
              B32 menu_open = 0;
              U64 open_menu_idx = 0;
              for(U64 idx = 0; idx < ArrayCount(items); idx += 1)
              {
                if(ui_ctx_menu_is_open(items[idx].menu_key))
                {
                  menu_open = 1;
                  open_menu_idx = idx;
                  break;
                }
              }
              
              // rjf: navigate between menus
              U64 open_menu_idx_prime = open_menu_idx;
              if(menu_open && ws->menu_bar_focused && window_is_focused)
              {
                for(UI_Event *evt = 0; ui_next_event(&evt);)
                {
                  B32 taken = 0;
                  if(evt->delta_2s32.x > 0)
                  {
                    taken = 1;
                    open_menu_idx_prime += 1;
                    open_menu_idx_prime = open_menu_idx_prime%ArrayCount(items);
                  }
                  if(evt->delta_2s32.x < 0)
                  {
                    taken = 1;
                    open_menu_idx_prime = open_menu_idx_prime > 0 ? open_menu_idx_prime-1 : (ArrayCount(items)-1);
                  }
                  if(taken)
                  {
                    ui_eat_event(evt);
                  }
                }
              }
              
              // rjf: make ui
              for(U64 idx = 0; idx < ArrayCount(items); idx += 1)
              {
                ui_set_next_fastpath_codepoint(items[idx].codepoint);
                B32 alt_fastpath_key = 0;
                if(ui_key_press(OS_EventFlag_Alt, items[idx].key))
                {
                  alt_fastpath_key = 1;
                }
                if((ws->menu_bar_key_held || ws->menu_bar_focused) && !ui_any_ctx_menu_is_open())
                {
                  ui_set_next_flags(UI_BoxFlag_DrawTextFastpathCodepoint);
                }
                UI_Signal sig = df_menu_bar_button(items[idx].name);
                os_window_push_custom_title_bar_client_area(ws->os, sig.box->rect);
                if(menu_open)
                {
                  if((ui_hovering(sig) && !ui_ctx_menu_is_open(items[idx].menu_key)) || (open_menu_idx_prime == idx && open_menu_idx_prime != open_menu_idx))
                  {
                    ui_ctx_menu_open(items[idx].menu_key, sig.box->key, v2f32(0, sig.box->rect.y1-sig.box->rect.y0));
                  }
                }
                else if(ui_pressed(sig) || alt_fastpath_key)
                {
                  if(ui_ctx_menu_is_open(items[idx].menu_key))
                  {
                    ui_ctx_menu_close();
                  }
                  else
                  {
                    ui_ctx_menu_open(items[idx].menu_key, sig.box->key, v2f32(0, sig.box->rect.y1-sig.box->rect.y0));
                  }
                }
              }
            }
          }
          
          ui_spacer(ui_em(0.75f, 1));
          
          // rjf: conversion task visualization
          UI_PrefWidth(ui_text_dim(10, 1)) UI_HeightFill
            DF_Palette(DF_PaletteCode_NeutralPopButton)
          {
            Temp scratch = scratch_begin(0, 0);
            DF_EntityList tasks = d_query_cached_entity_list_with_kind(DF_EntityKind_ConversionTask);
            for(DF_EntityNode *n = tasks.first; n != 0; n = n->next)
            {
              DF_Entity *task = n->entity;
              if(task->alloc_time_us + 500000 < os_now_microseconds())
              {
                String8 rdi_path = task->string;
                String8 rdi_name = str8_skip_last_slash(rdi_path);
                String8 task_text = push_str8f(scratch.arena, "Creating %S...", rdi_name);
                UI_Key key = ui_key_from_stringf(ui_key_zero(), "task_%p", task);
                UI_Box *box = ui_build_box_from_key(UI_BoxFlag_DrawHotEffects|UI_BoxFlag_DrawText|UI_BoxFlag_DrawBorder|UI_BoxFlag_DrawBackground|UI_BoxFlag_Clickable, key);
                os_window_push_custom_title_bar_client_area(ws->os, box->rect);
                UI_Signal sig = ui_signal_from_box(box);
                if(ui_hovering(sig)) UI_Tooltip
                {
                  ui_label(rdi_path);
                }
                ui_box_equip_display_string(box, task_text);
              }
            }
            scratch_end(scratch);
          }
        }
        
        //- rjf: center column
        UI_PrefWidth(ui_children_sum(1.f)) UI_Row
          UI_PrefWidth(ui_em(2.25f, 1))
          DF_Font(DF_FontSlot_Icons)
          UI_FontSize(ui_top_font_size()*0.85f)
        {
          Temp scratch = scratch_begin(0, 0);
          DF_EntityList targets = d_push_active_target_list(scratch.arena);
          DF_EntityList processes = d_query_cached_entity_list_with_kind(DF_EntityKind_Process);
          B32 have_targets = targets.count != 0;
          B32 can_send_signal = !d_ctrl_targets_running();
          B32 can_play  = (have_targets && (can_send_signal || d_ctrl_last_run_frame_idx()+4 > df_state->frame_index));
          B32 can_pause = (!can_send_signal);
          B32 can_stop  = (processes.count != 0);
          B32 can_step =  (processes.count != 0 && can_send_signal);
          
          //- rjf: play button
          if(can_play || !have_targets || processes.count == 0)
            UI_TextAlignment(UI_TextAlign_Center)
            UI_Flags((can_play ? 0 : UI_BoxFlag_Disabled))
            UI_Palette(ui_build_palette(ui_top_palette(), .text = df_rgba_from_theme_color(DF_ThemeColor_TextPositive)))
          {
            UI_Signal sig = ui_button(df_g_icon_kind_text_table[DF_IconKind_Play]);
            os_window_push_custom_title_bar_client_area(ws->os, sig.box->rect);
            if(ui_hovering(sig) && !can_play)
            {
              UI_Tooltip
                DF_Font(DF_FontSlot_Main)
                UI_FontSize(df_font_size_from_slot(DF_FontSlot_Main))
                ui_labelf("Disabled: %s", have_targets ? "Targets are currently running" : "No active targets exist");
            }
            if(ui_hovering(sig) && can_play)
            {
              UI_Tooltip
                DF_Font(DF_FontSlot_Main)
                UI_FontSize(df_font_size_from_slot(DF_FontSlot_Main))
              {
                if(can_stop)
                {
                  ui_labelf("Resume all processes");
                }
                else
                {
                  ui_labelf("Launch all active targets:");
                  for(DF_EntityNode *n = targets.first; n != 0; n = n->next)
                  {
                    String8 target_display_name = df_display_string_from_entity(scratch.arena, n->entity);
                    ui_label(target_display_name);
                  }
                }
              }
            }
            if(ui_clicked(sig))
            {
              df_cmd(DF_CmdKind_Run);
            }
          }
          
          //- rjf: restart button
          else UI_TextAlignment(UI_TextAlign_Center)
            UI_Palette(ui_build_palette(ui_top_palette(), .text = df_rgba_from_theme_color(DF_ThemeColor_TextPositive)))
          {
            UI_Signal sig = ui_button(df_g_icon_kind_text_table[DF_IconKind_Redo]);
            os_window_push_custom_title_bar_client_area(ws->os, sig.box->rect);
            if(ui_hovering(sig))
            {
              UI_Tooltip
                DF_Font(DF_FontSlot_Main)
                UI_FontSize(df_font_size_from_slot(DF_FontSlot_Main))
              {
                ui_labelf("Restart all running targets:");
                {
                  DF_EntityList processes = d_query_cached_entity_list_with_kind(DF_EntityKind_Process);
                  for(DF_EntityNode *n = processes.first; n != 0; n = n->next)
                  {
                    DF_Entity *process = n->entity;
                    DF_Entity *target = df_entity_from_handle(process->entity_handle);
                    if(!df_entity_is_nil(target))
                    {
                      String8 target_display_name = df_display_string_from_entity(scratch.arena, target);
                      ui_label(target_display_name);
                    }
                  }
                }
              }
            }
            if(ui_clicked(sig))
            {
              df_cmd(DF_CmdKind_Restart);
            }
          }
          
          //- rjf: pause button
          UI_TextAlignment(UI_TextAlign_Center) UI_Flags(can_pause ? 0 : UI_BoxFlag_Disabled)
            UI_Palette(ui_build_palette(ui_top_palette(), .text = df_rgba_from_theme_color(DF_ThemeColor_TextNeutral)))
          {
            UI_Signal sig = ui_button(df_g_icon_kind_text_table[DF_IconKind_Pause]);
            os_window_push_custom_title_bar_client_area(ws->os, sig.box->rect);
            if(ui_hovering(sig) && !can_pause)
            {
              UI_Tooltip
                DF_Font(DF_FontSlot_Main)
                UI_FontSize(df_font_size_from_slot(DF_FontSlot_Main))
                ui_labelf("Disabled: Already halted");
            }
            if(ui_hovering(sig) && can_pause)
            {
              UI_Tooltip
                DF_Font(DF_FontSlot_Main)
                UI_FontSize(df_font_size_from_slot(DF_FontSlot_Main))
                ui_labelf("Halt all target processes");
            }
            if(ui_clicked(sig))
            {
              df_cmd(DF_CmdKind_Halt);
            }
          }
          
          //- rjf: stop button
          UI_TextAlignment(UI_TextAlign_Center) UI_Flags(can_stop ? 0 : UI_BoxFlag_Disabled)
            UI_Palette(ui_build_palette(ui_top_palette(), .text = df_rgba_from_theme_color(DF_ThemeColor_TextNegative)))
          {
            UI_Signal sig = {0};
            {
              sig = ui_button(df_g_icon_kind_text_table[DF_IconKind_Stop]);
              os_window_push_custom_title_bar_client_area(ws->os, sig.box->rect);
            }
            if(ui_hovering(sig) && !can_stop)
            {
              UI_Tooltip
                DF_Font(DF_FontSlot_Main)
                UI_FontSize(df_font_size_from_slot(DF_FontSlot_Main))
                ui_labelf("Disabled: No processes are running");
            }
            if(ui_hovering(sig) && can_stop)
            {
              UI_Tooltip
                DF_Font(DF_FontSlot_Main)
                UI_FontSize(df_font_size_from_slot(DF_FontSlot_Main))
                ui_labelf("Kill all target processes");
            }
            if(ui_clicked(sig))
            {
              df_cmd(DF_CmdKind_Kill);
            }
          }
          
          //- rjf: step over button
          UI_TextAlignment(UI_TextAlign_Center) UI_Flags(can_step ? 0 : UI_BoxFlag_Disabled)
          {
            UI_Signal sig = ui_button(df_g_icon_kind_text_table[DF_IconKind_StepOver]);
            os_window_push_custom_title_bar_client_area(ws->os, sig.box->rect);
            if(ui_hovering(sig) && !can_step && can_pause)
            {
              UI_Tooltip
                DF_Font(DF_FontSlot_Main)
                UI_FontSize(df_font_size_from_slot(DF_FontSlot_Main))
                ui_labelf("Disabled: Running");
            }
            if(ui_hovering(sig) && !can_step && !can_stop)
            {
              UI_Tooltip
                DF_Font(DF_FontSlot_Main)
                UI_FontSize(df_font_size_from_slot(DF_FontSlot_Main))
                ui_labelf("Disabled: No processes are running");
            }
            if(ui_hovering(sig) && can_step)
            {
              UI_Tooltip
                DF_Font(DF_FontSlot_Main)
                UI_FontSize(df_font_size_from_slot(DF_FontSlot_Main))
                ui_labelf("Step Over");
            }
            if(ui_clicked(sig))
            {
              df_cmd(DF_CmdKind_StepOver);
            }
          }
          
          //- rjf: step into button
          UI_TextAlignment(UI_TextAlign_Center) UI_Flags(can_step ? 0 : UI_BoxFlag_Disabled)
          {
            UI_Signal sig = ui_button(df_g_icon_kind_text_table[DF_IconKind_StepInto]);
            os_window_push_custom_title_bar_client_area(ws->os, sig.box->rect);
            if(ui_hovering(sig) && !can_step && can_pause)
            {
              UI_Tooltip
                DF_Font(DF_FontSlot_Main)
                UI_FontSize(df_font_size_from_slot(DF_FontSlot_Main))
                ui_labelf("Disabled: Running");
            }
            if(ui_hovering(sig) && !can_step && !can_stop)
            {
              UI_Tooltip
                DF_Font(DF_FontSlot_Main)
                UI_FontSize(df_font_size_from_slot(DF_FontSlot_Main))
                ui_labelf("Disabled: No processes are running");
            }
            if(ui_hovering(sig) && can_step)
            {
              UI_Tooltip
                DF_Font(DF_FontSlot_Main)
                UI_FontSize(df_font_size_from_slot(DF_FontSlot_Main))
                ui_labelf("Step Into");
            }
            if(ui_clicked(sig))
            {
              df_cmd(DF_CmdKind_StepInto);
            }
          }
          
          //- rjf: step out button
          UI_TextAlignment(UI_TextAlign_Center) UI_Flags(can_step ? 0 : UI_BoxFlag_Disabled)
          {
            UI_Signal sig = ui_button(df_g_icon_kind_text_table[DF_IconKind_StepOut]);
            os_window_push_custom_title_bar_client_area(ws->os, sig.box->rect);
            if(ui_hovering(sig) && !can_step && can_pause)
            {
              UI_Tooltip
                DF_Font(DF_FontSlot_Main)
                UI_FontSize(df_font_size_from_slot(DF_FontSlot_Main))
                ui_labelf("Disabled: Running");
            }
            if(ui_hovering(sig) && !can_step && !can_stop)
            {
              UI_Tooltip
                DF_Font(DF_FontSlot_Main)
                UI_FontSize(df_font_size_from_slot(DF_FontSlot_Main))
                ui_labelf("Disabled: No processes are running");
            }
            if(ui_hovering(sig) && can_step)
            {
              UI_Tooltip
                DF_Font(DF_FontSlot_Main)
                UI_FontSize(df_font_size_from_slot(DF_FontSlot_Main))
                ui_labelf("Step Out");
            }
            if(ui_clicked(sig))
            {
              df_cmd(DF_CmdKind_StepOut);
            }
          }
          
          scratch_end(scratch);
        }
        
        //- rjf: right column
        UI_WidthFill UI_Row
        {
          B32 do_user_prof = (dim_2f32(top_bar_rect).x > ui_top_font_size()*80);
          
          ui_spacer(ui_pct(1, 0));
          
          // rjf: loaded user viz
          if(do_user_prof) DF_Palette(DF_PaletteCode_NeutralPopButton)
          {
            ui_set_next_pref_width(ui_children_sum(1));
            ui_set_next_child_layout_axis(Axis2_X);
            ui_set_next_hover_cursor(OS_Cursor_HandPoint);
            UI_Box *user_box = ui_build_box_from_stringf(UI_BoxFlag_Clickable|
                                                         UI_BoxFlag_DrawBorder|
                                                         UI_BoxFlag_DrawBackground|
                                                         UI_BoxFlag_DrawHotEffects|
                                                         UI_BoxFlag_DrawActiveEffects,
                                                         "###loaded_user_button");
            os_window_push_custom_title_bar_client_area(ws->os, user_box->rect);
            UI_Parent(user_box) UI_PrefWidth(ui_text_dim(10, 0)) UI_TextAlignment(UI_TextAlign_Center)
            {
              String8 user_path = df_cfg_path_from_src(D_CfgSrc_User);
              user_path = str8_chop_last_dot(user_path);
              DF_Font(DF_FontSlot_Icons)
                UI_TextRasterFlags(df_raster_flags_from_slot(DF_FontSlot_Icons))
                ui_label(df_g_icon_kind_text_table[DF_IconKind_Person]);
              ui_label(str8_skip_last_slash(user_path));
            }
            UI_Signal user_sig = ui_signal_from_box(user_box);
            if(ui_clicked(user_sig))
            {
              df_cmd(DF_CmdKind_RunCommand, .string = df_cmd_kind_info_table[DF_CmdKind_OpenUser].string);
            }
          }
          
          if(do_user_prof)
          {
            ui_spacer(ui_em(0.75f, 0));
          }
          
          // rjf: loaded project viz
          if(do_user_prof) DF_Palette(DF_PaletteCode_NeutralPopButton)
          {
            ui_set_next_pref_width(ui_children_sum(1));
            ui_set_next_child_layout_axis(Axis2_X);
            ui_set_next_hover_cursor(OS_Cursor_HandPoint);
            UI_Box *prof_box = ui_build_box_from_stringf(UI_BoxFlag_Clickable|
                                                         UI_BoxFlag_DrawBorder|
                                                         UI_BoxFlag_DrawBackground|
                                                         UI_BoxFlag_DrawHotEffects|
                                                         UI_BoxFlag_DrawActiveEffects,
                                                         "###loaded_project_button");
            os_window_push_custom_title_bar_client_area(ws->os, prof_box->rect);
            UI_Parent(prof_box) UI_PrefWidth(ui_text_dim(10, 0)) UI_TextAlignment(UI_TextAlign_Center)
            {
              String8 prof_path = df_cfg_path_from_src(D_CfgSrc_Project);
              prof_path = str8_chop_last_dot(prof_path);
              DF_Font(DF_FontSlot_Icons)
                ui_label(df_g_icon_kind_text_table[DF_IconKind_Briefcase]);
              ui_label(str8_skip_last_slash(prof_path));
            }
            UI_Signal prof_sig = ui_signal_from_box(prof_box);
            if(ui_clicked(prof_sig))
            {
              df_cmd(DF_CmdKind_RunCommand, .string = df_cmd_kind_info_table[DF_CmdKind_OpenProject].string);
            }
          }
          
          if(do_user_prof)
          {
            ui_spacer(ui_em(0.75f, 0));
          }
          
          // rjf: min/max/close buttons
          {
            UI_Signal min_sig = {0};
            UI_Signal max_sig = {0};
            UI_Signal cls_sig = {0};
            Vec2F32 bar_dim = dim_2f32(top_bar_rect);
            F32 button_dim = floor_f32(bar_dim.y);
            UI_PrefWidth(ui_px(button_dim, 1.f))
            {
              min_sig = df_icon_buttonf(DF_IconKind_Minus,  0, "##minimize");
              max_sig = df_icon_buttonf(DF_IconKind_Window, 0, "##maximize");
            }
            UI_PrefWidth(ui_px(button_dim, 1.f))
              DF_Palette(DF_PaletteCode_NegativePopButton)
            {
              cls_sig = df_icon_buttonf(DF_IconKind_X,      0, "##close");
            }
            if(ui_clicked(min_sig))
            {
              os_window_minimize(ws->os);
            }
            if(ui_clicked(max_sig))
            {
              os_window_set_maximized(ws->os, !os_window_is_maximized(ws->os));
            }
            if(ui_clicked(cls_sig))
            {
              df_cmd(DF_CmdKind_CloseWindow, .window = df_handle_from_window(ws));
            }
            os_window_push_custom_title_bar_client_area(ws->os, min_sig.box->rect);
            os_window_push_custom_title_bar_client_area(ws->os, max_sig.box->rect);
            os_window_push_custom_title_bar_client_area(ws->os, pad_2f32(cls_sig.box->rect, 2.f));
          }
        }
      }
    }
    
    ////////////////////////////
    //- rjf: bottom bar
    //
    ProfScope("build bottom bar")
    {
      B32 is_running = d_ctrl_targets_running() && d_ctrl_last_run_frame_idx() < df_state->frame_index;
      CTRL_Event stop_event = d_ctrl_last_stop_event();
      UI_Palette *positive_scheme = df_palette_from_code(DF_PaletteCode_PositivePopButton);
      UI_Palette *running_scheme  = df_palette_from_code(DF_PaletteCode_NeutralPopButton);
      UI_Palette *negative_scheme = df_palette_from_code(DF_PaletteCode_NegativePopButton);
      UI_Palette *palette = running_scheme;
      if(!is_running)
      {
        switch(stop_event.cause)
        {
          default:
          case CTRL_EventCause_Finished:
          {
            palette = positive_scheme;
          }break;
          case CTRL_EventCause_UserBreakpoint:
          case CTRL_EventCause_InterruptedByException:
          case CTRL_EventCause_InterruptedByTrap:
          case CTRL_EventCause_InterruptedByHalt:
          {
            palette = negative_scheme;
          }break;
        }
      }
      if(ws->error_t > 0.01f)
      {
        UI_Palette *blended_scheme = push_array(ui_build_arena(), UI_Palette, 1);
        MemoryCopyStruct(blended_scheme, palette);
        for(EachEnumVal(UI_ColorCode, code))
        {
          for(U64 idx = 0; idx < 4; idx += 1)
          {
            blended_scheme->colors[code].v[idx] += (negative_scheme->colors[code].v[idx] - blended_scheme->colors[code].v[idx]) * ws->error_t;
          }
        }
        palette = blended_scheme;
      }
      UI_Flags(UI_BoxFlag_DrawBackground) UI_CornerRadius(0)
        UI_Palette(palette)
        UI_Pane(bottom_bar_rect, str8_lit("###bottom_bar")) UI_WidthFill UI_Row
        UI_Flags(0)
      {
        // rjf: developer frame-time indicator
        if(DEV_updating_indicator)
        {
          F32 animation_t = pow_f32(sin_f32(df_state->time_in_seconds/2.f), 2.f);
          ui_spacer(ui_em(0.3f, 1.f));
          ui_spacer(ui_em(1.5f*animation_t, 1.f));
          UI_PrefWidth(ui_text_dim(10, 1)) ui_labelf("*");
          ui_spacer(ui_em(1.5f*(1-animation_t), 1.f));
        }
        
        // rjf: status
        {
          if(is_running)
          {
            ui_label(str8_lit("Running"));
          }
          else
          {
            Temp scratch = scratch_begin(0, 0);
            DF_IconKind icon = DF_IconKind_Null;
            String8 explanation = str8_lit("Not running");
            {
              String8 stop_explanation = df_stop_explanation_string_icon_from_ctrl_event(scratch.arena, &stop_event, &icon);
              if(stop_explanation.size != 0)
              {
                explanation = stop_explanation;
              }
            }
            if(icon != DF_IconKind_Null)
            {
              UI_PrefWidth(ui_em(2.25f, 1.f))
                DF_Font(DF_FontSlot_Icons)
                UI_FontSize(df_font_size_from_slot(DF_FontSlot_Icons))
                ui_label(df_g_icon_kind_text_table[icon]);
            }
            UI_PrefWidth(ui_text_dim(10, 1)) ui_label(explanation);
            scratch_end(scratch);
          }
        }
        
        ui_spacer(ui_pct(1, 0));
        
        // rjf: bind change visualization
        if(df_state->bind_change_active)
        {
          DF_CmdKindInfo *info = df_cmd_kind_info_from_string(df_state->bind_change_cmd_name);
          UI_PrefWidth(ui_text_dim(10, 1))
            UI_Flags(UI_BoxFlag_DrawBackground)
            UI_TextAlignment(UI_TextAlign_Center)
            UI_CornerRadius(4)
            DF_Palette(DF_PaletteCode_NeutralPopButton)
            ui_labelf("Currently rebinding \"%S\" hotkey", info->display_name);
        }
        
        // rjf: error visualization
        else if(ws->error_t >= 0.01f)
        {
          ws->error_t -= df_state->frame_dt/8.f;
          df_request_frame();
          String8 error_string = str8(ws->error_buffer, ws->error_string_size);
          if(error_string.size != 0)
          {
            ui_set_next_pref_width(ui_children_sum(1));
            UI_CornerRadius(4)
              UI_Row
              UI_PrefWidth(ui_text_dim(10, 1))
              UI_TextAlignment(UI_TextAlign_Center)
            {
              DF_Font(DF_FontSlot_Icons)
                UI_FontSize(df_font_size_from_slot(DF_FontSlot_Icons))
                ui_label(df_g_icon_kind_text_table[DF_IconKind_WarningBig]);
              df_label(error_string);
            }
          }
        }
      }
    }
    
    ////////////////////////////
    //- rjf: prepare query view stack for the in-progress command
    //
    if(ws->query_cmd_name.size != 0)
    {
      DF_CmdKindInfo *cmd_kind_info = df_cmd_kind_info_from_string(ws->query_cmd_name);
      DF_RegSlot missing_slot = cmd_kind_info->query.slot;
      String8 query_view_name = cmd_kind_info->query.view_name;
      if(query_view_name.size == 0)
      {
        switch(missing_slot)
        {
          default:{}break;
          case DF_RegSlot_Entity:{query_view_name = df_view_kind_name_lower_table[DF_ViewKind_EntityLister];}break;
          case DF_RegSlot_EntityList:{query_view_name = df_view_kind_name_lower_table[DF_ViewKind_EntityLister];}break;
          case DF_RegSlot_FilePath:{query_view_name = df_view_kind_name_lower_table[DF_ViewKind_FileSystem];}break;
          case DF_RegSlot_PID:{query_view_name = df_view_kind_name_lower_table[DF_ViewKind_SystemProcesses];}break;
        }
      }
      DF_ViewSpec *view_spec = df_view_spec_from_string(query_view_name);
      if(ws->query_view_stack_top->spec != view_spec ||
         df_view_is_nil(ws->query_view_stack_top))
      {
        Temp scratch = scratch_begin(0, 0);
        
        // rjf: clear existing query stack
        for(DF_View *query_view = ws->query_view_stack_top, *next = 0;
            !df_view_is_nil(query_view);
            query_view = next)
        {
          next = query_view->order_next;
          df_view_release(query_view);
        }
        
        // rjf: determine default query
        String8 default_query = {0};
        switch(missing_slot)
        {
          default:
          if(cmd_kind_info->query.flags & DF_QueryFlag_KeepOldInput)
          {
            default_query = df_push_search_string(scratch.arena);
          }break;
          case DF_RegSlot_FilePath:
          {
            default_query = path_normalized_from_string(scratch.arena, df_state->current_path);
            default_query = push_str8f(scratch.arena, "%S/", default_query);
          }break;
        }
        
        // rjf: construct & push new view
        DF_View *view = df_view_alloc();
        df_view_equip_spec(view, view_spec, default_query, &md_nil_node);
        if(cmd_kind_info->query.flags & DF_QueryFlag_SelectOldInput)
        {
          view->query_mark = txt_pt(1, 1);
        }
        ws->query_view_stack_top = view;
        ws->query_view_selected = 1;
        view->order_next = &df_nil_view;
        
        scratch_end(scratch);
      }
    }
    
    ////////////////////////////
    //- rjf: animate query info
    //
    {
      F32 rate = df_setting_val_from_code(DF_SettingCode_MenuAnimations).s32 ? 1 - pow_f32(2, (-60.f * df_state->frame_dt)) : 1.f;
      
      // rjf: animate query view selection transition
      {
        F32 target = (F32)!!ws->query_view_selected;
        F32 diff = abs_f32(target - ws->query_view_selected_t);
        if(diff > 0.005f)
        {
          df_request_frame();
          if(diff < 0.005f)
          {
            ws->query_view_selected_t = target;
          }
          ws->query_view_selected_t += (target - ws->query_view_selected_t) * rate;
        }
      }
      
      // rjf: animate query view open/close transition
      {
        F32 query_view_t_target = !df_view_is_nil(ws->query_view_stack_top);
        F32 diff = abs_f32(query_view_t_target - ws->query_view_t);
        if(diff > 0.005f)
        {
          df_request_frame();
        }
        if(diff < 0.005f)
        {
          ws->query_view_t = query_view_t_target;
        }
        ws->query_view_t += (query_view_t_target - ws->query_view_t) * rate;
      }
    }
    
    ////////////////////////////
    //- rjf: build query
    //
    if(!df_view_is_nil(ws->query_view_stack_top))
      UI_Focus((window_is_focused && !ui_any_ctx_menu_is_open() && !ws->menu_bar_focused && ws->query_view_selected) ? UI_FocusKind_On : UI_FocusKind_Off)
      DF_Palette(DF_PaletteCode_Floating)
    {
      DF_View *view = ws->query_view_stack_top;
      String8 query_cmd_name = ws->query_cmd_name;
      DF_CmdKindInfo *query_cmd_info = df_cmd_kind_info_from_string(query_cmd_name);
      DF_Query *query = &query_cmd_info->query;
      
      //- rjf: calculate rectangles
      Vec2F32 window_center = center_2f32(window_rect);
      F32 query_container_width = dim_2f32(window_rect).x*0.5f;
      F32 query_container_margin = ui_top_font_size()*8.f;
      F32 query_line_edit_height = ui_top_font_size()*3.f;
      Rng2F32 query_container_rect = r2f32p(window_center.x - query_container_width/2 + (1-ws->query_view_t)*query_container_width/4,
                                            window_rect.y0 + query_container_margin,
                                            window_center.x + query_container_width/2 - (1-ws->query_view_t)*query_container_width/4,
                                            window_rect.y1 - query_container_margin);
      if(ws->query_view_stack_top->spec == &df_nil_view_spec)
      {
        query_container_rect.y1 = query_container_rect.y0 + query_line_edit_height;
      }
      query_container_rect.y1 = mix_1f32(query_container_rect.y0, query_container_rect.y1, ws->query_view_t);
      Rng2F32 query_container_content_rect = r2f32p(query_container_rect.x0,
                                                    query_container_rect.y0+query_line_edit_height,
                                                    query_container_rect.x1,
                                                    query_container_rect.y1);
      
      //- rjf: build floating query view container
      UI_Box *query_container_box = &ui_g_nil_box;
      UI_Rect(query_container_rect)
        UI_CornerRadius(ui_top_font_size()*0.2f)
        UI_ChildLayoutAxis(Axis2_Y)
        UI_Squish(0.25f-ws->query_view_t*0.25f)
        UI_Transparency(1-ws->query_view_t)
      {
        query_container_box = ui_build_box_from_stringf(UI_BoxFlag_Floating|
                                                        UI_BoxFlag_AllowOverflow|
                                                        UI_BoxFlag_Clickable|
                                                        UI_BoxFlag_Clip|
                                                        UI_BoxFlag_DisableFocusOverlay|
                                                        UI_BoxFlag_DrawBorder|
                                                        UI_BoxFlag_DrawBackground|
                                                        UI_BoxFlag_DrawBackgroundBlur|
                                                        UI_BoxFlag_DrawDropShadow,
                                                        "panel_query_container");
      }
      
      //- rjf: build query text input
      B32 query_completed = 0;
      B32 query_cancelled = 0;
      UI_Parent(query_container_box)
        UI_WidthFill UI_PrefHeight(ui_px(query_line_edit_height, 1.f))
        UI_Focus(UI_FocusKind_On)
      {
        ui_set_next_flags(UI_BoxFlag_DrawDropShadow|UI_BoxFlag_DrawBorder);
        UI_Row
        {
          UI_PrefWidth(ui_text_dim(0.f, 1.f)) UI_Padding(ui_em(1.f, 1.f))
          {
            DF_IconKind icon_kind = query_cmd_info->icon_kind;
            if(icon_kind != DF_IconKind_Null)
            {
              DF_Font(DF_FontSlot_Icons) ui_label(df_g_icon_kind_text_table[icon_kind]);
            }
            ui_labelf("%S", query_cmd_info->display_name);
          }
          DF_Font((query->flags & DF_QueryFlag_CodeInput) ? DF_FontSlot_Code : DF_FontSlot_Main)
            UI_TextPadding(ui_top_font_size()*0.5f)
          {
            UI_Signal sig = df_line_edit(DF_LineEditFlag_Border|
                                         (DF_LineEditFlag_CodeContents * !!(query->flags & DF_QueryFlag_CodeInput)),
                                         0,
                                         0,
                                         &view->query_cursor,
                                         &view->query_mark,
                                         view->query_buffer,
                                         sizeof(view->query_buffer),
                                         &view->query_string_size,
                                         0,
                                         str8(view->query_buffer, view->query_string_size),
                                         str8_lit("###query_text_input"));
            if(ui_pressed(sig))
            {
              ws->query_view_selected = 1;
            }
          }
          UI_PrefWidth(ui_em(5.f, 1.f)) UI_Focus(UI_FocusKind_Off) DF_Palette(DF_PaletteCode_PositivePopButton)
          {
            if(ui_clicked(df_icon_buttonf(DF_IconKind_RightArrow, 0, "##complete_query")))
            {
              query_completed = 1;
            }
          }
          UI_PrefWidth(ui_em(3.f, 1.f)) UI_Focus(UI_FocusKind_Off) DF_Palette(DF_PaletteCode_PlainButton)
          {
            if(ui_clicked(df_icon_buttonf(DF_IconKind_X, 0, "##cancel_query")))
            {
              query_cancelled = 1;
            }
          }
        }
      }
      
      //- rjf: build query view
      UI_Parent(query_container_box) UI_WidthFill UI_Focus(UI_FocusKind_Null)
      {
        DF_ViewSpec *view_spec = view->spec;
        DF_ViewUIFunctionType *build_view_ui_function = view_spec->info.ui_hook;
        build_view_ui_function(view, view->params_roots[view->params_read_gen%ArrayCount(view->params_roots)], str8(view->query_buffer, view->query_string_size), query_container_content_rect);
      }
      
      //- rjf: query submission
      if(((ui_is_focus_active() || (window_is_focused && !ui_any_ctx_menu_is_open() && !ws->menu_bar_focused && !ws->query_view_selected)) &&
          ui_slot_press(UI_EventActionSlot_Cancel)) || query_cancelled)
      {
        df_cmd(DF_CmdKind_CancelQuery);
      }
      if((ui_is_focus_active() && ui_slot_press(UI_EventActionSlot_Accept)) || query_completed)
      {
        Temp scratch = scratch_begin(0, 0);
        DF_View *view = ws->query_view_stack_top;
        DF_RegsScope()
        {
          df_regs_fill_slot_from_string(query->slot, str8(view->query_buffer, view->query_string_size));
          df_cmd(DF_CmdKind_CompleteQuery);
        }
        scratch_end(scratch);
      }
      
      //- rjf: take fallthrough interaction in query view
      {
        UI_Signal sig = ui_signal_from_box(query_container_box);
        if(ui_pressed(sig))
        {
          ws->query_view_selected = 1;
        }
      }
      
      //- rjf: build darkening overlay for rest of screen
      UI_Palette(ui_build_palette(0, .background = mix_4f32(df_rgba_from_theme_color(DF_ThemeColor_InactivePanelOverlay), v4f32(0, 0, 0, 0), 1-ws->query_view_selected_t)))
        UI_Rect(window_rect)
      {
        ui_build_box_from_key(UI_BoxFlag_DrawBackground, ui_key_zero());
      }
    }
    else
    {
      ws->query_view_selected = 0;
    }
    
    ////////////////////////////
    //- rjf: build hover eval
    //
    ProfScope("build hover eval")
    {
      B32 build_hover_eval = hover_eval_is_open;
      
      // rjf: disable hover eval if hovered view is actively scrolling
      if(hover_eval_is_open)
      {
        for(DF_Panel *panel = ws->root_panel;
            !df_panel_is_nil(panel);
            panel = df_panel_rec_df_pre(panel).next)
        {
          if(!df_panel_is_nil(panel->first)) { continue; }
          Rng2F32 panel_rect = df_target_rect_from_panel(content_rect, ws->root_panel, panel);
          DF_View *view = df_selected_tab_from_panel(panel);
          if(!df_view_is_nil(view) &&
             contains_2f32(panel_rect, ui_mouse()) &&
             (abs_f32(view->scroll_pos.x.off) > 0.01f ||
              abs_f32(view->scroll_pos.y.off) > 0.01f))
          {
            build_hover_eval = 0;
            ws->hover_eval_first_frame_idx = df_state->frame_index;
          }
        }
      }
      
      // rjf: reset open animation
      if(ws->hover_eval_string.size == 0)
      {
        ws->hover_eval_open_t = 0;
        ws->hover_eval_num_visible_rows_t = 0;
      }
      
      // rjf: reset animation, but request frames if we're waiting to open
      if(ws->hover_eval_string.size != 0 && !hover_eval_is_open && ws->hover_eval_last_frame_idx < ws->hover_eval_first_frame_idx+20 && df_state->frame_index-ws->hover_eval_last_frame_idx < 50)
      {
        df_request_frame();
        ws->hover_eval_num_visible_rows_t = 0;
        ws->hover_eval_open_t = 0;
      }
      
      // rjf: reset focus state if hover eval is not being built
      if(!build_hover_eval || ws->hover_eval_string.size == 0 || !hover_eval_is_open)
      {
        ws->hover_eval_focused = 0;
      }
      
      // rjf: build hover eval
      if(build_hover_eval && ws->hover_eval_string.size != 0 && hover_eval_is_open)
        DF_Font(DF_FontSlot_Code)
        UI_FontSize(df_font_size_from_slot(DF_FontSlot_Main))
        DF_Palette(DF_PaletteCode_Floating)
      {
        Temp scratch = scratch_begin(0, 0);
        DI_Scope *scope = di_scope_open();
        String8 expr = ws->hover_eval_string;
        E_Eval eval = e_eval_from_string(scratch.arena, expr);
        EV_ViewRuleList top_level_view_rules = {0};
        
        //- rjf: build if good
        if(!e_type_key_match(eval.type_key, e_type_key_zero()) && !ui_any_ctx_menu_is_open())
          UI_Focus((hover_eval_is_open && !ui_any_ctx_menu_is_open() && ws->hover_eval_focused && (!query_is_open || !ws->query_view_selected)) ? UI_FocusKind_Null : UI_FocusKind_Off)
        {
          //- rjf: eval -> viz artifacts
          F32 row_height = floor_f32(ui_top_font_size()*2.8f);
          D_CfgTable cfg_table = {0};
          U64 expr_hash = d_hash_from_string(expr);
          String8 ev_view_key_string = push_str8f(scratch.arena, "eval_hover_%I64x", expr_hash);
          EV_View *ev_view = df_ev_view_from_key(d_hash_from_string(ev_view_key_string));
          EV_Key parent_key = ev_key_make(5381, 1);
          EV_Key key = ev_key_make(ev_hash_from_key(parent_key), 1);
          EV_BlockList viz_blocks = ev_block_list_from_view_expr_keys(scratch.arena, ev_view, &top_level_view_rules, expr, parent_key, key);
          DF_Entity *entity = d_entity_from_eval_space(eval.space);
          U32 default_radix = (entity->kind == DF_EntityKind_Thread ? 16 : 10);
          EV_WindowedRowList viz_rows = ev_windowed_row_list_from_block_list(scratch.arena, ev_view, r1s64(0, 50), &viz_blocks);
          
          //- rjf: animate
          {
            // rjf: animate height
            {
              F32 fish_rate = df_setting_val_from_code(DF_SettingCode_MenuAnimations).s32 ? 1 - pow_f32(2, (-60.f * df_state->frame_dt)) : 1.f;
              F32 hover_eval_container_height_target = row_height * Min(30, viz_blocks.total_visual_row_count);
              ws->hover_eval_num_visible_rows_t += (hover_eval_container_height_target - ws->hover_eval_num_visible_rows_t) * fish_rate;
              if(abs_f32(hover_eval_container_height_target - ws->hover_eval_num_visible_rows_t) > 0.5f)
              {
                df_request_frame();
              }
              else
              {
                ws->hover_eval_num_visible_rows_t = hover_eval_container_height_target;
              }
            }
            
            // rjf: animate open
            {
              F32 fish_rate = df_setting_val_from_code(DF_SettingCode_MenuAnimations).s32 ? 1 - pow_f32(2, (-60.f * df_state->frame_dt)) : 1.f;
              F32 diff = 1.f - ws->hover_eval_open_t;
              ws->hover_eval_open_t += diff*fish_rate;
              if(abs_f32(diff) < 0.01f)
              {
                ws->hover_eval_open_t = 1.f;
              }
              else
              {
                df_request_frame();
              }
            }
          }
          
          //- rjf: calculate width
          F32 width_px = 40.f*ui_top_font_size();
          F32 expr_column_width_px = 10.f*ui_top_font_size();
          F32 value_column_width_px = 30.f*ui_top_font_size();
          if(viz_rows.first != 0)
          {
            EV_Row *row = viz_rows.first;
            E_Eval row_eval = e_eval_from_expr(scratch.arena, row->expr);
            String8 row_expr_string = ev_expr_string_from_row(scratch.arena, row);
            String8 row_display_value = df_value_string_from_eval(scratch.arena, EV_StringFlag_ReadOnlyDisplayRules, default_radix, ui_top_font(), ui_top_font_size(), 500.f, row_eval, row->member, row->view_rules);
            expr_column_width_px = fnt_dim_from_tag_size_string(ui_top_font(), ui_top_font_size(), 0, 0, row_expr_string).x + ui_top_font_size()*5.f;
            value_column_width_px = fnt_dim_from_tag_size_string(ui_top_font(), ui_top_font_size(), 0, 0, row_display_value).x + ui_top_font_size()*5.f;
            F32 total_dim_px = (expr_column_width_px + value_column_width_px);
            width_px = Min(80.f*ui_top_font_size(), total_dim_px*1.5f);
          }
          
          //- rjf: build hover eval box
          F32 hover_eval_container_height = ws->hover_eval_num_visible_rows_t;
          F32 corner_radius = ui_top_font_size()*0.25f;
          ui_set_next_fixed_x(ws->hover_eval_spawn_pos.x);
          ui_set_next_fixed_y(ws->hover_eval_spawn_pos.y);
          ui_set_next_pref_width(ui_px(width_px, 1.f));
          ui_set_next_pref_height(ui_px(hover_eval_container_height, 1.f));
          ui_set_next_corner_radius_00(0);
          ui_set_next_corner_radius_01(corner_radius);
          ui_set_next_corner_radius_10(corner_radius);
          ui_set_next_corner_radius_11(corner_radius);
          ui_set_next_child_layout_axis(Axis2_Y);
          ui_set_next_squish(0.25f-0.25f*ws->hover_eval_open_t);
          ui_set_next_transparency(1.f-ws->hover_eval_open_t);
          UI_Focus(UI_FocusKind_On)
          {
            hover_eval_box = ui_build_box_from_stringf(UI_BoxFlag_DrawBorder|
                                                       UI_BoxFlag_DrawBackground|
                                                       UI_BoxFlag_DrawBackgroundBlur|
                                                       UI_BoxFlag_DrawDropShadow|
                                                       UI_BoxFlag_DisableFocusOverlay|
                                                       UI_BoxFlag_Clip|
                                                       UI_BoxFlag_AllowOverflowY|
                                                       UI_BoxFlag_ViewScroll|
                                                       UI_BoxFlag_ViewClamp|
                                                       UI_BoxFlag_Floating|
                                                       UI_BoxFlag_AnimatePos|
                                                       UI_BoxFlag_Clickable|
                                                       UI_BoxFlag_DefaultFocusNav,
                                                       "###hover_eval");
          }
          
          //- rjf: build contents
          UI_Parent(hover_eval_box) UI_PrefHeight(ui_px(row_height, 1.f))
          {
            //- rjf: build rows
            for(EV_Row *row = viz_rows.first; row != 0; row = row->next)
            {
              //- rjf: unpack row
              E_Eval row_eval = e_eval_from_expr(scratch.arena, row->expr);
              String8 row_expr_string = ev_expr_string_from_row(scratch.arena, row);
              String8 row_edit_value = df_value_string_from_eval(scratch.arena, 0, default_radix, ui_top_font(), ui_top_font_size(), 500.f, row_eval, row->member, row->view_rules);
              String8 row_display_value = df_value_string_from_eval(scratch.arena, EV_StringFlag_ReadOnlyDisplayRules, default_radix, ui_top_font(), ui_top_font_size(), 500.f, row_eval, row->member, row->view_rules);
              B32 row_is_editable = ev_row_is_editable(row);
              B32 row_is_expandable = ev_row_is_expandable(row);
              
              //- rjf: determine if row's data is fresh and/or bad
              B32 row_is_fresh = 0;
              B32 row_is_bad = 0;
              switch(row_eval.mode)
              {
                default:{}break;
                case E_Mode_Offset:
                {
                  DF_Entity *space_entity = d_entity_from_eval_space(row_eval.space);
                  if(space_entity->kind == DF_EntityKind_Process)
                  {
                    U64 size = e_type_byte_size_from_key(row_eval.type_key);
                    size = Min(size, 64);
                    Rng1U64 vaddr_rng = r1u64(row_eval.value.u64, row_eval.value.u64+size);
                    CTRL_ProcessMemorySlice slice = ctrl_query_cached_data_from_process_vaddr_range(scratch.arena, space_entity->ctrl_handle, vaddr_rng, 0);
                    for(U64 idx = 0; idx < (slice.data.size+63)/64; idx += 1)
                    {
                      if(slice.byte_changed_flags[idx] != 0)
                      {
                        row_is_fresh = 1;
                      }
                      if(slice.byte_bad_flags[idx] != 0)
                      {
                        row_is_bad = 1;
                      }
                    }
                  }
                }break;
              }
              
              //- rjf: build row
              UI_WidthFill UI_Row
              {
                ui_spacer(ui_em(0.75f, 1.f));
                if(row->depth > 0)
                {
                  for(S32 indent = 0; indent < row->depth; indent += 1)
                  {
                    ui_spacer(ui_em(0.75f, 1.f));
                    UI_Flags(UI_BoxFlag_DrawSideLeft) ui_spacer(ui_em(1.5f, 1.f));
                  }
                }
                U64 row_hash = ev_hash_from_key(row->key);
                B32 row_is_expanded = ev_expansion_from_key(ev_view, row->key);
                if(row_is_expandable)
                  UI_PrefWidth(ui_em(1.5f, 1)) 
                  if(ui_pressed(ui_expanderf(row_is_expanded, "###%I64x_%I64x_is_expanded", row->key.parent_hash, row->key.child_num)))
                {
                  ev_key_set_expansion(ev_view, row->parent_key, row->key, !row_is_expanded);
                }
                if(!row_is_expandable)
                {
                  UI_PrefWidth(ui_em(1.5f, 1))
                    UI_Flags(UI_BoxFlag_DrawTextWeak)
                    DF_Font(DF_FontSlot_Icons)
                    ui_label(df_g_icon_kind_text_table[DF_IconKind_Dot]);
                }
                UI_WidthFill UI_TextRasterFlags(df_raster_flags_from_slot(DF_FontSlot_Code))
                {
                  UI_PrefWidth(ui_px(expr_column_width_px, 1.f)) df_code_label(1.f, 1, df_rgba_from_theme_color(DF_ThemeColor_CodeDefault), row_expr_string);
                  ui_spacer(ui_em(1.5f, 1.f));
                  if(row_is_editable)
                  {
                    if(row_is_fresh)
                    {
                      Vec4F32 rgba = df_rgba_from_theme_color(DF_ThemeColor_HighlightOverlay);
                      ui_set_next_palette(ui_build_palette(ui_top_palette(), .background = rgba));
                    }
                    UI_Signal sig = df_line_editf(DF_LineEditFlag_CodeContents|
                                                  DF_LineEditFlag_DisplayStringIsCode|
                                                  DF_LineEditFlag_PreferDisplayString|
                                                  DF_LineEditFlag_Border,
                                                  0, 0, &ws->hover_eval_txt_cursor, &ws->hover_eval_txt_mark, ws->hover_eval_txt_buffer, sizeof(ws->hover_eval_txt_buffer), &ws->hover_eval_txt_size, 0, row_edit_value, "%S###val_%I64x", row_display_value, row_hash);
                    if(ui_pressed(sig))
                    {
                      ws->hover_eval_focused = 1;
                    }
                    if(ui_committed(sig))
                    {
                      String8 commit_string = str8(ws->hover_eval_txt_buffer, ws->hover_eval_txt_size);
                      B32 success = d_commit_eval_value_string(row_eval, commit_string);
                      if(success == 0)
                      {
                        log_user_error(str8_lit("Could not commit value successfully."));
                      }
                    }
                  }
                  else
                  {
                    if(row_is_fresh)
                    {
                      Vec4F32 rgba = df_rgba_from_theme_color(DF_ThemeColor_HighlightOverlay);
                      ui_set_next_palette(ui_build_palette(ui_top_palette(), .background = rgba));
                      ui_set_next_flags(UI_BoxFlag_DrawBackground);
                    }
                    df_code_label(1.f, 1, df_rgba_from_theme_color(DF_ThemeColor_CodeDefault), row_display_value);
                  }
                }
                if(row == viz_rows.first)
                {
                  UI_TextAlignment(UI_TextAlign_Center) UI_PrefWidth(ui_em(3.f, 1.f))
                    UI_CornerRadius00(0)
                    UI_CornerRadius01(0)
                    UI_CornerRadius10(0)
                    UI_CornerRadius11(0)
                  {
                    UI_Signal watch_sig = df_icon_buttonf(DF_IconKind_List, 0, "###watch_hover_eval");
                    if(ui_hovering(watch_sig)) UI_Tooltip DF_Font(DF_FontSlot_Main) UI_FontSize(df_font_size_from_slot(DF_FontSlot_Main))
                    {
                      ui_labelf("Add the hovered expression to an opened watch view.");
                    }
                    if(ui_clicked(watch_sig))
                    {
                      df_cmd(DF_CmdKind_ToggleWatchExpression, .string = expr);
                    }
                  }
                  if(ws->hover_eval_file_path.size != 0 || ws->hover_eval_vaddr != 0)
                    UI_TextAlignment(UI_TextAlign_Center) UI_PrefWidth(ui_em(3.f, 1.f))
                    UI_CornerRadius10(corner_radius)
                    UI_CornerRadius11(corner_radius)
                  {
                    UI_Signal pin_sig = df_icon_buttonf(DF_IconKind_Pin, 0, "###pin_hover_eval");
                    if(ui_hovering(pin_sig)) UI_Tooltip DF_Font(DF_FontSlot_Main) UI_FontSize(df_font_size_from_slot(DF_FontSlot_Main))
                      UI_CornerRadius00(0)
                      UI_CornerRadius01(0)
                      UI_CornerRadius10(0)
                      UI_CornerRadius11(0)
                    {
                      ui_labelf("Pin the hovered expression to this code location.");
                    }
                    if(ui_clicked(pin_sig))
                    {
                      df_cmd(DF_CmdKind_ToggleWatchPin,
                             .file_path  = ws->hover_eval_file_path,
                             .cursor     = ws->hover_eval_file_pt,
                             .vaddr      = ws->hover_eval_vaddr,
                             .string     = expr);
                    }
                  }
                }
              }
            }
            UI_PrefWidth(ui_px(0, 0)) ui_spacer(ui_px(hover_eval_container_height-row_height, 1.f));
          }
          
          //- rjf: interact
          {
            UI_Signal hover_eval_sig = ui_signal_from_box(hover_eval_box);
            if(ui_mouse_over(hover_eval_sig))
            {
              ws->hover_eval_last_frame_idx = df_state->frame_index;
            }
            else if(ws->hover_eval_last_frame_idx+2 < df_state->frame_index)
            {
              df_request_frame();
            }
            if(ui_pressed(hover_eval_sig))
            {
              ws->hover_eval_focused = 1;
            }
          }
        }
        
        di_scope_close(scope);
        scratch_end(scratch);
      }
    }
    
    ////////////////////////////
    //- rjf: panel non-leaf UI (drag boundaries, drag/drop sites)
    //
    B32 is_changing_panel_boundaries = 0;
    ProfScope("non-leaf panel UI")
      for(DF_Panel *panel = ws->root_panel;
          !df_panel_is_nil(panel);
          panel = df_panel_rec_df_pre(panel).next)
    {
      //////////////////////////
      //- rjf: continue on leaf panels
      //
      if(df_panel_is_nil(panel->first))
      {
        continue;
      }
      
      //////////////////////////
      //- rjf: grab info
      //
      Axis2 split_axis = panel->split_axis;
      Rng2F32 panel_rect = df_target_rect_from_panel(content_rect, ws->root_panel, panel);
      
      //////////////////////////
      //- rjf: boundary tab-drag/drop sites
      //
      {
        DF_View *drag_view = df_view_from_handle(df_drag_drop_payload.view);
        if(df_drag_is_active() && !df_view_is_nil(drag_view))
        {
          //- rjf: params
          F32 drop_site_major_dim_px = ceil_f32(ui_top_font_size()*7.f);
          F32 drop_site_minor_dim_px = ceil_f32(ui_top_font_size()*5.f);
          F32 corner_radius = ui_top_font_size()*0.5f;
          F32 padding = ceil_f32(ui_top_font_size()*0.5f);
          
          //- rjf: special case - build Y boundary drop sites on root panel
          //
          // (this does not naturally follow from the below algorithm, since the
          // root level panel only splits on X)
          if(panel == ws->root_panel) UI_CornerRadius(corner_radius)
          {
            Vec2F32 panel_rect_center = center_2f32(panel_rect);
            Axis2 axis = axis2_flip(ws->root_panel->split_axis);
            for(EachEnumVal(Side, side))
            {
              UI_Key key = ui_key_from_stringf(ui_key_zero(), "root_extra_split_%i", side);
              Rng2F32 site_rect = panel_rect;
              site_rect.p0.v[axis2_flip(axis)] = panel_rect_center.v[axis2_flip(axis)] - drop_site_major_dim_px/2;
              site_rect.p1.v[axis2_flip(axis)] = panel_rect_center.v[axis2_flip(axis)] + drop_site_major_dim_px/2;
              site_rect.p0.v[axis] = panel_rect.v[side].v[axis] - drop_site_minor_dim_px/2;
              site_rect.p1.v[axis] = panel_rect.v[side].v[axis] + drop_site_minor_dim_px/2;
              
              // rjf: build
              UI_Box *site_box = &ui_g_nil_box;
              {
                UI_Rect(site_rect)
                {
                  site_box = ui_build_box_from_key(UI_BoxFlag_DropSite, key);
                  ui_signal_from_box(site_box);
                }
                UI_Box *site_box_viz = &ui_g_nil_box;
                UI_Parent(site_box) UI_WidthFill UI_HeightFill
                  UI_Padding(ui_px(padding, 1.f))
                  UI_Column
                  UI_Padding(ui_px(padding, 1.f))
                {
                  ui_set_next_child_layout_axis(axis2_flip(axis));
                  if(ui_key_match(key, ui_drop_hot_key()))
                  {
                    ui_set_next_palette(ui_build_palette(ui_top_palette(), .border = df_rgba_from_theme_color(DF_ThemeColor_Hover)));
                  }
                  site_box_viz = ui_build_box_from_key(UI_BoxFlag_DrawBackground|
                                                       UI_BoxFlag_DrawBorder|
                                                       UI_BoxFlag_DrawDropShadow|
                                                       UI_BoxFlag_DrawBackgroundBlur, ui_key_zero());
                }
                UI_Parent(site_box_viz) UI_WidthFill UI_HeightFill UI_Padding(ui_px(padding, 1.f))
                {
                  ui_set_next_child_layout_axis(axis);
                  UI_Box *row_or_column = ui_build_box_from_key(0, ui_key_zero()); UI_Parent(row_or_column) UI_Padding(ui_px(padding, 1.f))
                  {
                    ui_build_box_from_key(UI_BoxFlag_DrawBorder, ui_key_zero());
                    ui_spacer(ui_px(padding, 1.f));
                    ui_build_box_from_key(UI_BoxFlag_DrawBorder, ui_key_zero());
                  }
                }
              }
              
              // rjf: viz
              if(ui_key_match(site_box->key, ui_drop_hot_key()))
              {
                Rng2F32 future_split_rect = site_rect;
                future_split_rect.p0.v[axis] -= drop_site_major_dim_px;
                future_split_rect.p1.v[axis] += drop_site_major_dim_px;
                future_split_rect.p0.v[axis2_flip(axis)] = panel_rect.p0.v[axis2_flip(axis)];
                future_split_rect.p1.v[axis2_flip(axis)] = panel_rect.p1.v[axis2_flip(axis)];
                UI_Rect(future_split_rect) DF_Palette(DF_PaletteCode_DropSiteOverlay)
                {
                  ui_build_box_from_key(UI_BoxFlag_DrawBackground, ui_key_zero());
                }
              }
              
              // rjf: drop
              DF_DragDropPayload payload = {0};
              if(ui_key_match(site_box->key, ui_drop_hot_key()) && df_drag_drop(&payload))
              {
                Dir2 dir = (axis == Axis2_Y ? (side == Side_Min ? Dir2_Up : Dir2_Down) :
                            axis == Axis2_X ? (side == Side_Min ? Dir2_Left : Dir2_Right) :
                            Dir2_Invalid);
                if(dir != Dir2_Invalid)
                {
                  DF_Panel *split_panel = panel;
                  df_cmd(DF_CmdKind_SplitPanel,
                         .dst_panel  = df_handle_from_panel(split_panel),
                         .panel      = payload.panel,
                         .view       = payload.view,
                         .dir2       = dir);
                }
              }
            }
          }
          
          //- rjf: iterate all children, build boundary drop sites
          Axis2 split_axis = panel->split_axis;
          UI_CornerRadius(corner_radius) for(DF_Panel *child = panel->first;; child = child->next)
          {
            // rjf: form rect
            Rng2F32 child_rect = df_target_rect_from_panel_child(panel_rect, panel, child);
            Vec2F32 child_rect_center = center_2f32(child_rect);
            UI_Key key = ui_key_from_stringf(ui_key_zero(), "drop_boundary_%p_%p", panel, child);
            Rng2F32 site_rect = r2f32(child_rect_center, child_rect_center);
            site_rect.p0.v[split_axis] = child_rect.p0.v[split_axis] - drop_site_minor_dim_px/2;
            site_rect.p1.v[split_axis] = child_rect.p0.v[split_axis] + drop_site_minor_dim_px/2;
            site_rect.p0.v[axis2_flip(split_axis)] -= drop_site_major_dim_px/2;
            site_rect.p1.v[axis2_flip(split_axis)] += drop_site_major_dim_px/2;
            
            // rjf: build
            UI_Box *site_box = &ui_g_nil_box;
            {
              UI_Rect(site_rect)
              {
                site_box = ui_build_box_from_key(UI_BoxFlag_DropSite, key);
                ui_signal_from_box(site_box);
              }
              UI_Box *site_box_viz = &ui_g_nil_box;
              UI_Parent(site_box) UI_WidthFill UI_HeightFill
                UI_Padding(ui_px(padding, 1.f))
                UI_Column
                UI_Padding(ui_px(padding, 1.f))
              {
                ui_set_next_child_layout_axis(axis2_flip(split_axis));
                if(ui_key_match(key, ui_drop_hot_key()))
                {
                  ui_set_next_palette(ui_build_palette(ui_top_palette(), .border = df_rgba_from_theme_color(DF_ThemeColor_Hover)));
                }
                site_box_viz = ui_build_box_from_key(UI_BoxFlag_DrawBackground|
                                                     UI_BoxFlag_DrawBorder|
                                                     UI_BoxFlag_DrawDropShadow|
                                                     UI_BoxFlag_DrawBackgroundBlur, ui_key_zero());
              }
              UI_Parent(site_box_viz) UI_WidthFill UI_HeightFill UI_Padding(ui_px(padding, 1.f))
              {
                ui_set_next_child_layout_axis(split_axis);
                UI_Box *row_or_column = ui_build_box_from_key(0, ui_key_zero()); UI_Parent(row_or_column) UI_Padding(ui_px(padding, 1.f))
                {
                  ui_build_box_from_key(UI_BoxFlag_DrawBorder, ui_key_zero());
                  ui_spacer(ui_px(padding, 1.f));
                  ui_build_box_from_key(UI_BoxFlag_DrawBorder, ui_key_zero());
                }
              }
            }
            
            // rjf: viz
            if(ui_key_match(site_box->key, ui_drop_hot_key()))
            {
              Rng2F32 future_split_rect = site_rect;
              future_split_rect.p0.v[split_axis] -= drop_site_major_dim_px;
              future_split_rect.p1.v[split_axis] += drop_site_major_dim_px;
              future_split_rect.p0.v[axis2_flip(split_axis)] = child_rect.p0.v[axis2_flip(split_axis)];
              future_split_rect.p1.v[axis2_flip(split_axis)] = child_rect.p1.v[axis2_flip(split_axis)];
              UI_Rect(future_split_rect) DF_Palette(DF_PaletteCode_DropSiteOverlay)
              {
                ui_build_box_from_key(UI_BoxFlag_DrawBackground, ui_key_zero());
              }
            }
            
            // rjf: drop
            DF_DragDropPayload payload = {0};
            if(ui_key_match(site_box->key, ui_drop_hot_key()) && df_drag_drop(&payload))
            {
              Dir2 dir = (panel->split_axis == Axis2_X ? Dir2_Left : Dir2_Up);
              DF_Panel *split_panel = child;
              if(df_panel_is_nil(split_panel))
              {
                split_panel = panel->last;
                dir = (panel->split_axis == Axis2_X ? Dir2_Right : Dir2_Down);
              }
              df_cmd(DF_CmdKind_SplitPanel,
                     .dst_panel  = df_handle_from_panel(split_panel),
                     .panel      = payload.panel,
                     .view       = payload.view,
                     .dir2       = dir);
            }
            
            // rjf: exit on opl child
            if(df_panel_is_nil(child))
            {
              break;
            }
          }
        }
      }
      
      //////////////////////////
      //- rjf: do UI for drag boundaries between all children
      //
      for(DF_Panel *child = panel->first; !df_panel_is_nil(child) && !df_panel_is_nil(child->next); child = child->next)
      {
        DF_Panel *min_child = child;
        DF_Panel *max_child = min_child->next;
        Rng2F32 min_child_rect = df_target_rect_from_panel_child(panel_rect, panel, min_child);
        Rng2F32 max_child_rect = df_target_rect_from_panel_child(panel_rect, panel, max_child);
        Rng2F32 boundary_rect = {0};
        {
          boundary_rect.p0.v[split_axis] = min_child_rect.p1.v[split_axis] - ui_top_font_size()/3;
          boundary_rect.p1.v[split_axis] = max_child_rect.p0.v[split_axis] + ui_top_font_size()/3;
          boundary_rect.p0.v[axis2_flip(split_axis)] = panel_rect.p0.v[axis2_flip(split_axis)];
          boundary_rect.p1.v[axis2_flip(split_axis)] = panel_rect.p1.v[axis2_flip(split_axis)];
        }
        
        UI_Rect(boundary_rect)
        {
          ui_set_next_hover_cursor(split_axis == Axis2_X ? OS_Cursor_LeftRight : OS_Cursor_UpDown);
          UI_Box *box = ui_build_box_from_stringf(UI_BoxFlag_Clickable, "###%p_%p", min_child, max_child);
          UI_Signal sig = ui_signal_from_box(box);
          if(ui_double_clicked(sig))
          {
            ui_kill_action();
            F32 sum_pct = min_child->pct_of_parent + max_child->pct_of_parent;
            min_child->pct_of_parent = 0.5f * sum_pct;
            max_child->pct_of_parent = 0.5f * sum_pct;
          }
          else if(ui_pressed(sig))
          {
            Vec2F32 v = {min_child->pct_of_parent, max_child->pct_of_parent};
            ui_store_drag_struct(&v);
          }
          else if(ui_dragging(sig))
          {
            Vec2F32 v = *ui_get_drag_struct(Vec2F32);
            Vec2F32 mouse_delta      = ui_drag_delta();
            F32 total_size           = dim_2f32(panel_rect).v[split_axis];
            F32 min_pct__before      = v.v[0];
            F32 min_pixels__before   = min_pct__before * total_size;
            F32 min_pixels__after    = min_pixels__before + mouse_delta.v[split_axis];
            if(min_pixels__after < 50.f)
            {
              min_pixels__after = 50.f;
            }
            F32 min_pct__after       = min_pixels__after / total_size;
            F32 pct_delta            = min_pct__after - min_pct__before;
            F32 max_pct__before      = v.v[1];
            F32 max_pct__after       = max_pct__before - pct_delta;
            F32 max_pixels__after    = max_pct__after * total_size;
            if(max_pixels__after < 50.f)
            {
              max_pixels__after = 50.f;
              max_pct__after = max_pixels__after / total_size;
              pct_delta = -(max_pct__after - max_pct__before);
              min_pct__after = min_pct__before + pct_delta;
            }
            min_child->pct_of_parent = min_pct__after;
            max_child->pct_of_parent = max_pct__after;
            is_changing_panel_boundaries = 1;
          }
        }
      }
    }
    
    ////////////////////////////
    //- rjf: animate panels
    //
    {
      F32 rate = df_setting_val_from_code(DF_SettingCode_MenuAnimations).s32 ? 1 - pow_f32(2, (-50.f * df_state->frame_dt)) : 1.f;
      Vec2F32 content_rect_dim = dim_2f32(content_rect);
      for(DF_Panel *panel = ws->root_panel; !df_panel_is_nil(panel); panel = df_panel_rec_df_pre(panel).next)
      {
        Rng2F32 target_rect_px = df_target_rect_from_panel(content_rect, ws->root_panel, panel);
        Rng2F32 target_rect_pct = r2f32p(target_rect_px.x0/content_rect_dim.x,
                                         target_rect_px.y0/content_rect_dim.y,
                                         target_rect_px.x1/content_rect_dim.x,
                                         target_rect_px.y1/content_rect_dim.y);
        if(abs_f32(target_rect_pct.x0 - panel->animated_rect_pct.x0) > 0.005f ||
           abs_f32(target_rect_pct.y0 - panel->animated_rect_pct.y0) > 0.005f ||
           abs_f32(target_rect_pct.x1 - panel->animated_rect_pct.x1) > 0.005f ||
           abs_f32(target_rect_pct.y1 - panel->animated_rect_pct.y1) > 0.005f)
        {
          df_request_frame();
        }
        panel->animated_rect_pct.x0 += rate * (target_rect_pct.x0 - panel->animated_rect_pct.x0);
        panel->animated_rect_pct.y0 += rate * (target_rect_pct.y0 - panel->animated_rect_pct.y0);
        panel->animated_rect_pct.x1 += rate * (target_rect_pct.x1 - panel->animated_rect_pct.x1);
        panel->animated_rect_pct.y1 += rate * (target_rect_pct.y1 - panel->animated_rect_pct.y1);
        if(ws->frames_alive < 5 || is_changing_panel_boundaries)
        {
          panel->animated_rect_pct = target_rect_pct;
        }
      }
    }
    
    ////////////////////////////
    //- rjf: panel leaf UI
    //
    ProfScope("leaf panel UI")
      for(DF_Panel *panel = ws->root_panel;
          !df_panel_is_nil(panel);
          panel = df_panel_rec_df_pre(panel).next)
    {
      if(!df_panel_is_nil(panel->first)) {continue;}
      B32 panel_is_focused = (window_is_focused &&
                              !ws->menu_bar_focused &&
                              (!query_is_open || !ws->query_view_selected) &&
                              !ui_any_ctx_menu_is_open() &&
                              !ws->hover_eval_focused &&
                              ws->focused_panel == panel);
      UI_Focus(panel_is_focused ? UI_FocusKind_Null : UI_FocusKind_Off)
      {
        //////////////////////////
        //- rjf: calculate UI rectangles
        //
        Vec2F32 content_rect_dim = dim_2f32(content_rect);
        Rng2F32 panel_rect_pct = panel->animated_rect_pct;
        Rng2F32 panel_rect = r2f32p(panel_rect_pct.x0*content_rect_dim.x,
                                    panel_rect_pct.y0*content_rect_dim.y,
                                    panel_rect_pct.x1*content_rect_dim.x,
                                    panel_rect_pct.y1*content_rect_dim.y);
        panel_rect = pad_2f32(panel_rect, -1.f);
        F32 tab_bar_rheight = ui_top_font_size()*3.f;
        F32 tab_bar_vheight = ui_top_font_size()*2.6f;
        F32 tab_bar_rv_diff = tab_bar_rheight - tab_bar_vheight;
        F32 tab_spacing = ui_top_font_size()*0.4f;
        F32 filter_bar_height = ui_top_font_size()*3.f;
        Rng2F32 tab_bar_rect = r2f32p(panel_rect.x0, panel_rect.y0, panel_rect.x1, panel_rect.y0 + tab_bar_vheight);
        Rng2F32 content_rect = r2f32p(panel_rect.x0, panel_rect.y0+tab_bar_vheight, panel_rect.x1, panel_rect.y1);
        Rng2F32 filter_rect = {0};
        if(panel->tab_side == Side_Max)
        {
          tab_bar_rect.y0 = panel_rect.y1 - tab_bar_vheight;
          tab_bar_rect.y1 = panel_rect.y1;
          content_rect.y0 = panel_rect.y0;
          content_rect.y1 = panel_rect.y1 - tab_bar_vheight;
        }
        {
          DF_View *tab = df_selected_tab_from_panel(panel);
          if(tab->is_filtering_t > 0.01f)
          {
            filter_rect.x0 = content_rect.x0;
            filter_rect.y0 = content_rect.y0;
            filter_rect.x1 = content_rect.x1;
            content_rect.y0 += filter_bar_height*tab->is_filtering_t;
            filter_rect.y1 = content_rect.y0;
          }
        }
        
        //////////////////////////
        //- rjf: build combined split+movetab drag/drop sites
        //
        {
          DF_View *view = df_view_from_handle(df_drag_drop_payload.view);
          if(df_drag_is_active() && !df_view_is_nil(view) && contains_2f32(panel_rect, ui_mouse()))
          {
            F32 drop_site_dim_px = ceil_f32(ui_top_font_size()*7.f);
            Vec2F32 drop_site_half_dim = v2f32(drop_site_dim_px/2, drop_site_dim_px/2);
            Vec2F32 panel_center = center_2f32(panel_rect);
            F32 corner_radius = ui_top_font_size()*0.5f;
            F32 padding = ceil_f32(ui_top_font_size()*0.5f);
            struct
            {
              UI_Key key;
              Dir2 split_dir;
              Rng2F32 rect;
            }
            sites[] =
            {
              {
                ui_key_from_stringf(ui_key_zero(), "drop_split_center_%p", panel),
                Dir2_Invalid,
                r2f32(sub_2f32(panel_center, drop_site_half_dim),
                      add_2f32(panel_center, drop_site_half_dim))
              },
              {
                ui_key_from_stringf(ui_key_zero(), "drop_split_up_%p", panel),
                Dir2_Up,
                r2f32p(panel_center.x-drop_site_half_dim.x,
                       panel_center.y-drop_site_half_dim.y - drop_site_half_dim.y*2,
                       panel_center.x+drop_site_half_dim.x,
                       panel_center.y+drop_site_half_dim.y - drop_site_half_dim.y*2),
              },
              {
                ui_key_from_stringf(ui_key_zero(), "drop_split_down_%p", panel),
                Dir2_Down,
                r2f32p(panel_center.x-drop_site_half_dim.x,
                       panel_center.y-drop_site_half_dim.y + drop_site_half_dim.y*2,
                       panel_center.x+drop_site_half_dim.x,
                       panel_center.y+drop_site_half_dim.y + drop_site_half_dim.y*2),
              },
              {
                ui_key_from_stringf(ui_key_zero(), "drop_split_left_%p", panel),
                Dir2_Left,
                r2f32p(panel_center.x-drop_site_half_dim.x - drop_site_half_dim.x*2,
                       panel_center.y-drop_site_half_dim.y,
                       panel_center.x+drop_site_half_dim.x - drop_site_half_dim.x*2,
                       panel_center.y+drop_site_half_dim.y),
              },
              {
                ui_key_from_stringf(ui_key_zero(), "drop_split_right_%p", panel),
                Dir2_Right,
                r2f32p(panel_center.x-drop_site_half_dim.x + drop_site_half_dim.x*2,
                       panel_center.y-drop_site_half_dim.y,
                       panel_center.x+drop_site_half_dim.x + drop_site_half_dim.x*2,
                       panel_center.y+drop_site_half_dim.y),
              },
            };
            UI_CornerRadius(corner_radius)
              for(U64 idx = 0; idx < ArrayCount(sites); idx += 1)
            {
              UI_Key key = sites[idx].key;
              Dir2 dir = sites[idx].split_dir;
              Rng2F32 rect = sites[idx].rect;
              Axis2 split_axis = axis2_from_dir2(dir);
              Side split_side = side_from_dir2(dir);
              if(dir != Dir2_Invalid && split_axis == panel->parent->split_axis)
              {
                continue;
              }
              UI_Box *site_box = &ui_g_nil_box;
              {
                UI_Rect(rect)
                {
                  site_box = ui_build_box_from_key(UI_BoxFlag_DropSite, key);
                  ui_signal_from_box(site_box);
                }
                UI_Box *site_box_viz = &ui_g_nil_box;
                UI_Parent(site_box) UI_WidthFill UI_HeightFill
                  UI_Padding(ui_px(padding, 1.f))
                  UI_Column
                  UI_Padding(ui_px(padding, 1.f))
                {
                  ui_set_next_child_layout_axis(axis2_flip(split_axis));
                  if(ui_key_match(key, ui_drop_hot_key()))
                  {
                    ui_set_next_palette(ui_build_palette(ui_top_palette(), .border = df_rgba_from_theme_color(DF_ThemeColor_Hover)));
                  }
                  site_box_viz = ui_build_box_from_key(UI_BoxFlag_DrawBackground|
                                                       UI_BoxFlag_DrawBorder|
                                                       UI_BoxFlag_DrawDropShadow|
                                                       UI_BoxFlag_DrawBackgroundBlur, ui_key_zero());
                }
                if(dir != Dir2_Invalid)
                {
                  UI_Parent(site_box_viz) UI_WidthFill UI_HeightFill UI_Padding(ui_px(padding, 1.f))
                  {
                    ui_set_next_child_layout_axis(split_axis);
                    UI_Box *row_or_column = ui_build_box_from_key(0, ui_key_zero()); UI_Parent(row_or_column) UI_Padding(ui_px(padding, 1.f))
                    {
                      if(split_side == Side_Min) { ui_set_next_flags(UI_BoxFlag_DrawBackground); }
                      DF_Palette(DF_PaletteCode_DropSiteOverlay) ui_build_box_from_key(UI_BoxFlag_DrawBorder, ui_key_zero());
                      ui_spacer(ui_px(padding, 1.f));
                      if(split_side == Side_Max) { ui_set_next_flags(UI_BoxFlag_DrawBackground); }
                      DF_Palette(DF_PaletteCode_DropSiteOverlay) ui_build_box_from_key(UI_BoxFlag_DrawBorder, ui_key_zero());
                    }
                  }
                }
                else
                {
                  UI_Parent(site_box_viz) UI_WidthFill UI_HeightFill UI_Padding(ui_px(padding, 1.f))
                  {
                    ui_set_next_child_layout_axis(split_axis);
                    UI_Box *row_or_column = ui_build_box_from_key(0, ui_key_zero());
                    UI_Parent(row_or_column) UI_Padding(ui_px(padding, 1.f)) DF_Palette(DF_PaletteCode_DropSiteOverlay)
                    {
                      ui_build_box_from_key(UI_BoxFlag_DrawBorder|UI_BoxFlag_DrawBackground, ui_key_zero());
                    }
                  }
                }
              }
              DF_DragDropPayload payload = {0};
              if(ui_key_match(site_box->key, ui_drop_hot_key()) && df_drag_drop(&payload))
              {
                if(dir != Dir2_Invalid)
                {
                  df_cmd(DF_CmdKind_SplitPanel,
                         .dst_panel = df_handle_from_panel(panel),
                         .panel = payload.panel,
                         .view = payload.view,
                         .dir2 = dir);
                }
                else
                {
                  df_cmd(DF_CmdKind_MoveTab,
                         .dst_panel = df_handle_from_panel(panel),
                         .panel = payload.panel,
                         .view = payload.view,
                         .prev_view = df_handle_from_view(panel->last_tab_view));
                }
              }
            }
            for(U64 idx = 0; idx < ArrayCount(sites); idx += 1)
            {
              B32 is_drop_hot = ui_key_match(ui_drop_hot_key(), sites[idx].key);
              if(is_drop_hot)
              {
                Axis2 split_axis = axis2_from_dir2(sites[idx].split_dir);
                Side split_side = side_from_dir2(sites[idx].split_dir);
                Rng2F32 future_split_rect = panel_rect;
                if(sites[idx].split_dir != Dir2_Invalid)
                {
                  Vec2F32 panel_center = center_2f32(panel_rect);
                  future_split_rect.v[side_flip(split_side)].v[split_axis] = panel_center.v[split_axis];
                }
                UI_Rect(future_split_rect) DF_Palette(DF_PaletteCode_DropSiteOverlay)
                {
                  ui_build_box_from_key(UI_BoxFlag_DrawBackground, ui_key_zero());
                }
              }
            }
          }
        }
        
        //////////////////////////
        //- rjf: build catch-all panel drop-site
        //
        B32 catchall_drop_site_hovered = 0;
        if(df_drag_is_active() && ui_key_match(ui_key_zero(), ui_drop_hot_key()))
        {
          UI_Rect(panel_rect)
          {
            UI_Key key = ui_key_from_stringf(ui_key_zero(), "catchall_drop_site_%p", panel);
            UI_Box *catchall_drop_site = ui_build_box_from_key(UI_BoxFlag_DropSite, key);
            ui_signal_from_box(catchall_drop_site);
            catchall_drop_site_hovered = ui_key_match(key, ui_drop_hot_key());
          }
        }
        
        //////////////////////////
        //- rjf: build filtering box
        //
        {
          DF_View *view = df_selected_tab_from_panel(panel);
          UI_Focus(UI_FocusKind_On)
          {
            if(view->is_filtering && ui_is_focus_active() && ui_slot_press(UI_EventActionSlot_Accept))
            {
              df_cmd(DF_CmdKind_ApplyFilter, .view = df_handle_from_view(view));
            }
            if(view->is_filtering || view->is_filtering_t > 0.01f)
            {
              UI_Box *filter_box = &ui_g_nil_box;
              UI_Rect(filter_rect)
              {
                ui_set_next_child_layout_axis(Axis2_X);
                filter_box = ui_build_box_from_stringf(UI_BoxFlag_DrawBackground|UI_BoxFlag_Clip|UI_BoxFlag_DrawBorder, "filter_box_%p", view);
              }
              UI_Parent(filter_box) UI_WidthFill UI_HeightFill
              {
                UI_PrefWidth(ui_em(3.f, 1.f))
                  UI_FlagsAdd(UI_BoxFlag_DrawTextWeak)
                  DF_Font(DF_FontSlot_Icons)
                  UI_TextAlignment(UI_TextAlign_Center)
                  ui_label(df_g_icon_kind_text_table[DF_IconKind_Find]);
                UI_PrefWidth(ui_text_dim(10, 1))
                {
                  ui_label(str8_lit("Filter"));
                }
                ui_spacer(ui_em(0.5f, 1.f));
                DF_Font(view->spec->info.flags & DF_ViewSpecFlag_FilterIsCode ? DF_FontSlot_Code : DF_FontSlot_Main)
                  UI_Focus(view->is_filtering ? UI_FocusKind_On : UI_FocusKind_Off)
                  UI_TextPadding(ui_top_font_size()*0.5f)
                {
                  UI_Signal sig = df_line_edit(DF_LineEditFlag_CodeContents*!!(view->spec->info.flags & DF_ViewSpecFlag_FilterIsCode),
                                               0,
                                               0,
                                               &view->query_cursor,
                                               &view->query_mark,
                                               view->query_buffer,
                                               sizeof(view->query_buffer),
                                               &view->query_string_size,
                                               0,
                                               str8(view->query_buffer, view->query_string_size),
                                               str8_lit("###filter_text_input"));
                  if(ui_pressed(sig))
                  {
                    df_cmd(DF_CmdKind_FocusPanel, .panel = df_handle_from_panel(panel));
                  }
                }
              }
            }
          }
        }
        
        //////////////////////////
        //- rjf: panel not selected? -> darken
        //
        if(panel != ws->focused_panel)
        {
          UI_Palette(ui_build_palette(0, .background = df_rgba_from_theme_color(DF_ThemeColor_InactivePanelOverlay)))
            UI_Rect(content_rect)
          {
            ui_build_box_from_key(UI_BoxFlag_DrawBackground, ui_key_zero());
          }
        }
        
        //////////////////////////
        //- rjf: build panel container box
        //
        UI_Box *panel_box = &ui_g_nil_box;
        UI_Rect(content_rect) UI_ChildLayoutAxis(Axis2_Y) UI_CornerRadius(0) UI_Focus(UI_FocusKind_On)
        {
          UI_Key panel_key = df_ui_key_from_panel(panel);
          panel_box = ui_build_box_from_key(UI_BoxFlag_MouseClickable|
                                            UI_BoxFlag_Clip|
                                            UI_BoxFlag_DrawBorder|
                                            UI_BoxFlag_DisableFocusOverlay|
                                            ((ws->focused_panel != panel)*UI_BoxFlag_DisableFocusBorder)|
                                            ((ws->focused_panel != panel)*UI_BoxFlag_DrawOverlay),
                                            panel_key);
        }
        
        //////////////////////////
        //- rjf: loading animation for stable view
        //
        UI_Parent(panel_box)
        {
          DF_View *view = df_selected_tab_from_panel(panel);
          df_loading_overlay(panel_rect, view->loading_t, view->loading_progress_v, view->loading_progress_v_target);
        }
        
        //////////////////////////
        //- rjf: build selected tab view
        //
        UI_Parent(panel_box)
          UI_Focus(panel_is_focused ? UI_FocusKind_Null : UI_FocusKind_Off)
          UI_WidthFill
        {
          //- rjf: push interaction registers, fill with per-view states
          df_push_regs();
          {
            DF_View *view = df_selected_tab_from_panel(panel);
            df_regs()->panel = df_handle_from_panel(panel);
            df_regs()->view  = df_handle_from_view(view);
            df_regs()->file_path = d_file_path_from_eval_string(d_frame_arena(), str8(view->query_buffer, view->query_string_size));
          }
          
          //- rjf: build view container
          UI_Box *view_container_box = &ui_g_nil_box;
          UI_FixedWidth(dim_2f32(content_rect).x)
            UI_FixedHeight(dim_2f32(content_rect).y)
            UI_ChildLayoutAxis(Axis2_Y)
          {
            view_container_box = ui_build_box_from_key(0, ui_key_zero());
          }
          
          //- rjf: build empty view
          UI_Parent(view_container_box) if(df_view_is_nil(df_selected_tab_from_panel(panel)))
          {
            DF_VIEW_UI_FUNCTION_NAME(empty)(&df_nil_view, &md_nil_node, str8_zero(), content_rect);
          }
          
          //- rjf: build tab view
          UI_Parent(view_container_box) if(!df_view_is_nil(df_selected_tab_from_panel(panel)))
          {
            DF_View *view = df_selected_tab_from_panel(panel);
            DF_ViewUIFunctionType *build_view_ui_function = view->spec->info.ui_hook;
            build_view_ui_function(view, view->params_roots[view->params_read_gen%ArrayCount(view->params_roots)], str8(view->query_buffer, view->query_string_size), content_rect);
          }
          
          //- rjf: pop interaction registers; commit if this is the selected view
          DF_Regs *view_regs = df_pop_regs();
          if(ws->focused_panel == panel)
          {
            MemoryCopyStruct(df_regs(), view_regs);
          }
        }
        
        //////////////////////////
        //- rjf: take events to automatically start/end filtering, if applicable
        //
        UI_Focus(UI_FocusKind_On)
        {
          DF_View *view = df_selected_tab_from_panel(panel);
          if(ui_is_focus_active() && view->spec->info.flags & DF_ViewSpecFlag_TypingAutomaticallyFilters && !view->is_filtering)
          {
            for(UI_Event *evt = 0; ui_next_event(&evt);)
            {
              if(evt->flags & UI_EventFlag_Paste)
              {
                ui_eat_event(evt);
                df_cmd(DF_CmdKind_Filter);
                df_cmd(DF_CmdKind_Paste);
              }
              else if(evt->string.size != 0 && evt->kind == UI_EventKind_Text)
              {
                ui_eat_event(evt);
                df_cmd(DF_CmdKind_Filter);
                df_cmd(DF_CmdKind_InsertText, .string = evt->string);
              }
            }
          }
          if(view->spec->info.flags & DF_ViewSpecFlag_CanFilter && (view->query_string_size != 0 || view->is_filtering) && ui_is_focus_active() && ui_slot_press(UI_EventActionSlot_Cancel))
          {
            df_cmd(DF_CmdKind_ClearFilter, .view = df_handle_from_view(view));
          }
        }
        
        //////////////////////////
        //- rjf: consume panel fallthrough interaction events
        //
        UI_Signal panel_sig = ui_signal_from_box(panel_box);
        if(ui_pressed(panel_sig))
        {
          df_cmd(DF_CmdKind_FocusPanel, .panel = df_handle_from_panel(panel));
        }
        
        //////////////////////////
        //- rjf: build tab bar
        //
        UI_Focus(UI_FocusKind_Off)
        {
          Temp scratch = scratch_begin(0, 0);
          
          // rjf: types
          typedef struct DropSite DropSite;
          struct DropSite
          {
            F32 p;
            DF_View *prev_view;
          };
          
          // rjf: prep output data
          DF_View *next_selected_tab_view = df_selected_tab_from_panel(panel);
          UI_Box *tab_bar_box = &ui_g_nil_box;
          U64 drop_site_count = panel->tab_view_count+1;
          DropSite *drop_sites = push_array(scratch.arena, DropSite, drop_site_count);
          F32 drop_site_max_p = 0;
          U64 view_idx = 0;
          
          // rjf: build
          UI_CornerRadius(0)
          {
            UI_Rect(tab_bar_rect) tab_bar_box = ui_build_box_from_stringf(UI_BoxFlag_Clip|UI_BoxFlag_AllowOverflowY|UI_BoxFlag_ViewClampX|UI_BoxFlag_ViewScrollX|UI_BoxFlag_Clickable, "tab_bar_%p", panel);
            if(panel->tab_side == Side_Max)
            {
              tab_bar_box->view_off.y = tab_bar_box->view_off_target.y = (tab_bar_rheight - tab_bar_vheight);
            }
            else
            {
              tab_bar_box->view_off.y = tab_bar_box->view_off_target.y = 0;
            }
          }
          UI_Parent(tab_bar_box) UI_PrefHeight(ui_pct(1, 0))
          {
            Temp scratch = scratch_begin(0, 0);
            F32 corner_radius = ui_em(0.6f, 1.f).value;
            ui_spacer(ui_px(1.f, 1.f));
            
            // rjf: build tabs
            UI_PrefWidth(ui_em(18.f, 0.5f))
              UI_CornerRadius00(panel->tab_side == Side_Min ? corner_radius : 0)
              UI_CornerRadius01(panel->tab_side == Side_Min ? 0 : corner_radius)
              UI_CornerRadius10(panel->tab_side == Side_Min ? corner_radius : 0)
              UI_CornerRadius11(panel->tab_side == Side_Min ? 0 : corner_radius)
              for(DF_View *view = panel->first_tab_view;; view = view->order_next, view_idx += 1)
            {
              temp_end(scratch);
              if(df_view_is_project_filtered(view)) { continue; }
              
              // rjf: if before this tab is the prev-view of the current tab drag,
              // draw empty space
              if(df_drag_is_active() && catchall_drop_site_hovered)
              {
                DF_Panel *dst_panel = df_panel_from_handle(df_last_drag_drop_panel);
                DF_View *drag_view = df_view_from_handle(df_drag_drop_payload.view);
                DF_View *dst_prev_view = df_view_from_handle(df_last_drag_drop_prev_tab);
                if(dst_panel == panel &&
                   ((!df_view_is_nil(view) && dst_prev_view == view->order_prev && drag_view != view && drag_view != view->order_prev) ||
                    (df_view_is_nil(view) && dst_prev_view == panel->last_tab_view && drag_view != panel->last_tab_view)))
                {
                  UI_PrefWidth(ui_em(9.f, 0.2f)) UI_Column
                  {
                    ui_spacer(ui_em(0.2f, 1.f));
                    UI_CornerRadius00(corner_radius)
                      UI_CornerRadius10(corner_radius)
                      DF_Palette(DF_PaletteCode_DropSiteOverlay)
                    {
                      ui_build_box_from_key(UI_BoxFlag_DrawBackground|UI_BoxFlag_DrawBorder, ui_key_zero());
                    }
                  }
                }
              }
              
              // rjf: end on nil view
              if(df_view_is_nil(view))
              {
                break;
              }
              
              // rjf: gather info for this tab
              B32 view_is_selected = (view == df_selected_tab_from_panel(panel));
              DF_IconKind icon_kind = df_icon_kind_from_view(view);
              DR_FancyStringList title_fstrs = df_title_fstrs_from_view(scratch.arena, view, ui_top_palette()->text, ui_top_palette()->text_weak, ui_top_font_size());
              
              // rjf: begin vertical region for this tab
              ui_set_next_child_layout_axis(Axis2_Y);
              UI_Box *tab_column_box = ui_build_box_from_stringf(!is_changing_panel_boundaries*UI_BoxFlag_AnimatePosX, "tab_column_%p", view);
              
              // rjf: build tab container box
              UI_Parent(tab_column_box) UI_PrefHeight(ui_px(tab_bar_vheight, 1)) DF_Palette(view_is_selected ? DF_PaletteCode_Tab : DF_PaletteCode_TabInactive)
              {
                if(panel->tab_side == Side_Max)
                {
                  ui_spacer(ui_px(tab_bar_rv_diff-1.f, 1.f));
                }
                else
                {
                  ui_spacer(ui_px(1.f, 1.f));
                }
                ui_set_next_hover_cursor(OS_Cursor_HandPoint);
                UI_Box *tab_box = ui_build_box_from_stringf(UI_BoxFlag_DrawHotEffects|
                                                            UI_BoxFlag_DrawBackground|
                                                            UI_BoxFlag_DrawBorder|
                                                            (UI_BoxFlag_DrawDropShadow*view_is_selected)|
                                                            UI_BoxFlag_Clickable,
                                                            "tab_%p", view);
                
                // rjf: build tab contents
                UI_Parent(tab_box)
                {
                  UI_WidthFill UI_Row
                  {
                    ui_spacer(ui_em(0.5f, 1.f));
                    if(icon_kind != DF_IconKind_Null)
                    {
                      UI_FlagsAdd(UI_BoxFlag_DrawTextWeak)
                        DF_Font(DF_FontSlot_Icons)
                        UI_TextAlignment(UI_TextAlign_Center)
                        UI_PrefWidth(ui_em(1.75f, 1.f))
                        ui_label(df_g_icon_kind_text_table[icon_kind]);
                    }
                    UI_PrefWidth(ui_text_dim(10, 0))
                    {
                      UI_Box *name_box = ui_build_box_from_key(UI_BoxFlag_DrawText, ui_key_zero());
                      ui_box_equip_display_fancy_strings(name_box, &title_fstrs);
                    }
                  }
                  UI_PrefWidth(ui_em(2.35f, 1.f)) UI_TextAlignment(UI_TextAlign_Center)
                    DF_Font(DF_FontSlot_Icons)
                    UI_FontSize(df_font_size_from_slot(DF_FontSlot_Icons)*0.75f)
                    UI_Flags(UI_BoxFlag_DrawTextWeak)
                    UI_CornerRadius00(0)
                    UI_CornerRadius01(0)
                  {
                    UI_Palette *palette = ui_build_palette(ui_top_palette());
                    palette->background = v4f32(0, 0, 0, 0);
                    ui_set_next_palette(palette);
                    UI_Signal sig = ui_buttonf("%S###close_view_%p", df_g_icon_kind_text_table[DF_IconKind_X], view);
                    if(ui_clicked(sig) || ui_middle_clicked(sig))
                    {
                      df_cmd(DF_CmdKind_CloseTab, .panel = df_handle_from_panel(panel), .view = df_handle_from_view(view));
                    }
                  }
                }
                
                // rjf: consume events for tab clicking
                {
                  UI_Signal sig = ui_signal_from_box(tab_box);
                  if(ui_pressed(sig))
                  {
                    next_selected_tab_view = view;
                    df_cmd(DF_CmdKind_FocusPanel, .panel = df_handle_from_panel(panel));
                  }
                  else if(ui_dragging(sig) && !df_drag_is_active() && length_2f32(ui_drag_delta()) > 10.f)
                  {
                    DF_DragDropPayload payload = {0};
                    {
                      payload.key = sig.box->key;
                      payload.panel = df_handle_from_panel(panel);
                      payload.view = df_handle_from_view(view);
                    }
                    df_drag_begin(&payload);
                  }
                  else if(ui_right_clicked(sig))
                  {
                    ui_ctx_menu_open(df_state->tab_ctx_menu_key, sig.box->key, v2f32(0, sig.box->rect.y1 - sig.box->rect.y0));
                    ws->tab_ctx_menu_panel = df_handle_from_panel(panel);
                    ws->tab_ctx_menu_view = df_handle_from_view(view);
                  }
                  else if(ui_middle_clicked(sig))
                  {
                    df_cmd(DF_CmdKind_CloseTab, .panel = df_handle_from_panel(panel), .view = df_handle_from_view(view));
                  }
                }
              }
              
              // rjf: space for next tab
              {
                ui_spacer(ui_em(0.3f, 1.f));
              }
              
              // rjf: store off drop-site
              drop_sites[view_idx].p = tab_column_box->rect.x0 - tab_spacing/2;
              drop_sites[view_idx].prev_view = view->order_prev;
              drop_site_max_p = Max(tab_column_box->rect.x1, drop_site_max_p);
            }
            
            // rjf: build add-new-tab button
            UI_TextAlignment(UI_TextAlign_Center)
              UI_PrefWidth(ui_px(tab_bar_vheight, 1.f))
              UI_PrefHeight(ui_px(tab_bar_vheight, 1.f))
              UI_Column
            {
              if(panel->tab_side == Side_Max)
              {
                ui_spacer(ui_px(tab_bar_rv_diff-1.f, 1.f));
              }
              else
              {
                ui_spacer(ui_px(1.f, 1.f));
              }
              UI_CornerRadius00(panel->tab_side == Side_Min ? corner_radius : 0)
                UI_CornerRadius10(panel->tab_side == Side_Min ? corner_radius : 0)
                UI_CornerRadius01(panel->tab_side == Side_Max ? corner_radius : 0)
                UI_CornerRadius11(panel->tab_side == Side_Max ? corner_radius : 0)
                DF_Font(DF_FontSlot_Icons)
                UI_FontSize(ui_top_font_size())
                UI_FlagsAdd(UI_BoxFlag_DrawTextWeak)
                UI_HoverCursor(OS_Cursor_HandPoint)
                DF_Palette(DF_PaletteCode_ImplicitButton)
              {
                UI_Box *add_new_box = ui_build_box_from_stringf(UI_BoxFlag_DrawBackground|
                                                                UI_BoxFlag_DrawText|
                                                                UI_BoxFlag_DrawBorder|
                                                                UI_BoxFlag_DrawHotEffects|
                                                                UI_BoxFlag_DrawActiveEffects|
                                                                UI_BoxFlag_Clickable|
                                                                UI_BoxFlag_DisableTextTrunc,
                                                                "%S##add_new_tab_button_%p",
                                                                df_g_icon_kind_text_table[DF_IconKind_Add],
                                                                panel);
                UI_Signal sig = ui_signal_from_box(add_new_box);
                if(ui_clicked(sig))
                {
                  df_cmd(DF_CmdKind_FocusPanel, .panel = df_handle_from_panel(panel));
                  UI_Key view_menu_key = ui_key_from_string(ui_key_zero(), str8_lit("_view_menu_key_"));
                  ui_ctx_menu_open(view_menu_key, add_new_box->key, v2f32(0, tab_bar_vheight));
                }
              }
            }
            
            scratch_end(scratch);
          }
          
          // rjf: interact with tab bar
          ui_signal_from_box(tab_bar_box);
          
          // rjf: fill out last drop site
          {
            drop_sites[drop_site_count-1].p = drop_site_max_p;
            drop_sites[drop_site_count-1].prev_view = panel->last_tab_view;
          }
          
          // rjf: more precise drop-sites on tab bar
          {
            Vec2F32 mouse = ui_mouse();
            DF_View *view = df_view_from_handle(df_drag_drop_payload.view);
            if(df_drag_is_active() && window_is_focused && contains_2f32(panel_rect, mouse) && !df_view_is_nil(view))
            {
              // rjf: mouse => hovered drop site
              F32 min_distance = 0;
              DropSite *active_drop_site = 0;
              if(catchall_drop_site_hovered)
              {
                for(U64 drop_site_idx = 0; drop_site_idx < drop_site_count; drop_site_idx += 1)
                {
                  F32 distance = abs_f32(drop_sites[drop_site_idx].p - mouse.x);
                  if(drop_site_idx == 0 || distance < min_distance)
                  {
                    active_drop_site = &drop_sites[drop_site_idx];
                    min_distance = distance;
                  }
                }
              }
              
              // rjf: store closest prev-view
              if(active_drop_site != 0)
              {
                df_last_drag_drop_prev_tab = df_handle_from_view(active_drop_site->prev_view);
              }
              else
              {
                df_last_drag_drop_prev_tab = d_handle_zero();
              }
              
              // rjf: vis
              DF_Panel *drag_panel = df_panel_from_handle(df_drag_drop_payload.panel);
              if(!df_view_is_nil(view) && active_drop_site != 0) 
              {
                DF_Palette(DF_PaletteCode_DropSiteOverlay) UI_Rect(tab_bar_rect)
                  ui_build_box_from_key(UI_BoxFlag_DrawBackground, ui_key_zero());
              }
              
              // rjf: drop
              DF_DragDropPayload payload = df_drag_drop_payload;
              if(catchall_drop_site_hovered && (active_drop_site != 0 && df_drag_drop(&payload)))
              {
                DF_View *view = df_view_from_handle(payload.view);
                DF_Panel *src_panel = df_panel_from_handle(payload.panel);
                if(!df_panel_is_nil(panel) && !df_view_is_nil(view))
                {
                  df_cmd(DF_CmdKind_MoveTab,
                         .panel = df_handle_from_panel(src_panel),
                         .dst_panel = df_handle_from_panel(panel),
                         .view = df_handle_from_view(view),
                         .prev_view = df_handle_from_view(active_drop_site->prev_view));
                }
              }
            }
          }
          
          // rjf: apply tab change
          {
            panel->selected_tab_view = df_handle_from_view(next_selected_tab_view);
          }
          
          scratch_end(scratch);
        }
        
        //////////////////////////
        //- rjf: less granular panel for tabs & entities drop-site
        //
        if(catchall_drop_site_hovered)
        {
          df_last_drag_drop_panel = df_handle_from_panel(panel);
          
          DF_DragDropPayload *payload = &df_drag_drop_payload;
          DF_View *dragged_view = df_view_from_handle(payload->view);
          B32 view_is_in_panel = 0;
          for(DF_View *view = panel->first_tab_view; !df_view_is_nil(view); view = view->order_next)
          {
            if(df_view_is_project_filtered(view)) { continue; }
            if(view == dragged_view)
            {
              view_is_in_panel = 1;
              break;
            }
          }
          
          if(view_is_in_panel == 0)
          {
            // rjf: vis
            {
              DF_Palette(DF_PaletteCode_DropSiteOverlay) UI_Rect(content_rect)
                ui_build_box_from_key(UI_BoxFlag_DrawBackground, ui_key_zero());
            }
            
            // rjf: drop
            {
              DF_DragDropPayload payload = {0};
              if(df_drag_drop(&payload))
              {
                DF_Panel *src_panel = df_panel_from_handle(payload.panel);
                DF_View *view = df_view_from_handle(payload.view);
                DF_Entity *entity = df_entity_from_handle(payload.entity);
                
                // rjf: view drop
                if(!df_view_is_nil(view))
                {
                  df_cmd(DF_CmdKind_MoveTab,
                         .prev_view = df_handle_from_view(panel->last_tab_view),
                         .panel = df_handle_from_panel(src_panel),
                         .dst_panel = df_handle_from_panel(panel),
                         .view = df_handle_from_view(view));
                }
                
                // rjf: entity drop
                if(!df_entity_is_nil(entity))
                {
                  df_cmd(DF_CmdKind_SpawnEntityView,
                         .panel = df_handle_from_panel(panel),
                         .cursor = payload.text_point,
                         .entity = df_handle_from_entity(entity));
                }
              }
            }
          }
        }
        
        //////////////////////////
        //- rjf: accept file drops
        //
        for(UI_Event *evt = 0; ui_next_event(&evt);)
        {
          if(evt->kind == UI_EventKind_FileDrop && contains_2f32(content_rect, evt->pos))
          {
            for(String8Node *n = evt->paths.first; n != 0; n = n->next)
            {
              Temp scratch = scratch_begin(0, 0);
              df_cmd(DF_CmdKind_Open, .file_path = path_normalized_from_string(scratch.arena, n->string));
              scratch_end(scratch);
            }
            ui_eat_event(evt);
          }
        }
      }
    }
    
    ////////////////////////////
    //- rjf: animate views
    //
    {
      Temp scratch = scratch_begin(0, 0);
      typedef struct Task Task;
      struct Task
      {
        Task *next;
        DF_Panel *panel;
        DF_View *list_first;
        DF_View *transient_owner;
      };
      Task start_task = {0, &df_nil_panel, ws->query_view_stack_top};
      Task *first_task = &start_task;
      Task *last_task = first_task;
      F32 rate = 1 - pow_f32(2, (-10.f * df_state->frame_dt));
      F32 fast_rate = 1 - pow_f32(2, (-40.f * df_state->frame_dt));
      for(DF_Panel *panel = ws->root_panel;
          !df_panel_is_nil(panel);
          panel = df_panel_rec_df_pre(panel).next)
      {
        Task *t = push_array(scratch.arena, Task, 1);
        SLLQueuePush(first_task, last_task, t);
        t->panel = panel;
        t->list_first = panel->first_tab_view;
      }
      for(Task *t = first_task; t != 0; t = t->next)
      {
        DF_View *list_first = t->list_first;
        for(DF_View *view = list_first; !df_view_is_nil(view); view = view->order_next)
        {
          if(!df_view_is_nil(view->first_transient))
          {
            Task *task = push_array(scratch.arena, Task, 1);
            SLLQueuePush(first_task, last_task, task);
            task->panel = t->panel;
            task->list_first = view->first_transient;
            task->transient_owner = view;
          }
          if(window_is_focused)
          {
            if(abs_f32(view->loading_t_target - view->loading_t) > 0.01f ||
               abs_f32(view->scroll_pos.x.off) > 0.01f ||
               abs_f32(view->scroll_pos.y.off) > 0.01f ||
               abs_f32(view->is_filtering_t - (F32)!!view->is_filtering))
            {
              df_request_frame();
            }
            if(view->loading_t_target != 0 && (view == df_selected_tab_from_panel(t->panel) ||
                                               t->transient_owner == df_selected_tab_from_panel(t->panel)))
            {
              df_request_frame();
            }
          }
          view->loading_t += (view->loading_t_target - view->loading_t) * rate;
          view->is_filtering_t += ((F32)!!view->is_filtering - view->is_filtering_t) * fast_rate;
          view->scroll_pos.x.off -= view->scroll_pos.x.off * (df_setting_val_from_code(DF_SettingCode_ScrollingAnimations).s32 ? fast_rate : 1.f);
          view->scroll_pos.y.off -= view->scroll_pos.y.off * (df_setting_val_from_code(DF_SettingCode_ScrollingAnimations).s32 ? fast_rate : 1.f);
          if(abs_f32(view->scroll_pos.x.off) < 0.01f)
          {
            view->scroll_pos.x.off = 0;
          }
          if(abs_f32(view->scroll_pos.y.off) < 0.01f)
          {
            view->scroll_pos.y.off = 0;
          }
          if(abs_f32(view->is_filtering_t - (F32)!!view->is_filtering) < 0.01f)
          {
            view->is_filtering_t = (F32)!!view->is_filtering;
          }
          if(view == df_selected_tab_from_panel(t->panel) ||
             t->transient_owner == df_selected_tab_from_panel(t->panel))
          {
            view->loading_t_target = 0;
          }
        }
      }
      scratch_end(scratch);
    }
    
    ////////////////////////////
    //- rjf: drag/drop cancelling
    //
    if(df_drag_is_active() && ui_slot_press(UI_EventActionSlot_Cancel))
    {
      df_drag_kill();
      ui_kill_action();
    }
    
    ////////////////////////////
    //- rjf: font size changing
    //
    for(UI_Event *evt = 0; ui_next_event(&evt);)
    {
      if(evt->kind == UI_EventKind_Scroll && evt->modifiers & OS_EventFlag_Ctrl)
      {
        ui_eat_event(evt);
        if(evt->delta_2f32.y < 0)
        {
          df_cmd(DF_CmdKind_IncUIFontScale, .window = df_handle_from_window(ws));
        }
        else if(evt->delta_2f32.y > 0)
        {
          df_cmd(DF_CmdKind_DecUIFontScale, .window = df_handle_from_window(ws));
        }
      }
    }
    
    ui_end_build();
  }
  
  //////////////////////////////
  //- rjf: ensure hover eval is in-bounds
  //
  if(!ui_box_is_nil(hover_eval_box))
  {
    UI_Box *root = hover_eval_box;
    Rng2F32 window_rect = os_client_rect_from_window(ui_window());
    Rng2F32 root_rect = root->rect;
    Vec2F32 shift =
    {
      -ClampBot(0, root_rect.x1 - window_rect.x1),
      -ClampBot(0, root_rect.y1 - window_rect.y1),
    };
    Rng2F32 new_root_rect = shift_2f32(root_rect, shift);
    root->fixed_position = new_root_rect.p0;
    root->fixed_size = dim_2f32(new_root_rect);
    root->rect = new_root_rect;
    for(Axis2 axis = (Axis2)0; axis < Axis2_COUNT; axis = (Axis2)(axis + 1))
    {
      ui_calc_sizes_standalone__in_place_rec(root, axis);
      ui_calc_sizes_upwards_dependent__in_place_rec(root, axis);
      ui_calc_sizes_downwards_dependent__in_place_rec(root, axis);
      ui_layout_enforce_constraints__in_place_rec(root, axis);
      ui_layout_position__in_place_rec(root, axis);
    }
  }
  
  //////////////////////////////
  //- rjf: attach autocomp box to root, or hide if it has not been renewed
  //
  if(!ui_box_is_nil(autocomp_box) && ws->autocomp_last_frame_idx+1 >= df_state->frame_index+1)
  {
    UI_Box *autocomp_root_box = ui_box_from_key(ws->autocomp_root_key);
    if(!ui_box_is_nil(autocomp_root_box))
    {
      Vec2F32 size = autocomp_box->fixed_size;
      autocomp_box->fixed_position = v2f32(autocomp_root_box->rect.x0, autocomp_root_box->rect.y1);
      autocomp_box->rect = r2f32(autocomp_box->fixed_position, add_2f32(autocomp_box->fixed_position, size));
      for(Axis2 axis = (Axis2)0; axis < Axis2_COUNT; axis = (Axis2)(axis + 1))
      {
        ui_calc_sizes_standalone__in_place_rec(autocomp_box, axis);
        ui_calc_sizes_upwards_dependent__in_place_rec(autocomp_box, axis);
        ui_calc_sizes_downwards_dependent__in_place_rec(autocomp_box, axis);
        ui_layout_enforce_constraints__in_place_rec(autocomp_box, axis);
        ui_layout_position__in_place_rec(autocomp_box, axis);
      }
    }
  }
  else if(!ui_box_is_nil(autocomp_box) && ws->autocomp_last_frame_idx+1 < df_state->frame_index+1)
  {
    UI_Box *autocomp_root_box = ui_box_from_key(ws->autocomp_root_key);
    if(!ui_box_is_nil(autocomp_root_box))
    {
      Vec2F32 size = autocomp_box->fixed_size;
      Rng2F32 window_rect = os_client_rect_from_window(ws->os);
      autocomp_box->fixed_position = v2f32(window_rect.x1, window_rect.y1);
      autocomp_box->rect = r2f32(autocomp_box->fixed_position, add_2f32(autocomp_box->fixed_position, size));
    }
  }
  
  //////////////////////////////
  //- rjf: hover eval cancelling
  //
  if(ws->hover_eval_string.size != 0 && ui_slot_press(UI_EventActionSlot_Cancel))
  {
    MemoryZeroStruct(&ws->hover_eval_string);
    arena_clear(ws->hover_eval_arena);
    df_request_frame();
  }
  
  //////////////////////////////
  //- rjf: animate
  //
  if(ui_animating_from_state(ws->ui))
  {
    df_request_frame();
  }
  
  //////////////////////////////
  //- rjf: draw UI
  //
  ws->draw_bucket = dr_bucket_make();
  D_BucketScope(ws->draw_bucket)
    ProfScope("draw UI")
  {
    Temp scratch = scratch_begin(0, 0);
    
    //- rjf: set up heatmap buckets
    F32 heatmap_bucket_size = 32.f;
    U64 *heatmap_buckets = 0;
    U64 heatmap_bucket_pitch = 0;
    U64 heatmap_bucket_count = 0;
    if(DEV_draw_ui_box_heatmap)
    {
      Rng2F32 rect = os_client_rect_from_window(ws->os);
      Vec2F32 size = dim_2f32(rect);
      Vec2S32 buckets_dim = {(S32)(size.x/heatmap_bucket_size), (S32)(size.y/heatmap_bucket_size)};
      heatmap_bucket_pitch = buckets_dim.x;
      heatmap_bucket_count = buckets_dim.x*buckets_dim.y;
      heatmap_buckets = push_array(scratch.arena, U64, heatmap_bucket_count);
    }
    
    //- rjf: draw background color
    {
      Vec4F32 bg_color = df_rgba_from_theme_color(DF_ThemeColor_BaseBackground);
      dr_rect(os_client_rect_from_window(ws->os), bg_color, 0, 0, 0);
    }
    
    //- rjf: draw window border
    {
      Vec4F32 color = df_rgba_from_theme_color(DF_ThemeColor_BaseBorder);
      dr_rect(os_client_rect_from_window(ws->os), color, 0, 1.f, 0.5f);
    }
    
    //- rjf: recurse & draw
    U64 total_heatmap_sum_count = 0;
    for(UI_Box *box = ui_root_from_state(ws->ui); !ui_box_is_nil(box);)
    {
      // rjf: get recursion
      UI_BoxRec rec = ui_box_rec_df_post(box, &ui_g_nil_box);
      
      // rjf: sum to box heatmap
      if(DEV_draw_ui_box_heatmap)
      {
        Vec2F32 center = center_2f32(box->rect);
        Vec2S32 p = v2s32(center.x / heatmap_bucket_size, center.y / heatmap_bucket_size);
        U64 bucket_idx = p.y * heatmap_bucket_pitch + p.x;
        if(bucket_idx < heatmap_bucket_count)
        {
          heatmap_buckets[bucket_idx] += 1;
          total_heatmap_sum_count += 1;
        }
      }
      
      // rjf: push transparency
      if(box->transparency != 0)
      {
        dr_push_transparency(box->transparency);
      }
      
      // rjf: push squish
      if(box->squish != 0)
      {
        Vec2F32 box_dim = dim_2f32(box->rect);
        Mat3x3F32 box2origin_xform = make_translate_3x3f32(v2f32(-box->rect.x0 - box_dim.x/8, -box->rect.y0));
        Mat3x3F32 scale_xform = make_scale_3x3f32(v2f32(1-box->squish, 1-box->squish));
        Mat3x3F32 origin2box_xform = make_translate_3x3f32(v2f32(box->rect.x0 + box_dim.x/8, box->rect.y0));
        Mat3x3F32 xform = mul_3x3f32(origin2box_xform, mul_3x3f32(scale_xform, box2origin_xform));
        dr_push_xform2d(xform);
        dr_push_tex2d_sample_kind(R_Tex2DSampleKind_Linear);
      }
      
      // rjf: draw drop shadow
      if(box->flags & UI_BoxFlag_DrawDropShadow)
      {
        Rng2F32 drop_shadow_rect = shift_2f32(pad_2f32(box->rect, 8), v2f32(4, 4));
        Vec4F32 drop_shadow_color = df_rgba_from_theme_color(DF_ThemeColor_DropShadow);
        dr_rect(drop_shadow_rect, drop_shadow_color, 0.8f, 0, 8.f);
      }
      
      // rjf: blur background
      if(box->flags & UI_BoxFlag_DrawBackgroundBlur && df_setting_val_from_code(DF_SettingCode_BackgroundBlur).s32)
      {
        R_PassParams_Blur *params = dr_blur(box->rect, box->blur_size*(1-box->transparency), 0);
        MemoryCopyArray(params->corner_radii, box->corner_radii);
      }
      
      // rjf: draw background
      if(box->flags & UI_BoxFlag_DrawBackground)
      {
        // rjf: main rectangle
        {
          R_Rect2DInst *inst = dr_rect(pad_2f32(box->rect, 1), box->palette->colors[UI_ColorCode_Background], 0, 0, 1.f);
          MemoryCopyArray(inst->corner_radii, box->corner_radii);
        }
        
        // rjf: hot effect extension
        if(box->flags & UI_BoxFlag_DrawHotEffects)
        {
          F32 effective_active_t = box->active_t;
          if(!(box->flags & UI_BoxFlag_DrawActiveEffects))
          {
            effective_active_t = 0;
          }
          F32 t = box->hot_t*(1-effective_active_t);
          
          // rjf: brighten
          {
            R_Rect2DInst *inst = dr_rect(box->rect, v4f32(0, 0, 0, 0), 0, 0, 1.f);
            Vec4F32 color = df_rgba_from_theme_color(DF_ThemeColor_Hover);
            color.w *= t*0.2f;
            inst->colors[Corner_00] = color;
            inst->colors[Corner_01] = color;
            inst->colors[Corner_10] = color;
            inst->colors[Corner_11] = color;
            inst->colors[Corner_10].w *= t;
            inst->colors[Corner_11].w *= t;
            MemoryCopyArray(inst->corner_radii, box->corner_radii);
          }
          
          // rjf: slight emboss fadeoff
          if(0)
          {
            Rng2F32 rect = r2f32p(box->rect.x0,
                                  box->rect.y0,
                                  box->rect.x1,
                                  box->rect.y1);
            R_Rect2DInst *inst = dr_rect(rect, v4f32(0, 0, 0, 0), 0, 0, 1.f);
            inst->colors[Corner_00] = v4f32(0.f, 0.f, 0.f, 0.0f*t);
            inst->colors[Corner_01] = v4f32(0.f, 0.f, 0.f, 0.3f*t);
            inst->colors[Corner_10] = v4f32(0.f, 0.f, 0.f, 0.0f*t);
            inst->colors[Corner_11] = v4f32(0.f, 0.f, 0.f, 0.3f*t);
            MemoryCopyArray(inst->corner_radii, box->corner_radii);
          }
        }
        
        // rjf: active effect extension
        if(box->flags & UI_BoxFlag_DrawActiveEffects)
        {
          Vec4F32 shadow_color = df_rgba_from_theme_color(DF_ThemeColor_Hover);
          shadow_color.x *= 0.3f;
          shadow_color.y *= 0.3f;
          shadow_color.z *= 0.3f;
          shadow_color.w *= 0.5f*box->active_t;
          Vec2F32 shadow_size =
          {
            (box->rect.x1 - box->rect.x0)*0.60f*box->active_t,
            (box->rect.y1 - box->rect.y0)*0.60f*box->active_t,
          };
          shadow_size.x = Clamp(0, shadow_size.x, box->font_size*2.f);
          shadow_size.y = Clamp(0, shadow_size.y, box->font_size*2.f);
          
          // rjf: top -> bottom dark effect
          {
            R_Rect2DInst *inst = dr_rect(r2f32p(box->rect.x0, box->rect.y0, box->rect.x1, box->rect.y0 + shadow_size.y), v4f32(0, 0, 0, 0), 0, 0, 1.f);
            inst->colors[Corner_00] = inst->colors[Corner_10] = shadow_color;
            inst->colors[Corner_01] = inst->colors[Corner_11] = v4f32(0.f, 0.f, 0.f, 0.0f);
            MemoryCopyArray(inst->corner_radii, box->corner_radii);
          }
          
          // rjf: bottom -> top light effect
          {
            R_Rect2DInst *inst = dr_rect(r2f32p(box->rect.x0, box->rect.y1 - shadow_size.y, box->rect.x1, box->rect.y1), v4f32(0, 0, 0, 0), 0, 0, 1.f);
            inst->colors[Corner_00] = inst->colors[Corner_10] = v4f32(0, 0, 0, 0);
            inst->colors[Corner_01] = inst->colors[Corner_11] = v4f32(0.4f, 0.4f, 0.4f, 0.4f*box->active_t);
            MemoryCopyArray(inst->corner_radii, box->corner_radii);
          }
          
          // rjf: left -> right dark effect
          {
            R_Rect2DInst *inst = dr_rect(r2f32p(box->rect.x0, box->rect.y0, box->rect.x0 + shadow_size.x, box->rect.y1), v4f32(0, 0, 0, 0), 0, 0, 1.f);
            inst->colors[Corner_10] = inst->colors[Corner_11] = v4f32(0.f, 0.f, 0.f, 0.f);
            inst->colors[Corner_00] = shadow_color;
            inst->colors[Corner_01] = shadow_color;
            MemoryCopyArray(inst->corner_radii, box->corner_radii);
          }
          
          // rjf: right -> left dark effect
          {
            R_Rect2DInst *inst = dr_rect(r2f32p(box->rect.x1 - shadow_size.x, box->rect.y0, box->rect.x1, box->rect.y1), v4f32(0, 0, 0, 0), 0, 0, 1.f);
            inst->colors[Corner_00] = inst->colors[Corner_01] = v4f32(0.f, 0.f, 0.f, 0.f);
            inst->colors[Corner_10] = shadow_color;
            inst->colors[Corner_11] = shadow_color;
            MemoryCopyArray(inst->corner_radii, box->corner_radii);
          }
        }
      }
      
      // rjf: draw string
      if(box->flags & UI_BoxFlag_DrawText)
      {
        Vec2F32 text_position = ui_box_text_position(box);
        if(DEV_draw_ui_text_pos)
        {
          dr_rect(r2f32p(text_position.x-4, text_position.y-4, text_position.x+4, text_position.y+4),
                  v4f32(1, 0, 1, 1), 1, 0, 1);
        }
        F32 max_x = 100000.f;
        FNT_Run ellipses_run = {0};
        if(!(box->flags & UI_BoxFlag_DisableTextTrunc))
        {
          max_x = (box->rect.x1-text_position.x);
          ellipses_run = fnt_push_run_from_string(scratch.arena, box->font, box->font_size, 0, box->tab_size, 0, str8_lit("..."));
        }
        dr_truncated_fancy_run_list(text_position, &box->display_string_runs, max_x, ellipses_run);
        if(box->flags & UI_BoxFlag_HasFuzzyMatchRanges)
        {
          Vec4F32 match_color = df_rgba_from_theme_color(DF_ThemeColor_HighlightOverlay);
          dr_truncated_fancy_run_fuzzy_matches(text_position, &box->display_string_runs, max_x, &box->fuzzy_match_ranges, match_color);
        }
      }
      
      // rjf: draw focus viz
      if(DEV_draw_ui_focus_debug)
      {
        B32 focused = (box->flags & (UI_BoxFlag_FocusHot|UI_BoxFlag_FocusActive) &&
                       box->flags & UI_BoxFlag_Clickable);
        B32 disabled = 0;
        for(UI_Box *p = box; !ui_box_is_nil(p); p = p->parent)
        {
          if(p->flags & (UI_BoxFlag_FocusHotDisabled|UI_BoxFlag_FocusActiveDisabled))
          {
            disabled = 1;
            break;
          }
        }
        if(focused)
        {
          Vec4F32 color = v4f32(0.3f, 0.8f, 0.3f, 1.f);
          if(disabled)
          {
            color = v4f32(0.8f, 0.3f, 0.3f, 1.f);
          }
          dr_rect(r2f32p(box->rect.x0-6, box->rect.y0-6, box->rect.x0+6, box->rect.y0+6), color, 2, 0, 1);
          dr_rect(box->rect, color, 2, 2, 1);
        }
        if(box->flags & (UI_BoxFlag_FocusHot|UI_BoxFlag_FocusActive))
        {
          if(box->flags & (UI_BoxFlag_FocusHotDisabled|UI_BoxFlag_FocusActiveDisabled))
          {
            dr_rect(r2f32p(box->rect.x0-6, box->rect.y0-6, box->rect.x0+6, box->rect.y0+6), v4f32(1, 0, 0, 0.2f), 2, 0, 1);
          }
          else
          {
            dr_rect(r2f32p(box->rect.x0-6, box->rect.y0-6, box->rect.x0+6, box->rect.y0+6), v4f32(0, 1, 0, 0.2f), 2, 0, 1);
          }
        }
      }
      
      // rjf: push clip
      if(box->flags & UI_BoxFlag_Clip)
      {
        Rng2F32 top_clip = dr_top_clip();
        Rng2F32 new_clip = pad_2f32(box->rect, -1);
        if(top_clip.x1 != 0 || top_clip.y1 != 0)
        {
          new_clip = intersect_2f32(new_clip, top_clip);
        }
        dr_push_clip(new_clip);
      }
      
      // rjf: custom draw list
      if(box->flags & UI_BoxFlag_DrawBucket)
      {
        Mat3x3F32 xform = make_translate_3x3f32(box->position_delta);
        DR_XForm2DScope(xform)
        {
          dr_sub_bucket(box->draw_bucket);
        }
      }
      
      // rjf: call custom draw callback
      if(box->custom_draw != 0)
      {
        box->custom_draw(box, box->custom_draw_user_data);
      }
      
      // rjf: pop
      {
        S32 pop_idx = 0;
        for(UI_Box *b = box; !ui_box_is_nil(b) && pop_idx <= rec.pop_count; b = b->parent)
        {
          pop_idx += 1;
          if(b == box && rec.push_count != 0)
          {
            continue;
          }
          
          // rjf: pop clips
          if(b->flags & UI_BoxFlag_Clip)
          {
            dr_pop_clip();
          }
          
          // rjf: draw border
          if(b->flags & UI_BoxFlag_DrawBorder)
          {
            R_Rect2DInst *inst = dr_rect(pad_2f32(b->rect, 1), b->palette->colors[UI_ColorCode_Border], 0, 1.f, 1.f);
            MemoryCopyArray(inst->corner_radii, b->corner_radii);
            
            // rjf: hover effect
            if(b->flags & UI_BoxFlag_DrawHotEffects)
            {
              Vec4F32 color = df_rgba_from_theme_color(DF_ThemeColor_Hover);
              color.w *= b->hot_t;
              R_Rect2DInst *inst = dr_rect(pad_2f32(b->rect, 1), color, 0, 1.f, 1.f);
              MemoryCopyArray(inst->corner_radii, b->corner_radii);
            }
          }
          
          // rjf: debug border rendering
          if(0)
          {
            R_Rect2DInst *inst = dr_rect(pad_2f32(b->rect, 1), v4f32(1, 0, 1, 0.25f), 0, 1.f, 1.f);
            MemoryCopyArray(inst->corner_radii, b->corner_radii);
          }
          
          // rjf: draw sides
          {
            Rng2F32 r = b->rect;
            F32 half_thickness = 1.f;
            F32 softness = 0.5f;
            if(b->flags & UI_BoxFlag_DrawSideTop)
            {
              dr_rect(r2f32p(r.x0, r.y0-half_thickness, r.x1, r.y0+half_thickness), b->palette->colors[UI_ColorCode_Border], 0, 0, softness);
            }
            if(b->flags & UI_BoxFlag_DrawSideBottom)
            {
              dr_rect(r2f32p(r.x0, r.y1-half_thickness, r.x1, r.y1+half_thickness), b->palette->colors[UI_ColorCode_Border], 0, 0, softness);
            }
            if(b->flags & UI_BoxFlag_DrawSideLeft)
            {
              dr_rect(r2f32p(r.x0-half_thickness, r.y0, r.x0+half_thickness, r.y1), b->palette->colors[UI_ColorCode_Border], 0, 0, softness);
            }
            if(b->flags & UI_BoxFlag_DrawSideRight)
            {
              dr_rect(r2f32p(r.x1-half_thickness, r.y0, r.x1+half_thickness, r.y1), b->palette->colors[UI_ColorCode_Border], 0, 0, softness);
            }
          }
          
          // rjf: draw focus overlay
          if(b->flags & UI_BoxFlag_Clickable && !(b->flags & UI_BoxFlag_DisableFocusOverlay) && b->focus_hot_t > 0.01f)
          {
            Vec4F32 color = df_rgba_from_theme_color(DF_ThemeColor_Focus);
            color.w *= 0.2f*b->focus_hot_t;
            R_Rect2DInst *inst = dr_rect(b->rect, color, 0, 0, 1.f);
            MemoryCopyArray(inst->corner_radii, b->corner_radii);
          }
          
          // rjf: draw focus border
          if(b->flags & UI_BoxFlag_Clickable && !(b->flags & UI_BoxFlag_DisableFocusBorder) && b->focus_active_t > 0.01f)
          {
            Vec4F32 color = df_rgba_from_theme_color(DF_ThemeColor_Focus);
            color.w *= b->focus_active_t;
            R_Rect2DInst *inst = dr_rect(pad_2f32(b->rect, 0.f), color, 0, 1.f, 1.f);
            MemoryCopyArray(inst->corner_radii, b->corner_radii);
          }
          
          // rjf: disabled overlay
          if(b->disabled_t >= 0.005f)
          {
            Vec4F32 color = df_rgba_from_theme_color(DF_ThemeColor_DisabledOverlay);
            color.w *= b->disabled_t;
            R_Rect2DInst *inst = dr_rect(b->rect, color, 0, 0, 1);
            MemoryCopyArray(inst->corner_radii, b->corner_radii);
          }
          
          // rjf: pop squish
          if(b->squish != 0)
          {
            dr_pop_xform2d();
            dr_pop_tex2d_sample_kind();
          }
          
          // rjf: pop transparency
          if(b->transparency != 0)
          {
            dr_pop_transparency();
          }
        }
      }
      
      // rjf: next
      box = rec.next;
    }
    
    //- rjf: draw heatmap
    if(DEV_draw_ui_box_heatmap)
    {
      U64 uniform_dist_count = total_heatmap_sum_count / heatmap_bucket_count;
      uniform_dist_count = ClampBot(uniform_dist_count, 10);
      for(U64 bucket_idx = 0; bucket_idx < heatmap_bucket_count; bucket_idx += 1)
      {
        U64 x = bucket_idx % heatmap_bucket_pitch;
        U64 y = bucket_idx / heatmap_bucket_pitch;
        U64 bucket = heatmap_buckets[bucket_idx];
        F32 pct = (F32)bucket / uniform_dist_count;
        pct = Clamp(0, pct, 1);
        Vec3F32 hsv = v3f32((1-pct) * 0.9411f, 1, 0.5f);
        Vec3F32 rgb = rgb_from_hsv(hsv);
        Rng2F32 rect = r2f32p(x*heatmap_bucket_size, y*heatmap_bucket_size, (x+1)*heatmap_bucket_size, (y+1)*heatmap_bucket_size);
        dr_rect(rect, v4f32(rgb.x, rgb.y, rgb.z, 0.3f), 0, 0, 0);
      }
    }
    
    //- rjf: draw border/overlay color to signify error
    if(ws->error_t > 0.01f)
    {
      Vec4F32 color = df_rgba_from_theme_color(DF_ThemeColor_NegativePopButtonBackground);
      color.w *= ws->error_t;
      Rng2F32 rect = os_client_rect_from_window(ws->os);
      dr_rect(pad_2f32(rect, 24.f), color, 0, 16.f, 12.f);
      dr_rect(rect, v4f32(color.x, color.y, color.z, color.w*0.05f), 0, 0, 0);
    }
    
    //- rjf: scratch debug mouse drawing
    if(DEV_scratch_mouse_draw)
    {
#if 1
      Vec2F32 p = add_2f32(os_mouse_from_window(ws->os), v2f32(30, 0));
      dr_rect(os_client_rect_from_window(ws->os), v4f32(0, 0, 0, 0.9f), 0, 0, 0);
      FNT_Run trailer_run = fnt_push_run_from_string(scratch.arena, df_font_from_slot(DF_FontSlot_Main), 16.f, 0, 0, 0, str8_lit("..."));
      DR_FancyStringList strs = {0};
      DR_FancyString str = {df_font_from_slot(DF_FontSlot_Main), str8_lit("Shift + F5"), v4f32(1, 1, 1, 1), 72.f, 0.f};
      dr_fancy_string_list_push(scratch.arena, &strs, &str);
      DR_FancyRunList runs = dr_fancy_run_list_from_fancy_string_list(scratch.arena, 0, FNT_RasterFlag_Smooth, &strs);
      dr_truncated_fancy_run_list(p, &runs, 1000000.f, trailer_run);
      dr_rect(r2f32(p, add_2f32(p, runs.dim)), v4f32(1, 0, 0, 0.5f), 0, 1, 0);
      dr_rect(r2f32(sub_2f32(p, v2f32(4, 4)), add_2f32(p, v2f32(4, 4))), v4f32(1, 0, 1, 1), 0, 0, 0);
#else
      Vec2F32 p = add_2f32(os_mouse_from_window(ws->os), v2f32(30, 0));
      dr_rect(os_client_rect_from_window(ws->os), v4f32(0, 0, 0, 0.4f), 0, 0, 0);
      DR_FancyStringList strs = {0};
      DR_FancyString str1 = {df_font_from_slot(DF_FontSlot_Main), str8_lit("T"), v4f32(1, 1, 1, 1), 16.f, 4.f};
      dr_fancy_string_list_push(scratch.arena, &strs, &str1);
      DR_FancyString str2 = {df_font_from_slot(DF_FontSlot_Main), str8_lit("his is a test of some "), v4f32(1, 0.5f, 0.5f, 1), 14.f, 0.f};
      dr_fancy_string_list_push(scratch.arena, &strs, &str2);
      DR_FancyString str3 = {df_font_from_slot(DF_FontSlot_Code), str8_lit("very fancy text!"), v4f32(1, 0.8f, 0.4f, 1), 18.f, 4.f, 4.f};
      dr_fancy_string_list_push(scratch.arena, &strs, &str3);
      DR_FancyRunList runs = dr_fancy_run_list_from_fancy_string_list(scratch.arena, 0, 0, &strs);
      FNT_Run trailer_run = fnt_push_run_from_string(scratch.arena, df_font_from_slot(DF_FontSlot_Main), 16.f, 0, 0, 0, str8_lit("..."));
      F32 limit = 500.f + sin_f32(df_state->time_in_seconds/10.f)*200.f;
      dr_truncated_fancy_run_list(p, &runs, limit, trailer_run);
      dr_rect(r2f32p(p.x+limit, 0, p.x+limit+2.f, 1000), v4f32(1, 0, 0, 1), 0, 0, 0);
      df_request_frame();
#endif
    }
    
    scratch_end(scratch);
  }
  
  //////////////////////////////
  //- rjf: increment per-window frame counter
  //
  ws->frames_alive += 1;
  
  ProfEnd();
}

#if COMPILER_MSVC && !BUILD_DEBUG
#pragma optimize("", on)
#endif

////////////////////////////////
//~ rjf: Eval Visualization

internal EV_View *
df_ev_view_from_key(U64 key)
{
  U64 slot_idx = key % df_state->eval_viz_view_cache_slots_count;
  DF_EvalVizViewCacheNode *node = 0;
  DF_EvalVizViewCacheSlot *slot = &df_state->eval_viz_view_cache_slots[slot_idx];
  for(DF_EvalVizViewCacheNode *n = slot->first; n != 0; n = n->next)
  {
    if(n->key == key)
    {
      node = n;
      break;
    }
  }
  if(node == 0)
  {
    node = df_state->eval_viz_view_cache_node_free;
    if(node)
    {
      SLLStackPop(df_state->eval_viz_view_cache_node_free);
    }
    else
    {
      node = push_array(df_state->arena, DF_EvalVizViewCacheNode, 1);
    }
    DLLPushBack(slot->first, slot->last, node);
    node->key = key;
    node->v = ev_view_alloc();
  }
  return node->v;
}

internal F32
df_append_value_strings_from_eval(Arena *arena, EV_StringFlags flags, U32 default_radix, FNT_Tag font, F32 font_size, F32 max_size, S32 depth, E_Eval eval, E_Member *member, EV_ViewRuleList *view_rules, String8List *out)
{
  ProfBeginFunction();
  Temp scratch = scratch_begin(&arena, 1);
  F32 space_taken = 0;
  
  //- rjf: unpack view rules
  U32 radix = default_radix;
  B32 no_addr = 0;
  B32 has_array = 0;
  for(EV_ViewRuleNode *n = view_rules->first; n != 0; n = n->next)
  {
    if(0){}
    else if(str8_match(n->v.root->string, str8_lit("dec"), 0)) {radix = 10;}
    else if(str8_match(n->v.root->string, str8_lit("hex"), 0)) {radix = 16;}
    else if(str8_match(n->v.root->string, str8_lit("bin"), 0)) {radix = 2; }
    else if(str8_match(n->v.root->string, str8_lit("oct"), 0)) {radix = 8; }
    else if(str8_match(n->v.root->string, str8_lit("no_addr"), 0)) {no_addr = 1;}
    else if(str8_match(n->v.root->string, str8_lit("array"), 0)) {has_array = 1;}
  }
  
  //- rjf: member evaluations -> display member info
  if(eval.mode == E_Mode_Null && !e_type_key_match(e_type_key_zero(), eval.type_key) && member != 0)
  {
    U64 member_byte_size = e_type_byte_size_from_key(eval.type_key);
    String8 offset_string = str8_from_u64(arena, member->off, radix, 0, 0);
    String8 size_string = str8_from_u64(arena, member_byte_size, radix, 0, 0);
    str8_list_pushf(arena, out, "member (%S offset, %S byte%s)", offset_string, size_string, member_byte_size == 1 ? "" : "s");
  }
  
  //- rjf: type evaluations -> display type basic information
  else if(eval.mode == E_Mode_Null && !e_type_key_match(e_type_key_zero(), eval.type_key) && eval.expr->kind != E_ExprKind_MemberAccess)
  {
    String8 basic_type_kind_string = e_kind_basic_string_table[e_type_kind_from_key(eval.type_key)];
    U64 byte_size = e_type_byte_size_from_key(eval.type_key);
    String8 size_string = str8_from_u64(arena, byte_size, radix, 0, 0);
    str8_list_pushf(arena, out, "%S (%S byte%s)", basic_type_kind_string, size_string, byte_size == 1 ? "" : "s");
  }
  
  //- rjf: value/offset evaluations
  else if(max_size > 0) switch(e_type_kind_from_key(e_type_unwrap(eval.type_key)))
  {
    //- rjf: default - leaf cases
    default:
    {
      E_Eval value_eval = e_value_eval_from_eval(eval);
      String8 string = ev_string_from_simple_typed_eval(arena, flags, radix, value_eval);
      space_taken += fnt_dim_from_tag_size_string(font, font_size, 0, 0, string).x;
      str8_list_push(arena, out, string);
    }break;
    
    //- rjf: pointers
    case E_TypeKind_Function:
    case E_TypeKind_Ptr:
    case E_TypeKind_LRef:
    case E_TypeKind_RRef:
    {
      // rjf: unpack type info
      E_TypeKind type_kind = e_type_kind_from_key(e_type_unwrap(eval.type_key));
      E_TypeKey direct_type_key = e_type_unwrap(e_type_ptee_from_key(e_type_unwrap(eval.type_key)));
      E_TypeKind direct_type_kind = e_type_kind_from_key(direct_type_key);
      
      // rjf: unpack info about pointer destination
      E_Eval value_eval = e_value_eval_from_eval(eval);
      B32 ptee_has_content = (direct_type_kind != E_TypeKind_Null && direct_type_kind != E_TypeKind_Void);
      B32 ptee_has_string  = (E_TypeKind_Char8 <= direct_type_kind && direct_type_kind <= E_TypeKind_UChar32);
      CTRL_Entity *thread = ctrl_entity_from_handle(d_state->ctrl_entity_store, df_regs()->thread);
      CTRL_Entity *process = ctrl_entity_ancestor_from_kind(thread, CTRL_EntityKind_Process);
      String8 symbol_name = d_symbol_name_from_process_vaddr(arena, process, value_eval.value.u64, 1);
      
      // rjf: special case: push strings for textual string content
      B32 did_content = 0;
      B32 did_string = 0;
      if(!did_content && ptee_has_string && !has_array)
      {
        did_content = 1;
        did_string = 1;
        U64 string_memory_addr = value_eval.value.u64;
        U64 element_size = e_type_byte_size_from_key(direct_type_key);
        U64 string_buffer_size = 1024;
        U8 *string_buffer = push_array(arena, U8, string_buffer_size);
        for(U64 try_size = string_buffer_size; try_size >= 16; try_size /= 2)
        {
          B32 read_good = e_space_read(eval.space, string_buffer, r1u64(string_memory_addr, string_memory_addr+try_size));
          if(read_good)
          {
            break;
          }
        }
        string_buffer[string_buffer_size-1] = 0;
        String8 string = {0};
        switch(element_size)
        {
          default:{string = str8_cstring((char *)string_buffer);}break;
          case 2: {string = str8_from_16(arena, str16_cstring((U16 *)string_buffer));}break;
          case 4: {string = str8_from_32(arena, str32_cstring((U32 *)string_buffer));}break;
        }
        String8 string_escaped = ev_escaped_from_raw_string(arena, string);
        space_taken += fnt_dim_from_tag_size_string(font, font_size, 0, 0, string_escaped).x;
        space_taken += 2*fnt_dim_from_tag_size_string(font, font_size, 0, 0, str8_lit("\"")).x;
        str8_list_push(arena, out, str8_lit("\""));
        str8_list_push(arena, out, string_escaped);
        str8_list_push(arena, out, str8_lit("\""));
      }
      
      // rjf: special case: push strings for symbols
      if(!did_content && symbol_name.size != 0 &&
         ((type_kind == E_TypeKind_Ptr && direct_type_kind == E_TypeKind_Void) ||
          (type_kind == E_TypeKind_Ptr && direct_type_kind == E_TypeKind_Function) ||
          (type_kind == E_TypeKind_Function)))
      {
        did_content = 1;
        str8_list_push(arena, out, symbol_name);
        space_taken += fnt_dim_from_tag_size_string(font, font_size, 0, 0, symbol_name).x;
      }
      
      // rjf: special case: need symbol name, don't have one
      if(!did_content && symbol_name.size == 0 &&
         ((type_kind == E_TypeKind_Ptr && direct_type_kind == E_TypeKind_Function) ||
          (type_kind == E_TypeKind_Function)) &&
         (flags & EV_StringFlag_ReadOnlyDisplayRules))
      {
        did_content = 1;
        String8 string = str8_lit("???");
        str8_list_push(arena, out, string);
        space_taken += fnt_dim_from_tag_size_string(font, font_size, 0, 0, string).x;
      }
      
      // rjf: descend for all other cases
      if(!did_content && ptee_has_content && (flags & EV_StringFlag_ReadOnlyDisplayRules))
      {
        did_content = 1;
        if(depth < 4)
        {
          E_Expr *deref_expr = e_expr_ref_deref(scratch.arena, eval.expr);
          E_Eval deref_eval = e_eval_from_expr(scratch.arena, deref_expr);
          space_taken += df_append_value_strings_from_eval(arena, flags, radix, font, font_size, max_size-space_taken, depth+1, deref_eval, 0, view_rules, out);
        }
        else
        {
          String8 ellipses = str8_lit("...");
          str8_list_push(arena, out, ellipses);
          space_taken += fnt_dim_from_tag_size_string(font, font_size, 0, 0, ellipses).x;
        }
      }
      
      // rjf: push pointer value
      B32 did_ptr_value = 0;
      if((!no_addr || !did_content) && ((flags & EV_StringFlag_ReadOnlyDisplayRules) || !did_string))
      {
        did_ptr_value = 1;
        if(did_content)
        {
          String8 ptr_prefix = str8_lit(" (");
          space_taken += fnt_dim_from_tag_size_string(font, font_size, 0, 0, ptr_prefix).x;
          str8_list_push(arena, out, ptr_prefix);
        }
        String8 string = ev_string_from_simple_typed_eval(arena, flags, radix, value_eval);
        space_taken += fnt_dim_from_tag_size_string(font, font_size, 0, 0, string).x;
        str8_list_push(arena, out, string);
        if(did_content)
        {
          String8 close = str8_lit(")");
          space_taken += fnt_dim_from_tag_size_string(font, font_size, 0, 0, close).x;
          str8_list_push(arena, out, close);
        }
      }
    }break;
    
    //- rjf: arrays
    case E_TypeKind_Array:
    {
      // rjf: unpack type info
      E_Type *eval_type = e_type_from_key(scratch.arena, e_type_unwrap(eval.type_key));
      E_TypeKey direct_type_key = eval_type->direct_type_key;
      E_TypeKind direct_type_kind = e_type_kind_from_key(direct_type_key);
      U64 array_count = eval_type->count;
      
      // rjf: get pointed-at type
      B32 array_is_string = ((E_TypeKind_Char8 <= direct_type_kind && direct_type_kind <= E_TypeKind_UChar32) ||
                             direct_type_kind == E_TypeKind_S8 ||
                             direct_type_kind == E_TypeKind_U8);
      
      // rjf: special case: push strings for textual string content
      B32 did_content = 0;
      if(!did_content && array_is_string && !has_array && (member == 0 || member->kind != E_MemberKind_Padding))
      {
        U64 element_size = e_type_byte_size_from_key(direct_type_key);
        did_content = 1;
        U64 string_buffer_size = 1024;
        U8 *string_buffer = push_array(arena, U8, string_buffer_size);
        switch(eval.mode)
        {
          default:{}break;
          case E_Mode_Offset:
          {
            U64 string_memory_addr = eval.value.u64;
            for(U64 try_size = string_buffer_size; try_size >= 16; try_size /= 2)
            {
              B32 read_good = e_space_read(eval.space, string_buffer, r1u64(string_memory_addr, string_memory_addr+try_size));
              if(read_good)
              {
                break;
              }
            }
          }break;
          case E_Mode_Value:
          {
            MemoryCopy(string_buffer, &eval.value.u512[0], Min(string_buffer_size, sizeof(eval.value)));
          }break;
        }
        string_buffer[string_buffer_size-1] = 0;
        String8 string = {0};
        switch(element_size)
        {
          default:{string = str8_cstring((char *)string_buffer);}break;
          case 2: {string = str8_from_16(arena, str16_cstring((U16 *)string_buffer));}break;
          case 4: {string = str8_from_32(arena, str32_cstring((U32 *)string_buffer));}break;
        }
        String8 string_escaped = ev_escaped_from_raw_string(arena, string);
        space_taken += fnt_dim_from_tag_size_string(font, font_size, 0, 0, string_escaped).x;
        space_taken += 2*fnt_dim_from_tag_size_string(font, font_size, 0, 0, str8_lit("\"")).x;
        str8_list_push(arena, out, str8_lit("\""));
        str8_list_push(arena, out, string_escaped);
        str8_list_push(arena, out, str8_lit("\""));
      }
      
      // rjf: descend in all other cases
      if(!did_content && (flags & EV_StringFlag_ReadOnlyDisplayRules))
      {
        did_content = 1;
        
        // rjf: [
        {
          String8 bracket = str8_lit("[");
          str8_list_push(arena, out, bracket);
          space_taken += fnt_dim_from_tag_size_string(font, font_size, 0, 0, bracket).x;
        }
        
        // rjf: build contents
        if(depth < 4)
        {
          for(U64 idx = 0; idx < array_count && max_size > space_taken; idx += 1)
          {
            E_Expr *element_expr = e_expr_ref_array_index(scratch.arena, eval.expr, idx);
            E_Eval element_eval = e_eval_from_expr(scratch.arena, element_expr);
            space_taken += df_append_value_strings_from_eval(arena, flags, radix, font, font_size, max_size-space_taken, depth+1, element_eval, 0, view_rules, out);
            if(idx+1 < array_count)
            {
              String8 comma = str8_lit(", ");
              space_taken += fnt_dim_from_tag_size_string(font, font_size, 0, 0, comma).x;
              str8_list_push(arena, out, comma);
            }
            if(space_taken > max_size && idx+1 < array_count)
            {
              String8 ellipses = str8_lit("...");
              space_taken += fnt_dim_from_tag_size_string(font, font_size, 0, 0, ellipses).x;
              str8_list_push(arena, out, ellipses);
            }
          }
        }
        else
        {
          String8 ellipses = str8_lit("...");
          str8_list_push(arena, out, ellipses);
          space_taken += fnt_dim_from_tag_size_string(font, font_size, 0, 0, ellipses).x;
        }
        
        // rjf: ]
        {
          String8 bracket = str8_lit("]");
          str8_list_push(arena, out, bracket);
          space_taken += fnt_dim_from_tag_size_string(font, font_size, 0, 0, bracket).x;
        }
      }
    }break;
    
    //- rjf: structs
    case E_TypeKind_Struct:
    case E_TypeKind_Union:
    case E_TypeKind_Class:
    case E_TypeKind_IncompleteStruct:
    case E_TypeKind_IncompleteUnion:
    case E_TypeKind_IncompleteClass:
    {
      // rjf: open brace
      {
        String8 brace = str8_lit("{");
        str8_list_push(arena, out, brace);
        space_taken += fnt_dim_from_tag_size_string(font, font_size, 0, 0, brace).x;
      }
      
      // rjf: content
      if(depth < 4)
      {
        E_MemberArray data_members = e_type_data_members_from_key(scratch.arena, e_type_unwrap(eval.type_key));
        for(U64 member_idx = 0; member_idx < data_members.count && max_size > space_taken; member_idx += 1)
        {
          E_Member *mem = &data_members.v[member_idx];
          E_Expr *dot_expr = e_expr_ref_member_access(scratch.arena, eval.expr, mem->name);
          E_Eval dot_eval = e_eval_from_expr(scratch.arena, dot_expr);
          space_taken += df_append_value_strings_from_eval(arena, flags, radix, font, font_size, max_size-space_taken, depth+1, dot_eval, 0, view_rules, out);
          if(member_idx+1 < data_members.count)
          {
            String8 comma = str8_lit(", ");
            space_taken += fnt_dim_from_tag_size_string(font, font_size, 0, 0, comma).x;
            str8_list_push(arena, out, comma);
          }
          if(space_taken > max_size && member_idx+1 < data_members.count)
          {
            String8 ellipses = str8_lit("...");
            space_taken += fnt_dim_from_tag_size_string(font, font_size, 0, 0, ellipses).x;
            str8_list_push(arena, out, ellipses);
          }
        }
      }
      else
      {
        String8 ellipses = str8_lit("...");
        str8_list_push(arena, out, ellipses);
        space_taken += fnt_dim_from_tag_size_string(font, font_size, 0, 0, ellipses).x;
      }
      
      // rjf: close brace
      {
        String8 brace = str8_lit("}");
        str8_list_push(arena, out, brace);
        space_taken += fnt_dim_from_tag_size_string(font, font_size, 0, 0, brace).x;
      }
    }break;
  }
  
  scratch_end(scratch);
  ProfEnd();
  return space_taken;
}

internal String8
df_value_string_from_eval(Arena *arena, EV_StringFlags flags, U32 default_radix, FNT_Tag font, F32 font_size, F32 max_size, E_Eval eval, E_Member *member, EV_ViewRuleList *view_rules)
{
  Temp scratch = scratch_begin(&arena, 1);
  String8List strs = {0};
  df_append_value_strings_from_eval(scratch.arena, flags, default_radix, font, font_size, max_size, 0, eval, member, view_rules, &strs);
  String8 result = str8_list_join(arena, &strs, 0);
  scratch_end(scratch);
  return result;
}

////////////////////////////////
//~ rjf: Hover Eval

internal void
df_set_hover_eval(Vec2F32 pos, String8 file_path, TxtPt pt, U64 vaddr, String8 string)
{
  DF_Window *window = df_window_from_handle(df_regs()->window);
  if(window->hover_eval_last_frame_idx+1 < df_state->frame_index &&
     ui_key_match(ui_active_key(UI_MouseButtonKind_Left), ui_key_zero()) &&
     ui_key_match(ui_active_key(UI_MouseButtonKind_Middle), ui_key_zero()) &&
     ui_key_match(ui_active_key(UI_MouseButtonKind_Right), ui_key_zero()))
  {
    B32 is_new_string = !str8_match(window->hover_eval_string, string, 0);
    if(is_new_string)
    {
      window->hover_eval_first_frame_idx = window->hover_eval_last_frame_idx = df_state->frame_index;
      arena_clear(window->hover_eval_arena);
      window->hover_eval_string = push_str8_copy(window->hover_eval_arena, string);
      window->hover_eval_file_path = push_str8_copy(window->hover_eval_arena, file_path);
      window->hover_eval_file_pt = pt;
      window->hover_eval_vaddr = vaddr;
      window->hover_eval_focused = 0;
    }
    window->hover_eval_spawn_pos = pos;
    window->hover_eval_last_frame_idx = df_state->frame_index;
  }
}

////////////////////////////////
//~ rjf: Auto-Complete Lister

internal void
df_autocomp_lister_item_chunk_list_push(Arena *arena, DF_AutoCompListerItemChunkList *list, U64 cap, DF_AutoCompListerItem *item)
{
  DF_AutoCompListerItemChunkNode *n = list->last;
  if(n == 0 || n->count >= n->cap)
  {
    n = push_array(arena, DF_AutoCompListerItemChunkNode, 1);
    SLLQueuePush(list->first, list->last, n);
    n->cap = cap;
    n->v = push_array_no_zero(arena, DF_AutoCompListerItem, n->cap);
    list->chunk_count += 1;
  }
  MemoryCopyStruct(&n->v[n->count], item);
  n->count += 1;
  list->total_count += 1;
}

internal DF_AutoCompListerItemArray
df_autocomp_lister_item_array_from_chunk_list(Arena *arena, DF_AutoCompListerItemChunkList *list)
{
  DF_AutoCompListerItemArray array = {0};
  array.count = list->total_count;
  array.v = push_array_no_zero(arena, DF_AutoCompListerItem, array.count);
  U64 idx = 0;
  for(DF_AutoCompListerItemChunkNode *n = list->first; n != 0; n = n->next)
  {
    MemoryCopy(array.v+idx, n->v, sizeof(DF_AutoCompListerItem)*n->count);
    idx += n->count;
  }
  return array;
}

internal int
df_autocomp_lister_item_qsort_compare(DF_AutoCompListerItem *a, DF_AutoCompListerItem *b)
{
  int result = 0;
  if(a->matches.count > b->matches.count)
  {
    result = -1;
  }
  else if(a->matches.count < b->matches.count)
  {
    result = +1;
  }
  else
  {
    result = strncmp((char *)a->string.str, (char *)b->string.str, Min(a->string.size, b->string.size));
  }
  return result;
}

internal void
df_autocomp_lister_item_array_sort__in_place(DF_AutoCompListerItemArray *array)
{
  quick_sort(array->v, array->count, sizeof(array->v[0]), df_autocomp_lister_item_qsort_compare);
}

internal String8
df_autocomp_query_word_from_input_string_off(String8 input, U64 cursor_off)
{
  U64 word_start_off = 0;
  for(U64 off = 0; off < input.size && off < cursor_off; off += 1)
  {
    if(!char_is_alpha(input.str[off]) && !char_is_digit(input.str[off], 10) && input.str[off] != '_')
    {
      word_start_off = off+1;
    }
  }
  String8 query = str8_skip(str8_prefix(input, cursor_off), word_start_off);
  return query;
}

internal DF_AutoCompListerParams
df_view_rule_autocomp_lister_params_from_input_cursor(Arena *arena, String8 string, U64 cursor_off)
{
  DF_AutoCompListerParams params = {0};
  {
    Temp scratch = scratch_begin(&arena, 1);
    
    //- rjf: do partial parse of input
    MD_TokenizeResult input_tokenize = md_tokenize_from_text(scratch.arena, string);
    
    //- rjf: find descension steps to cursor
    typedef struct DescendStep DescendStep;
    struct DescendStep
    {
      DescendStep *next;
      DescendStep *prev;
      String8 string;
    };
    DescendStep *first_step = 0;
    DescendStep *last_step = 0;
    DescendStep *free_step = 0;
    S32 paren_nest = 0;
    S32 colon_nest = 0;
    String8 last_step_string = {0};
    for(U64 idx = 0; idx < input_tokenize.tokens.count; idx += 1)
    {
      MD_Token *token = &input_tokenize.tokens.v[idx];
      if(token->range.min >= cursor_off)
      {
        break;
      }
      String8 token_string = str8_substr(string, token->range);
      if(token->flags & (MD_TokenFlag_Identifier|MD_TokenFlag_StringLiteral))
      {
        last_step_string = token_string;
      }
      if(str8_match(token_string, str8_lit("("), 0) || str8_match(token_string, str8_lit("["), 0) || str8_match(token_string, str8_lit("{"), 0))
      {
        paren_nest += 1;
      }
      if(str8_match(token_string, str8_lit(")"), 0) || str8_match(token_string, str8_lit("]"), 0) || str8_match(token_string, str8_lit("}"), 0))
      {
        paren_nest -= 1;
        for(;colon_nest > paren_nest; colon_nest -= 1)
        {
          if(last_step != 0)
          {
            DescendStep *step = last_step;
            DLLRemove(first_step, last_step, step);
            SLLStackPush(free_step, step);
          }
        }
        if(paren_nest == 0 && last_step != 0)
        {
          DescendStep *step = last_step;
          DLLRemove(first_step, last_step, step);
          SLLStackPush(free_step, step);
        }
      }
      if(str8_match(token_string, str8_lit(":"), 0))
      {
        colon_nest += 1;
        if(last_step_string.size != 0)
        {
          DescendStep *step = free_step;
          if(step != 0)
          {
            SLLStackPop(free_step);
            MemoryZeroStruct(step);
          }
          else
          {
            step = push_array(scratch.arena, DescendStep, 1);
          }
          step->string = last_step_string;
          DLLPushBack(first_step, last_step, step);
        }
      }
      if(str8_match(token_string, str8_lit(";"), 0) || str8_match(token_string, str8_lit(","), 0))
      {
        for(;colon_nest > paren_nest; colon_nest -= 1)
        {
          if(last_step != 0)
          {
            DescendStep *step = last_step;
            DLLRemove(first_step, last_step, step);
            SLLStackPush(free_step, step);
          }
        }
      }
    }
    
    //- rjf: map view rule root to spec
    D_ViewRuleSpec *spec = d_view_rule_spec_from_string(first_step ? first_step->string : str8_zero());
    
    //- rjf: do parse of schema
    MD_TokenizeResult schema_tokenize = md_tokenize_from_text(scratch.arena, spec->info.schema);
    MD_ParseResult schema_parse = md_parse_from_text_tokens(scratch.arena, str8_zero(), spec->info.schema, schema_tokenize.tokens);
    MD_Node *schema_rule_root = md_child_from_string(schema_parse.root, str8_lit("x"), 0);
    
    //- rjf: follow schema according to descend steps, gather flags from schema node matching cursor descension steps
    if(first_step != 0)
    {
      MD_Node *schema_node = schema_rule_root;
      for(DescendStep *step = first_step->next;;)
      {
        if(step == 0)
        {
          for(MD_EachNode(child, schema_node->first))
          {
            if(0){}
            else if(str8_match(child->string, str8_lit("expr"),           StringMatchFlag_CaseInsensitive)) {params.flags |= DF_AutoCompListerFlag_Locals;}
            else if(str8_match(child->string, str8_lit("member"),         StringMatchFlag_CaseInsensitive)) {params.flags |= DF_AutoCompListerFlag_Members;}
            else if(str8_match(child->string, str8_lit("lang"),           StringMatchFlag_CaseInsensitive)) {params.flags |= DF_AutoCompListerFlag_Languages;}
            else if(str8_match(child->string, str8_lit("arch"),           StringMatchFlag_CaseInsensitive)) {params.flags |= DF_AutoCompListerFlag_Architectures;}
            else if(str8_match(child->string, str8_lit("tex2dformat"),    StringMatchFlag_CaseInsensitive)) {params.flags |= DF_AutoCompListerFlag_Tex2DFormats;}
            else if(child->flags & (MD_NodeFlag_StringSingleQuote|MD_NodeFlag_StringDoubleQuote|MD_NodeFlag_StringTick))
            {
              str8_list_push(arena, &params.strings, child->string);
              params.flags |= DF_AutoCompListerFlag_ViewRuleParams;
            }
          }
          break;
        }
        if(step != 0)
        {
          MD_Node *next_node = md_child_from_string(schema_node, step->string, StringMatchFlag_CaseInsensitive);
          schema_node = next_node;
          step = step->next;
        }
        else
        {
          schema_node = schema_node->first;
        }
      }
    }
    
    scratch_end(scratch);
  }
  return params;
}

internal void
df_set_autocomp_lister_query(UI_Key root_key, DF_AutoCompListerParams *params, String8 input, U64 cursor_off)
{
  DF_Window *window = df_window_from_handle(df_regs()->window);
  String8 query = df_autocomp_query_word_from_input_string_off(input, cursor_off);
  String8 current_query = str8(window->autocomp_lister_query_buffer, window->autocomp_lister_query_size);
  if(cursor_off != window->autocomp_cursor_off)
  {
    window->autocomp_query_dirty = 1;
    window->autocomp_cursor_off = cursor_off;
  }
  if(!str8_match(query, current_query, 0))
  {
    window->autocomp_force_closed = 0;
  }
  if(!ui_key_match(window->autocomp_root_key, root_key))
  {
    window->autocomp_force_closed = 0;
    window->autocomp_num_visible_rows_t = 0;
    window->autocomp_open_t = 0;
  }
  if(window->autocomp_last_frame_idx+1 < df_state->frame_index)
  {
    window->autocomp_force_closed = 0;
    window->autocomp_num_visible_rows_t = 0;
    window->autocomp_open_t = 0;
  }
  window->autocomp_root_key = root_key;
  arena_clear(window->autocomp_lister_params_arena);
  MemoryCopyStruct(&window->autocomp_lister_params, params);
  window->autocomp_lister_params.strings = str8_list_copy(window->autocomp_lister_params_arena, &window->autocomp_lister_params.strings);
  window->autocomp_lister_query_size = Min(query.size, sizeof(window->autocomp_lister_query_buffer));
  MemoryCopy(window->autocomp_lister_query_buffer, query.str, window->autocomp_lister_query_size);
  window->autocomp_last_frame_idx = df_state->frame_index;
}

////////////////////////////////
//~ rjf: Search Strings

internal void
df_set_search_string(String8 string)
{
  arena_clear(df_state->string_search_arena);
  df_state->string_search_string = push_str8_copy(df_state->string_search_arena, string);
}

internal String8
df_push_search_string(Arena *arena)
{
  String8 result = push_str8_copy(arena, df_state->string_search_string);
  return result;
}

////////////////////////////////
//~ rjf: Colors, Fonts, Config

//- rjf: keybindings

internal OS_Key
df_os_key_from_cfg_string(String8 string)
{
  OS_Key result = OS_Key_Null;
  {
    for(OS_Key key = OS_Key_Null; key < OS_Key_COUNT; key = (OS_Key)(key+1))
    {
      if(str8_match(string, os_g_key_cfg_string_table[key], StringMatchFlag_CaseInsensitive))
      {
        result = key;
        break;
      }
    }
  }
  return result;
}

internal void
df_clear_bindings(void)
{
  arena_clear(df_state->key_map_arena);
  df_state->key_map_table_size = 1024;
  df_state->key_map_table = push_array(df_state->key_map_arena, DF_KeyMapSlot, df_state->key_map_table_size);
  df_state->key_map_total_count = 0;
}

internal DF_BindingList
df_bindings_from_name(Arena *arena, String8 name)
{
  DF_BindingList result = {0};
  U64 hash = d_hash_from_string(name);
  U64 slot = hash%df_state->key_map_table_size;
  for(DF_KeyMapNode *n = df_state->key_map_table[slot].first; n != 0; n = n->hash_next)
  {
    if(str8_match(n->name, name, 0))
    {
      DF_BindingNode *node = push_array(arena, DF_BindingNode, 1);
      node->binding = n->binding;
      SLLQueuePush(result.first, result.last, node);
      result.count += 1;
    }
  }
  return result;
}

internal void
df_bind_name(String8 name, DF_Binding binding)
{
  if(binding.key != OS_Key_Null)
  {
    U64 hash = d_hash_from_string(name);
    U64 slot = hash%df_state->key_map_table_size;
    DF_KeyMapNode *existing_node = 0;
    for(DF_KeyMapNode *n = df_state->key_map_table[slot].first; n != 0; n = n->hash_next)
    {
      if(str8_match(n->name, name, 0) && n->binding.key == binding.key && n->binding.flags == binding.flags)
      {
        existing_node = n;
        break;
      }
    }
    if(existing_node == 0)
    {
      DF_KeyMapNode *n = df_state->free_key_map_node;
      if(n == 0)
      {
        n = push_array(df_state->arena, DF_KeyMapNode, 1);
      }
      else
      {
        df_state->free_key_map_node = df_state->free_key_map_node->hash_next;
      }
      n->name = push_str8_copy(df_state->arena, name);
      n->binding = binding;
      DLLPushBack_NP(df_state->key_map_table[slot].first, df_state->key_map_table[slot].last, n, hash_next, hash_prev);
      df_state->key_map_total_count += 1;
    }
  }
}

internal void
df_unbind_name(String8 name, DF_Binding binding)
{
  U64 hash = d_hash_from_string(name);
  U64 slot = hash%df_state->key_map_table_size;
  for(DF_KeyMapNode *n = df_state->key_map_table[slot].first, *next = 0; n != 0; n = next)
  {
    next = n->hash_next;
    if(str8_match(n->name, name, 0) && n->binding.key == binding.key && n->binding.flags == binding.flags)
    {
      DLLRemove_NP(df_state->key_map_table[slot].first, df_state->key_map_table[slot].last, n, hash_next, hash_prev);
      n->hash_next = df_state->free_key_map_node;
      df_state->free_key_map_node = n;
      df_state->key_map_total_count -= 1;
    }
  }
}

internal String8List
df_cmd_name_list_from_binding(Arena *arena, DF_Binding binding)
{
  String8List result = {0};
  for(U64 idx = 0; idx < df_state->key_map_table_size; idx += 1)
  {
    for(DF_KeyMapNode *n = df_state->key_map_table[idx].first; n != 0; n = n->hash_next)
    {
      if(n->binding.key == binding.key && n->binding.flags == binding.flags)
      {
        str8_list_push(arena, &result, n->name);
      }
    }
  }
  return result;
}

//- rjf: colors

internal Vec4F32
df_rgba_from_theme_color(DF_ThemeColor color)
{
  return df_state->cfg_theme.colors[color];
}

internal DF_ThemeColor
df_theme_color_from_txt_token_kind(TXT_TokenKind kind)
{
  DF_ThemeColor color = DF_ThemeColor_CodeDefault;
  switch(kind)
  {
    default:break;
    case TXT_TokenKind_Keyword:{color = DF_ThemeColor_CodeKeyword;}break;
    case TXT_TokenKind_Numeric:{color = DF_ThemeColor_CodeNumeric;}break;
    case TXT_TokenKind_String: {color = DF_ThemeColor_CodeString;}break;
    case TXT_TokenKind_Meta:   {color = DF_ThemeColor_CodeMeta;}break;
    case TXT_TokenKind_Comment:{color = DF_ThemeColor_CodeComment;}break;
    case TXT_TokenKind_Symbol: {color = DF_ThemeColor_CodeDelimiterOperator;}break;
  }
  return color;
}

//- rjf: code -> palette

internal UI_Palette *
df_palette_from_code(DF_PaletteCode code)
{
  DF_Window *window = df_window_from_handle(df_regs()->window);
  UI_Palette *result = &window->cfg_palettes[code];
  return result;
}

//- rjf: fonts/sizes

internal FNT_Tag
df_font_from_slot(DF_FontSlot slot)
{
  FNT_Tag result = df_state->cfg_font_tags[slot];
  return result;
}

internal F32
df_font_size_from_slot(DF_FontSlot slot)
{
  F32 result = 0;
  DF_Window *ws = df_window_from_handle(df_regs()->window);
  F32 dpi = os_dpi_from_window(ws->os);
  if(dpi != ws->last_dpi)
  {
    F32 old_dpi = ws->last_dpi;
    F32 new_dpi = dpi;
    ws->last_dpi = dpi;
    S32 *pt_sizes[] =
    {
      &ws->setting_vals[DF_SettingCode_MainFontSize].s32,
      &ws->setting_vals[DF_SettingCode_CodeFontSize].s32,
    };
    for(U64 idx = 0; idx < ArrayCount(pt_sizes); idx += 1)
    {
      F32 ratio = pt_sizes[idx][0] / old_dpi;
      F32 new_pt_size = ratio*new_dpi;
      pt_sizes[idx][0] = (S32)new_pt_size;
    }
  }
  switch(slot)
  {
    case DF_FontSlot_Code:
    {
      result = (F32)ws->setting_vals[DF_SettingCode_CodeFontSize].s32;
    }break;
    default:
    case DF_FontSlot_Main:
    case DF_FontSlot_Icons:
    {
      result = (F32)ws->setting_vals[DF_SettingCode_MainFontSize].s32;
    }break;
  }
  return result;
}

internal FNT_RasterFlags
df_raster_flags_from_slot(DF_FontSlot slot)
{
  FNT_RasterFlags flags = FNT_RasterFlag_Smooth|FNT_RasterFlag_Hinted;
  switch(slot)
  {
    default:{}break;
    case DF_FontSlot_Icons:{flags = FNT_RasterFlag_Smooth;}break;
    case DF_FontSlot_Main: {flags = (!!df_setting_val_from_code(DF_SettingCode_SmoothUIText).s32*FNT_RasterFlag_Smooth)|(!!df_setting_val_from_code(DF_SettingCode_HintUIText).s32*FNT_RasterFlag_Hinted);}break;
    case DF_FontSlot_Code: {flags = (!!df_setting_val_from_code(DF_SettingCode_SmoothCodeText).s32*FNT_RasterFlag_Smooth)|(!!df_setting_val_from_code(DF_SettingCode_HintCodeText).s32*FNT_RasterFlag_Hinted);}break;
  }
  return flags;
}

//- rjf: settings

internal DF_SettingVal
df_setting_val_from_code(DF_SettingCode code)
{
  DF_Window *window = df_window_from_handle(df_regs()->window);
  DF_SettingVal result = {0};
  if(window != 0)
  {
    result = window->setting_vals[code];
  }
  if(result.set == 0)
  {
    for(EachEnumVal(D_CfgSrc, src))
    {
      if(df_state->cfg_setting_vals[src][code].set)
      {
        result = df_state->cfg_setting_vals[src][code];
        break;
      }
    }
  }
  return result;
}

//- rjf: config serialization

internal int
df_qsort_compare__cfg_string_bindings(DF_StringBindingPair *a, DF_StringBindingPair *b)
{
  return strncmp((char *)a->string.str, (char *)b->string.str, Min(a->string.size, b->string.size));
}

internal String8List
df_cfg_strings_from_gfx(Arena *arena, String8 root_path, D_CfgSrc source)
{
  ProfBeginFunction();
  local_persist char *spaces = "                                                                                ";
  local_persist char *slashes= "////////////////////////////////////////////////////////////////////////////////";
  String8List strs = {0};
  
  //- rjf: write all entities
  {
    for(EachEnumVal(DF_EntityKind, k))
    {
      DF_EntityKindFlags k_flags = d_entity_kind_flags_table[k];
      if(!(k_flags & DF_EntityKindFlag_IsSerializedToConfig))
      {
        continue;
      }
      B32 first = 1;
      DF_EntityList entities = d_query_cached_entity_list_with_kind(k);
      for(DF_EntityNode *n = entities.first; n != 0; n = n->next)
      {
        DF_Entity *entity = n->entity;
        if(entity->cfg_src != source)
        {
          continue;
        }
        if(first)
        {
          first = 0;
          String8 title_name = d_entity_kind_name_lower_plural_table[k];
          str8_list_pushf(arena, &strs, "/// %S %.*s\n\n",
                          title_name,
                          (int)Max(0, 79 - (title_name.size + 5)),
                          slashes);
        }
        DF_EntityRec rec = {0};
        S64 depth = 0;
        for(DF_Entity *e = entity; !df_entity_is_nil(e); e = rec.next)
        {
          //- rjf: get next iteration
          rec = df_entity_rec_depth_first_pre(e, entity);
          
          //- rjf: unpack entity info
          typedef U32 EntityInfoFlags;
          enum
          {
            EntityInfoFlag_HasName     = (1<<0),
            EntityInfoFlag_HasDisabled = (1<<1),
            EntityInfoFlag_HasTxtPt    = (1<<2),
            EntityInfoFlag_HasVAddr    = (1<<3),
            EntityInfoFlag_HasColor    = (1<<4),
            EntityInfoFlag_HasChildren = (1<<5),
          };
          String8 entity_name_escaped = e->string;
          if(d_entity_kind_flags_table[e->kind] & DF_EntityKindFlag_NameIsPath)
          {
            Temp scratch = scratch_begin(&arena, 1);
            String8 path_normalized = path_normalized_from_string(scratch.arena, e->string);
            entity_name_escaped = path_relative_dst_from_absolute_dst_src(arena, path_normalized, root_path);
            scratch_end(scratch);
          }
          else
          {
            entity_name_escaped = d_cfg_escaped_from_raw_string(arena, e->string);
          }
          EntityInfoFlags info_flags = 0;
          if(entity_name_escaped.size != 0)        { info_flags |= EntityInfoFlag_HasName; }
          if(!!e->disabled)                        { info_flags |= EntityInfoFlag_HasDisabled; }
          if(e->flags & DF_EntityFlag_HasTextPoint) { info_flags |= EntityInfoFlag_HasTxtPt; }
          if(e->flags & DF_EntityFlag_HasVAddr)     { info_flags |= EntityInfoFlag_HasVAddr; }
          if(e->flags & DF_EntityFlag_HasColor)     { info_flags |= EntityInfoFlag_HasColor; }
          if(!df_entity_is_nil(e->first))           { info_flags |= EntityInfoFlag_HasChildren; }
          
          //- rjf: write entity info
          B32 opened_brace = 0;
          switch(info_flags)
          {
            //- rjf: default path -> entity has lots of stuff, so write all info generically
            default:
            {
              opened_brace = 1;
              
              // rjf: write entity title
              str8_list_pushf(arena, &strs, "%S:\n{\n", d_entity_kind_name_lower_table[e->kind]);
              
              // rjf: write this entity's info
              if(entity_name_escaped.size != 0)
              {
                str8_list_pushf(arena, &strs, "name: \"%S\"\n", entity_name_escaped);
              }
              if(e->disabled)
              {
                str8_list_pushf(arena, &strs, "disabled: 1\n");
              }
              if(e->flags & DF_EntityFlag_HasColor)
              {
                Vec4F32 hsva = df_hsva_from_entity(e);
                Vec4F32 rgba = rgba_from_hsva(hsva);
                U32 rgba_hex = u32_from_rgba(rgba);
                str8_list_pushf(arena, &strs, "color: 0x%x\n", rgba_hex);
              }
              if(e->flags & DF_EntityFlag_HasTextPoint)
              {
                str8_list_pushf(arena, &strs, "line: %I64d\n", e->text_point.line);
              }
              if(e->flags & DF_EntityFlag_HasVAddr)
              {
                str8_list_pushf(arena, &strs, "vaddr: (0x%I64x)\n", e->vaddr);
              }
            }break;
            
            //- rjf: single-line fast-paths
            case EntityInfoFlag_HasName:
            {str8_list_pushf(arena, &strs, "%S: \"%S\"\n", d_entity_kind_name_lower_table[e->kind], entity_name_escaped);}break;
            case EntityInfoFlag_HasName|EntityInfoFlag_HasTxtPt:
            {str8_list_pushf(arena, &strs, "%S: (\"%S\":%I64d)\n", d_entity_kind_name_lower_table[e->kind], entity_name_escaped, e->text_point.line);}break;
            case EntityInfoFlag_HasVAddr:
            {str8_list_pushf(arena, &strs, "%S: (0x%I64x)\n", d_entity_kind_name_lower_table[e->kind], e->vaddr);}break;
            
            //- rjf: empty
            case 0:
            {}break;
          }
          
          // rjf: push
          depth += rec.push_count;
          
          // rjf: pop
          if(rec.push_count == 0)
          {
            for(S64 pop_idx = 0; pop_idx < rec.pop_count + opened_brace; pop_idx += 1)
            {
              if(depth > 0)
              {
                depth -= 1;
              }
              str8_list_pushf(arena, &strs, "}\n");
            }
          }
          
          // rjf: separate top-level entities with extra newline
          if(df_entity_is_nil(rec.next) && (rec.pop_count != 0 || n->next == 0))
          {
            str8_list_pushf(arena, &strs, "\n");
          }
        }
      }
    }
  }
  
  //- rjf: write exception code filters
  if(source == D_CfgSrc_Project)
  {
    str8_list_push(arena, &strs, str8_lit("/// exception code filters ////////////////////////////////////////////////////\n"));
    str8_list_push(arena, &strs, str8_lit("\n"));
    str8_list_push(arena, &strs, str8_lit("exception_code_filters:\n"));
    str8_list_push(arena, &strs, str8_lit("{\n"));
    for(CTRL_ExceptionCodeKind k = (CTRL_ExceptionCodeKind)(CTRL_ExceptionCodeKind_Null+1);
        k < CTRL_ExceptionCodeKind_COUNT;
        k = (CTRL_ExceptionCodeKind)(k+1))
    {
      String8 name = ctrl_exception_code_kind_lowercase_code_string_table[k];
      B32 value = !!(df_state->ctrl_exception_code_filters[k/64] & (1ull<<(k%64)));
      str8_list_pushf(arena, &strs, "  %S: %i\n", name, value);
    }
    str8_list_push(arena, &strs, str8_lit("}\n\n"));
  }
  
  //- rjf: serialize windows
  {
    B32 first = 1;
    for(DF_Window *window = df_state->first_window; window != 0; window = window->next)
    {
      if(window->cfg_src != source)
      {
        continue;
      }
      if(first)
      {
        first = 0;
        str8_list_push(arena, &strs, str8_lit("/// windows ///////////////////////////////////////////////////////////////////\n"));
        str8_list_push(arena, &strs, str8_lit("\n"));
      }
      OS_Handle monitor = os_monitor_from_window(window->os);
      String8 monitor_name = os_name_from_monitor(arena, monitor);
      DF_Panel *root_panel = window->root_panel;
      Rng2F32 rect = os_rect_from_window(window->os);
      Vec2F32 size = dim_2f32(rect);
      str8_list_push (arena, &strs,  str8_lit("window:\n"));
      str8_list_push (arena, &strs,  str8_lit("{\n"));
      str8_list_pushf(arena, &strs,           "  %s%s%s\n",
                      root_panel->split_axis == Axis2_X ? "split_x" : "split_y",
                      os_window_is_fullscreen(window->os) ? " fullscreen" : "",
                      os_window_is_maximized(window->os) ? " maximized" : "");
      str8_list_pushf(arena, &strs, "  monitor: \"%S\"\n", monitor_name);
      str8_list_pushf(arena, &strs, "  size: (%i %i)\n", (int)size.x, (int)size.y);
      str8_list_pushf(arena, &strs, "  dpi: %f\n", os_dpi_from_window(window->os));
      for(EachEnumVal(DF_SettingCode, code))
      {
        DF_SettingVal current = window->setting_vals[code];
        if(current.set)
        {
          str8_list_pushf(arena, &strs, "  %S: %i\n", df_g_setting_code_lower_string_table[code], current.s32);
        }
      }
      {
        DF_PanelRec rec = {0};
        S32 indentation = 2;
        String8 indent_str = str8_lit("                                                                                                   ");
        str8_list_pushf(arena, &strs, "  panels:\n");
        str8_list_pushf(arena, &strs, "  {\n");
        for(DF_Panel *p = root_panel; !df_panel_is_nil(p); p = rec.next)
        {
          // rjf: get recursion
          rec = df_panel_rec_df_pre(p);
          
          // rjf: non-root needs pct node
          if(p != root_panel)
          {
            str8_list_pushf(arena, &strs, "%.*s%g:\n", indentation*2, indent_str.str, p->pct_of_parent);
            str8_list_pushf(arena, &strs, "%.*s{\n", indentation*2, indent_str.str);
            indentation += 1;
          }
          
          // rjf: per-panel options
          struct { String8 key; B32 value; } options[] =
          {
            {str8_lit_comp("tabs_on_bottom"),   p->tab_side == Side_Max},
          };
          B32 has_options = 0;
          for(U64 op_idx = 0; op_idx < ArrayCount(options); op_idx += 1)
          {
            if(options[op_idx].value)
            {
              if(has_options == 0)
              {
                str8_list_pushf(arena, &strs, "%.*s", indentation*2, indent_str.str);
              }
              else
              {
                str8_list_pushf(arena, &strs, " ");
              }
              has_options = 1;
              str8_list_push(arena, &strs, options[op_idx].key);
            }
          }
          if(has_options)
          {
            str8_list_pushf(arena, &strs, "\n");
          }
          
          // rjf: views
          for(DF_View *view = p->first_tab_view; !df_view_is_nil(view); view = view->order_next)
          {
            String8 view_string = view->spec->info.name;
            
            // rjf: serialize views which can be serialized
            if(view->spec->info.flags & DF_ViewSpecFlag_CanSerialize)
            {
              str8_list_pushf(arena, &strs, "%.*s", indentation*2, indent_str.str);
              
              // rjf: serialize view string
              str8_list_push(arena, &strs, view_string);
              
              // rjf: serialize view parameterizations
              str8_list_push(arena, &strs, str8_lit(": {"));
              if(view == df_selected_tab_from_panel(p))
              {
                str8_list_push(arena, &strs, str8_lit("selected "));
              }
              {
                if(view->project_path.size != 0)
                {
                  Temp scratch = scratch_begin(&arena, 1);
                  String8 project_path_absolute = path_normalized_from_string(scratch.arena, view->project_path);
                  String8 project_path_relative = path_relative_dst_from_absolute_dst_src(scratch.arena, project_path_absolute, root_path);
                  str8_list_pushf(arena, &strs, "project:{\"%S\"} ", project_path_relative);
                  scratch_end(scratch);
                }
              }
              if(view->query_string_size != 0)
              {
                Temp scratch = scratch_begin(&arena, 1);
                String8 query_raw = str8(view->query_buffer, view->query_string_size);
                {
                  String8 query_file_path = d_file_path_from_eval_string(scratch.arena, query_raw);
                  if(query_file_path.size != 0)
                  {
                    query_file_path = path_relative_dst_from_absolute_dst_src(scratch.arena, query_file_path, root_path);
                    query_raw = push_str8f(scratch.arena, "file:\"%S\"", query_file_path);
                  }
                }
                String8 query_sanitized = d_cfg_escaped_from_raw_string(scratch.arena, query_raw);
                str8_list_pushf(arena, &strs, "query:{\"%S\"} ", query_sanitized);
                scratch_end(scratch);
              }
              {
                String8 reserved_keys[] =
                {
                  str8_lit("project"),
                  str8_lit("query"),
                  str8_lit("selected"),
                };
                MD_NodeRec rec = {0};
                MD_Node *params_root = view->params_roots[view->params_read_gen%ArrayCount(view->params_roots)];
                for(MD_Node *n = params_root;
                    !md_node_is_nil(n);
                    n = rec.next)
                {
                  rec = md_node_rec_depth_first_pre(n, params_root);
                  B32 is_reserved_key = 0;
                  for(U64 idx = 0; idx < ArrayCount(reserved_keys); idx += 1)
                  {
                    if(str8_match(n->string, reserved_keys[idx], 0))
                    {
                      is_reserved_key = 1;
                      break;
                    }
                  }
                  if(is_reserved_key)
                  {
                    rec = md_node_rec_depth_first(n, params_root, OffsetOf(MD_Node, next), OffsetOf(MD_Node, next));
                  }
                  if(!is_reserved_key && n != params_root)
                  {
                    str8_list_pushf(arena, &strs, "%S", n->string);
                    if(n->first != &md_nil_node)
                    {
                      str8_list_pushf(arena, &strs, ":{");
                    }
                    for(S32 pop_idx = 0; pop_idx < rec.pop_count; pop_idx += 1)
                    {
                      if(pop_idx == rec.pop_count-1 && rec.next == &md_nil_node)
                      {
                        break;
                      }
                      str8_list_pushf(arena, &strs, "}");
                    }
                    if(rec.pop_count != 0 || n->next != &md_nil_node)
                    {
                      str8_list_pushf(arena, &strs, " ");
                    }
                  }
                }
              }
              str8_list_push(arena, &strs, str8_lit("}\n"));
            }
          }
          
          // rjf: non-roots need closer
          if(p != root_panel && rec.push_count == 0)
          {
            indentation -= 1;
            str8_list_pushf(arena, &strs, "%.*s}\n", indentation*2, indent_str.str);
          }
          
          // rjf: pop
          for(S32 pop_idx = 0; pop_idx < rec.pop_count; pop_idx += 1)
          {
            indentation -= 1;
            if(pop_idx == rec.pop_count-1 && rec.next == &df_nil_panel)
            {
              break;
            }
            str8_list_pushf(arena, &strs, "%.*s}\n", indentation*2, indent_str.str);
          }
        }
        str8_list_pushf(arena, &strs, "  }\n");
      }
      str8_list_push (arena, &strs,  str8_lit("}\n"));
      str8_list_push (arena, &strs,  str8_lit("\n"));
    }
  }
  
  //- rjf: serialize keybindings
  if(source == D_CfgSrc_User)
  {
    Temp scratch = scratch_begin(&arena, 1);
    String8 indent_str = str8_lit("                                                                                                             ");
    U64 string_binding_pair_count = 0;
    DF_StringBindingPair *string_binding_pairs = push_array(scratch.arena, DF_StringBindingPair, df_state->key_map_total_count);
    for(U64 idx = 0;
        idx < df_state->key_map_table_size && string_binding_pair_count < df_state->key_map_total_count;
        idx += 1)
    {
      for(DF_KeyMapNode *n = df_state->key_map_table[idx].first;
          n != 0 && string_binding_pair_count < df_state->key_map_total_count;
          n = n->hash_next)
      {
        DF_StringBindingPair *pair = string_binding_pairs + string_binding_pair_count;
        pair->string = n->name;
        pair->binding = n->binding;
        string_binding_pair_count += 1;
      }
    }
    quick_sort(string_binding_pairs, string_binding_pair_count, sizeof(DF_StringBindingPair), df_qsort_compare__cfg_string_bindings);
    if(string_binding_pair_count != 0)
    {
      str8_list_push(arena, &strs, str8_lit("/// keybindings ///////////////////////////////////////////////////////////////\n"));
      str8_list_push(arena, &strs, str8_lit("\n"));
      str8_list_push(arena, &strs, str8_lit("keybindings:\n"));
      str8_list_push(arena, &strs, str8_lit("{\n"));
      for(U64 idx = 0; idx < string_binding_pair_count; idx += 1)
      {
        DF_StringBindingPair *pair = string_binding_pairs + idx;
        String8List event_flags_strings = os_string_list_from_event_flags(scratch.arena, pair->binding.flags);
        StringJoin join = {str8_lit(""), str8_lit(" "), str8_lit("")};
        String8 event_flags_string = str8_list_join(scratch.arena, &event_flags_strings, &join);
        String8 key_string = push_str8_copy(scratch.arena, os_g_key_cfg_string_table[pair->binding.key]);
        for(U64 i = 0; i < event_flags_string.size; i += 1)
        {
          event_flags_string.str[i] = char_to_lower(event_flags_string.str[i]);
        }
        String8 binding_string = push_str8f(scratch.arena, "%S%s%S",
                                            event_flags_string,
                                            event_flags_string.size > 0 ? " " : "",
                                            key_string);
        str8_list_pushf(arena, &strs, "  {\"%S\"%.*s%S%.*s}\n",
                        pair->string,
                        40 > pair->string.size ? ((int)(40 - pair->string.size)) : 0, indent_str.str,
                        binding_string,
                        20 > binding_string.size ? ((int)(20 - binding_string.size)) : 0, indent_str.str);
      }
      str8_list_push(arena, &strs, str8_lit("}\n\n"));
    }
    scratch_end(scratch);
  }
  
  //- rjf: serialize theme colors
  if(source == D_CfgSrc_User)
  {
    // rjf: determine if this theme matches an existing preset
    B32 is_preset = 0;
    DF_ThemePreset matching_preset = DF_ThemePreset_DefaultDark;
    {
      for(DF_ThemePreset p = (DF_ThemePreset)0; p < DF_ThemePreset_COUNT; p = (DF_ThemePreset)(p+1))
      {
        B32 matches_this_preset = 1;
        for(DF_ThemeColor c = (DF_ThemeColor)(DF_ThemeColor_Null+1); c < DF_ThemeColor_COUNT; c = (DF_ThemeColor)(c+1))
        {
          if(!MemoryMatchStruct(&df_state->cfg_theme_target.colors[c], &df_g_theme_preset_colors_table[p][c]))
          {
            matches_this_preset = 0;
            break;
          }
        }
        if(matches_this_preset)
        {
          is_preset = 1;
          matching_preset = p;
          break;
        }
      }
    }
    
    // rjf: serialize header
    String8 indent_str = str8_lit("                                                                                                             ");
    str8_list_push(arena, &strs, str8_lit("/// colors ////////////////////////////////////////////////////////////////////\n"));
    str8_list_push(arena, &strs, str8_lit("\n"));
    
    // rjf: serialize preset theme
    if(is_preset)
    {
      str8_list_pushf(arena, &strs, "color_preset: \"%S\"\n\n", df_g_theme_preset_code_string_table[matching_preset]);
    }
    
    // rjf: serialize non-preset theme
    if(!is_preset)
    {
      str8_list_push(arena, &strs, str8_lit("colors:\n"));
      str8_list_push(arena, &strs, str8_lit("{\n"));
      for(DF_ThemeColor color = (DF_ThemeColor)(DF_ThemeColor_Null+1);
          color < DF_ThemeColor_COUNT;
          color = (DF_ThemeColor)(color+1))
      {
        String8 color_name = df_g_theme_color_cfg_string_table[color];
        Vec4F32 color_rgba = df_state->cfg_theme_target.colors[color];
        String8 color_hex  = hex_string_from_rgba_4f32(arena, color_rgba);
        str8_list_pushf(arena, &strs, "  %S:%.*s0x%S\n",
                        color_name,
                        30 > color_name.size ? ((int)(30 - color_name.size)) : 0, indent_str.str,
                        color_hex);
      }
      str8_list_push(arena, &strs, str8_lit("}\n\n"));
    }
  }
  
  //- rjf: serialize fonts
  if(source == D_CfgSrc_User)
  {
    String8 code_font_path_escaped = d_cfg_escaped_from_raw_string(arena, df_state->cfg_code_font_path);
    String8 main_font_path_escaped = d_cfg_escaped_from_raw_string(arena, df_state->cfg_main_font_path);
    str8_list_push(arena, &strs, str8_lit("/// fonts /////////////////////////////////////////////////////////////////////\n"));
    str8_list_push(arena, &strs, str8_lit("\n"));
    str8_list_pushf(arena, &strs, "code_font: \"%S\"\n", code_font_path_escaped);
    str8_list_pushf(arena, &strs, "main_font: \"%S\"\n", main_font_path_escaped);
    str8_list_push(arena, &strs, str8_lit("\n"));
  }
  
  //- rjf: serialize global settings
  {
    B32 first = 1;
    for(EachEnumVal(DF_SettingCode, code))
    {
      if(df_g_setting_code_default_is_per_window_table[code])
      {
        continue;
      }
      DF_SettingVal current = df_state->cfg_setting_vals[source][code];
      if(current.set)
      {
        if(first)
        {
          first = 0;
          str8_list_push(arena, &strs, str8_lit("/// global settings ///////////////////////////////////////////////////////////\n"));
          str8_list_push(arena, &strs, str8_lit("\n"));
        }
        str8_list_pushf(arena, &strs, "%S: %i\n", df_g_setting_code_lower_string_table[code], current.s32);
      }
    }
    if(!first)
    {
      str8_list_push(arena, &strs, str8_lit("\n"));
    }
  }
  
  ProfEnd();
  return strs;
}

////////////////////////////////
//~ rjf: Process Control Info Stringification

internal String8
df_string_from_exception_code(U32 code)
{
  String8 string = {0};
  for(EachNonZeroEnumVal(CTRL_ExceptionCodeKind, k))
  {
    if(code == ctrl_exception_code_kind_code_table[k])
    {
      string = ctrl_exception_code_kind_display_string_table[k];
      break;
    }
  }
  return string;
}

internal String8
df_stop_explanation_string_icon_from_ctrl_event(Arena *arena, CTRL_Event *event, DF_IconKind *icon_out)
{
  DF_IconKind icon = DF_IconKind_Null;
  String8 explanation = {0};
  Temp scratch = scratch_begin(&arena, 1);
  DF_Entity *thread = df_entity_from_ctrl_handle(event->entity);
  String8 thread_display_string = df_display_string_from_entity(scratch.arena, thread);
  String8 process_thread_string = thread_display_string;
  DF_Entity *process = df_entity_ancestor_from_kind(thread, DF_EntityKind_Process);
  if(process->kind == DF_EntityKind_Process)
  {
    String8 process_display_string = df_display_string_from_entity(scratch.arena, process);
    process_thread_string = push_str8f(scratch.arena, "%S: %S", process_display_string, thread_display_string);
  }
  switch(event->kind)
  {
    default:
    {
      switch(event->cause)
      {
        default:{}break;
        case CTRL_EventCause_Finished:
        {
          if(!df_entity_is_nil(thread))
          {
            explanation = push_str8f(arena, "%S completed step", process_thread_string);
          }
          else
          {
            explanation = str8_lit("Stopped");
          }
        }break;
        case CTRL_EventCause_UserBreakpoint:
        {
          if(!df_entity_is_nil(thread))
          {
            icon = DF_IconKind_CircleFilled;
            explanation = push_str8f(arena, "%S hit a breakpoint", process_thread_string);
          }
        }break;
        case CTRL_EventCause_InterruptedByException:
        {
          if(!df_entity_is_nil(thread))
          {
            icon = DF_IconKind_WarningBig;
            switch(event->exception_kind)
            {
              default:
              {
                String8 exception_code_string = df_string_from_exception_code(event->exception_code);
                explanation = push_str8f(arena, "Exception thrown by %S - 0x%x%s%S", process_thread_string, event->exception_code, exception_code_string.size > 0 ? ": " : "", exception_code_string);
              }break;
              case CTRL_ExceptionKind_CppThrow:
              {
                explanation = push_str8f(arena, "Exception thrown by %S - 0x%x: C++ exception", process_thread_string, event->exception_code);
              }break;
              case CTRL_ExceptionKind_MemoryRead:
              {
                explanation = push_str8f(arena, "Exception thrown by %S - 0x%x: Access violation reading 0x%I64x",
                                         process_thread_string,
                                         event->exception_code,
                                         event->vaddr_rng.min);
              }break;
              case CTRL_ExceptionKind_MemoryWrite:
              {
                explanation = push_str8f(arena, "Exception thrown by %S - 0x%x: Access violation writing 0x%I64x",
                                         process_thread_string,
                                         event->exception_code,
                                         event->vaddr_rng.min);
              }break;
              case CTRL_ExceptionKind_MemoryExecute:
              {
                explanation = push_str8f(arena, "Exception thrown by %S - 0x%x: Access violation executing 0x%I64x",
                                         process_thread_string,
                                         event->exception_code,
                                         event->vaddr_rng.min);
              }break;
            }
          }
          else
          {
            icon = DF_IconKind_Pause;
            explanation = str8_lit("Interrupted");
          }
        }break;
        case CTRL_EventCause_InterruptedByTrap:
        {
          icon = DF_IconKind_WarningBig;
          explanation = push_str8f(arena, "%S interrupted by trap - 0x%x", process_thread_string, event->exception_code);
        }break;
        case CTRL_EventCause_InterruptedByHalt:
        {
          icon = DF_IconKind_Pause;
          explanation = str8_lit("Halted");
        }break;
      }
    }break;
  }
  scratch_end(scratch);
  if(icon_out)
  {
    *icon_out = icon;
  }
  return explanation;
}

////////////////////////////////
//~ rjf: Continuous Frame Requests

internal void
df_request_frame(void)
{
  df_state->num_frames_requested = 4;
}

////////////////////////////////
//~ rjf: Main State Accessors

//- rjf: per-frame arena

internal Arena *
df_frame_arena(void)
{
  return df_state->frame_arenas[df_state->frame_index%ArrayCount(df_state->frame_arenas)];
}

//- rjf: config paths

internal String8
df_cfg_path_from_src(D_CfgSrc src)
{
  return df_state->cfg_paths[src];
}

//- rjf: entity cache queries

internal DF_EntityList
d_query_cached_entity_list_with_kind(DF_EntityKind kind)
{
  ProfBeginFunction();
  D_EntityListCache *cache = &df_state->kind_caches[kind];
  
  // rjf: build cached list if we're out-of-date
  if(cache->alloc_gen != df_state->kind_alloc_gens[kind])
  {
    cache->alloc_gen = df_state->kind_alloc_gens[kind];
    if(cache->arena == 0)
    {
      cache->arena = arena_alloc();
    }
    arena_clear(cache->arena);
    cache->list = df_push_entity_list_with_kind(cache->arena, kind);
  }
  
  // rjf: grab & return cached list
  DF_EntityList result = cache->list;
  ProfEnd();
  return result;
}

internal DF_EntityList
d_push_active_target_list(Arena *arena)
{
  DF_EntityList active_targets = {0};
  DF_EntityList all_targets = d_query_cached_entity_list_with_kind(DF_EntityKind_Target);
  for(DF_EntityNode *n = all_targets.first; n != 0; n = n->next)
  {
    if(!n->entity->disabled)
    {
      df_entity_list_push(arena, &active_targets, n->entity);
    }
  }
  return active_targets;
}

internal DF_Entity *
d_entity_from_ev_key_and_kind(EV_Key key, DF_EntityKind kind)
{
  DF_Entity *result = &d_nil_entity;
  DF_EntityList list = d_query_cached_entity_list_with_kind(kind);
  for(DF_EntityNode *n = list.first; n != 0; n = n->next)
  {
    DF_Entity *entity = n->entity;
    if(ev_key_match(df_ev_key_from_entity(entity), key))
    {
      result = entity;
      break;
    }
  }
  return result;
}

//- rjf: config state

internal D_CfgTable *
df_cfg_table(void)
{
  return &df_state->cfg_table;
}

////////////////////////////////
//~ rjf: Registers

internal DF_Regs *
df_regs(void)
{
  DF_Regs *regs = &df_state->top_regs->v;
  return regs;
}

internal DF_Regs *
df_base_regs(void)
{
  DF_Regs *regs = &df_state->base_regs.v;
  return regs;
}

internal DF_Regs *
df_push_regs_(DF_Regs *regs)
{
  DF_RegsNode *n = push_array(df_frame_arena(), DF_RegsNode, 1);
  df_regs_copy_contents(df_frame_arena(), &n->v, regs);
  SLLStackPush(df_state->top_regs, n);
  return &n->v;
}

internal DF_Regs *
df_pop_regs(void)
{
  DF_Regs *regs = &df_state->top_regs->v;
  SLLStackPop(df_state->top_regs);
  if(df_state->top_regs == 0)
  {
    df_state->top_regs = &df_state->base_regs;
  }
  return regs;
}

internal void
df_regs_fill_slot_from_string(DF_RegSlot slot, String8 string)
{
  String8 error = {0};
  switch(slot)
  {
    default:
    case DF_RegSlot_String:
    {
      df_regs()->string = push_str8_copy(df_frame_arena(), string);
    }break;
    case DF_RegSlot_FilePath:
    {
      String8TxtPtPair pair = str8_txt_pt_pair_from_string(string);
      df_regs()->file_path = push_str8_copy(df_frame_arena(), pair.string);
      df_regs()->cursor = pair.pt;
    }break;
    case DF_RegSlot_Cursor:
    {
      U64 v = 0;
      if(try_u64_from_str8_c_rules(string, &v))
      {
        df_regs()->cursor.column = 1;
        df_regs()->cursor.line = v;
      }
      else
      {
        log_user_error(str8_lit("Couldn't interpret as a line number."));
      }
    }break;
    case DF_RegSlot_Vaddr: goto use_numeric_eval;
    case DF_RegSlot_Voff: goto use_numeric_eval;
    case DF_RegSlot_UnwindCount: goto use_numeric_eval;
    case DF_RegSlot_InlineDepth: goto use_numeric_eval;
    case DF_RegSlot_PID: goto use_numeric_eval;
    use_numeric_eval:
    {
      Temp scratch = scratch_begin(0, 0);
      E_Eval eval = e_eval_from_string(scratch.arena, string);
      if(eval.msgs.max_kind == E_MsgKind_Null)
      {
        E_TypeKind eval_type_kind = e_type_kind_from_key(e_type_unwrap(eval.type_key));
        if(eval_type_kind == E_TypeKind_Ptr ||
           eval_type_kind == E_TypeKind_LRef ||
           eval_type_kind == E_TypeKind_RRef)
        {
          eval = e_value_eval_from_eval(eval);
        }
        U64 u64 = eval.value.u64;
        switch(slot)
        {
          default:{}break;
          case DF_RegSlot_Vaddr:
          {
            df_regs()->vaddr = u64;
          }break;
          case DF_RegSlot_Voff:
          {
            df_regs()->voff = u64;
          }break;
          case DF_RegSlot_UnwindCount:
          {
            df_regs()->unwind_count = u64;
          }break;
          case DF_RegSlot_InlineDepth:
          {
            df_regs()->inline_depth = u64;
          }break;
          case DF_RegSlot_PID:
          {
            df_regs()->pid = u64;
          }break;
        }
      }
      else
      {
        log_user_errorf("Couldn't evaluate \"%S\" as an address.", string);
      }
      scratch_end(scratch);
    }break;
  }
}

////////////////////////////////
//~ rjf: Commands

// TODO(rjf): @msgs temporary glue
#if 0
internal D_CmdSpec *
df_cmd_spec_from_kind(DF_CmdKind kind)
{
  String8 string = df_cmd_kind_spec_info_table[kind].string;
  D_CmdSpec *result = d_cmd_spec_from_string(string);
  return result;
}
#endif

internal DF_CmdKind
df_cmd_kind_from_string(String8 string)
{
  DF_CmdKind result = DF_CmdKind_Null;
  for(U64 idx = 0; idx < ArrayCount(df_cmd_kind_info_table); idx += 1)
  {
    if(str8_match(string, df_cmd_kind_info_table[idx].string, 0))
    {
      result = (DF_CmdKind)idx;
      break;
    }
  }
  return result;
}

//- rjf: name -> info

internal DF_CmdKindInfo *
df_cmd_kind_info_from_string(String8 string)
{
  DF_CmdKindInfo *info = &df_nil_cmd_kind_info;
  {
    // TODO(rjf): extend this by looking up into dynamically-registered commands by views
    DF_CmdKind kind = df_cmd_kind_from_string(string);
    if(kind != DF_CmdKind_Null)
    {
      info = &df_cmd_kind_info_table[kind];
    }
  }
  return info;
}

//- rjf: pushing

internal void
df_push_cmd(String8 name, DF_Regs *regs)
{
  df_cmd_list_push_new(df_state->cmds_arena, &df_state->cmds, name, regs);
}

//- rjf: iterating

internal B32
df_next_cmd(DF_Cmd **cmd)
{
  DF_CmdNode *start_node = df_state->cmds.first;
  if(cmd[0] != 0)
  {
    start_node = CastFromMember(DF_CmdNode, cmd, cmd[0]);
    start_node = start_node->next;
  }
  cmd[0] = 0;
  if(start_node != 0)
  {
    cmd[0] = &start_node->cmd;
  }
  return !!cmd[0];
}

////////////////////////////////
//~ rjf: Main Layer Top-Level Calls

#if !defined(STBI_INCLUDE_STB_IMAGE_H)
# define STB_IMAGE_IMPLEMENTATION
# define STBI_ONLY_PNG
# define STBI_ONLY_BMP
# include "third_party/stb/stb_image.h"
#endif

internal void
df_init(CmdLine *cmdln)
{
  ProfBeginFunction();
  Arena *arena = arena_alloc();
  df_state = push_array(arena, DF_State, 1);
  df_state->arena = arena;
  for(U64 idx = 0; idx < ArrayCount(df_state->frame_arenas); idx += 1)
  {
    df_state->frame_arenas[idx] = arena_alloc();
  }
  df_state->log = log_alloc();
  log_select(df_state->log);
  {
    Temp scratch = scratch_begin(0, 0);
    String8 user_program_data_path = os_get_process_info()->user_program_data_path;
    String8 user_data_folder = push_str8f(scratch.arena, "%S/raddbg/logs", user_program_data_path);
    df_state->log_path = push_str8f(df_state->arena, "%S/ui_thread.raddbg_log", user_data_folder);
    os_make_directory(user_data_folder);
    os_write_data_to_file_path(df_state->log_path, str8_zero());
    scratch_end(scratch);
  }
  df_state->num_frames_requested = 2;
  df_state->seconds_until_autosave = 0.5f;
  df_state->cmds_arena = arena_alloc();
  df_state->entities_arena = arena_alloc(.reserve_size = GB(64), .commit_size = KB(64));
  df_state->entities_root = &d_nil_entity;
  df_state->entities_base = push_array(df_state->entities_arena, DF_Entity, 0);
  df_state->entities_count = 0;
  df_state->entities_root = df_entity_alloc(&d_nil_entity, DF_EntityKind_Root);
  df_state->key_map_arena = arena_alloc();
  df_state->confirm_arena = arena_alloc();
  df_state->view_spec_table_size = 256;
  df_state->view_spec_table = push_array(arena, DF_ViewSpec *, df_state->view_spec_table_size);
  df_state->view_rule_spec_table_size = 1024;
  df_state->view_rule_spec_table = push_array(arena, DF_ViewRuleSpec *, d_state->view_rule_spec_table_size);
  df_state->code_ctx_menu_key   = ui_key_from_string(ui_key_zero(), str8_lit("_code_ctx_menu_"));
  df_state->entity_ctx_menu_key = ui_key_from_string(ui_key_zero(), str8_lit("_entity_ctx_menu_"));
  df_state->tab_ctx_menu_key    = ui_key_from_string(ui_key_zero(), str8_lit("_tab_ctx_menu_"));
  df_state->string_search_arena = arena_alloc();
  df_state->eval_viz_view_cache_slots_count = 1024;
  df_state->eval_viz_view_cache_slots = push_array(arena, DF_EvalVizViewCacheSlot, df_state->eval_viz_view_cache_slots_count);
  df_state->cfg_main_font_path_arena = arena_alloc();
  df_state->cfg_code_font_path_arena = arena_alloc();
  df_state->bind_change_arena = arena_alloc();
  df_state->top_regs = &df_state->base_regs;
  df_clear_bindings();
  
  // rjf: set up initial entities
  {
    DF_Entity *local_machine = df_entity_alloc(df_state->entities_root, DF_EntityKind_Machine);
    df_entity_equip_ctrl_handle(local_machine, ctrl_handle_make(CTRL_MachineID_Local, dmn_handle_zero()));
    df_entity_equip_name(local_machine, str8_lit("This PC"));
  }
  
  // rjf: register gfx layer views
  {
    DF_ViewSpecInfoArray array = {df_g_gfx_view_kind_spec_info_table, ArrayCount(df_g_gfx_view_kind_spec_info_table)};
    df_register_view_specs(array);
  }
  
  // rjf: register gfx layer view rules
  {
    DF_ViewRuleSpecInfoArray array = {df_g_gfx_view_rule_spec_info_table, ArrayCount(df_g_gfx_view_rule_spec_info_table)};
    df_register_view_rule_specs(array);
  }
  
  // rjf: set up user / project paths
  {
    Temp scratch = scratch_begin(0, 0);
    
    // rjf: unpack command line arguments
    String8 user_cfg_path = cmd_line_string(cmdln, str8_lit("user"));
    String8 project_cfg_path = cmd_line_string(cmdln, str8_lit("project"));
    if(project_cfg_path.size == 0)
    {
      project_cfg_path = cmd_line_string(cmdln, str8_lit("profile"));
    }
    {
      String8 user_program_data_path = os_get_process_info()->user_program_data_path;
      String8 user_data_folder = push_str8f(scratch.arena, "%S/%S", user_program_data_path, str8_lit("raddbg"));
      os_make_directory(user_data_folder);
      if(user_cfg_path.size == 0)
      {
        user_cfg_path = push_str8f(scratch.arena, "%S/default.raddbg_user", user_data_folder);
      }
      if(project_cfg_path.size == 0)
      {
        project_cfg_path = push_str8f(scratch.arena, "%S/default.raddbg_project", user_data_folder);
      }
    }
    
    // rjf: set up config path state
    String8 cfg_src_paths[D_CfgSrc_COUNT] = {user_cfg_path, project_cfg_path};
    for(D_CfgSrc src = (D_CfgSrc)0; src < D_CfgSrc_COUNT; src = (D_CfgSrc)(src+1))
    {
      df_state->cfg_path_arenas[src] = arena_alloc();
      df_cmd(d_cfg_src_load_cmd_kind_table[src], .file_path = path_normalized_from_string(scratch.arena, cfg_src_paths[src]));
    }
    
    // rjf: set up config table arena
    df_state->cfg_arena = arena_alloc();
    scratch_end(scratch);
  }
  
  // rjf: set up initial exception filtering rules
  for(CTRL_ExceptionCodeKind k = (CTRL_ExceptionCodeKind)0; k < CTRL_ExceptionCodeKind_COUNT; k = (CTRL_ExceptionCodeKind)(k+1))
  {
    if(ctrl_exception_code_kind_default_enable_table[k])
    {
      df_state->ctrl_exception_code_filters[k/64] |= 1ull<<(k%64);
    }
  }
  
  // rjf: unpack icon image data
  {
    Temp scratch = scratch_begin(0, 0);
    String8 data = df_g_icon_file_bytes;
    U8 *ptr = data.str;
    U8 *opl = ptr+data.size;
    
    // rjf: read header
    ICO_Header hdr = {0};
    if(ptr+sizeof(hdr) < opl)
    {
      MemoryCopy(&hdr, ptr, sizeof(hdr));
      ptr += sizeof(hdr);
    }
    
    // rjf: read image entries
    U64 entries_count = hdr.num_images;
    ICO_Entry *entries = push_array(scratch.arena, ICO_Entry, hdr.num_images);
    {
      U64 bytes_to_read = sizeof(ICO_Entry)*entries_count;
      bytes_to_read = Min(bytes_to_read, opl-ptr);
      MemoryCopy(entries, ptr, bytes_to_read);
      ptr += bytes_to_read;
    }
    
    // rjf: find largest image
    ICO_Entry *best_entry = 0;
    U64 best_entry_area = 0;
    for(U64 idx = 0; idx < entries_count; idx += 1)
    {
      ICO_Entry *entry = &entries[idx];
      U64 width = entry->image_width_px;
      if(width == 0) { width = 256; }
      U64 height = entry->image_height_px;
      if(height == 0) { height = 256; }
      U64 entry_area = width*height;
      if(entry_area > best_entry_area)
      {
        best_entry = entry;
        best_entry_area = entry_area;
      }
    }
    
    // rjf: deserialize raw image data from best entry's offset
    U8 *image_data = 0;
    Vec2S32 image_dim = {0};
    if(best_entry != 0)
    {
      U8 *file_data_ptr = data.str + best_entry->image_data_off;
      U64 file_data_size = best_entry->image_data_size;
      int width = 0;
      int height = 0;
      int components = 0;
      image_data = stbi_load_from_memory(file_data_ptr, file_data_size, &width, &height, &components, 4);
      image_dim.x = width;
      image_dim.y = height;
    }
    
    // rjf: upload to gpu texture
    df_state->icon_texture = r_tex2d_alloc(R_ResourceKind_Static, image_dim, R_Tex2DFormat_RGBA8, image_data);
    
    // rjf: release
    stbi_image_free(image_data);
    scratch_end(scratch);
  }
  
  // rjf: set up initial browse path
  {
    Temp scratch = scratch_begin(0, 0);
    String8 current_path = os_get_current_path(scratch.arena);
    String8 current_path_with_slash = push_str8f(scratch.arena, "%S/", current_path);
    df_state->current_path_arena = arena_alloc();
    df_state->current_path = push_str8_copy(df_state->current_path_arena, current_path_with_slash);
    scratch_end(scratch);
  }
  
  ProfEnd();
}

internal void
df_frame(void)
{
  Temp scratch = scratch_begin(0, 0);
  DI_Scope *di_scope = di_scope_open();
  local_persist S32 depth = 0;
  log_scope_begin();
  
  //////////////////////////////
  //- rjf: do per-frame resets
  //
  arena_clear(df_frame_arena());
  df_state->top_regs = &df_state->base_regs;
  df_regs_copy_contents(df_frame_arena(), &df_state->top_regs->v, &df_state->top_regs->v);
  if(df_state->next_hover_regs != 0)
  {
    df_state->hover_regs = df_regs_copy(df_frame_arena(), df_state->next_hover_regs);
    df_state->next_hover_regs = 0;
  }
  else
  {
    df_state->hover_regs = push_array(df_frame_arena(), DF_Regs, 1);
  }
  
  //////////////////////////////
  //- rjf: get events from the OS
  //
  OS_EventList events = {0};
  if(depth == 0) DeferLoop(depth += 1, depth -= 1)
  {
    events = os_get_events(scratch.arena, df_state->num_frames_requested == 0);
  }
  
  //////////////////////////////
  //- rjf: pick target hz
  //
  // TODO(rjf): maximize target, given all windows and their monitors
  F32 target_hz = os_get_gfx_info()->default_refresh_rate;
  if(df_state->frame_index > 32)
  {
    // rjf: calculate average frame time out of the last N
    U64 num_frames_in_history = Min(ArrayCount(df_state->frame_time_us_history), df_state->frame_index);
    U64 frame_time_history_sum_us = 0;
    for(U64 idx = 0; idx < num_frames_in_history; idx += 1)
    {
      frame_time_history_sum_us += df_state->frame_time_us_history[idx];
    }
    U64 frame_time_history_avg_us = frame_time_history_sum_us/num_frames_in_history;
    
    // rjf: pick among a number of sensible targets to snap to, given how well
    // we've been performing
    F32 possible_alternate_hz_targets[] = {target_hz, 60.f, 120.f, 144.f, 240.f};
    F32 best_target_hz = target_hz;
    S64 best_target_hz_frame_time_us_diff = max_S64;
    for(U64 idx = 0; idx < ArrayCount(possible_alternate_hz_targets); idx += 1)
    {
      F32 candidate = possible_alternate_hz_targets[idx];
      if(candidate <= target_hz)
      {
        U64 candidate_frame_time_us = 1000000/(U64)candidate;
        S64 frame_time_us_diff = (S64)frame_time_history_avg_us - (S64)candidate_frame_time_us;
        if(abs_s64(frame_time_us_diff) < best_target_hz_frame_time_us_diff)
        {
          best_target_hz = candidate;
          best_target_hz_frame_time_us_diff = frame_time_us_diff;
        }
      }
    }
    target_hz = best_target_hz;
  }
  
  //////////////////////////////
  //- rjf: target Hz -> delta time
  //
  df_state->frame_dt = 1.f/target_hz;
  
  //////////////////////////////
  //- rjf: begin measuring actual per-frame work
  //
  U64 begin_time_us = os_now_microseconds();
  
  //////////////////////////////
  //- rjf: bind change
  //
  if(!df_state->confirm_active && df_state->bind_change_active)
  {
    if(os_key_press(&events, os_handle_zero(), 0, OS_Key_Esc))
    {
      df_request_frame();
      df_state->bind_change_active = 0;
    }
    if(os_key_press(&events, os_handle_zero(), 0, OS_Key_Delete))
    {
      df_request_frame();
      df_unbind_name(df_state->bind_change_cmd_name, df_state->bind_change_binding);
      df_state->bind_change_active = 0;
      df_cmd(d_cfg_src_write_cmd_kind_table[D_CfgSrc_User]);
    }
    for(OS_Event *event = events.first, *next = 0; event != 0; event = next)
    {
      if(event->kind == OS_EventKind_Press &&
         event->key != OS_Key_Esc &&
         event->key != OS_Key_Return &&
         event->key != OS_Key_Backspace &&
         event->key != OS_Key_Delete &&
         event->key != OS_Key_LeftMouseButton &&
         event->key != OS_Key_RightMouseButton &&
         event->key != OS_Key_MiddleMouseButton &&
         event->key != OS_Key_Ctrl &&
         event->key != OS_Key_Alt &&
         event->key != OS_Key_Shift)
      {
        df_state->bind_change_active = 0;
        DF_Binding binding = zero_struct;
        {
          binding.key = event->key;
          binding.flags = event->flags;
        }
        df_unbind_name(df_state->bind_change_cmd_name, df_state->bind_change_binding);
        df_bind_name(df_state->bind_change_cmd_name, binding);
        U32 codepoint = os_codepoint_from_event_flags_and_key(event->flags, event->key);
        os_text(&events, os_handle_zero(), codepoint);
        os_eat_event(&events, event);
        df_cmd(d_cfg_src_write_cmd_kind_table[D_CfgSrc_User]);
        df_request_frame();
        break;
      }
    }
  }
  
  //////////////////////////////
  //- rjf: consume events
  //
  {
    for(OS_Event *event = events.first, *next = 0;
        event != 0;
        event = next)
      DF_RegsScope()
    {
      next = event->next;
      DF_Window *window = df_window_from_os_handle(event->window);
      if(window != 0 && window != df_window_from_handle(df_regs()->window))
      {
        df_regs()->window = df_handle_from_window(window);
        df_regs()->panel  = df_handle_from_panel(window->focused_panel);
        df_regs()->view   = window->focused_panel->selected_tab_view;
      }
      B32 take = 0;
      
      //- rjf: try window close
      if(!take && event->kind == OS_EventKind_WindowClose && window != 0)
      {
        take = 1;
        df_cmd(DF_CmdKind_CloseWindow, .window = df_handle_from_window(window));
      }
      
      //- rjf: try menu bar operations
      {
        if(!take && event->kind == OS_EventKind_Press && event->key == OS_Key_Alt && event->flags == 0 && event->is_repeat == 0)
        {
          take = 1;
          df_request_frame();
          window->menu_bar_focused_on_press = window->menu_bar_focused;
          window->menu_bar_key_held = 1;
          window->menu_bar_focus_press_started = 1;
        }
        if(!take && event->kind == OS_EventKind_Release && event->key == OS_Key_Alt && event->flags == 0 && event->is_repeat == 0)
        {
          take = 1;
          df_request_frame();
          window->menu_bar_key_held = 0;
        }
        if(window->menu_bar_focused && event->kind == OS_EventKind_Press && event->key == OS_Key_Alt && event->flags == 0 && event->is_repeat == 0)
        {
          take = 1;
          df_request_frame();
          window->menu_bar_focused = 0;
        }
        else if(window->menu_bar_focus_press_started && !window->menu_bar_focused && event->kind == OS_EventKind_Release && event->flags == 0 && event->key == OS_Key_Alt && event->is_repeat == 0)
        {
          take = 1;
          df_request_frame();
          window->menu_bar_focused = !window->menu_bar_focused_on_press;
          window->menu_bar_focus_press_started = 0;
        }
        else if(event->kind == OS_EventKind_Press && event->key == OS_Key_Esc && window->menu_bar_focused && !ui_any_ctx_menu_is_open())
        {
          take = 1;
          df_request_frame();
          window->menu_bar_focused = 0;
        }
      }
      
      //- rjf: try hotkey presses
      if(!take && event->kind == OS_EventKind_Press)
      {
        DF_Binding binding = {event->key, event->flags};
        String8List spec_candidates = df_cmd_name_list_from_binding(scratch.arena, binding);
        if(spec_candidates.first != 0)
        {
          df_cmd(DF_CmdKind_RunCommand, .string = spec_candidates.first->string);
          U32 hit_char = os_codepoint_from_event_flags_and_key(event->flags, event->key);
          take = 1;
          if(event->flags & OS_EventFlag_Alt)
          {
            window->menu_bar_focus_press_started = 0;
          }
        }
        else if(OS_Key_F1 <= event->key && event->key <= OS_Key_F19)
        {
          window->menu_bar_focus_press_started = 0;
        }
        df_request_frame();
      }
      
      //- rjf: try text events
      if(!take && event->kind == OS_EventKind_Text)
      {
        String32 insertion32 = str32(&event->character, 1);
        String8 insertion8 = str8_from_32(scratch.arena, insertion32);
        df_cmd(DF_CmdKind_InsertText, .string = insertion8);
        df_request_frame();
        take = 1;
        if(event->flags & OS_EventFlag_Alt)
        {
          window->menu_bar_focus_press_started = 0;
        }
      }
      
      //- rjf: do fall-through
      if(!take)
      {
        take = 1;
        df_cmd(DF_CmdKind_OSEvent, .os_event = event);
      }
      
      //- rjf: take
      if(take)
      {
        os_eat_event(&events, event);
      }
    }
  }
  
  //////////////////////////////
  //- rjf: unpack eval-dependent info
  //
  CTRL_Entity *process = ctrl_entity_from_handle(d_state->ctrl_entity_store, df_regs()->process);
  CTRL_Entity *thread = ctrl_entity_from_handle(d_state->ctrl_entity_store, df_regs()->thread);
  Arch arch = thread->arch;
  U64 unwind_count = df_regs()->unwind_count;
  U64 rip_vaddr = d_query_cached_rip_from_thread_unwind(thread, unwind_count);
  CTRL_Unwind unwind = d_query_cached_unwind_from_thread(thread);
  CTRL_Entity *module = ctrl_module_from_process_vaddr(process, rip_vaddr);
  U64 rip_voff = ctrl_voff_from_vaddr(module, rip_vaddr);
  U64 tls_root_vaddr = ctrl_query_cached_tls_root_vaddr_from_thread(d_state->ctrl_entity_store, thread->handle);
  CTRL_EntityList all_modules = ctrl_entity_list_from_kind(d_state->ctrl_entity_store, CTRL_EntityKind_Module);
  U64 eval_modules_count = Max(1, all_modules.count);
  E_Module *eval_modules = push_array(scratch.arena, E_Module, eval_modules_count);
  E_Module *eval_modules_primary = &eval_modules[0];
  eval_modules_primary->rdi = &di_rdi_parsed_nil;
  eval_modules_primary->vaddr_range = r1u64(0, max_U64);
  DI_Key primary_dbgi_key = {0};
  {
    U64 eval_module_idx = 0;
    for(CTRL_EntityNode *n = all_modules.first; n != 0; n = n->next, eval_module_idx += 1)
    {
      CTRL_Entity *m = n->v;
      DI_Key dbgi_key = ctrl_dbgi_key_from_module(m);
      eval_modules[eval_module_idx].arch        = m->arch;
      eval_modules[eval_module_idx].rdi         = di_rdi_from_key(di_scope, &dbgi_key, 0);
      eval_modules[eval_module_idx].vaddr_range = m->vaddr_range;
      eval_modules[eval_module_idx].space       = d_eval_space_from_ctrl_entity(ctrl_entity_ancestor_from_kind(m, CTRL_EntityKind_Process));
      if(module == m)
      {
        eval_modules_primary = &eval_modules[eval_module_idx];
      }
    }
  }
  U64 rdis_count = Max(1, all_modules.count);
  RDI_Parsed **rdis = push_array(scratch.arena, RDI_Parsed *, rdis_count);
  rdis[0] = &di_rdi_parsed_nil;
  U64 rdis_primary_idx = 0;
  Rng1U64 *rdis_vaddr_ranges = push_array(scratch.arena, Rng1U64, rdis_count);
  {
    U64 idx = 0;
    for(CTRL_EntityNode *n = all_modules.first; n != 0; n = n->next, idx += 1)
    {
      DI_Key dbgi_key = ctrl_dbgi_key_from_module(n->v);
      rdis[idx] = di_rdi_from_key(di_scope, &dbgi_key, 0);
      rdis_vaddr_ranges[idx] = n->v->vaddr_range;
      if(n->v == module)
      {
        primary_dbgi_key = dbgi_key;
        rdis_primary_idx = idx;
      }
    }
  }
  
  //////////////////////////////
  //- rjf: build eval type context
  //
  E_TypeCtx *type_ctx = push_array(scratch.arena, E_TypeCtx, 1);
  {
    E_TypeCtx *ctx = type_ctx;
    ctx->ip_vaddr          = rip_vaddr;
    ctx->ip_voff           = rip_voff;
    ctx->modules           = eval_modules;
    ctx->modules_count     = eval_modules_count;
    ctx->primary_module    = eval_modules_primary;
  }
  e_select_type_ctx(type_ctx);
  
  //////////////////////////////
  //- rjf: build eval parse context
  //
  E_ParseCtx *parse_ctx = push_array(scratch.arena, E_ParseCtx, 1);
  ProfScope("build eval parse context")
  {
    E_ParseCtx *ctx = parse_ctx;
    ctx->ip_vaddr          = rip_vaddr;
    ctx->ip_voff           = rip_voff;
    ctx->ip_thread_space   = d_eval_space_from_ctrl_entity(thread);
    ctx->modules           = eval_modules;
    ctx->modules_count     = eval_modules_count;
    ctx->primary_module    = eval_modules_primary;
    ctx->regs_map      = ctrl_string2reg_from_arch(ctx->primary_module->arch);
    ctx->reg_alias_map = ctrl_string2alias_from_arch(ctx->primary_module->arch);
    ctx->locals_map    = d_query_cached_locals_map_from_dbgi_key_voff(&primary_dbgi_key, rip_voff);
    ctx->member_map    = d_query_cached_member_map_from_dbgi_key_voff(&primary_dbgi_key, rip_voff);
  }
  e_select_parse_ctx(parse_ctx);
  
  //////////////////////////////
  //- rjf: build eval IR context
  //
  E_IRCtx *ir_ctx = push_array(scratch.arena, E_IRCtx, 1);
  {
    E_IRCtx *ctx = ir_ctx;
    ctx->macro_map     = push_array(scratch.arena, E_String2ExprMap, 1);
    ctx->macro_map[0]  = e_string2expr_map_make(scratch.arena, 512);
    
    //- rjf: add macros for constants
    {
      // rjf: pid -> current process' ID
      if(process != &ctrl_entity_nil)
      {
        E_Expr *expr = e_push_expr(scratch.arena, E_ExprKind_LeafU64, 0);
        expr->value.u64 = process->id;
        e_string2expr_map_insert(scratch.arena, ctx->macro_map, str8_lit("pid"), expr);
      }
      
      // rjf: tid -> current thread's ID
      if(thread != &ctrl_entity_nil)
      {
        E_Expr *expr = e_push_expr(scratch.arena, E_ExprKind_LeafU64, 0);
        expr->value.u64 = thread->id;
        e_string2expr_map_insert(scratch.arena, ctx->macro_map, str8_lit("tid"), expr);
      }
    }
    
    //- rjf: add macros for entities
    {
      E_MemberList entity_members = {0};
      {
        e_member_list_push_new(scratch.arena, &entity_members, .name = str8_lit("enabled"),  .off = 0,        .type_key = e_type_key_basic(E_TypeKind_S64));
        e_member_list_push_new(scratch.arena, &entity_members, .name = str8_lit("hit_count"),.off = 0+8,      .type_key = e_type_key_basic(E_TypeKind_U64));
        e_member_list_push_new(scratch.arena, &entity_members, .name = str8_lit("label"),    .off = 0+8+8,    .type_key = e_type_key_cons_ptr(arch_from_context(), e_type_key_basic(E_TypeKind_Char8)));
        e_member_list_push_new(scratch.arena, &entity_members, .name = str8_lit("location"), .off = 0+8+8+8,  .type_key = e_type_key_cons_ptr(arch_from_context(), e_type_key_basic(E_TypeKind_Char8)));
        e_member_list_push_new(scratch.arena, &entity_members, .name = str8_lit("condition"),.off = 0+8+8+8+8,.type_key = e_type_key_cons_ptr(arch_from_context(), e_type_key_basic(E_TypeKind_Char8)));
      }
      E_MemberArray entity_members_array = e_member_array_from_list(scratch.arena, &entity_members);
      E_TypeKey entity_type = e_type_key_cons(.arch = arch_from_context(),
                                              .kind = E_TypeKind_Struct,
                                              .name = str8_lit("Entity"),
                                              .members = entity_members_array.v,
                                              .count = entity_members_array.count);
      DF_EntityKind evallable_kinds[] =
      {
        DF_EntityKind_Breakpoint,
        DF_EntityKind_WatchPin,
        DF_EntityKind_Target,
      };
      for(U64 idx = 0; idx < ArrayCount(evallable_kinds); idx += 1)
      {
        DF_EntityList entities = d_query_cached_entity_list_with_kind(evallable_kinds[idx]);
        for(DF_EntityNode *n = entities.first; n != 0; n = n->next)
        {
          DF_Entity *entity = n->entity;
          E_Expr *expr = e_push_expr(scratch.arena, E_ExprKind_LeafOffset, 0);
          expr->space    = d_eval_space_from_entity(entity);
          expr->mode     = E_Mode_Offset;
          expr->type_key = entity_type;
          e_string2expr_map_insert(scratch.arena, ctx->macro_map, push_str8f(scratch.arena, "$%I64u", entity->id), expr);
          if(entity->string.size != 0)
          {
            e_string2expr_map_insert(scratch.arena, ctx->macro_map, entity->string, expr);
          }
        }
      }
    }
    
    //- rjf: add macros for all watches which define identifiers
    DF_EntityList watches = d_query_cached_entity_list_with_kind(DF_EntityKind_Watch);
    for(DF_EntityNode *n = watches.first; n != 0; n = n->next)
    {
      DF_Entity *watch = n->entity;
      String8 expr = watch->string;
      E_TokenArray tokens   = e_token_array_from_text(scratch.arena, expr);
      E_Parse      parse    = e_parse_expr_from_text_tokens(scratch.arena, expr, &tokens);
      if(parse.msgs.max_kind == E_MsgKind_Null)
      {
        e_push_leaf_ident_exprs_from_expr__in_place(scratch.arena, ctx->macro_map, parse.expr);
      }
    }
  }
  e_select_ir_ctx(ir_ctx);
  
  //////////////////////////////
  //- rjf: build eval interpretation context
  //
  E_InterpretCtx *interpret_ctx = push_array(scratch.arena, E_InterpretCtx, 1);
  {
    E_InterpretCtx *ctx = interpret_ctx;
    ctx->space_read        = d_eval_space_read;
    ctx->space_write       = d_eval_space_write;
    ctx->primary_space     = eval_modules_primary->space;
    ctx->reg_arch          = eval_modules_primary->arch;
    ctx->reg_space         = d_eval_space_from_ctrl_entity(thread);
    ctx->reg_unwind_count  = unwind_count;
    ctx->module_base       = push_array(scratch.arena, U64, 1);
    ctx->module_base[0]    = module->vaddr_range.min;
    ctx->tls_base          = push_array(scratch.arena, U64, 1);
    ctx->tls_base[0]       = d_query_cached_tls_base_vaddr_from_process_root_rip(process, tls_root_vaddr, rip_vaddr);
  }
  e_select_interpret_ctx(interpret_ctx);
  
  //////////////////////////////
  //- rjf: build eval visualization view rule table
  //
  EV_ViewRuleInfoTable *view_rule_info_table = push_array(scratch.arena, EV_ViewRuleInfoTable, 1);
  {
    ev_view_rule_info_table_push_builtins(scratch.arena, view_rule_info_table);
  }
  ev_select_view_rule_info_table(view_rule_info_table);
  
  //////////////////////////////
  //- rjf: autosave if needed
  //
  {
    df_state->seconds_until_autosave -= df_state->frame_dt;
    if(df_state->seconds_until_autosave <= 0.f)
    {
      df_cmd(DF_CmdKind_WriteUserData);
      df_cmd(DF_CmdKind_WriteProjectData);
      df_state->seconds_until_autosave = 5.f;
    }
  }
  
  //////////////////////////////
  //- rjf: process top-level graphical commands
  //
  B32 panel_reset_done = 0;
  {
    for(DF_Cmd *cmd = 0; df_next_cmd(&cmd);) DF_RegsScope()
    {
      // rjf: unpack command
      DF_CmdKind kind = df_cmd_kind_from_string(cmd->name);
      df_regs_copy_contents(df_frame_arena(), df_regs(), cmd->regs);
      
      // rjf: request frame
      df_request_frame();
      
      // rjf: process command
      Dir2 split_dir = Dir2_Invalid;
      DF_Panel *split_panel = &df_nil_panel;
      U64 panel_sib_off = 0;
      U64 panel_child_off = 0;
      Vec2S32 panel_change_dir = {0};
      switch(kind)
      {
        //- rjf: default cases
        default:
        {
          // rjf: try to run engine command
          if(D_CmdKind_Null < (D_CmdKind)kind && (D_CmdKind)kind < D_CmdKind_COUNT)
          {
            D_CmdParams params = {0};
            params.machine       = df_regs()->machine;
            params.thread        = df_regs()->thread;
            // TODO(rjf): @msgs params.entity        = ???;
            // TODO(rjf): @msgs params.processes     = ???;
            params.file_path     = df_regs()->file_path;
            params.cursor        = df_regs()->cursor;
            params.vaddr         = df_regs()->vaddr;
            params.prefer_disasm = df_regs()->prefer_disasm;
            params.pid           = df_regs()->pid;
            // TODO(rjf): @msgs params.targets       = ???;
            // TODO(rjf): @msgs
#if 0
            params.window = df_regs()->window;
            params.panel  = df_regs()->panel;
            params.dest_panel = df_regs()->dst_panel;
            params.prev_view = df_regs()->prev_view;
            params.view = df_regs()->view;
            params.entity = df_regs()->entity;
            params.entity_list = df_regs()->entity_list;
            params.string = df_regs()->string;
            params.file_path = df_regs()->file_path;
            params.text_point = df_regs()->cursor;
            params.params_tree = df_regs()->params_tree;
            params.vaddr = df_regs()->vaddr;
            params.voff = df_regs()->voff;
            params.id = df_regs()->pid;
            params.prefer_dasm = df_regs()->prefer_disasm;
            params.force_confirm = df_regs()->force_confirm;
            params.dir2 = df_regs()->dir2;
            params.unwind_index = df_regs()->unwind_count;
            params.inline_depth = df_regs()->inline_depth;
#endif
            d_push_cmd((D_CmdKind)kind, &params);
          }
          
          // rjf: try to open tabs for "view driver" commands
          DF_ViewSpec *view_spec = df_view_spec_from_string(cmd->name);
          if(view_spec != &df_nil_view_spec)
          {
            df_cmd(DF_CmdKind_OpenTab, .string = cmd->name);
          }
        }break;
        
        //- rjf: command fast path
        case DF_CmdKind_RunCommand:
        {
          DF_CmdKindInfo *info = df_cmd_kind_info_from_string(cmd->regs->string);
          
          // rjf: command simply executes - just no-op in this layer
          if(!(info->query.flags & DF_QueryFlag_Required))
          {
            df_push_cmd(cmd->regs->string, df_regs());
          }
          
          // rjf: command has required query -> prep query
          else
          {
            DF_Window *window = df_window_from_handle(df_regs()->window);
            if(window != 0)
            {
              arena_clear(window->query_cmd_arena);
              window->query_cmd_name = push_str8_copy(window->query_cmd_arena, cmd->regs->string);
              window->query_cmd_regs = df_regs_copy(window->query_cmd_arena, df_regs());
              MemoryZeroArray(window->query_cmd_regs_mask);
              window->query_view_selected = 1;
            }
          }
        }break;
        
        //- rjf: exiting
        case DF_CmdKind_Exit:
        {
          // rjf: if control processes are live, but this is not force-confirmed, then
          // get confirmation from user
          CTRL_EntityList processes = ctrl_entity_list_from_kind(d_state->ctrl_entity_store, CTRL_EntityKind_Process);
          UI_Key key = ui_key_from_string(ui_key_zero(), str8_lit("lossy_exit_confirmation"));
          if(processes.count != 0 && !df_regs()->force_confirm && !ui_key_match(df_state->confirm_key, key))
          {
            df_state->confirm_key = key;
            df_state->confirm_active = 1;
            arena_clear(df_state->confirm_arena);
            MemoryZeroStruct(&df_state->confirm_cmds);
            df_state->confirm_title = push_str8f(df_state->confirm_arena, "Are you sure you want to exit?");
            df_state->confirm_desc = push_str8f(df_state->confirm_arena, "The debugger is still attached to %slive process%s.",
                                                processes.count == 1 ? "a " : "",
                                                processes.count == 1 ? ""   : "es");
            DF_Regs *regs = df_regs_copy(df_frame_arena(), df_regs());
            regs->force_confirm = 1;
            df_cmd_list_push_new(df_state->confirm_arena, &df_state->confirm_cmds, df_cmd_kind_info_table[DF_CmdKind_Exit].string, regs);
          }
          
          // rjf: otherwise, actually exit
          else
          {
            df_cmd(DF_CmdKind_WriteUserData);
            df_cmd(DF_CmdKind_WriteProjectData);
            df_state->quit = 1;
          }
        }break;
        
        //- rjf: windows
        case DF_CmdKind_OpenWindow:
        {
          DF_Window *originating_window = df_window_from_handle(df_regs()->window);
          if(originating_window == 0)
          {
            originating_window = df_state->first_window;
          }
          OS_Handle preferred_monitor = {0};
          DF_Window *new_ws = df_window_open(v2f32(1280, 720), preferred_monitor, D_CfgSrc_User);
          if(originating_window)
          {
            MemoryCopy(new_ws->setting_vals, originating_window->setting_vals, sizeof(DF_SettingVal)*DF_SettingCode_COUNT);
          }
        }break;
        case DF_CmdKind_CloseWindow:
        {
          DF_Window *ws = df_window_from_handle(df_regs()->window);
          if(ws != 0)
          {
            // rjf: is this the last window? -> exit
            if(df_state->first_window == df_state->last_window && df_state->first_window == ws)
            {
              df_cmd(DF_CmdKind_Exit);
            }
            
            // rjf: not the last window? -> just release this window
            else
            {
              // NOTE(rjf): we need to explicitly release all panel views, because views
              // are a global concept and otherwise would leak.
              for(DF_Panel *panel = ws->root_panel; !df_panel_is_nil(panel); panel = df_panel_rec_df_pre(panel).next)
              {
                df_panel_release_all_views(panel);
              }
              
              ui_state_release(ws->ui);
              DLLRemove(df_state->first_window, df_state->last_window, ws);
              r_window_unequip(ws->os, ws->r);
              os_window_close(ws->os);
              arena_release(ws->query_cmd_arena);
              arena_release(ws->code_ctx_menu_arena);
              arena_release(ws->hover_eval_arena);
              arena_release(ws->autocomp_lister_params_arena);
              arena_release(ws->arena);
              SLLStackPush(df_state->free_window, ws);
              ws->gen += 1;
            }
          }
        }break;
        case DF_CmdKind_ToggleFullscreen:
        {
          DF_Window *window = df_window_from_handle(df_regs()->window);
          if(window != 0)
          {
            os_window_set_fullscreen(window->os, !os_window_is_fullscreen(window->os));
          }
        }break;
        
        //- rjf: confirmations
        case DF_CmdKind_ConfirmAccept:
        {
          df_state->confirm_active = 0;
          df_state->confirm_key = ui_key_zero();
          for(DF_CmdNode *n = df_state->confirm_cmds.first; n != 0; n = n->next)
          {
            df_push_cmd(n->cmd.name, n->cmd.regs);
          }
        }break;
        case DF_CmdKind_ConfirmCancel:
        {
          df_state->confirm_active = 0;
          df_state->confirm_key = ui_key_zero();
        }break;
        
        //- rjf: config path saving/loading/applying
        case DF_CmdKind_OpenRecentProject:
        {
          DF_Entity *entity = df_entity_from_handle(df_regs()->entity);
          if(entity->kind == DF_EntityKind_RecentProject)
          {
            df_cmd(DF_CmdKind_OpenProject, .file_path = entity->string);
          }
        }break;
        case DF_CmdKind_OpenUser:
        case DF_CmdKind_OpenProject:
        {
          B32 load_cfg[D_CfgSrc_COUNT] = {0};
          for(D_CfgSrc src = (D_CfgSrc)0; src < D_CfgSrc_COUNT; src = (D_CfgSrc)(src+1))
          {
            load_cfg[src] = (kind == d_cfg_src_load_cmd_kind_table[src]);
          }
          
          //- rjf: normalize path
          String8 new_path = path_normalized_from_string(scratch.arena, df_regs()->file_path);
          
          //- rjf: path -> data
          FileProperties props = {0};
          String8 data = {0};
          {
            OS_Handle file = os_file_open(OS_AccessFlag_ShareRead|OS_AccessFlag_Read, new_path);
            props = os_properties_from_file(file);
            data = os_string_from_file_range(scratch.arena, file, r1u64(0, props.size));
            os_file_close(file);
          }
          
          //- rjf: investigate file path/data
          B32 file_is_okay = 1;
          if(props.modified != 0 && data.size != 0 && !str8_match(str8_prefix(data, 9), str8_lit("// raddbg"), 0))
          {
            file_is_okay = 0;
          }
          
          //- rjf: set new config paths
          if(file_is_okay)
          {
            for(D_CfgSrc src = (D_CfgSrc)0; src < D_CfgSrc_COUNT; src = (D_CfgSrc)(src+1))
            {
              if(load_cfg[src])
              {
                arena_clear(df_state->cfg_path_arenas[src]);
                df_state->cfg_paths[src] = push_str8_copy(df_state->cfg_path_arenas[src], new_path);
              }
            }
          }
          
          //- rjf: get config file properties
          FileProperties cfg_props[D_CfgSrc_COUNT] = {0};
          if(file_is_okay)
          {
            for(D_CfgSrc src = (D_CfgSrc)0; src < D_CfgSrc_COUNT; src = (D_CfgSrc)(src+1))
            {
              String8 path = df_cfg_path_from_src(src);
              cfg_props[src] = os_properties_from_file_path(path);
            }
          }
          
          //- rjf: load files
          String8 cfg_data[D_CfgSrc_COUNT] = {0};
          U64 cfg_timestamps[D_CfgSrc_COUNT] = {0};
          if(file_is_okay)
          {
            for(D_CfgSrc src = (D_CfgSrc)0; src < D_CfgSrc_COUNT; src = (D_CfgSrc)(src+1))
            {
              String8 path = df_cfg_path_from_src(src);
              OS_Handle file = os_file_open(OS_AccessFlag_ShareRead|OS_AccessFlag_Read, path);
              FileProperties props = os_properties_from_file(file);
              String8 data = os_string_from_file_range(scratch.arena, file, r1u64(0, props.size));
              if(data.size != 0)
              {
                cfg_data[src] = data;
                cfg_timestamps[src] = props.modified;
              }
              os_file_close(file);
            }
          }
          
          //- rjf: determine if we need to save config
          B32 cfg_save[D_CfgSrc_COUNT] = {0};
          if(file_is_okay)
          {
            for(D_CfgSrc src = (D_CfgSrc)0; src < D_CfgSrc_COUNT; src = (D_CfgSrc)(src+1))
            {
              cfg_save[src] = (load_cfg[src] && cfg_props[src].created == 0);
            }
          }
          
          //- rjf: determine if we need to reload config
          B32 cfg_load[D_CfgSrc_COUNT] = {0};
          B32 cfg_load_any = 0;
          if(file_is_okay)
          {
            for(D_CfgSrc src = (D_CfgSrc)0; src < D_CfgSrc_COUNT; src = (D_CfgSrc)(src+1))
            {
              cfg_load[src] = (load_cfg[src] && ((cfg_save[src] == 0 && df_state->cfg_cached_timestamp[src] != cfg_timestamps[src]) || cfg_props[src].created == 0));
              cfg_load_any = cfg_load_any || cfg_load[src];
            }
          }
          
          //- rjf: load => build new config table
          if(cfg_load_any)
          {
            arena_clear(df_state->cfg_arena);
            MemoryZeroStruct(&df_state->cfg_table);
            for(D_CfgSrc src = (D_CfgSrc)0; src < D_CfgSrc_COUNT; src = (D_CfgSrc)(src+1))
            {
              d_cfg_table_push_unparsed_string(df_state->cfg_arena, &df_state->cfg_table, cfg_data[src], src);
            }
          }
          
          //- rjf: load => dispatch apply
          //
          // NOTE(rjf): must happen before `save`. we need to create a default before saving, which
          // occurs in the 'apply' path.
          //
          if(file_is_okay)
          {
            for(D_CfgSrc src = (D_CfgSrc)0; src < D_CfgSrc_COUNT; src = (D_CfgSrc)(src+1))
            {
              if(cfg_load[src])
              {
                DF_CmdKind cmd_kind = d_cfg_src_apply_cmd_kind_table[src];
                df_cmd(cmd_kind);
                df_state->cfg_cached_timestamp[src] = cfg_timestamps[src];
              }
            }
          }
          
          //- rjf: save => dispatch write
          if(file_is_okay)
          {
            for(D_CfgSrc src = (D_CfgSrc)0; src < D_CfgSrc_COUNT; src = (D_CfgSrc)(src+1))
            {
              if(cfg_save[src])
              {
                DF_CmdKind cmd_kind = d_cfg_src_write_cmd_kind_table[src];
                df_cmd(cmd_kind);
              }
            }
          }
          
          //- rjf: bad file -> alert user
          if(!file_is_okay)
          {
            log_user_errorf("\"%S\" appears to refer to an existing file which is not a RADDBG config file. This would overwrite the file.", new_path);
          }
        }break;
        
        //- rjf: loading/applying stateful config changes
        case DF_CmdKind_ApplyUserData:
        case DF_CmdKind_ApplyProjectData:
        {
          D_CfgTable *table = df_cfg_table();
          OS_HandleArray monitors = os_push_monitors_array(scratch.arena);
          
          //- rjf: get config source
          D_CfgSrc src = D_CfgSrc_User;
          for(D_CfgSrc s = (D_CfgSrc)0; s < D_CfgSrc_COUNT; s = (D_CfgSrc)(s+1))
          {
            if(kind == d_cfg_src_apply_cmd_kind_table[s])
            {
              src = s;
              break;
            }
          }
          
          //- rjf: get paths
          String8 cfg_path   = df_cfg_path_from_src(src);
          String8 cfg_folder = str8_chop_last_slash(cfg_path);
          
          //- rjf: keep track of recent projects
          if(src == D_CfgSrc_Project)
          {
            DF_EntityList recent_projects = d_query_cached_entity_list_with_kind(DF_EntityKind_RecentProject);
            DF_Entity *recent_project = &d_nil_entity;
            for(DF_EntityNode *n = recent_projects.first; n != 0; n = n->next)
            {
              if(path_match_normalized(cfg_path, n->entity->string))
              {
                recent_project = n->entity;
                break;
              }
            }
            if(df_entity_is_nil(recent_project))
            {
              recent_project = df_entity_alloc(df_entity_root(), DF_EntityKind_RecentProject);
              df_entity_equip_name(recent_project, path_normalized_from_string(scratch.arena, cfg_path));
              df_entity_equip_cfg_src(recent_project, D_CfgSrc_User);
            }
          }
          
          //- rjf: eliminate all existing entities which are derived from config
          {
            for(EachEnumVal(DF_EntityKind, k))
            {
              DF_EntityKindFlags k_flags = d_entity_kind_flags_table[k];
              if(k_flags & DF_EntityKindFlag_IsSerializedToConfig)
              {
                DF_EntityList entities = d_query_cached_entity_list_with_kind(k);
                for(DF_EntityNode *n = entities.first; n != 0; n = n->next)
                {
                  if(n->entity->cfg_src == src)
                  {
                    df_entity_mark_for_deletion(n->entity);
                  }
                }
              }
            }
          }
          
          //- rjf: apply all entities
          {
            for(EachEnumVal(DF_EntityKind, k))
            {
              DF_EntityKindFlags k_flags = d_entity_kind_flags_table[k];
              if(k_flags & DF_EntityKindFlag_IsSerializedToConfig)
              {
                D_CfgVal *k_val = d_cfg_val_from_string(table, d_entity_kind_name_lower_table[k]);
                for(D_CfgTree *k_tree = k_val->first;
                    k_tree != &d_nil_cfg_tree;
                    k_tree = k_tree->next)
                {
                  if(k_tree->source != src)
                  {
                    continue;
                  }
                  DF_Entity *entity = df_entity_alloc(df_entity_root(), k);
                  df_entity_equip_cfg_src(entity, k_tree->source);
                  
                  // rjf: iterate config tree
                  typedef struct Task Task;
                  struct Task
                  {
                    Task *next;
                    DF_Entity *entity;
                    MD_Node *n;
                  };
                  Task start_task = {0, entity, k_tree->root};
                  Task *first_task = &start_task;
                  Task *last_task = first_task;
                  for(Task *t = first_task; t != 0; t = t->next)
                  {
                    MD_Node *node = t->n;
                    for(MD_EachNode(child, node->first))
                    {
                      // rjf: standalone string literals under an entity -> name
                      if(child->flags & MD_NodeFlag_StringLiteral && child->first == &md_nil_node)
                      {
                        String8 string = d_cfg_raw_from_escaped_string(scratch.arena, child->string);
                        if(d_entity_kind_flags_table[t->entity->kind] & DF_EntityKindFlag_NameIsPath)
                        {
                          string = path_absolute_dst_from_relative_dst_src(scratch.arena, string, cfg_folder);
                        }
                        df_entity_equip_name(t->entity, string);
                      }
                      
                      // rjf: standalone string literals under an entity, with a numeric child -> name & text location
                      if(child->flags & MD_NodeFlag_StringLiteral && child->first->flags & MD_NodeFlag_Numeric && child->first->first == &md_nil_node)
                      {
                        String8 string = d_cfg_raw_from_escaped_string(scratch.arena, child->string);
                        if(d_entity_kind_flags_table[t->entity->kind] & DF_EntityKindFlag_NameIsPath)
                        {
                          string = path_absolute_dst_from_relative_dst_src(scratch.arena, string, cfg_folder);
                        }
                        df_entity_equip_name(t->entity, string);
                        S64 line = 0;
                        try_s64_from_str8_c_rules(child->first->string, &line);
                        TxtPt pt = txt_pt(line, 1);
                        df_entity_equip_txt_pt(t->entity, pt);
                      }
                      
                      // rjf: standalone hex literals under an entity -> vaddr
                      if(child->flags & MD_NodeFlag_Numeric && child->first == &md_nil_node && str8_match(str8_substr(child->string, r1u64(0, 2)), str8_lit("0x"), 0))
                      {
                        U64 vaddr = 0;
                        try_u64_from_str8_c_rules(child->string, &vaddr);
                        df_entity_equip_vaddr(t->entity, vaddr);
                      }
                      
                      // rjf: specifically named entity equipment
                      if((str8_match(child->string, str8_lit("name"), StringMatchFlag_CaseInsensitive) ||
                          str8_match(child->string, str8_lit("label"), StringMatchFlag_CaseInsensitive)) &&
                         child->first != &md_nil_node)
                      {
                        String8 string = d_cfg_raw_from_escaped_string(scratch.arena, child->first->string);
                        if(d_entity_kind_flags_table[t->entity->kind] & DF_EntityKindFlag_NameIsPath)
                        {
                          string = path_absolute_dst_from_relative_dst_src(scratch.arena, string, cfg_folder);
                        }
                        df_entity_equip_name(t->entity, string);
                      }
                      if((str8_match(child->string, str8_lit("active"), StringMatchFlag_CaseInsensitive) ||
                          str8_match(child->string, str8_lit("enabled"), StringMatchFlag_CaseInsensitive)) &&
                         child->first != &md_nil_node)
                      {
                        df_entity_equip_disabled(t->entity, !str8_match(child->first->string, str8_lit("1"), 0));
                      }
                      if(str8_match(child->string, str8_lit("disabled"), StringMatchFlag_CaseInsensitive) && child->first != &md_nil_node)
                      {
                        df_entity_equip_disabled(t->entity, str8_match(child->first->string, str8_lit("1"), 0));
                      }
                      if(str8_match(child->string, str8_lit("hsva"), StringMatchFlag_CaseInsensitive) && child->first != &md_nil_node)
                      {
                        Vec4F32 hsva = {0};
                        hsva.x = (F32)f64_from_str8(child->first->string);
                        hsva.y = (F32)f64_from_str8(child->first->next->string);
                        hsva.z = (F32)f64_from_str8(child->first->next->next->string);
                        hsva.w = (F32)f64_from_str8(child->first->next->next->next->string);
                        df_entity_equip_color_hsva(t->entity, hsva);
                      }
                      if(str8_match(child->string, str8_lit("color"), StringMatchFlag_CaseInsensitive) && child->first != &md_nil_node)
                      {
                        Vec4F32 rgba = rgba_from_hex_string_4f32(child->first->string);
                        Vec4F32 hsva = hsva_from_rgba(rgba);
                        df_entity_equip_color_hsva(t->entity, hsva);
                      }
                      if(str8_match(child->string, str8_lit("line"), StringMatchFlag_CaseInsensitive) && child->first != &md_nil_node)
                      {
                        S64 line = 0;
                        try_s64_from_str8_c_rules(child->first->string, &line);
                        TxtPt pt = txt_pt(line, 1);
                        df_entity_equip_txt_pt(t->entity, pt);
                      }
                      if((str8_match(child->string, str8_lit("vaddr"), StringMatchFlag_CaseInsensitive) ||
                          str8_match(child->string, str8_lit("addr"), StringMatchFlag_CaseInsensitive)) &&
                         child->first != &md_nil_node)
                      {
                        U64 vaddr = 0;
                        try_u64_from_str8_c_rules(child->first->string, &vaddr);
                        df_entity_equip_vaddr(t->entity, vaddr);
                      }
                      
                      // rjf: sub-entity -> create new task
                      DF_EntityKind sub_entity_kind = DF_EntityKind_Nil;
                      for(EachEnumVal(DF_EntityKind, k2))
                      {
                        if(child->flags & MD_NodeFlag_Identifier && child->first != &md_nil_node &&
                           (str8_match(child->string, d_entity_kind_name_lower_table[k2], StringMatchFlag_CaseInsensitive) ||
                            (k2 == DF_EntityKind_Executable && str8_match(child->string, str8_lit("exe"), StringMatchFlag_CaseInsensitive))))
                        {
                          Task *task = push_array(scratch.arena, Task, 1);
                          task->next = t->next;
                          task->entity = df_entity_alloc(t->entity, k2);
                          task->n = child;
                          t->next = task;
                          break;
                        }
                      }
                    }
                  }
                }
              }
            }
          }
          
          //- rjf: apply exception code filters
          D_CfgVal *filter_tables = d_cfg_val_from_string(table, str8_lit("exception_code_filters"));
          for(D_CfgTree *table = filter_tables->first;
              table != &d_nil_cfg_tree;
              table = table->next)
          {
            for(MD_EachNode(rule, table->root->first))
            {
              String8 name = rule->string;
              String8 val_string = rule->first->string;
              U64 val = 0;
              if(try_u64_from_str8_c_rules(val_string, &val))
              {
                CTRL_ExceptionCodeKind kind = CTRL_ExceptionCodeKind_Null;
                for(CTRL_ExceptionCodeKind k = (CTRL_ExceptionCodeKind)(CTRL_ExceptionCodeKind_Null+1);
                    k < CTRL_ExceptionCodeKind_COUNT;
                    k = (CTRL_ExceptionCodeKind)(k+1))
                {
                  if(str8_match(name, ctrl_exception_code_kind_lowercase_code_string_table[k], 0))
                  {
                    kind = k;
                    break;
                  }
                }
                if(kind != CTRL_ExceptionCodeKind_Null)
                {
                  if(val)
                  {
                    df_state->ctrl_exception_code_filters[kind/64] |= (1ull<<(kind%64));
                  }
                  else
                  {
                    df_state->ctrl_exception_code_filters[kind/64] &= ~(1ull<<(kind%64));
                  }
                }
              }
            }
          }
          
          //- rjf: eliminate all windows
          for(DF_Window *window = df_state->first_window; window != 0; window = window->next)
          {
            if(window->cfg_src != src)
            {
              continue;
            }
            df_cmd(DF_CmdKind_CloseWindow, .window = df_handle_from_window(window));
          }
          
          //- rjf: apply fonts
          {
            FNT_Tag defaults[DF_FontSlot_COUNT] =
            {
              fnt_tag_from_static_data_string(&df_g_default_main_font_bytes),
              fnt_tag_from_static_data_string(&df_g_default_code_font_bytes),
              fnt_tag_from_static_data_string(&df_g_icon_font_bytes),
            };
            MemoryZeroArray(df_state->cfg_font_tags);
            {
              D_CfgVal *code_font_val = d_cfg_val_from_string(table, str8_lit("code_font"));
              D_CfgVal *main_font_val = d_cfg_val_from_string(table, str8_lit("main_font"));
              MD_Node *code_font_node = code_font_val->last->root;
              MD_Node *main_font_node = main_font_val->last->root;
              String8 code_font_relative_path = code_font_node->first->string;
              String8 main_font_relative_path = main_font_node->first->string;
              if(!md_node_is_nil(code_font_node))
              {
                arena_clear(df_state->cfg_code_font_path_arena);
                df_state->cfg_code_font_path = push_str8_copy(df_state->cfg_code_font_path_arena, code_font_relative_path);
              }
              if(!md_node_is_nil(main_font_node))
              {
                arena_clear(df_state->cfg_main_font_path_arena);
                df_state->cfg_main_font_path = push_str8_copy(df_state->cfg_main_font_path_arena, main_font_relative_path);
              }
              String8 code_font_path = path_absolute_dst_from_relative_dst_src(scratch.arena, code_font_relative_path, cfg_folder);
              String8 main_font_path = path_absolute_dst_from_relative_dst_src(scratch.arena, main_font_relative_path, cfg_folder);
              if(os_file_path_exists(code_font_path) && !md_node_is_nil(code_font_node) && code_font_relative_path.size != 0)
              {
                df_state->cfg_font_tags[DF_FontSlot_Code] = fnt_tag_from_path(code_font_path);
              }
              if(os_file_path_exists(main_font_path) && !md_node_is_nil(main_font_node) && main_font_relative_path.size != 0)
              {
                df_state->cfg_font_tags[DF_FontSlot_Main] = fnt_tag_from_path(main_font_path);
              }
            }
            for(DF_FontSlot slot = (DF_FontSlot)0; slot < DF_FontSlot_COUNT; slot = (DF_FontSlot)(slot+1))
            {
              if(fnt_tag_match(fnt_tag_zero(), df_state->cfg_font_tags[slot]))
              {
                df_state->cfg_font_tags[slot] = defaults[slot];
              }
            }
          }
          
          //- rjf: build windows & panel layouts
          D_CfgVal *windows = d_cfg_val_from_string(table, str8_lit("window"));
          for(D_CfgTree *window_tree = windows->first;
              window_tree != &d_nil_cfg_tree;
              window_tree = window_tree->next)
          {
            // rjf: skip wrong source
            if(window_tree->source != src)
            {
              continue;
            }
            
            // rjf: grab metadata
            B32 is_fullscreen = 0;
            B32 is_maximized = 0;
            Axis2 top_level_split_axis = Axis2_X;
            OS_Handle preferred_monitor = os_primary_monitor();
            Vec2F32 size = {0};
            F32 dpi = 0.f;
            DF_SettingVal setting_vals[DF_SettingCode_COUNT] = {0};
            {
              for(MD_EachNode(n, window_tree->root->first))
              {
                if(n->flags & MD_NodeFlag_Identifier &&
                   md_node_is_nil(n->first) &&
                   str8_match(n->string, str8_lit("split_x"), StringMatchFlag_CaseInsensitive))
                {
                  top_level_split_axis = Axis2_X;
                }
                if(n->flags & MD_NodeFlag_Identifier &&
                   md_node_is_nil(n->first) &&
                   str8_match(n->string, str8_lit("split_y"), StringMatchFlag_CaseInsensitive))
                {
                  top_level_split_axis = Axis2_Y;
                }
                if(n->flags & MD_NodeFlag_Identifier &&
                   md_node_is_nil(n->first) &&
                   str8_match(n->string, str8_lit("fullscreen"), StringMatchFlag_CaseInsensitive))
                {
                  is_fullscreen = 1;
                }
                if(n->flags & MD_NodeFlag_Identifier &&
                   md_node_is_nil(n->first) &&
                   str8_match(n->string, str8_lit("maximized"), StringMatchFlag_CaseInsensitive))
                {
                  is_maximized = 1;
                }
              }
              MD_Node *monitor_node = md_child_from_string(window_tree->root, str8_lit("monitor"), 0);
              String8 preferred_monitor_name = monitor_node->first->string;
              for(U64 idx = 0; idx < monitors.count; idx += 1)
              {
                String8 monitor_name = os_name_from_monitor(scratch.arena, monitors.v[idx]);
                if(str8_match(monitor_name, preferred_monitor_name, StringMatchFlag_CaseInsensitive))
                {
                  preferred_monitor = monitors.v[idx];
                  break;
                }
              }
              Vec2F32 preferred_monitor_size = os_dim_from_monitor(preferred_monitor);
              MD_Node *size_node = md_child_from_string(window_tree->root, str8_lit("size"), 0);
              {
                String8 x_string = size_node->first->string;
                String8 y_string = size_node->first->next->string;
                U64 x_u64 = 0;
                U64 y_u64 = 0;
                if(!try_u64_from_str8_c_rules(x_string, &x_u64))
                {
                  x_u64 = (U64)(preferred_monitor_size.x*2/3);
                }
                if(!try_u64_from_str8_c_rules(y_string, &y_u64))
                {
                  y_u64 = (U64)(preferred_monitor_size.y*2/3);
                }
                size.x = (F32)x_u64;
                size.y = (F32)y_u64;
              }
              MD_Node *dpi_node = md_child_from_string(window_tree->root, str8_lit("dpi"), 0);
              String8 dpi_string = md_string_from_children(scratch.arena, dpi_node);
              dpi = f64_from_str8(dpi_string);
              for(EachEnumVal(DF_SettingCode, code))
              {
                MD_Node *code_node = md_child_from_string(window_tree->root, df_g_setting_code_lower_string_table[code], 0);
                if(!md_node_is_nil(code_node))
                {
                  S64 val_s64 = 0;
                  try_s64_from_str8_c_rules(code_node->first->string, &val_s64);
                  setting_vals[code].set = 1;
                  setting_vals[code].s32 = (S32)val_s64;
                  setting_vals[code].s32 = clamp_1s32(df_g_setting_code_s32_range_table[code], setting_vals[code].s32);
                }
              }
            }
            
            // rjf: open window
            DF_Window *ws = df_window_open(size, preferred_monitor, window_tree->source);
            if(dpi != 0.f) { ws->last_dpi = dpi; }
            for(EachEnumVal(DF_SettingCode, code))
            {
              if(setting_vals[code].set == 0 && df_g_setting_code_default_is_per_window_table[code])
              {
                setting_vals[code] = df_g_setting_code_default_val_table[code];
              }
            }
            MemoryCopy(ws->setting_vals, setting_vals, sizeof(setting_vals[0])*ArrayCount(setting_vals));
            
            // rjf: build panel tree
            MD_Node *panel_tree = md_child_from_string(window_tree->root, str8_lit("panels"), 0);
            DF_Panel *panel_parent = ws->root_panel;
            panel_parent->split_axis = top_level_split_axis;
            MD_NodeRec rec = {0};
            for(MD_Node *n = panel_tree, *next = &md_nil_node;
                !md_node_is_nil(n);
                n = next)
            {
              // rjf: assume we're just moving to the next one initially...
              next = n->next;
              
              // rjf: grab root panel
              DF_Panel *panel = &df_nil_panel;
              if(n == panel_tree)
              {
                panel = ws->root_panel;
                panel->pct_of_parent = 1.f;
              }
              
              // rjf: allocate & insert non-root panels - these will have a numeric string, determining
              // pct of parent
              if(n->flags & MD_NodeFlag_Numeric)
              {
                panel = df_panel_alloc(ws);
                df_panel_insert(panel_parent, panel_parent->last, panel);
                panel->split_axis = axis2_flip(panel_parent->split_axis);
                panel->pct_of_parent = (F32)f64_from_str8(n->string);
              }
              
              // rjf: do general per-panel work
              if(!df_panel_is_nil(panel))
              {
                // rjf: determine if this panel has panel children
                B32 has_panel_children = 0;
                for(MD_EachNode(child, n->first))
                {
                  if(child->flags & MD_NodeFlag_Numeric)
                  {
                    has_panel_children = 1;
                    break;
                  }
                }
                
                // rjf: apply panel options
                for(MD_EachNode(op, n->first))
                {
                  if(md_node_is_nil(op->first) && str8_match(op->string, str8_lit("tabs_on_bottom"), 0))
                  {
                    panel->tab_side = Side_Max;
                  }
                }
                
                // rjf: apply panel views/tabs/commands
                DF_View *selected_view = &df_nil_view;
                for(MD_EachNode(op, n->first))
                {
                  DF_ViewSpec *view_spec = df_view_spec_from_string(op->string);
                  if(view_spec == &df_nil_view_spec || has_panel_children != 0)
                  {
                    continue;
                  }
                  
                  // rjf: allocate view & apply view-specific parameterizations
                  DF_View *view = &df_nil_view;
                  B32 view_is_selected = 0;
                  DF_ViewSpecFlags view_spec_flags = view_spec->info.flags;
                  if(view_spec_flags & DF_ViewSpecFlag_CanSerialize)
                  {
                    // rjf: allocate view
                    view = df_view_alloc();
                    
                    // rjf: check if this view is selected
                    view_is_selected = !md_node_is_nil(md_child_from_string(op, str8_lit("selected"), 0));
                    
                    // rjf: read project path
                    String8 project_path = str8_lit("");
                    {
                      MD_Node *project_node = md_child_from_string(op, str8_lit("project"), 0);
                      if(!md_node_is_nil(project_node))
                      {
                        project_path = path_absolute_dst_from_relative_dst_src(scratch.arena, project_node->first->string, cfg_folder);
                      }
                    }
                    
                    // rjf: read view query string
                    String8 view_query = str8_lit("");
                    {
                      String8 escaped_query = md_child_from_string(op, str8_lit("query"), 0)->first->string;
                      view_query = d_cfg_raw_from_escaped_string(scratch.arena, escaped_query);
                    }
                    
                    // rjf: convert file queries from relative to absolute
                    {
                      String8 query_file_path = d_file_path_from_eval_string(scratch.arena, view_query);
                      if(query_file_path.size != 0)
                      {
                        query_file_path = path_absolute_dst_from_relative_dst_src(scratch.arena, query_file_path, cfg_folder);
                        view_query = push_str8f(scratch.arena, "file:\"%S\"", query_file_path);
                      }
                    }
                    
                    // rjf: set up view
                    df_view_equip_spec(view, view_spec, view_query, op);
                    if(project_path.size != 0)
                    {
                      arena_clear(view->project_path_arena);
                      view->project_path = push_str8_copy(view->project_path_arena, project_path);
                    }
                  }
                  
                  // rjf: insert
                  if(!df_view_is_nil(view))
                  {
                    df_panel_insert_tab_view(panel, panel->last_tab_view, view);
                    if(view_is_selected)
                    {
                      selected_view = view;
                    }
                  }
                }
                
                // rjf: select selected view
                if(!df_view_is_nil(selected_view))
                {
                  panel->selected_tab_view = df_handle_from_view(selected_view);
                }
                
                // rjf: recurse from this panel
                if(has_panel_children)
                {
                  next = n->first;
                  panel_parent = panel;
                }
                else for(MD_Node *p = n;
                         p != &md_nil_node && p != panel_tree;
                         p = p->parent, panel_parent = panel_parent->parent)
                {
                  if(p->next != &md_nil_node)
                  {
                    next = p->next;
                    break;
                  }
                }
              }
            }
            
            // rjf: initiate fullscreen
            if(is_fullscreen)
            {
              os_window_set_fullscreen(ws->os, 1);
            }
            
            // rjf: initiate maximize
            if(is_maximized)
            {
              os_window_set_maximized(ws->os, 1);
            }
            
            // rjf: focus the biggest panel
            {
              DF_Panel *best_leaf_panel = &df_nil_panel;
              F32 best_leaf_panel_area = 0;
              Rng2F32 root_rect = r2f32p(0, 0, 1000, 1000); // NOTE(rjf): we can assume any size - just need proportions.
              for(DF_Panel *panel = ws->root_panel; !df_panel_is_nil(panel); panel = df_panel_rec_df_pre(panel).next)
              {
                if(df_panel_is_nil(panel->first))
                {
                  Rng2F32 rect = df_target_rect_from_panel(root_rect, ws->root_panel, panel);
                  Vec2F32 dim = dim_2f32(rect);
                  F32 area = dim.x*dim.y;
                  if(best_leaf_panel_area == 0 || area > best_leaf_panel_area)
                  {
                    best_leaf_panel_area = area;
                    best_leaf_panel = panel;
                  }
                }
              }
              ws->focused_panel = best_leaf_panel;
            }
          }
          
          //- rjf: apply keybindings
          if(src == D_CfgSrc_User)
          {
            df_clear_bindings();
          }
          D_CfgVal *keybindings = d_cfg_val_from_string(table, str8_lit("keybindings"));
          for(D_CfgTree *keybinding_set = keybindings->first;
              keybinding_set != &d_nil_cfg_tree;
              keybinding_set = keybinding_set->next)
          {
            for(MD_EachNode(keybind, keybinding_set->root->first))
            {
              String8 cmd_name = {0};
              OS_Key key = OS_Key_Null;
              MD_Node *ctrl_node = &md_nil_node;
              MD_Node *shift_node = &md_nil_node;
              MD_Node *alt_node = &md_nil_node;
              for(MD_EachNode(child, keybind->first))
              {
                if(str8_match(child->string, str8_lit("ctrl"), 0))
                {
                  ctrl_node = child;
                }
                else if(str8_match(child->string, str8_lit("shift"), 0))
                {
                  shift_node = child;
                }
                else if(str8_match(child->string, str8_lit("alt"), 0))
                {
                  alt_node = child;
                }
                else
                {
                  OS_Key k = df_os_key_from_cfg_string(child->string);
                  if(k != OS_Key_Null)
                  {
                    key = k;
                  }
                  else
                  {
                    cmd_name = child->string;
                    for(U64 idx = 0; idx < ArrayCount(df_g_binding_version_remap_old_name_table); idx += 1)
                    {
                      if(str8_match(df_g_binding_version_remap_old_name_table[idx], child->string, StringMatchFlag_CaseInsensitive))
                      {
                        String8 new_name = df_g_binding_version_remap_new_name_table[idx];
                        cmd_name = new_name;
                      }
                    }
                  }
                }
              }
              if(cmd_name.size != 0 && key != OS_Key_Null)
              {
                OS_EventFlags flags = 0;
                if(!md_node_is_nil(ctrl_node))  { flags |= OS_EventFlag_Ctrl; }
                if(!md_node_is_nil(shift_node)) { flags |= OS_EventFlag_Shift; }
                if(!md_node_is_nil(alt_node))   { flags |= OS_EventFlag_Alt; }
                DF_Binding binding = {key, flags};
                df_bind_name(cmd_name, binding);
              }
            }
          }
          
          //- rjf: reset theme to default
          MemoryCopy(df_state->cfg_theme_target.colors, df_g_theme_preset_colors__default_dark, sizeof(df_g_theme_preset_colors__default_dark));
          MemoryCopy(df_state->cfg_theme.colors, df_g_theme_preset_colors__default_dark, sizeof(df_g_theme_preset_colors__default_dark));
          
          //- rjf: apply theme presets
          D_CfgVal *color_preset = d_cfg_val_from_string(table, str8_lit("color_preset"));
          B32 preset_applied = 0;
          if(color_preset != &d_nil_cfg_val)
          {
            String8 color_preset_name = color_preset->last->root->first->string;
            DF_ThemePreset preset = (DF_ThemePreset)0;
            B32 found_preset = 0;
            for(DF_ThemePreset p = (DF_ThemePreset)0; p < DF_ThemePreset_COUNT; p = (DF_ThemePreset)(p+1))
            {
              if(str8_match(color_preset_name, df_g_theme_preset_code_string_table[p], StringMatchFlag_CaseInsensitive))
              {
                found_preset = 1;
                preset = p;
                break;
              }
            }
            if(found_preset)
            {
              preset_applied = 1;
              MemoryCopy(df_state->cfg_theme_target.colors, df_g_theme_preset_colors_table[preset], sizeof(df_g_theme_preset_colors__default_dark));
              MemoryCopy(df_state->cfg_theme.colors, df_g_theme_preset_colors_table[preset], sizeof(df_g_theme_preset_colors__default_dark));
            }
          }
          
          //- rjf: apply individual theme colors
          B8 theme_color_hit[DF_ThemeColor_COUNT] = {0};
          D_CfgVal *colors = d_cfg_val_from_string(table, str8_lit("colors"));
          for(D_CfgTree *colors_set = colors->first;
              colors_set != &d_nil_cfg_tree;
              colors_set = colors_set->next)
          {
            for(MD_EachNode(color, colors_set->root->first))
            {
              String8 saved_color_name = color->string;
              String8List candidate_color_names = {0};
              str8_list_push(scratch.arena, &candidate_color_names, saved_color_name);
              for(U64 idx = 0; idx < ArrayCount(df_g_theme_color_version_remap_old_name_table); idx += 1)
              {
                if(str8_match(df_g_theme_color_version_remap_old_name_table[idx], saved_color_name, StringMatchFlag_CaseInsensitive))
                {
                  str8_list_push(scratch.arena, &candidate_color_names, df_g_theme_color_version_remap_new_name_table[idx]);
                }
              }
              for(String8Node *name_n = candidate_color_names.first; name_n != 0; name_n = name_n->next)
              {
                String8 name = name_n->string;
                DF_ThemeColor color_code = DF_ThemeColor_Null;
                for(DF_ThemeColor c = DF_ThemeColor_Null; c < DF_ThemeColor_COUNT; c = (DF_ThemeColor)(c+1))
                {
                  if(str8_match(df_g_theme_color_cfg_string_table[c], name, StringMatchFlag_CaseInsensitive))
                  {
                    color_code = c;
                    break;
                  }
                }
                if(color_code != DF_ThemeColor_Null)
                {
                  theme_color_hit[color_code] = 1;
                  MD_Node *hex_cfg = color->first;
                  String8 hex_string = hex_cfg->string;
                  U64 hex_val = 0;
                  try_u64_from_str8_c_rules(hex_string, &hex_val);
                  Vec4F32 color_rgba = rgba_from_u32((U32)hex_val);
                  df_state->cfg_theme_target.colors[color_code] = color_rgba;
                  if(df_state->frame_index <= 2)
                  {
                    df_state->cfg_theme.colors[color_code] = color_rgba;
                  }
                }
              }
            }
          }
          
          //- rjf: no preset -> autofill all missing colors from the preset with the most similar background
          if(!preset_applied)
          {
            DF_ThemePreset closest_preset = DF_ThemePreset_DefaultDark;
            F32 closest_preset_bg_distance = 100000000;
            for(DF_ThemePreset p = (DF_ThemePreset)0; p < DF_ThemePreset_COUNT; p = (DF_ThemePreset)(p+1))
            {
              Vec4F32 cfg_bg = df_state->cfg_theme_target.colors[DF_ThemeColor_BaseBackground];
              Vec4F32 pre_bg = df_g_theme_preset_colors_table[p][DF_ThemeColor_BaseBackground];
              Vec4F32 diff = sub_4f32(cfg_bg, pre_bg);
              Vec3F32 diff3 = diff.xyz;
              F32 distance = length_3f32(diff3);
              if(distance < closest_preset_bg_distance)
              {
                closest_preset = p;
                closest_preset_bg_distance = distance;
              }
            }
            for(DF_ThemeColor c = (DF_ThemeColor)(DF_ThemeColor_Null+1);
                c < DF_ThemeColor_COUNT;
                c = (DF_ThemeColor)(c+1))
            {
              if(!theme_color_hit[c])
              {
                df_state->cfg_theme_target.colors[c] = df_state->cfg_theme.colors[c] = df_g_theme_preset_colors_table[closest_preset][c];
              }
            }
          }
          
          //- rjf: if theme colors are all zeroes, then set to default - config appears busted
          {
            B32 all_colors_are_zero = 1;
            Vec4F32 zero_color = {0};
            for(DF_ThemeColor c = (DF_ThemeColor)(DF_ThemeColor_Null+1); c < DF_ThemeColor_COUNT; c = (DF_ThemeColor)(c+1))
            {
              if(!MemoryMatchStruct(&df_state->cfg_theme_target.colors[c], &zero_color))
              {
                all_colors_are_zero = 0;
                break;
              }
            }
            if(all_colors_are_zero)
            {
              MemoryCopy(df_state->cfg_theme_target.colors, df_g_theme_preset_colors__default_dark, sizeof(df_g_theme_preset_colors__default_dark));
              MemoryCopy(df_state->cfg_theme.colors, df_g_theme_preset_colors__default_dark, sizeof(df_g_theme_preset_colors__default_dark));
            }
          }
          
          //- rjf: apply settings
          B8 setting_codes_hit[DF_SettingCode_COUNT] = {0};
          MemoryZero(&df_state->cfg_setting_vals[src][0], sizeof(DF_SettingVal)*DF_SettingCode_COUNT);
          for(EachEnumVal(DF_SettingCode, code))
          {
            String8 name = df_g_setting_code_lower_string_table[code];
            D_CfgVal *code_cfg_val = d_cfg_val_from_string(table, name);
            D_CfgTree *code_tree = code_cfg_val->last;
            if(code_tree->source == src)
            {
              MD_Node *val_node = code_tree->root->first;
              S64 val = 0;
              if(try_s64_from_str8_c_rules(val_node->string, &val))
              {
                df_state->cfg_setting_vals[src][code].set = 1;
                df_state->cfg_setting_vals[src][code].s32 = (S32)val;
              }
              setting_codes_hit[code] = !md_node_is_nil(val_node);
            }
          }
          
          //- rjf: if config applied 0 settings, we need to do some sensible default
          if(src == D_CfgSrc_User)
          {
            for(EachEnumVal(DF_SettingCode, code))
            {
              if(!setting_codes_hit[code])
              {
                df_state->cfg_setting_vals[src][code] = df_g_setting_code_default_val_table[code];
              }
            }
          }
          
          //- rjf: if config opened 0 windows, we need to do some sensible default
          if(src == D_CfgSrc_User && windows->first == &d_nil_cfg_tree)
          {
            OS_Handle preferred_monitor = os_primary_monitor();
            Vec2F32 monitor_dim = os_dim_from_monitor(preferred_monitor);
            Vec2F32 window_dim = v2f32(monitor_dim.x*4/5, monitor_dim.y*4/5);
            DF_Window *ws = df_window_open(window_dim, preferred_monitor, D_CfgSrc_User);
            if(monitor_dim.x < 1920)
            {
              df_cmd(DF_CmdKind_ResetToCompactPanels);
            }
            else
            {
              df_cmd(DF_CmdKind_ResetToDefaultPanels);
            }
          }
          
          //- rjf: if config bound 0 keys, we need to do some sensible default
          if(src == D_CfgSrc_User && df_state->key_map_total_count == 0)
          {
            for(U64 idx = 0; idx < ArrayCount(df_g_default_binding_table); idx += 1)
            {
              DF_StringBindingPair *pair = &df_g_default_binding_table[idx];
              df_bind_name(pair->string, pair->binding);
            }
          }
          
          //- rjf: always ensure that the meta controls have bindings
          if(src == D_CfgSrc_User)
          {
            struct
            {
              String8 name;
              OS_Key fallback_key;
            }
            meta_ctrls[] =
            {
              { df_cmd_kind_info_table[DF_CmdKind_Edit].string, OS_Key_F2 },
              { df_cmd_kind_info_table[DF_CmdKind_Accept].string, OS_Key_Return },
              { df_cmd_kind_info_table[DF_CmdKind_Cancel].string, OS_Key_Esc },
            };
            for(U64 idx = 0; idx < ArrayCount(meta_ctrls); idx += 1)
            {
              DF_BindingList bindings = df_bindings_from_name(scratch.arena, meta_ctrls[idx].name);
              if(bindings.count == 0)
              {
                DF_Binding binding = {meta_ctrls[idx].fallback_key, 0};
                df_bind_name(meta_ctrls[idx].name, binding);
              }
            }
          }
        }break;
        
        //- rjf: writing config changes
        case DF_CmdKind_WriteUserData:
        case DF_CmdKind_WriteProjectData:
        {
          D_CfgSrc src = D_CfgSrc_User;
          for(D_CfgSrc s = (D_CfgSrc)0; s < D_CfgSrc_COUNT; s = (D_CfgSrc)(s+1))
          {
            if(kind == d_cfg_src_write_cmd_kind_table[s])
            {
              src = s;
              break;
            }
          }
          String8 path = df_cfg_path_from_src(src);
          String8List d_strs = d_cfg_strings_from_core(scratch.arena, path, src);
          String8List df_strs = df_cfg_strings_from_gfx(scratch.arena, path, src);
          String8 header = push_str8f(scratch.arena, "// raddbg %s file\n\n", d_cfg_src_string_table[src].str);
          String8List strs = {0};
          str8_list_push(scratch.arena, &strs, header);
          str8_list_concat_in_place(&strs, &d_strs);
          str8_list_concat_in_place(&strs, &df_strs);
          String8 data = str8_list_join(scratch.arena, &strs, 0);
          String8 data_indented = indented_from_string(scratch.arena, data);
          os_write_data_to_file_path(path, data_indented);
        }break;
        
        //- rjf: code navigation
        case DF_CmdKind_FindTextForward:
        case DF_CmdKind_FindTextBackward:
        {
          df_set_search_string(df_regs()->string);
        }break;
        
        //- rjf: find next and find prev
        case DF_CmdKind_FindNext:
        {
          df_cmd(DF_CmdKind_FindTextForward, .string = df_push_search_string(scratch.arena));
        }break;
        case DF_CmdKind_FindPrev:
        {
          df_cmd(DF_CmdKind_FindTextBackward, .string = df_push_search_string(scratch.arena));
        }break;
        
        //- rjf: font sizes
        case DF_CmdKind_IncUIFontScale:
        {
          DF_Window *window = df_window_from_handle(df_regs()->window);
          if(window != 0)
          {
            window->setting_vals[DF_SettingCode_MainFontSize].set = 1;
            window->setting_vals[DF_SettingCode_MainFontSize].s32 += 1;
            window->setting_vals[DF_SettingCode_MainFontSize].s32 = clamp_1s32(df_g_setting_code_s32_range_table[DF_SettingCode_MainFontSize], window->setting_vals[DF_SettingCode_MainFontSize].s32);
          }
        }break;
        case DF_CmdKind_DecUIFontScale:
        {
          DF_Window *window = df_window_from_handle(df_regs()->window);
          if(window != 0)
          {
            window->setting_vals[DF_SettingCode_MainFontSize].set = 1;
            window->setting_vals[DF_SettingCode_MainFontSize].s32 -= 1;
            window->setting_vals[DF_SettingCode_MainFontSize].s32 = clamp_1s32(df_g_setting_code_s32_range_table[DF_SettingCode_MainFontSize], window->setting_vals[DF_SettingCode_MainFontSize].s32);
          }
        }break;
        case DF_CmdKind_IncCodeFontScale:
        {
          DF_Window *window = df_window_from_handle(df_regs()->window);
          if(window != 0)
          {
            window->setting_vals[DF_SettingCode_CodeFontSize].set = 1;
            window->setting_vals[DF_SettingCode_CodeFontSize].s32 += 1;
            window->setting_vals[DF_SettingCode_CodeFontSize].s32 = clamp_1s32(df_g_setting_code_s32_range_table[DF_SettingCode_CodeFontSize], window->setting_vals[DF_SettingCode_CodeFontSize].s32);
          }
        }break;
        case DF_CmdKind_DecCodeFontScale:
        {
          DF_Window *window = df_window_from_handle(df_regs()->window);
          if(window != 0)
          {
            window->setting_vals[DF_SettingCode_CodeFontSize].set = 1;
            window->setting_vals[DF_SettingCode_CodeFontSize].s32 -= 1;
            window->setting_vals[DF_SettingCode_CodeFontSize].s32 = clamp_1s32(df_g_setting_code_s32_range_table[DF_SettingCode_CodeFontSize], window->setting_vals[DF_SettingCode_CodeFontSize].s32);
          }
        }break;
        
        //- rjf: panel creation
        case DF_CmdKind_NewPanelLeft: {split_dir = Dir2_Left;}goto split;
        case DF_CmdKind_NewPanelUp:   {split_dir = Dir2_Up;}goto split;
        case DF_CmdKind_NewPanelRight:{split_dir = Dir2_Right;}goto split;
        case DF_CmdKind_NewPanelDown: {split_dir = Dir2_Down;}goto split;
        case DF_CmdKind_SplitPanel:
        {
          split_dir = df_regs()->dir2;
          split_panel = df_panel_from_handle(df_regs()->dst_panel);
        }goto split;
        split:;
        if(split_dir != Dir2_Invalid)
        {
          DF_Window *ws = df_window_from_handle(df_regs()->window);
          if(df_panel_is_nil(split_panel))
          {
            split_panel = ws->focused_panel;
          }
          DF_Panel *new_panel = &df_nil_panel;
          Axis2 split_axis = axis2_from_dir2(split_dir);
          Side split_side = side_from_dir2(split_dir);
          DF_Panel *panel = split_panel;
          DF_Panel *parent = panel->parent;
          if(!df_panel_is_nil(parent) && parent->split_axis == split_axis)
          {
            DF_Panel *next = df_panel_alloc(ws);
            df_panel_insert(parent, split_side == Side_Max ? panel : panel->prev, next);
            next->pct_of_parent = 1.f/parent->child_count;
            for(DF_Panel *child = parent->first; !df_panel_is_nil(child); child = child->next)
            {
              if(child != next)
              {
                child->pct_of_parent *= (F32)(parent->child_count-1) / (parent->child_count);
              }
            }
            ws->focused_panel = next;
            new_panel = next;
          }
          else
          {
            DF_Panel *pre_prev = panel->prev;
            DF_Panel *pre_parent = parent;
            DF_Panel *new_parent = df_panel_alloc(ws);
            new_parent->pct_of_parent = panel->pct_of_parent;
            if(!df_panel_is_nil(pre_parent))
            {
              df_panel_remove(pre_parent, panel);
              df_panel_insert(pre_parent, pre_prev, new_parent);
            }
            else
            {
              ws->root_panel = new_parent;
            }
            DF_Panel *left = panel;
            DF_Panel *right = df_panel_alloc(ws);
            new_panel = right;
            if(split_side == Side_Min)
            {
              Swap(DF_Panel *, left, right);
            }
            df_panel_insert(new_parent, &df_nil_panel, left);
            df_panel_insert(new_parent, left, right);
            new_parent->split_axis = split_axis;
            left->pct_of_parent = 0.5f;
            right->pct_of_parent = 0.5f;
            ws->focused_panel = new_panel;
          }
          if(!df_panel_is_nil(new_panel->prev))
          {
            Rng2F32 prev_rect_pct = new_panel->prev->animated_rect_pct;
            new_panel->animated_rect_pct = prev_rect_pct;
            new_panel->animated_rect_pct.p0.v[split_axis] = new_panel->animated_rect_pct.p1.v[split_axis];
          }
          if(!df_panel_is_nil(new_panel->next))
          {
            Rng2F32 next_rect_pct = new_panel->next->animated_rect_pct;
            new_panel->animated_rect_pct = next_rect_pct;
            new_panel->animated_rect_pct.p1.v[split_axis] = new_panel->animated_rect_pct.p0.v[split_axis];
          }
          DF_Panel *move_tab_panel = df_panel_from_handle(df_regs()->panel);
          DF_View *move_tab = df_view_from_handle(df_regs()->view);
          if(!df_panel_is_nil(new_panel) && !df_view_is_nil(move_tab) && !df_panel_is_nil(move_tab_panel) &&
             kind == DF_CmdKind_SplitPanel)
          {
            df_panel_remove_tab_view(move_tab_panel, move_tab);
            df_panel_insert_tab_view(new_panel, new_panel->last_tab_view, move_tab);
            new_panel->selected_tab_view = df_handle_from_view(move_tab);
            B32 move_tab_panel_is_empty = 1;
            for(DF_View *v = move_tab_panel->first_tab_view; !df_view_is_nil(v); v = v->order_next)
            {
              if(!df_view_is_project_filtered(v))
              {
                move_tab_panel_is_empty = 0;
                break;
              }
            }
            if(move_tab_panel_is_empty && move_tab_panel != ws->root_panel &&
               move_tab_panel != new_panel->prev && move_tab_panel != new_panel->next)
            {
              df_cmd(DF_CmdKind_ClosePanel, .panel = df_handle_from_panel(move_tab_panel));
            }
          }
        }break;
        
        //- rjf: panel rotation
        case DF_CmdKind_RotatePanelColumns:
        {
          DF_Window *ws = df_window_from_handle(df_regs()->window);
          DF_Panel *panel = ws->focused_panel;
          DF_Panel *parent = &df_nil_panel;
          for(DF_Panel *p = panel->parent; !df_panel_is_nil(p); p = p->parent)
          {
            if(p->split_axis == Axis2_X)
            {
              parent = p;
              break;
            }
          }
          if(!df_panel_is_nil(parent) && parent->child_count > 1)
          {
            DF_Panel *old_first = parent->first;
            DF_Panel *new_first = parent->first->next;
            old_first->next = &df_nil_panel;
            old_first->prev = parent->last;
            parent->last->next = old_first;
            new_first->prev = &df_nil_panel;
            parent->first = new_first;
            parent->last = old_first;
          }
        }break;
        
        //- rjf: panel focusing
        case DF_CmdKind_NextPanel: panel_sib_off = OffsetOf(DF_Panel, next); panel_child_off = OffsetOf(DF_Panel, first); goto cycle;
        case DF_CmdKind_PrevPanel: panel_sib_off = OffsetOf(DF_Panel, prev); panel_child_off = OffsetOf(DF_Panel, last); goto cycle;
        cycle:;
        {
          DF_Window *ws = df_window_from_handle(df_regs()->window);
          for(DF_Panel *panel = ws->focused_panel; !df_panel_is_nil(panel);)
          {
            DF_PanelRec rec = df_panel_rec_df(panel, panel_sib_off, panel_child_off);
            panel = rec.next;
            if(df_panel_is_nil(panel))
            {
              panel = ws->root_panel;
            }
            if(df_panel_is_nil(panel->first))
            {
              df_cmd(DF_CmdKind_FocusPanel, .panel = df_handle_from_panel(panel));
              break;
            }
          }
        }break;
        case DF_CmdKind_FocusPanel:
        {
          DF_Window *ws = df_window_from_handle(df_regs()->window);
          DF_Panel *panel = df_panel_from_handle(df_regs()->panel);
          if(!df_panel_is_nil(panel))
          {
            ws->focused_panel = panel;
            ws->menu_bar_focused = 0;
            ws->query_view_selected = 0;
          }
        }break;
        
        //- rjf: directional panel focus changing
        case DF_CmdKind_FocusPanelRight: panel_change_dir = v2s32(+1, +0); goto focus_panel_dir;
        case DF_CmdKind_FocusPanelLeft:  panel_change_dir = v2s32(-1, +0); goto focus_panel_dir;
        case DF_CmdKind_FocusPanelUp:    panel_change_dir = v2s32(+0, -1); goto focus_panel_dir;
        case DF_CmdKind_FocusPanelDown:  panel_change_dir = v2s32(+0, +1); goto focus_panel_dir;
        focus_panel_dir:;
        {
          DF_Window *ws = df_window_from_handle(df_regs()->window);
          DF_Panel *src_panel = ws->focused_panel;
          Rng2F32 src_panel_rect = df_target_rect_from_panel(r2f32(v2f32(0, 0), v2f32(1000, 1000)), ws->root_panel, src_panel);
          Vec2F32 src_panel_center = center_2f32(src_panel_rect);
          Vec2F32 src_panel_half_dim = scale_2f32(dim_2f32(src_panel_rect), 0.5f);
          Vec2F32 travel_dim = add_2f32(src_panel_half_dim, v2f32(10.f, 10.f));
          Vec2F32 travel_dst = add_2f32(src_panel_center, mul_2f32(travel_dim, v2f32((F32)panel_change_dir.x, (F32)panel_change_dir.y)));
          DF_Panel *dst_root = &df_nil_panel;
          for(DF_Panel *p = ws->root_panel; !df_panel_is_nil(p); p = df_panel_rec_df_pre(p).next)
          {
            if(p == src_panel || !df_panel_is_nil(p->first))
            {
              continue;
            }
            Rng2F32 p_rect = df_target_rect_from_panel(r2f32(v2f32(0, 0), v2f32(1000, 1000)), ws->root_panel, p);
            if(contains_2f32(p_rect, travel_dst))
            {
              dst_root = p;
              break;
            }
          }
          if(!df_panel_is_nil(dst_root))
          {
            DF_Panel *dst_panel = &df_nil_panel;
            for(DF_Panel *p = dst_root; !df_panel_is_nil(p); p = df_panel_rec_df_pre(p).next)
            {
              if(df_panel_is_nil(p->first) && p != src_panel)
              {
                dst_panel = p;
                break;
              }
            }
            df_cmd(DF_CmdKind_FocusPanel, .panel = df_handle_from_panel(dst_panel));
          }
        }break;
        
        //- rjf: undo/redo
        case DF_CmdKind_Undo:{}break;
        case DF_CmdKind_Redo:{}break;
        
        //- rjf: focus history
        case DF_CmdKind_GoBack:{}break;
        case DF_CmdKind_GoForward:{}break;
        
        //- rjf: files
        case DF_CmdKind_SetCurrentPath:
        {
          arena_clear(df_state->current_path_arena);
          df_state->current_path = push_str8_copy(df_state->current_path_arena, df_regs()->file_path);
        }break;
        
        //- rjf: override file links
        case DF_CmdKind_SetFileOverrideLinkSrc:
        case DF_CmdKind_SetFileOverrideLinkDst:
        {
#if 0 // TODO(rjf): @msgs
          // rjf: unpack args
          DF_Entity *map = df_entity_from_handle(df_regs()->entity);
          String8 path = path_normalized_from_string(scratch.arena, df_regs()->file_path);
          String8 path_folder = str8_chop_last_slash(path);
          String8 path_file = str8_skip_last_slash(path);
          
          // rjf: src -> move map & commit name; dst -> open destination file & refer to it in map
          switch(kind)
          {
            default:{}break;
            case DF_CmdKind_SetFileOverrideLinkSrc:
            {
              DF_Entity *map_parent = (df_regs()->file_path.size != 0) ? d_entity_from_path(path_folder, D_EntityFromPathFlag_OpenAsNeeded|D_EntityFromPathFlag_OpenMissing) : df_entity_root();
              if(df_entity_is_nil(map))
              {
                map = df_entity_alloc(map_parent, DF_EntityKind_FilePathMap);
              }
              else
              {
                df_entity_change_parent(map, map->parent, map_parent, &d_nil_entity);
              }
              df_entity_equip_name(map, path_file);
            }break;
            case DF_CmdKind_SetFileOverrideLinkDst:
            {
              if(df_entity_is_nil(map))
              {
                map = df_entity_alloc(df_entity_root(), DF_EntityKind_FilePathMap);
              }
              DF_Entity *map_dst_entity = &d_nil_entity;
              if(df_regs()->file_path.size != 0)
              {
                map_dst_entity = d_entity_from_path(path, D_EntityFromPathFlag_All);
              }
              df_entity_equip_entity_handle(map, df_handle_from_entity(map_dst_entity));
            }break;
          }
          
          // rjf: empty src/dest -> delete
          if(!df_entity_is_nil(map) && map->string.size == 0 && df_entity_is_nil(df_entity_from_handle(map->entity_handle)))
          {
            df_entity_mark_for_deletion(map);
          }
#endif
        }break;
        case DF_CmdKind_SetFileReplacementPath:
        {
          // NOTE(rjf):
          //
          // C:/foo/bar/baz.c
          // D:/foo/bar/baz.c
          // -> override C: -> D:
          //
          // C:/1/2/foo/bar.c
          // C:/2/3/foo/bar.c
          // -> override C:/1/2 -> C:2/3
          //
          // C:/foo/bar/baz.c
          // D:/1/2/3.c
          // -> override C:/foo/bar/baz.c -> D:/1/2/3.c
          
          //- rjf: unpack
          String8 src_path = df_regs()->string;
          String8 dst_path = df_regs()->file_path;
#if 0 // TODO(rjf): @msgs
          
          //- rjf: grab src file & chosen replacement
          DF_Entity *file = df_entity_from_handle(params.entity);
          DF_Entity *replacement = d_entity_from_path(params.file_path, D_EntityFromPathFlag_OpenAsNeeded|D_EntityFromPathFlag_OpenMissing);
          
          //- rjf: find 
          DF_Entity *first_diff_src = file;
          DF_Entity *first_diff_dst = replacement;
          for(;!df_entity_is_nil(first_diff_src) && !df_entity_is_nil(first_diff_dst);)
          {
            if(!str8_match(first_diff_src->string, first_diff_dst->string, StringMatchFlag_CaseInsensitive) ||
               first_diff_src->parent->kind != DF_EntityKind_File ||
               first_diff_src->parent->parent->kind != DF_EntityKind_File ||
               first_diff_dst->parent->kind != DF_EntityKind_File ||
               first_diff_dst->parent->parent->kind != DF_EntityKind_File)
            {
              break;
            }
            first_diff_src = first_diff_src->parent;
            first_diff_dst = first_diff_dst->parent;
          }
          
          //- rjf: override first different
          if(!df_entity_is_nil(first_diff_src) && !df_entity_is_nil(first_diff_dst))
          {
            DF_Entity *link = df_entity_child_from_string_and_kind(first_diff_src->parent, first_diff_src->name, DF_EntityKind_FilePathMap);
            if(df_entity_is_nil(link))
            {
              link = df_entity_alloc(first_diff_src->parent, DF_EntityKind_FilePathMap);
              df_entity_equip_name(link, first_diff_src->name);
            }
            df_entity_equip_entity_handle(link, df_handle_from_entity(first_diff_dst));
          }
#endif
        }break;
        
        //- rjf: auto view rules
        case DF_CmdKind_SetAutoViewRuleType:
        case DF_CmdKind_SetAutoViewRuleViewRule:
        {
        }break;
        
        //- rjf: panel removal
        case DF_CmdKind_ClosePanel:
        {
          DF_Window *ws = df_window_from_handle(df_regs()->window);
          DF_Panel *panel = df_panel_from_handle(df_regs()->panel);
          DF_Panel *parent = panel->parent;
          if(!df_panel_is_nil(parent))
          {
            Axis2 split_axis = parent->split_axis;
            
            // NOTE(rjf): If we're removing all but the last child of this parent,
            // we should just remove both children.
            if(parent->child_count == 2)
            {
              DF_Panel *discard_child = panel;
              DF_Panel *keep_child = panel == parent->first ? parent->last : parent->first;
              DF_Panel *grandparent = parent->parent;
              DF_Panel *parent_prev = parent->prev;
              F32 pct_of_parent = parent->pct_of_parent;
              
              // rjf: unhook kept child
              df_panel_remove(parent, keep_child);
              
              // rjf: unhook this subtree
              if(!df_panel_is_nil(grandparent))
              {
                df_panel_remove(grandparent, parent);
              }
              
              // rjf: release the things we should discard
              {
                df_panel_release(ws, parent);
                df_panel_release(ws, discard_child);
              }
              
              // rjf: re-hook our kept child into the overall tree
              if(df_panel_is_nil(grandparent))
              {
                ws->root_panel = keep_child;
              }
              else
              {
                df_panel_insert(grandparent, parent_prev, keep_child);
              }
              keep_child->pct_of_parent = pct_of_parent;
              
              // rjf: reset focus, if needed
              if(ws->focused_panel == discard_child)
              {
                ws->focused_panel = keep_child;
                for(DF_Panel *grandchild = ws->focused_panel; !df_panel_is_nil(grandchild); grandchild = grandchild->first)
                {
                  ws->focused_panel = grandchild;
                }
              }
              
              // rjf: keep-child split-axis == grandparent split-axis? bubble keep-child up into grandparent's children
              if(!df_panel_is_nil(grandparent) && grandparent->split_axis == keep_child->split_axis && !df_panel_is_nil(keep_child->first))
              {
                df_panel_remove(grandparent, keep_child);
                DF_Panel *prev = parent_prev;
                for(DF_Panel *child = keep_child->first, *next = 0; !df_panel_is_nil(child); child = next)
                {
                  next = child->next;
                  df_panel_remove(keep_child, child);
                  df_panel_insert(grandparent, prev, child);
                  prev = child;
                  child->pct_of_parent *= keep_child->pct_of_parent;
                }
                df_panel_release(ws, keep_child);
              }
            }
            // NOTE(rjf): Otherwise we can just remove this child.
            else
            {
              DF_Panel *next = &df_nil_panel;
              F32 removed_size_pct = panel->pct_of_parent;
              if(df_panel_is_nil(next)) { next = panel->prev; }
              if(df_panel_is_nil(next)) { next = panel->next; }
              df_panel_remove(parent, panel);
              df_panel_release(ws, panel);
              if(ws->focused_panel == panel)
              {
                ws->focused_panel = next;
              }
              for(DF_Panel *child = parent->first; !df_panel_is_nil(child); child = child->next)
              {
                child->pct_of_parent /= 1.f-removed_size_pct;
              }
            }
          }
        }break;
        
        //- rjf: panel tab controls
        case DF_CmdKind_NextTab:
        {
          DF_Panel *panel = df_panel_from_handle(df_regs()->panel);
          DF_View *view = df_selected_tab_from_panel(panel);
          DF_View *next_view = view;
          for(DF_View *v = view; !df_view_is_nil(v); v = df_view_is_nil(v->order_next) ? panel->first_tab_view : v->order_next)
          {
            if(!df_view_is_project_filtered(v) && v != view)
            {
              next_view = v;
              break;
            }
          }
          view = next_view;
          panel->selected_tab_view = df_handle_from_view(view);
        }break;
        case DF_CmdKind_PrevTab:
        {
          DF_Panel *panel = df_panel_from_handle(df_regs()->panel);
          DF_View *view = df_selected_tab_from_panel(panel);
          DF_View *next_view = view;
          for(DF_View *v = view; !df_view_is_nil(v); v = df_view_is_nil(v->order_prev) ? panel->last_tab_view : v->order_prev)
          {
            if(!df_view_is_project_filtered(v) && v != view)
            {
              next_view = v;
              break;
            }
          }
          view = next_view;
          panel->selected_tab_view = df_handle_from_view(view);
        }break;
        case DF_CmdKind_MoveTabRight:
        case DF_CmdKind_MoveTabLeft:
        {
          DF_Window *ws = df_window_from_handle(df_regs()->window);
          DF_Panel *panel = ws->focused_panel;
          DF_View *view = df_selected_tab_from_panel(panel);
          DF_View *prev_view = (kind == DF_CmdKind_MoveTabRight ? view->order_next : view->order_prev->order_prev);
          if(!df_view_is_nil(prev_view) || kind == DF_CmdKind_MoveTabLeft)
          {
            df_cmd(DF_CmdKind_MoveTab,
                   .panel = df_handle_from_panel(panel),
                   .dst_panel = df_handle_from_panel(panel),
                   .view = df_handle_from_view(view),
                   .prev_view = df_handle_from_view(prev_view));
          }
        }break;
        case DF_CmdKind_OpenTab:
        {
          DF_Panel *panel = df_panel_from_handle(df_regs()->panel);
          DF_ViewSpec *spec = df_view_spec_from_string(df_regs()->string);
          DF_Entity *entity = &d_nil_entity;
          if(spec->info.flags & DF_ViewSpecFlag_ParameterizedByEntity)
          {
            entity = df_entity_from_handle(df_regs()->entity);
          }
          if(!df_panel_is_nil(panel) && spec != &df_nil_view_spec)
          {
            DF_View *view = df_view_alloc();
            String8 query = {0};
            if(!df_entity_is_nil(entity))
            {
              query = d_eval_string_from_entity(scratch.arena, entity);
            }
            else if(df_regs()->file_path.size != 0)
            {
              query = d_eval_string_from_file_path(scratch.arena, df_regs()->file_path);
            }
            else if(df_regs()->string.size != 0)
            {
              query = df_regs()->string;
            }
            df_view_equip_spec(view, spec, query, df_regs()->params_tree);
            df_panel_insert_tab_view(panel, panel->last_tab_view, view);
          }
        }break;
        case DF_CmdKind_CloseTab:
        {
          DF_Panel *panel = df_panel_from_handle(df_regs()->panel);
          DF_View *view = df_view_from_handle(df_regs()->view);
          if(!df_view_is_nil(view))
          {
            df_panel_remove_tab_view(panel, view);
            df_view_release(view);
          }
        }break;
        case DF_CmdKind_MoveTab:
        {
          DF_Window *ws = df_window_from_handle(df_regs()->window);
          DF_Panel *src_panel = df_panel_from_handle(df_regs()->panel);
          DF_View *view = df_view_from_handle(df_regs()->view);
          DF_Panel *dst_panel = df_panel_from_handle(df_regs()->dst_panel);
          DF_View *prev_view = df_view_from_handle(df_regs()->prev_view);
          if(!df_panel_is_nil(src_panel) &&
             !df_panel_is_nil(dst_panel) &&
             prev_view != view)
          {
            df_panel_remove_tab_view(src_panel, view);
            df_panel_insert_tab_view(dst_panel, prev_view, view);
            ws->focused_panel = dst_panel;
            B32 src_panel_is_empty = 1;
            for(DF_View *v = src_panel->first_tab_view; !df_view_is_nil(v); v = v->order_next)
            {
              if(!df_view_is_project_filtered(v))
              {
                src_panel_is_empty = 0;
                break;
              }
            }
            if(src_panel_is_empty && src_panel != ws->root_panel)
            {
              df_cmd(DF_CmdKind_ClosePanel, .panel = df_handle_from_panel(src_panel));
            }
          }
        }break;
        case DF_CmdKind_TabBarTop:
        {
          DF_Panel *panel = df_panel_from_handle(df_regs()->panel);
          panel->tab_side = Side_Min;
        }break;
        case DF_CmdKind_TabBarBottom:
        {
          DF_Panel *panel = df_panel_from_handle(df_regs()->panel);
          panel->tab_side = Side_Max;
        }break;
        
        //- rjf: files
        case DF_CmdKind_Open:
        {
          DF_Window *ws = df_window_from_handle(df_regs()->window);
          String8 path = df_regs()->file_path;
          FileProperties props = os_properties_from_file_path(path);
          if(props.created != 0)
          {
            df_cmd(DF_CmdKind_PendingFile);
          }
          else
          {
            log_user_errorf("Couldn't open file at \"%S\".", path);
          }
        }break;
        case DF_CmdKind_Switch:
        {
          // TODO(rjf): @msgs
#if 0
          B32 already_opened = 0;
          DF_Panel *panel = df_panel_from_handle(df_regs()->panel);
          for(DF_View *v = panel->first_tab_view; !df_view_is_nil(v); v = v->next)
          {
            if(df_view_is_project_filtered(v)) { continue; }
            DF_Entity *v_param_entity = df_entity_from_handle(v->params_entity);
            if(v_param_entity == df_entity_from_handle(df_regs()->entity))
            {
              panel->selected_tab_view = df_handle_from_view(v);
              already_opened = 1;
              break;
            }
          }
          if(already_opened == 0)
          {
            D_CmdParams p = params;
            p.window = df_handle_from_window(ws);
            p.panel = df_handle_from_panel(ws->focused_panel);
            p.entity = df_regs()->entity;
            d_cmd_list_push(arena, cmds, &p, d_cmd_spec_from_kind(D_CmdKind_PendingFile));
          }
#endif
        }break;
        case DF_CmdKind_SwitchToPartnerFile:
        {
          DF_Panel *panel = df_panel_from_handle(df_regs()->panel);
          DF_View *view = df_selected_tab_from_panel(panel);
          DF_ViewKind view_kind = df_view_kind_from_string(view->spec->info.name);
          if(view_kind == DF_ViewKind_Text)
          {
            String8 file_path      = d_file_path_from_eval_string(scratch.arena, str8(view->query_buffer, view->query_string_size));
            String8 file_full_path = path_normalized_from_string(scratch.arena, file_path);
            String8 file_folder    = str8_chop_last_slash(file_full_path);
            String8 file_name      = str8_skip_last_slash(str8_chop_last_dot(file_full_path));
            String8 file_ext       = str8_skip_last_dot(file_full_path);
            String8 partner_ext_candidates[] =
            {
              str8_lit_comp("h"),
              str8_lit_comp("hpp"),
              str8_lit_comp("hxx"),
              str8_lit_comp("c"),
              str8_lit_comp("cc"),
              str8_lit_comp("cxx"),
              str8_lit_comp("cpp"),
            };
            for(U64 idx = 0; idx < ArrayCount(partner_ext_candidates); idx += 1)
            {
              if(!str8_match(partner_ext_candidates[idx], file_ext, StringMatchFlag_CaseInsensitive))
              {
                String8 candidate = push_str8f(scratch.arena, "%S.%S", file_name, partner_ext_candidates[idx]);
                String8 candidate_path = push_str8f(scratch.arena, "%S/%S", file_folder, candidate);
                FileProperties candidate_props = os_properties_from_file_path(candidate_path);
                if(candidate_props.modified != 0)
                {
                  // TODO(rjf):
                  //D_CmdParams p = df_cmd_params_from_panel(ws, panel);
                  //p.entity = df_handle_from_entity(candidate);
                  //d_cmd_list_push(arena, cmds, &p, d_cmd_spec_from_kind(D_CmdKind_Switch));
                  break;
                }
              }
            }
          }
        }break;
        
        //- rjf: panel built-in layout builds
        case DF_CmdKind_ResetToDefaultPanels:
        case DF_CmdKind_ResetToCompactPanels:
        {
          panel_reset_done = 1;
          DF_Window *ws = df_window_from_handle(df_regs()->window);
          
          typedef enum Layout
          {
            Layout_Default,
            Layout_Compact,
          }
          Layout;
          Layout layout = Layout_Default;
          switch(kind)
          {
            default:{}break;
            case DF_CmdKind_ResetToDefaultPanels:{layout = Layout_Default;}break;
            case DF_CmdKind_ResetToCompactPanels:{layout = Layout_Compact;}break;
          }
          
          //- rjf: gather all panels in the panel tree - remove & gather views
          // we'd like to keep in the next layout
          D_HandleList panels_to_close = {0};
          D_HandleList views_to_close = {0};
          DF_View *watch = &df_nil_view;
          DF_View *locals = &df_nil_view;
          DF_View *regs = &df_nil_view;
          DF_View *globals = &df_nil_view;
          DF_View *tlocals = &df_nil_view;
          DF_View *types = &df_nil_view;
          DF_View *procs = &df_nil_view;
          DF_View *callstack = &df_nil_view;
          DF_View *breakpoints = &df_nil_view;
          DF_View *watch_pins = &df_nil_view;
          DF_View *output = &df_nil_view;
          DF_View *targets = &df_nil_view;
          DF_View *scheduler = &df_nil_view;
          DF_View *modules = &df_nil_view;
          DF_View *disasm = &df_nil_view;
          DF_View *memory = &df_nil_view;
          DF_View *getting_started = &df_nil_view;
          D_HandleList code_views = {0};
          for(DF_Panel *panel = ws->root_panel; !df_panel_is_nil(panel); panel = df_panel_rec_df_pre(panel).next)
          {
            D_Handle handle = df_handle_from_panel(panel);
            d_handle_list_push(scratch.arena, &panels_to_close, handle);
            for(DF_View *view = panel->first_tab_view, *next = 0; !df_view_is_nil(view); view = next)
            {
              next = view->order_next;
              DF_ViewKind view_kind = df_view_kind_from_string(view->spec->info.name);
              B32 needs_delete = 1;
              switch(view_kind)
              {
                default:{}break;
                case DF_ViewKind_Watch:         {if(df_view_is_nil(watch))               { needs_delete = 0; watch = view;} }break;
                case DF_ViewKind_Locals:        {if(df_view_is_nil(locals))              { needs_delete = 0; locals = view;} }break;
                case DF_ViewKind_Registers:     {if(df_view_is_nil(regs))                { needs_delete = 0; regs = view;} }break;
                case DF_ViewKind_Globals:       {if(df_view_is_nil(globals))             { needs_delete = 0; globals = view;} }break;
                case DF_ViewKind_ThreadLocals:  {if(df_view_is_nil(tlocals))             { needs_delete = 0; tlocals = view;} }break;
                case DF_ViewKind_Types:         {if(df_view_is_nil(types))               { needs_delete = 0; types = view;} }break;
                case DF_ViewKind_Procedures:    {if(df_view_is_nil(procs))               { needs_delete = 0; procs = view;} }break;
                case DF_ViewKind_CallStack:     {if(df_view_is_nil(callstack))           { needs_delete = 0; callstack = view;} }break;
                case DF_ViewKind_Breakpoints:   {if(df_view_is_nil(breakpoints))         { needs_delete = 0; breakpoints = view;} }break;
                case DF_ViewKind_WatchPins:     {if(df_view_is_nil(watch_pins))          { needs_delete = 0; watch_pins = view;} }break;
                case DF_ViewKind_Output:        {if(df_view_is_nil(output))              { needs_delete = 0; output = view;} }break;
                case DF_ViewKind_Targets:       {if(df_view_is_nil(targets))             { needs_delete = 0; targets = view;} }break;
                case DF_ViewKind_Scheduler:     {if(df_view_is_nil(scheduler))           { needs_delete = 0; scheduler = view;} }break;
                case DF_ViewKind_Modules:       {if(df_view_is_nil(modules))             { needs_delete = 0; modules = view;} }break;
                case DF_ViewKind_Disasm:        {if(df_view_is_nil(disasm))              { needs_delete = 0; disasm = view;} }break;
                case DF_ViewKind_Memory:        {if(df_view_is_nil(memory))              { needs_delete = 0; memory = view;} }break;
                case DF_ViewKind_GettingStarted:{if(df_view_is_nil(getting_started))     { needs_delete = 0; getting_started = view;} }break;
                case DF_ViewKind_Text:
                {
                  needs_delete = 0;
                  d_handle_list_push(scratch.arena, &code_views, df_handle_from_view(view));
                }break;
              }
              if(!needs_delete)
              {
                df_panel_remove_tab_view(panel, view);
              }
            }
          }
          
          //- rjf: close all panels/views
          for(D_HandleNode *n = panels_to_close.first; n != 0; n = n->next)
          {
            DF_Panel *panel = df_panel_from_handle(n->handle);
            if(panel != ws->root_panel)
            {
              df_panel_release(ws, panel);
            }
            else
            {
              df_panel_release_all_views(panel);
              panel->first = panel->last = &df_nil_panel;
            }
          }
          
          //- rjf: allocate any missing views
          if(df_view_is_nil(watch))
          {
            watch = df_view_alloc();
            df_view_equip_spec(watch, df_view_spec_from_kind(DF_ViewKind_Watch), str8_zero(), &md_nil_node);
          }
          if(layout == Layout_Default && df_view_is_nil(locals))
          {
            locals = df_view_alloc();
            df_view_equip_spec(locals, df_view_spec_from_kind(DF_ViewKind_Locals), str8_zero(), &md_nil_node);
          }
          if(layout == Layout_Default && df_view_is_nil(regs))
          {
            regs = df_view_alloc();
            df_view_equip_spec(regs, df_view_spec_from_kind(DF_ViewKind_Registers), str8_zero(), &md_nil_node);
          }
          if(layout == Layout_Default && df_view_is_nil(globals))
          {
            globals = df_view_alloc();
            df_view_equip_spec(globals, df_view_spec_from_kind(DF_ViewKind_Globals), str8_zero(), &md_nil_node);
          }
          if(layout == Layout_Default && df_view_is_nil(tlocals))
          {
            tlocals = df_view_alloc();
            df_view_equip_spec(tlocals, df_view_spec_from_kind(DF_ViewKind_ThreadLocals), str8_zero(), &md_nil_node);
          }
          if(df_view_is_nil(types))
          {
            types = df_view_alloc();
            df_view_equip_spec(types, df_view_spec_from_kind(DF_ViewKind_Types), str8_zero(), &md_nil_node);
          }
          if(layout == Layout_Default && df_view_is_nil(procs))
          {
            procs = df_view_alloc();
            df_view_equip_spec(procs, df_view_spec_from_kind(DF_ViewKind_Procedures), str8_zero(), &md_nil_node);
          }
          if(df_view_is_nil(callstack))
          {
            callstack = df_view_alloc();
            df_view_equip_spec(callstack, df_view_spec_from_kind(DF_ViewKind_CallStack), str8_zero(), &md_nil_node);
          }
          if(df_view_is_nil(breakpoints))
          {
            breakpoints = df_view_alloc();
            df_view_equip_spec(breakpoints, df_view_spec_from_kind(DF_ViewKind_Breakpoints), str8_zero(), &md_nil_node);
          }
          if(layout == Layout_Default && df_view_is_nil(watch_pins))
          {
            watch_pins = df_view_alloc();
            df_view_equip_spec(watch_pins, df_view_spec_from_kind(DF_ViewKind_WatchPins), str8_zero(), &md_nil_node);
          }
          if(df_view_is_nil(output))
          {
            output = df_view_alloc();
            df_view_equip_spec(output, df_view_spec_from_kind(DF_ViewKind_Output), str8_zero(), &md_nil_node);
          }
          if(df_view_is_nil(targets))
          {
            targets = df_view_alloc();
            df_view_equip_spec(targets, df_view_spec_from_kind(DF_ViewKind_Targets), str8_zero(), &md_nil_node);
          }
          if(df_view_is_nil(scheduler))
          {
            scheduler = df_view_alloc();
            df_view_equip_spec(scheduler, df_view_spec_from_kind(DF_ViewKind_Scheduler), str8_zero(), &md_nil_node);
          }
          if(df_view_is_nil(modules))
          {
            modules = df_view_alloc();
            df_view_equip_spec(modules, df_view_spec_from_kind(DF_ViewKind_Modules), str8_zero(), &md_nil_node);
          }
          if(df_view_is_nil(disasm))
          {
            disasm = df_view_alloc();
            df_view_equip_spec(disasm, df_view_spec_from_kind(DF_ViewKind_Disasm), str8_zero(), &md_nil_node);
          }
          if(layout == Layout_Default && df_view_is_nil(memory))
          {
            memory = df_view_alloc();
            df_view_equip_spec(memory, df_view_spec_from_kind(DF_ViewKind_Memory), str8_zero(), &md_nil_node);
          }
          if(code_views.count == 0 && df_view_is_nil(getting_started))
          {
            getting_started = df_view_alloc();
            df_view_equip_spec(getting_started, df_view_spec_from_kind(DF_ViewKind_GettingStarted), str8_zero(), &md_nil_node);
          }
          
          //- rjf: apply layout
          switch(layout)
          {
            //- rjf: default layout
            case Layout_Default:
            {
              // rjf: root split
              ws->root_panel->split_axis = Axis2_X;
              DF_Panel *root_0 = df_panel_alloc(ws);
              DF_Panel *root_1 = df_panel_alloc(ws);
              df_panel_insert(ws->root_panel, ws->root_panel->last, root_0);
              df_panel_insert(ws->root_panel, ws->root_panel->last, root_1);
              root_0->pct_of_parent = 0.85f;
              root_1->pct_of_parent = 0.15f;
              
              // rjf: root_0 split
              root_0->split_axis = Axis2_Y;
              DF_Panel *root_0_0 = df_panel_alloc(ws);
              DF_Panel *root_0_1 = df_panel_alloc(ws);
              df_panel_insert(root_0, root_0->last, root_0_0);
              df_panel_insert(root_0, root_0->last, root_0_1);
              root_0_0->pct_of_parent = 0.80f;
              root_0_1->pct_of_parent = 0.20f;
              
              // rjf: root_1 split
              root_1->split_axis = Axis2_Y;
              DF_Panel *root_1_0 = df_panel_alloc(ws);
              DF_Panel *root_1_1 = df_panel_alloc(ws);
              df_panel_insert(root_1, root_1->last, root_1_0);
              df_panel_insert(root_1, root_1->last, root_1_1);
              root_1_0->pct_of_parent = 0.50f;
              root_1_1->pct_of_parent = 0.50f;
              df_panel_insert_tab_view(root_1_0, root_1_0->last_tab_view, targets);
              df_panel_insert_tab_view(root_1_1, root_1_1->last_tab_view, scheduler);
              root_1_0->selected_tab_view = df_handle_from_view(targets);
              root_1_1->selected_tab_view = df_handle_from_view(scheduler);
              root_1_1->tab_side = Side_Max;
              
              // rjf: root_0_0 split
              root_0_0->split_axis = Axis2_X;
              DF_Panel *root_0_0_0 = df_panel_alloc(ws);
              DF_Panel *root_0_0_1 = df_panel_alloc(ws);
              df_panel_insert(root_0_0, root_0_0->last, root_0_0_0);
              df_panel_insert(root_0_0, root_0_0->last, root_0_0_1);
              root_0_0_0->pct_of_parent = 0.25f;
              root_0_0_1->pct_of_parent = 0.75f;
              
              // rjf: root_0_0_0 split
              root_0_0_0->split_axis = Axis2_Y;
              DF_Panel *root_0_0_0_0 = df_panel_alloc(ws);
              DF_Panel *root_0_0_0_1 = df_panel_alloc(ws);
              df_panel_insert(root_0_0_0, root_0_0_0->last, root_0_0_0_0);
              df_panel_insert(root_0_0_0, root_0_0_0->last, root_0_0_0_1);
              root_0_0_0_0->pct_of_parent = 0.5f;
              root_0_0_0_1->pct_of_parent = 0.5f;
              df_panel_insert_tab_view(root_0_0_0_0, root_0_0_0_0->last_tab_view, disasm);
              root_0_0_0_0->selected_tab_view = df_handle_from_view(disasm);
              df_panel_insert_tab_view(root_0_0_0_1, root_0_0_0_1->last_tab_view, breakpoints);
              df_panel_insert_tab_view(root_0_0_0_1, root_0_0_0_1->last_tab_view, watch_pins);
              df_panel_insert_tab_view(root_0_0_0_1, root_0_0_0_1->last_tab_view, output);
              df_panel_insert_tab_view(root_0_0_0_1, root_0_0_0_1->last_tab_view, memory);
              root_0_0_0_1->selected_tab_view = df_handle_from_view(output);
              
              // rjf: root_0_1 split
              root_0_1->split_axis = Axis2_X;
              DF_Panel *root_0_1_0 = df_panel_alloc(ws);
              DF_Panel *root_0_1_1 = df_panel_alloc(ws);
              df_panel_insert(root_0_1, root_0_1->last, root_0_1_0);
              df_panel_insert(root_0_1, root_0_1->last, root_0_1_1);
              root_0_1_0->pct_of_parent = 0.60f;
              root_0_1_1->pct_of_parent = 0.40f;
              df_panel_insert_tab_view(root_0_1_0, root_0_1_0->last_tab_view, watch);
              df_panel_insert_tab_view(root_0_1_0, root_0_1_0->last_tab_view, locals);
              df_panel_insert_tab_view(root_0_1_0, root_0_1_0->last_tab_view, regs);
              df_panel_insert_tab_view(root_0_1_0, root_0_1_0->last_tab_view, globals);
              df_panel_insert_tab_view(root_0_1_0, root_0_1_0->last_tab_view, tlocals);
              df_panel_insert_tab_view(root_0_1_0, root_0_1_0->last_tab_view, types);
              df_panel_insert_tab_view(root_0_1_0, root_0_1_0->last_tab_view, procs);
              root_0_1_0->selected_tab_view = df_handle_from_view(watch);
              root_0_1_0->tab_side = Side_Max;
              df_panel_insert_tab_view(root_0_1_1, root_0_1_1->last_tab_view, callstack);
              df_panel_insert_tab_view(root_0_1_1, root_0_1_1->last_tab_view, modules);
              root_0_1_1->selected_tab_view = df_handle_from_view(callstack);
              root_0_1_1->tab_side = Side_Max;
              
              // rjf: fill main panel with getting started, OR all collected code views
              if(!df_view_is_nil(getting_started))
              {
                df_panel_insert_tab_view(root_0_0_1, root_0_0_1->last_tab_view, getting_started);
              }
              for(D_HandleNode *n = code_views.first; n != 0; n = n->next)
              {
                DF_View *view = df_view_from_handle(n->handle);
                if(!df_view_is_nil(view))
                {
                  df_panel_insert_tab_view(root_0_0_1, root_0_0_1->last_tab_view, view);
                }
              }
              
              // rjf: choose initial focused panel
              ws->focused_panel = root_0_0_1;
            }break;
            
            //- rjf: compact layout:
            case Layout_Compact:
            {
              // rjf: root split
              ws->root_panel->split_axis = Axis2_X;
              DF_Panel *root_0 = df_panel_alloc(ws);
              DF_Panel *root_1 = df_panel_alloc(ws);
              df_panel_insert(ws->root_panel, ws->root_panel->last, root_0);
              df_panel_insert(ws->root_panel, ws->root_panel->last, root_1);
              root_0->pct_of_parent = 0.25f;
              root_1->pct_of_parent = 0.75f;
              
              // rjf: root_0 split
              root_0->split_axis = Axis2_Y;
              DF_Panel *root_0_0 = df_panel_alloc(ws);
              {
                if(!df_view_is_nil(watch)) { df_panel_insert_tab_view(root_0_0, root_0_0->last_tab_view, watch); }
                if(!df_view_is_nil(types)) { df_panel_insert_tab_view(root_0_0, root_0_0->last_tab_view, types); }
                root_0_0->selected_tab_view = df_handle_from_view(watch);
              }
              DF_Panel *root_0_1 = df_panel_alloc(ws);
              {
                if(!df_view_is_nil(scheduler))     { df_panel_insert_tab_view(root_0_1, root_0_1->last_tab_view, scheduler); }
                if(!df_view_is_nil(targets))       { df_panel_insert_tab_view(root_0_1, root_0_1->last_tab_view, targets); }
                if(!df_view_is_nil(breakpoints))   { df_panel_insert_tab_view(root_0_1, root_0_1->last_tab_view, breakpoints); }
                if(!df_view_is_nil(watch_pins))    { df_panel_insert_tab_view(root_0_1, root_0_1->last_tab_view, watch_pins); }
                root_0_1->selected_tab_view = df_handle_from_view(scheduler);
              }
              DF_Panel *root_0_2 = df_panel_alloc(ws);
              {
                if(!df_view_is_nil(disasm))    { df_panel_insert_tab_view(root_0_2, root_0_2->last_tab_view, disasm); }
                if(!df_view_is_nil(output))    { df_panel_insert_tab_view(root_0_2, root_0_2->last_tab_view, output); }
                root_0_2->selected_tab_view = df_handle_from_view(disasm);
              }
              DF_Panel *root_0_3 = df_panel_alloc(ws);
              {
                if(!df_view_is_nil(callstack))    { df_panel_insert_tab_view(root_0_3, root_0_3->last_tab_view, callstack); }
                if(!df_view_is_nil(modules))      { df_panel_insert_tab_view(root_0_3, root_0_3->last_tab_view, modules); }
                root_0_3->selected_tab_view = df_handle_from_view(callstack);
              }
              df_panel_insert(root_0, root_0->last, root_0_0);
              df_panel_insert(root_0, root_0->last, root_0_1);
              df_panel_insert(root_0, root_0->last, root_0_2);
              df_panel_insert(root_0, root_0->last, root_0_3);
              root_0_0->pct_of_parent = 0.25f;
              root_0_1->pct_of_parent = 0.25f;
              root_0_2->pct_of_parent = 0.25f;
              root_0_3->pct_of_parent = 0.25f;
              
              // rjf: fill main panel with getting started, OR all collected code views
              if(!df_view_is_nil(getting_started))
              {
                df_panel_insert_tab_view(root_1, root_1->last_tab_view, getting_started);
              }
              for(D_HandleNode *n = code_views.first; n != 0; n = n->next)
              {
                DF_View *view = df_view_from_handle(n->handle);
                if(!df_view_is_nil(view))
                {
                  df_panel_insert_tab_view(root_1, root_1->last_tab_view, view);
                }
              }
              
              // rjf: choose initial focused panel
              ws->focused_panel = root_1;
            }break;
          }
          
          // rjf: dispatch cfg saves
          for(D_CfgSrc src = (D_CfgSrc)0; src < D_CfgSrc_COUNT; src = (D_CfgSrc)(src+1))
          {
            DF_CmdKind write_cmd = d_cfg_src_write_cmd_kind_table[src];
            df_cmd(write_cmd, .file_path = df_cfg_path_from_src(src));
          }
        }break;
        
        
        //- rjf: thread finding
        case DF_CmdKind_FindThread:
        for(DF_Window *ws = df_state->first_window; ws != 0; ws = ws->next)
        {
          DI_Scope *scope = di_scope_open();
          CTRL_Entity *thread = ctrl_entity_from_handle(d_state->ctrl_entity_store, df_regs()->thread);
          U64 unwind_index = df_regs()->unwind_count;
          U64 inline_depth = df_regs()->inline_depth;
          if(thread->kind == CTRL_EntityKind_Thread)
          {
            // rjf: grab rip
            U64 rip_vaddr = d_query_cached_rip_from_thread_unwind(thread, unwind_index);
            
            // rjf: extract thread/rip info
            CTRL_Entity *process = ctrl_entity_ancestor_from_kind(thread, CTRL_EntityKind_Process);
            CTRL_Entity *module = ctrl_module_from_process_vaddr(process, rip_vaddr);
            DI_Key dbgi_key = ctrl_dbgi_key_from_module(module);
            RDI_Parsed *rdi = di_rdi_from_key(scope, &dbgi_key, 0);
            U64 rip_voff = ctrl_voff_from_vaddr(module, rip_vaddr);
            D_LineList lines = d_lines_from_dbgi_key_voff(scratch.arena, &dbgi_key, rip_voff);
            D_Line line = {0};
            {
              U64 idx = 0;
              for(D_LineNode *n = lines.first; n != 0; n = n->next, idx += 1)
              {
                line = n->v;
                if(idx == inline_depth)
                {
                  break;
                }
              }
            }
            
            // rjf: snap to resolved line
            B32 missing_rip   = (rip_vaddr == 0);
            B32 dbgi_missing  = (dbgi_key.min_timestamp == 0 || dbgi_key.path.size == 0);
            B32 dbgi_pending  = !dbgi_missing && rdi == &di_rdi_parsed_nil;
            B32 has_line_info = (line.voff_range.max != 0);
            B32 has_module    = (module != &ctrl_entity_nil);
            B32 has_dbg_info  = has_module && !dbgi_missing;
            if(!dbgi_pending && (has_line_info || has_module))
            {
              df_cmd(DF_CmdKind_FindCodeLocation,
                     .file_path    = line.file_path,
                     .cursor       = line.pt,
                     .process      = process->handle,
                     .voff         = rip_voff,
                     .vaddr        = rip_vaddr,
                     .unwind_count = unwind_index,
                     .inline_depth = inline_depth);
            }
            
            // rjf: snap to resolved address w/o line info
            if(!missing_rip && !dbgi_pending && !has_line_info && !has_module)
            {
              df_cmd(DF_CmdKind_FindCodeLocation,
                     .process      = process->handle,
                     .module       = module->handle,
                     .voff         = rip_voff,
                     .vaddr        = rip_vaddr,
                     .unwind_count = unwind_index,
                     .inline_depth = inline_depth);
            }
            
            // rjf: retry on stopped, pending debug info
            if(!d_ctrl_targets_running() && (dbgi_pending || missing_rip))
            {
              df_cmd(DF_CmdKind_FindThread, .thread = thread->handle);
            }
          }
          di_scope_close(scope);
        }break;
        case DF_CmdKind_FindSelectedThread:
        for(DF_Window *ws = df_state->first_window; ws != 0; ws = ws->next)
        {
          CTRL_Entity *selected_thread = ctrl_entity_from_handle(d_state->ctrl_entity_store, df_base_regs()->thread);
          df_cmd(DF_CmdKind_FindThread,
                 .thread       = selected_thread->handle,
                 .unwind_count = df_base_regs()->unwind_count,
                 .inline_depth = df_base_regs()->inline_depth);
        }break;
        
        //- rjf: name finding
        case DF_CmdKind_GoToName:
        {
          String8 name = df_regs()->string;
          if(name.size != 0)
          {
            B32 name_resolved = 0;
            
            // rjf: try to resolve name as a symbol
            U64 voff = 0;
            DI_Key voff_dbgi_key = {0};
            if(name_resolved == 0)
            {
              DI_KeyList keys = d_push_active_dbgi_key_list(scratch.arena);
              for(DI_KeyNode *n = keys.first; n != 0; n = n->next)
              {
                U64 binary_voff = d_voff_from_dbgi_key_symbol_name(&n->v, name);
                if(binary_voff != 0)
                {
                  voff = binary_voff;
                  voff_dbgi_key = n->v;
                  name_resolved = 1;
                  break;
                }
              }
            }
            
            // rjf: try to resolve name as a file
#if 0 // TODO(rjf): @msgs
            DF_Entity *file = &d_nil_entity;
            if(name_resolved == 0)
            {
              DF_Entity *src_entity = df_entity_from_handle(df_regs()->entity);
              String8 file_part_of_name = name;
              U64 quote_pos = str8_find_needle(name, 0, str8_lit("\""), 0);
              if(quote_pos < name.size)
              {
                file_part_of_name = str8_skip(name, quote_pos+1);
                U64 ender_quote_pos = str8_find_needle(file_part_of_name, 0, str8_lit("\""), 0);
                file_part_of_name = str8_prefix(file_part_of_name, ender_quote_pos);
              }
              if(file_part_of_name.size != 0)
              {
                String8 folder_path = str8_chop_last_slash(file_part_of_name);
                String8 file_name = str8_skip_last_slash(file_part_of_name);
                String8List folders = str8_split_path(scratch.arena, folder_path);
                
                // rjf: some folders are specified
                if(folders.node_count != 0)
                {
                  String8 first_folder_name = folders.first->string;
                  DF_Entity *root_folder = &d_nil_entity;
                  
                  // rjf: try to find root folder as if it's an absolute path
                  if(df_entity_is_nil(root_folder))
                  {
                    root_folder = d_entity_from_path(first_folder_name, D_EntityFromPathFlag_OpenAsNeeded);
                  }
                  
                  // rjf: try to find root folder as if it's a path we've already loaded
                  if(df_entity_is_nil(root_folder))
                  {
                    root_folder = df_entity_from_name_and_kind(first_folder_name, DF_EntityKind_File);
                  }
                  
                  // rjf: try to find root folder as if it's inside of a path we've already loaded
                  if(df_entity_is_nil(root_folder))
                  {
                    DF_EntityList all_files = d_query_cached_entity_list_with_kind(DF_EntityKind_File);
                    for(DF_EntityNode *n = all_files.first; n != 0; n = n->next)
                    {
                      if(n->entity->flags & DF_EntityFlag_IsFolder)
                      {
                        String8 n_entity_path = df_full_path_from_entity(scratch.arena, n->entity);
                        String8 estimated_full_path = push_str8f(scratch.arena, "%S/%S", n_entity_path, first_folder_name);
                        root_folder = d_entity_from_path(estimated_full_path, D_EntityFromPathFlag_OpenAsNeeded);
                        if(!df_entity_is_nil(root_folder))
                        {
                          break;
                        }
                      }
                    }
                  }
                  
                  // rjf: has root folder -> descend downwards
                  if(!df_entity_is_nil(root_folder))
                  {
                    String8 root_folder_path = df_full_path_from_entity(scratch.arena, root_folder);
                    String8List full_file_path_parts = {0};
                    str8_list_push(scratch.arena, &full_file_path_parts, root_folder_path);
                    for(String8Node *n = folders.first->next; n != 0; n = n->next)
                    {
                      str8_list_push(scratch.arena, &full_file_path_parts, n->string);
                    }
                    str8_list_push(scratch.arena, &full_file_path_parts, file_name);
                    StringJoin join = {0};
                    join.sep = str8_lit("/");
                    String8 full_file_path = str8_list_join(scratch.arena, &full_file_path_parts, &join);
                    file = d_entity_from_path(full_file_path, D_EntityFromPathFlag_AllowOverrides|D_EntityFromPathFlag_OpenAsNeeded|D_EntityFromPathFlag_OpenMissing);
                  }
                }
                
                // rjf: no folders specified => just try the local folder, then try globally
                else if(src_entity->kind == DF_EntityKind_File)
                {
                  file = df_entity_from_name_and_kind(file_name, DF_EntityKind_File);
                  if(df_entity_is_nil(file))
                  {
                    String8 src_entity_full_path = df_full_path_from_entity(scratch.arena, src_entity);
                    String8 src_entity_folder = str8_chop_last_slash(src_entity_full_path);
                    String8 estimated_full_path = push_str8f(scratch.arena, "%S/%S", src_entity_folder, file_name);
                    file = d_entity_from_path(estimated_full_path, D_EntityFromPathFlag_All);
                  }
                }
              }
              name_resolved = !df_entity_is_nil(file) && !(file->flags & DF_EntityFlag_IsMissing) && !(file->flags & DF_EntityFlag_IsFolder);
            }
#endif
            
            // rjf: process resolved info
            if(name_resolved == 0)
            {
              log_user_errorf("`%S` could not be found.", name);
            }
            
            // rjf: name resolved to voff * dbg info
            if(name_resolved != 0 && voff != 0)
            {
              D_LineList lines = d_lines_from_dbgi_key_voff(scratch.arena, &voff_dbgi_key, voff);
              if(lines.first != 0)
              {
                CTRL_Entity *process = &ctrl_entity_nil;
                U64 vaddr = 0;
                if(voff_dbgi_key.path.size != 0)
                {
                  CTRL_EntityList modules = ctrl_modules_from_dbgi_key(scratch.arena, d_state->ctrl_entity_store, &voff_dbgi_key);
                  CTRL_Entity *module = ctrl_entity_list_first(&modules);
                  process = ctrl_entity_ancestor_from_kind(module, CTRL_EntityKind_Process);
                  if(process != &ctrl_entity_nil)
                  {
                    vaddr = module->vaddr_range.min + lines.first->v.voff_range.min;
                  }
                }
                df_cmd(DF_CmdKind_FindCodeLocation,
                       .file_path = lines.first->v.file_path,
                       .cursor    = lines.first->v.pt,
                       .process   = process->handle,
                       .module    = module->handle,
                       .vaddr     = module->vaddr_range.min + lines.first->v.voff_range.min);
              }
            }
            
            // rjf: name resolved to a file
#if 0 // TODO(rjf): @msgs
            if(name_resolved != 0 && !df_entity_is_nil(file))
            {
              String8 path = df_full_path_from_entity(scratch.arena, file);
              D_CmdParams p = *params;
              p.file_path = path;
              p.text_point = txt_pt(1, 1);
              df_push_cmd(df_cmd_spec_from_kind(DF_CmdKind_FindCodeLocation), &p);
            }
#endif
          }
        }break;
        
        //- rjf: editors
        case DF_CmdKind_EditEntity:
        {
#if 0 // TODO(rjf): @msgs
          DF_Entity *entity = df_entity_from_handle(df_regs()->entity);
          switch(entity->kind)
          {
            default: break;
            case DF_EntityKind_Target:
            {
              df_push_cmd(df_cmd_spec_from_kind(DF_CmdKind_EditTarget), params);
            }break;
          }
#endif
        }break;
        
        //- rjf: targets
        case DF_CmdKind_EditTarget:
        {
#if 0 // TODO(rjf): @msgs
          DF_Entity *entity = df_entity_from_handle(df_regs()->entity);
          if(!df_entity_is_nil(entity) && entity->kind == DF_EntityKind_Target)
          {
            df_push_cmd(df_cmd_spec_from_kind(DF_CmdKind_Target), params);
          }
          else
          {
            log_user_errorf("Invalid target.");
          }
#endif
        }break;
        
        //- rjf: catchall general entity activation paths (drag/drop, clicking)
        case DF_CmdKind_EntityRefFastPath:
        {
          DF_Entity *entity = df_entity_from_handle(df_regs()->entity);
          switch(entity->kind)
          {
            default:
            {
              df_cmd(DF_CmdKind_SpawnEntityView, .entity = df_handle_from_entity(entity));
            }break;
            case DF_EntityKind_Thread:
            {
              df_cmd(DF_CmdKind_SelectThread, .entity = df_handle_from_entity(entity));
            }break;
            case DF_EntityKind_Target:
            {
              df_cmd(DF_CmdKind_SelectTarget, .entity = df_handle_from_entity(entity));
            }break;
          }
        }break;
        case DF_CmdKind_SpawnEntityView:
        {
#if 0 // TODO(rjf): @msgs
          DF_Window *ws = df_window_from_handle(df_regs()->window);
          DF_Panel *panel = df_panel_from_handle(df_regs()->panel);
          DF_Entity *entity = df_entity_from_handle(df_regs()->entity);
          switch(entity->kind)
          {
            default:{}break;
            
            case DF_EntityKind_Target:
            {
              D_CmdParams params = df_cmd_params_from_panel(ws, panel);
              params.entity = df_handle_from_entity(entity);
              df_push_cmd(df_cmd_spec_from_kind(DF_CmdKind_EditTarget), &params);
            }break;
          }
#endif
        }break;
        case DF_CmdKind_FindCodeLocation:
        {
          // NOTE(rjf): This command is where a lot of high-level flow things
          // in the debugger come together. It's that codepath that runs any
          // time a source code location is clicked in the UI, when a thread
          // is selected, or when a thread causes a halt (hitting a breakpoint
          // or exception or something). This is the logic that manages the
          // flow of how views and panels are changed, opened, etc. when
          // something like that happens.
          //
          // The gist of the intended rule for textual source code locations
          // is the following:
          //
          // 1. Try to find a panel that's viewing the file (has it open in a
          //    tab, *and* that tab is selected).
          // 2. Try to find a panel that has the file open in a tab, but does not
          //    currently have that tab selected.
          // 3. Try to find a panel that has ANY source code open in any tab.
          // 4. If the above things fail, try to pick the biggest panel, which
          //    is generally a decent rule (because it matches the popular
          //    debugger usage UI paradigm).
          //
          // The reason why this is a little more complicated than you might
          // imagine is because this debugger frontend does not have any special
          // "code panels" or anything like that, unlike e.g. VS or Remedy. All
          // panels are identical in nature to allow for the user to organize
          // the interface how they want, but in cases like this, we have to
          // "fish out" the best option given the user's configuration. This
          // can't be what the user wants in 100% of cases (this program cannot
          // read anyone's mind), but it does provide expected behavior in
          // common cases.
          //
          // The gist of the intended rule for finding disassembly locations is
          // the following:
          //
          // 1. Try to find a panel that's viewing disassembly already - if so,
          //    snap it to the right address.
          // 2. If there is no disassembly tab open, then we need to open one
          //    ONLY if source code was not found.
          // 3. If we need to open a disassembly tab, we will first try to pick
          //    the biggest empty panel.
          // 4. If there is no empty panel, then we will pick the biggest
          //    panel.
          DF_Window *ws = df_window_from_handle(df_regs()->window);
          
          // rjf: grab things to find. path * point, process * address, etc.
          String8 file_path = {0};
          TxtPt point = {0};
          CTRL_Entity *thread = &ctrl_entity_nil;
          CTRL_Entity *process = &ctrl_entity_nil;
          U64 vaddr = 0;
          {
            file_path = df_regs()->file_path;
            point     = df_regs()->cursor;
            thread    = ctrl_entity_from_handle(d_state->ctrl_entity_store, df_regs()->thread);
            process   = ctrl_entity_from_handle(d_state->ctrl_entity_store, df_regs()->process);
            vaddr     = df_regs()->vaddr;
          }
          
          // rjf: given a src code location, if no vaddr is specified,
          // try to map the src coordinates to a vaddr via line info
          if(vaddr == 0 && file_path.size != 0)
          {
            D_LineList lines = d_lines_from_file_path_line_num(scratch.arena, file_path, point.line);
            for(D_LineNode *n = lines.first; n != 0; n = n->next)
            {
              CTRL_EntityList modules = ctrl_modules_from_dbgi_key(scratch.arena, d_state->ctrl_entity_store, &n->v.dbgi_key);
              CTRL_Entity *module = ctrl_module_from_thread_candidates(d_state->ctrl_entity_store, thread, &modules);
              vaddr = ctrl_vaddr_from_voff(module, n->v.voff_range.min);
              break;
            }
          }
          
          // rjf: first, try to find panel/view pair that already has the src file open
          DF_Panel *panel_w_this_src_code = &df_nil_panel;
          DF_View *view_w_this_src_code = &df_nil_view;
          for(DF_Panel *panel = ws->root_panel; !df_panel_is_nil(panel); panel = df_panel_rec_df_pre(panel).next)
          {
            if(!df_panel_is_nil(panel->first))
            {
              continue;
            }
            for(DF_View *view = panel->first_tab_view; !df_view_is_nil(view); view = view->order_next)
            {
              if(df_view_is_project_filtered(view)) { continue; }
              String8 view_file_path = d_file_path_from_eval_string(scratch.arena, str8(view->query_buffer, view->query_string_size));
              DF_ViewKind view_kind = df_view_kind_from_string(view->spec->info.name);
              if((view_kind == DF_ViewKind_Text || view_kind == DF_ViewKind_PendingFile) &&
                 path_match_normalized(view_file_path, file_path))
              {
                panel_w_this_src_code = panel;
                view_w_this_src_code = view;
                if(view == df_selected_tab_from_panel(panel))
                {
                  break;
                }
              }
            }
          }
          
          // rjf: find a panel that already has *any* code open
          DF_Panel *panel_w_any_src_code = &df_nil_panel;
          for(DF_Panel *panel = ws->root_panel; !df_panel_is_nil(panel); panel = df_panel_rec_df_pre(panel).next)
          {
            if(!df_panel_is_nil(panel->first))
            {
              continue;
            }
            for(DF_View *view = panel->first_tab_view; !df_view_is_nil(view); view = view->order_next)
            {
              if(df_view_is_project_filtered(view)) { continue; }
              DF_ViewKind view_kind = df_view_kind_from_string(view->spec->info.name);
              if(view_kind == DF_ViewKind_Text)
              {
                panel_w_any_src_code = panel;
                break;
              }
            }
          }
          
          // rjf: try to find panel/view pair that has disassembly open
          DF_Panel *panel_w_disasm = &df_nil_panel;
          DF_View *view_w_disasm = &df_nil_view;
          for(DF_Panel *panel = ws->root_panel; !df_panel_is_nil(panel); panel = df_panel_rec_df_pre(panel).next)
          {
            if(!df_panel_is_nil(panel->first))
            {
              continue;
            }
            for(DF_View *view = panel->first_tab_view; !df_view_is_nil(view); view = view->order_next)
            {
              if(df_view_is_project_filtered(view)) { continue; }
              DF_ViewKind view_kind = df_view_kind_from_string(view->spec->info.name);
              if(view_kind == DF_ViewKind_Disasm && view->query_string_size == 0)
              {
                panel_w_disasm = panel;
                view_w_disasm = view;
                if(view == df_selected_tab_from_panel(panel))
                {
                  break;
                }
              }
            }
          }
          
          // rjf: find the biggest panel
          DF_Panel *biggest_panel = &df_nil_panel;
          {
            Rng2F32 root_rect = os_client_rect_from_window(ws->os);
            F32 best_panel_area = 0;
            for(DF_Panel *panel = ws->root_panel; !df_panel_is_nil(panel); panel = df_panel_rec_df_pre(panel).next)
            {
              if(!df_panel_is_nil(panel->first))
              {
                continue;
              }
              Rng2F32 panel_rect = df_target_rect_from_panel(root_rect, ws->root_panel, panel);
              Vec2F32 panel_rect_dim = dim_2f32(panel_rect);
              F32 area = panel_rect_dim.x * panel_rect_dim.y;
              if((best_panel_area == 0 || area > best_panel_area))
              {
                best_panel_area = area;
                biggest_panel = panel;
              }
            }
          }
          
          // rjf: find the biggest empty panel
          DF_Panel *biggest_empty_panel = &df_nil_panel;
          {
            Rng2F32 root_rect = os_client_rect_from_window(ws->os);
            F32 best_panel_area = 0;
            for(DF_Panel *panel = ws->root_panel; !df_panel_is_nil(panel); panel = df_panel_rec_df_pre(panel).next)
            {
              if(!df_panel_is_nil(panel->first))
              {
                continue;
              }
              Rng2F32 panel_rect = df_target_rect_from_panel(root_rect, ws->root_panel, panel);
              Vec2F32 panel_rect_dim = dim_2f32(panel_rect);
              F32 area = panel_rect_dim.x * panel_rect_dim.y;
              B32 panel_is_empty = 1;
              for(DF_View *v = panel->first_tab_view; !df_view_is_nil(v); v = v->order_next)
              {
                if(!df_view_is_project_filtered(v))
                {
                  panel_is_empty = 0;
                  break;
                }
              }
              if(panel_is_empty && (best_panel_area == 0 || area > best_panel_area))
              {
                best_panel_area = area;
                biggest_empty_panel = panel;
              }
            }
          }
          
          // rjf: given the above, find source code location.
          B32 disasm_view_prioritized = 0;
          DF_Panel *panel_used_for_src_code = &df_nil_panel;
          if(file_path.size != 0)
          {
            // rjf: determine which panel we will use to find the code loc
            DF_Panel *dst_panel = &df_nil_panel;
            {
              if(df_panel_is_nil(dst_panel)) { dst_panel = panel_w_this_src_code; }
              if(df_panel_is_nil(dst_panel)) { dst_panel = panel_w_any_src_code; }
              if(df_panel_is_nil(dst_panel)) { dst_panel = biggest_empty_panel; }
              if(df_panel_is_nil(dst_panel)) { dst_panel = biggest_panel; }
            }
            
            // rjf: construct new view if needed
            DF_View *dst_view = view_w_this_src_code;
            if(!df_panel_is_nil(dst_panel) && df_view_is_nil(view_w_this_src_code))
            {
              DF_View *view = df_view_alloc();
              String8 file_path_query = d_eval_string_from_file_path(scratch.arena, file_path);
              df_view_equip_spec(view, df_view_spec_from_kind(DF_ViewKind_Text), file_path_query, &md_nil_node);
              df_panel_insert_tab_view(dst_panel, dst_panel->last_tab_view, view);
              dst_view = view;
            }
            
            // rjf: determine if we need a contain or center
            DF_CmdKind cursor_snap_kind = DF_CmdKind_CenterCursor;
            if(!df_panel_is_nil(dst_panel) && dst_view == view_w_this_src_code && df_selected_tab_from_panel(dst_panel) == dst_view)
            {
              cursor_snap_kind = DF_CmdKind_ContainCursor;
            }
            
            // rjf: move cursor & snap-to-cursor
            if(!df_panel_is_nil(dst_panel))
            {
              disasm_view_prioritized = (df_selected_tab_from_panel(dst_panel) == view_w_disasm);
              dst_panel->selected_tab_view = df_handle_from_view(dst_view);
              df_cmd(DF_CmdKind_GoToLine, .cursor = point);
              df_cmd(cursor_snap_kind);
              panel_used_for_src_code = dst_panel;
            }
          }
          
          // rjf: given the above, find disassembly location.
          if(process != &ctrl_entity_nil && vaddr != 0)
          {
            // rjf: determine which panel we will use to find the disasm loc -
            // we *cannot* use the same panel we used for source code, if any.
            DF_Panel *dst_panel = &df_nil_panel;
            {
              if(df_panel_is_nil(dst_panel)) { dst_panel = panel_w_disasm; }
              if(df_panel_is_nil(panel_used_for_src_code) && df_panel_is_nil(dst_panel)) { dst_panel = biggest_empty_panel; }
              if(df_panel_is_nil(panel_used_for_src_code) && df_panel_is_nil(dst_panel)) { dst_panel = biggest_panel; }
              if(dst_panel == panel_used_for_src_code &&
                 !disasm_view_prioritized)
              {
                dst_panel = &df_nil_panel;
              }
            }
            
            // rjf: construct new view if needed
            DF_View *dst_view = view_w_disasm;
            if(!df_panel_is_nil(dst_panel) && df_view_is_nil(view_w_disasm))
            {
              DF_View *view = df_view_alloc();
              df_view_equip_spec(view, df_view_spec_from_kind(DF_ViewKind_Disasm), str8_zero(), &md_nil_node);
              df_panel_insert_tab_view(dst_panel, dst_panel->last_tab_view, view);
              dst_view = view;
            }
            
            // rjf: determine if we need a contain or center
            DF_CmdKind cursor_snap_kind = DF_CmdKind_CenterCursor;
            if(dst_view == view_w_disasm && df_selected_tab_from_panel(dst_panel) == dst_view)
            {
              cursor_snap_kind = DF_CmdKind_ContainCursor;
            }
            
            // rjf: move cursor & snap-to-cursor
            if(!df_panel_is_nil(dst_panel))
            {
              dst_panel->selected_tab_view = df_handle_from_view(dst_view);
              df_cmd(DF_CmdKind_GoToAddress, .process = process->handle, .vaddr = vaddr);
              df_cmd(cursor_snap_kind);
            }
          }
        }break;
        
        //- rjf: filtering
        case DF_CmdKind_Filter:
        {
          DF_View *view = df_view_from_handle(df_regs()->view);
          DF_Panel *panel = df_panel_from_handle(df_regs()->panel);
          B32 view_is_tab = 0;
          for(DF_View *tab = panel->first_tab_view; !df_view_is_nil(tab); tab = tab->order_next)
          {
            if(df_view_is_project_filtered(tab)) { continue; }
            if(tab == view)
            {
              view_is_tab = 1;
              break;
            }
          }
          if(view_is_tab && view->spec->info.flags & DF_ViewSpecFlag_CanFilter)
          {
            view->is_filtering ^= 1;
            view->query_cursor = txt_pt(1, 1+(S64)view->query_string_size);
            view->query_mark = txt_pt(1, 1);
          }
        }break;
        case DF_CmdKind_ClearFilter:
        {
          DF_View *view = df_view_from_handle(df_regs()->view);
          if(!df_view_is_nil(view))
          {
            view->query_string_size = 0;
            view->is_filtering = 0;
            view->query_cursor = view->query_mark = txt_pt(1, 1);
          }
        }break;
        case DF_CmdKind_ApplyFilter:
        {
          DF_View *view = df_view_from_handle(df_regs()->view);
          if(!df_view_is_nil(view))
          {
            view->is_filtering = 0;
          }
        }break;
        
        //- rjf: query completion
        case DF_CmdKind_CompleteQuery:
        {
          DF_Window *ws = df_window_from_handle(df_regs()->window);
          String8 query_cmd_name = ws->query_cmd_name;
          DF_CmdKindInfo *info = df_cmd_kind_info_from_string(query_cmd_name);
          DF_RegSlot slot = info->query.slot;
          
          // rjf: compound command parameters
          if(slot != DF_RegSlot_Null && !(ws->query_cmd_regs_mask[slot/64] & (1ull<<(slot%64))))
          {
            DF_Regs *regs_copy = df_regs_copy(ws->query_cmd_arena, df_regs());
            Rng1U64 offset_range_in_regs = df_reg_slot_range_table[slot];
            MemoryCopy((U8 *)(ws->query_cmd_regs) + offset_range_in_regs.min,
                       (U8 *)(regs_copy) + offset_range_in_regs.min,
                       dim_1u64(offset_range_in_regs));
            ws->query_cmd_regs_mask[slot/64] |= (1ull<<(slot%64));
          }
          
          // rjf: determine if command is ready to run
          B32 command_ready = 1;
          if(slot != DF_RegSlot_Null && !(ws->query_cmd_regs_mask[slot/64] & (1ull<<(slot%64))))
          {
            command_ready = 0;
          }
          
          // rjf: end this query
          if(!(info->query.flags & DF_QueryFlag_KeepOldInput))
          {
            df_cmd(DF_CmdKind_CancelQuery);
          }
          
          // rjf: push command if possible
          if(command_ready)
          {
            df_push_cmd(ws->query_cmd_name, ws->query_cmd_regs);
          }
        }break;
        case DF_CmdKind_CancelQuery:
        {
          DF_Window *ws = df_window_from_handle(df_regs()->window);
          arena_clear(ws->query_cmd_arena);
          MemoryZeroStruct(&ws->query_cmd_name);
          ws->query_cmd_regs = 0;
          MemoryZeroArray(ws->query_cmd_regs_mask);
          for(DF_View *v = ws->query_view_stack_top, *next = 0; !df_view_is_nil(v); v = next)
          {
            next = v->order_next;
            df_view_release(v);
          }
          ws->query_view_stack_top = &df_nil_view;
        }break;
        
        //- rjf: developer commands
        case DF_CmdKind_ToggleDevMenu:
        {
          DF_Window *ws = df_window_from_handle(df_regs()->window);
          ws->dev_menu_is_open ^= 1;
        }break;
        
        //- rjf: general entity operations
        case DF_CmdKind_EnableEntity:
        case DF_CmdKind_EnableBreakpoint:
        case DF_CmdKind_EnableTarget:
        {
          DF_Entity *entity = df_entity_from_handle(df_regs()->entity);
          df_entity_equip_disabled(entity, 0);
        }break;
        case DF_CmdKind_DisableEntity:
        case DF_CmdKind_DisableBreakpoint:
        case DF_CmdKind_DisableTarget:
        {
          DF_Entity *entity = df_entity_from_handle(df_regs()->entity);
          df_entity_equip_disabled(entity, 1);
        }break;
        case DF_CmdKind_RemoveEntity:
        case DF_CmdKind_RemoveBreakpoint:
        case DF_CmdKind_RemoveTarget:
        {
          DF_Entity *entity = df_entity_from_handle(df_regs()->entity);
          DF_EntityKindFlags kind_flags = d_entity_kind_flags_table[entity->kind];
          if(kind_flags & DF_EntityKindFlag_CanDelete)
          {
            df_entity_mark_for_deletion(entity);
          }
        }break;
        case DF_CmdKind_NameEntity:
        {
          DF_Entity *entity = df_entity_from_handle(df_regs()->entity);
          String8 string = df_regs()->string;
          df_entity_equip_name(entity, string);
        }break;
        case DF_CmdKind_DuplicateEntity:
        {
          DF_Entity *src = df_entity_from_handle(df_regs()->entity);
          if(!df_entity_is_nil(src))
          {
            typedef struct Task Task;
            struct Task
            {
              Task *next;
              DF_Entity *src_n;
              DF_Entity *dst_parent;
            };
            Task starter_task = {0, src, src->parent};
            Task *first_task = &starter_task;
            Task *last_task = &starter_task;
            for(Task *task = first_task; task != 0; task = task->next)
            {
              DF_Entity *src_n = task->src_n;
              DF_Entity *dst_n = df_entity_alloc(task->dst_parent, task->src_n->kind);
              if(src_n->flags & DF_EntityFlag_HasTextPoint)    {df_entity_equip_txt_pt(dst_n, src_n->text_point);}
              if(src_n->flags & DF_EntityFlag_HasU64)          {df_entity_equip_u64(dst_n, src_n->u64);}
              if(src_n->flags & DF_EntityFlag_HasColor)        {df_entity_equip_color_hsva(dst_n, df_hsva_from_entity(src_n));}
              if(src_n->flags & DF_EntityFlag_HasVAddrRng)     {df_entity_equip_vaddr_rng(dst_n, src_n->vaddr_rng);}
              if(src_n->flags & DF_EntityFlag_HasVAddr)        {df_entity_equip_vaddr(dst_n, src_n->vaddr);}
              if(src_n->disabled)                             {df_entity_equip_disabled(dst_n, 1);}
              if(src_n->string.size != 0)                     {df_entity_equip_name(dst_n, src_n->string);}
              dst_n->cfg_src = src_n->cfg_src;
              for(DF_Entity *src_child = task->src_n->first; !df_entity_is_nil(src_child); src_child = src_child->next)
              {
                Task *child_task = push_array(scratch.arena, Task, 1);
                child_task->src_n = src_child;
                child_task->dst_parent = dst_n;
                SLLQueuePush(first_task, last_task, child_task);
              }
            }
          }
        }break;
        case DF_CmdKind_RelocateEntity:
        {
          DF_Entity *entity = df_entity_from_handle(df_regs()->entity);
          DF_Entity *location = df_entity_child_from_kind(entity, DF_EntityKind_Location);
          if(df_entity_is_nil(location))
          {
            location = df_entity_alloc(entity, DF_EntityKind_Location);
          }
          location->flags &= ~DF_EntityFlag_HasTextPoint;
          location->flags &= ~DF_EntityFlag_HasVAddr;
          if(df_regs()->cursor.line != 0)
          {
            df_entity_equip_txt_pt(location, df_regs()->cursor);
          }
          if(df_regs()->vaddr != 0)
          {
            df_entity_equip_vaddr(location, df_regs()->vaddr);
          }
          if(df_regs()->file_path.size != 0)
          {
            df_entity_equip_name(location, df_regs()->file_path);
          }
        }break;
        
        //- rjf: breakpoints
        case DF_CmdKind_AddBreakpoint:
        case DF_CmdKind_ToggleBreakpoint:
        {
          String8 file_path = df_regs()->file_path;
          TxtPt pt = df_regs()->cursor;
          String8 string = df_regs()->string;
          U64 vaddr = df_regs()->vaddr;
          B32 removed_already_existing = 0;
          if(kind == DF_CmdKind_ToggleBreakpoint)
          {
            DF_EntityList bps = d_query_cached_entity_list_with_kind(DF_EntityKind_Breakpoint);
            for(DF_EntityNode *n = bps.first; n != 0; n = n->next)
            {
              DF_Entity *bp = n->entity;
              DF_Entity *loc = df_entity_child_from_kind(bp, DF_EntityKind_Location);
              if((loc->flags & DF_EntityFlag_HasTextPoint && path_match_normalized(loc->string, file_path) && loc->text_point.line == pt.line) ||
                 (loc->flags & DF_EntityFlag_HasVAddr && loc->vaddr == vaddr) ||
                 (!(loc->flags & DF_EntityFlag_HasTextPoint) && str8_match(loc->string, string, 0)))
              {
                df_entity_mark_for_deletion(bp);
                removed_already_existing = 1;
                break;
              }
            }
          }
          if(!removed_already_existing)
          {
            DF_Entity *bp = df_entity_alloc(df_entity_root(), DF_EntityKind_Breakpoint);
            df_entity_equip_cfg_src(bp, D_CfgSrc_Project);
            DF_Entity *loc = df_entity_alloc(bp, DF_EntityKind_Location);
            if(file_path.size != 0 && pt.line != 0)
            {
              df_entity_equip_name(loc, file_path);
              df_entity_equip_txt_pt(loc, pt);
            }
            else if(string.size != 0)
            {
              df_entity_equip_name(loc, string);
            }
            else if(vaddr != 0)
            {
              df_entity_equip_vaddr(loc, vaddr);
            }
          }
        }break;
        case DF_CmdKind_AddAddressBreakpoint:
        case DF_CmdKind_AddFunctionBreakpoint:
        {
          df_cmd(DF_CmdKind_AddBreakpoint);
        }break;
        
        //- rjf: watch pins
        case DF_CmdKind_AddWatchPin:
        case DF_CmdKind_ToggleWatchPin:
        {
          String8 file_path = df_regs()->file_path;
          TxtPt pt = df_regs()->cursor;
          String8 string = df_regs()->string;
          U64 vaddr = df_regs()->vaddr;
          B32 removed_already_existing = 0;
          if(kind == DF_CmdKind_ToggleWatchPin)
          {
            DF_EntityList wps = d_query_cached_entity_list_with_kind(DF_EntityKind_WatchPin);
            for(DF_EntityNode *n = wps.first; n != 0; n = n->next)
            {
              DF_Entity *wp = n->entity;
              DF_Entity *loc = df_entity_child_from_kind(wp, DF_EntityKind_Location);
              if((loc->flags & DF_EntityFlag_HasTextPoint && path_match_normalized(loc->string, file_path) && loc->text_point.line == pt.line) ||
                 (loc->flags & DF_EntityFlag_HasVAddr && loc->vaddr == vaddr) ||
                 (!(loc->flags & DF_EntityFlag_HasTextPoint) && str8_match(loc->string, string, 0)))
              {
                df_entity_mark_for_deletion(wp);
                removed_already_existing = 1;
                break;
              }
            }
          }
          if(!removed_already_existing)
          {
            DF_Entity *wp = df_entity_alloc(df_entity_root(), DF_EntityKind_WatchPin);
            df_entity_equip_name(wp, string);
            df_entity_equip_cfg_src(wp, D_CfgSrc_Project);
            DF_Entity *loc = df_entity_alloc(wp, DF_EntityKind_Location);
            if(file_path.size != 0 && pt.line != 0)
            {
              df_entity_equip_name(loc, file_path);
              df_entity_equip_txt_pt(loc, pt);
            }
            else if(vaddr != 0)
            {
              df_entity_equip_vaddr(loc, vaddr);
            }
          }
        }break;
        
        //- rjf: watches
        case DF_CmdKind_ToggleWatchExpression:
        if(df_regs()->string.size != 0)
        {
          DF_Entity *existing_watch = df_entity_from_name_and_kind(df_regs()->string, DF_EntityKind_Watch);
          if(df_entity_is_nil(existing_watch))
          {
            DF_Entity *watch = &d_nil_entity;
            watch = df_entity_alloc(df_entity_root(), DF_EntityKind_Watch);
            df_entity_equip_cfg_src(watch, D_CfgSrc_Project);
            df_entity_equip_name(watch, df_regs()->string);
          }
          else
          {
            df_entity_mark_for_deletion(existing_watch);
          }
        }break;
        
        //- rjf: cursor operations
#if 0 // TODO(rjf): @msgs these should no longer be necessary; "at cursor" -> just run the command with whatever the registers have
        case DF_CmdKind_ToggleBreakpointAtCursor:
        {
          D_Regs *regs = df_regs();
          D_CmdParams p = d_cmd_params_zero();
          p.file_path  = regs->file_path;
          p.text_point = regs->cursor;
          p.vaddr      = regs->vaddr_range.min;
          df_push_cmd(df_cmd_spec_from_kind(DF_CmdKind_ToggleBreakpoint), &p);
        }break;
        case DF_CmdKind_ToggleWatchPinAtCursor:
        {
          D_Regs *regs = df_regs();
          D_CmdParams p = d_cmd_params_zero();
          p.file_path  = regs->file_path;
          p.text_point = regs->cursor;
          p.vaddr      = regs->vaddr_range.min;
          p.string     = df_regs()->string;
          df_push_cmd(df_cmd_spec_from_kind(DF_CmdKind_ToggleWatchPin), &p);
        }break;
#endif
        case DF_CmdKind_GoToNameAtCursor:
        case DF_CmdKind_ToggleWatchExpressionAtCursor:
        {
          HS_Scope *hs_scope = hs_scope_open();
          TXT_Scope *txt_scope = txt_scope_open();
          DF_Regs *regs = df_regs();
          U128 text_key = regs->text_key;
          TXT_LangKind lang_kind = regs->lang_kind;
          TxtRng range = txt_rng(regs->cursor, regs->mark);
          U128 hash = {0};
          TXT_TextInfo info = txt_text_info_from_key_lang(txt_scope, text_key, lang_kind, &hash);
          String8 data = hs_data_from_hash(hs_scope, hash);
          Rng1U64 expr_off_range = {0};
          if(range.min.column != range.max.column)
          {
            expr_off_range = r1u64(txt_off_from_info_pt(&info, range.min), txt_off_from_info_pt(&info, range.max));
          }
          else
          {
            expr_off_range = txt_expr_off_range_from_info_data_pt(&info, data, range.min);
          }
          String8 expr = str8_substr(data, expr_off_range);
          df_cmd((kind == DF_CmdKind_GoToNameAtCursor ? DF_CmdKind_GoToName :
                  kind == DF_CmdKind_ToggleWatchExpressionAtCursor ? DF_CmdKind_ToggleWatchExpression :
                  DF_CmdKind_GoToName),
                 .string = expr);
          txt_scope_close(txt_scope);
          hs_scope_close(hs_scope);
        }break;
        case DF_CmdKind_RunToCursor:
        {
          if(df_regs()->file_path.size != 0)
          {
            df_cmd(DF_CmdKind_RunToLine);
          }
          else
          {
            df_cmd(DF_CmdKind_RunToAddress);
          }
        }break;
        case DF_CmdKind_SetNextStatement:
        {
          CTRL_Entity *thread = ctrl_entity_from_handle(d_state->ctrl_entity_store, df_regs()->thread);
          String8 file_path = df_regs()->file_path;
          U64 new_rip_vaddr = df_regs()->vaddr_range.min;
          if(file_path.size != 0)
          {
            D_LineList *lines = &df_regs()->lines;
            for(D_LineNode *n = lines->first; n != 0; n = n->next)
            {
              CTRL_EntityList modules = ctrl_modules_from_dbgi_key(scratch.arena, d_state->ctrl_entity_store, &n->v.dbgi_key);
              CTRL_Entity *module = ctrl_module_from_thread_candidates(d_state->ctrl_entity_store, thread, &modules);
              if(module != &ctrl_entity_nil)
              {
                new_rip_vaddr = ctrl_vaddr_from_voff(module, n->v.voff_range.min);
                break;
              }
            }
          }
          d_cmd(D_CmdKind_SetThreadIP, .vaddr = new_rip_vaddr);
        }break;
        
        //- rjf: targets
        case DF_CmdKind_AddTarget:
        {
          // rjf: build target
          DF_Entity *entity = &d_nil_entity;
          entity = df_entity_alloc(df_entity_root(), DF_EntityKind_Target);
          df_entity_equip_disabled(entity, 1);
          df_entity_equip_cfg_src(entity, D_CfgSrc_Project);
          DF_Entity *exe = df_entity_alloc(entity, DF_EntityKind_Executable);
          df_entity_equip_name(exe, df_regs()->file_path);
          String8 working_dir = str8_chop_last_slash(df_regs()->file_path);
          if(working_dir.size != 0)
          {
            String8 working_dir_path = push_str8f(scratch.arena, "%S/", working_dir);
            DF_Entity *execution_path = df_entity_alloc(entity, DF_EntityKind_WorkingDirectory);
            df_entity_equip_name(execution_path, working_dir_path);
          }
          df_cmd(DF_CmdKind_EditTarget, .entity = df_handle_from_entity(entity));
          df_cmd(DF_CmdKind_SelectTarget, .entity = df_handle_from_entity(entity));
        }break;
        case DF_CmdKind_SelectTarget:
        {
          DF_Entity *entity = df_entity_from_handle(df_regs()->entity);
          if(entity->kind == DF_EntityKind_Target)
          {
            DF_EntityList all_targets = d_query_cached_entity_list_with_kind(DF_EntityKind_Target);
            B32 is_selected = !entity->disabled;
            for(DF_EntityNode *n = all_targets.first; n != 0; n = n->next)
            {
              DF_Entity *target = n->entity;
              df_entity_equip_disabled(target, 1);
            }
            if(!is_selected)
            {
              df_entity_equip_disabled(entity, 0);
            }
          }
        }break;
        
        //- rjf: jit-debugger registration
        case DF_CmdKind_RegisterAsJITDebugger:
        {
#if OS_WINDOWS
          char filename_cstr[MAX_PATH] = {0};
          GetModuleFileName(0, filename_cstr, sizeof(filename_cstr));
          String8 debugger_binary_path = str8_cstring(filename_cstr);
          String8 name8 = str8_lit("Debugger");
          String8 data8 = push_str8f(scratch.arena, "%S --jit_pid:%%ld --jit_code:%%ld --jit_addr:0x%%p", debugger_binary_path);
          String16 name16 = str16_from_8(scratch.arena, name8);
          String16 data16 = str16_from_8(scratch.arena, data8);
          B32 likely_not_in_admin_mode = 0;
          {
            HKEY reg_key = 0;
            LSTATUS status = 0;
            status = RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\WOW6432Node\\Microsoft\\Windows NT\\CurrentVersion\\AeDebug\\", 0, KEY_SET_VALUE, &reg_key);
            likely_not_in_admin_mode = (status == ERROR_ACCESS_DENIED);
            status = RegSetValueExW(reg_key, (LPCWSTR)name16.str, 0, REG_SZ, (BYTE *)data16.str, data16.size*sizeof(U16)+2);
            RegCloseKey(reg_key);
          }
          {
            HKEY reg_key = 0;
            LSTATUS status = 0;
            status = RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\AeDebug\\", 0, KEY_SET_VALUE, &reg_key);
            likely_not_in_admin_mode = (status == ERROR_ACCESS_DENIED);
            status = RegSetValueExW(reg_key, (LPCWSTR)name16.str, 0, REG_SZ, (BYTE *)data16.str, data16.size*sizeof(U16)+2);
            RegCloseKey(reg_key);
          }
          if(likely_not_in_admin_mode)
          {
            log_user_error(str8_lit("Could not register as the just-in-time debugger, access was denied; try running the debugger as administrator."));
          }
#else
          log_user_error(str8_lit("Registering as the just-in-time debugger is currently not supported on this system."));
#endif
        }break;
        
        //- rjf: developer commands
        case DF_CmdKind_LogMarker:
        {
          log_infof("\"#MARKER\"");
        }break;
        
        //- rjf: os event passthrough
        case DF_CmdKind_OSEvent:
        {
          OS_Event *os_event = df_regs()->os_event;
          DF_Window *ws = df_window_from_os_handle(os_event->window);
          if(os_event != 0 && ws != 0)
          {
            UI_Event ui_event = zero_struct;
            UI_EventKind kind = UI_EventKind_Null;
            {
              switch(os_event->kind)
              {
                default:{}break;
                case OS_EventKind_Press:     {kind = UI_EventKind_Press;}break;
                case OS_EventKind_Release:   {kind = UI_EventKind_Release;}break;
                case OS_EventKind_MouseMove: {kind = UI_EventKind_MouseMove;}break;
                case OS_EventKind_Text:      {kind = UI_EventKind_Text;}break;
                case OS_EventKind_Scroll:    {kind = UI_EventKind_Scroll;}break;
                case OS_EventKind_FileDrop:  {kind = UI_EventKind_FileDrop;}break;
              }
            }
            ui_event.kind         = kind;
            ui_event.key          = os_event->key;
            ui_event.modifiers    = os_event->flags;
            ui_event.string       = os_event->character ? str8_from_32(ui_build_arena(), str32(&os_event->character, 1)) : str8_zero();
            ui_event.paths        = str8_list_copy(ui_build_arena(), &os_event->strings);
            ui_event.pos          = os_event->pos;
            ui_event.delta_2f32   = os_event->delta;
            ui_event.timestamp_us = os_event->timestamp_us;
            ui_event_list_push(scratch.arena, &ws->ui_events, &ui_event);
          }
        }break;
        
        //- rjf: debug control context management operations
        case DF_CmdKind_SelectThread:
        {
          CTRL_Entity *thread = ctrl_entity_from_handle(d_state->ctrl_entity_store, df_regs()->thread);
          CTRL_Entity *process = ctrl_entity_ancestor_from_kind(thread, CTRL_EntityKind_Process);
          CTRL_Entity *module = ctrl_module_from_process_vaddr(process, ctrl_query_cached_rip_from_thread(d_state->ctrl_entity_store, thread->handle));
          CTRL_Entity *machine = ctrl_entity_ancestor_from_kind(process, CTRL_EntityKind_Machine);
          df_state->base_regs.v.unwind_count = 0;
          df_state->base_regs.v.inline_depth = 0;
          df_state->base_regs.v.thread  = thread->handle;
          df_state->base_regs.v.module  = module->handle;
          df_state->base_regs.v.process = process->handle;
          df_state->base_regs.v.machine = machine->handle;
          df_cmd(DF_CmdKind_FindThread, .thread = thread->handle);
        }break;
        case DF_CmdKind_SelectUnwind:
        {
          DI_Scope *di_scope = di_scope_open();
          CTRL_Entity *thread = ctrl_entity_from_handle(d_state->ctrl_entity_store, df_base_regs()->thread);
          CTRL_Entity *process = ctrl_entity_ancestor_from_kind(thread, CTRL_EntityKind_Process);
          CTRL_Unwind base_unwind = d_query_cached_unwind_from_thread(thread);
          D_Unwind rich_unwind = d_unwind_from_ctrl_unwind(scratch.arena, di_scope, process, &base_unwind);
          if(df_regs()->unwind_count < rich_unwind.frames.concrete_frame_count)
          {
            D_UnwindFrame *frame = &rich_unwind.frames.v[df_regs()->unwind_count];
            df_state->base_regs.v.unwind_count = df_regs()->unwind_count;
            df_state->base_regs.v.inline_depth = 0;
            if(df_regs()->inline_depth <= frame->inline_frame_count)
            {
              df_state->base_regs.v.inline_depth = df_regs()->inline_depth;
            }
          }
          df_cmd(DF_CmdKind_FindThread, .thread = thread->handle);
          di_scope_close(di_scope);
        }break;
        case DF_CmdKind_UpOneFrame:
        case DF_CmdKind_DownOneFrame:
        {
          DI_Scope *di_scope = di_scope_open();
          CTRL_Entity *thread = ctrl_entity_from_handle(d_state->ctrl_entity_store, df_base_regs()->thread);
          CTRL_Entity *process = ctrl_entity_ancestor_from_kind(thread, CTRL_EntityKind_Process);
          CTRL_Unwind base_unwind = d_query_cached_unwind_from_thread(thread);
          D_Unwind rich_unwind = d_unwind_from_ctrl_unwind(scratch.arena, di_scope, process, &base_unwind);
          U64 crnt_unwind_idx = df_state->base_regs.v.unwind_count;
          U64 crnt_inline_dpt = df_state->base_regs.v.inline_depth;
          U64 next_unwind_idx = crnt_unwind_idx;
          U64 next_inline_dpt = crnt_inline_dpt;
          if(crnt_unwind_idx < rich_unwind.frames.concrete_frame_count)
          {
            D_UnwindFrame *f = &rich_unwind.frames.v[crnt_unwind_idx];
            switch(kind)
            {
              default:{}break;
              case DF_CmdKind_UpOneFrame:
              {
                if(crnt_inline_dpt < f->inline_frame_count)
                {
                  next_inline_dpt += 1;
                }
                else if(crnt_unwind_idx > 0)
                {
                  next_unwind_idx -= 1;
                  next_inline_dpt = 0;
                }
              }break;
              case DF_CmdKind_DownOneFrame:
              {
                if(crnt_inline_dpt > 0)
                {
                  next_inline_dpt -= 1;
                }
                else if(crnt_unwind_idx < rich_unwind.frames.concrete_frame_count)
                {
                  next_unwind_idx += 1;
                  next_inline_dpt = (f+1)->inline_frame_count;
                }
              }break;
            }
          }
          df_cmd(DF_CmdKind_SelectUnwind,
                 .unwind_count = next_unwind_idx,
                 .inline_depth = next_inline_dpt);
          di_scope_close(di_scope);
        }break;
        
        //- rjf: meta controls
        case DF_CmdKind_Edit:
        {
          DF_Window *ws = df_window_from_handle(df_regs()->window);
          UI_Event evt = zero_struct;
          evt.kind       = UI_EventKind_Press;
          evt.slot       = UI_EventActionSlot_Edit;
          ui_event_list_push(scratch.arena, &ws->ui_events, &evt);
        }break;
        case DF_CmdKind_Accept:
        {
          DF_Window *ws = df_window_from_handle(df_regs()->window);
          UI_Event evt = zero_struct;
          evt.kind       = UI_EventKind_Press;
          evt.slot       = UI_EventActionSlot_Accept;
          ui_event_list_push(scratch.arena, &ws->ui_events, &evt);
        }break;
        case DF_CmdKind_Cancel:
        {
          DF_Window *ws = df_window_from_handle(df_regs()->window);
          UI_Event evt = zero_struct;
          evt.kind       = UI_EventKind_Press;
          evt.slot       = UI_EventActionSlot_Cancel;
          ui_event_list_push(scratch.arena, &ws->ui_events, &evt);
        }break;
        
        //- rjf: directional movement & text controls
        //
        // NOTE(rjf): These all get funneled into a separate intermediate that
        // can be used by the UI build phase for navigation and stuff, as well
        // as builder codepaths that want to use these controls to modify text.
        //
        case DF_CmdKind_MoveLeft:
        {
          DF_Window *ws = df_window_from_handle(df_regs()->window);
          UI_Event evt = zero_struct;
          evt.kind       = UI_EventKind_Navigate;
          evt.flags      = UI_EventFlag_PickSelectSide|UI_EventFlag_ZeroDeltaOnSelect|UI_EventFlag_ExplicitDirectional;
          evt.delta_unit = UI_EventDeltaUnit_Char;
          evt.delta_2s32 = v2s32(-1, +0);
          ui_event_list_push(scratch.arena, &ws->ui_events, &evt);
        }break;
        case DF_CmdKind_MoveRight:
        {
          DF_Window *ws = df_window_from_handle(df_regs()->window);
          UI_Event evt = zero_struct;
          evt.kind       = UI_EventKind_Navigate;
          evt.flags      = UI_EventFlag_PickSelectSide|UI_EventFlag_ZeroDeltaOnSelect|UI_EventFlag_ExplicitDirectional;
          evt.delta_unit = UI_EventDeltaUnit_Char;
          evt.delta_2s32 = v2s32(+1, +0);
          ui_event_list_push(scratch.arena, &ws->ui_events, &evt);
        }break;
        case DF_CmdKind_MoveUp:
        {
          DF_Window *ws = df_window_from_handle(df_regs()->window);
          UI_Event evt = zero_struct;
          evt.kind       = UI_EventKind_Navigate;
          evt.flags      = UI_EventFlag_ExplicitDirectional;
          evt.delta_unit = UI_EventDeltaUnit_Char;
          evt.delta_2s32 = v2s32(+0, -1);
          ui_event_list_push(scratch.arena, &ws->ui_events, &evt);
        }break;
        case DF_CmdKind_MoveDown:
        {
          DF_Window *ws = df_window_from_handle(df_regs()->window);
          UI_Event evt = zero_struct;
          evt.kind       = UI_EventKind_Navigate;
          evt.flags      = UI_EventFlag_ExplicitDirectional;
          evt.delta_unit = UI_EventDeltaUnit_Char;
          evt.delta_2s32 = v2s32(+0, +1);
          ui_event_list_push(scratch.arena, &ws->ui_events, &evt);
        }break;
        case DF_CmdKind_MoveLeftSelect:
        {
          DF_Window *ws = df_window_from_handle(df_regs()->window);
          UI_Event evt = zero_struct;
          evt.kind       = UI_EventKind_Navigate;
          evt.flags      = UI_EventFlag_KeepMark|UI_EventFlag_ExplicitDirectional;
          evt.delta_unit = UI_EventDeltaUnit_Char;
          evt.delta_2s32 = v2s32(-1, +0);
          ui_event_list_push(scratch.arena, &ws->ui_events, &evt);
        }break;
        case DF_CmdKind_MoveRightSelect:
        {
          DF_Window *ws = df_window_from_handle(df_regs()->window);
          UI_Event evt = zero_struct;
          evt.kind       = UI_EventKind_Navigate;
          evt.flags      = UI_EventFlag_KeepMark|UI_EventFlag_ExplicitDirectional;
          evt.delta_unit = UI_EventDeltaUnit_Char;
          evt.delta_2s32 = v2s32(+1, +0);
          ui_event_list_push(scratch.arena, &ws->ui_events, &evt);
        }break;
        case DF_CmdKind_MoveUpSelect:
        {
          DF_Window *ws = df_window_from_handle(df_regs()->window);
          UI_Event evt = zero_struct;
          evt.kind       = UI_EventKind_Navigate;
          evt.flags      = UI_EventFlag_KeepMark|UI_EventFlag_ExplicitDirectional;
          evt.delta_unit = UI_EventDeltaUnit_Char;
          evt.delta_2s32 = v2s32(+0, -1);
          ui_event_list_push(scratch.arena, &ws->ui_events, &evt);
        }break;
        case DF_CmdKind_MoveDownSelect:
        {
          DF_Window *ws = df_window_from_handle(df_regs()->window);
          UI_Event evt = zero_struct;
          evt.kind       = UI_EventKind_Navigate;
          evt.flags      = UI_EventFlag_KeepMark|UI_EventFlag_ExplicitDirectional;
          evt.delta_unit = UI_EventDeltaUnit_Char;
          evt.delta_2s32 = v2s32(+0, +1);
          ui_event_list_push(scratch.arena, &ws->ui_events, &evt);
        }break;
        case DF_CmdKind_MoveLeftChunk:
        {
          DF_Window *ws = df_window_from_handle(df_regs()->window);
          UI_Event evt = zero_struct;
          evt.kind       = UI_EventKind_Navigate;
          evt.flags      = UI_EventFlag_ExplicitDirectional;
          evt.delta_unit = UI_EventDeltaUnit_Word;
          evt.delta_2s32 = v2s32(-1, +0);
          ui_event_list_push(scratch.arena, &ws->ui_events, &evt);
        }break;
        case DF_CmdKind_MoveRightChunk:
        {
          DF_Window *ws = df_window_from_handle(df_regs()->window);
          UI_Event evt = zero_struct;
          evt.kind       = UI_EventKind_Navigate;
          evt.flags      = UI_EventFlag_ExplicitDirectional;
          evt.delta_unit = UI_EventDeltaUnit_Word;
          evt.delta_2s32 = v2s32(+1, +0);
          ui_event_list_push(scratch.arena, &ws->ui_events, &evt);
        }break;
        case DF_CmdKind_MoveUpChunk:
        {
          DF_Window *ws = df_window_from_handle(df_regs()->window);
          UI_Event evt = zero_struct;
          evt.kind       = UI_EventKind_Navigate;
          evt.flags      = UI_EventFlag_ExplicitDirectional;
          evt.delta_unit = UI_EventDeltaUnit_Word;
          evt.delta_2s32 = v2s32(+0, -1);
          ui_event_list_push(scratch.arena, &ws->ui_events, &evt);
        }break;
        case DF_CmdKind_MoveDownChunk:
        {
          DF_Window *ws = df_window_from_handle(df_regs()->window);
          UI_Event evt = zero_struct;
          evt.kind       = UI_EventKind_Navigate;
          evt.flags      = UI_EventFlag_ExplicitDirectional;
          evt.delta_unit = UI_EventDeltaUnit_Word;
          evt.delta_2s32 = v2s32(+0, +1);
          ui_event_list_push(scratch.arena, &ws->ui_events, &evt);
        }break;
        case DF_CmdKind_MoveUpPage:
        {
          DF_Window *ws = df_window_from_handle(df_regs()->window);
          UI_Event evt = zero_struct;
          evt.kind       = UI_EventKind_Navigate;
          evt.delta_unit = UI_EventDeltaUnit_Page;
          evt.delta_2s32 = v2s32(+0, -1);
          ui_event_list_push(scratch.arena, &ws->ui_events, &evt);
        }break;
        case DF_CmdKind_MoveDownPage:
        {
          DF_Window *ws = df_window_from_handle(df_regs()->window);
          UI_Event evt = zero_struct;
          evt.kind       = UI_EventKind_Navigate;
          evt.delta_unit = UI_EventDeltaUnit_Page;
          evt.delta_2s32 = v2s32(+0, +1);
          ui_event_list_push(scratch.arena, &ws->ui_events, &evt);
        }break;
        case DF_CmdKind_MoveUpWhole:
        {
          DF_Window *ws = df_window_from_handle(df_regs()->window);
          UI_Event evt = zero_struct;
          evt.kind       = UI_EventKind_Navigate;
          evt.delta_unit = UI_EventDeltaUnit_Whole;
          evt.delta_2s32 = v2s32(+0, -1);
          ui_event_list_push(scratch.arena, &ws->ui_events, &evt);
        }break;
        case DF_CmdKind_MoveDownWhole:
        {
          DF_Window *ws = df_window_from_handle(df_regs()->window);
          UI_Event evt = zero_struct;
          evt.kind       = UI_EventKind_Navigate;
          evt.delta_unit = UI_EventDeltaUnit_Whole;
          evt.delta_2s32 = v2s32(+0, +1);
          ui_event_list_push(scratch.arena, &ws->ui_events, &evt);
        }break;
        case DF_CmdKind_MoveLeftChunkSelect:
        {
          DF_Window *ws = df_window_from_handle(df_regs()->window);
          UI_Event evt = zero_struct;
          evt.kind       = UI_EventKind_Navigate;
          evt.flags      = UI_EventFlag_KeepMark|UI_EventFlag_ExplicitDirectional;
          evt.delta_unit = UI_EventDeltaUnit_Word;
          evt.delta_2s32 = v2s32(-1, +0);
          ui_event_list_push(scratch.arena, &ws->ui_events, &evt);
        }break;
        case DF_CmdKind_MoveRightChunkSelect:
        {
          DF_Window *ws = df_window_from_handle(df_regs()->window);
          UI_Event evt = zero_struct;
          evt.kind       = UI_EventKind_Navigate;
          evt.flags      = UI_EventFlag_KeepMark|UI_EventFlag_ExplicitDirectional;
          evt.delta_unit = UI_EventDeltaUnit_Word;
          evt.delta_2s32 = v2s32(+1, +0);
          ui_event_list_push(scratch.arena, &ws->ui_events, &evt);
        }break;
        case DF_CmdKind_MoveUpChunkSelect:
        {
          DF_Window *ws = df_window_from_handle(df_regs()->window);
          UI_Event evt = zero_struct;
          evt.kind       = UI_EventKind_Navigate;
          evt.flags      = UI_EventFlag_KeepMark|UI_EventFlag_ExplicitDirectional;
          evt.delta_unit = UI_EventDeltaUnit_Word;
          evt.delta_2s32 = v2s32(+0, -1);
          ui_event_list_push(scratch.arena, &ws->ui_events, &evt);
        }break;
        case DF_CmdKind_MoveDownChunkSelect:
        {
          DF_Window *ws = df_window_from_handle(df_regs()->window);
          UI_Event evt = zero_struct;
          evt.kind       = UI_EventKind_Navigate;
          evt.flags      = UI_EventFlag_KeepMark|UI_EventFlag_ExplicitDirectional;
          evt.delta_unit = UI_EventDeltaUnit_Word;
          evt.delta_2s32 = v2s32(+0, +1);
          ui_event_list_push(scratch.arena, &ws->ui_events, &evt);
        }break;
        case DF_CmdKind_MoveUpPageSelect:
        {
          DF_Window *ws = df_window_from_handle(df_regs()->window);
          UI_Event evt = zero_struct;
          evt.kind       = UI_EventKind_Navigate;
          evt.flags      = UI_EventFlag_KeepMark;
          evt.delta_unit = UI_EventDeltaUnit_Page;
          evt.delta_2s32 = v2s32(+0, -1);
          ui_event_list_push(scratch.arena, &ws->ui_events, &evt);
        }break;
        case DF_CmdKind_MoveDownPageSelect:
        {
          DF_Window *ws = df_window_from_handle(df_regs()->window);
          UI_Event evt = zero_struct;
          evt.kind       = UI_EventKind_Navigate;
          evt.flags      = UI_EventFlag_KeepMark;
          evt.delta_unit = UI_EventDeltaUnit_Page;
          evt.delta_2s32 = v2s32(+0, +1);
          ui_event_list_push(scratch.arena, &ws->ui_events, &evt);
        }break;
        case DF_CmdKind_MoveUpWholeSelect:
        {
          DF_Window *ws = df_window_from_handle(df_regs()->window);
          UI_Event evt = zero_struct;
          evt.kind       = UI_EventKind_Navigate;
          evt.flags      = UI_EventFlag_KeepMark;
          evt.delta_unit = UI_EventDeltaUnit_Whole;
          evt.delta_2s32 = v2s32(+0, -1);
          ui_event_list_push(scratch.arena, &ws->ui_events, &evt);
        }break;
        case DF_CmdKind_MoveDownWholeSelect:
        {
          DF_Window *ws = df_window_from_handle(df_regs()->window);
          UI_Event evt = zero_struct;
          evt.kind       = UI_EventKind_Navigate;
          evt.flags      = UI_EventFlag_KeepMark;
          evt.delta_unit = UI_EventDeltaUnit_Whole;
          evt.delta_2s32 = v2s32(+0, +1);
          ui_event_list_push(scratch.arena, &ws->ui_events, &evt);
        }break;
        case DF_CmdKind_MoveUpReorder:
        {
          DF_Window *ws = df_window_from_handle(df_regs()->window);
          UI_Event evt = zero_struct;
          evt.kind       = UI_EventKind_Navigate;
          evt.flags      = UI_EventFlag_Reorder;
          evt.delta_unit = UI_EventDeltaUnit_Char;
          evt.delta_2s32 = v2s32(+0, -1);
          ui_event_list_push(scratch.arena, &ws->ui_events, &evt);
        }break;
        case DF_CmdKind_MoveDownReorder:
        {
          DF_Window *ws = df_window_from_handle(df_regs()->window);
          UI_Event evt = zero_struct;
          evt.kind       = UI_EventKind_Navigate;
          evt.flags      = UI_EventFlag_Reorder;
          evt.delta_unit = UI_EventDeltaUnit_Char;
          evt.delta_2s32 = v2s32(+0, +1);
          ui_event_list_push(scratch.arena, &ws->ui_events, &evt);
        }break;
        case DF_CmdKind_MoveHome:
        {
          DF_Window *ws = df_window_from_handle(df_regs()->window);
          UI_Event evt = zero_struct;
          evt.kind       = UI_EventKind_Navigate;
          evt.delta_unit = UI_EventDeltaUnit_Line;
          evt.delta_2s32 = v2s32(-1, +0);
          ui_event_list_push(scratch.arena, &ws->ui_events, &evt);
        }break;
        case DF_CmdKind_MoveEnd:
        {
          DF_Window *ws = df_window_from_handle(df_regs()->window);
          UI_Event evt = zero_struct;
          evt.kind       = UI_EventKind_Navigate;
          evt.delta_unit = UI_EventDeltaUnit_Line;
          evt.delta_2s32 = v2s32(+1, +0);
          ui_event_list_push(scratch.arena, &ws->ui_events, &evt);
        }break;
        case DF_CmdKind_MoveHomeSelect:
        {
          DF_Window *ws = df_window_from_handle(df_regs()->window);
          UI_Event evt = zero_struct;
          evt.kind       = UI_EventKind_Navigate;
          evt.flags      = UI_EventFlag_KeepMark;
          evt.delta_unit = UI_EventDeltaUnit_Line;
          evt.delta_2s32 = v2s32(-1, +0);
          ui_event_list_push(scratch.arena, &ws->ui_events, &evt);
        }break;
        case DF_CmdKind_MoveEndSelect:
        {
          DF_Window *ws = df_window_from_handle(df_regs()->window);
          UI_Event evt = zero_struct;
          evt.kind       = UI_EventKind_Navigate;
          evt.flags      = UI_EventFlag_KeepMark;
          evt.delta_unit = UI_EventDeltaUnit_Line;
          evt.delta_2s32 = v2s32(+1, +0);
          ui_event_list_push(scratch.arena, &ws->ui_events, &evt);
        }break;
        case DF_CmdKind_SelectAll:
        {
          DF_Window *ws = df_window_from_handle(df_regs()->window);
          UI_Event evt1 = zero_struct;
          evt1.kind       = UI_EventKind_Navigate;
          evt1.delta_unit = UI_EventDeltaUnit_Whole;
          evt1.delta_2s32 = v2s32(-1, +0);
          ui_event_list_push(scratch.arena, &ws->ui_events, &evt1);
          UI_Event evt2 = zero_struct;
          evt2.kind       = UI_EventKind_Navigate;
          evt2.flags      = UI_EventFlag_KeepMark;
          evt2.delta_unit = UI_EventDeltaUnit_Whole;
          evt2.delta_2s32 = v2s32(+1, +0);
          ui_event_list_push(scratch.arena, &ws->ui_events, &evt2);
        }break;
        case DF_CmdKind_DeleteSingle:
        {
          DF_Window *ws = df_window_from_handle(df_regs()->window);
          UI_Event evt = zero_struct;
          evt.kind       = UI_EventKind_Edit;
          evt.flags      = UI_EventFlag_Delete;
          evt.delta_unit = UI_EventDeltaUnit_Char;
          evt.delta_2s32 = v2s32(+1, +0);
          ui_event_list_push(scratch.arena, &ws->ui_events, &evt);
        }break;
        case DF_CmdKind_DeleteChunk:
        {
          DF_Window *ws = df_window_from_handle(df_regs()->window);
          UI_Event evt = zero_struct;
          evt.kind       = UI_EventKind_Edit;
          evt.flags      = UI_EventFlag_Delete;
          evt.delta_unit = UI_EventDeltaUnit_Word;
          evt.delta_2s32 = v2s32(+1, +0);
          ui_event_list_push(scratch.arena, &ws->ui_events, &evt);
        }break;
        case DF_CmdKind_BackspaceSingle:
        {
          DF_Window *ws = df_window_from_handle(df_regs()->window);
          UI_Event evt = zero_struct;
          evt.kind       = UI_EventKind_Edit;
          evt.flags      = UI_EventFlag_Delete|UI_EventFlag_ZeroDeltaOnSelect;
          evt.delta_unit = UI_EventDeltaUnit_Char;
          evt.delta_2s32 = v2s32(-1, +0);
          ui_event_list_push(scratch.arena, &ws->ui_events, &evt);
        }break;
        case DF_CmdKind_BackspaceChunk:
        {
          DF_Window *ws = df_window_from_handle(df_regs()->window);
          UI_Event evt = zero_struct;
          evt.kind       = UI_EventKind_Edit;
          evt.flags      = UI_EventFlag_Delete;
          evt.delta_unit = UI_EventDeltaUnit_Word;
          evt.delta_2s32 = v2s32(-1, +0);
          ui_event_list_push(scratch.arena, &ws->ui_events, &evt);
        }break;
        case DF_CmdKind_Copy:
        {
          DF_Window *ws = df_window_from_handle(df_regs()->window);
          UI_Event evt = zero_struct;
          evt.kind  = UI_EventKind_Edit;
          evt.flags = UI_EventFlag_Copy|UI_EventFlag_KeepMark;
          ui_event_list_push(scratch.arena, &ws->ui_events, &evt);
        }break;
        case DF_CmdKind_Cut:
        {
          DF_Window *ws = df_window_from_handle(df_regs()->window);
          UI_Event evt = zero_struct;
          evt.kind  = UI_EventKind_Edit;
          evt.flags = UI_EventFlag_Copy|UI_EventFlag_Delete;
          ui_event_list_push(scratch.arena, &ws->ui_events, &evt);
        }break;
        case DF_CmdKind_Paste:
        {
          DF_Window *ws = df_window_from_handle(df_regs()->window);
          UI_Event evt = zero_struct;
          evt.kind   = UI_EventKind_Text;
          evt.string = os_get_clipboard_text(scratch.arena);
          ui_event_list_push(scratch.arena, &ws->ui_events, &evt);
        }break;
        case DF_CmdKind_InsertText:
        {
          DF_Window *ws = df_window_from_handle(df_regs()->window);
          UI_Event evt = zero_struct;
          evt.kind   = UI_EventKind_Text;
          evt.string = df_regs()->string;
          ui_event_list_push(scratch.arena, &ws->ui_events, &evt);
        }break;
      }
    }
  }
  
  //////////////////////////////
  //- rjf: clear commands
  //
  {
    arena_clear(df_state->cmds_arena);
    MemoryZeroStruct(&df_state->cmds);
  }
  
  //////////////////////////////
  //- rjf: gather targets
  //
  D_TargetArray targets = {0};
  {
    DF_EntityList target_entities = d_query_cached_entity_list_with_kind(DF_EntityKind_Target);
    targets.count = target_entities.count;
    targets.v = push_array(scratch.arena, D_Target, targets.count);
    U64 idx = 0;
    for(DF_EntityNode *n = target_entities.first; n != 0; n = n->next, idx += 1)
    {
      DF_Entity *src_target = n->entity;
      DF_Entity *src_target_exe   = df_entity_child_from_kind(src_target, DF_EntityKind_Executable);
      DF_Entity *src_target_args  = df_entity_child_from_kind(src_target, DF_EntityKind_Arguments);
      DF_Entity *src_target_wdir  = df_entity_child_from_kind(src_target, DF_EntityKind_WorkingDirectory);
      DF_Entity *src_target_entry = df_entity_child_from_kind(src_target, DF_EntityKind_EntryPoint);
      D_Target *dst_target = &targets.v[idx];
      dst_target->exe                     = src_target_exe->string;
      dst_target->args                    = src_target_args->string;
      dst_target->working_directory       = src_target_wdir->string;
      dst_target->custom_entry_point_name = src_target_entry->string;
    }
  }
  
  //////////////////////////////
  //- rjf: gather breakpoints
  //
  D_BreakpointArray breakpoints = {0};
  {
    DF_EntityList bp_entities = d_query_cached_entity_list_with_kind(DF_EntityKind_Breakpoint);
    breakpoints.count = bp_entities.count;
    breakpoints.v = push_array(scratch.arena, D_Breakpoint, breakpoints.count);
    U64 idx = 0;
    for(DF_EntityNode *n = bp_entities.first; n != 0; n = n->next)
    {
      DF_Entity *src_bp = n->entity;
      if(src_bp->disabled)
      {
        breakpoints.count -= 1;
        continue;
      }
      DF_Entity *src_bp_loc = df_entity_child_from_kind(src_bp, DF_EntityKind_Location);
      DF_Entity *src_bp_cnd = df_entity_child_from_kind(src_bp, DF_EntityKind_Condition);
      D_Breakpoint *dst_bp = &breakpoints.v[idx];
      dst_bp->file_path   = src_bp_loc->string;
      dst_bp->pt          = src_bp_loc->text_point;
      dst_bp->symbol_name = src_bp_loc->string;
      dst_bp->vaddr       = src_bp_loc->vaddr;
      dst_bp->condition   = src_bp_cnd->string;
      idx += 1;
    }
  }
  
  //////////////////////////////
  //- rjf: gather path maps
  //
  D_PathMapArray path_maps = {0};
  {
    // TODO(rjf): @msgs
  }
  
  //////////////////////////////
  //- rjf: gather exception code filters
  //
  U64 exception_code_filters[(CTRL_ExceptionCodeKind_COUNT+63)/64] = {0};
  {
    // TODO(rjf): @msgs
  }
  
  //////////////////////////////
  //- rjf: tick debug engine
  //
  D_EventList engine_events = d_tick(scratch.arena, &targets, &breakpoints, &path_maps, exception_code_filters);
  
  //////////////////////////////
  //- rjf: no selected thread? -> try to snap to any existing thread
  //
  if(ctrl_entity_from_handle(d_state->ctrl_entity_store, df_base_regs()->thread) == &ctrl_entity_nil)
  {
    CTRL_Entity *process = ctrl_entity_from_handle(d_state->ctrl_entity_store, df_base_regs()->process);
    if(process == &ctrl_entity_nil)
    {
      CTRL_EntityList all_processes = ctrl_entity_list_from_kind(d_state->ctrl_entity_store, CTRL_EntityKind_Process);
      if(all_processes.count != 0)
      {
        process = all_processes.first->v;
      }
    }
    CTRL_Entity *new_thread = ctrl_entity_child_from_kind(process, CTRL_EntityKind_Thread);
    if(new_thread != &ctrl_entity_nil)
    {
      df_cmd(DF_CmdKind_SelectThread, .thread = new_thread->handle);
    }
  }
  
  //////////////////////////////
  //- rjf: process debug engine events
  //
  for(D_EventNode *n = engine_events.first; n != 0; n = n->next)
  {
    D_Event *evt = &n->v;
    switch(evt->kind)
    {
      default:{}break;
      case D_EventKind_Stop:
      {
        CTRL_Entity *thread = ctrl_entity_from_handle(d_state->ctrl_entity_store, evt->thread);
        U64 vaddr = evt->vaddr;
        CTRL_Entity *process = ctrl_entity_ancestor_from_kind(thread, CTRL_EntityKind_Process);
        CTRL_Entity *module = ctrl_module_from_process_vaddr(process, vaddr);
        DI_Key dbgi_key = ctrl_dbgi_key_from_module(module);
        U64 voff = ctrl_voff_from_vaddr(module, vaddr);
        
        // rjf: valid stop thread? -> select & snap
        if(thread != &ctrl_entity_nil)
        {
          df_cmd(DF_CmdKind_SelectThread, .thread = thread->handle);
          df_cmd(DF_CmdKind_FindThread, .thread = thread->handle);
        }
        
        // rjf: no stop-causing thread, but have selected thread? -> snap to selected
        CTRL_Entity *selected_thread = &ctrl_entity_nil; // TODO(rjf): ctrl_entity_from_handle(d_state->ctrl_entity_store, df_base_regs()->thread);
        if(thread == &ctrl_entity_nil && selected_thread != &ctrl_entity_nil)
        {
          df_cmd(DF_CmdKind_FindThread);
        }
        
        // rjf: increment breakpoint hit counts
        if(evt->cause == D_EventCause_UserBreakpoint)
        {
          DF_EntityList user_bps = d_query_cached_entity_list_with_kind(DF_EntityKind_Breakpoint);
          for(DF_EntityNode *n = user_bps.first; n != 0; n = n->next)
          {
            DF_Entity *bp = n->entity;
            DF_Entity *loc = df_entity_child_from_kind(bp, DF_EntityKind_Location);
            D_LineList loc_lines = d_lines_from_file_path_line_num(scratch.arena, loc->string, loc->text_point.line);
            if(loc_lines.first != 0)
            {
              for(D_LineNode *n = loc_lines.first; n != 0; n = n->next)
              {
                if(contains_1u64(n->v.voff_range, voff))
                {
                  bp->u64 += 1;
                  break;
                }
              }
            }
            else if(loc->flags & DF_EntityFlag_HasVAddr && vaddr == loc->vaddr)
            {
              bp->u64 += 1;
            }
            else if(loc->string.size != 0)
            {
              U64 symb_voff = d_voff_from_dbgi_key_symbol_name(&dbgi_key, loc->string);
              if(symb_voff == voff)
              {
                bp->u64 += 1;
              }
            }
          }
        }
      }break;
    }
  }
  
  //////////////////////////////
  //- rjf: animate confirmation
  //
  {
    F32 rate = df_setting_val_from_code(DF_SettingCode_MenuAnimations).s32 ? 1 - pow_f32(2, (-10.f * df_state->frame_dt)) : 1.f;
    B32 confirm_open = df_state->confirm_active;
    df_state->confirm_t += rate * ((F32)!!confirm_open-df_state->confirm_t);
    if(abs_f32(df_state->confirm_t - (F32)!!confirm_open) > 0.005f)
    {
      df_request_frame();
    }
  }
  
  //////////////////////////////
  //- rjf: animate theme
  //
  {
    DF_Theme *current = &df_state->cfg_theme;
    DF_Theme *target = &df_state->cfg_theme_target;
    F32 rate = 1 - pow_f32(2, (-50.f * df_state->frame_dt));
    for(DF_ThemeColor color = DF_ThemeColor_Null;
        color < DF_ThemeColor_COUNT;
        color = (DF_ThemeColor)(color+1))
    {
      if(abs_f32(target->colors[color].x - current->colors[color].x) > 0.01f ||
         abs_f32(target->colors[color].y - current->colors[color].y) > 0.01f ||
         abs_f32(target->colors[color].z - current->colors[color].z) > 0.01f ||
         abs_f32(target->colors[color].w - current->colors[color].w) > 0.01f)
      {
        df_request_frame();
      }
      current->colors[color].x += (target->colors[color].x - current->colors[color].x) * rate;
      current->colors[color].y += (target->colors[color].y - current->colors[color].y) * rate;
      current->colors[color].z += (target->colors[color].z - current->colors[color].z) * rate;
      current->colors[color].w += (target->colors[color].w - current->colors[color].w) * rate;
    }
  }
  
  //////////////////////////////
  //- rjf: animate alive-transitions for entities
  //
  {
    F32 rate = 1.f - pow_f32(2.f, -20.f*df_state->frame_dt);
    for(DF_Entity *e = df_entity_root(); !df_entity_is_nil(e); e = df_entity_rec_depth_first_pre(e, df_entity_root()).next)
    {
      F32 diff = (1.f - e->alive_t);
      e->alive_t += diff * rate;
      if(diff >= 0.01f)
      {
        df_request_frame();
      }
    }
  }
  
  //////////////////////////////
  //- rjf: capture is active? -> keep rendering
  //
  if(ProfIsCapturing() || DEV_telemetry_capture)
  {
    df_request_frame();
  }
  
  //////////////////////////////
  //- rjf: commit params changes for all views
  //
  {
    for(DF_View *v = df_state->first_view; !df_view_is_nil(v); v = v->alloc_next)
    {
      if(v->params_write_gen == v->params_read_gen+1)
      {
        v->params_read_gen += 1;
      }
    }
  }
  
  //////////////////////////////
  //- rjf: queue drag drop (TODO(rjf): @msgs)
  //
  B32 queue_drag_drop = 0;
  if(queue_drag_drop)
  {
    df_queue_drag_drop();
  }
  
  //////////////////////////////
  //- rjf: update/render all windows
  //
  {
    dr_begin_frame();
    for(DF_Window *w = df_state->first_window; w != 0; w = w->next)
    {
      B32 window_is_focused = os_window_is_focused(w->os);
      if(window_is_focused)
      {
        df_state->last_focused_window = df_handle_from_window(w);
      }
      df_push_regs();
      df_regs()->window = df_handle_from_window(w);
      df_window_frame(w);
      MemoryZeroStruct(&w->ui_events);
      DF_Regs *window_regs = df_pop_regs();
      if(df_window_from_handle(df_state->last_focused_window) == w)
      {
        MemoryCopyStruct(df_regs(), window_regs);
      }
    }
  }
  
  //////////////////////////////
  //- rjf: simulate lag
  //
  if(DEV_simulate_lag)
  {
    os_sleep_milliseconds(300);
  }
  
  //////////////////////////////
  //- rjf: end drag/drop if needed
  //
  if(df_state->drag_drop_state == DF_DragDropState_Dropping)
  {
    df_state->drag_drop_state = DF_DragDropState_Null;
    MemoryZeroStruct(&df_drag_drop_payload);
  }
  
  //////////////////////////////
  //- rjf: clear frame request state
  //
  if(df_state->num_frames_requested > 0)
  {
    df_state->num_frames_requested -= 1;
  }
  
  //////////////////////////////
  //- rjf: submit rendering to all windows
  //
  {
    r_begin_frame();
    for(DF_Window *w = df_state->first_window; w != 0; w = w->next)
    {
      r_window_begin_frame(w->os, w->r);
      dr_submit_bucket(w->os, w->r, w->draw_bucket);
      r_window_end_frame(w->os, w->r);
    }
    r_end_frame();
  }
  
  //////////////////////////////
  //- rjf: show windows after first frame
  //
  if(depth == 0)
  {
    D_HandleList windows_to_show = {0};
    for(DF_Window *w = df_state->first_window; w != 0; w = w->next)
    {
      if(w->frames_alive == 1)
      {
        d_handle_list_push(scratch.arena, &windows_to_show, df_handle_from_window(w));
      }
    }
    for(D_HandleNode *n = windows_to_show.first; n != 0; n = n->next)
    {
      DF_Window *window = df_window_from_handle(n->handle);
      DeferLoop(depth += 1, depth -= 1) os_window_first_paint(window->os);
    }
  }
  
  //////////////////////////////
  //- rjf: eliminate entities that are marked for deletion
  //
  ProfScope("eliminate deleted entities")
  {
    for(DF_Entity *entity = df_entity_root(), *next = 0; !df_entity_is_nil(entity); entity = next)
    {
      next = df_entity_rec_depth_first_pre(entity, &d_nil_entity).next;
      if(entity->flags & DF_EntityFlag_MarkedForDeletion)
      {
        B32 undoable = (d_entity_kind_flags_table[entity->kind] & DF_EntityKindFlag_UserDefinedLifetime);
        
        // rjf: fixup next entity to iterate to
        next = df_entity_rec_depth_first(entity, &d_nil_entity, OffsetOf(DF_Entity, next), OffsetOf(DF_Entity, next)).next;
        
        // rjf: eliminate root entity if we're freeing it
        if(entity == df_state->entities_root)
        {
          df_state->entities_root = &d_nil_entity;
        }
        
        // rjf: unhook & release this entity tree
        df_entity_change_parent(entity, entity->parent, &d_nil_entity, &d_nil_entity);
        df_entity_release(entity);
      }
    }
  }
  
  //////////////////////////////
  //- rjf: determine frame time, record into history
  //
  U64 end_time_us = os_now_microseconds();
  U64 frame_time_us = end_time_us-begin_time_us;
  df_state->frame_time_us_history[df_state->frame_index%ArrayCount(df_state->frame_time_us_history)] = frame_time_us;
  
  //////////////////////////////
  //- rjf: bump frame time counters
  //
  df_state->frame_index += 1;
  df_state->time_in_seconds += df_state->frame_dt;
  
  //////////////////////////////
  //- rjf: collect logs
  //
  {
    LogScopeResult log = log_scope_end(scratch.arena);
    os_append_data_to_file_path(df_state->log_path, log.strings[LogMsgKind_Info]);
    if(log.strings[LogMsgKind_UserError].size != 0)
    {
      for(DF_Window *w = df_state->first_window; w != 0; w = w->next)
      {
        w->error_string_size = Min(sizeof(w->error_buffer), log.strings[LogMsgKind_UserError].size);
        MemoryCopy(w->error_buffer, log.strings[LogMsgKind_UserError].str, w->error_string_size);
        w->error_t = 1.f;
      }
    }
  }
  
  di_scope_close(di_scope);
  scratch_end(scratch);
}
