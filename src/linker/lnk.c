// Copyright (c) 2024 Epic Games Tools
// Licensed under the MIT license (https://opensource.org/license/mit/)

////////////////////////////////
// Build Options

#define BUILD_CONSOLE_INTERFACE 1
#define BUILD_VERSION_MAJOR 0
#define BUILD_VERSION_MINOR 6
#define BUILD_VERSION_PATCH 0
#define BUILD_RELEASE_PHASE_STRING_LITERAL "ALPHA"
#define BUILD_VERSION_STRING Stringify(BUILD_VERSION_MAJOR) "." Stringify(BUILD_VERSION_MINOR) "." Stringify(BUILD_VERSION_PATCH)
#define BUILD_TITLE "Epic Games Tools (R) RAD PE/COFF Linker " BUILD_VERSION_STRING

////////////////////////////////

#define ARENA_FREE_LIST 1

////////////////////////////////
// Third Party

#include "base_ext/base_blake3.h"
#include "base_ext/base_blake3.c"
#include "third_party_ext/md5/md5.c"
#include "third_party_ext/md5/md5.h"
#include "third_party_ext/xxHash/xxhash.c"
#include "third_party_ext/xxHash/xxhash.h"

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable:4789)
#endif
#include "third_party_ext/radsort/radsort.h"
#if defined(_MSC_VER)
#pragma warning(pop)
#endif

////////////////////////////////
// Code Base

#if defined(__clang__)
# pragma clang diagnostic push
# pragma clang diagnostic ignored "-Winitializer-overrides"
# pragma clang diagnostic ignored "-Wswitch"
#endif

#include "base/base_inc.h"
#include "os/os_inc.h"
#include "path/path.h"
#include "coff/coff.h"
#include "pe/pe.h"
#include "codeview/codeview.h"
#include "msf/msf.h"
#include "msf/msf_parse.h"
#include "pdb/pdb.h"

#include "base/base_inc.c"
#include "os/os_inc.c"
#include "path/path.c"
#include "coff/coff.c"
#include "pe/pe.c"
#include "codeview/codeview.c"
#include "msf/msf_parse.c"
#include "pdb/pdb.c"

#if defined(__clang__)
# pragma clang diagnostic pop
#endif

////////////////////////////////
// RDI

#include "rdi/rdi_overrides.h"
#include "lib_rdi_format/rdi_format.h"
#include "rdi/rdi.h"
#include "lib_rdi_format/rdi_format.c"
#include "rdi/rdi.c"

////////////////////////////////
// Code Base Extensions

#include "base_ext/base_inc.h"
#include "path_ext/path.h"
#include "hash_table.h"
#include "thread_pool/thread_pool.h"
#include "os_ext/os_inc.h"
#include "codeview_ext/codeview.h"
#include "pdb_ext/msf_builder.h"
#include "pdb_ext/pdb.h"
#include "pdb_ext/pdb_helpers.h"
#include "pdb_ext/pdb_builder.h"

#include "base_ext/base_inc.c"
#include "path_ext/path.c"
#include "hash_table.c"
#include "thread_pool/thread_pool.c"
#include "os_ext/os_inc.c"
#include "codeview_ext/codeview.c"
#include "pdb_ext/msf_builder.c"
#include "pdb_ext/pdb.c"
#include "pdb_ext/pdb_helpers.c"
#include "pdb_ext/pdb_builder.c"

////////////////////////////////
// RDI Builder

#include "rdi/rdi_builder.h"
#include "rdi/rdi_coff.h" 
#include "rdi/rdi_cv.h"

#include "rdi/rdi_builder.c"
#include "rdi/rdi_coff.c"
#include "rdi/rdi_cv.c"

////////////////////////////////
// Linker

#include "lnk_error.h"
#include "lnk_log.h"
#include "lnk_timer.h"
#include "lnk_cmd_line.h"
#include "lnk_config.h"
#include "lnk_chunk.h"
#include "lnk_reloc.h"
#include "lnk_directive.h"
#include "lnk_symbol_table.h"
#include "lnk_section_table.h"
#include "lnk_obj.h"
#include "lnk_import_table.h"
#include "lnk_export_table.h"
#include "lnk_lib.h"
#include "lnk_debug_info.h"
#include "lnk.h"

#include "lnk_error.c"
#include "lnk_log.c"
#include "lnk_timer.c"
#include "lnk_cmd_line.c"
#include "lnk_config.c"
#include "lnk_chunk.c"
#include "lnk_reloc.c"
#include "lnk_directive.c"
#include "lnk_symbol_table.c"
#include "lnk_section_table.c"
#include "lnk_obj.c"
#include "lnk_import_table.c"
#include "lnk_export_table.c"
#include "lnk_lib.c"
#include "lnk_debug_info.c"

////////////////////////////////

internal LNK_InputImport *
lnk_input_import_list_push(Arena *arena, LNK_InputImportList *list)
{
  LNK_InputImport *node = push_array(arena, LNK_InputImport, 1);
  SLLQueuePush(list->first, list->last, node);
  list->count += 1;
  return node; 
}

internal void
lnk_input_import_list_concat_in_place(LNK_InputImportList *list, LNK_InputImportList *to_concat)
{
  SLLConcatInPlace(list, to_concat);
}

internal LNK_InputImport **
lnk_input_import_arr_from_list(Arena *arena, LNK_InputImportList list)
{
  LNK_InputImport **result = push_array_no_zero(arena, LNK_InputImport *, list.count);
  U64 idx = 0;
  for (LNK_InputImport *node = list.first; node != 0; node = node->next) {
    Assert(idx < list.count);
    result[idx++] = node;
  }
  return result;
}

internal LNK_InputImportList
lnk_list_from_input_import_arr(LNK_InputImport **arr, U64 count)
{
  LNK_InputImportList list; MemoryZeroStruct(&list);
  for (U64 i = 0; i < count; i += 1) {
    SLLQueuePush(list.first, list.last, arr[i]);
    list.count += 1;
  }
  return list;
}

int
lnk_input_import_is_before(void *raw_a, void *raw_b)
{
  LNK_InputImport **a = raw_a;
  LNK_InputImport **b = raw_b;
  int cmp = str8_compar_ignore_case(&(*a)->import_header.dll_name, &(*b)->import_header.dll_name);
  if (cmp == 0) {
    cmp = str8_compar_case_sensetive(&(*a)->import_header.func_name, &(*b)->import_header.func_name);
  }
  return cmp < 0;
}

int
lnk_input_import_compar(const void *raw_a, const void *raw_b)
{
  const LNK_InputImport **a = (const LNK_InputImport **) raw_a;
  const LNK_InputImport **b = (const LNK_InputImport **) raw_b;
  int cmp = str8_compar_ignore_case(&(*a)->import_header.dll_name, &(*b)->import_header.dll_name);
  if (cmp == 0) {
    cmp = str8_compar_case_sensetive(&(*a)->import_header.func_name, &(*b)->import_header.func_name);
  }
  return cmp;
}

////////////////////////////////

internal void
lnk_write_data_list_to_file_path(String8 path, String8List data)
{
#if PROFILE_TELEMETRY
  Temp scratch = scratch_begin(0, 0);
  String8 size_str = str8_from_memory_size2(scratch.arena, data.total_size);
  ProfBeginDynamic("Write %.*s to %.*s", str8_varg(size_str), str8_varg(path));
  scratch_end(scratch);
#endif

  B32 is_written = os_write_data_list_to_file_path(path, data);
  if (is_written) {
    if (lnk_get_log_status(LNK_Log_IO)) {
      Temp scratch = scratch_begin(0,0);
      String8 size_str = str8_from_memory_size2(scratch.arena, data.total_size);
      lnk_log(LNK_Log_IO, "File \"%S\" %S written", path, size_str);
      scratch_end(scratch);
    }
  } else {
    lnk_error(LNK_Error_NoAccess, "don't have access to write to %S", path);
  }
  ProfEnd();
}

internal void
lnk_write_data_to_file_path(String8 path, String8 data)
{
  Temp scratch = scratch_begin(0,0);
  String8List data_list = {0};
  str8_list_push(scratch.arena, &data_list, data);
  lnk_write_data_list_to_file_path(path, data_list);
  scratch_end(scratch);
}

internal String8
lnk_make_full_path(Arena *arena, String8 work_dir, PathStyle system_path_style, String8 path)
{
  ProfBeginFunction();
  String8 result = str8(0,0);
  PathStyle path_style = path_style_from_str8(path);
  if (path_style == PathStyle_Relative) {
    Temp scratch = scratch_begin(&arena, 1);
    String8List list; MemoryZeroStruct(&list);
    str8_list_push(scratch.arena, &list, work_dir);
    str8_list_push(scratch.arena, &list, path);
    result = str8_path_list_join_by_style(arena, &list, system_path_style);
    scratch_end(scratch);
  } else {
    result = push_str8_copy(arena, path);
  }
  ProfEnd();
  return result;
}

////////////////////////////////

internal String8List
lnk_make_linker_manifest(Arena      *arena,
                         B32         manifest_uac,
                         String8     manifest_level,
                         String8     manifest_ui_access,
                         String8List manifest_dependency_list)
{
  String8List srl = {0};
  str8_serial_begin(arena, &srl);
  str8_serial_push_string(arena, &srl, str8_lit(
        "<?xml version=\"1.0\" standalone=\"yes\"?>\n"
        "<assembly xmlns=\"urn:schemas-microsoft-com:asm.v1\"\n"
        "          manifestVersion=\"1.0\">\n"));
  if (manifest_uac) {
    String8 uac = push_str8f(arena,
          "   <trustInfo>\n"
          "     <security>\n"
          "       <requestedPrivileges>\n"
          "         <requestedExecutionLevel level=%S uiAccess=%S/>\n"
          "       </requestedPrivileges>\n"
          "     </security>\n"
          "   </trustInfo>\n",
          manifest_level,
          manifest_ui_access);
    str8_serial_push_string(arena, &srl, uac);
  }
  for (String8Node *node = manifest_dependency_list.first; node != 0; node = node->next) {
    String8 dep = push_str8f(arena, 
        " <dependency>\n"
        "   <dependentAssembly>\n"
        "     <assemblyIdentity %S/>\n"
        "   </dependentAssembly>\n"
        " </dependency>\n",
        node->string);
    str8_serial_push_string(arena, &srl, dep);
  }
  str8_serial_push_string(arena, &srl, str8_lit("</assembly>\n"));
  return srl;
}

internal String8
lnk_merge_manifest_files(Arena *arena, String8 mt_path, String8 manifest_name, String8List manifest_path_list)
{
  ProfBeginFunction();

  Temp scratch = scratch_begin(&arena,1);

  String8List invoke_cmd_line = {0};
  str8_list_push(arena, &invoke_cmd_line, mt_path);
  String8 work_dir = os_get_current_path(arena);
  for (String8Node *man_node = manifest_path_list.first;
       man_node != 0;
       man_node = man_node->next) {
    String8 full_path = path_absolute_dst_from_relative_dst_src(arena, man_node->string, work_dir);
    full_path = path_convert_slashes(arena, full_path, PathStyle_UnixAbsolute);
    str8_list_pushf(arena, &invoke_cmd_line, "-manifest");
    str8_list_push(arena, &invoke_cmd_line, full_path);
  }
  str8_list_pushf(arena, &invoke_cmd_line, "-out:%S", manifest_name);
  str8_list_pushf(arena, &invoke_cmd_line, "-nologo");

  OS_ProcessLaunchParams launch_opts = {0};
  launch_opts.cmd_line               = invoke_cmd_line;
  launch_opts.path                   = str8_chop_last_slash(mt_path);
  launch_opts.inherit_env            = 1;
  launch_opts.consoleless            = 1;

  OS_Handle mt_handle = os_process_launch(&launch_opts);
  if (!os_handle_match(mt_handle, os_handle_zero())) {
    if (os_process_join(mt_handle, max_U64)) {
      if (!os_file_path_exists(manifest_name)) {
        lnk_error(LNK_Error_Mt, "something went wrong, manifest was not written to \"%S\"", manifest_name);
      }
    }
    os_process_detach(mt_handle);
  } else {
    lnk_error(LNK_Error_Mt, "unable to start process for %S", mt_path);
  }

  scratch_end(scratch);
  ProfEnd();
  return manifest_name;
} 
internal String8
lnk_res_from_data(Arena *arena, String8 data)
{
  Temp scratch = scratch_begin(&arena, 1);

  COFF_ResourceID type;
  type.type = COFF_ResourceIDType_NUMBER;
  type.u.number = PE_ResourceKind_MANIFEST;

  COFF_ResourceID name;
  name.type = COFF_ResourceIDType_NUMBER;
  name.u.number = 1;

  String8List res_list = coff_write_resource(arena, type, name, 1, 0, 1033, 0, 0, data);
  String8 res_data = str8_serial_end(arena, &res_list);

  scratch_end(scratch);
  return res_data;
}

////////////////////////////////

internal int
lnk_res_string_id_is_before(void *raw_a, void *raw_b)
{
  PE_Resource *a = raw_a;
  PE_Resource *b = raw_b;
  Assert(a->id.type == COFF_ResourceIDType_STRING);
  Assert(b->id.type == COFF_ResourceIDType_STRING);
  int is_before = str8_is_before_case_sensetive(&a->id.u.string, &b->id.u.string);
  return is_before;
}

internal int
lnk_res_number_id_is_before(void *raw_a, void *raw_b)
{
  PE_Resource *a = raw_a;
  PE_Resource *b = raw_b;
  Assert(a->id.type == COFF_ResourceIDType_NUMBER);
  Assert(b->id.type == COFF_ResourceIDType_NUMBER);
  int is_before = u16_is_before(&a->id.u.number, &b->id.u.number);
  return is_before;
}

internal void
lnk_serialize_pe_resource_tree(LNK_SectionTable *st, LNK_SymbolTable *symtab, PE_ResourceDir *root_dir)
{
  ProfBeginFunction();
  
  static const U64 ALIGN = 4;
  
  struct stack_s {
    struct stack_s *next;
    U64 arr_idx;
    U64 res_idx[2];
    PE_ResourceArray res_arr[2];
    LNK_Chunk *coff_entry_array_chunk;
    LNK_Chunk *coff_entry_chunk;
  };
  
  Temp scratch = scratch_begin(0, 0);
  
  LNK_Section *dir_sect = lnk_section_table_push(st, str8_lit(".rsrc$01"), LNK_RSRC_SECTION_FLAGS);
  LNK_Section *data_sect = lnk_section_table_push(st, str8_lit(".rsrc$02"), LNK_RSRC_SECTION_FLAGS);
  
  LNK_Chunk *dir_tree_chunk = lnk_section_push_chunk_list(dir_sect, dir_sect->root, str8(0,0));
  LNK_Chunk *dir_data_chunk = lnk_section_push_chunk_list(dir_sect, dir_sect->root, str8(0,0));
  LNK_Chunk *dir_string_chunk = lnk_section_push_chunk_list(dir_sect, dir_sect->root, str8(0,0));
  
  dir_tree_chunk->sort_idx   = str8_lit("a");
  dir_string_chunk->sort_idx = str8_lit("b");
  dir_data_chunk->sort_idx   = str8_lit("c");
  
  PE_Resource root_wrapper; MemoryZeroStruct(&root_wrapper);
  root_wrapper.id.type = COFF_ResourceIDType_NUMBER;
  root_wrapper.id.u.number = 0;
  root_wrapper.kind = PE_ResDataKind_DIR;
  root_wrapper.u.dir = root_dir;
  
  struct stack_s *stack = push_array(scratch.arena, struct stack_s, 1);
  stack->res_arr[0].count = 1;
  stack->res_arr[0].v = &root_wrapper;
  
  U64 res_counter = 0;
  
  while (stack) {
    while (stack->arr_idx < ArrayCount(stack->res_arr)) {
      while (stack->res_idx[stack->arr_idx] < stack->res_arr[stack->arr_idx].count) {
        PE_Resource *res = &stack->res_arr[stack->arr_idx].v[stack->res_idx[stack->arr_idx]];
        stack->res_idx[stack->arr_idx] += 1;
        
        String8 flag_name = push_str8f(symtab->arena, "flag_%u", res_counter);
        String8 offset_name = push_str8f(symtab->arena, "offset_%u", res_counter);
        ++res_counter;
        
        if (stack->coff_entry_array_chunk) {
          COFF_ResourceDirEntry *entry = push_array(dir_sect->arena, COFF_ResourceDirEntry, 1);
          stack->coff_entry_chunk = lnk_section_push_chunk_data(dir_sect, stack->coff_entry_array_chunk, str8_struct(entry), str8(0,0));
          
          switch (res->id.type) {
          case COFF_ResourceIDType_NUMBER: {
            entry->name.id = res->id.u.number;
          } break;
          case COFF_ResourceIDType_STRING: {
            // TODO: we can make string table smaller by reusing offsets for same strings
            
            // not sure why high bit has to be turned on here since number id and string id entries are
            // in separate arrays but windows doesn't treat name offset like string without this bit.
            entry->name.offset |= (1 << 31);
            
            // convert name to utf-16
            String16 name16 = str16_from_8(dir_sect->arena, res->id.u.string);
            
            // build name string
            U64 name16_byte_size = name16.size * sizeof(U16);
            U64 buffer_size = /* char count: */ sizeof(U16) + name16_byte_size;
            U8 *buffer = push_array_no_zero(dir_sect->arena, U8, buffer_size);
            *(U16*)buffer = name16.size;
            MemoryCopy(buffer + sizeof(U16), name16.str, name16_byte_size);
            
            // push string table chunk
            String8 name_data = str8(buffer, buffer_size);
            LNK_Chunk *name_chunk = lnk_section_push_chunk_data(dir_sect, dir_string_chunk, name_data, str8(0,0));
            
            // push name chunk symbol
            LNK_Symbol *name_symbol = lnk_make_defined_symbol_chunk(symtab->arena, str8_lit("COFF_RESOURCE_ID_STRING"), LNK_DefinedSymbolVisibility_Static, 0, name_chunk, 0, 0, 0);
            lnk_section_push_reloc(dir_sect, stack->coff_entry_chunk, LNK_Reloc_SECT_REL, OffsetOf(COFF_ResourceDirEntry, name.offset), name_symbol);
          } break;
          case COFF_ResourceIDType_NULL: break;
          default: InvalidPath;
          }
        }
        
        switch (res->kind) {
        case PE_ResDataKind_DIR: {
          // initialize directory header
          COFF_ResourceDirTable *dir_header = push_array(dir_sect->arena, COFF_ResourceDirTable, 1);
          dir_header->characteristics = res->u.dir->characteristics;
          dir_header->time_stamp = res->u.dir->time_stamp;
          dir_header->major_version = res->u.dir->major_version;
          dir_header->minor_version = res->u.dir->minor_version;
          dir_header->name_entry_count = res->u.dir->named_list.count;
          dir_header->id_entry_count = res->u.dir->id_list.count;
          
          // push sub directory chunk layout
          LNK_Chunk *dir_node_chunk = lnk_section_push_chunk_list(dir_sect, dir_tree_chunk, str8(0,0));
          dir_node_chunk->align = ALIGN;
          LNK_Chunk *dir_header_chunk = lnk_section_push_chunk_data(dir_sect, dir_node_chunk, str8_struct(dir_header), str8(0,0));
          LNK_Chunk *entry_array_chunk = lnk_section_push_chunk_list(dir_sect, dir_node_chunk, str8(0,0));
          lnk_chunk_set_debugf(dir_sect->arena, dir_header_chunk, "DIR_HEADER_CHUNK");
          lnk_chunk_set_debugf(dir_sect->arena, entry_array_chunk, "DIR_ENTRY_ARRAY_CHUNK");
          
          // push symbols to patch coff entry
          LNK_Symbol *flag_symbol = lnk_make_defined_symbol_va(symtab->arena, flag_name, LNK_DefinedSymbolVisibility_Internal, 0, COFF_RESOURCE_SUB_DIR_FLAG);
          LNK_Symbol *offset_symbol = lnk_make_defined_symbol_chunk(symtab->arena, offset_name, LNK_DefinedSymbolVisibility_Internal, 0, dir_header_chunk, 0, 0, 0);
          lnk_symbol_table_push(symtab, flag_symbol); // set high bit to indicate directory
          lnk_symbol_table_push(symtab, offset_symbol); // write offset for this directory
          
          // patch resource dir header
          if (stack->coff_entry_chunk) {
            lnk_section_push_reloc(dir_sect, stack->coff_entry_chunk, LNK_Reloc_ADDR_32, OffsetOf(COFF_ResourceDirEntry, id.data_entry_offset), flag_symbol);
            lnk_section_push_reloc(dir_sect, stack->coff_entry_chunk, LNK_Reloc_SECT_REL, OffsetOf(COFF_ResourceDirEntry, id.data_entry_offset), offset_symbol);
          }
          
          // sort entries by id
          PE_ResourceArray named_array = pe_resource_list_to_array(scratch.arena, &res->u.dir->named_list);
          PE_ResourceArray id_array = pe_resource_list_to_array(scratch.arena, &res->u.dir->id_list);
          radsort(named_array.v, named_array.count, lnk_res_string_id_is_before);
          radsort(id_array.v, id_array.count, lnk_res_number_id_is_before);
          
          // frame for sub directory
          struct stack_s *frame = push_array(scratch.arena, struct stack_s, 1);
          frame->coff_entry_array_chunk = entry_array_chunk;
          frame->res_arr[0] = named_array;
          frame->res_arr[1] = id_array;
          SLLStackPush(stack, frame);
        } goto yeild; // recurse to sub directory
        
        case PE_ResDataKind_COFF_RESOURCE: {
          COFF_ResourceDataEntry *coff_resource_data_entry = push_array(dir_sect->arena, COFF_ResourceDataEntry, 1);
          coff_resource_data_entry->data_size = res->u.coff_res.data.size;
          coff_resource_data_entry->data_voff = 0; // relocated
          coff_resource_data_entry->code_page = 0; // TODO: whats this for? (lld-link writes zero)
          
          // push layout chunks
          LNK_Chunk *coff_resource_data_entry_chunk = lnk_section_push_chunk_data(dir_sect, dir_data_chunk, str8_struct(coff_resource_data_entry), str8(0,0));
          LNK_Chunk *resource_data_chunk = lnk_section_push_chunk_data(data_sect, data_sect->root, res->u.coff_res.data, str8(0,0));
          
          // windows errors out on unaligned data
          coff_resource_data_entry_chunk->align = ALIGN;
          resource_data_chunk->align = ALIGN;
          
          // relocate data
          String8 resource_data_symbol_name = push_str8f(symtab->arena, "$R%06X", res_counter);
          LNK_Symbol *resource_data_symbol = lnk_make_defined_symbol_chunk(symtab->arena, resource_data_symbol_name, LNK_DefinedSymbolVisibility_Static, 0, resource_data_chunk, 0, 0, 0);
          lnk_section_push_reloc(dir_sect, coff_resource_data_entry_chunk, LNK_Reloc_VIRT_OFF_32, OffsetOf(COFF_ResourceDataEntry, data_voff), resource_data_symbol);
          
          // push symbol for data offset relocation
          LNK_Symbol *coff_data_offset_symbol = lnk_make_defined_symbol_chunk(symtab->arena, offset_name, LNK_DefinedSymbolVisibility_Internal, 0, coff_resource_data_entry_chunk, 0, 0, 0);
          lnk_symbol_table_push(symtab, coff_data_offset_symbol);
          
          Assert(stack->coff_entry_chunk);
          lnk_section_push_reloc(dir_sect, stack->coff_entry_chunk, LNK_Reloc_SECT_REL, OffsetOf(COFF_ResourceDirEntry, id.data_entry_offset), coff_data_offset_symbol);
        } break;
        
        case PE_ResDataKind_NULL: break;
        
        // we must not have this resource node here, it is used to represent on-disk version of entry
        case PE_ResDataKind_COFF_LEAF: InvalidPath;
        }
      }
      ++stack->arr_idx;
    }
    SLLStackPop(stack);
    yeild:;
  }
  
  scratch_end(scratch);
  ProfEnd();
}

internal void
lnk_add_resource_debug_s(LNK_SectionTable *st,
                         LNK_SymbolTable  *symtab,
                         String8           obj_path,
                         String8           cwd_path,
                         String8           exe_path,
                         CV_Arch           arch,
                         String8List       res_file_list,
                         MD5Hash          *res_hash_array)
{
  ProfBeginFunction();
  Temp scratch = scratch_begin(0,0);
  
  // init serial for tables
  String8List string_srl, file_srl; MemoryZeroStruct(&string_srl); MemoryZeroStruct(&file_srl);
  str8_serial_begin(scratch.arena, &string_srl);
  str8_serial_begin(scratch.arena, &file_srl);
  
  // reserve first byte for null
  str8_serial_push_u8(scratch.arena, &string_srl, 0);
  
  // build file and string table
  U64 node_idx = 0;
  for (String8Node *n = res_file_list.first; n != NULL; n = n->next, ++node_idx) {
    CV_C13Checksum checksum = {0};
    checksum.name_off = string_srl.total_size;
    checksum.len = sizeof(MD5Hash);
    checksum.kind = CV_C13ChecksumKind_MD5;
    str8_serial_push_struct(scratch.arena, &file_srl, &checksum);
    str8_serial_push_struct(scratch.arena, &file_srl, &res_hash_array[node_idx]);
    str8_serial_push_align(scratch.arena, &file_srl, CV_FileCheckSumsAlign);
    str8_serial_push_cstr(scratch.arena, &string_srl, n->string);
  }
  
  // build symbols
  String8 obj_data = cv_make_obj_name(scratch.arena, obj_path, 0);
  
  String8 exe_name_with_ext = str8_skip_last_slash(exe_path);
  String8 exe_name_ext = str8_skip_last_dot(exe_name_with_ext);
  String8 exe_name = str8_chop(exe_name_with_ext, exe_name_ext.size);
  if (exe_name_ext.size > 0) {
    exe_name = str8_chop(exe_name, 1);
  }
  String8 version_string = push_str8f(scratch.arena, BUILD_TITLE);
  String8 comp_data = cv_make_comp3(scratch.arena, CV_Compile3Flag_EC, CV_Language_CVTRES, arch,
                                    0, 0, 0, 0,
                                    1, 0, 1, 0,
                                    version_string);
  
  String8List env_list; MemoryZeroStruct(&env_list);
  str8_list_push(scratch.arena, &env_list, str8_lit("cwd"));
  str8_list_push(scratch.arena, &env_list, cwd_path);
  str8_list_push(scratch.arena, &env_list, str8_lit("exe"));
  str8_list_push(scratch.arena, &env_list, exe_path);
  str8_list_push(scratch.arena, &env_list, str8_lit(""));
  str8_list_push(scratch.arena, &env_list, str8_lit(""));
  String8 envblock_data = cv_make_envblock(scratch.arena, env_list);
  
  String8 obj_symbol = cv_make_symbol(scratch.arena, CV_SymKind_OBJNAME, obj_data);
  String8 comp_symbol = cv_make_symbol(scratch.arena, CV_SymKind_COMPILE3, comp_data);
  String8 envblock_symbol = cv_make_symbol(scratch.arena, CV_SymKind_ENVBLOCK, envblock_data);
  
  String8List symbol_srl; MemoryZeroStruct(&symbol_srl);
  str8_serial_begin(scratch.arena, &symbol_srl);
  str8_serial_push_string(scratch.arena, &symbol_srl, obj_symbol);
  str8_serial_push_string(scratch.arena, &symbol_srl, comp_symbol);
  str8_serial_push_string(scratch.arena, &symbol_srl, envblock_symbol);
  
  // build code view sub-sections
  String8List sub_sect_srl; MemoryZeroStruct(&sub_sect_srl);
  str8_serial_begin(scratch.arena, &sub_sect_srl);
  CV_Signature sig = CV_Signature_C13;
  str8_serial_push_struct(scratch.arena, &sub_sect_srl, &sig);
  
  CV_C13SubSectionHeader string_header;
  string_header.kind = CV_C13SubSectionKind_StringTable;
  string_header.size = string_srl.total_size;
  str8_serial_push_struct(scratch.arena, &sub_sect_srl, &string_header);
  str8_serial_push_data_list(scratch.arena, &sub_sect_srl, string_srl.first);
  str8_serial_push_align(scratch.arena, &sub_sect_srl, CV_C13SubSectionAlign);
  
  CV_C13SubSectionHeader file_header;
  file_header.kind = CV_C13SubSectionKind_FileChksms;
  file_header.size = file_srl.total_size;
  str8_serial_push_struct(scratch.arena, &sub_sect_srl, &file_header);
  str8_serial_push_data_list(scratch.arena, &sub_sect_srl, file_srl.first);
  str8_serial_push_align(scratch.arena, &sub_sect_srl, CV_C13SubSectionAlign);
  
  CV_C13SubSectionHeader symbol_header;
  symbol_header.kind = CV_C13SubSectionKind_Symbols;
  symbol_header.size = symbol_srl.total_size;
  str8_serial_push_struct(scratch.arena, &sub_sect_srl, &symbol_header);
  str8_serial_push_data_list(scratch.arena, &sub_sect_srl, symbol_srl.first);
  str8_serial_push_align(scratch.arena, &sub_sect_srl, CV_C13SubSectionAlign);
  
  LNK_Section *debug_s = lnk_section_table_push(st, str8_lit(".debug$S"), LNK_DEBUG_SECTION_FLAGS);
  String8 sub_sect_data = str8_serial_end(debug_s->arena, &sub_sect_srl);
  lnk_section_push_chunk_data(debug_s, debug_s->root, sub_sect_data, str8(0,0));
  
  scratch_end(scratch);
  ProfEnd();
}

internal String8
lnk_make_res_obj(TP_Context       *tp,
                 Arena            *arena,
                 PE_ResourceDir   *root_dir,
                 COFF_MachineType  machine,
                 COFF_TimeStamp    time_stamp,
                 String8           path,
                 String8           cwd_path,
                 String8           exe_path,
                 String8List       res_file_list,
                 MD5Hash          *res_hash_array)
{
  ProfBeginFunction();
  Temp scratch = scratch_begin(&arena, 1);
  
  static const U64 sect_virt_align = 1;
  static const U64 sect_file_align = 1;
  
  LNK_SymbolTable  *symtab      = lnk_symbol_table_alloc();
  LNK_SectionTable *st          = lnk_section_table_alloc(0, sect_virt_align, sect_file_align);
  LNK_Section      *header_sect = lnk_section_table_push(st, str8_lit(".null"), 0);
  
  lnk_serialize_pe_resource_tree(st, symtab, root_dir);
  
  CV_Arch cv_arch = cv_arch_from_coff_machine(machine);
  lnk_add_resource_debug_s(st, symtab, path, cwd_path, exe_path, cv_arch, res_file_list, res_hash_array);
  
  // register section symbols (after this point don't push new sections)
  for (LNK_SectionNode *sect_node = st->list.first; sect_node != 0; sect_node = sect_node->next) {
    LNK_Section *sect        = &sect_node->data;
    LNK_Symbol  *sect_symbol = lnk_make_defined_symbol_chunk(symtab->arena, sect->name, LNK_DefinedSymbolVisibility_Internal, 0, sect->root, 0, 0, 0);
    lnk_symbol_table_push(symtab, sect_symbol);
  }
  st->null_sect = lnk_section_list_remove(&st->list, str8_lit(".null"));
  lnk_section_table_build_data(tp, st, machine);
  lnk_section_table_push_null(st);
  lnk_section_table_assign_indices(st);
  LNK_Section **sect_id_map = lnk_sect_id_map_from_section_table(scratch.arena, st);
  
  COFF_Symbol16List coff_symbol_list = {0};
  
  COFF_Symbol16 coff_feat00  = {0};
  MemoryCopyStr8(&coff_feat00.name, str8_lit("@feat.00"));
  coff_feat00.value          = COFF_FeatFlag_HAS_SAFE_SEH|COFF_FeatFlag_UNKNOWN_4;
  coff_feat00.section_number = COFF_SYMBOL_ABS_SECTION_16;
  coff_feat00.storage_class  = COFF_SymStorageClass_STATIC;
  coff_symbol16_list_push(scratch.arena, &coff_symbol_list, coff_feat00);
  
  // emit coff symbols for section definitions
  for (LNK_SectionNode *sect_node = st->list.first; sect_node != 0; sect_node = sect_node->next) {
    LNK_Section *sect = &sect_node->data;
    if (sect == header_sect) continue;
    if (!sect->emit_header) continue;
    
    U64 reloc_count = 0;
    LNK_Symbol *coff_reloc_count_symbol = lnk_symbol_table_searchf(symtab, LNK_SymbolScopeFlag_Internal, "%S.coff_relocs[].count", sect->name);
    if (coff_reloc_count_symbol) {
      reloc_count = coff_reloc_count_symbol->u.defined.u.va;
    }
    
    U64 sect_size = lnk_virt_size_from_chunk_ref(sect_id_map, sect->root->ref);
    
    COFF_Symbol16 coff_sect_symbol = {0};
    Assert(sect->name.size <= 8);
    MemoryCopyStr8(&coff_sect_symbol.name, sect->name);
    coff_sect_symbol.value            = 0;
    coff_sect_symbol.section_number   = sect->isect;
    coff_sect_symbol.aux_symbol_count = 1;
    coff_sect_symbol.storage_class    = COFF_SymStorageClass_STATIC;
    
    COFF_SymbolSecDef secdef     = {0};
    secdef.length                = safe_cast_u32(sect_size);
    secdef.number                = sect->isect;
    secdef.number_of_relocations = safe_cast_u32(reloc_count);
    
    coff_symbol16_list_push(scratch.arena, &coff_symbol_list, coff_sect_symbol);
    coff_symbol16_list_push(scratch.arena, &coff_symbol_list, *((COFF_Symbol16*)&secdef));
  }
  
  // convert relocations and symbols to coff format
  {
    for (LNK_SectionNode *sect_node = st->list.first; sect_node != 0; sect_node = sect_node->next) {
      LNK_Section *sect = &sect_node->data;
      
      // filter out resource relocations
      LNK_RelocList reloc_list          = {0};
      LNK_RelocList res_data_reloc_list = {0};
      for (LNK_Reloc *reloc = sect->reloc_list.first; reloc != 0; reloc = reloc->next) {
        B32 is_reloc_symbol = str8_match(str8_lit("$R"), reloc->symbol->name, StringMatchFlag_RightSideSloppy);
        LNK_Reloc *dst;
        if (is_reloc_symbol) {
          dst = lnk_reloc_list_push(sect->arena, &res_data_reloc_list);
        } else {
          dst = lnk_reloc_list_push(sect->arena, &reloc_list);
        }
        dst->chunk     = reloc->chunk;
        dst->type      = reloc->type;
        dst->apply_off = reloc->apply_off;
        dst->symbol    = reloc->symbol;
      }
      sect->reloc_list = reloc_list;
      
      COFF_RelocList coff_reloc_list = {0};
      for (LNK_Reloc *reloc = res_data_reloc_list.first; reloc != 0; reloc = reloc->next) {
        LNK_Symbol *symbol = reloc->symbol;

        Assert(LNK_Symbol_IsDefined(symbol->type));
        Assert(symbol->u.defined.value_type == LNK_DefinedSymbolValue_Chunk);
        LNK_DefinedSymbol *def = &symbol->u.defined;
        
        // resolve symbol offset
        LNK_Section *symbol_sect   = lnk_sect_from_chunk_ref(sect_id_map, def->u.chunk->ref);
        U64          chunk_off     = lnk_off_from_chunk_ref(sect_id_map, def->u.chunk->ref);
        U64          symbol_offset = chunk_off + def->u.chunk_offset;
        U64          symbol_idx    = coff_symbol_list.count;
        
        // push coff symbol
        COFF_Symbol16 coff_symbol = {0};
        Assert(symbol->name.size <= 8);
        String8 symbol_name = push_str8f(scratch.arena, "$R%06X", symbol_offset);
        MemoryCopyStr8(&coff_symbol.name, symbol_name);
        coff_symbol.value          = symbol_offset;
        coff_symbol.section_number = symbol_sect->isect;
        coff_symbol.storage_class  = COFF_SymStorageClass_STATIC;
        coff_symbol16_list_push(scratch.arena, &coff_symbol_list, coff_symbol);
        
        // push coff reloc
        U64 reloc_off = lnk_off_from_chunk_ref(sect_id_map, reloc->chunk->ref);
        reloc_off += reloc->apply_off;

        COFF_Reloc coff_reloc = {0};
        coff_reloc.apply_off  = reloc_off;
        coff_reloc.isymbol    = safe_cast_u32(symbol_idx);
        coff_reloc.type       = lnk_ext_reloc_type_to_coff(machine, reloc->type);
        coff_reloc_list_push(scratch.arena, &coff_reloc_list, coff_reloc);
      }
      
      if (coff_reloc_list.count == 0) continue;
      
      // push section for relocation data
      String8      sect_name  = push_str8f(st->arena, "%S.relocs", sect->name);
      LNK_Section *reloc_sect = lnk_section_table_push(st, sect_name, 0);
      reloc_sect->emit_header = 0;
      
      // push chunk layout for relocations
      LNK_Chunk *reloc_array_chunk = lnk_section_push_chunk_list(reloc_sect, reloc_sect->root, str8(0,0));
      for (COFF_RelocNode *i = coff_reloc_list.first; i != 0; i = i->next) {
        String8 reloc_data = push_str8_copy(reloc_sect->arena, str8_struct(&i->data));
        lnk_section_push_chunk_data(reloc_sect, reloc_array_chunk, reloc_data, str8(0,0));
      }
      
      // emit symbols for coff section header patch
      String8     coff_reloc_symbol_name       = push_str8f(symtab->arena, "%S.coff_reloc[]", sect->name);
      String8     coff_reloc_count_symbol_name = push_str8f(symtab->arena, "%S.coff_reloc[].count", sect->name);
      LNK_Symbol *coff_reloc_symbol            = lnk_make_defined_symbol_chunk(symtab->arena, coff_reloc_symbol_name, LNK_DefinedSymbolVisibility_Internal, 0, reloc_array_chunk, 0, 0, 0);
      LNK_Symbol *coff_reloc_count_symbol      = lnk_make_defined_symbol_va(symtab->arena, coff_reloc_count_symbol_name, LNK_DefinedSymbolVisibility_Internal, 0, coff_reloc_list.count);
      lnk_symbol_table_push(symtab, coff_reloc_symbol);
      lnk_symbol_table_push(symtab, coff_reloc_count_symbol);
    }
  }

  LNK_Section *misc_sect = lnk_section_table_push(st, str8_lit(".misc"), COFF_SectionFlag_LNK_INFO|COFF_SectionFlag_LNK_REMOVE);
  misc_sect->emit_header = 0;
  
  // serialize coff symbol list
  String8List srl = {0};
  str8_serial_begin(scratch.arena, &srl);
  for (COFF_Symbol16Node *i = coff_symbol_list.first; i != 0; i = i->next) {
    str8_serial_push_struct(scratch.arena, &srl, &i->data);
  }
  String8     coff_symbol_table_data   = str8_serial_end(scratch.arena, &srl);
  LNK_Chunk  *coff_symbol_table_chunk  = lnk_section_push_chunk_data(misc_sect, misc_sect->root, coff_symbol_table_data, str8(0,0));
  LNK_Symbol *coff_symbol_table_symbol = lnk_make_defined_symbol_chunk(symtab->arena, str8_lit("COFF_SYMBOL_TABLE"), LNK_DefinedSymbolVisibility_Internal, 0, coff_symbol_table_chunk, 0, 0, 0);
  lnk_symbol_table_push(symtab, coff_symbol_table_symbol);
  
  LNK_Symbol *coff_symbol_count_symbol = lnk_make_defined_symbol_va(symtab->arena, str8_lit("COFF_SYMBOL_COUNT"), LNK_DefinedSymbolVisibility_Internal, 0, coff_symbol_list.count);
  lnk_symbol_table_push(symtab, coff_symbol_count_symbol);
  
  // build obj header
  {
    // init header
    COFF_Header *coff_header          = push_array(header_sect->arena, COFF_Header, 1);
    coff_header->machine              = machine;
    coff_header->section_count        = 0; // relocated
    coff_header->time_stamp           = time_stamp;
    coff_header->symbol_table_foff    = 0; // relocated
    coff_header->symbol_count         = 0; // relocated
    coff_header->optional_header_size = 0; // no PE header in obj
    coff_header->flags                = COFF_Flag_32BIT_MACHINE;
    
    // push coff header chunk
    String8    coff_header_data  = str8_struct(coff_header);
    LNK_Chunk *coff_header_chunk = lnk_section_push_chunk_data(header_sect, header_sect->root, coff_header_data, str8(0,0));
    
    // relocate coff header fields
    lnk_section_push_reloc_undefined(header_sect, coff_header_chunk, LNK_Reloc_ADDR_32, OffsetOf(COFF_Header, section_count), str8_lit(LNK_COFF_SECT_HEADER_COUNT_SYMBOL_NAME), LNK_SymbolScopeFlag_Internal);
    lnk_section_push_reloc(header_sect, coff_header_chunk, LNK_Reloc_FILE_OFF_32, OffsetOf(COFF_Header, symbol_table_foff), coff_symbol_table_symbol);
    lnk_section_push_reloc(header_sect, coff_header_chunk, LNK_Reloc_ADDR_32, OffsetOf(COFF_Header, symbol_count), coff_symbol_count_symbol);
    
    // push coff header symbol
    LNK_Symbol *coff_header_symbol = lnk_make_defined_symbol_chunk(symtab->arena, str8_lit(LNK_COFF_HEADER_SYMBOL_NAME), LNK_DefinedSymbolVisibility_Internal, 0, coff_header_chunk, 0, 0, 0);
    lnk_symbol_table_push(symtab, coff_header_symbol);
  }
  
  // build section headers
  {
    LNK_Chunk *coff_section_header_array_chunk = lnk_section_push_chunk_list(header_sect, header_sect->root, str8(0,0));
    for (LNK_SectionNode *sect_node = st->list.first; sect_node != 0; sect_node = sect_node->next) {
      if (sect_node == st->null_sect) continue;
      if (!sect_node->data.emit_header) continue;
      LNK_Section *sect = &sect_node->data;
      
      // init section header
      COFF_SectionHeader *coff_sect_header = push_array(header_sect->arena, COFF_SectionHeader, 1);
      Assert(sect->name.size <= sizeof(coff_sect_header->name));
      MemoryCopyStr8(&coff_sect_header->name[0], sect->name);
      coff_sect_header->flags       = sect->flags;
      coff_sect_header->vsize       = 0; // ignored
      coff_sect_header->voff        = 0; // ignored
      coff_sect_header->fsize       = 0; // relocated
      coff_sect_header->foff        = 0; // relocated
      coff_sect_header->relocs_foff = 0; // relocated
      coff_sect_header->lines_foff  = 0; // obsolete
      coff_sect_header->line_count  = 0; // obsolete
      coff_sect_header->reloc_count = 0; // relocated
      
      // push section header chunk
      String8    coff_sect_header_data  = str8_struct(coff_sect_header);
      String8    sort_index             = lnk_make_section_sort_index(header_sect->arena, str8(0,0), 0, sect->isect);
      LNK_Chunk *coff_sect_header_chunk = lnk_section_push_chunk_data(header_sect, coff_section_header_array_chunk, coff_sect_header_data, sort_index);
      lnk_chunk_set_debugf(header_sect->arena, coff_sect_header_chunk, "%S", sect->name);
      
      // patch reloc fields
      if (sect->reloc_list.count) {
        String8 coff_reloc_symbol_name       = push_str8f(scratch.arena, "%S.coff_reloc[]", sect->name);
        String8 coff_reloc_count_symbol_name = push_str8f(scratch.arena, "%S.coff_reloc[].count", sect->name);
        lnk_section_push_reloc_undefined(header_sect, coff_sect_header_chunk, LNK_Reloc_FILE_OFF_32, OffsetOf(COFF_SectionHeader, relocs_foff), coff_reloc_symbol_name, LNK_SymbolScopeFlag_Internal);
        lnk_section_push_reloc_undefined(header_sect, coff_sect_header_chunk, LNK_Reloc_ADDR_32, OffsetOf(COFF_SectionHeader, reloc_count), coff_reloc_count_symbol_name, LNK_SymbolScopeFlag_Internal);
      }
      
      // patch file fields
      if (~sect->flags & COFF_SectionFlag_CNT_UNINITIALIZED_DATA) {
        LNK_Symbol *sect_symbol = lnk_symbol_table_search(symtab, LNK_SymbolScopeFlag_Internal, sect->name);
        lnk_section_push_reloc(header_sect, coff_sect_header_chunk, LNK_Reloc_CHUNK_SIZE_FILE_32, OffsetOf(COFF_SectionHeader, fsize), sect_symbol);
        lnk_section_push_reloc(header_sect, coff_sect_header_chunk, LNK_Reloc_FILE_OFF_32, OffsetOf(COFF_SectionHeader, foff), sect_symbol);
      }
    }
    
    // push section header count symbol
    U64         symbol_count                     = coff_section_header_array_chunk->u.list->count;
    LNK_Symbol *coff_section_header_count_symbol = lnk_make_defined_symbol_va(symtab->arena, str8_lit(LNK_COFF_SECT_HEADER_COUNT_SYMBOL_NAME), LNK_DefinedSymbolVisibility_Internal, 0, symbol_count);
    lnk_symbol_table_push(symtab, coff_section_header_count_symbol);
  }
  
  lnk_section_table_assign_indices(st);
  lnk_section_table_build_data(tp, st, machine);
  lnk_section_table_assign_file_offsets(st);
  lnk_patch_relocs(tp, symtab, st, 0);
  
  String8 res_obj = lnk_section_table_serialize(arena, st);
  
  lnk_section_table_release(&st);
  lnk_symbol_table_release(&symtab);
  
  scratch_end(scratch);
  ProfEnd();
  return res_obj;
}

internal String8
lnk_obj_from_res_file_list(TP_Context       *tp,
                           Arena            *arena,
                           LNK_SectionTable *st,
                           LNK_SymbolTable  *symtab,
                           String8List       res_data_list,
                           String8List       res_path_list,
                           COFF_MachineType  machine,
                           U32               time_stamp,
                           String8           work_dir,
                           PathStyle         system_path_style,
                           String8           obj_name)
{
  ProfBeginFunction();
  Temp scratch = scratch_begin(0,0);

  Assert(res_data_list.node_count == res_path_list.node_count);
  
  // load res files
  PE_ResourceDir *root_dir       = push_array(scratch.arena, PE_ResourceDir, 1);
  MD5Hash        *res_hash_array = push_array(scratch.arena, MD5Hash, res_data_list.node_count);
  U64 node_idx = 0;
  for (String8Node *node = res_data_list.first; node != 0; node = node->next, node_idx += 1) {
    res_hash_array[node_idx] = md5_hash_from_string(node->string);
    pe_resource_dir_push_res_file(scratch.arena, root_dir, node->string);
  }
  
  // convert res paths to stable paths
  String8List stable_res_file_list = {0};
  for (String8Node *node = res_path_list.first; node != 0; node = node->next) {
    String8 stable_res_path = lnk_make_full_path(scratch.arena, work_dir, system_path_style, node->string);
    str8_list_push(scratch.arena, &stable_res_file_list, stable_res_path);
  }

  // convert res to obj
  OS_ProcessInfo *process_info = os_get_process_info();
  String8List exe_path_strs = {0};
  str8_list_push(scratch.arena, &exe_path_strs, process_info->binary_path);
  String8 exe_path = str8_list_first(&exe_path_strs);
  String8 res_obj = lnk_make_res_obj(tp,
                                     arena,
                                     root_dir,
                                     machine,
                                     time_stamp,
                                     obj_name,
                                     work_dir,
                                     exe_path,
                                     stable_res_file_list,
                                     res_hash_array);
  
  scratch_end(scratch);
  ProfEnd();
  return res_obj;
}

////////////////////////////////

internal String8
lnk_make_linker_coff_obj(TP_Context       *tp,
                         Arena            *arena,
                         COFF_TimeStamp    time_stamp,
                         COFF_MachineType  machine,
                         String8           cwd_path,
                         String8           exe_path,
                         String8           pdb_path,
                         String8           cmd_line,
                         String8           obj_name)
{
  Temp scratch = scratch_begin(&arena, 1);
  
  LNK_SymbolTable *symtab = lnk_symbol_table_alloc();
  LNK_SectionTable *st = lnk_section_table_alloc(0, 1, 1);
  
  LNK_Section *header_sect = lnk_section_table_push(st, str8_lit(".coffhdr"), 0);
  LNK_Section *debug_s_sect = lnk_section_table_push(st, str8_lit(".debug$S"), LNK_DEBUG_SECTION_FLAGS);
  
  // TODO: remove! hack!
  header_sect->emit_header = 0;
  
  {
    COFF_Header *coff_header = push_array(header_sect->arena, COFF_Header, 1);
    coff_header->machine = machine;
    coff_header->section_count = 0;
    coff_header->time_stamp = time_stamp;
    
    LNK_Chunk *coff_header_chunk = lnk_section_push_chunk_raw(header_sect, header_sect->root, coff_header, sizeof(*coff_header), str8(0,0));
    lnk_section_push_reloc_undefined(header_sect, coff_header_chunk, LNK_Reloc_ADDR_32, OffsetOf(COFF_Header, section_count), str8_lit(LNK_COFF_SECT_HEADER_COUNT_SYMBOL_NAME), LNK_SymbolScopeFlag_Internal);
  }
  
  {
    CV_SymbolList symbol_list = {0};
    symbol_list.signature = CV_Signature_C13;
    
    // S_OBJ
    String8 obj_data = cv_make_obj_name(scratch.arena, obj_name, 0);
    cv_symbol_list_push_data(scratch.arena, &symbol_list, CV_SymKind_OBJNAME, obj_data);
    
    // S_COMPILE3
    CV_Arch cv_arch = cv_arch_from_coff_machine(machine);
    U64 ver_fe_major = 0;
    U64 ver_fe_minor = 0;
    U64 ver_fe_build = 0;
    U64 ver_feqfe = 0;
    U64 ver_major = 14;
    U64 ver_minor = 36;
    U64 ver_build = 32537;
    U64 ver_qfe = 0;
    String8 version_string = push_str8f(scratch.arena, "Epic Games Tools (R) RAD Linker");
    String8 comp3_data = cv_make_comp3(scratch.arena, 0, CV_Language_LINK, cv_arch, ver_fe_major, ver_fe_minor, ver_fe_build, ver_feqfe, ver_major, ver_minor, ver_build, ver_qfe, version_string);
    cv_symbol_list_push_data(scratch.arena, &symbol_list, CV_SymKind_COMPILE3, comp3_data);
    
    // S_ENVBLOCK
    String8List env_list = {0};
    str8_list_push(scratch.arena, &env_list, str8_lit("cwd"));
    str8_list_push(scratch.arena, &env_list, cwd_path);
    str8_list_push(scratch.arena, &env_list, str8_lit("exe"));
    str8_list_push(scratch.arena, &env_list, exe_path);
    str8_list_push(scratch.arena, &env_list, str8_lit("pdb"));
    str8_list_push(scratch.arena, &env_list, pdb_path);
    str8_list_push(scratch.arena, &env_list, str8_lit("cmd"));
    str8_list_push(scratch.arena, &env_list, cmd_line);
    str8_list_push(scratch.arena, &env_list, str8_lit(""));
    str8_list_push(scratch.arena, &env_list, str8_lit(""));
    String8 env_data = cv_make_envblock(scratch.arena, env_list);
    cv_symbol_list_push_data(scratch.arena, &symbol_list, CV_SymKind_ENVBLOCK, env_data);
    
    // TODO: emit S_SECTION and S_COFFGROUP
    // TODO: emit S_TRAMPOLINE
    
    String8List symbol_data_list = cv_data_from_symbol_list(scratch.arena, symbol_list, CV_SymbolAlign);
    
    CV_DebugS debug_s = {0};
    
    String8List *symbols_list_ptr = cv_sub_section_ptr_from_debug_s(&debug_s, CV_C13SubSectionKind_Symbols);
    *symbols_list_ptr = symbol_data_list;
    
    B32 include_sig = 1;
    String8List debug_s_data_list = cv_data_c13_from_debug_s(scratch.arena, &debug_s, include_sig);
    
    // push debug info to section
    String8 debug_s_data = str8_list_join(debug_s_sect->arena, &debug_s_data_list, 0);
    lnk_section_push_chunk_data(debug_s_sect, debug_s_sect->root, debug_s_data, str8(0,0));
  }
  
  {
    // register section symbols (after this point don't push new sections)
    for (LNK_SectionNode *sect_node = st->list.first; sect_node != NULL; sect_node = sect_node->next) {
      LNK_Section *sect = &sect_node->data;
      LNK_Symbol *sect_symbol = lnk_make_defined_symbol_chunk(symtab->arena, sect->name, LNK_DefinedSymbolVisibility_Internal, 0, sect->root, 0, 0, 0);
      lnk_symbol_table_push(symtab, sect_symbol);
    }
    
    LNK_Chunk *coff_section_header_array_chunk = lnk_section_push_chunk_list(header_sect, header_sect->root, str8(0,0));
    for (LNK_SectionNode *sect_node = st->list.first; sect_node != NULL; sect_node = sect_node->next) {
      if (sect_node == st->null_sect) continue;
      if (!sect_node->data.emit_header) continue;
      LNK_Section *sect = &sect_node->data;
      
      // init section header
      COFF_SectionHeader *coff_sect_header = push_array(header_sect->arena, COFF_SectionHeader, 1);
      Assert(sect->name.size <= sizeof(coff_sect_header->name));
      MemoryCopy(&coff_sect_header->name[0], sect->name.str, sect->name.size);
      coff_sect_header->flags       = sect->flags;
      coff_sect_header->vsize       = 0; // ignored
      coff_sect_header->voff        = 0; // ignored
      coff_sect_header->fsize       = 0; // relocated
      coff_sect_header->foff        = 0; // relocated
      coff_sect_header->relocs_foff = 0; // relocated
      coff_sect_header->lines_foff  = 0; // obsolete
      coff_sect_header->line_count  = 0; // obsolete
      coff_sect_header->reloc_count = 0; // relocated
      
      // push section header chunk
      String8 sort_index = lnk_make_section_sort_index(header_sect->arena, str8(0,0), 0, sect->isect);
      LNK_Chunk *coff_sect_header_chunk = lnk_section_push_chunk_raw(header_sect, coff_section_header_array_chunk, coff_sect_header, sizeof(*coff_sect_header), sort_index);
      lnk_chunk_set_debugf(header_sect->arena, coff_sect_header_chunk, "%S", sect->name);
      
      // emit relocs for reloc fields
      if (sect->reloc_list.count) {
        String8 coff_reloc_symbol_name = push_str8f(scratch.arena, "%S.coff_reloc[]", sect->name);
        String8 coff_reloc_count_symbol_name = push_str8f(scratch.arena, "%S.coff_reloc[].count", sect->name);
        lnk_section_push_reloc_undefined(header_sect, coff_sect_header_chunk, LNK_Reloc_FILE_OFF_32, OffsetOf(COFF_SectionHeader, relocs_foff), coff_reloc_symbol_name, LNK_SymbolScopeFlag_Internal);
        lnk_section_push_reloc_undefined(header_sect, coff_sect_header_chunk, LNK_Reloc_ADDR_32, OffsetOf(COFF_SectionHeader, reloc_count), coff_reloc_count_symbol_name, LNK_SymbolScopeFlag_Internal);
      }
      
      // emit relocs for file fields
      if (~sect->flags & COFF_SectionFlag_CNT_UNINITIALIZED_DATA) {
        LNK_Symbol *sect_symbol = lnk_symbol_table_search(symtab, LNK_SymbolScopeFlag_Internal, sect->name);
        lnk_section_push_reloc(header_sect, coff_sect_header_chunk, LNK_Reloc_CHUNK_SIZE_FILE_32, OffsetOf(COFF_SectionHeader, fsize), sect_symbol);
        lnk_section_push_reloc(header_sect, coff_sect_header_chunk, LNK_Reloc_FILE_OFF_32, OffsetOf(COFF_SectionHeader, foff), sect_symbol);
      }
    }
    
    // push section header count symbol
    U64 symbol_count = coff_section_header_array_chunk->u.list->count;
    LNK_Symbol *coff_section_header_count_symbol = lnk_make_defined_symbol_va(symtab->arena, str8_lit(LNK_COFF_SECT_HEADER_COUNT_SYMBOL_NAME), LNK_DefinedSymbolVisibility_Internal, 0, symbol_count);
    lnk_symbol_table_push(symtab, coff_section_header_count_symbol);
  }
  
  lnk_section_table_assign_indices(st);
  lnk_section_table_build_data(tp, st, machine);
  lnk_section_table_assign_file_offsets(st);
  lnk_patch_relocs(tp, symtab, st, 0);
  String8 coff_data = lnk_section_table_serialize(arena, st);
  
  lnk_section_table_release(&st);
  
  scratch_end(scratch);
  return coff_data;
}

////////////////////////////////

internal
THREAD_POOL_TASK_FUNC(lnk_load_thin_objs_task)
{
  LNK_InputObj *input = ((LNK_InputObj **)raw_task)[task_id];
  if (input->is_thin) {
    input->data = os_data_from_file_path(arena, input->path);
    input->has_disk_read_failed = (input->data.size == 0);
  }
}

internal String8
lnk_get_lib_name(String8 path)
{
  static String8 LIB_EXT = str8_lit_comp(".LIB");

  // strip path
  String8 name = str8_skip_last_slash(path);

  // strip extension
  String8 name_ext = str8_postfix(name, LIB_EXT.size);
  if (str8_match(name_ext, LIB_EXT, StringMatchFlag_CaseInsensitive)) {
    name = str8_chop(name, LIB_EXT.size);
  }

  return name;
}

internal B32
lnk_is_lib_disallowed(HashTable *disallow_lib_ht, String8 path)
{
  String8 lib_name = lnk_get_lib_name(path);
  return hash_table_search_path(disallow_lib_ht, lib_name) != 0;
}

internal B32
lnk_is_lib_loaded(HashTable *default_lib_ht, HashTable *loaded_lib_ht, LNK_InputSourceType input_source, String8 path)
{
  // when /defaultlib:path is comes from command line or obj directive
  // check against lib name
  if (input_source == LNK_InputSource_Default ||
      input_source == LNK_InputSource_Obj) {
    String8 lib_name = str8_skip_last_slash(path);
    if (hash_table_search_path(default_lib_ht, lib_name)) {
      return 1;
    }
  }
  return hash_table_search_path(loaded_lib_ht, path) != 0;
}

internal void
lnk_push_disallow_lib(Arena *arena, HashTable *disallow_lib_ht, String8 path)
{
  String8 lib_name = lnk_get_lib_name(path);
  hash_table_push_path_u64(arena, disallow_lib_ht, lib_name, 0);
}

internal void
lnk_push_loaded_lib(Arena     *arena,
                    HashTable *default_lib_ht,
                    HashTable *loaded_lib_ht,
                    String8    path)
{
  String8 lib_name = str8_skip_last_slash(path);
  if (!hash_table_search_path(default_lib_ht, lib_name)) {
    hash_table_push_path_u64(arena, default_lib_ht, lib_name, 0);
  }

  if (!hash_table_search_path(loaded_lib_ht, path)) {
    hash_table_push_string_u64(arena, loaded_lib_ht, path, 0);
  }
}

internal
THREAD_POOL_TASK_FUNC(lnk_lazy_initer)
{
  LNK_LazyIniter *task  = raw_task;
  Rng1U64         range = task->range_arr[task_id];
  for (U64 lib_idx = range.min; lib_idx < range.max; lib_idx += 1) {
    LNK_Lib     *lib       = &task->lib_arr[lib_idx].data;
    String8Node *name_node = lib->symbol_name_list.first;
    for (U64 symbol_idx = 0; symbol_idx < lib->symbol_count; symbol_idx += 1, name_node = name_node->next) {
      LNK_Symbol *symbol = &task->symbol_arr_arr[lib_idx][symbol_idx];
      lnk_init_lazy_symbol(symbol, name_node->string, lib, lib->member_off_arr[symbol_idx]);
    }
  }
}

internal void
lnk_push_input_from_lazy(Arena *arena, PathStyle path_style, LNK_LazySymbol *lazy, LNK_InputImportList *input_import_list, LNK_InputObjList *input_obj_list)
{
  // parse member
  COFF_ArchiveMember member_info = coff_read_archive_member(lazy->lib->data, lazy->member_offset);
  COFF_DataType      member_type = coff_data_type_from_data(member_info.data);
  
  switch (member_type) {
  case COFF_DataType_IMPORT: {
    LNK_InputImport *input = lnk_input_import_list_push(arena, input_import_list);
    input->import_header = coff_archive_import_from_data(member_info.data);
  } break;
  case COFF_DataType_BIG_OBJ:
  case COFF_DataType_OBJ: {
    String8 obj_path = coff_read_archive_long_name(lazy->lib->long_names, member_info.header.name);
    
    // obj path in thin archive has slash appended which screws up 
    // file lookup on disk; it couble be there to enable paths to symbols
    // but we don't use this feature
    String8 slash = str8_lit("/");
    if (str8_ends_with(obj_path, slash, 0)) {
      obj_path = str8_chop(obj_path, slash.size);
    }

    // obj path in thin archive is relative to directory with archive
    B32 is_thin = lazy->lib->type == COFF_Archive_Thin;
    if (is_thin) {
      Temp scratch = scratch_begin(&arena, 1);
      String8List obj_path_list; MemoryZeroStruct(&obj_path_list);
      str8_list_push(scratch.arena, &obj_path_list, str8_chop_last_slash(lazy->lib->path));
      str8_list_push(scratch.arena, &obj_path_list, obj_path);
      obj_path = str8_path_list_join_by_style(arena, &obj_path_list, path_style);
      scratch_end(scratch);
    }
    
    LNK_InputObj *input = lnk_input_obj_list_push(arena, input_obj_list);
    input->is_thin  = is_thin;
    input->dedup_id = push_str8f(arena, "%S(%S)", lazy->lib->path, obj_path);
    input->path     = obj_path;
    input->data     = member_info.data;
    input->lib_path = lazy->lib->path;
  } break;
  }
}

internal void
lnk_push_linker_symbols(LNK_SymbolTable *symtab, COFF_MachineType machine)
{
  // Emit __ImageBase symbol.
  //
  // This symbol is used with REL32 to compute delta from current IP
  // to the image base. CRT uses this trick to get to HINSTANCE * without
  // passing it around as a function argument.
  //
  //  100h: lea rax, [rip + ffffff00h] ; -100h 
  LNK_Symbol *image_base = lnk_make_defined_symbol_chunk(symtab->arena, str8_lit("__ImageBase"), LNK_DefinedSymbolVisibility_Extern, 0, g_null_chunk_ptr, 0, COFF_ComdatSelectType_ANY, 0);
  lnk_symbol_table_push(symtab, image_base);
  
  { // load config symbols
    if (machine == COFF_MachineType_X86) {
      LNK_Symbol *safe_se_handler_table = lnk_make_defined_symbol_chunk(symtab->arena, str8_lit(LNK_SAFE_SE_HANDLER_TABLE_SYMBOL_NAME), LNK_DefinedSymbolVisibility_Extern, 0, g_null_chunk_ptr, 0, COFF_ComdatSelectType_NODUPLICATES, 0);
      LNK_Symbol *safe_se_handler_count = lnk_make_defined_symbol_chunk(symtab->arena, str8_lit(LNK_SAFE_SE_HANDLER_COUNT_SYMBOL_NAME), LNK_DefinedSymbolVisibility_Extern, 0, g_null_chunk_ptr, 0, COFF_ComdatSelectType_NODUPLICATES, 0);
      lnk_symbol_table_push(symtab, safe_se_handler_table);
      lnk_symbol_table_push(symtab, safe_se_handler_count);
    }
    
    // TODO: investigate IMAGE_ENCLAVE_CONFIG 32/64
    LNK_Symbol *enclave_config = lnk_make_defined_symbol_va(symtab->arena, str8_lit(LNK_ENCLAVE_CONFIG_SYMBOL_NAME), LNK_DefinedSymbolVisibility_Extern, 0, 0);
    
    LNK_Symbol *guard_flags         = lnk_make_defined_symbol(symtab->arena, str8_lit(LNK_GUARD_FLAGS_SYMBOL_NAME)        , LNK_DefinedSymbolVisibility_Extern, 0);
    LNK_Symbol *guard_fids_table    = lnk_make_defined_symbol(symtab->arena, str8_lit(LNK_GUARD_FIDS_TABLE_SYMBOL_NAME)   , LNK_DefinedSymbolVisibility_Extern, 0);
    LNK_Symbol *guard_fids_count    = lnk_make_defined_symbol(symtab->arena, str8_lit(LNK_GUARD_FIDS_COUNT_SYMBOL_NAME)   , LNK_DefinedSymbolVisibility_Extern, 0);
    LNK_Symbol *guard_iat_table     = lnk_make_defined_symbol(symtab->arena, str8_lit(LNK_GUARD_IAT_TABLE_SYMBOL_NAME)    , LNK_DefinedSymbolVisibility_Extern, 0);
    LNK_Symbol *guard_iat_count     = lnk_make_defined_symbol(symtab->arena, str8_lit(LNK_GUARD_IAT_COUNT_SYMBOL_NAME)    , LNK_DefinedSymbolVisibility_Extern, 0);
    LNK_Symbol *guard_longjmp_table = lnk_make_defined_symbol(symtab->arena, str8_lit(LNK_GUARD_LONGJMP_TABLE_SYMBOL_NAME), LNK_DefinedSymbolVisibility_Extern, 0);
    LNK_Symbol *guard_longjmp_count = lnk_make_defined_symbol(symtab->arena, str8_lit(LNK_GUARD_LONGJMP_COUNT_SYMBOL_NAME), LNK_DefinedSymbolVisibility_Extern, 0);
    LNK_Symbol *guard_ehcont_table  = lnk_make_defined_symbol(symtab->arena, str8_lit(LNK_GUARD_EHCONT_TABLE_SYMBOL_NAME) , LNK_DefinedSymbolVisibility_Extern, 0);
    LNK_Symbol *guard_ehcont_count  = lnk_make_defined_symbol(symtab->arena, str8_lit(LNK_GUARD_EHCONT_COUNT_SYMBOL_NAME) , LNK_DefinedSymbolVisibility_Extern, 0);
    
    lnk_symbol_table_push(symtab, enclave_config);
    lnk_symbol_table_push(symtab, guard_flags);
    lnk_symbol_table_push(symtab, guard_fids_table);
    lnk_symbol_table_push(symtab, guard_fids_count);
    lnk_symbol_table_push(symtab, guard_iat_table);
    lnk_symbol_table_push(symtab, guard_iat_count);
    lnk_symbol_table_push(symtab, guard_longjmp_table);
    lnk_symbol_table_push(symtab, guard_longjmp_count);
    lnk_symbol_table_push(symtab, guard_ehcont_table);
    lnk_symbol_table_push(symtab, guard_ehcont_count);
  }
}

////////////////////////////////

internal void
lnk_push_coff_symbols_from_data(Arena *arena, LNK_SymbolList *symbol_list, String8 data, LNK_SymbolArray obj_symbols)
{
  if (data.size % sizeof(U32)) {
    // TODO: report invalid data size
  }
  U64 count = data.size / sizeof(U32);
  for (U32 *ptr = (U32*)data.str, *opl = ptr + count; ptr < opl; ++ptr) {
    U32 coff_symbol_idx = *ptr;
    if (coff_symbol_idx >= obj_symbols.count) {
      // TODO: report invalid symbol index
      continue;
    }
    Assert(coff_symbol_idx < obj_symbols.count);
    LNK_Symbol *symbol = obj_symbols.v + coff_symbol_idx;
    lnk_symbol_list_push(arena, symbol_list, symbol);
  }
}

internal String8
lnk_build_guard_data(Arena *arena, U64Array voff_arr, U64 stride)
{
  Assert(stride >= sizeof(U32));
  
  // check for duplicates
#if DEBUG
  for (U64 i = 1; i < voff_arr.count; ++i) {
    Assert(voff_arr.[i-1] != voff_ptr[i]);
  }
#endif
  
  U64 buffer_size = stride * voff_arr.count;
  U8 *buffer = push_array(arena, U8, buffer_size);
  for (U64 i = 0; i < voff_arr.count; ++i) {
    U32 *voff_ptr = (U32*)(buffer + i * stride);
    *voff_ptr = voff_arr.v[i];
  }
  
  String8 guard_data = str8(buffer, buffer_size);
  return guard_data;
}

internal void
lnk_push_pe_debug_data_directory(LNK_Section           *sect,
                                 LNK_Chunk             *dir_array_chunk,
                                 LNK_Symbol            *data_symbol,
                                 PE_DebugDirectoryType  type,
                                 COFF_TimeStamp         time_stamp)
{
  // init directory
  PE_DebugDirectory *dir = push_array(sect->arena, PE_DebugDirectory, 1);
  dir->time_stamp        = time_stamp;
  dir->type              = type;
  //dir->voff            = 0; // relocated through 'symbol'
  //dir->foff            = 0; // relocated through 'symbol'
  //dir->size            = 0; // relocated through 'symbol'

  // push chunk
  LNK_Chunk *dir_entry_chunk = lnk_section_push_chunk_data(sect, dir_array_chunk, str8_struct(dir), str8(0,0));
  lnk_chunk_set_debugf(sect->arena, dir_entry_chunk, "DebugDirectory[%u]", type);

  // push debug directory relocs
  lnk_section_push_reloc(sect, dir_entry_chunk, LNK_Reloc_VIRT_OFF_32,        OffsetOf(PE_DebugDirectory, voff), data_symbol);
  lnk_section_push_reloc(sect, dir_entry_chunk, LNK_Reloc_FILE_OFF_32,        OffsetOf(PE_DebugDirectory, foff), data_symbol);
  lnk_section_push_reloc(sect, dir_entry_chunk, LNK_Reloc_CHUNK_SIZE_VIRT_32, OffsetOf(PE_DebugDirectory, size), data_symbol);
}

internal void
lnk_build_debug_pdb(LNK_SectionTable *st,
                    LNK_SymbolTable  *symtab,
                    LNK_Section      *sect,
                    LNK_Chunk        *dir_array_chunk,
                    COFF_TimeStamp    time_stamp,
                    OS_Guid           guid,
                    U32               age,
                    String8           pdb_path)
{
  ProfBeginFunction();
  
  // push chunks
  String8    debug_pdb_data  = pe_make_debug_header_pdb70(sect->arena, guid, age, pdb_path);
  LNK_Chunk *debug_pdb_chunk = lnk_section_push_chunk_data(sect, sect->root, debug_pdb_data, str8(0, 0));
  lnk_chunk_set_debugf(sect->arena, debug_pdb_chunk, LNK_CV_HEADER_PDB70_SYMBOL_NAME);
  
  // push symbols
  LNK_Symbol *debug_pdb_symbol = lnk_make_defined_symbol_chunk(symtab->arena, str8_lit(LNK_CV_HEADER_PDB70_SYMBOL_NAME), LNK_DefinedSymbolVisibility_Internal, 0, debug_pdb_chunk, 0, 0, 0);
  lnk_symbol_table_push(symtab, debug_pdb_symbol);

  LNK_Symbol *guid_symbol = lnk_make_defined_symbol_chunk(symtab->arena, str8_lit(LNK_CV_HEADER_GUID_SYMBOL_NAME), LNK_DefinedSymbolVisibility_Internal, 0, debug_pdb_chunk, OffsetOf(PE_CvHeaderPDB70, guid), 0, 0);
  lnk_symbol_table_push(symtab, guid_symbol);

  // push debug directory
  lnk_push_pe_debug_data_directory(sect, dir_array_chunk, debug_pdb_symbol, PE_DebugDirectoryType_CODEVIEW, time_stamp);

  ProfEnd();
}

internal void
lnk_build_debug_rdi(LNK_SectionTable *st,
                    LNK_SymbolTable  *symtab,
                    LNK_Section      *debug_sect,
                    LNK_Chunk        *debug_dir_array_chunk,
                    COFF_TimeStamp    time_stamp,
                    OS_Guid           guid,
                    String8           rdi_path)
{
  ProfBeginFunction();

  LNK_Section *rdi_sect = lnk_section_table_push(st, str8_lit(".raddbg"), COFF_SectionFlag_CNT_INITIALIZED_DATA|COFF_SectionFlag_MEM_READ);

  // push chunks
  String8    debug_rdi       = pe_make_debug_header_rdi(rdi_sect->arena, guid, rdi_path);
  LNK_Chunk *debug_rdi_chunk = lnk_section_push_chunk_data(rdi_sect, rdi_sect->root, debug_rdi, str8(0,0)); lnk_chunk_set_debugf(rdi_sect->arena, debug_rdi, LNK_CV_HEADER_RDI_SYMBOL_NAME);

  // push symbols
  LNK_Symbol *debug_rdi_symbol = lnk_make_defined_symbol_chunk(symtab->arena, str8_lit(LNK_CV_HEADER_RDI_SYMBOL_NAME), LNK_DefinedSymbolVisibility_Internal, 0, debug_rdi_chunk, 0, 0, 0);
  lnk_symbol_table_push(symtab, debug_rdi_symbol);

  LNK_Symbol *guid_symbol = lnk_make_defined_symbol_chunk(symtab->arena, str8_lit(LNK_CV_HEADER_GUID_SYMBOL_NAME), LNK_DefinedSymbolVisibility_Internal, 0, debug_rdi_chunk, OffsetOf(PE_CvHeaderRDI, guid), 0, 0);
  lnk_symbol_table_push(symtab, guid_symbol);

  // push debug directory
  lnk_push_pe_debug_data_directory(debug_sect, debug_dir_array_chunk, debug_rdi_symbol, PE_DebugDirectoryType_CODEVIEW, time_stamp);

  ProfEnd();
}

internal void
lnk_build_guard_tables(TP_Context       *tp,
                       LNK_SectionTable *st,
                       LNK_SymbolTable  *symtab,
                       LNK_ExportTable  *exptab,
                       LNK_ObjList       obj_list,
                       COFF_MachineType  machine,
                       String8           entry_point_name,
                       LNK_GuardFlags    guard_flags,
                       B32               emit_suppress_flag)
{
  ProfBeginFunction();
  Temp scratch = scratch_begin(0, 0);
  
  LNK_Section **sect_id_map = lnk_sect_id_map_from_section_table(scratch.arena, st);
  
  enum { GUARD_FIDS, GUARD_IATS, GUARD_LJMP, GUARD_EHCONT, GUARD_COUNT };
  LNK_SymbolList guard_symbol_list_table[GUARD_COUNT]; MemoryZeroStruct(&guard_symbol_list_table[0]);
  
  // collect symbols from objs
  for (LNK_ObjNode *obj_node = obj_list.first; obj_node != NULL; obj_node = obj_node->next) {
    LNK_Obj *obj = &obj_node->data;
    COFF_FeatFlags feat_flags = lnk_obj_get_features(obj);
    B32 has_guard_flags = (feat_flags & COFF_FeatFlag_GUARD_CF) || (feat_flags & COFF_FeatFlag_GUARD_EH_CONT);
    if (has_guard_flags) {
      LNK_SymbolArray symbol_arr = lnk_symbol_array_from_list(scratch.arena, obj->symbol_list);
      if (guard_flags & LNK_Guard_Cf) {
        LNK_ChunkList gfids_list = lnk_obj_search_chunks(scratch.arena, obj, str8_lit(".gfids"), str8(0,0), 1);
        for (LNK_ChunkNode *node = gfids_list.first; node != 0; node = node->next) {
          Assert(node->data->type == LNK_Chunk_Leaf);
          lnk_push_coff_symbols_from_data(scratch.arena, &guard_symbol_list_table[GUARD_FIDS], node->data->u.leaf, symbol_arr);
        }
        LNK_ChunkList giats_list = lnk_obj_search_chunks(scratch.arena, obj, str8_lit(".giats"), str8(0,0), 1);
        for (LNK_ChunkNode *node = giats_list.first; node != 0; node = node->next) {
          Assert(node->data->type == LNK_Chunk_Leaf);
          lnk_push_coff_symbols_from_data(scratch.arena, &guard_symbol_list_table[GUARD_IATS], node->data->u.leaf, symbol_arr);
        }
      }
      if (guard_flags & LNK_Guard_LongJmp) {
        LNK_ChunkList gljmp_list = lnk_obj_search_chunks(scratch.arena, obj, str8_lit(".gljmp"), str8(0,0), 1);
        for (LNK_ChunkNode *node = gljmp_list.first; node != 0; node = node->next) {
          Assert(node->data->type == LNK_Chunk_Leaf);
          lnk_push_coff_symbols_from_data(scratch.arena, &guard_symbol_list_table[GUARD_LJMP], node->data->u.leaf, symbol_arr);
        }
      }
      if (guard_flags & LNK_Guard_EhCont) {
        LNK_ChunkList gehcont_list = lnk_obj_search_chunks(scratch.arena, obj, str8_lit(".gehcont"), str8(0,0), 1);
        for (LNK_ChunkNode *node = gehcont_list.first; node != 0; node = node->next) {
          Assert(node->data->type == LNK_Chunk_Leaf);
          lnk_push_coff_symbols_from_data(scratch.arena, &guard_symbol_list_table[GUARD_EHCONT], node->data->u.leaf, symbol_arr);
        }
      }
    } else {
      // use relocation data in code sections to get function symbols
      for (U64 isect = 0; isect < obj->sect_count; ++isect) {
        LNK_Chunk *chunk = &obj->chunk_arr[isect];
        if (!chunk) {
          continue;
        }
        if (lnk_chunk_is_discarded(chunk)) {
          continue;
        }
        if (~chunk->flags & COFF_SectionFlag_CNT_CODE) {
          continue;
        }
        Assert(chunk->type == LNK_Chunk_Leaf);
        for (LNK_Reloc *reloc = obj->sect_reloc_list_arr[isect].first; reloc != 0; reloc = reloc->next) {
          LNK_Symbol *symbol = lnk_resolve_symbol(symtab, reloc->symbol);
          if (!LNK_Symbol_IsDefined(symbol->type)) {
            continue;
          }
          LNK_DefinedSymbol *defined_symbol = &symbol->u.defined;
          if (~defined_symbol->flags & LNK_DefinedSymbolFlag_IsFunc) {
            continue;
          }
          LNK_Chunk *symbol_chunk = defined_symbol->u.chunk;
          if (!symbol_chunk) {
            continue;
          }
          if (symbol_chunk->type != LNK_Chunk_Leaf) {
            continue;
          }
          if (~symbol_chunk->flags & COFF_SectionFlag_CNT_CODE) {
            continue;
          }
          lnk_symbol_list_push(scratch.arena, &guard_symbol_list_table[GUARD_FIDS], symbol);
        }
      }
    }
  }
  
  // entry point
  LNK_Symbol *entry_point_symbol = lnk_symbol_table_search(symtab, LNK_SymbolScopeFlag_Main, entry_point_name);
  lnk_symbol_list_push(scratch.arena, &guard_symbol_list_table[GUARD_FIDS], entry_point_symbol);
  
  // push exports
  for (LNK_Export *exp = exptab->name_export_list.first; exp != NULL; exp = exp->next) {
    lnk_symbol_list_push(scratch.arena, &guard_symbol_list_table[GUARD_FIDS], exp->symbol);
  }
  
  // TODO: push noname exports
  
  // push thunks
  LNK_SymbolScopeIndex scope_array[] = { LNK_SymbolScopeIndex_Defined, LNK_SymbolScopeIndex_Internal };
  for (U64 iscope = 0; iscope < ArrayCount(scope_array); ++iscope) {
    LNK_SymbolScopeIndex scope = scope_array[iscope];
    for (U64 ibucket = 0; ibucket < symtab->bucket_count[scope]; ++ibucket) {
      for (LNK_SymbolNode *symbol_node = symtab->buckets[scope][ibucket].first;
           symbol_node != NULL;
           symbol_node = symbol_node->next) {
        LNK_Symbol *symbol = symbol_node->data;
        if (!LNK_Symbol_IsDefined(symbol->type)) continue;
        LNK_DefinedSymbol *defined_symbol = &symbol->u.defined;
        if (~defined_symbol->flags & LNK_DefinedSymbolFlag_IsThunk) continue;
        lnk_symbol_list_push(scratch.arena, &guard_symbol_list_table[GUARD_FIDS], symbol);
      } 
    }
  }
  
  // build section data
  lnk_section_table_build_data(tp, st, machine);
  lnk_section_table_assign_virtual_offsets(st);
  
  // compute symbols virtual offsets
  U64Array guard_voff_arr_table[GUARD_COUNT];
  for (U64 i = 0; i < ArrayCount(guard_symbol_list_table); ++i) {
    U64List voff_list; MemoryZeroStruct(&voff_list);
    LNK_SymbolList symbol_list = guard_symbol_list_table[i];
    for (LNK_SymbolNode *symbol_node = symbol_list.first; symbol_node != NULL; symbol_node = symbol_node->next) {
      LNK_Symbol *symbol = lnk_resolve_symbol(symtab, symbol_node->data);
      if (!LNK_Symbol_IsDefined(symbol->type)) {
        continue;
      }
      LNK_DefinedSymbol *defined_symbol = &symbol->u.defined;
      LNK_Chunk *chunk = defined_symbol->u.chunk;
      if (!chunk) {
        continue;
      }
      if (lnk_chunk_is_discarded(chunk)) {
        continue;
      }
      U64 chunk_voff = lnk_virt_off_from_chunk_ref(sect_id_map, chunk->ref);
      U64 symbol_voff = chunk_voff + defined_symbol->u.chunk_offset;
      Assert(symbol_voff != 0);
      u64_list_push(scratch.arena, &voff_list, symbol_voff);
    }
    U64Array voff_arr = u64_array_from_list(scratch.arena, &voff_list);
    radsort(voff_arr.v, voff_arr.count, u64_compar_is_before);
    guard_voff_arr_table[i] = u64_array_remove_duplicates(scratch.arena, voff_arr);
  }
  
  // push guard sections
  static struct {
    char *name;
    char *symbol;
    int flags;
  } sect_layout[] = {
    { ".gfids",   LNK_GFIDS_SYMBOL_NAME,   LNK_GFIDS_SECTION_FLAGS   },
    { ".giats",   LNK_GIATS_SYMBOL_NAME,   LNK_GIATS_SECTION_FLAGS   },
    { ".gljmp",   LNK_GLJMP_SYMBOL_NAME,   LNK_GLJMP_SECTION_FLAGS   },
    { ".gehcont", LNK_GEHCONT_SYMBOL_NAME, LNK_GEHCONT_SECTION_FLAGS },
  };
  for (U64 i = 0; i < ArrayCount(sect_layout); ++i) {
    LNK_Section *sect = lnk_section_table_push(st, str8_cstring(sect_layout[i].name), sect_layout[i].flags);
    LNK_Symbol *symbol = lnk_make_defined_symbol_chunk(symtab->arena, str8_cstring(sect_layout[i].symbol), LNK_DefinedSymbolVisibility_Internal, 0, sect->root, 0, 0, 0);
    lnk_symbol_table_push(symtab, symbol);
  }
  
  // TODO: emit table for SEH on X86
  if (machine == COFF_MachineType_X86) {
    lnk_not_implemented("__safe_se_handler_table");
    lnk_not_implemented("__safe_se_handler_count");
  }
  
  LNK_Symbol *gfids_symbol = lnk_symbol_table_search(symtab, LNK_SymbolScopeFlag_Internal, str8_lit(LNK_GFIDS_SYMBOL_NAME));
  LNK_Symbol *giats_symbol = lnk_symbol_table_search(symtab, LNK_SymbolScopeFlag_Internal, str8_lit(LNK_GIATS_SYMBOL_NAME));
  LNK_Symbol *gljmp_symbol = lnk_symbol_table_search(symtab, LNK_SymbolScopeFlag_Internal, str8_lit(LNK_GLJMP_SYMBOL_NAME));
  LNK_Symbol *gehcont_symbol = lnk_symbol_table_search(symtab, LNK_SymbolScopeFlag_Internal, str8_lit(LNK_GEHCONT_SYMBOL_NAME));
  
  LNK_Section *gfids_sect = lnk_section_table_search_id(st, gfids_symbol->u.defined.u.chunk->ref.sect_id);
  LNK_Section *giats_sect = lnk_section_table_search_id(st, giats_symbol->u.defined.u.chunk->ref.sect_id);
  LNK_Section *gljmp_sect = lnk_section_table_search_id(st, gljmp_symbol->u.defined.u.chunk->ref.sect_id);
  LNK_Section *gehcont_sect = lnk_section_table_search_id(st, gehcont_symbol->u.defined.u.chunk->ref.sect_id);
  
  LNK_Chunk *gfids_array_chunk = gfids_sect->root;
  LNK_Chunk *giats_array_chunk = giats_sect->root;
  LNK_Chunk *gljmp_array_chunk = gljmp_sect->root;
  LNK_Chunk *gehcont_array_chunk = gehcont_sect->root;
  
  // first 4 bytes are call's destination virtual offset
  U64 entry_stride = sizeof(U32);
  if (emit_suppress_flag) {
    // 4th byte tells kernel what to do when destination VA is not in the bitmap. 
    // If byte is 1 exception is suppressed and program keeps running.
    // If zero then exception is raised with nt!_KiRaiseSecurityCheckFailure(FAST_FAIL_GUARD_ICALL_CHECK_FAILURE) and exception code 0xA.
    entry_stride = 5;
  }
  
  // make guard data from virtual offsets
  String8 gfids_data   = lnk_build_guard_data(gfids_sect->arena, guard_voff_arr_table[GUARD_FIDS], entry_stride);
  String8 giats_data   = lnk_build_guard_data(giats_sect->arena, guard_voff_arr_table[GUARD_IATS], entry_stride);
  String8 gljmp_data   = lnk_build_guard_data(gljmp_sect->arena, guard_voff_arr_table[GUARD_LJMP], entry_stride);
  String8 gehcont_data = lnk_build_guard_data(gehcont_sect->arena, guard_voff_arr_table[GUARD_EHCONT], entry_stride);
  
  // push guard data
  lnk_section_push_chunk_data(gfids_sect, gfids_array_chunk, gfids_data, str8(0,0));
  lnk_section_push_chunk_data(giats_sect, giats_array_chunk, giats_data, str8(0,0));
  lnk_section_push_chunk_data(gljmp_sect, gljmp_array_chunk, gljmp_data, str8(0,0));
  lnk_section_push_chunk_data(gehcont_sect, gehcont_array_chunk, gehcont_data, str8(0,0));
  
  LNK_Symbol *gflags_symbol = lnk_symbol_table_search(symtab, LNK_SymbolScopeFlag_Main, str8_lit(LNK_GUARD_FLAGS_SYMBOL_NAME));
  LNK_Symbol *gfids_table_symbol = lnk_symbol_table_search(symtab, LNK_SymbolScopeFlag_Main, str8_lit(LNK_GUARD_FIDS_TABLE_SYMBOL_NAME));
  LNK_Symbol *gfids_count_symbol = lnk_symbol_table_search(symtab, LNK_SymbolScopeFlag_Main, str8_lit(LNK_GUARD_FIDS_COUNT_SYMBOL_NAME));
  LNK_Symbol *giats_table_symbol = lnk_symbol_table_search(symtab, LNK_SymbolScopeFlag_Main, str8_lit(LNK_GUARD_IAT_TABLE_SYMBOL_NAME));
  LNK_Symbol *giats_count_symbol = lnk_symbol_table_search(symtab, LNK_SymbolScopeFlag_Main, str8_lit(LNK_GUARD_IAT_COUNT_SYMBOL_NAME));
  LNK_Symbol *gljmp_table_symbol = lnk_symbol_table_search(symtab, LNK_SymbolScopeFlag_Main, str8_lit(LNK_GUARD_LONGJMP_TABLE_SYMBOL_NAME));
  LNK_Symbol *gljmp_count_symbol = lnk_symbol_table_search(symtab, LNK_SymbolScopeFlag_Main, str8_lit(LNK_GUARD_LONGJMP_COUNT_SYMBOL_NAME));
  LNK_Symbol *gehcont_table_symbol = lnk_symbol_table_search(symtab, LNK_SymbolScopeFlag_Main, str8_lit(LNK_GUARD_EHCONT_TABLE_SYMBOL_NAME));
  LNK_Symbol *gehcont_count_symbol = lnk_symbol_table_search(symtab, LNK_SymbolScopeFlag_Main, str8_lit(LNK_GUARD_EHCONT_COUNT_SYMBOL_NAME));
  
  LNK_DefinedSymbol *gflags_def = &gflags_symbol->u.defined;
  LNK_DefinedSymbol *gfids_table_def = &gfids_table_symbol->u.defined;
  LNK_DefinedSymbol *gfids_count_def = &gfids_count_symbol->u.defined;
  LNK_DefinedSymbol *giats_table_def = &giats_table_symbol->u.defined;
  LNK_DefinedSymbol *giats_count_def = &giats_count_symbol->u.defined;
  LNK_DefinedSymbol *gljmp_table_def = &gljmp_table_symbol->u.defined;
  LNK_DefinedSymbol *gljmp_count_def = &gljmp_count_symbol->u.defined;
  LNK_DefinedSymbol *gehcont_table_def = &gehcont_table_symbol->u.defined;
  LNK_DefinedSymbol *gehcont_count_def = &gehcont_count_symbol->u.defined;
  
  // guard flags
  gflags_def->value_type = LNK_DefinedSymbolValue_VA;
  gflags_def->u.va = PE_LoadConfigGuardFlags_CF_INSTRUMENTED;
  if ((guard_flags & LNK_Guard_Cf)) {
    gflags_def->u.va |= PE_LoadConfigGuardFlags_CF_FUNCTION_TABLE_PRESENT;
  }
  if ((guard_flags & LNK_Guard_LongJmp) && guard_voff_arr_table[GUARD_LJMP].count) {
    gflags_def->u.va |= PE_LoadConfigGuardFlags_CF_LONGJUMP_TABLE_PRESENT;
  }
  if ((guard_flags & LNK_Guard_EhCont) && guard_voff_arr_table[GUARD_EHCONT].count) {
    gflags_def->u.va |= PE_LoadConfigGuardFlags_EH_CONTINUATION_TABLE_PRESENT;
  }
  {
    LNK_Section *didat_sect = lnk_section_table_search(st, str8_lit(".didat"));
    if (didat_sect) {
      gflags_def->u.va |= PE_LoadConfigGuardFlags_DELAYLOAD_IAT_IN_ITS_OWN_SECTION;
    }
  }
  if (entry_stride > sizeof(U32)) {
    U64 size_bit = (entry_stride - 5);
    if (emit_suppress_flag) {
      gflags_def->u.va |= PE_LoadConfigGuardFlags_CF_EXPORT_SUPPRESSION_INFO_PRESENT;
    }
    gflags_def->u.va |= (1 << size_bit) << PE_LoadConfigGuardFlags_CF_FUNCTION_TABLE_SIZE_SHIFT;
  }
  
  // gfids
  if (guard_voff_arr_table[GUARD_FIDS].count) {
    gfids_table_def->value_type = LNK_DefinedSymbolValue_Chunk;
    gfids_table_def->u.chunk = gfids_array_chunk;
  }
  gfids_count_def->value_type = LNK_DefinedSymbolValue_VA;
  gfids_count_def->u.va = guard_voff_arr_table[GUARD_FIDS].count;
  
  // giats
  if (guard_voff_arr_table[GUARD_IATS].count) {
    giats_table_def->value_type = LNK_DefinedSymbolValue_Chunk;
    giats_table_def->u.chunk = giats_array_chunk;
  }
  giats_count_def->value_type = LNK_DefinedSymbolValue_VA;
  giats_count_def->u.va = guard_voff_arr_table[GUARD_IATS].count;
  
  // gljmp
  if (guard_voff_arr_table[GUARD_LJMP].count) {
    gljmp_table_def->value_type = LNK_DefinedSymbolValue_Chunk;
    gljmp_table_def->u.chunk = gljmp_array_chunk;
  }
  gljmp_count_def->value_type = LNK_DefinedSymbolValue_VA;
  gljmp_count_def->u.va = guard_voff_arr_table[GUARD_LJMP].count;
  
  // gehcont
  if (guard_voff_arr_table[GUARD_EHCONT].count) {
    gehcont_table_def->value_type = LNK_DefinedSymbolValue_Chunk;
    gehcont_table_def->u.chunk = gehcont_array_chunk;
  }
  gehcont_count_def->value_type = LNK_DefinedSymbolValue_VA;
  gehcont_count_def->u.va = guard_voff_arr_table[GUARD_EHCONT].count;
  
  scratch_end(scratch);
  ProfEnd();
}

internal void
lnk_emit_base_reloc_info(Arena                 *arena,
                         LNK_Section          **sect_id_map,
                         U64                    page_size,
                         HashTable             *page_ht,
                         LNK_BaseRelocPageList *page_list,
                         LNK_Reloc             *reloc)
{
  B32 is_addr = (reloc->type == LNK_Reloc_ADDR_64 || reloc->type == LNK_Reloc_ADDR_32);
  if (is_addr) {
    U64 reloc_voff = lnk_virt_off_from_reloc(sect_id_map, reloc);
    U64 page_voff  = AlignDownPow2(reloc_voff, page_size);

    LNK_BaseRelocPageNode *page;
    {
      KeyValuePair *is_page_present = hash_table_search_u64(page_ht, page_voff);
      if (is_page_present) {
        page = is_page_present->value_raw;
      } else {
        // fill out page
        page = push_array(arena, LNK_BaseRelocPageNode, 1);
        page->v.voff = page_voff;

        // push page
        SLLQueuePush(page_list->first, page_list->last, page);
        page_list->count += 1;

        // register page voff
        hash_table_push_u64_raw(arena, page_ht, page_voff, page);
      }
    }

    u64_list_push(arena, &page->v.entries, reloc_voff);
  }
}

internal
THREAD_POOL_TASK_FUNC(lnk_emit_base_relocs_from_reloc_array_task)
{
  LNK_BaseRelocTask     *task      = raw_task;
  Rng1U64                range     = task->range_arr[task_id];
  LNK_BaseRelocPageList *page_list = &task->list_arr[task_id];
  HashTable             *page_ht   = task->page_ht_arr[task_id];

  for (U64 reloc_idx = range.min; reloc_idx < range.max; reloc_idx += 1) {
    LNK_Reloc *reloc = task->reloc_arr[reloc_idx];
    lnk_emit_base_reloc_info(arena, task->sect_id_map, task->page_size, page_ht, page_list, reloc);
  }
}

internal
THREAD_POOL_TASK_FUNC(lnk_emit_base_relocs_from_objs_task)
{
  ProfBeginFunction();
  LNK_ObjBaseRelocTask  *task      = raw_task;
  LNK_BaseRelocPageList *page_list = &task->list_arr[task_id];
  HashTable             *page_ht   = task->page_ht_arr[task_id];
  Rng1U64                range     = task->ranges[task_id];

  for (U64 obj_idx = range.min; obj_idx < range.max; ++obj_idx) {
    LNK_Obj *obj = task->obj_arr[obj_idx];
    for (U64 sect_idx = 0; sect_idx < obj->sect_count; sect_idx += 1) {
      B32 is_live = !lnk_chunk_is_discarded(&obj->chunk_arr[sect_idx]);
      if (is_live) {
        LNK_RelocList reloc_list = obj->sect_reloc_list_arr[sect_idx];
        for (LNK_Reloc *reloc = reloc_list.first; reloc != 0; reloc = reloc->next) {
          lnk_emit_base_reloc_info(arena, task->sect_id_map, task->page_size, page_ht, page_list, reloc);
        }
      }
    }
  }
  ProfEnd();
}

internal LNK_BaseRelocPageArray
lnk_base_reloc_page_array_from_list(Arena* arena, LNK_BaseRelocPageList list)
{
  LNK_BaseRelocPageArray result = {0};
  result.count                  = 0;
  result.v                      = push_array_no_zero(arena, LNK_BaseRelocPage, list.count);
  for (LNK_BaseRelocPageNode* n = list.first; n != 0; n = n->next) {
    result.v[result.count++] = n->v;
  }
  Assert(result.count == list.count);
  return result;
}

int
lnk_base_reloc_page_is_before(void *raw_a, void *raw_b)
{
  LNK_BaseRelocPage* a = raw_a;
  LNK_BaseRelocPage* b = raw_b;
  return a->voff < b->voff;
}

internal void
lnk_base_reloc_page_array_sort(LNK_BaseRelocPageArray arr)
{
  ProfBeginFunction();
  radsort(arr.v, arr.count, lnk_base_reloc_page_is_before);
  ProfEnd();
}

internal void
lnk_build_base_relocs(TP_Context       *tp,
                      TP_Arena         *tp_arena,
                      LNK_SectionTable *st,
                      LNK_SymbolTable  *symtab,
                      COFF_MachineType  machine,
                      U64               page_size,
                      LNK_ObjList       obj_list)
{
  ProfBeginFunction();

  TP_Temp temp = tp_temp_begin(tp_arena);

  lnk_section_table_build_data(tp, st, machine);
  lnk_section_table_assign_virtual_offsets(st);

  LNK_Section **sect_id_map = lnk_sect_id_map_from_section_table(tp_arena->v[0], st);

  LNK_BaseRelocPageList  *page_list_arr = push_array(tp_arena->v[0], LNK_BaseRelocPageList, tp->worker_count);
  HashTable             **page_ht_arr   = push_array_no_zero(tp_arena->v[0], HashTable *, tp->worker_count);
  for (U64 i = 0; i < tp->worker_count; ++i) {
    page_ht_arr[i] = hash_table_init(tp_arena->v[0], 1024);
  }

  // emit pages from relocs defined in section table
  ProfBegin("Emit Relocs From Section Table");
  for (LNK_SectionNode *sect_node = st->list.first; sect_node != 0; sect_node = sect_node->next) {
    LNK_BaseRelocTask task = {0};
    task.page_size         = page_size;
    task.sect_id_map       = sect_id_map;
    task.list_arr          = page_list_arr;
    task.page_ht_arr       = page_ht_arr;
    task.reloc_arr         = lnk_reloc_array_from_list(tp_arena->v[0], sect_node->data.reloc_list);
    task.range_arr         = tp_divide_work(tp_arena->v[0], sect_node->data.reloc_list.count, tp->worker_count);
    tp_for_parallel(tp, tp_arena, tp->worker_count, lnk_emit_base_relocs_from_reloc_array_task, &task);
  }
  ProfEnd();

  // emit pages from relocs defined in objs
  ProfBegin("Emit Relocs From Objs");
  {
    LNK_ObjBaseRelocTask task = {0};
    task.ranges               = tp_divide_work(tp_arena->v[0], obj_list.count, tp->worker_count);
    task.page_size            = page_size;
    task.sect_id_map          = sect_id_map;
    task.page_ht_arr          = page_ht_arr;
    task.list_arr             = page_list_arr;
    task.obj_arr              = lnk_obj_arr_from_list(tp_arena->v[0], obj_list);
    tp_for_parallel(tp, tp_arena, tp->worker_count, lnk_emit_base_relocs_from_objs_task, &task);
  }
  ProfEnd();

  // merge page lists

  ProfBegin("Merge Worker Page Lists");

  HashTable             *main_ht        = page_ht_arr[0];
  LNK_BaseRelocPageList *main_page_list = &page_list_arr[0];

  for (U64 list_idx = 1; list_idx < tp->worker_count; ++list_idx) {
    LNK_BaseRelocPageList src = page_list_arr[list_idx];

    for (LNK_BaseRelocPageNode *src_page = src.first, *src_next; src_page != 0; src_page = src_next) {
      src_next = src_page->next;

      KeyValuePair *is_page_present = hash_table_search_u64(main_ht, src_page->v.voff);
      if (is_page_present) {
        // page exists concat voffs
        LNK_BaseRelocPageNode *page = is_page_present->value_raw;
        Assert(page != src_page);
        u64_list_concat_in_place(&page->v.entries, &src_page->v.entries);
      } else {
        // push page to main list
        SLLQueuePush(main_page_list->first, main_page_list->last, src_page);
        main_page_list->count += 1;

        // store lookup voff 
        hash_table_push_u64_raw(tp_arena->v[0], main_ht, src_page->v.voff, src_page);
      }
    }
  }

  ProfEnd();

  if (main_page_list->count > 0) {
    LNK_Section *base_reloc_sect = lnk_section_table_push(st, str8_lit(".reloc"), LNK_RELOC_SECTION_FLAGS);
    LNK_Symbol *base_reloc_symbol = lnk_make_defined_symbol_chunk(symtab->arena, str8_lit(LNK_BASE_RELOC_SYMBOL_NAME), LNK_DefinedSymbolVisibility_Internal, 0, base_reloc_sect->root, 0, 0, 0);
    lnk_symbol_table_push(symtab, base_reloc_symbol);

    ProfBegin("Page List -> Array");
    LNK_BaseRelocPageArray page_arr = lnk_base_reloc_page_array_from_list(base_reloc_sect->arena, *main_page_list);
    ProfEnd();
    
    ProfBegin("Sort Pages on VOFF");
    lnk_base_reloc_page_array_sort(page_arr);
    ProfEnd();

    HashTable *voff_ht = hash_table_init(tp_arena->v[0], page_size);
    
    ProfBegin("Serialize Pages");
    for (U64 page_idx = 0; page_idx < page_arr.count; ++page_idx) {
      LNK_BaseRelocPage *page = &page_arr.v[page_idx];

      // push buffer
      U64 buf_align = sizeof(U32);
      U64 buf_size  = AlignPow2(sizeof(U32)*2 + sizeof(U16)*page->entries.count, buf_align);
      U8 *buf       = push_array_no_zero(base_reloc_sect->arena, U8, buf_size);

      // setup pointers into buffer
      U32 *page_voff_ptr  = (U32*)buf;
      U32 *block_size_ptr = page_voff_ptr + 1;
      U16 *reloc_arr_base = (U16*)(block_size_ptr + 1);
      U16 *reloc_arr_ptr  = reloc_arr_base;

      // write reloc array
      for (U64Node *i = page->entries.first; i != 0; i = i->next) {
        // was base reloc entry made?
        if (hash_table_search_u64(voff_ht, i->data)) {
          continue;
        }
        hash_table_push_u64_u64(tp_arena->v[0], voff_ht, i->data, 0);

        // write entry
        U64 rel_off = i->data - page->voff;
        Assert(rel_off <= page_size);
        *reloc_arr_ptr++ = PE_BaseRelocMake(PE_BaseRelocKind_DIR64, rel_off);
      }

      // write pad
      U64 pad_reloc_count = AlignPadPow2(page->entries.count, sizeof(reloc_arr_ptr[0]));
      MemoryZeroTyped(reloc_arr_ptr, pad_reloc_count); // fill pad with PE_BaseRelocKind_ABSOLUTE
      reloc_arr_ptr += pad_reloc_count;

      // compute block size
      U64 reloc_arr_size = (U64)((U8*)reloc_arr_ptr - (U8*)reloc_arr_base);
      U64 block_size     = sizeof(*page_voff_ptr) + sizeof(*block_size_ptr) + reloc_arr_size;
      
      // write header
      *page_voff_ptr  = safe_cast_u32(page->voff);
      *block_size_ptr = safe_cast_u32(block_size);
      Assert(*block_size_ptr <= buf_size);
      
      // push page chunk
      lnk_section_push_chunk_raw(base_reloc_sect, base_reloc_sect->root, buf, block_size, str8(0,0));

      // purge voffs for next run
      hash_table_purge(voff_ht);
    }
    ProfEnd();
  }

  tp_temp_end(temp);
  ProfEnd();
}

internal LNK_Chunk *
lnk_build_dos_header(LNK_SymbolTable *symtab, LNK_Section *header_sect, LNK_Chunk *parent_chunk)
{
  U32 dos_stub_size = sizeof(PE_DosHeader) + pe_dos_program.size;

  PE_DosHeader *dos_header          = push_array(header_sect->arena, PE_DosHeader, 1);
  dos_header->magic                 = PE_DOS_MAGIC;
  dos_header->last_page_size        = dos_stub_size % 512;
  dos_header->page_count            = CeilIntegerDiv(dos_stub_size, 512);
  dos_header->paragraph_header_size = sizeof(PE_DosHeader) / 16;
  dos_header->min_paragraph         = 0;
  dos_header->max_paragraph         = 0;
  dos_header->init_ss               = 0;
  dos_header->init_sp               = 0;
  dos_header->checksum              = 0;
  dos_header->init_ip               = 0xFFFF;
  dos_header->init_cs               = 0;
  dos_header->reloc_table_file_off  = sizeof(PE_DosHeader);
  dos_header->overlay_number        = 0;
  MemoryZeroStruct(dos_header->reserved);
  dos_header->oem_id                = 0;
  dos_header->oem_info              = 0;
  MemoryZeroArray(dos_header->reserved2);
  dos_header->coff_file_offset      = 0; // :coff_file_offset

  LNK_Chunk *dos_chunk         = lnk_section_push_chunk_list(header_sect, parent_chunk, str8(0,0));
  LNK_Chunk *dos_header_chunk  = lnk_section_push_chunk_raw(header_sect, dos_chunk, dos_header, sizeof(*dos_header), str8(0,0));
  LNK_Chunk *dos_program_chunk = lnk_section_push_chunk_data(header_sect, dos_chunk, pe_dos_program, str8(0,0));
  lnk_chunk_set_debugf(header_sect->arena, dos_chunk, "DOS Header & Stub");
  lnk_chunk_set_debugf(header_sect->arena, dos_header_chunk, LNK_DOS_HEADER_SYMBOL_NAME);
  lnk_chunk_set_debugf(header_sect->arena, dos_program_chunk, LNK_DOS_PROGRAM_SYMBOL_NAME);

  LNK_Symbol *dos_header_symbol  = lnk_make_defined_symbol_chunk(symtab->arena, str8_lit(LNK_DOS_HEADER_SYMBOL_NAME), LNK_DefinedSymbolVisibility_Internal, 0, dos_header_chunk, 0, 0, 0);
  LNK_Symbol *dos_program_symbol = lnk_make_defined_symbol_chunk(symtab->arena, str8_lit(LNK_DOS_PROGRAM_SYMBOL_NAME), LNK_DefinedSymbolVisibility_Internal, 0, dos_program_chunk, 0, 0, 0);
  lnk_symbol_table_push(symtab, dos_header_symbol);
  lnk_symbol_table_push(symtab, dos_program_symbol);

  // :coff_file_offset
  lnk_section_push_reloc_undefined(header_sect, dos_header_chunk, LNK_Reloc_FILE_OFF_32, OffsetOf(PE_DosHeader, coff_file_offset), str8_lit(LNK_NT_HEADERS_SYMBOL_NAME), LNK_SymbolScopeFlag_Internal);

  return dos_chunk;
}

internal LNK_Chunk *
lnk_build_pe_magic(LNK_SymbolTable *symtab, LNK_Section *header_sect, LNK_Chunk *parent)
{
  U32 *pe_magic = push_array_no_zero(header_sect->arena, U32, 1);
  *pe_magic = PE_MAGIC;
  
  LNK_Chunk *pe_magic_chunk = lnk_section_push_chunk_raw(header_sect, parent, pe_magic, sizeof(*pe_magic), str8(0,0));
  lnk_chunk_set_debugf(header_sect->arena, pe_magic_chunk, LNK_PE_MAGIC_SYMBOL_NAME);

  LNK_Symbol *pe_magic_symbol = lnk_make_defined_symbol_chunk(symtab->arena, str8_lit(LNK_PE_MAGIC_SYMBOL_NAME), LNK_DefinedSymbolVisibility_Internal, 0, pe_magic_chunk, 0, 0, 0);
  lnk_symbol_table_push(symtab, pe_magic_symbol);

  return pe_magic_chunk;
}

internal LNK_Chunk *
lnk_build_coff_file_header(LNK_SymbolTable *symtab, LNK_Section *header_sect, LNK_Chunk *parent,
                           COFF_MachineType machine, COFF_TimeStamp time_stamp, PE_ImageFileCharacteristics file_characteristics)
{
  COFF_Header *file_header          = push_array_no_zero(header_sect->arena, COFF_Header, 1);
  file_header->machine              = machine;
  file_header->time_stamp           = time_stamp;
  file_header->symbol_table_foff    = 0;
  file_header->symbol_count         = 0;
  file_header->section_count        = 0; // :section_count
  file_header->optional_header_size = 0; // :optional_header_size
  file_header->flags                = file_characteristics;
  
  LNK_Chunk *file_header_chunk = lnk_section_push_chunk_raw(header_sect, parent, file_header, sizeof(*file_header), str8(0,0));
  lnk_chunk_set_debugf(header_sect->arena, file_header_chunk, LNK_COFF_HEADER_SYMBOL_NAME);

  LNK_Symbol *file_header_symbol = lnk_make_defined_symbol_chunk(symtab->arena, str8_lit(LNK_COFF_HEADER_SYMBOL_NAME), LNK_DefinedSymbolVisibility_Internal, 0, file_header_chunk, 0, 0, 0);
  lnk_symbol_table_push(symtab, file_header_symbol);

  // :section_count
  lnk_section_push_reloc_undefined(header_sect, file_header_chunk, LNK_Reloc_ADDR_16, OffsetOf(COFF_Header, section_count), str8_lit(LNK_COFF_SECT_HEADER_COUNT_SYMBOL_NAME), LNK_SymbolScopeFlag_Internal);
  
  // :optional_header_size
  lnk_section_push_reloc_undefined(header_sect, file_header_chunk, LNK_Reloc_CHUNK_SIZE_FILE_16, OffsetOf(COFF_Header, optional_header_size), str8_lit(LNK_PE_OPT_HEADER_SYMBOL_NAME), LNK_SymbolScopeFlag_Internal);
  lnk_section_push_reloc_undefined(header_sect, file_header_chunk, LNK_Reloc_CHUNK_SIZE_FILE_16, OffsetOf(COFF_Header, optional_header_size), str8_lit(LNK_PE_DIRECTORY_ARRAY_SYMBOL_NAME), LNK_SymbolScopeFlag_Internal);

  return file_header_chunk;
}

internal LNK_Chunk *
lnk_build_pe_optional_header_x64(LNK_SymbolTable       *symtab,
                                 LNK_Section           *header_sect,
                                 LNK_Chunk             *parent,
                                 COFF_MachineType       machine,
                                 U64                    base_addr,
                                 U64                    sect_align,
                                 U64                    file_align,
                                 Version                linker_ver,
                                 Version                os_ver,
                                 Version                image_ver,
                                 Version                subsystem_ver,
                                 PE_WindowsSubsystem    subsystem,
                                 PE_DllCharacteristics  dll_characteristics,
                                 U64                    stack_reserve,
                                 U64                    stack_commit,
                                 U64                    heap_reserve,
                                 U64                    heap_commit,
                                 String8                entry_point_name,
                                 LNK_SectionArray       sect_arr)
{
  PE_OptionalHeader32Plus *opt_header = push_array_no_zero(header_sect->arena, PE_OptionalHeader32Plus, 1);
  opt_header->magic                   = PE_PE32PLUS_MAGIC;
  opt_header->major_linker_version    = linker_ver.major;
  opt_header->minor_linker_version    = linker_ver.minor;
  opt_header->sizeof_code             = 0; // :sizeof_code
  opt_header->sizeof_inited_data      = 0; // :sizeof_inited_data
  opt_header->sizeof_uninited_data    = 0; // :sizeof_uninited_data
  opt_header->entry_point_va          = 0; // :entry_point_va
  opt_header->code_base               = 0; // :code_base
  opt_header->image_base              = base_addr;
  opt_header->section_alignment       = sect_align;
  opt_header->file_alignment          = file_align;
  opt_header->major_os_ver            = os_ver.major;
  opt_header->minor_os_ver            = os_ver.minor;
  opt_header->major_img_ver           = image_ver.major;
  opt_header->minor_img_ver           = image_ver.minor;
  opt_header->major_subsystem_ver     = subsystem_ver.major;
  opt_header->minor_subsystem_ver     = subsystem_ver.minor;
  opt_header->win32_version_value     = 0; // MSVC writes zero
  opt_header->sizeof_image            = 0; // :sizeof_image
  opt_header->sizeof_headers          = 0; // :sizeof_headers
  opt_header->check_sum               = 0; // :check_sum
  opt_header->subsystem               = subsystem;
  opt_header->dll_characteristics     = dll_characteristics;
  opt_header->sizeof_stack_reserve    = stack_reserve;
  opt_header->sizeof_stack_commit     = stack_commit;
  opt_header->sizeof_heap_reserve     = heap_reserve;
  opt_header->sizeof_heap_commit      = heap_commit;
  opt_header->loader_flags            = 0; // for dynamic linker, always zero
  opt_header->data_dir_count          = 0; // :data_dir_count
  
  // push chunk
  LNK_Chunk *opt_header_chunk = lnk_section_push_chunk_raw(header_sect, parent, opt_header, sizeof(*opt_header), str8(0,0));
  lnk_chunk_set_debugf(header_sect->arena, opt_header_chunk, LNK_PE_OPT_HEADER_SYMBOL_NAME);

  // define optional header symbol
  LNK_Symbol *opt_header_symbol = lnk_make_defined_symbol_chunk(symtab->arena, str8_lit(LNK_PE_OPT_HEADER_SYMBOL_NAME), LNK_DefinedSymbolVisibility_Internal, 0, opt_header_chunk, 0, 0, 0);
  lnk_symbol_table_push(symtab, opt_header_symbol);
  
  // :entry_point_va
  lnk_section_push_reloc_undefined(header_sect, opt_header_chunk, LNK_Reloc_VIRT_OFF_32, OffsetOf(PE_OptionalHeader32Plus, entry_point_va), entry_point_name, LNK_SymbolScopeFlag_Main);
  
  // :code_base
  lnk_section_push_reloc_undefined(header_sect, opt_header_chunk, LNK_Reloc_VIRT_OFF_32, OffsetOf(PE_OptionalHeader32Plus, code_base), str8_lit(LNK_TEXT_SYMBOL_NAME), LNK_SymbolScopeFlag_Internal);

  LNK_Section *last_sect = 0;
  for (LNK_Section *sect = &sect_arr.v[0], *sect_opl = sect + sect_arr.count; sect < sect_opl; sect += 1) {
    if (!sect->has_layout) {
      continue;
    }
    // :sizeof_uninited_data
    if (sect->flags & COFF_SectionFlag_CNT_UNINITIALIZED_DATA) {
      lnk_section_push_reloc_undefined(header_sect, opt_header_chunk, LNK_Reloc_CHUNK_SIZE_VIRT_32, OffsetOf(PE_OptionalHeader32Plus, sizeof_uninited_data), sect->name, LNK_SymbolScopeFlag_Internal);
    }

    // :sizeof_inited_data
    if (sect->flags & COFF_SectionFlag_CNT_INITIALIZED_DATA) {
      lnk_section_push_reloc_undefined(header_sect, opt_header_chunk, LNK_Reloc_CHUNK_SIZE_FILE_32, OffsetOf(PE_OptionalHeader32Plus, sizeof_inited_data), sect->name, LNK_SymbolScopeFlag_Internal);
    }

    // :sizeof_code
    if (sect->flags & COFF_SectionFlag_CNT_CODE) { 
      lnk_section_push_reloc_undefined(header_sect, opt_header_chunk, LNK_Reloc_CHUNK_SIZE_FILE_32, OffsetOf(PE_OptionalHeader32Plus, sizeof_code), sect->name, LNK_SymbolScopeFlag_Internal);
    }

    last_sect = sect;
  }
  
  // :sizeof_image
  if (last_sect) {
    lnk_section_push_reloc_undefined(header_sect, opt_header_chunk, LNK_Reloc_VIRT_OFF_32, OffsetOf(PE_OptionalHeader32Plus, sizeof_image), last_sect->name, LNK_SymbolScopeFlag_Internal);
    lnk_section_push_reloc_undefined(header_sect, opt_header_chunk, LNK_Reloc_CHUNK_SIZE_VIRT_32, OffsetOf(PE_OptionalHeader32Plus, sizeof_image), last_sect->name, LNK_SymbolScopeFlag_Internal);
    lnk_section_push_reloc(header_sect, opt_header_chunk, LNK_Reloc_VIRT_ALIGN_32, OffsetOf(PE_OptionalHeader32Plus, sizeof_image), &g_null_symbol);
  }

  // :sizeof_headers
  lnk_section_push_reloc_undefined(header_sect, opt_header_chunk, LNK_Reloc_CHUNK_SIZE_FILE_32, OffsetOf(PE_OptionalHeader32Plus, sizeof_headers), str8_lit(LNK_WIN32_HEADER_SYMBOL_NAME), LNK_SymbolScopeFlag_Internal);
  lnk_section_push_reloc(header_sect, opt_header_chunk, LNK_Reloc_FILE_ALIGN_32, OffsetOf(PE_OptionalHeader32Plus, sizeof_headers), &g_null_symbol);

  // :check_sum
  LNK_Symbol *checksum_symbol = lnk_make_defined_symbol_chunk(symtab->arena, str8_lit(LNK_PE_CHECKSUM_SYMBOL_NAME), LNK_DefinedSymbolVisibility_Internal, 0, opt_header_chunk, OffsetOf(PE_OptionalHeader32Plus, check_sum), COFF_ComdatSelectType_NODUPLICATES, 0);
  lnk_symbol_table_push(symtab, checksum_symbol);

  // :data_dir_count
  lnk_section_push_reloc_undefined(header_sect, opt_header_chunk, LNK_Reloc_ADDR_32, OffsetOf(PE_OptionalHeader32Plus, data_dir_count), str8_lit(LNK_PE_DIRECTORY_COUNT_SYMBOL_NAME), LNK_SymbolScopeFlag_Internal);

  return opt_header_chunk;
}

internal LNK_Chunk *
lnk_build_pe_directories(LNK_SymbolTable *symtab, LNK_Section *header_sect, LNK_Chunk *parent)
{
  static struct {
    char *name;
    PE_DataDirectoryIndex index;
    LNK_SymbolScopeFlags scope;
  } directory_map[] = {
    { LNK_LOAD_CONFIG_SYMBOL_NAME             , PE_DataDirectoryIndex_LOAD_CONFIG , LNK_SymbolScopeFlag_Main   },
    { LNK_PDATA_SYMBOL_NAME                   , PE_DataDirectoryIndex_EXCEPTIONS  , LNK_SymbolScopeFlag_Internal },
    { LNK_EDATA_SYMBOL_NAME                   , PE_DataDirectoryIndex_EXPORT      , LNK_SymbolScopeFlag_Internal },
    { LNK_BASE_RELOC_SYMBOL_NAME              , PE_DataDirectoryIndex_BASE_RELOC  , LNK_SymbolScopeFlag_Internal },
    { LNK_IMPORT_DLL_TABLE_SYMBOL_NAME        , PE_DataDirectoryIndex_IMPORT      , LNK_SymbolScopeFlag_Internal },
    { LNK_IMPORT_IAT_SYMBOL_NAME              , PE_DataDirectoryIndex_IMPORT_ADDR , LNK_SymbolScopeFlag_Internal },
    { LNK_DELAYED_IMPORT_DLL_TABLE_SYMBOL_NAME, PE_DataDirectoryIndex_DELAY_IMPORT, LNK_SymbolScopeFlag_Internal },
    { LNK_TLS_SYMBOL_NAME                     , PE_DataDirectoryIndex_TLS         , LNK_SymbolScopeFlag_Main   },
    { LNK_DEBUG_DIR_SYMBOL_NAME               , PE_DataDirectoryIndex_DEBUG       , LNK_SymbolScopeFlag_Internal },
    { LNK_RSRC_SYMBOL_NAME                    , PE_DataDirectoryIndex_RESOURCES   , LNK_SymbolScopeFlag_Internal },
  };
  
  // init directory virtual coords from symbol names
  U64 directory_count = PE_DataDirectoryIndex_COUNT;
  PE_DataDirectory *directory_array = push_array(header_sect->arena, PE_DataDirectory, directory_count);
  
  LNK_Chunk *directory_array_chunk = lnk_section_push_chunk_raw(header_sect, parent, directory_array, sizeof(directory_array[0])*directory_count, str8(0,0));
  lnk_chunk_set_debugf(header_sect->arena, directory_array_chunk, LNK_PE_DIRECTORY_ARRAY_SYMBOL_NAME);

  // define PE directory symbols
  LNK_Symbol *directory_array_symbol = lnk_make_defined_symbol_chunk(symtab->arena, str8_lit(LNK_PE_DIRECTORY_ARRAY_SYMBOL_NAME), LNK_DefinedSymbolVisibility_Internal, 0, directory_array_chunk, 0, 0, 0);
  LNK_Symbol *directory_count_symbol = lnk_make_defined_symbol_va(symtab->arena, str8_lit(LNK_PE_DIRECTORY_COUNT_SYMBOL_NAME), LNK_DefinedSymbolVisibility_Internal, 0, directory_count);
  lnk_symbol_table_push(symtab, directory_array_symbol);
  lnk_symbol_table_push(symtab, directory_count_symbol);
  
  for (U64 dir_idx = 0; dir_idx < ArrayCount(directory_map); dir_idx += 1) {
    String8 symbol_name = str8_cstring(directory_map[dir_idx].name);
    LNK_Symbol *symbol = lnk_symbol_table_search(symtab, directory_map[dir_idx].scope, symbol_name);
    if (symbol) {
      U64 virt_off_field_off = sizeof(PE_DataDirectory) * directory_map[dir_idx].index + OffsetOf(PE_DataDirectory, virt_off);
      U64 virt_size_field_off = sizeof(PE_DataDirectory) * directory_map[dir_idx].index + OffsetOf(PE_DataDirectory, virt_size);
      lnk_section_push_reloc(header_sect, directory_array_chunk, LNK_Reloc_VIRT_OFF_32, virt_off_field_off, symbol);
      lnk_section_push_reloc(header_sect, directory_array_chunk, LNK_Reloc_CHUNK_SIZE_VIRT_32, virt_size_field_off, symbol);
    }
  }

  return directory_array_chunk;
}

internal LNK_Chunk *
lnk_build_coff_section_table(LNK_SymbolTable *symtab, LNK_Section *header_sect, LNK_Chunk *parent_chunk, LNK_SectionArray sect_arr)
{
  // register section symbols
  for (LNK_Section *sect = &sect_arr.v[0], *sect_opl = sect + sect_arr.count;
       sect < sect_opl;
       sect += 1) {
    // was section symbol defined elsewhere?
    LNK_Symbol *test_symbol = lnk_symbol_table_search(symtab, LNK_SymbolScopeFlag_Internal, sect->name);
    Assert(!test_symbol); (void)test_symbol;

    // define symbol
    String8 sect_symbol_name = push_str8_copy(symtab->arena, sect->name);
    LNK_Symbol *sect_symbol = lnk_make_defined_symbol_chunk(symtab->arena, sect_symbol_name, LNK_DefinedSymbolVisibility_Internal, 0, sect->root, 0, 0, 0);
    lnk_symbol_table_push(symtab, sect_symbol);
  }

  // push COFF header array chunk
  LNK_Chunk *coff_header_array_chunk = lnk_section_push_chunk_list(header_sect, parent_chunk, str8(0,0));
  lnk_chunk_set_debugf(header_sect->arena, coff_header_array_chunk, LNK_COFF_SECT_HEADER_ARRAY_SYMBOL_NAME);

  // define symbol for COFF header array
  LNK_Symbol *coff_header_array_symbol = lnk_make_defined_symbol_chunk(symtab->arena, str8_lit(LNK_COFF_SECT_HEADER_ARRAY_SYMBOL_NAME), LNK_DefinedSymbolVisibility_Internal, 0, coff_header_array_chunk, 0, 0, 0);
  lnk_symbol_table_push(symtab, coff_header_array_symbol);
  
  // push headers
  for (LNK_Section *sect = &sect_arr.v[0], *sect_opl = sect + sect_arr.count; sect < sect_opl; sect += 1) {
    if (!sect->emit_header) {
      continue;
    }
    if (!sect->has_layout) {
      continue;
    }
    COFF_SectionHeader *coff_header = push_array_no_zero(header_sect->arena, COFF_SectionHeader, 1);

    // TODO: for objs we can store long name in string table and write here /offset
    if (sect->name.size > sizeof(coff_header->name)) {
      lnk_error(LNK_Warning_LongSectionName, "not enough space in COFF section header to store entire name \"%S\"", sect->name);
    }

    MemorySet(&coff_header->name[0], 0, sizeof(coff_header->name));
    MemoryCopy(&coff_header->name[0], sect->name.str, Min(sect->name.size, sizeof(coff_header->name)));
    coff_header->vsize       = 0; // :vsize
    coff_header->voff        = 0; // :voff
    coff_header->fsize       = 0; // :fsize
    coff_header->foff        = 0; // :foff
    coff_header->relocs_foff = 0; // :relocs_foff
    coff_header->lines_foff  = 0; // obsolete
    coff_header->reloc_count = 0; // :reloc_count
    coff_header->line_count  = 0; // obsolete
    coff_header->flags       = sect->flags;

    // push chunk
    LNK_Chunk *coff_header_chunk = lnk_section_push_chunk_raw(header_sect, coff_header_array_chunk, coff_header, sizeof(*coff_header), str8(0,0));

    // :vsize
    lnk_section_push_reloc_undefined(header_sect, coff_header_chunk, LNK_Reloc_CHUNK_SIZE_VIRT_32, OffsetOf(COFF_SectionHeader, vsize), sect->name, LNK_SymbolScopeFlag_Internal);
    // :voff
    lnk_section_push_reloc_undefined(header_sect, coff_header_chunk, LNK_Reloc_VIRT_OFF_32, OffsetOf(COFF_SectionHeader, voff), sect->name, LNK_SymbolScopeFlag_Internal);
    
    if (~sect->flags & COFF_SectionFlag_CNT_UNINITIALIZED_DATA) {
      // :fsize
      lnk_section_push_reloc_undefined(header_sect, coff_header_chunk, LNK_Reloc_CHUNK_SIZE_FILE_32, OffsetOf(COFF_SectionHeader, fsize), sect->name, LNK_SymbolScopeFlag_Internal);
      // :foff
      lnk_section_push_reloc_undefined(header_sect, coff_header_chunk, LNK_Reloc_FILE_OFF_32, OffsetOf(COFF_SectionHeader, foff), sect->name, LNK_SymbolScopeFlag_Internal);
    }

    // TODO: :reloc_off
    // TODO: :reloc_count
  }

  // push symbol for section header count
  U64 header_count = coff_header_array_chunk->u.list->count;
  LNK_Symbol *header_symbol = lnk_make_defined_symbol_va(symtab->arena, str8_lit(LNK_COFF_SECT_HEADER_COUNT_SYMBOL_NAME), LNK_DefinedSymbolVisibility_Internal, 0, header_count);
  lnk_symbol_table_push(symtab, header_symbol);

  return coff_header_array_chunk;
}

internal LNK_Chunk *
lnk_build_win32_image_header(LNK_SymbolTable     *symtab,
                             LNK_Section         *header_sect,
                             LNK_Chunk           *parent_chunk,
                             LNK_Config          *config,
                             LNK_SectionArray     sect_arr)
{
  ProfBeginFunction();

  // header sections must be written first
  Assert(header_sect->id == 0); 
  
  LNK_Chunk *win32_header_chunk     = lnk_section_push_chunk_list(header_sect, parent_chunk      , str8(0,0)    );
  LNK_Chunk *dos_chunk              = lnk_section_push_chunk_list(header_sect, win32_header_chunk, str8_lit("a"));
  LNK_Chunk *nt_chunk               = lnk_section_push_chunk_list(header_sect, win32_header_chunk, str8_lit("b"));
  LNK_Chunk *pe_magic_chunk         = lnk_section_push_chunk_list(header_sect, nt_chunk          , str8_lit("a"));
  LNK_Chunk *coff_file_header_chunk = lnk_section_push_chunk_list(header_sect, nt_chunk          , str8_lit("b"));
  LNK_Chunk *pe_optional_chunk      = lnk_section_push_chunk_list(header_sect, nt_chunk          , str8_lit("c"));
  LNK_Chunk *coff_sect_header_chunk = lnk_section_push_chunk_list(header_sect, nt_chunk          , str8_lit("d"));

  lnk_chunk_set_debugf(header_sect->arena, win32_header_chunk    , "Win32 Headers"                 );
  lnk_chunk_set_debugf(header_sect->arena, dos_chunk             , "DOS Chunk"                     );
  lnk_chunk_set_debugf(header_sect->arena, nt_chunk              , "NT Chunk"                      );
  lnk_chunk_set_debugf(header_sect->arena, pe_magic_chunk        , "PE Magic Container"            );
  lnk_chunk_set_debugf(header_sect->arena, coff_file_header_chunk, "COFF File Header Container"    );
  lnk_chunk_set_debugf(header_sect->arena, pe_optional_chunk     , "PE Optional Header Container"  );
  lnk_chunk_set_debugf(header_sect->arena, coff_sect_header_chunk, "COFF Section Headers Container");

  LNK_Symbol *win32_header_symbol     = lnk_make_defined_symbol_chunk(symtab->arena, str8_lit(LNK_WIN32_HEADER_SYMBOL_NAME)                 , LNK_DefinedSymbolVisibility_Internal, 0, win32_header_chunk    , 0, 0, 0);
  LNK_Symbol *dos_symbol              = lnk_make_defined_symbol_chunk(symtab->arena, str8_lit(LNK_DOS_SYMBOL_NAME)                          , LNK_DefinedSymbolVisibility_Internal, 0, dos_chunk             , 0, 0, 0);
  LNK_Symbol *nt_headers_symbol       = lnk_make_defined_symbol_chunk(symtab->arena, str8_lit(LNK_NT_HEADERS_SYMBOL_NAME)                   , LNK_DefinedSymbolVisibility_Internal, 0, nt_chunk              , 0, 0, 0);
  LNK_Symbol *pe_magic_symbol         = lnk_make_defined_symbol_chunk(symtab->arena, str8_lit(LNK_PE_MAGIC_CONTAINER_SYMBOL_NAME)           , LNK_DefinedSymbolVisibility_Internal, 0, pe_magic_chunk        , 0, 0, 0);
  LNK_Symbol *coff_file_header_symbol = lnk_make_defined_symbol_chunk(symtab->arena, str8_lit(LNK_COFF_FILE_HEADER_CONTAINER_SYMBOL_NAME)   , LNK_DefinedSymbolVisibility_Internal, 0, coff_file_header_chunk, 0, 0, 0);
  LNK_Symbol *pe_optional_symbol      = lnk_make_defined_symbol_chunk(symtab->arena, str8_lit(LNK_PE_OPT_HEADER_CONTAINER_SYMBOL_NAME)      , LNK_DefinedSymbolVisibility_Internal, 0, pe_optional_chunk     , 0, 0, 0);
  LNK_Symbol *coff_sect_header_symbol = lnk_make_defined_symbol_chunk(symtab->arena, str8_lit(LNK_COFF_SECTION_HEADER_CONTAINER_SYMBOL_NAME), LNK_DefinedSymbolVisibility_Internal, 0, coff_sect_header_chunk, 0, 0, 0);

  lnk_symbol_table_push(symtab, win32_header_symbol    );
  lnk_symbol_table_push(symtab, dos_symbol             );
  lnk_symbol_table_push(symtab, nt_headers_symbol      );
  lnk_symbol_table_push(symtab, pe_magic_symbol        );
  lnk_symbol_table_push(symtab, coff_file_header_symbol);
  lnk_symbol_table_push(symtab, pe_optional_symbol     );
  lnk_symbol_table_push(symtab, coff_sect_header_symbol);

  lnk_build_dos_header(symtab, header_sect, dos_chunk);
  lnk_build_pe_magic(symtab, header_sect, pe_magic_chunk);
  lnk_build_coff_file_header(symtab, header_sect, coff_file_header_chunk, config->machine, config->time_stamp, config->file_characteristics);
  switch (config->machine) {
  case COFF_MachineType_X64: {
    lnk_build_pe_optional_header_x64(symtab,
                                     header_sect,
                                     pe_optional_chunk,
                                     config->machine,
                                     lnk_get_base_addr(config),
                                     config->sect_align,
                                     config->file_align,
                                     config->link_ver,
                                     config->os_ver,
                                     config->image_ver,
                                     config->subsystem_ver,
                                     config->subsystem,
                                     config->dll_characteristics,
                                     config->stack_reserve,
                                     config->stack_commit,
                                     config->heap_reserve,
                                     config->heap_commit,
                                     config->entry_point_name,
                                     sect_arr);
  } break;
  default: {
    lnk_not_implemented("TODO: PE Optional Header for %S", coff_string_from_machine_type(config->machine));
  } break;
  }
  lnk_build_pe_directories(symtab, header_sect, pe_optional_chunk);
  lnk_build_coff_section_table(symtab, header_sect, coff_sect_header_chunk, sect_arr);

  ProfEnd();
  return win32_header_chunk;
}

////////////////////////////////

internal
THREAD_POOL_TASK_FUNC(lnk_undef_symbol_finder)
{
  LNK_SymbolFinder       *task   = raw_task;
  LNK_SymbolFinderResult *result = &task->result_arr[task_id];
  Rng1U64                 range  = task->range_arr[task_id];

  for (U64 symbol_idx = range.min; symbol_idx < range.max; symbol_idx += 1) {
    LNK_SymbolNode         *symbol_node = task->lookup_node_arr.v[symbol_idx];
    LNK_Symbol             *symbol      = symbol_node->data;
    Assert(symbol->type == LNK_Symbol_Undefined);
    LNK_UndefinedSymbol    *undef       = &symbol->u.undefined;

    LNK_SymbolNode *has_defn = lnk_symbol_table_search_node(task->symtab, undef->scope_flags, symbol->name);
    if (has_defn) {
      Assert(LNK_Symbol_IsDefined(has_defn->data->type) || has_defn->data->type == LNK_Symbol_Weak);
      continue;
    }

    LNK_SymbolNode *lazy = lnk_symbol_table_search_node(task->symtab, LNK_SymbolScopeFlag_Lib, symbol->name);
    if (lazy) {
      lnk_push_input_from_lazy(arena, task->path_style, &lazy->data->u.lazy, &result->input_import_list, &result->input_obj_list);
    } else {
      lnk_symbol_list_push_node(&result->unresolved_symbol_list, symbol_node);
    }
  }
}

internal
THREAD_POOL_TASK_FUNC(lnk_weak_symbol_finder)
{
  LNK_SymbolFinder       *task   = raw_task;
  LNK_SymbolFinderResult *result = &task->result_arr[task_id];
  Rng1U64                 range  = task->range_arr[task_id];

  for (U64 symbol_idx = range.min; symbol_idx < range.max; symbol_idx += 1) {
    LNK_SymbolNode         *symbol_node = task->lookup_node_arr.v[symbol_idx];
    LNK_Symbol             *symbol      = symbol_node->data;
    Assert(symbol->type == LNK_Symbol_Weak);
    LNK_WeakSymbol         *weak        = &symbol->u.weak;

    Assert((weak->scope_flags & ~(LNK_SymbolScopeFlag_Defined | LNK_SymbolScopeFlag_Internal)) == 0);
    LNK_SymbolNode *has_strong_defn = lnk_symbol_table_search_node(task->symtab, weak->scope_flags, symbol->name);
    if (has_strong_defn) {
      Assert(LNK_Symbol_IsDefined(has_strong_defn->data->type));
      continue;
    }

    LNK_SymbolNode *lazy = 0;
    switch (weak->lookup_type) {
    case COFF_WeakExtType_NOLIBRARY: {
      // NOLIBRARY means weak symbol should be resolved in case where strong definition pulls in lib member.
    } break;
    case COFF_WeakExtType_SEARCH_LIBRARY: {
      lazy = lnk_symbol_table_search_node(task->symtab, LNK_SymbolScopeFlag_Lib, symbol->name);
    } break;
    case COFF_WeakExtType_SEARCH_ALIAS: {
      lazy = lnk_symbol_table_search_node(task->symtab, LNK_SymbolScopeFlag_Lib, symbol->name);
      if (!lazy) {
        if (str8_match(str8_lit(".weak."), symbol->name, StringMatchFlag_RightSideSloppy)) {
          // TODO: Clang and MingGW encode extra info in alias
          // 
          // __attribute__((weak,alias("foo"))) void bar(void);
          // static void foo() {}
          //
          // Clang write these COFF symbols in obj for code above:
          //
          // 30 00000000 0000000001 0    FUNC NULL EXTERNAL         foo
          // ...
          // 33 00000000 UNDEF      1    NULL NULL WEAK_EXTERNAL    bar
          // Tag Index 35, Characteristics SEARCH_ALIAS
          // 35 00000000 0000000001 0    NULL NULL EXTERNAL         .weak.bar.default.foo
          //
          // In this case linker needs to parse .weak.bar.default.foo and search for bar and foo as well.
          Assert("TODO: MinGW weak symbol");
        } else {
          lazy = lnk_symbol_table_search_node(task->symtab, LNK_SymbolScopeFlag_Lib, weak->fallback_symbol->name);
        }
      }
    } break;
    }

    if (lazy) {
      lnk_push_input_from_lazy(arena, task->path_style, &lazy->data->u.lazy, &result->input_import_list, &result->input_obj_list);
    } else {
      lnk_symbol_list_push_node(&result->unresolved_symbol_list, symbol_node);
    }
  }
}

internal LNK_SymbolFinderResult
lnk_run_symbol_finder(TP_Context      *tp,
                      TP_Arena        *arena,
                      PathStyle        path_style,
                      LNK_SymbolTable *symtab,
                      LNK_SymbolList   lookup_list,
                      TP_TaskFunc     *task_func)
{
  ProfBeginFunction();
  Temp scratch = scratch_begin(arena->v, arena->count);

  ProfBegin("Setup Task");
  LNK_SymbolFinder task  = {0};
  task.path_style        = path_style;
  task.symtab            = symtab;
  task.lookup_node_arr   = lnk_symbol_node_array_from_list(scratch.arena, lookup_list);
  task.result_arr        = push_array(scratch.arena, LNK_SymbolFinderResult, tp->worker_count);
  task.range_arr         = tp_divide_work(scratch.arena, task.lookup_node_arr.count, tp->worker_count);
  ProfEnd();

  ProfBegin("Run Task");
  tp_for_parallel(tp, arena, tp->worker_count, task_func, &task);
  ProfEnd();

  ProfBegin("Concat Results");
  LNK_SymbolFinderResult result = {0};
  for (U64 i = 0; i < tp->worker_count; ++i) {
    LNK_SymbolFinderResult *src = &task.result_arr[i];
    lnk_symbol_list_concat_in_place(&result.unresolved_symbol_list, &src->unresolved_symbol_list);
    lnk_input_obj_list_concat_in_place(&result.input_obj_list, &src->input_obj_list);
    lnk_input_import_list_concat_in_place(&result.input_import_list, &src->input_import_list);
  }
  ProfEnd();

  // to get deterministic output accross multiple linker runs we have to sort inputs
  ProfBegin("Sort Objs [Count %llu]", result.input_obj_list.count);
  LNK_InputObj **input_obj_ptr_arr = lnk_array_from_input_obj_list(scratch.arena, result.input_obj_list);
  qsort(input_obj_ptr_arr, result.input_obj_list.count, sizeof(input_obj_ptr_arr[0]), lnk_input_obj_compar);
  //radsort(input_obj_ptr_arr, result.input_obj_list.count, lnk_input_obj_compar_is_before);
  result.input_obj_list = lnk_list_from_input_obj_arr(input_obj_ptr_arr, result.input_obj_list.count);
  ProfEnd();

  ProfBegin("Sort Imports [Count %llu]", result.input_import_list.count);
  LNK_InputImport **input_imp_ptr_arr = lnk_input_import_arr_from_list(scratch.arena, result.input_import_list);
  //radsort(input_imp_ptr_arr, result.input_import_list.count, lnk_input_import_is_before);
  qsort(input_imp_ptr_arr, result.input_import_list.count, sizeof(input_obj_ptr_arr[0]), lnk_input_import_compar);
  result.input_import_list = lnk_list_from_input_import_arr(input_imp_ptr_arr, result.input_import_list.count);
  ProfEnd();
  
  scratch_end(scratch);
  ProfEnd();
  return result;
}

internal
THREAD_POOL_TASK_FUNC(lnk_defined_symbol_inserter)
{
  LNK_DefinedSymbolInserter *task   = raw_task;
  LNK_SymbolTable           *symtab = task->symtab;
  Rng1U64                    range  = task->range_arr[task_id];
  for (U64 bucket_idx = range.min; bucket_idx < range.max; bucket_idx += 1) {
    LNK_SymbolList *bucket = &task->bucket_arr[bucket_idx];
    for (LNK_SymbolNode *curr = bucket->first, *next; curr != 0; curr = next) {
      next = curr->next;
      LNK_SymbolNode *extant_node = lnk_symbol_table_search_bucket(symtab, LNK_SymbolScopeIndex_Defined, bucket_idx, curr->data->name, curr->hash);
      if (extant_node) {
        LNK_SymbolList *symtab_bucket = lnk_symbol_table_bucket_from_hash(symtab, LNK_SymbolScopeIndex_Defined, curr->hash);
        lnk_symbol_list_insert_after(symtab_bucket, extant_node, curr);
      } else {
        lnk_symbol_table_push_(symtab, LNK_SymbolScopeIndex_Defined, curr, curr->hash);
      }
    }
  }
}

////////////////////////////////

internal void
lnk_apply_reloc(U64               base_addr,
                U64               virt_align,
                U64               file_align,
                LNK_Section     **sect_id_map,
                LNK_SymbolTable  *symtab,
                String8           chunk_data,
                LNK_Reloc        *reloc)
{
  LNK_Symbol *symbol = lnk_resolve_symbol(symtab, reloc->symbol);

  // TODO: check if user forced to link with unresolved symbols and accordingly report the error
  if (!LNK_Symbol_IsDefined(symbol->type)) {
    lnk_error(LNK_Error_UndefinedSymbol, "%S", symbol->name);
    return;
  }
  
  U64 symbol_vsize = 0;
  U64 symbol_fsize = 0;
  U64 symbol_isect = 0;
  U64 symbol_off   = 0;
  U64 symbol_voff  = 0;
  U64 symbol_foff  = 0;
  
  LNK_DefinedSymbol *defined_symbol = &symbol->u.defined;
  switch (defined_symbol->value_type) {
  case LNK_DefinedSymbolValue_Null: break;
  case LNK_DefinedSymbolValue_Chunk: {
    symbol_isect = lnk_isect_from_symbol(sect_id_map, symbol);
    symbol_vsize = lnk_virt_size_from_symbol(sect_id_map, symbol);
    symbol_fsize = lnk_file_size_from_symbol(sect_id_map, symbol);
    symbol_off   = lnk_sect_off_from_symbol(sect_id_map, symbol);
    symbol_voff  = lnk_virt_off_from_symbol(sect_id_map, symbol);
    symbol_foff  = lnk_file_off_from_symbol(sect_id_map, symbol);
  } break;
  case LNK_DefinedSymbolValue_VA: {
    symbol_voff = defined_symbol->u.va - base_addr;
  } break;
  }

#if LNK_DEBUG
  if (str8_match(str8_lit("__ImageBase"), symbol->name, 0)) {
    Assert(symbol_isect == 0);
    Assert(symbol_voff == 0);
    Assert(symbol_foff == 0);
    Assert(symbol_vsize == 0);
    Assert(symbol_fsize == 0);
  }
#endif
  
  U64 reloc_align = 1;
  U64 reloc_size  = 0;
  S64 reloc_value = 0;
  
  switch (reloc->type) {
  case LNK_Reloc_NULL: /* ignore */ break;
  case LNK_Reloc_ADDR_16: {
    reloc_value = safe_cast_u16(base_addr + symbol_voff);
    reloc_size  = 2;
  } break;
  case LNK_Reloc_ADDR_32: {
    reloc_value = safe_cast_u32(base_addr + symbol_voff);
    reloc_size  = 4;
  } break;
  case LNK_Reloc_ADDR_64: {
    reloc_value = base_addr + symbol_voff;
    reloc_size  = 8;
  } break;
  case LNK_Reloc_CHUNK_SIZE_FILE_16: {
    reloc_value = safe_cast_u16(symbol_fsize);
    reloc_size  = 2;
  } break;
  case LNK_Reloc_CHUNK_SIZE_FILE_32: {
    reloc_value = symbol_fsize;
    reloc_size  = 4;
  } break;
  case LNK_Reloc_CHUNK_SIZE_VIRT_32: {
    reloc_value = symbol_vsize;
    reloc_size  = 4;
  } break;
  case LNK_Reloc_FILE_ALIGN_32: {
    reloc_value = 0;
    reloc_size  = 4;
    reloc_align = file_align;
  } break;
  case LNK_Reloc_FILE_OFF_32: {
    reloc_value = safe_cast_u32(symbol_foff);
    reloc_size  = 4;
  } break;
  case LNK_Reloc_FILE_OFF_64: {
    reloc_value = symbol_foff;
    reloc_size  = 8;
  } break;
  case LNK_Reloc_REL32: {
    U64 reloc_voff = lnk_virt_off_from_reloc(sect_id_map, reloc);
    reloc_value    = safe_cast_s32((S64)(symbol_voff - reloc_voff) - (4 + 0));
    reloc_size     = 4;
  } break;
  case LNK_Reloc_REL32_1: {
    U64 reloc_voff = lnk_virt_off_from_reloc(sect_id_map, reloc);
    reloc_value    = safe_cast_s32((S64)(symbol_voff - reloc_voff) - (4 + 1));
    reloc_size     = 4;
  } break;
  case LNK_Reloc_REL32_2: {
    U64 reloc_voff = lnk_virt_off_from_reloc(sect_id_map, reloc);
    reloc_value    = safe_cast_s32((S64)(symbol_voff - reloc_voff) - (4 + 2));
    reloc_size     = 4;
  } break;
  case LNK_Reloc_REL32_3: {
    U64 reloc_voff = lnk_virt_off_from_reloc(sect_id_map, reloc);
    reloc_value    = safe_cast_s32((S64)(symbol_voff - reloc_voff) - (4 + 3));
    reloc_size     = 4;
  } break;
  case LNK_Reloc_REL32_4: {
    U64 reloc_voff = lnk_virt_off_from_reloc(sect_id_map, reloc);
    reloc_value    = safe_cast_s32((S64)(symbol_voff - reloc_voff) - (4 + 4));
    reloc_size     = 4;
  } break;
  case LNK_Reloc_REL32_5: {
    U64 reloc_voff = lnk_virt_off_from_reloc(sect_id_map, reloc);
    reloc_value    = safe_cast_s32((S64)(symbol_voff - reloc_voff) - (4 + 5));
    reloc_size     = 4;
  } break;
  case LNK_Reloc_SECT_REL: {
    reloc_value = safe_cast_u32(symbol_off);
    reloc_size  = 4;
  } break;
  case LNK_Reloc_SECT_IDX: {
    reloc_value = safe_cast_u32(symbol_isect);
    reloc_size  = 4;
  } break;
  case LNK_Reloc_VIRT_ALIGN_32: {
    reloc_value = 0;
    reloc_size  = 4;
    reloc_align = virt_align;
  } break;
  case LNK_Reloc_VIRT_OFF_32: {
    reloc_value = safe_cast_u32(symbol_voff);
    reloc_size  = 4;
  } break;
  default: NotImplemented;
  }
  
  // read addend
  Assert(reloc->apply_off + reloc_size <= chunk_data.size);
  U64 raw_addend = 0;
  MemoryCopy(&raw_addend, chunk_data.str + reloc->apply_off, reloc_size);
  S64 addend = extend_sign64(raw_addend, reloc_size);
  
  // commit reloc value
  reloc_value += addend;
  reloc_value = AlignPow2(reloc_value, reloc_align);
  MemoryCopy(chunk_data.str + reloc->apply_off, &reloc_value, reloc_size);
}

internal
THREAD_POOL_TASK_FUNC(lnk_section_reloc_patcher)
{
  LNK_SectionRelocPatcher *task = raw_task;

  LNK_SymbolTable  *symtab      = task->symtab;
  LNK_SectionTable *st          = task->st;
  LNK_Section     **sect_id_map = task->sect_id_map;
  U64               base_addr   = task->base_addr;
  Rng1U64           range       = task->range_arr[task_id];

  for (U64 sect_idx = range.min; sect_idx < range.max; sect_idx += 1) {
    LNK_Section *sect = task->sect_arr[sect_idx];

    if (sect->has_layout) {
      for (LNK_Reloc *reloc = sect->reloc_list.first; reloc != 0; reloc = reloc->next) {
        LNK_Chunk *chunk = reloc->chunk;
        if (lnk_chunk_is_discarded(chunk)) {
          continue;
        }
        String8 chunk_data = lnk_data_from_chunk_ref(sect_id_map, chunk->ref);
        lnk_apply_reloc(base_addr, st->sect_align, st->file_align, sect_id_map, symtab, chunk_data, reloc);
        int bad_vs = 0; (void)bad_vs;
      }
    } else {
      for (LNK_Reloc *reloc = sect->reloc_list.first; reloc != 0; reloc = reloc->next) {
        LNK_Chunk *chunk = reloc->chunk;
        if (lnk_chunk_is_discarded(chunk)) {
          continue;
        }
        if (chunk->type != LNK_Chunk_Leaf) {
          continue;
        }
        lnk_apply_reloc(base_addr, st->sect_align, st->file_align, sect_id_map, symtab, chunk->u.leaf, reloc);
        int bad_vs = 0; (void)bad_vs;
      }
    }
  }
}

internal void
lnk_patch_relocs(TP_Context *tp, LNK_SymbolTable *symtab, LNK_SectionTable *st, U64 base_addr)
{
  ProfBeginFunction();
  Temp scratch = scratch_begin(0,0);

  LNK_SectionPtrArray sect_arr = lnk_section_ptr_array_from_list(scratch.arena, st->list);

  LNK_SectionRelocPatcher task;
  task.symtab      = symtab;
  task.st          = st;
  task.sect_id_map = lnk_sect_id_map_from_section_table(scratch.arena, st);
  task.sect_arr    = sect_arr.v;
  task.base_addr   = base_addr;
  task.range_arr   = tp_divide_work(scratch.arena, sect_arr.count, tp->worker_count);
  tp_for_parallel(tp, 0, tp->worker_count, lnk_section_reloc_patcher, &task);

  scratch_end(scratch);
  ProfEnd();
}

internal
THREAD_POOL_TASK_FUNC(lnk_obj_reloc_patcher)
{
  LNK_ObjRelocPatcher *task = raw_task;
  LNK_Obj             *obj  = task->obj_arr[task_id];

  for (U64 sect_idx = 0; sect_idx < obj->sect_count; sect_idx += 1) {
    LNK_RelocList reloc_list = obj->sect_reloc_list_arr[sect_idx];
    for (LNK_Reloc *reloc = reloc_list.first; reloc != 0; reloc = reloc->next) {
      if (lnk_chunk_is_discarded(reloc->chunk)) {
        continue;
      }
      Assert(reloc->chunk->type == LNK_Chunk_Leaf);

      String8 chunk_data;
      {
        LNK_Section *sect = lnk_sect_from_chunk_ref(task->sect_id_map, reloc->chunk->ref);
        if (sect->has_layout) {
          chunk_data = lnk_data_from_chunk_ref(task->sect_id_map, reloc->chunk->ref);
        } else {
          chunk_data = reloc->chunk->u.leaf;
        }
      }

      lnk_apply_reloc(task->base_addr, task->st->sect_align, task->st->file_align, task->sect_id_map, task->symtab, chunk_data, reloc);
      int bad_vs = 0; (void)bad_vs;
    }
  }
}

internal void
lnk_patch_relocs_obj(TP_Context *tp, LNK_ObjList obj_list, LNK_SymbolTable *symtab, LNK_SectionTable *st, U64 base_addr)
{
  ProfBeginFunction();
  Temp scratch = scratch_begin(0,0);

  LNK_ObjRelocPatcher task;
  task.symtab      = symtab;
  task.st          = st;
  task.sect_id_map = lnk_sect_id_map_from_section_table(scratch.arena, st);
  task.base_addr   = base_addr;
  task.obj_arr     = lnk_obj_arr_from_list(scratch.arena, obj_list);
  tp_for_parallel(tp, 0, obj_list.count, lnk_obj_reloc_patcher, &task);

  scratch_end(scratch);
  ProfEnd();
}

////////////////////////////////

internal LNK_SectionTable *
lnk_init_section_table(LNK_SymbolTable *symtab, U64 section_virt_off, U64 sect_align, U64 file_align)
{
  ProfBeginFunction();

  static struct {
    char *name;
    char *symbol;
    int flags;
  } sect_layout[] = {
    { ".null",    LNK_NULL_SYMBOL_NAME,  0                       },
    { ".text",    LNK_TEXT_SYMBOL_NAME,  LNK_TEXT_SECTION_FLAGS  },
    { ".data",    LNK_DATA_SYMBOL_NAME,  LNK_DATA_SECTION_FLAGS  },
    { ".rdata",   LNK_RDATA_SYMBOL_NAME, LNK_RDATA_SECTION_FLAGS },
    { ".bss",     LNK_BSS_SYMBOL_NAME,   LNK_BSS_SECTION_FLAGS   },
    { ".xdata",   LNK_XDATA_SYMBOL_NAME, LNK_XDATA_SECTION_FLAGS },
    { ".pdata",   LNK_PDATA_SYMBOL_NAME, LNK_PDATA_SECTION_FLAGS },
    { ".edata",   LNK_EDATA_SYMBOL_NAME, LNK_EDATA_SECTION_FLAGS }, 
    { ".rsrc",    LNK_RSRC_SYMBOL_NAME,  LNK_RSRC_SECTION_FLAGS  },
    { ".debug",   LNK_DEBUG_SYMBOL_NAME, LNK_DEBUG_SECTION_FLAGS },
  };
  
  LNK_SectionTable *st = lnk_section_table_alloc(section_virt_off, sect_align, file_align);
  for (U64 i = 0; i < ArrayCount(sect_layout); ++i) {
    LNK_Section *sect = lnk_section_table_push(st, str8_cstring(sect_layout[i].name), sect_layout[i].flags);
    sect->symbol_name = str8_cstring(sect_layout[i].symbol);
    sect->symbol_name = push_str8_copy(sect->arena, sect->symbol_name);
    
    LNK_Symbol *symbol = lnk_make_defined_symbol_chunk(symtab->arena, sect->symbol_name, LNK_DefinedSymbolVisibility_Internal, 0, sect->root, 0, 0, 0);
    lnk_symbol_table_push(symtab, symbol);
  }
  
  st->null_sect = lnk_section_list_remove(&st->list, str8_lit(".null"));

  // dont build layout because we discard debug from image and move it to pdb
  LNK_Section *debug_sect = lnk_section_table_search(st, str8_lit(".debug"));
  debug_sect->emit_header = 0;
  debug_sect->has_layout  = 0;
  
  ProfEnd();
  return st;
}

internal LNK_MergeDirectiveList
lnk_init_merge_directive_list(Arena *arena, LNK_ObjList obj_list)
{
  ProfBeginFunction();

  LNK_MergeDirectiveList result = {0};
  
  lnk_merge_directive_list_push(arena, &result, (LNK_MergeDirective){ str8_lit_comp(".xdata") , str8_lit_comp(".rdata") });
  
  // collect merge directives from objs
  for (LNK_ObjNode *obj_node = obj_list.first; obj_node != 0; obj_node = obj_node->next) {
    LNK_Obj *obj = &obj_node->data;
    for (LNK_Directive *dir = obj->directive_info.v[LNK_Directive_Merge].first; dir != 0; dir = dir->next) {
      for (String8Node *value_node = dir->value_list.first; value_node != 0; value_node = value_node->next) {
        LNK_MergeDirective merge_dir;
        if (lnk_parse_merge_directive(value_node->string, &merge_dir)) { lnk_merge_directive_list_push(arena, &result, merge_dir);
        } else {
          lnk_error_obj(LNK_Warning_IllData, obj, "can't parse merge directive \"%S\"", value_node->string);
        }
      }
    }
  }
  
  ProfEnd();
  return result;
}

internal void
lnk_discard_meta_data_sections(LNK_SectionTable *st)
{
  static char * meta_data_sect_arr[] = {
      ".gfids",
      ".giats",
      ".gljmp",
      ".gehcont",
  };
  for (U64 meta_idx = 0; meta_idx < ArrayCount(meta_data_sect_arr); meta_idx += 1) {
    String8 name = str8_cstring(meta_data_sect_arr[meta_idx]);
    LNK_Section *sect = lnk_section_table_search(st, name);
    if (sect) {
      lnk_visit_chunks(sect->id, sect->root, lnk_chunk_mark_discarded, NULL);
      sect->root->is_discarded = 0;
    }
  }
}

////////////////////////////////

internal int
lnk_pdata_is_before_x8664(void *raw_a, void *raw_b)
{
  PE_IntelPdata *a = raw_a;
  PE_IntelPdata *b = raw_b;
  int is_before = a->voff_first < b->voff_first;
  return is_before;
}

////////////////////////////////

internal void
lnk_log_size_breakdown(LNK_SectionTable *st, LNK_SymbolTable *symtab)
{
  Temp scratch = scratch_begin(0, 0);
  
  LNK_Section **sect_id_map = lnk_sect_id_map_from_section_table(scratch.arena, st);
  
  U64 code_size = 0;
  U64 data_size = 0;
  for (LNK_SectionNode *sect_node = st->list.first; sect_node != NULL; sect_node = sect_node->next) {
    LNK_Section *sect = &sect_node->data;
    if (sect->has_layout) {
      U64 sect_size = lnk_file_size_from_chunk_ref(sect_id_map, sect->root->ref);
      if (sect->flags & COFF_SectionFlag_CNT_CODE) {
        code_size += sect_size;
      } else if (sect->flags & COFF_SectionFlag_CNT_INITIALIZED_DATA) {
        data_size += sect_size;
      }
    }
  }
  
  LNK_Symbol *dos_header_symbol          = lnk_symbol_table_search(symtab, LNK_SymbolScopeFlag_Internal, str8_lit(LNK_DOS_HEADER_SYMBOL_NAME));
  LNK_Symbol *dos_program_symbol         = lnk_symbol_table_search(symtab, LNK_SymbolScopeFlag_Internal, str8_lit(LNK_DOS_PROGRAM_SYMBOL_NAME));
  LNK_Symbol *coff_header_symbol         = lnk_symbol_table_search(symtab, LNK_SymbolScopeFlag_Internal, str8_lit(LNK_COFF_HEADER_SYMBOL_NAME));
  LNK_Symbol *coff_section_header_symbol = lnk_symbol_table_search(symtab, LNK_SymbolScopeFlag_Internal, str8_lit(LNK_COFF_SECT_HEADER_ARRAY_SYMBOL_NAME));
  LNK_Symbol *pe_opt_header_symbol       = lnk_symbol_table_search(symtab, LNK_SymbolScopeFlag_Internal, str8_lit(LNK_PE_OPT_HEADER_SYMBOL_NAME));
  LNK_Symbol *pe_directories_symbol      = lnk_symbol_table_search(symtab, LNK_SymbolScopeFlag_Internal, str8_lit(LNK_PE_DIRECTORY_ARRAY_SYMBOL_NAME));
  
  LNK_Chunk *dos_header_chunk          = dos_header_symbol->u.defined.u.chunk;
  LNK_Chunk *dos_program_chunk         = dos_program_symbol->u.defined.u.chunk;
  LNK_Chunk *coff_header_chunk         = coff_header_symbol->u.defined.u.chunk;
  LNK_Chunk *coff_section_header_chunk = coff_section_header_symbol->u.defined.u.chunk;
  LNK_Chunk *pe_opt_header_chunk       = pe_opt_header_symbol->u.defined.u.chunk;
  LNK_Chunk *pe_directories_chunk      = pe_directories_symbol->u.defined.u.chunk;
  
  U64 dos_header_size          = lnk_file_size_from_chunk_ref(sect_id_map, dos_header_chunk->ref);
  U64 dos_program_size         = lnk_file_size_from_chunk_ref(sect_id_map, dos_program_chunk->ref);
  U64 coff_header_size         = lnk_file_size_from_chunk_ref(sect_id_map, coff_header_chunk->ref);
  U64 coff_section_header_size = lnk_file_size_from_chunk_ref(sect_id_map, coff_section_header_chunk->ref);
  U64 pe_opt_header_size       = lnk_file_size_from_chunk_ref(sect_id_map, pe_opt_header_chunk->ref);
  U64 pe_directories_size      = lnk_file_size_from_chunk_ref(sect_id_map, pe_directories_chunk->ref);
  
  String8 code_size_str = str8_from_memory_size2(scratch.arena, code_size);
  String8 data_size_str = str8_from_memory_size2(scratch.arena, data_size);
  
  String8List output_list; MemoryZeroStruct(&output_list);
  str8_list_pushf(scratch.arena, &output_list, "--- Image Size Breakdown -------------------------------------------------------");
  str8_list_pushf(scratch.arena, &output_list, "  DOS Header:           %u", dos_header_size);
  str8_list_pushf(scratch.arena, &output_list, "  DOS Program Stub:     %u", dos_program_size);
  str8_list_pushf(scratch.arena, &output_list, "  COFF Header:          %u", coff_header_size);
  str8_list_pushf(scratch.arena, &output_list, "  COFF Section Headers: %u", coff_section_header_size);
  str8_list_pushf(scratch.arena, &output_list, "  PE Header:            %u", pe_opt_header_size);
  str8_list_pushf(scratch.arena, &output_list, "  Directories:          %u", pe_directories_size);
  str8_list_pushf(scratch.arena, &output_list, "  Code Size:            %S", code_size_str);
  str8_list_pushf(scratch.arena, &output_list, "  Data Size:            %S", data_size_str);
  
  StringJoin new_line_join = { str8_lit_comp(""), str8_lit_comp("\n"), str8_lit_comp("") };
  String8 output = str8_list_join(scratch.arena, &output_list, &new_line_join);
  lnk_log(LNK_Log_SizeBreakdown, "%S\n", output);
  
  scratch_end(scratch);
}

internal void
lnk_log_link_stats(LNK_ObjList obj_list, LNK_LibList *lib_index, LNK_SectionTable *st)
{
  Temp scratch = scratch_begin(0, 0);
  
  U32 lib_count = 0;
  for (U32 i = 0; i < LNK_InputSource_Count; i += 1) {
    lib_count += lib_index[i].count;
  }
  U32 reloc_count = 0;
  for (LNK_SectionNode *sect_node = st->list.first; sect_node != NULL; sect_node = sect_node->next) {
    reloc_count += sect_node->data.reloc_list.count;
  }
  
  String8List output_list = {0};
  str8_list_pushf(scratch.arena, &output_list, "------ Link Stats --------------------------------------------------------------");
  str8_list_pushf(scratch.arena, &output_list, "  Linked Objs:    %u", obj_list.count);
  str8_list_pushf(scratch.arena, &output_list, "  Linked Libs:    %u", lib_count);
  str8_list_pushf(scratch.arena, &output_list, "  Relocs Patched: %u", reloc_count);
  
  StringJoin new_line_join = { str8_lit_comp(""), str8_lit_comp("\n"), str8_lit_comp("") };
  String8 output = str8_list_join(scratch.arena, &output_list, &new_line_join);
  lnk_log(LNK_Log_LinkStats, "%S\n", output);
  
  scratch_end(scratch);
}

internal void
lnk_log_timers(void)
{
  Temp scratch = scratch_begin(0, 0);
  
  U64 total_build_time_micro = 0;
  for (U64 i = 0; i < LNK_Timer_Count; ++i) {
    total_build_time_micro += g_timers[i].end - g_timers[i].begin;
  }
  
  String8List output_list = {0};
  str8_list_pushf(scratch.arena, &output_list, "------ Link Times --------------------------------------------------------------");
  for (U64 i = 0; i < LNK_Timer_Count; ++i) {
    U64 build_time_micro = g_timers[i].end - g_timers[i].begin;
    if (build_time_micro != 0) {
      String8  timer_name = lnk_string_from_timer_type(i);
      DateTime time       = date_time_from_micro_seconds(build_time_micro);
      String8  time_str   = string_from_elapsed_time(scratch.arena, time);
      str8_list_pushf(scratch.arena, &output_list, "  %-5S Time: %S", timer_name, time_str);
    }
  }
  
  DateTime total_time = date_time_from_micro_seconds(total_build_time_micro);
  String8 total_time_str = string_from_elapsed_time(scratch.arena, total_time);
  str8_list_pushf(scratch.arena, &output_list, "  Total Time: %S", total_time_str);
  
  StringJoin new_line_join = { str8_lit_comp(""), str8_lit_comp("\n"), str8_lit_comp("") };
  String8 output = str8_list_join(scratch.arena, &output_list, &new_line_join);
  lnk_log(LNK_Log_Timers, "%S\n", output);
  
  scratch_end(scratch);
}

internal void
lnk_write_thread(void *raw_ctx)
{
  ProfBeginFunction();
  LNK_WriteThreadContext *ctx = raw_ctx;
  lnk_write_data_to_file_path(ctx->path, ctx->data);
  ProfEnd();
}

internal
THREAD_POOL_TASK_FUNC(lnk_blake3_hasher_task)
{
  ProfBeginFunction();

  LNK_Blake3Hasher *task     = raw_task;
  Rng1U64           range    = task->ranges[task_id];
  String8           sub_data = str8_substr(task->data, range);

  blake3_hasher hasher; blake3_hasher_init(&hasher);
  blake3_hasher_update(&hasher, sub_data.str, sub_data.size);
  blake3_hasher_finalize(&hasher, (U8 *)task->hashes[task_id].u64, sizeof(task->hashes[task_id].u64));

  ProfEnd();
}

internal U128
lnk_blake3_hash_parallel(TP_Context *tp, U64 chunk_count, String8 data)
{
  ProfBeginFunction();
  Temp scratch = scratch_begin(0, 0);

  ProfBegin("Hash Chunks");
  LNK_Blake3Hasher task = {0};
  task.data             = data;
  task.ranges           = tp_divide_work(scratch.arena, data.size, chunk_count);
  task.hashes           = push_array(scratch.arena, U128, chunk_count);
  tp_for_parallel(tp, 0, chunk_count, lnk_blake3_hasher_task, &task);
  ProfEnd();

  ProfBegin("Combine Hashes");
  blake3_hasher hasher; blake3_hasher_init(&hasher);
  for (U64 i = 0; i < chunk_count; ++i) {
    blake3_hasher_update(&hasher, (U8 *)task.hashes[i].u64, sizeof(task.hashes[i].u64));
  }
  U128 result;
  blake3_hasher_finalize(&hasher, (U8 *)result.u64, sizeof(result.u64));
  ProfEnd();

  scratch_end(scratch);
  ProfEnd();
  return result;
}

internal void
lnk_run(int argc, char **argv)
{
  enum State {
    State_Null,
    State_InputSymbols,
    State_InputImports,
    State_InputDisallowLibs,
    State_InputLibs,
    State_InputObjs,
    State_LookupUndef,
    State_LookupWeak,
    State_BuildAndInputLinkerObj,
    State_BuildAndInputResObj,
    State_PushDllHelperUndefSymbol,
    State_PushLinkerSymbols,
    State_PushLoadConfigUndefSymbol,
    State_SearchEntryPoint,
    State_CheckUnusedDelayLoads,
    State_ReportUnresolvedSymbols,
    State_RewireComdats,
    State_DiscardMetaDataSections,
    State_BuildDebugDirectory,
    State_BuildExportTable,
    State_MergeSections,
    State_BuildCFGuards,
    State_BuildBaseRelocs,
    State_BuildWin32Header,
    State_PatchRelocs,
    State_SortExceptionInfo,
    State_BuildImpLib,
    State_BuildDebugInfo,
    State_WriteImage,
  };
  struct StateNode {
    struct StateNode *next;
    enum   State      state;
  };
  struct StateList {
    U64               count;
    struct StateNode *first;
    struct StateNode *last;
  };

#define state_list_push(a, l, s) do {                            \
    struct StateNode *node = push_array(a, struct StateNode, 1); \
    node->state = s;                                             \
    SLLQueuePush(l.first, l.last, node);                         \
    l.count += 1;                                                \
    } while (0)
#define state_list_pop(l) (l).first->state; SLLQueuePop((l).first, (l).last); (l).count -= 1

  ProfBeginFunction();

  Temp scratch = scratch_begin(0, 0);

  LNK_Config *config = lnk_build_config(scratch.arena, argc, argv);

  TP_Context *tp       = tp_alloc(scratch.arena, config->worker_count);
  TP_Arena   *tp_arena = tp_arena_alloc(tp);

  #if PROFILE_TELEMETRY
  {
    String8 cmdl = str8_list_join(scratch.arena, &config->raw_cmd_line, &(StringJoin){ .sep = str8_lit_comp(" ") });
    tmMessage(0, TMMF_ICON_NOTE, "Command Line: %.*s", str8_varg(cmdl));
  }
  #endif

  // inputs
  String8List         include_symbol_list               = config->include_symbol_list;
  String8List         input_disallow_lib_list           = config->disallow_lib_list;
  LNK_AltNameList     alt_name_list                     = config->alt_name_list;
  LNK_InputLibList    input_libs[LNK_InputSource_Count] = {0};
  LNK_InputObjList    input_obj_list                    = {0};
  LNK_InputImportList input_import_list                 = {0};
  LNK_SymbolList      input_defn_list                   = {0};
  LNK_SymbolList      input_weak_list                   = {0};

  // :null_obj
  lnk_input_obj_list_push(scratch.arena, &input_obj_list);

  // input command line objs
  LNK_InputObjList cmd_line_obj_inputs = lnk_input_obj_list_from_string_list(scratch.arena, config->input_list[LNK_Input_Obj]);
  lnk_input_obj_list_concat_in_place(&input_obj_list, &cmd_line_obj_inputs);

  // input command line libs
  input_libs[LNK_InputSource_CmdLine] = config->input_list[LNK_Input_Lib];
  input_libs[LNK_InputSource_Default] = config->input_default_lib_list;

  // state
  LNK_SymbolTable     *symtab                           = lnk_symbol_table_alloc_ex(config->symbol_table_cap_defined, config->symbol_table_cap_internal, config->symbol_table_cap_weak, config->symbol_table_cap_lib);
  LNK_SectionTable    *st                               = lnk_init_section_table(symtab, config->section_virt_off, config->sect_align, config->file_align);
  LNK_ImportTable     *imptab_regular                   = 0;
  LNK_ImportTable     *imptab_delayed                   = 0;
  LNK_ExportTable     *exptab                           = lnk_export_table_alloc();
  HashTable           *disallow_lib_ht                  = hash_table_init(scratch.arena, 0x100);
  HashTable           *delay_load_dll_ht                = hash_table_init(scratch.arena, 0x100);
  HashTable           *default_lib_ht                   = hash_table_init(scratch.arena, 0x100);
  HashTable           *loaded_lib_ht                    = hash_table_init(scratch.arena, 0x100);
  HashTable           *missing_lib_ht                   = hash_table_init(scratch.arena, 0x100);
  HashTable           *loaded_obj_ht                    = hash_table_init(scratch.arena, 0x4000);
  LNK_SymbolList       lookup_undef_list                = {0};
  LNK_SymbolList       lookup_weak_list                 = {0};
  LNK_SymbolList       unresolved_undef_list            = {0};
  LNK_SymbolList       unresolved_weak_list             = {0};
  U64                  entry_search_attempts            = 0;
  B32                  build_debug_info                 = lnk_do_debug_info(config);
  B32                  build_linker_obj                 = build_debug_info;
  B32                  build_debug_directory            = build_debug_info;
  B32                  build_res_obj                    = 1;
  B32                  discard_meta_data_sections       = 1;
  B32                  merge_sections                   = !!(config->flags & LNK_ConfigFlag_Merge);
  B32                  build_cf_guards                  = 0; // (config->flags != LNK_Guard_NONE);
  B32                  build_export_table               = 1;
  B32                  build_base_relocs                = !(config->flags & LNK_ConfigFlag_Fixed);
  B32                  report_unresolved_symbols        = 1;
  B32                  check_unused_delay_loads         = !!(config->flags & LNK_ConfigFlag_CheckUnusedDelayLoadDll);
  B32                  do_comdat_rewire                 = 1;
  B32                  build_win32_header               = 1;
  B32                  patch_relocs                     = 1;
  B32                  sort_exception_info              = 1;
  B32                  build_imp_lib                    = 1;
  LNK_ObjList          obj_list                         = {0};
  LNK_LibList          lib_index[LNK_InputSource_Count] = {0};
  String8              image_data                       = str8_zero();
  OS_Handle            image_write_thread               = {0};

  // init state machine
  struct StateList state_list = {0};
  state_list_push(scratch.arena, state_list, State_InputObjs);
  state_list_push(scratch.arena, state_list, State_InputLibs);
  state_list_push(scratch.arena, state_list, State_PushLinkerSymbols);
  if (config->delay_load_dll_list.node_count) {
    for (String8Node *delay_load_dll_node = config->delay_load_dll_list.first;
         delay_load_dll_node != 0;
         delay_load_dll_node = delay_load_dll_node->next) {
      hash_table_push_path_u64(scratch.arena, delay_load_dll_ht, delay_load_dll_node->string, 0);
    }
    state_list_push(scratch.arena, state_list, State_PushDllHelperUndefSymbol);
  }
  if (config->guard_flags != LNK_Guard_None) {
    state_list_push(scratch.arena, state_list, State_PushLoadConfigUndefSymbol);
  }

  ProfBegin("Image"); // :EndImage
  ProfBegin("Build"); // :EndBuild
  lnk_timer_begin(LNK_Timer_Image);

  // run states
  for (;;) {
    while (state_list.count) {
      enum State state = state_list_pop(state_list);
      switch (state) {
      case State_Null: break;
      case State_SearchEntryPoint: {
        ProfBegin("Serach Entry Point");
        LNK_Symbol *entry_point_symbol = 0;

        B32 is_entry_point_unspecified = config->entry_point_name.size == 0;
        if (is_entry_point_unspecified) {
          if (config->subsystem == PE_WindowsSubsystem_UNKNOWN) {
            // we don't have a subsystem and entry point name,
            // so we loop over every subsystem and search potential entry
            // points in the symbol table 
            for (U64 subsys_idx = 0; subsys_idx < PE_WindowsSubsystem_COUNT; subsys_idx += 1) {
              String8Array name_arr  = pe_get_entry_point_names(config->machine, (PE_WindowsSubsystem)subsys_idx, config->file_characteristics);
              for (U64 entry_idx = 0; entry_idx < name_arr.count; entry_idx += 1) {
                entry_point_symbol = lnk_symbol_table_search(symtab, LNK_SymbolScopeFlag_Defined, name_arr.v[entry_idx]);
                if (entry_point_symbol) {
                  config->subsystem = (PE_WindowsSubsystem)subsys_idx;
                  goto dbl_break;
                }
              }
            }

            // search for potential entry points in libs
            if (!entry_point_symbol) {
              for (U64 subsys_idx = 0; subsys_idx < PE_WindowsSubsystem_COUNT; subsys_idx += 1) {
                String8Array name_arr = pe_get_entry_point_names(config->machine, (PE_WindowsSubsystem)subsys_idx, config->file_characteristics);
                for (U64 entry_idx = 0; entry_idx < name_arr.count; entry_idx += 1) {
                  entry_point_symbol = lnk_symbol_table_search(symtab, LNK_SymbolScopeFlag_Lib, name_arr.v[entry_idx]);
                  if (entry_point_symbol) {
                    config->subsystem = (PE_WindowsSubsystem)subsys_idx;
                    goto dbl_break;
                  }
                }
              }
            } 

            dbl_break:;
          } else {
            // we have subsystem but no entry point name, get potential entry point names
            // and see which is in the symbol table
            String8Array name_arr = pe_get_entry_point_names(config->machine, config->subsystem, config->file_characteristics);
            for (U64 entry_idx = 0; entry_idx < name_arr.count; entry_idx += 1) {
              LNK_Symbol *symbol = lnk_symbol_table_search(symtab, LNK_SymbolScopeFlag_Defined, name_arr.v[entry_idx]);
              if (symbol) {
                if (entry_point_symbol) {
                  lnk_error(LNK_Error_EntryPoint,
                            "multiple entry point symbols found: %S(%S) and %S(%S)",
                            entry_point_symbol->name, entry_point_symbol->debug,
                            symbol->name, symbol->debug);
                } else {
                  entry_point_symbol = symbol;
                }
              }
            }

            // search for entry point in libs
            if (!entry_point_symbol) {
              for (U64 entry_idx = 0; entry_idx < name_arr.count; entry_idx += 1) {
                entry_point_symbol = lnk_symbol_table_search(symtab, LNK_SymbolScopeFlag_Lib, name_arr.v[entry_idx]);
                if (entry_point_symbol) {
                  break;
                }
              }
            }
          }

          // redirect user entry to appropriate CRT entry
          if (entry_point_symbol) {
            config->entry_point_name = entry_point_symbol->name;
            if (str8_match(config->entry_point_name, str8_lit("wmain"), 0)) {
              config->entry_point_name = str8_lit("wmainCRTStartup");
            } else if (str8_match(config->entry_point_name, str8_lit("main"), 0)) {
              config->entry_point_name = str8_lit("mainCRTStartup");
            } else if (str8_match(config->entry_point_name, str8_lit("WinMain"), 0)) {
              config->entry_point_name = str8_lit("WinMainCRTStartup");
            } else if (str8_match(config->entry_point_name, str8_lit("wWinMain"), 0)) {
              config->entry_point_name = str8_lit("wWinMainCRTStartup");
            }
          }
        }

        // generate undefined symbol so in case obj is in lib it will be linked
        if (config->entry_point_name.size) {
          str8_list_push(scratch.arena, &include_symbol_list, config->entry_point_name);
        }
        // no entry point, error and exit
        else {
          lnk_error(LNK_Error_EntryPoint, "unable to find entry point symbol");
        }

        // by default terminal server is enabled for windows and console applications
        if (~config->flags & LNK_ConfigFlag_NoTsAware && 
            ~config->file_characteristics & PE_ImageFileCharacteristic_FILE_DLL) {
          if (config->subsystem == PE_WindowsSubsystem_WINDOWS_GUI || config->subsystem == PE_WindowsSubsystem_WINDOWS_CUI) {
            config->dll_characteristics |= PE_DllCharacteristic_TERMINAL_SERVER_AWARE;
          }
        }

        // do we have a subsystem?
        if (config->subsystem == PE_WindowsSubsystem_UNKNOWN) {
          lnk_error(LNK_Error_NoSubsystem, "unknown subsystem, please use /SUBSYSTEM to set subsytem type you need");
        }
      
        if (config->subsystem_ver.major == 0 && config->subsystem_ver.minor == 0) {
          // subsystem version not specified, set default values
          config->subsystem_ver = lnk_get_default_subsystem_version(config->subsystem, config->machine);
        }

        // check subsystem version against allowed min version
        Version min_subsystem_ver = lnk_get_min_subsystem_version(config->subsystem, config->machine);
        int ver_cmp = version_compar(config->subsystem_ver, min_subsystem_ver);
        if (ver_cmp < 0) {
          lnk_error(LNK_Error_Cmdl, "subsystem version %I64u.%I64u can't be lower than %I64u.%I64u", 
                    config->subsystem_ver.major, config->subsystem_ver.minor, min_subsystem_ver.major, min_subsystem_ver.minor);
        }

        ProfEnd();
      } break;
      case State_PushDllHelperUndefSymbol: {
        ProfBegin("Puhs Dll Helper Undef Symbol");

        String8 delay_helper_name = str8_zero();
        switch (config->machine) {
        case COFF_MachineType_X86: delay_helper_name = str8_cstring(LNK_DELAY_LOAD_HELPER2_X86_SYMBOL_NAME); break;
        case COFF_MachineType_X64: delay_helper_name = str8_cstring(LNK_DELAY_LOAD_HELPER2_SYMBOL_NAME);     break;
        default: NotImplemented;
        }

        str8_list_push(scratch.arena, &include_symbol_list, delay_helper_name);
        ProfEnd();
      } break;
      case State_PushLoadConfigUndefSymbol: {
        ProfBegin("Push Load Config Undef Symbol");
        String8 load_config_name = str8_lit(LNK_LOAD_CONFIG_SYMBOL_NAME);
        str8_list_push(scratch.arena, &include_symbol_list, load_config_name);
        ProfEnd();
      } break;
      case State_PushLinkerSymbols: {
        ProfBegin("Push Linker Symbols");
        lnk_push_linker_symbols(symtab, config->machine);
        ProfEnd();
      } break;
      case State_InputSymbols: {
        ProfBegin("Input Symbols");

        ProfBegin("Push /INCLUDE Symbols");
        for (String8Node *include_node = include_symbol_list.first; include_node != 0; include_node = include_node->next) {
          String8     name   = push_str8_copy(symtab->arena, include_node->string);
          LNK_Symbol *symbol = lnk_make_undefined_symbol(symtab->arena, name, LNK_SymbolScopeFlag_Main);
          lnk_symbol_list_push(scratch.arena, &lookup_undef_list, symbol);
        }
        ProfEnd();

        ProfBegin("Push /ALTERNATIVENAME Symbols");
        Assert(alt_name_list.from_list.node_count == alt_name_list.to_list.node_count);
        for (String8Node *from_node = alt_name_list.from_list.first, *to_node = alt_name_list.to_list.first;
             from_node != 0;
             from_node = from_node->next, to_node = to_node->next) {
          String8     to_name   = push_str8_copy(symtab->arena, to_node->string);
          String8     from_name = push_str8_copy(symtab->arena, from_node->string);
          LNK_Symbol *fallback  = lnk_make_undefined_symbol(symtab->arena, to_name, LNK_SymbolScopeFlag_Main);
          LNK_Symbol *weak      = lnk_make_weak_symbol(symtab->arena, from_name, COFF_WeakExtType_SEARCH_ALIAS, fallback);
          lnk_symbol_list_push(scratch.arena, &input_weak_list, weak);
        }
        ProfEnd();

        ProfBegin("Push Defined Symbols");
        {
          Temp temp = temp_begin(scratch.arena);

          ProfBegin("List -> Array");
          LNK_SymbolNodeArray symbol_arr = lnk_symbol_node_array_from_list(temp.arena, input_defn_list);
          ProfEnd();

          ProfBegin("Hash Symbol Names");
          lnk_symbol_node_ptr_array_hash(tp, symbol_arr.v, symbol_arr.count);
          ProfEnd();

          ProfBegin("Populate Buckets");
          LNK_SymbolList *bucket_arr = push_array(temp.arena, LNK_SymbolList, symtab->bucket_count[LNK_SymbolScopeIndex_Defined]);
          for (U64 symbol_idx = 0; symbol_idx < symbol_arr.count; symbol_idx += 1) {
            LNK_SymbolNode *symbol_node = symbol_arr.v[symbol_idx];
            U64             bucket_idx  = symbol_node->hash % symtab->bucket_count[LNK_SymbolScopeIndex_Defined];
            lnk_symbol_list_push_node(&bucket_arr[bucket_idx], symbol_node);
          }
          ProfEnd();

          ProfBegin("Insert Defined Symbols");
          LNK_DefinedSymbolInserter symbol_inserter = {0};
          symbol_inserter.symtab                    = symtab;
          symbol_inserter.bucket_arr                = bucket_arr;
          symbol_inserter.range_arr                 = tp_divide_work(temp.arena, symtab->bucket_count[LNK_SymbolScopeIndex_Defined], tp->worker_count);
          tp_for_parallel(tp, 0, tp->worker_count, lnk_defined_symbol_inserter, &symbol_inserter);
          ProfEnd();

          temp_end(temp);
        }
        ProfEnd();

        ProfBegin("Push Weak Symbols");
        for (LNK_SymbolNode *curr = input_weak_list.first; curr != 0; curr = curr->next) {
          lnk_symbol_table_push(symtab, curr->data);
        }
        ProfEnd();

        LNK_SymbolList new_weak_symbols = lnk_symbol_list_copy(scratch.arena, input_weak_list);

        // we defined new symbols, give unresolved symbols another chance to be resolved
        lnk_symbol_list_concat_in_place(&lookup_undef_list, &unresolved_undef_list);
        lnk_symbol_list_concat_in_place(&lookup_weak_list, &new_weak_symbols);
        lnk_symbol_list_concat_in_place(&lookup_weak_list, &unresolved_weak_list);

        // reset inputs
        MemoryZeroStruct(&include_symbol_list);
        MemoryZeroStruct(&alt_name_list);
        MemoryZeroStruct(&input_defn_list);
        MemoryZeroStruct(&input_weak_list);

        ProfEnd();
      } break;
      case State_InputImports: {
        ProfBegin("Input Imports");
        for (LNK_InputImport *input = input_import_list.first; input != 0; input = input->next) {
          COFF_ImportHeader *import_header = &input->import_header;
          KeyValuePair *is_delayed = hash_table_search_path(delay_load_dll_ht, import_header->dll_name);

          if (is_delayed) {
            if (!imptab_delayed) {
              Assert(config->machine != COFF_MachineType_UNKNOWN);
              B32 is_unloadable = !!(config->flags & LNK_ConfigFlag_DelayUnload);
              B32 is_bindable   = !!(config->flags & LNK_ConfigFlag_DelayBind);
              imptab_delayed = lnk_import_table_alloc_delayed(st, symtab, config->machine, is_unloadable, is_bindable); 
            }
            LNK_ImportDLL *dll = lnk_import_table_search_dll(imptab_delayed, import_header->dll_name);
            if (!dll) {
              dll = lnk_import_table_push_dll_delayed(imptab_delayed, symtab, import_header->dll_name, import_header->machine);
            }
            LNK_ImportFunc *func = lnk_import_table_search_func(dll, import_header->func_name);
            if (!func) {
              func = lnk_import_table_push_func_delayed(imptab_delayed, symtab, dll, import_header);
            }
          } else {
            if (!imptab_regular) {
              Assert(config->machine != COFF_MachineType_UNKNOWN);
              imptab_regular = lnk_import_table_alloc_regular(st, symtab, config->machine);
            }
            LNK_ImportDLL *dll = lnk_import_table_search_dll(imptab_regular, import_header->dll_name);
            if (!dll) {
              dll = lnk_import_table_push_dll_regular(imptab_regular, symtab, import_header->dll_name, import_header->machine);
            }
            LNK_ImportFunc *func = lnk_import_table_search_func(dll, import_header->func_name);
            if (!func) {
              func = lnk_import_table_push_func_regular(imptab_regular, symtab, dll, import_header);
            }
          }
        }

        // reset input
        MemoryZeroStruct(&input_import_list);

        ProfEnd();
      } break;
      case State_InputDisallowLibs: {
        ProfBegin("Input /disallowlib");

        for (String8Node *name_n = input_disallow_lib_list.first; name_n != 0; name_n = name_n->next) {
          if ( ! lnk_is_lib_disallowed(disallow_lib_ht, name_n->string)) {
            lnk_push_disallow_lib(scratch.arena, disallow_lib_ht, name_n->string);
          }
        }

        // reset input
        MemoryZeroStruct(&input_disallow_lib_list);

        ProfEnd();
      } break;
      case State_InputObjs: {
        ProfBegin("Input Objs [Count %llu]", input_obj_list.count);
  
        ProfBegin("Gather Objs");
        LNK_InputObjList unique_obj_input_list = {0};
        for (LNK_InputObj *input = input_obj_list.first, *next; input != 0; input = next) {
          next = input->next;

          B32 was_obj_loaded = hash_table_search_path_u64(loaded_obj_ht, input->dedup_id, 0);
          if (was_obj_loaded) {
            continue;
          }

          String8 full_path          = os_make_full_path(scratch.arena, input->dedup_id);
          B32     was_full_path_used = hash_table_search_path_u64(loaded_obj_ht, full_path, 0);
          if (was_full_path_used) {
            continue;
          }

          hash_table_push_path_u64(scratch.arena, loaded_obj_ht, input->dedup_id, 0);
          if (!str8_match(input->dedup_id, full_path, StringMatchFlag_CaseInsensitive|StringMatchFlag_SlashInsensitive)) {
            hash_table_push_path_u64(scratch.arena, loaded_obj_ht, full_path, 0);
          }

          lnk_input_obj_list_push_node(&unique_obj_input_list, input);
          lnk_log(LNK_Log_InputObj, "Input Obj: %S", full_path);
        }
        ProfEnd();

        ProfBegin("Load Objs From Disk");
        LNK_InputObj **input_obj_arr = lnk_array_from_input_obj_list(scratch.arena, unique_obj_input_list);
        tp_for_parallel(tp, tp_arena, unique_obj_input_list.count, lnk_load_thin_objs_task, input_obj_arr);
        ProfEnd();

        ProfBegin("Disk Read Check");
        for (U64 input_idx = 0; input_idx < unique_obj_input_list.count; ++input_idx) {
          if (input_obj_arr[input_idx]->has_disk_read_failed) {
            lnk_error(LNK_Error_InvalidPath, "unable to find obj \"%S\"", input_obj_arr[input_idx]->path);
          }
        }
        ProfEnd();

        LNK_ObjNodeArray obj_node_arr = lnk_obj_list_push_parallel(tp, tp_arena, &obj_list, st, unique_obj_input_list.count, input_obj_arr);

        ProfBegin("Machine Compat Check");
        for (U64 obj_idx = 0; obj_idx < obj_node_arr.count; ++obj_idx) {
          LNK_Obj *obj = &obj_node_arr.v[obj_idx].data;

          // derive machine from obj
          if (config->machine == COFF_MachineType_UNKNOWN) {
            config->machine = obj->machine;
          }

          // is obj machine compatible? 
          if (obj->machine != COFF_MachineType_UNKNOWN && // obj with unknown machine type is compatible with any other machine type
              config->machine != obj->machine) {
            lnk_error(LNK_Error_IncompatibleObj,
                "conflicting machine types expected %S but got %S in obj %S",
                coff_string_from_machine_type(config->machine),
                coff_string_from_machine_type(obj->machine),
                obj->path);
          }
        }
        ProfEnd();

        ProfBegin("Collect Directives");
        for (U64 i = 0; i < obj_node_arr.count; ++i) {
          LNK_Obj *obj = &obj_node_arr.v[i].data;
          str8_list_concat_in_place(&include_symbol_list, &obj->include_symbol_list);
          lnk_alt_name_list_concat_in_place(&alt_name_list, &obj->alt_name_list);
          for (LNK_Directive *dir = obj->directive_info.v[LNK_Directive_DisallowLib].first; dir != 0; dir = dir->next) {
            str8_list_concat_in_place(&input_disallow_lib_list, &dir->value_list);
          }
        }
        ProfEnd();

        // gather libs for input
        LNK_InputLibList lib_list = lnk_collect_default_lib_obj_arr(tp, tp_arena, obj_node_arr); // TODO: put these on temp arena
        str8_list_concat_in_place(&input_libs[LNK_InputSource_Obj], &lib_list);

        // gather symbols for input
        LNK_SymbolList new_defn_list  = lnk_run_symbol_collector(tp, tp_arena, obj_node_arr, LNK_Symbol_DefinedExtern);
        LNK_SymbolList new_weak_list  = lnk_run_symbol_collector(tp, tp_arena, obj_node_arr, LNK_Symbol_Weak);
        LNK_SymbolList new_undef_list = lnk_run_symbol_collector(tp, tp_arena, obj_node_arr, LNK_Symbol_Undefined); // TODO: allocate these on temp arena

        // schedule symbol input
        lnk_symbol_list_concat_in_place(&input_defn_list,   &new_defn_list);
        lnk_symbol_list_concat_in_place(&input_weak_list,   &new_weak_list);
        lnk_symbol_list_concat_in_place(&lookup_undef_list, &new_undef_list);

        // reset input objs
        MemoryZeroStruct(&input_obj_list);

        if (lnk_get_log_status(LNK_Log_InputObj)) {
          U64 input_size = 0;
          for (U64 i = 0; i < obj_node_arr.count; ++i) {
           input_size += obj_node_arr.v[i].data.data.size;
          }
          String8 input_size_string = str8_from_memory_size2(scratch.arena, input_size);
          lnk_log(LNK_Log_InputObj, "[ Obj Input Size %S ]", input_size_string);
        }

        ProfEnd();
      } break;
      case State_InputLibs: {
        ProfBegin("Input Libs");

        for (U64 input_source = 0; input_source < ArrayCount(input_libs); ++input_source) {
          LNK_InputLibList input_lib_list = input_libs[input_source];

          ProfBegin("Remove Duplicte Input Paths");
          LNK_InputLibList unique_input_lib_list = {0};
          for (LNK_InputLib *input = input_lib_list.first; input != 0; input = input->next) {
            String8 path = input->string;

            if (lnk_is_lib_disallowed(disallow_lib_ht, path)) {
              continue;
            }
            if (lnk_is_lib_loaded(default_lib_ht, loaded_lib_ht, input_source, path)) {
              continue;
            }

            // search disk for library
            String8List match_list    = os_file_search(scratch.arena, config->lib_dir_list, path);
            String8     absolute_path = match_list.node_count ? match_list.first->string : str8(0,0);

            // default to first match
            if (lnk_is_lib_loaded(default_lib_ht, loaded_lib_ht, input_source, absolute_path)) {
              continue;
            }

            // warn about missing lib
            if (match_list.node_count == 0) {
              KeyValuePair *was_reported = hash_table_search_path(missing_lib_ht, path);
              if (!was_reported) {
                hash_table_push_path_u64(scratch.arena, missing_lib_ht, path, 0);
                lnk_error(LNK_Warning_FileNotFound, "unable to find library `%S`", path);
              }
              continue;
            }

            // warn about multiple matches
            if (match_list.node_count > 1) {
              lnk_error(LNK_Warning_MultipleLibMatch, "multiple libs match `%S` (picking first match)", path);
              lnk_supplement_error_list(match_list);
            }

            // save paths for future checks
            lnk_push_loaded_lib(scratch.arena, default_lib_ht, loaded_lib_ht, path);
            lnk_push_loaded_lib(scratch.arena, default_lib_ht, loaded_lib_ht, absolute_path);

            // push library for loading
            str8_list_push(scratch.arena, &unique_input_lib_list, absolute_path);

            lnk_log(LNK_Log_InputLib, "Input Lib: %S", absolute_path);
          }
          ProfEnd();

          LNK_LibNodeArray lib_arr;
          {
            ProfBegin("Disk Read Libs");
            String8Array path_arr  = str8_array_from_list(scratch.arena, &unique_input_lib_list);
            String8Array data_arr  = os_data_from_file_path_parallel(tp, tp_arena->v[0], path_arr);
            ProfEnd();

            ProfBegin("Lib Init");
            lib_arr = lnk_lib_list_push_parallel(tp, tp_arena, &lib_index[input_source], data_arr, path_arr);
            ProfEnd();

            ProfBegin("Count Symbols");
            U64 total_symbol_count = 0;
            for (U64 lib_idx = 0; lib_idx < lib_arr.count; lib_idx += 1) {
              total_symbol_count += lib_arr.v[lib_idx].data.symbol_count;
            }
            ProfEnd();

            ProfBegin("Setup Symbol Array Pointers");
            LNK_Symbol  *symbol_arr     = push_array_no_zero(symtab->arena, LNK_Symbol,   total_symbol_count);
            LNK_Symbol **symbol_arr_arr = push_array_no_zero(scratch.arena, LNK_Symbol *, lib_arr.count);
            for (U64 lib_idx = 0, cursor = 0; lib_idx < lib_arr.count; lib_idx += 1) {
              symbol_arr_arr[lib_idx] = &symbol_arr[cursor];
              cursor += lib_arr.v[lib_idx].data.symbol_count;
            }
            ProfEnd();

            ProfBegin("Lazy Symbol Init");
            LNK_LazyIniter lazy_initer_ctx = {0};
            lazy_initer_ctx.range_arr      = tp_divide_work(scratch.arena, lib_arr.count, tp->worker_count);
            lazy_initer_ctx.lib_arr        = lib_arr.v;
            lazy_initer_ctx.symbol_arr_arr = symbol_arr_arr;
            tp_for_parallel(tp, 0, tp->worker_count, lnk_lazy_initer, &lazy_initer_ctx);
            ProfEnd();

            lnk_symbol_table_push_lazy_arr(tp, symtab, symbol_arr, total_symbol_count);
          }

          if (lnk_get_log_status(LNK_Log_InputLib)) {
            if (lib_arr.count > 0) {
              U64 input_size = 0;
              for (U64 i = 0; i < lib_arr.count; ++i) {
               input_size += lib_arr.v[i].data.data.size;
              }
              String8 input_size_string = str8_from_memory_size2(scratch.arena, input_size);
              lnk_log(LNK_Log_InputObj, "[ Lib Input Size %S ]", input_size_string);
            }
          }
        }

        // reset input libs
        MemoryZeroArray(input_libs);

        ProfEnd();
      } break;
      case State_BuildAndInputResObj: {
        String8List res_data_list = {0};
        String8List res_path_list = {0};

        ProfBegin("Build * Resources *");
        {
          // load .res from disk
          for (String8Node *node = config->input_list[LNK_Input_Res].first; node != 0; node = node->next) {
            String8 res_data = os_data_from_file_path(tp_arena->v[0], node->string);
            if (res_data.size > 0) {
              if (pe_is_res(res_data)) {
                String8 stable_res_path = lnk_make_full_path(tp_arena->v[0], config->work_dir, config->path_style, node->string);
                str8_list_push(scratch.arena, &res_path_list, stable_res_path);
                str8_list_push(tp_arena->v[0], &res_data_list, res_data);
              } else {
                lnk_error(LNK_Error_IllData, "file is not of RES format: %S", node->string);
              }
            } else {
              lnk_error(LNK_Error_FileNotFound, "unable to open res file: %S", node->string);
            }
          }
        }
        ProfEnd();

        // handle manifest
        ProfBegin("Manifest");
        {
          LNK_Obj     **obj_arr      = lnk_obj_arr_from_list(scratch.arena, obj_list);
          String8List   obj_dep_list = lnk_collect_manifest_dependency_list(tp, tp_arena, obj_arr, obj_list.count);
          String8List   cmd_dep_list = str8_list_copy(scratch.arena, &config->manifest_dependency_list);

          String8List dep_list = {0};
          str8_list_concat_in_place(&dep_list, &obj_dep_list);
          str8_list_concat_in_place(&dep_list, &cmd_dep_list);

          B32 create_manifest = config->input_list[LNK_Input_Manifest].node_count > 0 ||
                                dep_list.node_count > 0 ||
                                config->manifest_opt == LNK_ManifestOpt_Embed;
          if (create_manifest) {
            String8List input_manifest_path_list = str8_list_copy(tp_arena->v[0], &config->input_list[LNK_Input_Manifest]);

            // TODO: we write a temp file with manifest attributes collected from obj directives and command line switches
            // so we can pass file to mt.exe or llvm-mt.exe, when we have our own tool for merging manifest we can switch
            // to writing manifest file in memory to skip roun-trip to disk
            String8List linker_manifest_data_list = lnk_make_linker_manifest(tp_arena->v[0], config->manifest_uac, config->manifest_level, config->manifest_ui_access, dep_list);
            String8 linker_manifest_path = push_str8f(scratch.arena, "%S.manifest.temp", config->manifest_name);
            lnk_write_data_list_to_file_path(linker_manifest_path, linker_manifest_data_list);
            str8_list_push(tp_arena->v[0], &input_manifest_path_list, linker_manifest_path);

            String8 manifest_path = lnk_merge_manifest_files(tp_arena->v[0], config->mt_path, config->manifest_name, input_manifest_path_list);

            if (config->manifest_opt == LNK_ManifestOpt_Embed) {
              // TODO: currently we convert manifest to res and parse res again, this unnecessary instead push manifest 
              // resource to the tree directly

              String8 manifest_data = os_data_from_file_path(scratch.arena, manifest_path);
              if (manifest_data.size == 0) {
                lnk_error(LNK_Error_Mt, "unable to locate manifest to embed on disk, path \"%S\"", manifest_path);
              }
              String8 manifest_res = lnk_res_from_data(tp_arena->v[0], manifest_data);
              str8_list_push(tp_arena->v[0], &res_data_list, manifest_res);
              str8_list_push(tp_arena->v[0], &res_path_list, manifest_path);
            }

            // cleanup disk
            os_delete_file_at_path(linker_manifest_path);
            if (config->delete_manifest) {
              os_delete_file_at_path(manifest_path);
            }
          }
        }
        ProfEnd(); // Manifest

        if (res_data_list.node_count > 0) {
          String8 obj_name = str8_lit("* Resources *");
          String8 obj_data = lnk_obj_from_res_file_list(tp,
                                                        tp_arena->v[0],
                                                        st,
                                                        symtab,
                                                        res_data_list,
                                                        res_path_list,
                                                        config->machine,
                                                        config->time_stamp,
                                                        config->work_dir,
                                                        config->path_style,
                                                        obj_name);

          LNK_InputObj *input = lnk_input_obj_list_push(scratch.arena, &input_obj_list);
          input->dedup_id = obj_name;
          input->path     = obj_name;
          input->data     = obj_data;
        }
      } break;
      case State_BuildAndInputLinkerObj: {
        ProfBegin("Build * Linker * Obj");

        String8 obj_name = str8_lit("* Linker *");

        StringJoin join         = { str8_lit_comp(""),  str8_lit_comp(" "), str8_lit_comp("") };
        String8    raw_cmd_line = str8_list_join(scratch.arena, &config->raw_cmd_line, &join);

        String8 obj_data = lnk_make_linker_coff_obj(tp, scratch.arena, config->time_stamp, config->machine, config->work_dir, config->image_name, config->pdb_name, raw_cmd_line, obj_name);

        LNK_InputObj *input = lnk_input_obj_list_push(scratch.arena, &input_obj_list);
        input->dedup_id = obj_name;
        input->path     = obj_name;
        input->data     = obj_data;

        ProfEnd();
      } break;
      case State_LookupUndef: {
        ProfBegin("Lookup Undefined Symbols");
        // search archives
        LNK_SymbolFinderResult result = lnk_run_symbol_finder(tp, tp_arena, config->path_style, symtab, lookup_undef_list, lnk_undef_symbol_finder); // TODO: put these on temp arena

        // new inputs found
        input_obj_list    = result.input_obj_list;
        input_import_list = result.input_import_list;

        // undefined symbols that weren't resolved
        lnk_symbol_list_concat_in_place(&unresolved_undef_list, &result.unresolved_symbol_list);

        // reset input
        MemoryZeroStruct(&lookup_undef_list);
        ProfEnd();
      } break;
      case State_LookupWeak: {
        ProfBegin("Lookup Weak Symbols");
        // search archives
        LNK_SymbolFinderResult result = lnk_run_symbol_finder(tp, tp_arena, config->path_style, symtab, lookup_weak_list, lnk_weak_symbol_finder); // TODO: put these on temp arena

        // schedule new inputs
        input_obj_list    = result.input_obj_list;
        input_import_list = result.input_import_list;

        // weak symbols that weren't resolved
        lnk_symbol_list_concat_in_place(&unresolved_weak_list, &result.unresolved_symbol_list);

        // reset input
        MemoryZeroStruct(&lookup_weak_list);
        ProfEnd();
      } break;
      case State_CheckUnusedDelayLoads: {
        if (imptab_delayed) {
          for (String8Node *node = config->delay_load_dll_list.first; node != 0; node = node->next) {
            LNK_ImportDLL *dll = lnk_import_table_search_dll(imptab_delayed, node->string);
            if (dll == 0) {
              lnk_error(LNK_Warning_UnusedDelayLoadDll, "/DELAYLOAD: %S found no imports", node->string);
            }
          }
        }
      } break;
      case State_ReportUnresolvedSymbols: {
        // report unresolved symbols
        for (LNK_SymbolNode *node = unresolved_undef_list.first; node != 0; node = node->next) {
          lnk_error(LNK_Error_UnresolvedSymbol, "unresolved symbol %S", node->data->name);
        }
        if (unresolved_undef_list.count) {
          goto exit;
        }
      } break;
      case State_RewireComdats: {
        ProfBegin("Fold COMDAT symbols");
        lnk_fold_comdat_chunks(tp, symtab);
        ProfEnd();
      } break;

      case State_DiscardMetaDataSections: {
        ProfBegin("Discard Meta Data Sections");
        lnk_discard_meta_data_sections(st);
        ProfEnd();
      } break;
      case State_BuildDebugDirectory: {
        ProfBegin("Build Debug Directory");

        // push debug directory layout chunks
        LNK_Section *debug_sect            = lnk_section_table_search(st, str8_lit(".rdata"));
        LNK_Chunk   *debug_chunk           = lnk_section_push_chunk_list(debug_sect, debug_sect->root, str8(0,0));
        LNK_Chunk   *debug_dir_array_chunk = lnk_section_push_chunk_list(debug_sect, debug_chunk, str8(0,0));

        // push symbols for PE directory patch
        LNK_Symbol *dir_array_symbol = lnk_make_defined_symbol_chunk(symtab->arena, str8_lit(LNK_DEBUG_DIR_SYMBOL_NAME), LNK_DefinedSymbolVisibility_Internal, 0, debug_dir_array_chunk, 0, 0, 0);
        lnk_symbol_table_push(symtab, dir_array_symbol);

        // debug entry for PDB
        if (config->debug_mode != LNK_DebugMode_None && config->debug_mode != LNK_DebugMode_Null) {
          lnk_build_debug_pdb(st, symtab, debug_sect, debug_dir_array_chunk, config->time_stamp, config->guid, config->age, config->pdb_name);
        }

        // debug entry for RDI
        if (config->rad_debug == LNK_SwitchState_Yes) {
          lnk_build_debug_rdi(st, symtab, debug_sect, debug_dir_array_chunk, config->time_stamp, config->guid, config->rad_debug_name);
        }

        ProfEnd();
      } break;
      case State_BuildExportTable: {
        ProfBegin("Build Export Table");

        lnk_collect_exports_from_obj_directives(exptab, obj_list, symtab);
        lnk_build_edata(exptab, st, symtab, config->image_name, config->machine);

        ProfEnd();
      } break;
      case State_MergeSections: {
        ProfBegin("Merge Sections");
        LNK_MergeDirectiveList merge_list = lnk_init_merge_directive_list(scratch.arena, obj_list);
        lnk_section_table_merge(st, merge_list);
        ProfEnd();
      } break;
      case State_BuildCFGuards: {
        ProfBegin("Build CF Guards");
        B32 emit_suppress_flag = 1; // MSVC emits this flag but every entry has zero set.
        lnk_build_guard_tables(tp, st, symtab, exptab, obj_list, config->machine, config->entry_point_name, config->guard_flags, emit_suppress_flag);
        ProfEnd();
      } break;
      case State_BuildBaseRelocs: {
        ProfBegin("Base Relocs");
        lnk_build_base_relocs(tp, tp_arena, st, symtab, config->machine, config->page_size, obj_list);
        ProfEnd();
      } break;
      case State_BuildWin32Header: {
        ProfBegin("Build Win32 Header");

        // remove empty section headers from output image
        lnk_section_table_remove_empties(st, symtab);
  
        // gather output sections
        LNK_SectionArray out_sect_arr = lnk_section_table_get_output_sections(scratch.arena, st);

        // push back null section where we store image header
        LNK_Section *header_sect = lnk_section_table_push_null(st);

        // fill out header section with win32 image header data
        lnk_build_win32_image_header(symtab, header_sect, header_sect->root, config, out_sect_arr);
        
        ProfEnd();
      } break;
      case State_PatchRelocs: {
        ProfBegin("Patch Relocs");
        U64 base_addr = lnk_get_base_addr(config);
        lnk_section_table_build_data(tp, st, config->machine);
        lnk_section_table_assign_indices(st);
        lnk_section_table_assign_virtual_offsets(st);
        lnk_section_table_assign_file_offsets(st);
        lnk_patch_relocs_obj(tp, obj_list, symtab, st, base_addr);
        lnk_patch_relocs(tp, symtab, st, base_addr);
        ProfEnd();
      } break;
      case State_SortExceptionInfo: {
        ProfBegin("Sort Exception Info");
        LNK_Symbol *pdata_symbol = lnk_symbol_table_searchf(symtab, LNK_SymbolScopeFlag_Internal, LNK_PDATA_SYMBOL_NAME);
        if (pdata_symbol) {
          LNK_Section **sect_id_map = lnk_sect_id_map_from_section_table(scratch.arena, st);
          String8 pdata = lnk_data_from_chunk_ref_no_pad(sect_id_map, pdata_symbol->u.defined.u.chunk->ref);

          switch (config->machine) {
          case COFF_MachineType_X86:
          case COFF_MachineType_X64: {
            U64 count = pdata.size / sizeof(PE_IntelPdata);
            radsort((PE_IntelPdata *)pdata.str, count, lnk_pdata_is_before_x8664);
          } break;
          case COFF_MachineType_ARM64:
          case COFF_MachineType_ARM: {
            Assert(!"TOOD: ARM");
          } break;
          case COFF_MachineType_MIPSFPU:
          case COFF_MachineType_MIPS16:
          case COFF_MachineType_MIPSFPU16: {
            Assert(!"TODO: MIPS");
          } break;
          }
        }
        ProfEnd();
      } break;
      case State_WriteImage: {
        ProfEnd(); // :EndBuild

        if (lnk_get_log_status(LNK_Log_InputObj)) {
          U64 total_input_size = 0;
          for (LNK_ObjNode *obj_n = obj_list.first; obj_n != 0; obj_n = obj_n->next) {
            total_input_size += obj_n->data.data.size;
          }
          String8 size_string = str8_from_memory_size2(scratch.arena, total_input_size);
          lnk_log(LNK_Log_InputObj, "[Total Obj Input Size %S]", size_string);
        }
        if (lnk_get_log_status(LNK_Log_InputLib)) {
          U64 total_input_size = 0;
          for (U64 i = 0; i < ArrayCount(lib_index); ++i) {
            LNK_LibList list = lib_index[i];
            for (LNK_LibNode *lib_n = list.first; lib_n != 0; lib_n = lib_n->next) {
              total_input_size += lib_n->data.data.size;
            }
          }
          String8 size_string = str8_from_memory_size2(scratch.arena, total_input_size);
          lnk_log(LNK_Log_InputLib, "[Total Lib Input Size %S]", size_string);
        }

        ProfBegin("Image Serialize");
        image_data = lnk_section_table_serialize(scratch.arena, st);
        ProfEnd();
        
        LNK_Section **sect_id_map = lnk_sect_id_map_from_section_table(scratch.arena, st);

        if (config->flags & LNK_ConfigFlag_WriteImageChecksum) {
          ProfBegin("Image Checksum");

          U32 image_checksum = pe_compute_checksum(image_data.str, image_data.size);

          LNK_Symbol   *checksum_symbol = lnk_symbol_table_searchf(symtab, LNK_SymbolScopeFlag_Internal, LNK_PE_CHECKSUM_SYMBOL_NAME); 
          U64           checksum_foff   = lnk_file_off_from_symbol(sect_id_map, checksum_symbol);

          U32 *checksum_ptr = (U32 *)(image_data.str + checksum_foff);
          *checksum_ptr = image_checksum;

          ProfEnd();
        }

        LNK_Symbol *guid_symbol = lnk_symbol_table_searchf(symtab, LNK_SymbolScopeFlag_Internal, LNK_CV_HEADER_GUID_SYMBOL_NAME);
        if (guid_symbol) {
          Assert(build_debug_info);

          switch (config->guid_type) {
          case LNK_DebugInfoGuid_Null: break;
          case Lnk_DebugInfoGuid_ImageBlake3: {
            ProfBegin("Hash Image With Blake3");

            U128 hash = lnk_blake3_hash_parallel(tp, 128, image_data);
            MemoryCopy(&config->guid, hash.u64, sizeof(hash.u64));

            U64      guid_foff = lnk_file_off_from_symbol(sect_id_map, guid_symbol);
            OS_Guid *guid_ptr  = (OS_Guid *)(image_data.str + guid_foff);
            MemoryCopy(guid_ptr, hash.u64, sizeof(hash.u64));

            ProfEnd();
          } break;
          }
        }

        LNK_WriteThreadContext *ctx = push_array(scratch.arena, LNK_WriteThreadContext, 1);
        ctx->path                   = config->image_name;
        ctx->data                   = image_data;
        image_write_thread = os_thread_launch(lnk_write_thread, ctx, 0);

        lnk_timer_end(LNK_Timer_Image);
        ProfEnd(); // :EndImage
      } break;
      case State_BuildImpLib: {
        ProfBegin("Build Imp Lib");
        lnk_timer_begin(LNK_Timer_Lib);
        String8List lib_list = lnk_build_import_lib(tp, tp_arena, config->machine, config->time_stamp, config->imp_lib_name, config->image_name, exptab);
        lnk_write_data_list_to_file_path(config->imp_lib_name, lib_list);
        lnk_timer_end(LNK_Timer_Lib);
        ProfEnd();
      } break;
      case State_BuildDebugInfo: {
        ProfBegin("Debug Info");
        lnk_timer_begin(LNK_Timer_Debug);

        LNK_CodeViewInput input       = lnk_make_code_view_input(tp, tp_arena, config->lib_dir_list, obj_list);
        CV_DebugT        *types       = lnk_import_types(tp, tp_arena, &input);
        LNK_Section     **sect_id_map = lnk_sect_id_map_from_section_table(scratch.arena, st);

        if (config->rad_debug == LNK_SwitchState_Yes) {
          lnk_timer_begin(LNK_Timer_Rdi);
          RDI_Arch         arch        = rdi_arch_from_coff_machine(config->machine);
          LNK_SectionArray image_sects = lnk_section_table_get_output_sections(scratch.arena, st);

          String8List rdi_data = lnk_build_rad_debug_info(tp,
                                                          tp_arena,
                                                          config->target_os,
                                                          arch,
                                                          config->image_name,
                                                          image_data,
                                                          image_sects,
                                                          sect_id_map,
                                                          input.count,
                                                          input.obj_arr,
                                                          input.debug_s_arr,
                                                          input.total_symbol_input_count,
                                                          input.symbol_inputs,
                                                          input.parsed_symbols,
                                                          types);

          lnk_write_data_list_to_file_path(config->rad_debug_name, rdi_data);
          lnk_timer_end(LNK_Timer_Rdi);
        }

        // TODO: Parallel debug info builds are currently blocked by the patch
        // strings in $$FILE_CHECKSUM step in `lnk_process_c13_data_task`.
        if (config->debug_mode == LNK_DebugMode_Full || config->debug_mode == LNK_DebugMode_GHash) {
          lnk_timer_begin(LNK_Timer_Pdb);
          String8List pdb_data = lnk_build_pdb(tp,
                                               tp_arena,
                                               config->guid,
                                               config->machine,
                                               config->time_stamp,
                                               config->age,
                                               config->pdb_page_size,
                                               config->pdb_name,
                                               config->lib_dir_list,
                                               config->natvis_list,
                                               symtab,
                                               sect_id_map,
                                               input.count,
                                               input.obj_arr,
                                               input.debug_s_arr,
                                               input.total_symbol_input_count,
                                               input.symbol_inputs,
                                               input.parsed_symbols,
                                               types);

          lnk_write_data_list_to_file_path(config->pdb_name, pdb_data);
          lnk_timer_end(LNK_Timer_Pdb);
        } else if (config->debug_mode == LNK_DebugMode_FastLink) {
          lnk_not_implemented("FASTLINK");
        }

        lnk_timer_end(LNK_Timer_Debug);
        ProfEnd();
      } break;
      }
    }

    if (input_disallow_lib_list.node_count) {
      state_list_push(scratch.arena, state_list, State_InputDisallowLibs);
      continue;
    }
    if (input_import_list.count) {
      state_list_push(scratch.arena, state_list, State_InputImports);
      continue;
    }
    if (input_defn_list.count ||
        input_weak_list.count ||
        include_symbol_list.node_count ||
        alt_name_list.from_list.node_count) {
      state_list_push(scratch.arena, state_list, State_InputSymbols);
      continue;
    }
    if (input_obj_list.count) {
      state_list_push(scratch.arena, state_list, State_InputObjs);
      continue;
    }
    {
      B32 have_pending_lib_inputs = 0;
      for (U64 i = 0; i < ArrayCount(input_libs); ++i) {
        if (input_libs[i].node_count) {
         have_pending_lib_inputs = 1;
          break;
        }
      }
      if (have_pending_lib_inputs) {
        state_list_push(scratch.arena, state_list, State_InputLibs);
        continue;
      }
    }
    if (lookup_undef_list.count) {
      state_list_push(scratch.arena, state_list, State_LookupUndef);
      continue;
    }
    if (lookup_weak_list.count) {
      state_list_push(scratch.arena, state_list, State_LookupWeak);
      continue;
    }
    if (unresolved_weak_list.count) {
      // we can't find strong definitions for unresolved weak symbols
      // so now we have to use fallback symbols
      MemoryZeroStruct(&unresolved_weak_list);

      // make sure fallback symbols are defined, if not try to find definitions
      for (LNK_SymbolNode *symbol_n = unresolved_weak_list.first; symbol_n != 0; symbol_n = symbol_n->next) {
        if (symbol_n->data->u.weak.fallback_symbol->type == LNK_Symbol_Undefined) {
          lnk_symbol_list_push(scratch.arena, &lookup_undef_list, symbol_n->data->u.weak.fallback_symbol);
        }
      }

      continue;
    }
    if (entry_search_attempts == 0) {
      state_list_push(scratch.arena, state_list, State_SearchEntryPoint);
      entry_search_attempts += 1;
      continue;
    }
    if (build_res_obj) {
      build_res_obj = 0;
      state_list_push(scratch.arena, state_list, State_BuildAndInputResObj);
      continue;
    }
    if (build_linker_obj) {
      build_linker_obj = 0;
      state_list_push(scratch.arena, state_list, State_BuildAndInputLinkerObj);
      continue;
    }
    if (check_unused_delay_loads) {
      check_unused_delay_loads = 0;
      state_list_push(scratch.arena, state_list, State_CheckUnusedDelayLoads);
      continue;
    }
    if (unresolved_undef_list.count) {
      if (report_unresolved_symbols) {
        report_unresolved_symbols = 0;
        state_list_push(scratch.arena, state_list, State_ReportUnresolvedSymbols);
        continue;
      }
    }
    if (do_comdat_rewire) {
      do_comdat_rewire = 0;
      state_list_push(scratch.arena, state_list, State_RewireComdats);
      continue;
    }

    /// --- inputs are ready ---

    if (discard_meta_data_sections) {
      discard_meta_data_sections = 0;
      state_list_push(scratch.arena, state_list, State_DiscardMetaDataSections);
      continue;
    }
    if (build_debug_directory) {
      build_debug_directory = 0;
      state_list_push(scratch.arena, state_list, State_BuildDebugDirectory);
      continue;
    }
    if (build_export_table) {
      build_export_table = 0;
      state_list_push(scratch.arena, state_list, State_BuildExportTable);
      continue;
    }
    if (merge_sections) {
      merge_sections = 0;
      state_list_push(scratch.arena, state_list, State_MergeSections);
      continue;
    }
    if (build_cf_guards) {
      build_cf_guards = 0;
      state_list_push(scratch.arena, state_list, State_BuildCFGuards);
      continue;
    }
    if (build_base_relocs) {
      build_base_relocs = 0;
      state_list_push(scratch.arena, state_list, State_BuildBaseRelocs);
      continue;
    }
    if (build_win32_header) {
      build_win32_header = 0;
      state_list_push(scratch.arena, state_list, State_BuildWin32Header);
      continue;
    }
    if (patch_relocs) {
      patch_relocs = 0;
      state_list_push(scratch.arena, state_list, State_PatchRelocs);
      continue;
    }
    if (sort_exception_info) {
      sort_exception_info = 0;
      state_list_push(scratch.arena, state_list, State_SortExceptionInfo);
      continue;
    }
    if (image_data.size == 0) {
      state_list_push(scratch.arena, state_list, State_WriteImage);
      continue;
    }
    if (build_imp_lib) {
      build_imp_lib = 0;
      if (config->file_characteristics & PE_ImageFileCharacteristic_FILE_DLL) {
        state_list_push(scratch.arena, state_list, State_BuildImpLib);
        continue;
      }
    }
    if (build_debug_info) {
      build_debug_info = 0;
      state_list_push(scratch.arena, state_list, State_BuildDebugInfo);
      continue;
    }

    // wait for the thread to finish writing image to disk
    os_thread_join(image_write_thread, -1);

    break;
  }

  if (lnk_get_log_status(LNK_Log_SizeBreakdown)) {
    lnk_log_size_breakdown(st, symtab);
  }
  if (lnk_get_log_status(LNK_Log_LinkStats)) {
    lnk_log_link_stats(obj_list, lib_index, st);
  }
  if (lnk_get_log_status(LNK_Log_Timers)) {
    lnk_log_timers();
  }

exit:;

  // linker is done punt memory release to OS
  //lnk_section_table_release(&st);
  //lnk_symbol_table_release(&symtab);
  //lnk_export_table_release(&export_table);
  //lnk_import_table_release(&imptab_regular);
  //lnk_import_table_release(&imptab_delayed);
  //tp_arena_release(&tp_arena);

  scratch_end(scratch);
  ProfEnd();

#undef state_list_push
#undef state_list_pop
}

internal void
entry_point(CmdLine *cmdline)
{
  Temp scratch = scratch_begin(0,0);

#if PROFILE_TELEMETRY
  tmMessage(0, TMMF_ICON_NOTE, BUILD_TITLE);
#endif

  // TODO: temp hack to make custom command line work while syncing with latest code base changes
  int    argc;
  char **argv;
  {
    LPWSTR w32_cmd_line = GetCommandLineW();
    argc = 0;
    LPWSTR *argvw = CommandLineToArgvW(w32_cmd_line, &argc);
    argv = push_array(scratch.arena, char *, argc);
    for(int i = 0; i < argc; ++i)
    {
      String16 arg16 = str16_cstring((U16 *)argvw[i]);
      String8  arg8  = str8_from_16(scratch.arena, arg16);
      argv[i] = (char *)arg8.str;
    }
  }

  lnk_init_error_handler();
  lnk_run(argc, argv);

  scratch_end(scratch);
}

#if 0
internal void
lnk_dump_symbol_table(FILE *f, LNK_SymbolTable *symtab)
{
  for (U64 bucket_idx = 0; bucket_idx < symtab->bucket_count; bucket_idx += 1) {
    LNK_SymbolList *bucket = symtab->buckets[bucket_idx];
    if (bucket) {
      U64 node_idx = 0;
      for (LNK_SymbolNode *symbol_node = bucket->first; symbol_node != 0; symbol_node = symbol_node->next, node_idx += 1) {
        LNK_Symbol *symbol = symbol_node->data;
        fprintf(f, "[%04llX,%04llX] %.*s\n", bucket_idx, node_idx, str8_varg(symbol->name));
      }
    }
  }
}

int
lnk_chunk_size_compar(void *ud, const void *a, const void *b)
{
  LNK_Section **sect_id_map = (LNK_Section**)ud;
  LNK_ChunkPtr ac = *(LNK_ChunkPtr*)a;
  LNK_ChunkPtr bc = *(LNK_ChunkPtr*)b;
  U64 as = lnk_virt_size_from_chunk_ref(sect_id_map, ac->ref);
  U64 bs = lnk_virt_size_from_chunk_ref(sect_id_map, bc->ref);
  int cmp = as < bs ? -1 : as > bs ? +1 : 0;
  return cmp;
}

internal LNK_ChunkArray
lnk_query_chunks_near_voff_ex(Arena *arena, LNK_SectionTable *st, LNK_Section **sect_id_map, U64 voff)
{
  Temp scratch = scratch_begin(&arena, 1);
  LNK_ChunkArray result; MemoryZeroStruct(&result);
  for (U64 id = 0; id < st->id_max; ++id) {
    LNK_Section *sect = sect_id_map[id];
    U64 root_voff = lnk_virt_off_from_chunk_ref(sect_id_map, sect->root->ref);
    U64 root_size = lnk_virt_size_from_chunk_ref(sect_id_map, sect->root->ref);
    if (root_voff <= voff && voff < root_voff + root_size) {
      U64List list; MemoryZeroStruct(&list);
      for (U64 chunk_id = 0; chunk_id < sect->cman->total_chunk_count; ++chunk_id) {
        LNK_ChunkRef chunk_ref = { sect->id, chunk_id };
        U64 chunk_voff = lnk_virt_off_from_chunk_ref(sect_id_map, chunk_ref);
        U64 chunk_size = lnk_virt_size_from_chunk_ref(sect_id_map, chunk_ref);
        if (chunk_voff <= voff && voff < chunk_voff + chunk_size) {
          u64_list_push(scratch.arena, &list, chunk_id);
        }
      }
      
      if (list.count) {
        result.count = 0;
        result.v = push_array_no_zero(arena, LNK_ChunkPtr, list.count);
        LNK_ChunkPtr *chunk_id_map = lnk_make_chunk_id_map(scratch.arena, sect->cman);
        for (U64Node *i = list.first; i != NULL; i = i->next) {
          result.v[result.count++] = chunk_id_map[i->data];
        }
        qsort_s((void*)result.v, result.count, sizeof(result.v[0]), lnk_chunk_size_compar, sect_id_map);
      }
      
      break;
    }
  }
  scratch_end(scratch);
  return result;
}

internal LNK_ChunkArray
lnk_query_chunks_near_voff(Arena *arena, LNK_SectionTable *st, U64 voff)
{
  Temp scratch = scratch_begin(&arena, 1);
  LNK_Section **sect_id_map = lnk_sect_id_map_from_section_table(scratch.arena, st);
  LNK_ChunkArray result = lnk_query_chunks_near_voff_ex(arena, st, sect_id_map, voff);
  scratch_end(scratch);
  return result;
}

internal void
lnk_dump_crt_inits(LNK_SectionTable *st, LNK_SymbolTable *symtab)
{
  static struct {
    char *first;
    char *last;
  } table[] = {
    { "__xi_a", "__xi_z" },
    { "__xc_a", "__xc_z" },
    { "__xp_a", "__xp_z" },
    { "__xt_a", "__xt_z" }
  };
  
  Temp scratch = scratch_begin(0, 0);
  LNK_Section **sect_id_map = lnk_sect_id_map_from_section_table(scratch.arena, st);
  for (U64 i = 0; i < ArrayCount(table); ++i) {
    LNK_Symbol *first = lnk_symbol_table_searchf(symtab, LNK_SymbolScopeFlag_Main, table[i].first);
    LNK_Symbol *last = lnk_symbol_table_searchf(symtab, LNK_SymbolScopeFlag_Main, table[i].last);
    U64 first_voff = lnk_virt_off_from_chunk_ref(sect_id_map, first->u.defined.u.chunk->ref);
    U64 last_voff = lnk_virt_off_from_chunk_ref(sect_id_map, last->u.defined.u.chunk->ref);
    U64 ptr_size = sizeof(U64);
    U64 count = (last_voff - first_voff) / ptr_size;
    printf("(%s-%s)\n", table[i].first, table[i].last);
    for (U64 ptr_idx = 0; ptr_idx < count; ++ptr_idx) {
      LNK_ChunkArray chunk_ptr_arr = lnk_query_chunks_near_voff_ex(scratch.arena, st, sect_id_map, first_voff + ptr_idx * ptr_size);
      LNK_Chunk *chunk = chunk_ptr_arr.v[0];
      printf("\t%.*s\n", str8_varg(chunk->debug));
    }
  }
  scratch_end(scratch);
}

internal void
lnk_dump_resource_dir_(COFF_ResourceID dir_id, PE_ResourceDir *dir)
{
  Temp scratch = scratch_begin(0, 0);
  
  SYMS_String8 dir_id_syms = syms_str8(0,0);
  if (dir_id.type == COFF_ResourceIDType_NUMBER) {
    dir_id_syms = syms_pe_resource_type_to_string(dir_id.u.number);
  }
  if (dir_id_syms.size == 0) {
    dir_id_syms = syms_coff_resource_id_to_string(scratch.arena, dir_id);
  }
  
  tool_fprintf(stdout, "ID: %.*s, Characteristics: %u, Time stamp: %u, Version: %u.%u\n",
               syms_expand_string(dir_id_syms), dir->characteristics, dir->time_stamp, dir->major_version, dir->minor_version);
  tool_fprintf(stdout, "{\n");
  tool_indent(stdout);
  
  PE_ResourceList list_arr[2];
  list_arr[0] = dir->named_list;
  list_arr[1] = dir->id_list;
  
  for (U64 i = 0; i < ArrayCount(list_arr); ++i) {
    PE_ResourceList *list = &list_arr[i];
    for (PE_ResourceNode *n = list->first; n != NULL; n = n->next) {
      PE_Resource *res = &n->data;
      switch (res->type) {
      default: InvalidPath;
      case PE_ResData_NULL: break;
      case PE_ResData_DIR: {
        lnk_dump_resource_dir_(res->id, res->u.dir);
      } break;
      case PE_ResData_COFF_LEAF: {
        SYMS_String8 id_syms = syms_coff_resource_id_to_string(scratch.arena, res->id);
        tool_fprintf(stdout, "ID: %.*s Data voff: 0x%X, Data size: %u, Code page: %u, Reserved: %u\n", 
                     syms_expand_string(id_syms), res->u.leaf.data_voff, res->u.leaf.data_size, res->u.leaf.code_page, res->u.leaf.reserved);
      } break;
      case PE_ResData_COFF_RESOURCE: {
        SYMS_String8 id_syms = syms_coff_resource_id_to_string(scratch.arena, res->id);
        SYMS_String8 type_syms = syms_str8(0,0);
        if (res->u.coff_res.type.type == COFF_ResourceIDType_NUMBER) {
          type_syms = syms_pe_resource_type_to_string(res->u.coff_res.type.u.number);
        }
        if (type_syms.size == 0) {
          type_syms = syms_coff_resource_id_to_string(scratch.arena, res->u.coff_res.type);
        }
        tool_fprintf(stdout, "ID: %.*s Data version: %u, Version: %u, Memory flags: %u, Data Byte Count: %u\n",
                     syms_expand_string(id_syms), res->u.coff_res.data_version, res->u.coff_res.version, res->u.coff_res.memory_flags, res->u.coff_res.data.size);
      } break;
      }
    }
  }
  
  tool_unindent(stdout);
  tool_fprintf(stdout, "}\n");
  scratch_end(scratch);
}

internal void
lnk_dump_resource_dir(PE_ResourceDir *dir)
{
  COFF_ResourceID dir_id;
  dir_id.type = COFF_ResourceIDType_NUMBER;
  dir_id.u.number = 0;
  lnk_dump_resource_dir_(dir_id, dir);
}

#endif
