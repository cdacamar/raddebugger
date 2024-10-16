// Copyright (c) 2024 Epic Games Tools
// Licensed under the MIT license (https://opensource.org/license/mit/)

#pragma once

typedef enum
{
  LNK_SymbolScopeIndex_Defined,
  LNK_SymbolScopeIndex_Internal, // symbols defined by linker
  LNK_SymbolScopeIndex_Weak,
  LNK_SymbolScopeIndex_Lib,
  LNK_SymbolScopeIndex_Count
} LNK_SymbolScopeIndex;

enum
{
  LNK_SymbolScopeFlag_Defined  = 1,
  LNK_SymbolScopeFlag_Internal = 2,
  LNK_SymbolScopeFlag_Weak     = 4,
  LNK_SymbolScopeFlag_Lib      = 8,
  
  LNK_SymbolScopeFlag_Main = LNK_SymbolScopeFlag_Defined | LNK_SymbolScopeFlag_Weak,
  LNK_SymbolScopeFlag_All  = LNK_SymbolScopeFlag_Defined | LNK_SymbolScopeFlag_Weak | LNK_SymbolScopeFlag_Lib | LNK_SymbolScopeFlag_Internal
};
typedef U64 LNK_SymbolScopeFlags;

typedef enum
{
  LNK_DefinedSymbolVisibility_Static,
  LNK_DefinedSymbolVisibility_Extern,
  LNK_DefinedSymbolVisibility_Internal,
} LNK_DefinedSymbolVisibility;

enum
{
  LNK_DefinedSymbolFlag_IsFunc  = (1 << 0),
  LNK_DefinedSymbolFlag_IsThunk = (1 << 1),
};
typedef U64 LNK_DefinedSymbolFlags;

typedef enum
{
  LNK_DefinedSymbolValue_Null,
  LNK_DefinedSymbolValue_Chunk,
  LNK_DefinedSymbolValue_VA
} LNK_DefinedSymbolValueType;

typedef struct LNK_DefinedSymbol
{
  LNK_DefinedSymbolFlags     flags;
  LNK_DefinedSymbolValueType value_type;
  union {
    struct {
      LNK_Chunk            *chunk;
      U64                   chunk_offset;
      U32                   check_sum;
      COFF_ComdatSelectType selection;
    };
    U64 va;
  } u;
} LNK_DefinedSymbol;

typedef struct LNK_WeakSymbol
{
  LNK_SymbolScopeFlags scope_flags;
  COFF_WeakExtType     lookup_type;
  struct LNK_Symbol   *fallback_symbol;
} LNK_WeakSymbol;

typedef struct LNK_UndefinedSymbol
{
  LNK_SymbolScopeFlags scope_flags;
} LNK_UndefinedSymbol;

typedef struct LNK_LazySymbol
{
  struct LNK_Lib *lib;
  U64             member_offset;
} LNK_LazySymbol;

#define LNK_Symbol_IsDefined(type) ((type) == LNK_Symbol_DefinedStatic || (type) == LNK_Symbol_DefinedExtern || (type) == LNK_Symbol_DefinedInternal)
typedef enum 
{
  LNK_Symbol_Null,
  LNK_Symbol_DefinedStatic,
  LNK_Symbol_DefinedExtern,
  LNK_Symbol_DefinedInternal,
  LNK_Symbol_Weak,
  LNK_Symbol_Lazy,
  LNK_Symbol_Undefined,
} LNK_SymbolType;

#define LNK_DEBUG_SYMBOLS 1
typedef struct LNK_Symbol
{
  String8        name;
  LNK_SymbolType type;
  union {
    LNK_DefinedSymbol   defined;
    LNK_WeakSymbol      weak;
    LNK_UndefinedSymbol undefined;
    LNK_LazySymbol      lazy;
  } u;
#if LNK_DEBUG_SYMBOLS
  String8 debug;
#endif
} LNK_Symbol;

typedef struct LNK_SymbolNode
{
  struct LNK_SymbolNode *next;
  struct LNK_SymbolNode *prev;
  U64                    hash;
  LNK_Symbol            *data;
} LNK_SymbolNode;

typedef struct LNK_SymbolList
{
  U64             count;
  LNK_SymbolNode *first;
  LNK_SymbolNode *last;
} LNK_SymbolList;

typedef struct LNK_SymbolNodeArray
{
  U64              count;
  LNK_SymbolNode **v;
} LNK_SymbolNodeArray;

typedef struct LNK_SymbolArray
{
  U64         count;
  LNK_Symbol *v;
} LNK_SymbolArray;

typedef struct LNK_SymbolTable
{
  Arena          *arena;
  U64             bucket_count[LNK_SymbolScopeIndex_Count];
  LNK_SymbolList *buckets[LNK_SymbolScopeIndex_Count];
} LNK_SymbolTable;

////////////////////////////////
// parallel for wrappers

typedef struct
{
  LNK_Symbol *symbol_arr;
  Rng1U64    *range_arr;
  U64        *hash_arr;
} LNK_SymbolNameHasher;

typedef struct
{
  LNK_SymbolNode **input_arr;
  Rng1U64         *range_arr;
} LNK_SymbolNodePtrHasher;

typedef struct
{
  LNK_SymbolNode *input_arr;
  Rng1U64        *range_arr;
} LNK_SymbolNodeHasher;

typedef struct
{
  LNK_SymbolTable *symtab;
  LNK_SymbolList  *bucket_arr;
  Rng1U64         *range_arr;
} LNK_DefinedSymbolInserter;

typedef struct
{
  LNK_SymbolTable *symtab;
  LNK_SymbolList  *bucket_arr;
  Rng1U64         *range_arr;
} LNK_LazySymbolInserter;

typedef struct
{
  LNK_SymbolTable *symtab;
  Rng1U64         *range_arr;
} LNK_ComdatFolder;

////////////////////////////////

extern LNK_Symbol  g_null_symbol;
extern LNK_Symbol *g_null_symbol_ptr;

internal void lnk_init_symbol(LNK_Symbol *symbol, String8 name, LNK_SymbolType type);
internal void lnk_init_defined_symbol(LNK_Symbol *symbol, String8 name, LNK_DefinedSymbolVisibility visibility, LNK_DefinedSymbolFlags flags);
internal void lnk_init_defined_symbol_chunk(LNK_Symbol *symbol, String8 name, LNK_DefinedSymbolVisibility visibility, LNK_DefinedSymbolFlags flags, LNK_Chunk *chunk, U64 offset, COFF_ComdatSelectType selection, U32 check_sum);
internal void lnk_init_defined_symbol_va(LNK_Symbol *symbol, String8 name, LNK_DefinedSymbolVisibility visibility, LNK_DefinedSymbolFlags flags, U64 va);
internal void lnk_init_undefined_symbol(LNK_Symbol *symbol, String8 name, LNK_SymbolScopeFlags scope_flags);
internal void lnk_init_weak_symbol(LNK_Symbol *symbol, String8 name, COFF_WeakExtType lookup, LNK_Symbol *fallback);

internal LNK_Symbol * lnk_make_defined_symbol(Arena *arena, String8 name, LNK_DefinedSymbolVisibility visibility, LNK_DefinedSymbolFlags flags);
internal LNK_Symbol * lnk_make_defined_symbol_chunk(Arena *arena, String8 name, LNK_DefinedSymbolVisibility visibility, LNK_DefinedSymbolFlags flags, LNK_Chunk *chunk, U64 offset, COFF_ComdatSelectType selection, U32 check_sum);
internal LNK_Symbol * lnk_make_defined_symbol_va(Arena *arena, String8 name, LNK_DefinedSymbolVisibility visibility, LNK_DefinedSymbolFlags flags, U64 va);
internal LNK_Symbol * lnk_make_undefined_symbol(Arena *arena, String8 name, LNK_SymbolScopeFlags scope_flags);
internal LNK_Symbol * lnk_make_weak_symbol(Arena *arena, String8 name, COFF_WeakExtType lookup, LNK_Symbol *fallback);
internal LNK_Symbol * lnk_make_lazy_symbol(Arena *arena, String8 name, struct LNK_Lib *lib, U64 member_offset);

#if LNK_DEBUG_SYMBOLS
#define lnk_symbol_set_debugf(a, s, fmt, ...) do { (s)->debug = push_str8f((a), fmt, __VA_ARGS__); } while (0)
#define lnk_symbol_set_debug(s, str) do { (s)->debug = str; } while (0)
#else
#define lnk_symbol_set_debugf(...)
#define lnk_symbol_set_debug(...)
#endif

internal LNK_Chunk * lnk_defined_symbol_get_chunk(LNK_DefinedSymbol *symbol);

internal void lnk_symbol_update_chunk_ref(LNK_Symbol *symbol, U64 src_sect_id, U64 dst_sect_id, U64 *id_map, U64 id_count);

internal void                lnk_symbol_list_push_node(LNK_SymbolList *list, LNK_SymbolNode *node);
internal LNK_SymbolNode *    lnk_symbol_list_push(Arena *arena, LNK_SymbolList *list, LNK_Symbol *symbol);
internal void                lnk_symbol_list_push_list(LNK_SymbolList *list, LNK_SymbolList *to_push);
internal void                lnk_symbol_list_insert_after(LNK_SymbolList *list, LNK_SymbolNode *node, LNK_SymbolNode *insert);
internal LNK_SymbolNode *    lnk_symbol_list_pop_node(LNK_SymbolList *list);
internal LNK_Symbol *        lnk_symbol_list_pop(LNK_SymbolList *list);
internal void                lnk_symbol_list_remove(LNK_SymbolList *list, LNK_SymbolNode *node);
internal void                lnk_symbol_list_concat_in_place(LNK_SymbolList *list, LNK_SymbolList *to_concat);
internal LNK_SymbolNodeArray lnk_symbol_node_array_from_list(Arena *arena, LNK_SymbolList list);

internal LNK_SymbolList      lnk_symbol_list_from_array(Arena *arena, LNK_SymbolArray arr);
internal LNK_SymbolNodeArray lnk_symbol_node_array_from_list(Arena *arena, LNK_SymbolList list);
internal LNK_SymbolArray     lnk_symbol_array_from_list(Arena *arena, LNK_SymbolList list);
internal LNK_Symbol *        lnk_symbol_array_search(LNK_SymbolArray symarr, String8 name, StringMatchFlags flags);
internal U64 *               lnk_symbol_array_hash(TP_Context *tp, Arena *arena, LNK_Symbol *arr, U64 count);

internal LNK_SymbolTable * lnk_symbol_table_alloc(void);
internal LNK_SymbolTable * lnk_symbol_table_alloc_ex(U64 defined_cap, U64 internal_cap, U64 weak_cap, U64 lib_cap);
internal void              lnk_symbol_table_release(LNK_SymbolTable **symtab);
internal U64               lnk_symbol_table_hash(String8 string);
internal LNK_SymbolNode *  lnk_symbol_table_search_bucket(LNK_SymbolTable *symtab, LNK_SymbolScopeIndex scope_idx, U64 bucket_idx, String8 name, U64 hash);
internal LNK_SymbolNode *  lnk_symbol_table_search_node_hash(LNK_SymbolTable *symtab, LNK_SymbolScopeFlags scope_flags, String8 name, U64 hash);
internal LNK_SymbolNode *  lnk_symbol_table_search_node(LNK_SymbolTable *symtab, LNK_SymbolScopeFlags scope, String8 name);
internal LNK_Symbol *      lnk_symbol_table_search(LNK_SymbolTable *symtab, LNK_SymbolScopeFlags scope_flags, String8 name);
internal LNK_Symbol *      lnk_symbol_table_searchf(LNK_SymbolTable *symtab, LNK_SymbolScopeFlags scope_flags, char *fmt, ...);
internal void              lnk_symbol_table_push_node_hash(LNK_SymbolTable *symtab, LNK_SymbolNode *node, U64 hash);
internal void              lnk_symbol_table_push_node(LNK_SymbolTable *symtab, LNK_SymbolNode *node);
internal LNK_SymbolNode *  lnk_symbol_table_push(LNK_SymbolTable *symtab, LNK_Symbol *symbol);
internal void              lnk_symbol_table_push_lazy_arr(TP_Context *tp, LNK_SymbolTable *symtab, LNK_Symbol *arr, U64 count);
internal void              lnk_symbol_table_remove(LNK_SymbolTable *symtab, LNK_SymbolScopeIndex scope, String8 name);
internal void              lnk_symbol_table_replace(LNK_SymbolTable *symtab, LNK_SymbolScopeIndex iscope, LNK_Symbol *symbol);

internal LNK_Symbol * lnk_resolve_symbol(LNK_SymbolTable *symtab, LNK_Symbol *resolve_symbol);

internal LNK_SymbolList   lnk_pop_comdat_chain(LNK_SymbolList *bucket, LNK_SymbolNode **cursor);
internal LNK_SymbolNode * lnk_fold_comdat_chain(LNK_SymbolList chain_list);
internal void             lnk_fold_comdat_chunks(TP_Context *tp, LNK_SymbolTable *symtab);

