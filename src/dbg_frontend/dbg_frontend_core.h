// Copyright (c) 2024 Epic Games Tools
// Licensed under the MIT license (https://opensource.org/license/mit/)

#ifndef DBG_FRONTEND_CORE_H
#define DBG_FRONTEND_CORE_H

////////////////////////////////
//~ rjf: Entity Kind Flags

typedef U32 DF_EntityKindFlags;
enum
{
  //- rjf: allowed operations
  DF_EntityKindFlag_CanDelete                = (1<<0),
  DF_EntityKindFlag_CanFreeze                = (1<<1),
  DF_EntityKindFlag_CanEdit                  = (1<<2),
  DF_EntityKindFlag_CanRename                = (1<<3),
  DF_EntityKindFlag_CanEnable                = (1<<4),
  DF_EntityKindFlag_CanCondition             = (1<<5),
  DF_EntityKindFlag_CanDuplicate             = (1<<6),
  
  //- rjf: name categorization
  DF_EntityKindFlag_NameIsCode               = (1<<7),
  DF_EntityKindFlag_NameIsPath               = (1<<8),
  
  //- rjf: lifetime categorization
  DF_EntityKindFlag_UserDefinedLifetime      = (1<<9),
  
  //- rjf: serialization
  DF_EntityKindFlag_IsSerializedToConfig     = (1<<10),
};

////////////////////////////////
//~ rjf: Entity Kind Flags

typedef U32 DF_EntityFlags;
enum
{
  //- rjf: allocationless, simple equipment
  DF_EntityFlag_HasTextPoint      = (1<<0),
  DF_EntityFlag_HasEntityHandle   = (1<<2),
  DF_EntityFlag_HasU64            = (1<<4),
  DF_EntityFlag_HasColor          = (1<<6),
  DF_EntityFlag_DiesOnRunStop     = (1<<8),
  
  //- rjf: ctrl entity equipment
  DF_EntityFlag_HasCtrlHandle     = (1<<9),
  DF_EntityFlag_HasArch           = (1<<10),
  DF_EntityFlag_HasCtrlID         = (1<<11),
  DF_EntityFlag_HasStackBase      = (1<<12),
  DF_EntityFlag_HasTLSRoot        = (1<<13),
  DF_EntityFlag_HasVAddrRng       = (1<<14),
  DF_EntityFlag_HasVAddr          = (1<<15),
  
  //- rjf: file properties
  DF_EntityFlag_IsFolder          = (1<<16),
  DF_EntityFlag_IsMissing         = (1<<17),
  
  //- rjf: deletion
  DF_EntityFlag_MarkedForDeletion = (1<<31),
};

////////////////////////////////
//~ rjf: Evaluation Spaces

typedef U64 DF_EvalSpaceKind;
enum
{
  DF_EvalSpaceKind_CtrlEntity = E_SpaceKind_FirstUserDefined,
  DF_EvalSpaceKind_CfgEntity,
};

////////////////////////////////
//~ rjf: Binding Types

typedef struct DF_Binding DF_Binding;
struct DF_Binding
{
  OS_Key key;
  OS_EventFlags flags;
};

typedef struct DF_BindingNode DF_BindingNode;
struct DF_BindingNode
{
  DF_BindingNode *next;
  DF_Binding binding;
};

typedef struct DF_BindingList DF_BindingList;
struct DF_BindingList
{
  DF_BindingNode *first;
  DF_BindingNode *last;
  U64 count;
};

typedef struct DF_StringBindingPair DF_StringBindingPair;
struct DF_StringBindingPair
{
  String8 string;
  DF_Binding binding;
};

////////////////////////////////
//~ rjf: Key Map Types

typedef struct DF_KeyMapNode DF_KeyMapNode;
struct DF_KeyMapNode
{
  DF_KeyMapNode *hash_next;
  DF_KeyMapNode *hash_prev;
  String8 name;
  DF_Binding binding;
};

typedef struct DF_KeyMapSlot DF_KeyMapSlot;
struct DF_KeyMapSlot
{
  DF_KeyMapNode *first;
  DF_KeyMapNode *last;
};

////////////////////////////////
//~ rjf: Setting Types

typedef struct DF_SettingVal DF_SettingVal;
struct DF_SettingVal
{
  B32 set;
  S32 s32;
};

////////////////////////////////
//~ rjf: View Hook Function Types

typedef struct DF_View DF_View;

#define DF_VIEW_SETUP_FUNCTION_SIG(name) void name(DF_View *view, MD_Node *params, String8 string)
#define DF_VIEW_SETUP_FUNCTION_NAME(name) df_view_setup_##name
#define DF_VIEW_SETUP_FUNCTION_DEF(name) internal DF_VIEW_SETUP_FUNCTION_SIG(DF_VIEW_SETUP_FUNCTION_NAME(name))
typedef DF_VIEW_SETUP_FUNCTION_SIG(DF_ViewSetupFunctionType);

#define DF_VIEW_CMD_FUNCTION_SIG(name) void name(DF_View *view, MD_Node *params, String8 string)
#define DF_VIEW_CMD_FUNCTION_NAME(name) df_view_cmds_##name
#define DF_VIEW_CMD_FUNCTION_DEF(name) internal DF_VIEW_CMD_FUNCTION_SIG(DF_VIEW_CMD_FUNCTION_NAME(name))
typedef DF_VIEW_CMD_FUNCTION_SIG(DF_ViewCmdFunctionType);

#define DF_VIEW_UI_FUNCTION_SIG(name) void name(DF_View *view, MD_Node *params, String8 string, Rng2F32 rect)
#define DF_VIEW_UI_FUNCTION_NAME(name) df_view_ui_##name
#define DF_VIEW_UI_FUNCTION_DEF(name) internal DF_VIEW_UI_FUNCTION_SIG(DF_VIEW_UI_FUNCTION_NAME(name))
typedef DF_VIEW_UI_FUNCTION_SIG(DF_ViewUIFunctionType);

////////////////////////////////
//~ rjf: View Rule Info Types

typedef U32 DF_ViewRuleInfoFlags;
enum
{
  DF_ViewRuleInfoFlag_ShowInDocs                 = (1<<0),
  DF_ViewRuleInfoFlag_CanFilter                  = (1<<1),
  DF_ViewRuleInfoFlag_FilterIsCode               = (1<<2),
  DF_ViewRuleInfoFlag_TypingAutomaticallyFilters = (1<<3),
};

#define DF_VIEW_RULE_UI_FUNCTION_SIG(name) void name(DF_View *view, String8 string, MD_Node *params, Rng2F32 rect)
#define DF_VIEW_RULE_UI_FUNCTION_NAME(name) df_view_rule_ui_##name
#define DF_VIEW_RULE_UI_FUNCTION_DEF(name) internal DF_VIEW_RULE_UI_FUNCTION_SIG(DF_VIEW_RULE_UI_FUNCTION_NAME(name))
typedef DF_VIEW_RULE_UI_FUNCTION_SIG(DF_ViewRuleUIFunctionType);

////////////////////////////////
//~ rjf: View Specification Types

#if 1 // TODO(rjf): @msgs
typedef U32 DF_ViewSpecFlags;
enum
{
  DF_ViewSpecFlag_ParameterizedByEntity      = (1<<0),
  DF_ViewSpecFlag_ProjectSpecific            = (1<<1),
  DF_ViewSpecFlag_CanSerialize               = (1<<2),
  DF_ViewSpecFlag_CanFilter                  = (1<<3),
  DF_ViewSpecFlag_FilterIsCode               = (1<<4),
  DF_ViewSpecFlag_TypingAutomaticallyFilters = (1<<5),
};

typedef struct DF_ViewSpecInfo DF_ViewSpecInfo;
struct DF_ViewSpecInfo
{
  DF_ViewSpecFlags flags;
  String8 name;
  String8 display_string;
  U32 icon_kind;
  DF_ViewSetupFunctionType *setup_hook;
  DF_ViewCmdFunctionType *cmd_hook;
  DF_ViewUIFunctionType *ui_hook;
};

typedef struct DF_ViewSpec DF_ViewSpec;
struct DF_ViewSpec
{
  DF_ViewSpec *hash_next;
  DF_ViewSpecInfo info;
};

typedef struct DF_ViewSpecInfoArray DF_ViewSpecInfoArray;
struct DF_ViewSpecInfoArray
{
  DF_ViewSpecInfo *v;
  U64 count;
};

typedef struct DF_CmdParamSlotViewSpecRuleNode DF_CmdParamSlotViewSpecRuleNode;
struct DF_CmdParamSlotViewSpecRuleNode
{
  DF_CmdParamSlotViewSpecRuleNode *next;
  DF_ViewSpec *view_spec;
  String8 cmd_name;
};

typedef struct DF_CmdParamSlotViewSpecRuleList DF_CmdParamSlotViewSpecRuleList;
struct DF_CmdParamSlotViewSpecRuleList
{
  DF_CmdParamSlotViewSpecRuleNode *first;
  DF_CmdParamSlotViewSpecRuleNode *last;
  U64 count;
};
#endif

////////////////////////////////
//~ rjf: View Types

typedef struct DF_ArenaExt DF_ArenaExt;
struct DF_ArenaExt
{
  DF_ArenaExt *next;
  Arena *arena;
};

typedef struct DF_TransientViewNode DF_TransientViewNode;
struct DF_TransientViewNode
{
  DF_TransientViewNode *next;
  DF_TransientViewNode *prev;
  EV_Key key;
  DF_View *view;
  Arena *initial_params_arena;
  MD_Node *initial_params;
  U64 first_frame_index_touched;
  U64 last_frame_index_touched;
};

typedef struct DF_TransientViewSlot DF_TransientViewSlot;
struct DF_TransientViewSlot
{
  DF_TransientViewNode *first;
  DF_TransientViewNode *last;
};

typedef struct DF_View DF_View;
struct DF_View
{
  // rjf: allocation links (for iterating all views)
  DF_View *alloc_next;
  DF_View *alloc_prev;
  
  // rjf: ownership links ('owners' can have lists of views)
  DF_View *order_next;
  DF_View *order_prev;
  
  // rjf: transient view children
  DF_View *first_transient;
  DF_View *last_transient;
  
  // rjf: view specification info
  DF_ViewSpec *spec;
  
  // rjf: allocation info
  U64 generation;
  
  // rjf: loading animation state
  F32 loading_t;
  F32 loading_t_target;
  U64 loading_progress_v;
  U64 loading_progress_v_target;
  
  // rjf: view project (for project-specific/filtered views)
  Arena *project_path_arena;
  String8 project_path;
  
  // rjf: view state
  UI_ScrollPt2 scroll_pos;
  
  // rjf: view-lifetime allocation & user data extensions
  Arena *arena;
  DF_ArenaExt *first_arena_ext;
  DF_ArenaExt *last_arena_ext;
  U64 transient_view_slots_count;
  DF_TransientViewSlot *transient_view_slots;
  DF_TransientViewNode *free_transient_view_node;
  void *user_data;
  
  // rjf: filter mode
  B32 is_filtering;
  F32 is_filtering_t;
  
  // rjf: params tree state
  Arena *params_arenas[2];
  MD_Node *params_roots[2];
  U64 params_write_gen;
  U64 params_read_gen;
  
  // rjf: text query state
  TxtPt query_cursor;
  TxtPt query_mark;
  U64 query_string_size;
  U8 query_buffer[KB(4)];
};

////////////////////////////////
//~ rjf: Panel Types

typedef struct DF_Panel DF_Panel;
struct DF_Panel
{
  // rjf: tree links/data
  DF_Panel *first;
  DF_Panel *last;
  DF_Panel *next;
  DF_Panel *prev;
  DF_Panel *parent;
  U64 child_count;
  
  // rjf: allocation data
  U64 generation;
  
  // rjf: split data
  Axis2 split_axis;
  F32 pct_of_parent;
  
  // rjf: animated rectangle data
  Rng2F32 animated_rect_pct;
  
  // rjf: tab params
  Side tab_side;
  
  // rjf: stable views (tabs)
  DF_View *first_tab_view;
  DF_View *last_tab_view;
  U64 tab_view_count;
  D_Handle selected_tab_view;
};

typedef struct DF_PanelRec DF_PanelRec;
struct DF_PanelRec
{
  DF_Panel *next;
  int push_count;
  int pop_count;
};

////////////////////////////////
//~ rjf: Drag/Drop Types

typedef enum DF_DragDropState
{
  DF_DragDropState_Null,
  DF_DragDropState_Dragging,
  DF_DragDropState_Dropping,
  DF_DragDropState_COUNT
}
DF_DragDropState;

typedef struct DF_DragDropPayload DF_DragDropPayload;
struct DF_DragDropPayload
{
  UI_Key key;
  D_Handle panel;
  D_Handle view;
  D_Handle entity;
  TxtPt text_point;
};

////////////////////////////////
//~ rjf: View Rule Spec Types

typedef U32 DF_ViewRuleSpecInfoFlags; // NOTE(rjf): see @view_rule_info
enum
{
  DF_ViewRuleSpecInfoFlag_VizRowProd     = (1<<0),
  DF_ViewRuleSpecInfoFlag_LineStringize  = (1<<1),
  DF_ViewRuleSpecInfoFlag_RowUI          = (1<<2),
  DF_ViewRuleSpecInfoFlag_ViewUI         = (1<<3),
};

#define DF_VIEW_RULE_VIZ_ROW_PROD_FUNCTION_SIG(name) void name(void)
#define DF_VIEW_RULE_VIZ_ROW_PROD_FUNCTION_NAME(name) df_view_rule_viz_row_prod__##name
#define DF_VIEW_RULE_VIZ_ROW_PROD_FUNCTION_DEF(name) internal DF_VIEW_RULE_VIZ_ROW_PROD_FUNCTION_SIG(DF_VIEW_RULE_VIZ_ROW_PROD_FUNCTION_NAME(name))

#define DF_VIEW_RULE_LINE_STRINGIZE_FUNCTION_SIG(name) void name(void)
#define DF_VIEW_RULE_LINE_STRINGIZE_FUNCTION_NAME(name) df_view_rule_line_stringize__##name
#define DF_VIEW_RULE_LINE_STRINGIZE_FUNCTION_DEF(name) internal DF_VIEW_RULE_LINE_STRINGIZE_FUNCTION_SIG(DF_VIEW_RULE_LINE_STRINGIZE_FUNCTION_NAME(name))

#define DF_VIEW_RULE_ROW_UI_FUNCTION_SIG(name) void name(EV_Key key, MD_Node *params, String8 string)
#define DF_VIEW_RULE_ROW_UI_FUNCTION_NAME(name) df_view_rule_row_ui__##name
#define DF_VIEW_RULE_ROW_UI_FUNCTION_DEF(name) DF_VIEW_RULE_ROW_UI_FUNCTION_SIG(DF_VIEW_RULE_ROW_UI_FUNCTION_NAME(name))

typedef DF_VIEW_RULE_VIZ_ROW_PROD_FUNCTION_SIG(DF_ViewRuleVizRowProdHookFunctionType);
typedef DF_VIEW_RULE_LINE_STRINGIZE_FUNCTION_SIG(DF_ViewRuleLineStringizeHookFunctionType);
typedef DF_VIEW_RULE_ROW_UI_FUNCTION_SIG(DF_ViewRuleRowUIFunctionType);

typedef struct DF_ViewRuleSpecInfo DF_ViewRuleSpecInfo;
struct DF_ViewRuleSpecInfo
{
  String8 string;
  DF_ViewRuleSpecInfoFlags flags;
  DF_ViewRuleVizRowProdHookFunctionType *viz_row_prod;
  DF_ViewRuleLineStringizeHookFunctionType *line_stringize;
  DF_ViewRuleRowUIFunctionType *row_ui;
};

typedef struct DF_ViewRuleSpecInfoArray DF_ViewRuleSpecInfoArray;
struct DF_ViewRuleSpecInfoArray
{
  DF_ViewRuleSpecInfo *v;
  U64 count;
};

typedef struct DF_ViewRuleSpec DF_ViewRuleSpec;
struct DF_ViewRuleSpec
{
  DF_ViewRuleSpec *hash_next;
  DF_ViewRuleSpecInfo info;
};

////////////////////////////////
//~ rjf: Command Kind Types

typedef U32 DF_QueryFlags;
enum
{
  DF_QueryFlag_AllowFiles       = (1<<0),
  DF_QueryFlag_AllowFolders     = (1<<1),
  DF_QueryFlag_CodeInput        = (1<<2),
  DF_QueryFlag_KeepOldInput     = (1<<3),
  DF_QueryFlag_SelectOldInput   = (1<<4),
  DF_QueryFlag_Required         = (1<<5),
};

typedef U32 DF_CmdKindFlags;
enum
{
  DF_CmdKindFlag_ListInUI      = (1<<0),
  DF_CmdKindFlag_ListInIPCDocs = (1<<1),
};

////////////////////////////////
//~ rjf: Generated Code

#include "generated/dbg_frontend.meta.h"

////////////////////////////////
//~ rjf: Entity Types

typedef U64 DF_EntityID;

typedef struct DF_Entity DF_Entity;
struct DF_Entity
{
  // rjf: tree links
  DF_Entity *first;
  DF_Entity *last;
  DF_Entity *next;
  DF_Entity *prev;
  DF_Entity *parent;
  
  // rjf: metadata
  DF_EntityKind kind;
  DF_EntityFlags flags;
  DF_EntityID id;
  U64 gen;
  U64 alloc_time_us;
  F32 alive_t;
  
  // rjf: basic equipment
  TxtPt text_point;
  D_Handle entity_handle;
  B32 disabled;
  U64 u64;
  Vec4F32 color_hsva;
  D_CfgSrc cfg_src;
  U64 timestamp;
  
  // rjf: ctrl equipment
  CTRL_Handle ctrl_handle;
  Arch arch;
  U32 ctrl_id;
  U64 stack_base;
  Rng1U64 vaddr_rng;
  U64 vaddr;
  
  // rjf: string equipment
  String8 string;
  
  // rjf: parameter tree
  Arena *params_arena;
  MD_Node *params_root;
};

typedef struct DF_EntityNode DF_EntityNode;
struct DF_EntityNode
{
  DF_EntityNode *next;
  DF_Entity *entity;
};

typedef struct DF_EntityList DF_EntityList;
struct DF_EntityList
{
  DF_EntityNode *first;
  DF_EntityNode *last;
  U64 count;
};

typedef struct DF_EntityArray DF_EntityArray;
struct DF_EntityArray
{
  DF_Entity **v;
  U64 count;
};

typedef struct DF_EntityRec DF_EntityRec;
struct DF_EntityRec
{
  DF_Entity *next;
  S32 push_count;
  S32 pop_count;
};

////////////////////////////////
//~ rjf: Entity Evaluation Types

typedef struct DF_EntityEval DF_EntityEval;
struct DF_EntityEval
{
  B64 enabled;
  U64 hit_count;
  U64 label_off;
  U64 location_off;
  U64 condition_off;
};

////////////////////////////////
//~ rjf: Entity Fuzzy Listing Types

typedef struct DF_EntityFuzzyItem DF_EntityFuzzyItem;
struct DF_EntityFuzzyItem
{
  DF_Entity *entity;
  FuzzyMatchRangeList matches;
};

typedef struct DF_EntityFuzzyItemArray DF_EntityFuzzyItemArray;
struct DF_EntityFuzzyItemArray
{
  DF_EntityFuzzyItem *v;
  U64 count;
};

////////////////////////////////
//~ rjf: Command Types

typedef struct DF_Cmd DF_Cmd;
struct DF_Cmd
{
  String8 name;
  DF_Regs *regs;
};

typedef struct DF_CmdNode DF_CmdNode;
struct DF_CmdNode
{
  DF_CmdNode *next;
  DF_CmdNode *prev;
  DF_Cmd cmd;
};

typedef struct DF_CmdList DF_CmdList;
struct DF_CmdList
{
  DF_CmdNode *first;
  DF_CmdNode *last;
  U64 count;
};

////////////////////////////////
//~ rjf: Context Register Types

typedef struct DF_RegsNode DF_RegsNode;
struct DF_RegsNode
{
  DF_RegsNode *next;
  DF_Regs v;
};

////////////////////////////////
//~ rjf: Theme Types

typedef struct DF_Theme DF_Theme;
struct DF_Theme
{
  Vec4F32 colors[DF_ThemeColor_COUNT];
};

typedef enum DF_FontSlot
{
  DF_FontSlot_Main,
  DF_FontSlot_Code,
  DF_FontSlot_Icons,
  DF_FontSlot_COUNT
}
DF_FontSlot;

typedef enum DF_PaletteCode
{
  DF_PaletteCode_Base,
  DF_PaletteCode_MenuBar,
  DF_PaletteCode_Floating,
  DF_PaletteCode_ImplicitButton,
  DF_PaletteCode_PlainButton,
  DF_PaletteCode_PositivePopButton,
  DF_PaletteCode_NegativePopButton,
  DF_PaletteCode_NeutralPopButton,
  DF_PaletteCode_ScrollBarButton,
  DF_PaletteCode_Tab,
  DF_PaletteCode_TabInactive,
  DF_PaletteCode_DropSiteOverlay,
  DF_PaletteCode_COUNT
}
DF_PaletteCode;

////////////////////////////////
//~ rjf: Auto-Complete Lister Types

typedef U32 DF_AutoCompListerFlags;
enum
{
  DF_AutoCompListerFlag_Locals        = (1<<0),
  DF_AutoCompListerFlag_Registers     = (1<<1),
  DF_AutoCompListerFlag_ViewRules     = (1<<2),
  DF_AutoCompListerFlag_ViewRuleParams= (1<<3),
  DF_AutoCompListerFlag_Members       = (1<<4),
  DF_AutoCompListerFlag_Languages     = (1<<5),
  DF_AutoCompListerFlag_Architectures = (1<<6),
  DF_AutoCompListerFlag_Tex2DFormats  = (1<<7),
};

typedef struct DF_AutoCompListerItem DF_AutoCompListerItem;
struct DF_AutoCompListerItem
{
  String8 string;
  String8 kind_string;
  FuzzyMatchRangeList matches;
};

typedef struct DF_AutoCompListerItemChunkNode DF_AutoCompListerItemChunkNode;
struct DF_AutoCompListerItemChunkNode
{
  DF_AutoCompListerItemChunkNode *next;
  DF_AutoCompListerItem *v;
  U64 count;
  U64 cap;
};

typedef struct DF_AutoCompListerItemChunkList DF_AutoCompListerItemChunkList;
struct DF_AutoCompListerItemChunkList
{
  DF_AutoCompListerItemChunkNode *first;
  DF_AutoCompListerItemChunkNode *last;
  U64 chunk_count;
  U64 total_count;
};

typedef struct DF_AutoCompListerItemArray DF_AutoCompListerItemArray;
struct DF_AutoCompListerItemArray
{
  DF_AutoCompListerItem *v;
  U64 count;
};

typedef struct DF_AutoCompListerParams DF_AutoCompListerParams;
struct DF_AutoCompListerParams
{
  DF_AutoCompListerFlags flags;
  String8List strings;
};

////////////////////////////////
//~ rjf: Per-Window State

typedef struct DF_Window DF_Window;
struct DF_Window
{
  // rjf: links & metadata
  DF_Window *next;
  DF_Window *prev;
  U64 gen;
  U64 frames_alive;
  D_CfgSrc cfg_src;
  
  // rjf: top-level info & handles
  Arena *arena;
  OS_Handle os;
  R_Handle r;
  UI_State *ui;
  F32 last_dpi;
  B32 window_temporarily_focused_ipc;
  
  // rjf: config/settings
  DF_SettingVal setting_vals[DF_SettingCode_COUNT];
  UI_Palette cfg_palettes[DF_PaletteCode_COUNT]; // derivative from theme
  
  // rjf: dev interface state
  B32 dev_menu_is_open;
  
  // rjf: menu bar state
  B32 menu_bar_focused;
  B32 menu_bar_focused_on_press;
  B32 menu_bar_key_held;
  B32 menu_bar_focus_press_started;
  
  // rjf: code context menu state
  Arena *code_ctx_menu_arena;
  String8 code_ctx_menu_file_path;
  U128 code_ctx_menu_text_key;
  TXT_LangKind code_ctx_menu_lang_kind;
  TxtRng code_ctx_menu_range;
  U64 code_ctx_menu_vaddr;
  D_LineList code_ctx_menu_lines;
  
  // rjf: entity context menu state
  D_Handle entity_ctx_menu_entity;
  U8 entity_ctx_menu_input_buffer[1024];
  U64 entity_ctx_menu_input_size;
  TxtPt entity_ctx_menu_input_cursor;
  TxtPt entity_ctx_menu_input_mark;
  
  // rjf: tab context menu state
  D_Handle tab_ctx_menu_panel;
  D_Handle tab_ctx_menu_view;
  U8 tab_ctx_menu_input_buffer[1024];
  U64 tab_ctx_menu_input_size;
  TxtPt tab_ctx_menu_input_cursor;
  TxtPt tab_ctx_menu_input_mark;
  
  // rjf: autocomplete lister state
  U64 autocomp_last_frame_idx;
  B32 autocomp_force_closed;
  B32 autocomp_query_dirty;
  UI_Key autocomp_root_key;
  Arena *autocomp_lister_params_arena;
  DF_AutoCompListerParams autocomp_lister_params;
  U64 autocomp_cursor_off;
  U8 autocomp_lister_query_buffer[1024];
  U64 autocomp_lister_query_size;
  F32 autocomp_open_t;
  F32 autocomp_num_visible_rows_t;
  S64 autocomp_cursor_num;
  
  // rjf: query view stack
  Arena *query_cmd_arena;
  String8 query_cmd_name;
  DF_Regs *query_cmd_regs;
  U64 query_cmd_regs_mask[(DF_RegSlot_COUNT + 63) / 64];
  DF_View *query_view_stack_top;
  B32 query_view_selected;
  F32 query_view_selected_t;
  F32 query_view_t;
  
  // rjf: hover eval stable state
  B32 hover_eval_focused;
  TxtPt hover_eval_txt_cursor;
  TxtPt hover_eval_txt_mark;
  U8 hover_eval_txt_buffer[1024];
  U64 hover_eval_txt_size;
  Arena *hover_eval_arena;
  Vec2F32 hover_eval_spawn_pos;
  String8 hover_eval_string;
  
  // rjf: hover eval timer
  U64 hover_eval_first_frame_idx;
  U64 hover_eval_last_frame_idx;
  
  // rjf: hover eval params
  String8 hover_eval_file_path;
  TxtPt hover_eval_file_pt;
  U64 hover_eval_vaddr;
  F32 hover_eval_open_t;
  F32 hover_eval_num_visible_rows_t;
  
  // rjf: error state
  U8 error_buffer[512];
  U64 error_string_size;
  F32 error_t;
  
  // rjf: panel state
  DF_Panel *root_panel;
  DF_Panel *free_panel;
  DF_Panel *focused_panel;
  
  // rjf: per-frame ui events state
  UI_EventList ui_events;
  
  // rjf: per-frame drawing state
  DR_Bucket *draw_bucket;
};

////////////////////////////////
//~ rjf: Eval Visualization View Cache Types

typedef struct DF_EvalVizViewCacheNode DF_EvalVizViewCacheNode;
struct DF_EvalVizViewCacheNode
{
  DF_EvalVizViewCacheNode *next;
  DF_EvalVizViewCacheNode *prev;
  U64 key;
  EV_View *v;
};

typedef struct DF_EvalVizViewCacheSlot DF_EvalVizViewCacheSlot;
struct DF_EvalVizViewCacheSlot
{
  DF_EvalVizViewCacheNode *first;
  DF_EvalVizViewCacheNode *last;
};

////////////////////////////////
//~ rjf: Main Per-Process Graphical State

typedef struct D_NameChunkNode D_NameChunkNode;
struct D_NameChunkNode
{
  D_NameChunkNode *next;
  U64 size;
};

typedef struct D_EntityListCache D_EntityListCache;
struct D_EntityListCache
{
  Arena *arena;
  U64 alloc_gen;
  DF_EntityList list;
};

typedef struct DF_State DF_State;
struct DF_State
{
  // rjf: top-level state
  Arena *arena;
  B32 quit;
  F64 time_in_seconds;
  Log *log;
  String8 log_path;
  
  // rjf: frame state
  F32 frame_dt;
  U64 frame_index;
  Arena *frame_arenas[2];
  
  // rjf: frame time history
  U64 frame_time_us_history[64];
  
  // rjf: registers stack
  DF_RegsNode base_regs;
  DF_RegsNode *top_regs;
  
  // rjf: commands
  Arena *cmds_arena;
  DF_CmdList cmds;
  
  // rjf: frame request state
  U64 num_frames_requested;
  
  // rjf: autosave timer
  F32 seconds_until_autosave;
  
  // rjf: name allocator
  D_NameChunkNode *free_name_chunks[8];
  
  // rjf: entity state
  Arena *entities_arena;
  DF_Entity *entities_base;
  U64 entities_count;
  U64 entities_id_gen;
  DF_Entity *entities_root;
  DF_Entity *entities_free[2]; // [0] -> normal lifetime, not user defined; [1] -> user defined lifetime (& thus undoable)
  U64 entities_free_count;
  U64 entities_active_count;
  
  // rjf: entity query caches
  U64 kind_alloc_gens[DF_EntityKind_COUNT];
  D_EntityListCache kind_caches[DF_EntityKind_COUNT];
  
  // rjf: key map table
  Arena *key_map_arena;
  U64 key_map_table_size;
  DF_KeyMapSlot *key_map_table;
  DF_KeyMapNode *free_key_map_node;
  U64 key_map_total_count;
  
  // rjf: bind change
  Arena *bind_change_arena;
  B32 bind_change_active;
  String8 bind_change_cmd_name;
  DF_Binding bind_change_binding;
  
  // rjf: top-level context menu keys
  UI_Key code_ctx_menu_key;
  UI_Key entity_ctx_menu_key;
  UI_Key tab_ctx_menu_key;
  
  // rjf: confirmation popup state
  UI_Key confirm_key;
  B32 confirm_active;
  F32 confirm_t;
  Arena *confirm_arena;
  DF_CmdList confirm_cmds;
  String8 confirm_title;
  String8 confirm_desc;
  
  // rjf: string search state
  Arena *string_search_arena;
  String8 string_search_string;
  
  // rjf: view specs
  U64 view_spec_table_size;
  DF_ViewSpec **view_spec_table;
  
  // rjf: view rule specs
  U64 view_rule_spec_table_size;
  DF_ViewRuleSpec **view_rule_spec_table;
  
  // rjf: windows
  DF_Window *first_window;
  DF_Window *last_window;
  DF_Window *free_window;
  U64 window_count;
  B32 last_window_queued_save;
  D_Handle last_focused_window;
  
  // rjf: eval visualization view cache
  U64 eval_viz_view_cache_slots_count;
  DF_EvalVizViewCacheSlot *eval_viz_view_cache_slots;
  DF_EvalVizViewCacheNode *eval_viz_view_cache_node_free;
  
  // rjf: view state
  DF_View *first_view;
  DF_View *last_view;
  DF_View *free_view;
  U64 free_view_count;
  U64 allocated_view_count;
  
  // rjf: drag/drop state machine
  DF_DragDropState drag_drop_state;
  
  // rjf: rich hover info
  DF_Regs *hover_regs;
  DF_Regs *next_hover_regs;
  
  // rjf: config reading state
  Arena *cfg_path_arenas[D_CfgSrc_COUNT];
  String8 cfg_paths[D_CfgSrc_COUNT];
  U64 cfg_cached_timestamp[D_CfgSrc_COUNT];
  Arena *cfg_arena;
  D_CfgTable cfg_table;
  U64 ctrl_exception_code_filters[(CTRL_ExceptionCodeKind_COUNT+63)/64];
  
  // rjf: running theme state
  DF_Theme cfg_theme_target;
  DF_Theme cfg_theme;
  Arena *cfg_main_font_path_arena;
  Arena *cfg_code_font_path_arena;
  String8 cfg_main_font_path;
  String8 cfg_code_font_path;
  FNT_Tag cfg_font_tags[DF_FontSlot_COUNT]; // derivative from font paths
  
  // rjf: global settings
  DF_SettingVal cfg_setting_vals[D_CfgSrc_COUNT][DF_SettingCode_COUNT];
  
  // rjf: icon texture
  R_Handle icon_texture;
  
  // rjf: current path
  Arena *current_path_arena;
  String8 current_path;
};

////////////////////////////////
//~ rjf: Globals

read_only global DF_Entity d_nil_entity =
{
  &d_nil_entity,
  &d_nil_entity,
  &d_nil_entity,
  &d_nil_entity,
  &d_nil_entity,
};

read_only global DF_CmdKindInfo df_nil_cmd_kind_info = {0};

read_only global DF_ViewRuleInfo df_nil_view_rule_info = {0};

read_only global DF_ViewSpec df_nil_view_spec =
{
  &df_nil_view_spec,
  {
    0,
    {0},
    {0},
    DF_IconKind_Null,
    DF_VIEW_SETUP_FUNCTION_NAME(null),
    DF_VIEW_CMD_FUNCTION_NAME(null),
    DF_VIEW_UI_FUNCTION_NAME(null),
  },
};

read_only global DF_ViewRuleSpec df_nil_view_rule_spec =
{
  &df_nil_view_rule_spec,
};

read_only global DF_View df_nil_view =
{
  &df_nil_view,
  &df_nil_view,
  &df_nil_view,
  &df_nil_view,
  &df_nil_view,
  &df_nil_view,
  &df_nil_view_spec,
};

read_only global DF_Panel df_nil_panel =
{
  &df_nil_panel,
  &df_nil_panel,
  &df_nil_panel,
  &df_nil_panel,
  &df_nil_panel,
};

global DF_State *df_state = 0;
global DF_DragDropPayload df_drag_drop_payload = {0};
global D_Handle df_last_drag_drop_panel = {0};
global D_Handle df_last_drag_drop_prev_tab = {0};

////////////////////////////////
//~ rjf: Registers Type Functions

internal void df_regs_copy_contents(Arena *arena, DF_Regs *dst, DF_Regs *src);
internal DF_Regs *df_regs_copy(Arena *arena, DF_Regs *src);

////////////////////////////////
//~ rjf: Commands Type Functions

internal void df_cmd_list_push_new(Arena *arena, DF_CmdList *cmds, String8 name, DF_Regs *regs);

////////////////////////////////
//~ rjf: Entity Type Pure Functions

//- rjf: nil
internal B32 df_entity_is_nil(DF_Entity *entity);
#define df_require_entity_nonnil(entity, if_nil_stmts) do{if(df_entity_is_nil(entity)){if_nil_stmts;}}while(0)

//- rjf: handle <-> entity conversions
internal U64 df_index_from_entity(DF_Entity *entity);
internal D_Handle df_handle_from_entity(DF_Entity *entity);
internal DF_Entity *df_entity_from_handle(D_Handle handle);
internal D_HandleList df_handle_list_from_entity_list(Arena *arena, DF_EntityList entities);

//- rjf: entity recursion iterators
internal DF_EntityRec df_entity_rec_depth_first(DF_Entity *entity, DF_Entity *subtree_root, U64 sib_off, U64 child_off);
#define df_entity_rec_depth_first_pre(entity, subtree_root)  df_entity_rec_depth_first((entity), (subtree_root), OffsetOf(DF_Entity, next), OffsetOf(DF_Entity, first))
#define df_entity_rec_depth_first_post(entity, subtree_root) df_entity_rec_depth_first((entity), (subtree_root), OffsetOf(DF_Entity, prev), OffsetOf(DF_Entity, last))

//- rjf: ancestor/child introspection
internal DF_Entity *df_entity_child_from_kind(DF_Entity *entity, DF_EntityKind kind);
internal DF_Entity *df_entity_ancestor_from_kind(DF_Entity *entity, DF_EntityKind kind);
internal DF_EntityList df_push_entity_child_list_with_kind(Arena *arena, DF_Entity *entity, DF_EntityKind kind);
internal DF_Entity *df_entity_child_from_string_and_kind(DF_Entity *parent, String8 string, DF_EntityKind kind);

//- rjf: entity list building
internal void df_entity_list_push(Arena *arena, DF_EntityList *list, DF_Entity *entity);
internal DF_EntityArray df_entity_array_from_list(Arena *arena, DF_EntityList *list);
#define df_first_entity_from_list(list) ((list)->first != 0 ? (list)->first->entity : &d_nil_entity)

//- rjf: entity fuzzy list building
internal DF_EntityFuzzyItemArray df_entity_fuzzy_item_array_from_entity_list_needle(Arena *arena, DF_EntityList *list, String8 needle);
internal DF_EntityFuzzyItemArray df_entity_fuzzy_item_array_from_entity_array_needle(Arena *arena, DF_EntityArray *array, String8 needle);

//- rjf: full path building, from file/folder entities
internal String8 df_full_path_from_entity(Arena *arena, DF_Entity *entity);

//- rjf: display string entities, for referencing entities in ui
internal String8 df_display_string_from_entity(Arena *arena, DF_Entity *entity);

//- rjf: extra search tag strings for fuzzy filtering entities
internal String8 df_search_tags_from_entity(Arena *arena, DF_Entity *entity);

//- rjf: entity -> color operations
internal Vec4F32 df_hsva_from_entity(DF_Entity *entity);
internal Vec4F32 df_rgba_from_entity(DF_Entity *entity);

//- rjf: entity -> expansion tree keys
internal EV_Key df_ev_key_from_entity(DF_Entity *entity);
internal EV_Key df_parent_ev_key_from_entity(DF_Entity *entity);

//- rjf: entity -> evaluation
internal DF_EntityEval *df_eval_from_entity(Arena *arena, DF_Entity *entity);

////////////////////////////////
//~ rjf: View Type Functions

internal B32 df_view_is_nil(DF_View *view);
internal B32 df_view_is_project_filtered(DF_View *view);
internal D_Handle df_handle_from_view(DF_View *view);
internal DF_View *df_view_from_handle(D_Handle handle);

////////////////////////////////
//~ rjf: View Spec Type Functions

internal DF_ViewKind df_view_kind_from_string(String8 string);

////////////////////////////////
//~ rjf: Panel Type Functions

//- rjf: basic type functions
internal B32 df_panel_is_nil(DF_Panel *panel);
internal D_Handle df_handle_from_panel(DF_Panel *panel);
internal DF_Panel *df_panel_from_handle(D_Handle handle);
internal UI_Key df_ui_key_from_panel(DF_Panel *panel);

//- rjf: tree construction
internal void df_panel_insert(DF_Panel *parent, DF_Panel *prev_child, DF_Panel *new_child);
internal void df_panel_remove(DF_Panel *parent, DF_Panel *child);

//- rjf: tree walk
internal DF_PanelRec df_panel_rec_df(DF_Panel *panel, U64 sib_off, U64 child_off);
#define df_panel_rec_df_pre(panel) df_panel_rec_df(panel, OffsetOf(DF_Panel, next), OffsetOf(DF_Panel, first))
#define df_panel_rec_df_post(panel) df_panel_rec_df(panel, OffsetOf(DF_Panel, prev), OffsetOf(DF_Panel, last))

//- rjf: panel -> rect calculations
internal Rng2F32 df_target_rect_from_panel_child(Rng2F32 parent_rect, DF_Panel *parent, DF_Panel *panel);
internal Rng2F32 df_target_rect_from_panel(Rng2F32 root_rect, DF_Panel *root, DF_Panel *panel);

//- rjf: view ownership insertion/removal
internal void df_panel_insert_tab_view(DF_Panel *panel, DF_View *prev_view, DF_View *view);
internal void df_panel_remove_tab_view(DF_Panel *panel, DF_View *view);
internal DF_View *df_selected_tab_from_panel(DF_Panel *panel);

//- rjf: icons & display strings
internal DF_IconKind df_icon_kind_from_view(DF_View *view);
internal DR_FancyStringList df_title_fstrs_from_view(Arena *arena, DF_View *view, Vec4F32 primary_color, Vec4F32 secondary_color, F32 size);

////////////////////////////////
//~ rjf: Window Type Functions

internal D_Handle df_handle_from_window(DF_Window *window);
internal DF_Window *df_window_from_handle(D_Handle handle);

////////////////////////////////
//~ rjf: Command Parameters From Context

internal B32 df_prefer_dasm_from_window(DF_Window *window);
#if 0 // TODO(rjf): @msgs
internal D_CmdParams df_cmd_params_from_window(DF_Window *window);
internal D_CmdParams df_cmd_params_from_panel(DF_Window *window, DF_Panel *panel);
internal D_CmdParams df_cmd_params_from_view(DF_Window *window, DF_Panel *panel, DF_View *view);
#endif

////////////////////////////////
//~ rjf: Global Cross-Window UI Interaction State Functions

internal B32 df_drag_is_active(void);
internal void df_drag_begin(DF_DragDropPayload *payload);
internal B32 df_drag_drop(DF_DragDropPayload *out_payload);
internal void df_drag_kill(void);
internal void df_queue_drag_drop(void);

internal void df_set_hover_regs(void);
internal DF_Regs *df_get_hover_regs(void);

////////////////////////////////
//~ rjf: Name Allocation

internal U64 df_name_bucket_idx_from_string_size(U64 size);
internal String8 df_name_alloc(String8 string);
internal void df_name_release(String8 string);

////////////////////////////////
//~ rjf: Entity Stateful Functions

//- rjf: entity allocation + tree forming
internal DF_Entity *df_entity_alloc(DF_Entity *parent, DF_EntityKind kind);
internal void df_entity_mark_for_deletion(DF_Entity *entity);
internal void df_entity_release(DF_Entity *entity);
internal void df_entity_change_parent(DF_Entity *entity, DF_Entity *old_parent, DF_Entity *new_parent, DF_Entity *prev_child);

//- rjf: entity simple equipment
internal void df_entity_equip_txt_pt(DF_Entity *entity, TxtPt point);
internal void df_entity_equip_entity_handle(DF_Entity *entity, D_Handle handle);
internal void df_entity_equip_disabled(DF_Entity *entity, B32 b32);
internal void df_entity_equip_u64(DF_Entity *entity, U64 u64);
internal void df_entity_equip_color_rgba(DF_Entity *entity, Vec4F32 rgba);
internal void df_entity_equip_color_hsva(DF_Entity *entity, Vec4F32 hsva);
internal void df_entity_equip_cfg_src(DF_Entity *entity, D_CfgSrc cfg_src);
internal void df_entity_equip_timestamp(DF_Entity *entity, U64 timestamp);

//- rjf: control layer correllation equipment
internal void df_entity_equip_ctrl_handle(DF_Entity *entity, CTRL_Handle handle);
internal void df_entity_equip_arch(DF_Entity *entity, Arch arch);
internal void df_entity_equip_ctrl_id(DF_Entity *entity, U32 id);
internal void df_entity_equip_stack_base(DF_Entity *entity, U64 stack_base);
internal void df_entity_equip_vaddr_rng(DF_Entity *entity, Rng1U64 range);
internal void df_entity_equip_vaddr(DF_Entity *entity, U64 vaddr);

//- rjf: name equipment
internal void df_entity_equip_name(DF_Entity *entity, String8 name);

//- rjf: file path map override lookups
internal String8List d_possible_overrides_from_file_path(Arena *arena, String8 file_path);

//- rjf: top-level state queries
internal DF_Entity *df_entity_root(void);
internal DF_EntityList df_push_entity_list_with_kind(Arena *arena, DF_EntityKind kind);
internal DF_Entity *df_entity_from_id(DF_EntityID id);
internal DF_Entity *df_machine_entity_from_machine_id(CTRL_MachineID machine_id);
internal DF_Entity *df_entity_from_ctrl_handle(CTRL_Handle handle);
internal DF_Entity *df_entity_from_ctrl_id(CTRL_MachineID machine_id, U32 id);
internal DF_Entity *df_entity_from_name_and_kind(String8 string, DF_EntityKind kind);

////////////////////////////////
//~ rjf: Evaluation Spaces

//- rjf: ctrl entity <-> eval space
internal CTRL_Entity *d_ctrl_entity_from_eval_space(E_Space space);
internal E_Space d_eval_space_from_ctrl_entity(CTRL_Entity *entity);

//- rjf: entity <-> eval space
internal DF_Entity *d_entity_from_eval_space(E_Space space);
internal E_Space d_eval_space_from_entity(DF_Entity *entity);

//- rjf: eval space reads/writes
internal B32 d_eval_space_read(void *u, E_Space space, void *out, Rng1U64 range);
internal B32 d_eval_space_write(void *u, E_Space space, void *in, Rng1U64 range);

//- rjf: asynchronous streamed reads -> hashes from spaces
internal U128 d_key_from_eval_space_range(E_Space space, Rng1U64 range, B32 zero_terminated);

//- rjf: space -> entire range
internal Rng1U64 d_whole_range_from_eval_space(E_Space space);

////////////////////////////////
//~ rjf: Evaluation Visualization

//- rjf: writing values back to child processes
internal B32 d_commit_eval_value_string(E_Eval dst_eval, String8 string);

//- rjf: eval / view rule params tree info extraction
internal U64 d_base_offset_from_eval(E_Eval eval);
internal E_Value d_value_from_params_key(MD_Node *params, String8 key);
internal Rng1U64 d_range_from_eval_params(E_Eval eval, MD_Node *params);
internal TXT_LangKind d_lang_kind_from_eval_params(E_Eval eval, MD_Node *params);
internal Arch d_arch_from_eval_params(E_Eval eval, MD_Node *params);
internal Vec2S32 d_dim2s32_from_eval_params(E_Eval eval, MD_Node *params);
internal R_Tex2DFormat d_tex2dformat_from_eval_params(E_Eval eval, MD_Node *params);

//- rjf: eval <-> entity
internal DF_Entity *d_entity_from_eval_string(String8 string);
internal String8 d_eval_string_from_entity(Arena *arena, DF_Entity *entity);

//- rjf: eval <-> file path
internal String8 d_file_path_from_eval_string(Arena *arena, String8 string);
internal String8 d_eval_string_from_file_path(Arena *arena, String8 string);

////////////////////////////////
//~ rjf: View Rule Kind Functions

internal DF_ViewRuleInfo *df_view_rule_info_from_string(String8 string);

////////////////////////////////
//~ rjf: View Spec State Functions

internal void df_register_view_specs(DF_ViewSpecInfoArray specs);
internal DF_ViewSpec *df_view_spec_from_string(String8 string);
internal DF_ViewSpec *df_view_spec_from_kind(DF_ViewKind kind);

////////////////////////////////
//~ rjf: View Rule Spec State Functions

internal void df_register_view_rule_specs(DF_ViewRuleSpecInfoArray specs);
internal DF_ViewRuleSpec *df_view_rule_spec_from_string(String8 string);

////////////////////////////////
//~ rjf: View State Functions

//- rjf: allocation/releasing
internal DF_View *df_view_alloc(void);
internal void df_view_release(DF_View *view);

//- rjf: equipment
internal void df_view_equip_spec(DF_View *view, DF_ViewSpec *spec, String8 query, MD_Node *params);
internal void df_view_equip_query(DF_View *view, String8 query);
internal void df_view_equip_loading_info(DF_View *view, B32 is_loading, U64 progress_v, U64 progress_target);

//- rjf: user state extensions
internal void *df_view_get_or_push_user_state(DF_View *view, U64 size);
internal Arena *df_view_push_arena_ext(DF_View *view);
#define df_view_user_state(view, type) (type *)df_view_get_or_push_user_state((view), sizeof(type))

//- rjf: param saving
internal void df_view_store_param(DF_View *view, String8 key, String8 value);
internal void df_view_store_paramf(DF_View *view, String8 key, char *fmt, ...);
#define df_view_store_param_f32(view, key, f32) df_view_store_paramf((view), (key), "%ff", (f32))
#define df_view_store_param_s64(view, key, s64) df_view_store_paramf((view), (key), "%I64d", (s64))
#define df_view_store_param_u64(view, key, u64) df_view_store_paramf((view), (key), "0x%I64x", (u64))

////////////////////////////////
//~ rjf: Expand-Keyed Transient View Functions

internal DF_TransientViewNode *df_transient_view_node_from_ev_key(DF_View *owner_view, EV_Key key);

////////////////////////////////
//~ rjf: Panel State Functions

internal DF_Panel *df_panel_alloc(DF_Window *ws);
internal void df_panel_release(DF_Window *ws, DF_Panel *panel);
internal void df_panel_release_all_views(DF_Panel *panel);

////////////////////////////////
//~ rjf: Window State Functions

internal DF_Window *df_window_open(Vec2F32 size, OS_Handle preferred_monitor, D_CfgSrc cfg_src);

internal DF_Window *df_window_from_os_handle(OS_Handle os);

internal void df_window_frame(DF_Window *ws);

////////////////////////////////
//~ rjf: Eval Visualization

internal EV_View *df_ev_view_from_key(U64 key);
internal F32 df_append_value_strings_from_eval(Arena *arena, EV_StringFlags flags, U32 default_radix, FNT_Tag font, F32 font_size, F32 max_size, S32 depth, E_Eval eval, E_Member *member, EV_ViewRuleList *view_rules, String8List *out);
internal String8 df_value_string_from_eval(Arena *arena, EV_StringFlags flags, U32 default_radix, FNT_Tag font, F32 font_size, F32 max_size, E_Eval eval, E_Member *member, EV_ViewRuleList *view_rules);

////////////////////////////////
//~ rjf: Hover Eval

internal void df_set_hover_eval(Vec2F32 pos, String8 file_path, TxtPt pt, U64 vaddr, String8 string);

////////////////////////////////
//~ rjf: Auto-Complete Lister

internal void df_autocomp_lister_item_chunk_list_push(Arena *arena, DF_AutoCompListerItemChunkList *list, U64 cap, DF_AutoCompListerItem *item);
internal DF_AutoCompListerItemArray df_autocomp_lister_item_array_from_chunk_list(Arena *arena, DF_AutoCompListerItemChunkList *list);
internal int df_autocomp_lister_item_qsort_compare(DF_AutoCompListerItem *a, DF_AutoCompListerItem *b);
internal void df_autocomp_lister_item_array_sort__in_place(DF_AutoCompListerItemArray *array);

internal String8 df_autocomp_query_word_from_input_string_off(String8 input, U64 cursor_off);
internal DF_AutoCompListerParams df_view_rule_autocomp_lister_params_from_input_cursor(Arena *arena, String8 string, U64 cursor_off);
internal void df_set_autocomp_lister_query(UI_Key root_key, DF_AutoCompListerParams *params, String8 input, U64 cursor_off);

////////////////////////////////
//~ rjf: Search Strings

internal void df_set_search_string(String8 string);
internal String8 df_push_search_string(Arena *arena);

////////////////////////////////
//~ rjf: Colors, Fonts, Config

//- rjf: keybindings
internal OS_Key df_os_key_from_cfg_string(String8 string);
internal void df_clear_bindings(void);
internal DF_BindingList df_bindings_from_name(Arena *arena, String8 name);
internal void df_bind_name(String8 name, DF_Binding binding);
internal void df_unbind_name(String8 name, DF_Binding binding);
internal String8List df_cmd_name_list_from_binding(Arena *arena, DF_Binding binding);

//- rjf: colors
internal Vec4F32 df_rgba_from_theme_color(DF_ThemeColor color);
internal DF_ThemeColor df_theme_color_from_txt_token_kind(TXT_TokenKind kind);

//- rjf: code -> palette
internal UI_Palette *df_palette_from_code(DF_PaletteCode code);

//- rjf: fonts/sizes
internal FNT_Tag df_font_from_slot(DF_FontSlot slot);
internal F32 df_font_size_from_slot(DF_FontSlot slot);
internal FNT_RasterFlags df_raster_flags_from_slot(DF_FontSlot slot);

//- rjf: settings
internal DF_SettingVal df_setting_val_from_code(DF_SettingCode code);

//- rjf: config serialization
internal int df_qsort_compare__cfg_string_bindings(DF_StringBindingPair *a, DF_StringBindingPair *b);
internal String8List df_cfg_strings_from_gfx(Arena *arena, String8 root_path, D_CfgSrc source);

////////////////////////////////
//~ rjf: Process Control Info Stringification

internal String8 df_string_from_exception_code(U32 code);
internal String8 df_stop_explanation_string_icon_from_ctrl_event(Arena *arena, CTRL_Event *event, DF_IconKind *icon_out);

////////////////////////////////
//~ rjf: Continuous Frame Requests

internal void df_request_frame(void);

////////////////////////////////
//~ rjf: Main State Accessors

//- rjf: per-frame arena
internal Arena *df_frame_arena(void);

//- rjf: config paths
internal String8 df_cfg_path_from_src(D_CfgSrc src);

//- rjf: entity cache queries
internal DF_EntityList d_query_cached_entity_list_with_kind(DF_EntityKind kind);
internal DF_EntityList d_push_active_target_list(Arena *arena);
internal DF_Entity *d_entity_from_ev_key_and_kind(EV_Key key, DF_EntityKind kind);

//- rjf: config state
internal D_CfgTable *df_cfg_table(void);

////////////////////////////////
//~ rjf: Registers

internal DF_Regs *df_regs(void);
internal DF_Regs *df_base_regs(void);
internal DF_Regs *df_push_regs_(DF_Regs *regs);
#define df_push_regs(...) df_push_regs_(&(DF_Regs){df_regs_lit_init_top __VA_ARGS__})
internal DF_Regs *df_pop_regs(void);
#define DF_RegsScope(...) DeferLoop(df_push_regs(__VA_ARGS__), df_pop_regs())
internal void df_regs_fill_slot_from_string(DF_RegSlot slot, String8 string);

////////////////////////////////
//~ rjf: Commands

// TODO(rjf): @msgs temporary glue
#if 0
internal D_CmdSpec *df_cmd_spec_from_kind(DF_CmdKind kind);
#endif
internal DF_CmdKind df_cmd_kind_from_string(String8 string);

//- rjf: name -> info
internal DF_CmdKindInfo *df_cmd_kind_info_from_string(String8 string);

//- rjf: pushing
internal void df_push_cmd(String8 name, DF_Regs *regs);
#define df_cmd(kind, ...) df_push_cmd(df_cmd_kind_info_table[kind].string, &(DF_Regs){df_regs_lit_init_top __VA_ARGS__})

//- rjf: iterating
internal B32 df_next_cmd(DF_Cmd **cmd);

////////////////////////////////
//~ rjf: Main Layer Top-Level Calls

internal void df_init(CmdLine *cmdln);
internal void df_frame(void);

#endif // DBG_FRONTEND_CORE_H
