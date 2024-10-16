// Copyright (c) 2024 Epic Games Tools
// Licensed under the MIT license (https://opensource.org/license/mit/)

////////////////////////////////

internal U64
hash_from_cv_symbol(CV_Symbol *symbol)
{
  XXH3_state_t hasher;
  XXH3_64bits_reset(&hasher);
  XXH3_64bits_update(&hasher, &symbol->kind, sizeof(symbol->kind));
  XXH3_64bits_update(&hasher, &symbol->data.size, sizeof(symbol->data.size));
  XXH3_64bits_update(&hasher, symbol->data.str, symbol->data.size);
  XXH64_hash_t hash = XXH3_64bits_digest(&hasher);
  return hash;
}

////////////////////////////////
// Type Index Helpers

internal CV_TypeIndexInfo *
cv_symbol_type_index_info_push(Arena *arena, CV_TypeIndexInfoList *list, CV_TypeIndexSource source, U64 offset)
{
  CV_TypeIndexInfo *info = push_array_no_zero(arena, CV_TypeIndexInfo, 1);
  info->next   = 0;
  info->offset = offset;
  info->source = source;

  SLLQueuePush(list->first, list->last, info);
  list->count += 1;

  return info;
}

internal CV_TypeIndexInfoList
cv_get_symbol_type_index_offsets(Arena *arena, CV_SymKind kind, String8 data)
{
  CV_TypeIndexInfoList list = {0};
  switch (kind) {
  case CV_SymKind_BUILDINFO: {
    cv_symbol_type_index_info_push(arena, &list, CV_TypeIndexSource_IPI, OffsetOf(CV_SymBuildInfo, id));
  } break;
  case CV_SymKind_GDATA32:
  case CV_SymKind_LDATA32: {
    cv_symbol_type_index_info_push(arena, &list, CV_TypeIndexSource_TPI, OffsetOf(CV_SymData32, itype));
  } break;
  case CV_SymKind_LPROC32_ID:
  case CV_SymKind_GPROC32_ID: 
  case CV_SymKind_LPROC32_DPC_ID: {
    cv_symbol_type_index_info_push(arena, &list, CV_TypeIndexSource_IPI, OffsetOf(CV_SymProc32, itype));
  } break;
  case CV_SymKind_GPROC32:
  case CV_SymKind_LPROC32: 
  case CV_SymKind_LPROC32_DPC: {
    cv_symbol_type_index_info_push(arena, &list, CV_TypeIndexSource_TPI, OffsetOf(CV_SymProc32, itype));
  } break;
  case CV_SymKind_UDT: {
    cv_symbol_type_index_info_push(arena, &list, CV_TypeIndexSource_TPI, OffsetOf(CV_SymUDT, itype));
  } break;
  case CV_SymKind_GTHREAD32: {
    cv_symbol_type_index_info_push(arena, &list, CV_TypeIndexSource_TPI, OffsetOf(CV_SymThread32, itype));
  } break;
  case CV_SymKind_FILESTATIC: {
    cv_symbol_type_index_info_push(arena, &list, CV_TypeIndexSource_TPI, OffsetOf(CV_SymFileStatic, itype));
  } break;
  case CV_SymKind_LOCAL: {
    cv_symbol_type_index_info_push(arena, &list, CV_TypeIndexSource_TPI, OffsetOf(CV_SymLocal, itype));
  } break;
  case CV_SymKind_REGREL32: 
  case CV_SymKind_BPREL32: {
    cv_symbol_type_index_info_push(arena, &list, CV_TypeIndexSource_TPI, OffsetOf(CV_SymRegrel32, itype));
  } break;
  case CV_SymKind_REGISTER: {
    cv_symbol_type_index_info_push(arena, &list, CV_TypeIndexSource_TPI, OffsetOf(CV_SymRegister, itype));
  } break;
  case CV_SymKind_CONSTANT: {
    cv_symbol_type_index_info_push(arena, &list, CV_TypeIndexSource_TPI, OffsetOf(CV_SymConstant, itype));
  } break;
  case CV_SymKind_CALLSITEINFO: {
    cv_symbol_type_index_info_push(arena, &list, CV_TypeIndexSource_TPI, OffsetOf(CV_SymCallSiteInfo, itype));
  } break;
  case CV_SymKind_CALLERS:
  case CV_SymKind_CALLEES:
  case CV_SymKind_INLINEES: {
    Assert(data.size >= sizeof(CV_SymFunctionList));
    CV_SymFunctionList *func_list = (CV_SymFunctionList*)data.str;
    for (U64 i = 0; i < func_list->count; ++i) {
      cv_symbol_type_index_info_push(arena, &list, CV_TypeIndexSource_IPI, sizeof(CV_SymFunctionList) + i * sizeof(CV_TypeIndex));
    }
  } break;
  case CV_SymKind_INLINESITE: {
    cv_symbol_type_index_info_push(arena, &list, CV_TypeIndexSource_IPI, OffsetOf(CV_SymInlineSite, inlinee));
  } break;
  case CV_SymKind_HEAPALLOCSITE: {
    cv_symbol_type_index_info_push(arena, &list, CV_TypeIndexSource_TPI, OffsetOf(CV_SymHeapAllocSite, itype));
  } break;
  }
  return list;
}

internal CV_TypeIndexInfoList
cv_get_leaf_type_index_offsets(Arena *arena, CV_LeafKind leaf_kind, String8 data)
{
  CV_TypeIndexInfoList list = {0};
  switch (leaf_kind) {
  case CV_LeafKind_NOTYPE:
  case CV_LeafKind_VTSHAPE:
  case CV_LeafKind_LABEL:
  case CV_LeafKind_NULL: 
  case CV_LeafKind_NOTTRAN: {
    // no type indices
  } break;
  case CV_LeafKind_MODIFIER: {
    cv_symbol_type_index_info_push(arena, &list, CV_TypeIndexSource_TPI, OffsetOf(CV_LeafModifier, itype));
  } break;
  case CV_LeafKind_POINTER: {
    cv_symbol_type_index_info_push(arena, &list, CV_TypeIndexSource_TPI, OffsetOf(CV_LeafPointer, itype));
    CV_LeafPointer *ptr = (CV_LeafPointer *)data.str;
    CV_PointerKind ptr_kind = CV_PointerAttribs_ExtractKind(ptr->attribs);
    if (ptr_kind == CV_PointerKind_BaseType) {
      // TODO: add CV_LeafPointerBaseType
      cv_symbol_type_index_info_push(arena, &list, CV_TypeIndexSource_TPI, sizeof(CV_LeafPointer) + 0);
    } else {
      CV_PointerMode ptr_mode = CV_PointerAttribs_ExtractMode(ptr->attribs);
      if (ptr_mode == CV_PointerMode_PtrMem || ptr_mode == CV_PointerMode_PtrMethod) {
        // TODO: add type for the CvLeafPointerMember to syms_cv.mc
        cv_symbol_type_index_info_push(arena, &list, CV_TypeIndexSource_TPI, sizeof(CV_LeafPointer) + 0);
      }
    }
  } break;
  case CV_LeafKind_ARRAY: {
    cv_symbol_type_index_info_push(arena, &list, CV_TypeIndexSource_TPI, OffsetOf(CV_LeafArray, entry_itype));
    cv_symbol_type_index_info_push(arena, &list, CV_TypeIndexSource_TPI, OffsetOf(CV_LeafArray, index_itype));
  } break;
  case CV_LeafKind_CLASS: 
  case CV_LeafKind_STRUCTURE:
  case CV_LeafKind_INTERFACE: {
    cv_symbol_type_index_info_push(arena, &list, CV_TypeIndexSource_TPI, OffsetOf(CV_LeafStruct, field_itype));
    cv_symbol_type_index_info_push(arena, &list, CV_TypeIndexSource_TPI, OffsetOf(CV_LeafStruct, derived_itype));
    cv_symbol_type_index_info_push(arena, &list, CV_TypeIndexSource_TPI, OffsetOf(CV_LeafStruct, vshape_itype));
  } break;
  case CV_LeafKind_CLASS2:
  case CV_LeafKind_STRUCT2: {
    cv_symbol_type_index_info_push(arena, &list, CV_TypeIndexSource_TPI, OffsetOf(CV_LeafStruct2, field_itype));
    cv_symbol_type_index_info_push(arena, &list, CV_TypeIndexSource_TPI, OffsetOf(CV_LeafStruct2, derived_itype));
    cv_symbol_type_index_info_push(arena, &list, CV_TypeIndexSource_TPI, OffsetOf(CV_LeafStruct2, vshape_itype));
  } break;
  case CV_LeafKind_UNION: {
    cv_symbol_type_index_info_push(arena, &list, CV_TypeIndexSource_TPI, OffsetOf(CV_LeafUnion, field_itype));
  } break;
  case CV_LeafKind_ALIAS: {
    cv_symbol_type_index_info_push(arena, &list, CV_TypeIndexSource_TPI, OffsetOf(CV_LeafAlias, itype));
  } break;
  case CV_LeafKind_FUNC_ID: {
    cv_symbol_type_index_info_push(arena, &list, CV_TypeIndexSource_IPI, OffsetOf(CV_LeafFuncId, scope_string_id));
    cv_symbol_type_index_info_push(arena, &list, CV_TypeIndexSource_TPI, OffsetOf(CV_LeafFuncId, itype));
  } break;
  case CV_LeafKind_MFUNC_ID: {
    cv_symbol_type_index_info_push(arena, &list, CV_TypeIndexSource_TPI, OffsetOf(CV_LeafMFuncId, owner_itype));
    cv_symbol_type_index_info_push(arena, &list, CV_TypeIndexSource_TPI, OffsetOf(CV_LeafMFuncId, itype));
  } break;
  case CV_LeafKind_STRING_ID: {
    cv_symbol_type_index_info_push(arena, &list, CV_TypeIndexSource_IPI, OffsetOf(CV_LeafStringId, substr_list_id));
  } break;
  case CV_LeafKind_UDT_SRC_LINE: {
    cv_symbol_type_index_info_push(arena, &list, CV_TypeIndexSource_TPI, OffsetOf(CV_LeafUDTSrcLine, udt_itype));
    cv_symbol_type_index_info_push(arena, &list, CV_TypeIndexSource_IPI, OffsetOf(CV_LeafUDTSrcLine, src_string_id));
  } break;
  case CV_LeafKind_UDT_MOD_SRC_LINE: {
    cv_symbol_type_index_info_push(arena, &list, CV_TypeIndexSource_TPI, OffsetOf(CV_LeafUDTModSrcLine, udt_itype));
    cv_symbol_type_index_info_push(arena, &list, CV_TypeIndexSource_IPI, OffsetOf(CV_LeafUDTModSrcLine, src_string_id));
  } break;
  case CV_LeafKind_BUILDINFO: {
    Assert(data.size >= sizeof(CV_LeafBuildInfo));
    CV_LeafBuildInfo *build_info = (CV_LeafBuildInfo *)data.str;
    for (U16 i = 0; i < build_info->count; ++i) {
      cv_symbol_type_index_info_push(arena, &list, CV_TypeIndexSource_IPI, sizeof(CV_LeafBuildInfo) + i * sizeof(CV_ItemId));
    }
  } break;
  case CV_LeafKind_ENUM: {
    cv_symbol_type_index_info_push(arena, &list, CV_TypeIndexSource_TPI, OffsetOf(CV_LeafEnum, base_itype));
    cv_symbol_type_index_info_push(arena, &list, CV_TypeIndexSource_TPI, OffsetOf(CV_LeafEnum, field_itype));
  } break;
  case CV_LeafKind_PROCEDURE: {
    cv_symbol_type_index_info_push(arena, &list, CV_TypeIndexSource_TPI, OffsetOf(CV_LeafProcedure, ret_itype));
    cv_symbol_type_index_info_push(arena, &list, CV_TypeIndexSource_TPI, OffsetOf(CV_LeafProcedure, arg_itype));
  } break;
  case CV_LeafKind_MFUNCTION: {
    cv_symbol_type_index_info_push(arena, &list, CV_TypeIndexSource_TPI, OffsetOf(CV_LeafMFunction, ret_itype));
    cv_symbol_type_index_info_push(arena, &list, CV_TypeIndexSource_TPI, OffsetOf(CV_LeafMFunction, class_itype));
    cv_symbol_type_index_info_push(arena, &list, CV_TypeIndexSource_TPI, OffsetOf(CV_LeafMFunction, this_itype));
    cv_symbol_type_index_info_push(arena, &list, CV_TypeIndexSource_TPI, OffsetOf(CV_LeafMFunction, arg_itype));
  } break;
  case CV_LeafKind_VFTABLE: {
    cv_symbol_type_index_info_push(arena, &list, CV_TypeIndexSource_TPI, OffsetOf(CV_LeafVFTable, owner_itype));
    cv_symbol_type_index_info_push(arena, &list, CV_TypeIndexSource_TPI, OffsetOf(CV_LeafVFTable, base_table_itype));
  } break;
  case CV_LeafKind_VFTPATH: {
    Assert(sizeof(CV_LeafVFPath) <= data.size);
    CV_LeafVFPath *vfpath = (CV_LeafVFPath *)data.str;
    for (U32 i = 0; i < vfpath->count; ++i) {
      cv_symbol_type_index_info_push(arena, &list, CV_TypeIndexSource_TPI, sizeof(CV_LeafVFPath) + i * sizeof(CV_TypeId));
    }
  } break;
  case CV_LeafKind_TYPESERVER:
  case CV_LeafKind_TYPESERVER2:
  case CV_LeafKind_TYPESERVER_ST: {
    // no type indices
  } break;
  case CV_LeafKind_SKIP: {
    cv_symbol_type_index_info_push(arena, &list, CV_TypeIndexSource_TPI, OffsetOf(CV_LeafSkip, itype));
  } break;
  case CV_LeafKind_SUBSTR_LIST: {
    Assert(sizeof(CV_LeafArgList) <= data.size);
    CV_LeafArgList *arg_list = (CV_LeafArgList*)data.str;
    for (U32 i = 0; i < arg_list->count; ++i) {
      cv_symbol_type_index_info_push(arena, &list, CV_TypeIndexSource_IPI, sizeof(CV_LeafArgList) + i * sizeof(CV_TypeIndex));
    }
  } break;
  case CV_LeafKind_ARGLIST: {
    Assert(sizeof(CV_LeafArgList) <= data.size);
    CV_LeafArgList *arg_list = (CV_LeafArgList*)data.str;
    for (U32 i = 0; i < arg_list->count; ++i) {
      cv_symbol_type_index_info_push(arena, &list, CV_TypeIndexSource_TPI, sizeof(CV_LeafArgList) + i * sizeof(CV_TypeIndex));
    }
  } break;
  case CV_LeafKind_LIST: 
  case CV_LeafKind_FIELDLIST: {
    for (U64 cursor = 0; cursor < data.size; ) {
      CV_LeafKind list_member_kind = 0;
      U64 read_size = str8_deserial_read_struct(data, cursor, &list_member_kind);

      if(read_size != sizeof(list_member_kind)) {
        Assert(!"malformed LF_FIELDLIST");
        break;
      }
      cursor += read_size;
      
      switch (list_member_kind) {
      default: Assert(!"TODO: handle malformed field member"); break;
      case CV_LeafKind_INDEX: {
        cv_symbol_type_index_info_push(arena, &list, CV_TypeIndexSource_TPI, cursor + OffsetOf(CV_LeafIndex, itype));
        cursor += sizeof(CV_LeafIndex);
      } break;
      case CV_LeafKind_MEMBER: {
        cv_symbol_type_index_info_push(arena, &list, CV_TypeIndexSource_TPI, cursor + OffsetOf(CV_LeafMember, itype));
        cursor += sizeof(CV_LeafMember);

        CV_NumericParsed size;
        cursor += cv_read_numeric(data, cursor, &size);

        String8 name;
        cursor += str8_deserial_read_cstr(data, cursor, &name);
      } break;
      case CV_LeafKind_STMEMBER: {
        cv_symbol_type_index_info_push(arena, &list, CV_TypeIndexSource_TPI, cursor + OffsetOf(CV_LeafStMember, itype));
        cursor += sizeof(CV_LeafStMember);

        String8 name;
        cursor += str8_deserial_read_cstr(data, cursor, &name);
      } break;
      case CV_LeafKind_METHOD: {
        cv_symbol_type_index_info_push(arena, &list, CV_TypeIndexSource_TPI, cursor + OffsetOf(CV_LeafMethod, list_itype));
        cursor += sizeof(CV_LeafMethod);

        String8 name;
        cursor += str8_deserial_read_cstr(data, cursor, &name);
      } break;
      case CV_LeafKind_ONEMETHOD: {
        cv_symbol_type_index_info_push(arena, &list, CV_TypeIndexSource_TPI, cursor + OffsetOf(CV_LeafOneMethod, itype));

        CV_LeafOneMethod onemethod;
        cursor += str8_deserial_read_struct(data, cursor, &onemethod);

        CV_MethodProp prop = CV_FieldAttribs_ExtractMethodProp(onemethod.attribs);
        if(prop == CV_MethodProp_PureIntro || prop == CV_MethodProp_Intro)
        {
          cursor += sizeof(U32); // virtoff
        }

        String8 name;
        cursor += str8_deserial_read_cstr(data, cursor, &name);
      } break;
      case CV_LeafKind_ENUMERATE: {
        // no type index
        cursor += sizeof(CV_LeafEnumerate);
        CV_NumericParsed value;
        cursor += cv_read_numeric(data, cursor, &value);
        String8 name;
        cursor += str8_deserial_read_cstr(data, cursor, &name);
      } break;
      case CV_LeafKind_NESTTYPE: {
        cv_symbol_type_index_info_push(arena, &list, CV_TypeIndexSource_TPI, cursor + OffsetOf(CV_LeafNestType, itype));
        cursor += sizeof(CV_LeafNestType);

        String8 name;
        cursor += str8_deserial_read_cstr(data, cursor, &name);
      } break;
      case CV_LeafKind_NESTTYPEEX: {
        cv_symbol_type_index_info_push(arena, &list, CV_TypeIndexSource_TPI, cursor + OffsetOf(CV_LeafNestTypeEx, itype));

        cursor += sizeof(CV_LeafNestTypeEx);
        String8 name;
        cursor += str8_deserial_read_cstr(data, cursor, &name);
      } break;
      case CV_LeafKind_BCLASS: {
        cv_symbol_type_index_info_push(arena, &list, CV_TypeIndexSource_TPI, cursor + OffsetOf(CV_LeafBClass, itype));

        cursor += sizeof(CV_LeafBClass);
        CV_NumericParsed offset;
        cursor += cv_read_numeric(data, cursor, &offset);
      } break;
      case CV_LeafKind_VBCLASS:
      case CV_LeafKind_IVBCLASS: {
        cv_symbol_type_index_info_push(arena, &list, CV_TypeIndexSource_TPI, cursor + OffsetOf(CV_LeafVBClass, itype));
        cursor += sizeof(CV_LeafVBClass);

        CV_NumericParsed virtual_base_pointer;
        cursor += cv_read_numeric(data, cursor, &virtual_base_pointer);

        CV_NumericParsed virtual_base_offset;
        cursor += cv_read_numeric(data, cursor, &virtual_base_offset);
      } break;
      case CV_LeafKind_VFUNCTAB: {
        cv_symbol_type_index_info_push(arena, &list, CV_TypeIndexSource_TPI, cursor + OffsetOf(CV_LeafVFuncTab, itype));
        cursor += sizeof(CV_LeafVFuncTab);
      } break;
      case CV_LeafKind_VFUNCOFF: {
        cv_symbol_type_index_info_push(arena, &list, CV_TypeIndexSource_TPI, cursor + OffsetOf(CV_LeafVFuncOff, itype));
        cursor += sizeof(CV_LeafVFuncOff);
      } break;
      }
      cursor = AlignPow2(cursor, 4);
    }
  } break;
  case CV_LeafKind_METHOD: {
    cv_symbol_type_index_info_push(arena, &list, CV_TypeIndexSource_TPI, OffsetOf(CV_LeafMethod, list_itype));
  } break;
  case CV_LeafKind_METHODLIST: {
    for (U64 cursor = 0; cursor < data.size; ) {
      // read method
      CV_LeafMethodListMember method;
      U64 read_size = str8_deserial_read_struct(data, cursor, &method);

      // error check read
      if (read_size != sizeof(method)) {
        Assert(!"malformed LF_METHODLIST");
        break;
      }

      // push type index offset
      cv_symbol_type_index_info_push(arena, &list, CV_TypeIndexSource_TPI, cursor + OffsetOf(CV_LeafMethodListMember, itype));

      // take into account intro virtual offset
      CV_MethodProp mprop = CV_FieldAttribs_ExtractMethodProp(method.attribs);
      if (mprop == CV_MethodProp_Intro || mprop == CV_MethodProp_PureIntro) {
        read_size += sizeof(U32);
      }

      // advance
      cursor += read_size;
    }
  } break;
  case CV_LeafKind_ONEMETHOD: {
    cv_symbol_type_index_info_push(arena, &list, CV_TypeIndexSource_TPI, OffsetOf(CV_LeafOneMethod, itype));
  } break;
  case CV_LeafKind_BITFIELD: {
    cv_symbol_type_index_info_push(arena, &list, CV_TypeIndexSource_TPI, OffsetOf(CV_LeafBitField, itype));
  } break;
  case CV_LeafKind_PRECOMP:
  case CV_LeafKind_REFSYM: {
    // no type indices
  } break;
  case CV_LeafKind_INDEX: {
    cv_symbol_type_index_info_push(arena, &list, CV_TypeIndexSource_TPI, OffsetOf(CV_LeafIndex, itype));
  } break;
  case CV_LeafKind_MEMBER: {
    cv_symbol_type_index_info_push(arena, &list, CV_TypeIndexSource_TPI, OffsetOf(CV_LeafMember, itype));
  } break;
  case CV_LeafKind_VFUNCTAB: {
    cv_symbol_type_index_info_push(arena, &list, CV_TypeIndexSource_TPI, OffsetOf(CV_LeafVFuncTab, itype));
  } break;
  case CV_LeafKind_VFUNCOFF: {
    cv_symbol_type_index_info_push(arena, &list, CV_TypeIndexSource_TPI, OffsetOf(CV_LeafVFuncOff, itype));
  } break;
  case CV_LeafKind_NESTTYPE: {
    cv_symbol_type_index_info_push(arena, &list, CV_TypeIndexSource_TPI, OffsetOf(CV_LeafNestType, itype));
  } break;
  case CV_LeafKind_NESTTYPEEX: {
    cv_symbol_type_index_info_push(arena, &list, CV_TypeIndexSource_TPI, OffsetOf(CV_LeafNestTypeEx, itype));
  } break;
  default: {
    NotImplemented;
  } break;
  }
  return list;
}

internal CV_TypeIndexInfoList
cv_get_inlinee_type_index_offsets(Arena *arena, String8 raw_data)
{
  CV_TypeIndexInfoList list = {0};

  U64 cursor = 0;

  // first four bytes are always signature
  CV_C13InlineeLinesSig sig = max_U32;
  cursor += str8_deserial_read_struct(raw_data, cursor, &sig);

  while (cursor < raw_data.size) {
    // read header
    CV_C13InlineeSourceLineHeader *header = (CV_C13InlineeSourceLineHeader *) str8_deserial_get_raw_ptr(raw_data, cursor, sizeof(CV_C13InlineeSourceLineHeader));

    // store type index offset
    cv_symbol_type_index_info_push(arena, &list, CV_TypeIndexSource_IPI, cursor + OffsetOf(CV_C13InlineeSourceLineHeader, inlinee));

    // advance past header
    cursor += sizeof(*header);

    // skip extra files
    B32 has_extra_files = (sig == CV_C13InlineeLinesSig_EXTRA_FILES);
    if (has_extra_files) {
      U32 file_count = 0;
      cursor += str8_deserial_read_struct(raw_data, cursor, &file_count);
      cursor += /* file id: */ sizeof(U32) * file_count;
    }
  }

  return list;
}

internal String8Array
cv_get_data_around_type_indices(Arena *arena, CV_TypeIndexInfoList ti_list, String8 data)
{
  String8Array result;
  if (ti_list.count > 0) {
    result.count = ti_list.count + 1;
    result.v = push_array_no_zero(arena, String8, result.count);

    U64 cursor = 0;
    U64 ti_idx = 0;

    for (CV_TypeIndexInfo *ti_info = ti_list.first; ti_info != 0; ti_info = ti_info->next, ++ti_idx) {
      result.v[ti_idx].size = ti_info->offset - cursor;
      result.v[ti_idx].str  = data.str + cursor;
      cursor = ti_info->offset + sizeof(CV_TypeIndex);
    }

    result.v[result.count-1].size = data.size - cursor;
    result.v[result.count-1].str  = data.str + cursor;
  } else {
    result.count = 1;
    result.v = push_array_no_zero(arena, String8, 1);
    result.v[0] = data;
  }
  return result;
}

internal CV_TypeIndexSource
cv_type_index_source_from_leaf_kind(CV_LeafKind leaf_kind)
{
  CV_TypeIndexSource source;
  if (leaf_kind == CV_LeafKind_FUNC_ID      ||
      leaf_kind == CV_LeafKind_MFUNC_ID     ||
      leaf_kind == CV_LeafKind_BUILDINFO    ||
      leaf_kind == CV_LeafKind_SUBSTR_LIST  ||
      leaf_kind == CV_LeafKind_STRING_ID    ||
      leaf_kind == CV_LeafKind_UDT_SRC_LINE ||
      leaf_kind == CV_LeafKind_UDT_MOD_SRC_LINE) {
    source = CV_TypeIndexSource_IPI;
  } else if (leaf_kind == CV_LeafKind_NOTYPE) {
    source = CV_TypeIndexSource_NULL;
  } else {
    source = CV_TypeIndexSource_TPI;
  }
  return source;
}

////////////////////////////////

internal U64
cv_name_offset_from_symbol(CV_SymKind kind, String8 data)
{
  U64 offset = data.size;
  switch (kind) {
  case CV_SymKind_COMPILE: break;
  case CV_SymKind_OBJNAME: break;
  case CV_SymKind_THUNK32: {
    offset = sizeof(CV_SymThunk32); 
  } break;
  case CV_SymKind_LABEL32: {
    offset = sizeof(CV_SymLabel32); 
  } break;
  case CV_SymKind_REGISTER: {
    offset = sizeof(CV_SymRegister); 
  } break;
  case CV_SymKind_CONSTANT: {
    offset = sizeof(CV_SymConstant);
    CV_NumericParsed size;
    offset += cv_read_numeric(data, offset, &size);
  } break;
  case CV_SymKind_UDT: {
    offset = sizeof(CV_SymUDT);
  } break;
  case CV_SymKind_BPREL32: {
    offset = sizeof(CV_SymBPRel32);
  } break;
  case CV_SymKind_LDATA32:
  case CV_SymKind_GDATA32: {
    offset = sizeof(CV_SymData32);
  } break;
  case CV_SymKind_PUB32: {
    offset = sizeof(CV_SymPub32);
  } break;
  case CV_SymKind_LPROC32: 
  case CV_SymKind_GPROC32: 
  case CV_SymKind_LPROC32_ID:
  case CV_SymKind_GPROC32_ID: {
    offset = sizeof(CV_SymProc32);
  } break;
  case CV_SymKind_REGREL32: {
    offset = sizeof(CV_SymRegrel32);
  } break;
  case CV_SymKind_LTHREAD32:
  case CV_SymKind_GTHREAD32: {
    offset = sizeof(CV_SymData32);
  } break;
  case CV_SymKind_COMPILE2: break;
  case CV_SymKind_LOCALSLOT: {
    offset = sizeof(CV_SymSlot);
  } break;
  case CV_SymKind_PROCREF: 
  case CV_SymKind_LPROCREF:
  case CV_SymKind_DATAREF: {
    offset = sizeof(CV_SymRef2);
  } break;
  case CV_SymKind_TRAMPOLINE: break;
  case CV_SymKind_LOCAL: {
    offset = sizeof(CV_SymLocal);
  } break;
  default: InvalidPath;
  }
  return offset;
}

internal String8
cv_name_from_symbol(CV_SymKind kind, String8 data)
{
  U64 buf_off = cv_name_offset_from_symbol(kind, data);
  U8 *buf_ptr = data.str + buf_off;
  U8 *buf_opl = data.str + data.size;
  String8 name = str8_cstring_capped(buf_ptr, buf_opl);
  return name;
}

internal CV_UDTInfo
cv_get_udt_info(CV_LeafKind kind, String8 data)
{
  String8      name        = str8_zero();
  String8      unique_name = str8_zero();
  CV_TypeProps props       = 0;
  
  switch(kind) {
  case CV_LeafKind_CLASS:
  case CV_LeafKind_STRUCTURE:
  case CV_LeafKind_INTERFACE: {
    U64 cursor = 0;
    
    CV_LeafStruct udt;
    cursor += str8_deserial_read_struct(data, cursor, &udt);

    props = udt.props;
    
    CV_NumericParsed size;
    cursor += cv_read_numeric(data, cursor, &size);
    
    cursor += str8_deserial_read_cstr(data, cursor, &name);
    
    if (udt.props & CV_TypeProp_HasUniqueName) {
      cursor += str8_deserial_read_cstr(data, cursor, &unique_name);
    }
  } break;

  case CV_LeafKind_CLASS2:
  case CV_LeafKind_STRUCT2: {
    U64 cursor = 0;

    CV_LeafStruct2 udt;
    cursor += str8_deserial_read_struct(data, cursor, &udt);

    props = udt.props;

    CV_NumericParsed size;
    cursor += cv_read_numeric(data, cursor, &size);

    cursor += str8_deserial_read_cstr(data, cursor, &name);

    if (udt.props & CV_TypeProp_HasUniqueName) {
      cursor += str8_deserial_read_cstr(data, cursor, &unique_name);
    }
  } break;
  
  case CV_LeafKind_UNION: {
    U64 cursor = 0;
    
    CV_LeafUnion udt;
    cursor += str8_deserial_read_struct(data, cursor, &udt);

    CV_NumericParsed size;
    cursor += cv_read_numeric(data, cursor, &size);

    props = udt.props;
    
    cursor += str8_deserial_read_cstr(data, cursor, &name);
    
    if(udt.props & CV_TypeProp_HasUniqueName) {
      cursor += str8_deserial_read_cstr(data, cursor, &unique_name);
    }
  } break;
  
  case CV_LeafKind_ENUM: {
    U64 cursor = 0;
    
    CV_LeafEnum udt;
    cursor += str8_deserial_read_struct(data, cursor, &udt);

    props = udt.props;
    
    cursor += str8_deserial_read_cstr(data, cursor, &name);
    
    if(udt.props & CV_TypeProp_HasUniqueName) {
      cursor += str8_deserial_read_cstr(data, cursor, &unique_name);
    }
  } break;
  
  // dbi/tpi.cpp:1332
  case CV_LeafKind_UDT_SRC_LINE: {
    CV_LeafUDTSrcLine *src_line = str8_deserial_get_raw_ptr(data, 0, sizeof(CV_LeafUDTSrcLine));
    name = str8_struct(&src_line->udt_itype);
  } break;
  case CV_LeafKind_UDT_MOD_SRC_LINE: {
    CV_LeafUDTModSrcLine *mod_src_line = str8_deserial_get_raw_ptr(data, 0, sizeof(CV_LeafUDTModSrcLine));
    name = str8_struct(&mod_src_line->udt_itype);
  } break;
  
  case CV_LeafKind_ALIAS: {
    str8_deserial_read_cstr(data, 0, &name);
  } break;
  
  default: {
    InvalidPath;
  } break;
  }
  
  CV_UDTInfo info  = {0};
  info.name        = name;
  info.unique_name = unique_name;
  info.props       = props;
  return info;
}

internal String8
cv_name_from_udt_info(CV_UDTInfo udt_info)
{
  if (udt_info.props & CV_TypeProp_HasUniqueName) {
    return udt_info.unique_name;
  }
  return udt_info.name;
}

internal B32
cv_is_udt_name_anon(String8 name)
{
  // corresponds to fUDTAnon from dbi/tm.cpp:817
  B32 is_anon = str8_match(str8_lit("<unnamed-tag>"), name, 0) ||
                str8_match(str8_lit("__unnamed"), name, 0) ||
                str8_match(str8_lit("::<unnamed-tag>"), name, StringMatchFlag_RightSideSloppy) ||
                str8_match(str8_lit("::__unnamed"), name, StringMatchFlag_RightSideSloppy);
  return is_anon;
}

internal B32
cv_is_udt(CV_LeafKind kind)
{
  B32 is_udt = kind == CV_LeafKind_CLASS            ||
               kind == CV_LeafKind_STRUCTURE        || 
               kind == CV_LeafKind_CLASS2           || 
               kind == CV_LeafKind_STRUCT2          || 
               kind == CV_LeafKind_INTERFACE        || 
               kind == CV_LeafKind_UNION            || 
               kind == CV_LeafKind_ENUM             || 
               kind == CV_LeafKind_UDT_MOD_SRC_LINE || 
               kind == CV_LeafKind_UDT_SRC_LINE     || 
               kind == CV_LeafKind_ALIAS;
  return is_udt;
}

internal B32
cv_is_global_symbol(CV_SymKind kind)
{
  B32 is_global_symbol = kind == CV_SymKind_CONSTANT       ||
                         kind == CV_SymKind_GDATA16        ||
                         kind == CV_SymKind_GDATA32_16t    ||
                         kind == CV_SymKind_GDATA32_ST     ||
                         kind == CV_SymKind_GDATA32        ||
                         kind == CV_SymKind_GTHREAD32_16t  ||
                         kind == CV_SymKind_GTHREAD32_ST   ||
                         kind == CV_SymKind_GTHREAD32;
  return is_global_symbol;
}

internal B32
cv_is_typedef(CV_SymKind kind)
{
  B32 is_typedef = kind == CV_SymKind_UDT_16t ||
                   kind == CV_SymKind_UDT_ST  ||
                   kind == CV_SymKind_UDT;
  return is_typedef;
}

internal B32
cv_is_scope_symbol(CV_SymKind kind)
{
  B32 is_scope = kind == CV_SymKind_GPROC32     || 
                 kind == CV_SymKind_LPROC32     || 
                 kind == CV_SymKind_BLOCK32     || 
                 kind == CV_SymKind_THUNK32     || 
                 kind == CV_SymKind_INLINESITE  ||
                 kind == CV_SymKind_INLINESITE2 || 
                 kind == CV_SymKind_WITH32      ||
                 kind == CV_SymKind_SEPCODE     ||
                 kind == CV_SymKind_GPROC32_ID  ||
                 kind == CV_SymKind_LPROC32_ID;
  return is_scope;
}

internal B32
cv_is_end_symbol(CV_SymKind kind)
{
  B32 is_end = kind == CV_SymKind_END         ||
               kind == CV_SymKind_PROC_ID_END ||
               kind == CV_SymKind_INLINESITE_END;
  return is_end;
}

internal B32
cv_is_leaf_type_server(CV_LeafKind kind)
{
  B32 is_type_server = kind == CV_LeafKind_TYPESERVER  ||
                       kind == CV_LeafKind_TYPESERVER2 ||
                       kind == CV_LeafKind_TYPESERVER_ST;
  return is_type_server;
}

internal B32
cv_is_leaf_pch(CV_LeafKind kind)
{
  B32 is_pch = kind == CV_LeafKind_PRECOMP    ||
               kind == CV_LeafKind_PRECOMP_ST ||
               kind == CV_LeafKind_PRECOMP_16t;
  return is_pch;
}

internal CV_ObjInfo
cv_obj_info_from_symbol(CV_Symbol symbol)
{
  CV_ObjInfo result; MemoryZeroStruct(&result);
  switch (symbol.kind) {
  case CV_SymKind_OBJNAME: {
    CV_SymObjName *obj_name = (CV_SymObjName *) symbol.data.str;
    result.sig = obj_name->sig;
    str8_deserial_read_cstr(symbol.data, sizeof(CV_SymObjName), &result.name);
  } break;
  case CV_SymKind_OBJNAME_ST: {
    NotImplemented;
  } break;
  default: {
    InvalidPath;
  } break;
  }
  return result;
}

internal CV_TypeServerInfo
cv_type_server_info_from_leaf(CV_Leaf leaf)
{
  CV_TypeServerInfo result = {0};
  switch (leaf.kind) {
  case CV_LeafKind_TYPESERVER: {
    CV_LeafTypeServer *ts   = (CV_LeafTypeServer *) leaf.data.str;

    result.name      = str8_cstring_capped_reverse(ts + 1, leaf.data.str + leaf.data.size);
    result.sig.data1 = ts->sig;
    result.age       = ts->age;
  } break;
  case CV_LeafKind_TYPESERVER2: {
    CV_LeafTypeServer2 *ts = (CV_LeafTypeServer2 *) leaf.data.str;
    
    Assert(sizeof(result.sig) == sizeof(ts->sig70));
    MemoryCopy(&result.sig, &ts->sig70, sizeof(ts->sig70));
    result.name = str8_cstring_capped_reverse(ts + 1, leaf.data.str + leaf.data.size);
    result.age  = ts->age;
  } break;
  case CV_LeafKind_TYPESERVER_ST: {
    Assert("TODO: LF_TYPESERVER_ST");
  } break;
  default: InvalidPath;
  }
  return result;
}

internal CV_PrecompInfo
cv_precomp_info_from_leaf(CV_Leaf leaf)
{
  CV_PrecompInfo result = {0};
  switch (leaf.kind) {
  case CV_LeafKind_PRECOMP: {
    CV_LeafPreComp *precomp = (CV_LeafPreComp*)leaf.data.str;
    result.start_index = precomp->start_index;
    result.sig         = precomp->sig;
    result.leaf_count  = precomp->count;
    str8_deserial_read_cstr(leaf.data, sizeof(CV_LeafPreComp), &result.obj_name);
  } break;
  case CV_LeafKind_PRECOMP_16t: {
    NotImplemented;
  } break;
  case CV_LeafKind_PRECOMP_ST: {
    NotImplemented;
  } break;
  default: {
    InvalidPath;
  } break;
  }
  return result;
}

internal B32
cv_is_reg_sp(CV_Arch arch, CV_Reg reg)
{
  switch (arch) {
  case CV_Arch_8086: return reg == CV_Regx86_ESP;
  case CV_Arch_X64:  return reg == CV_Regx64_RSP;
  default: NotImplemented;
  }
  return 0;
}

////////////////////////////////
//~ Leaf Helpers

internal U64
cv_compute_leaf_record_size(String8 data, U64 align)
{
  U64 size = 0;
  size += sizeof(CV_LeafSize);
  size += sizeof(CV_LeafKind);
  size += data.size;
  size = AlignPow2(size, align);
  return size;
}

internal U64
cv_serialize_leaf_to_buffer(U8 *buffer, U64 buffer_cursor, U64 buffer_size, CV_LeafKind kind, String8 data, U64 align)
{
  U64 buffer_cursor_start = buffer_cursor;

  // compute record size
  U64 record_size = sizeof(kind) + data.size;
  Assert(record_size <= CV_LeafSize_Max);
  CV_LeafSize record_size16 = (CV_LeafSize)record_size;

  // compute pad
  static U8 LEAF_PAD_ARR[] = { 0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff };
  U64 pad_size = AlignPadPow2(data.size, align);
  Assert(pad_size <= ArrayCount(LEAF_PAD_ARR));

  // write header
  CV_LeafHeader *header_ptr = (CV_LeafHeader *)(buffer + buffer_cursor);
  header_ptr->size = record_size16;
  header_ptr->kind = kind;
  buffer_cursor += sizeof(*header_ptr);

  // write body
  U8 *leaf_data_ptr = buffer + buffer_cursor;
  MemoryCopy(leaf_data_ptr, data.str, data.size);
  buffer_cursor += data.size;

  // write pad
  U8 *pad_data_ptr = buffer + buffer_cursor;
  MemoryCopy(pad_data_ptr, &LEAF_PAD_ARR[0], pad_size);
  buffer_cursor += pad_size;

  U64 write_size = buffer_cursor - buffer_cursor_start;
  return write_size;
}

internal String8
cv_serialize_leaf_ex(Arena *arena, CV_LeafKind kind, String8 data, U64 align)
{
  U64      buffer_size = cv_compute_leaf_record_size(data, align);
  U8      *buffer      = push_array_no_zero(arena, U8, buffer_size);
  U64      size        = cv_serialize_leaf_to_buffer(buffer, 0, buffer_size, kind, data, align);
  String8  raw_leaf    = str8(buffer, size);
  return raw_leaf;
}

internal String8
cv_serialize_leaf(Arena *arena, CV_Leaf *leaf, U64 align)
{
  return cv_serialize_leaf_ex(arena, leaf->kind, leaf->data, align);
}

internal CV_Leaf
cv_make_leaf(Arena *arena, CV_LeafKind kind, String8 data)
{
  CV_Leaf result = {0};
  String8 raw_leaf = cv_serialize_leaf_ex(arena, kind, data, 1);
  cv_deserial_leaf(raw_leaf, 0, 1, &result);
  return result;
}

internal U64
cv_deserial_leaf(String8 raw_data, U64 off, U64 align, CV_Leaf *leaf_out)
{
  // do we have enough bytes to read header?
  Assert(raw_data.size >= sizeof(CV_LeafHeader));

  CV_LeafHeader *header = (CV_LeafHeader*)(raw_data.str + off);

  // leaf size must have enough bytes for the kind enum
  Assert(header->size >= sizeof(CV_LeafKind));

  // do we have enough bytes to read leaf data?
  Assert(sizeof(CV_LeafSize) + header->size <= raw_data.size);

  // fill out leaf
  leaf_out->kind = header->kind;
  leaf_out->data = str8(raw_data.str + sizeof(CV_LeafHeader), header->size - sizeof(CV_LeafKind));

  U64 leaf_size = AlignPow2(sizeof(CV_LeafHeader) + leaf_out->data.size, align);
  Assert(leaf_size <= raw_data.size);
  return leaf_size;
}

internal CV_Leaf
cv_leaf_from_string(String8 raw_data)
{
  CV_Leaf result;
  cv_deserial_leaf(raw_data, 0, 1, &result);
  return result;
}

////////////////////////////////
//~ Symbol Helpers

internal U64
cv_compute_symbol_record_size(CV_Symbol *symbol, U64 align)
{
  U64 size = 0;
  size += sizeof(CV_SymSize);
  size += sizeof(CV_SymKind);
  size += AlignPow2(symbol->data.size, align);
  return size;
}

internal U64
cv_serialize_symbol_to_buffer(U8 *buffer, U64 buffer_cursor, U64 buffer_size, CV_Symbol *symbol, U64 align)
{
  U64 write_size = cv_compute_symbol_record_size(symbol, align);
  Assert(buffer_cursor + write_size <= buffer_size);

  U64 record_size = 0;
  record_size += sizeof(symbol->kind);
  record_size += AlignPow2(symbol->data.size, align);
  
  Assert(record_size <= CV_SymSize_Max);
  CV_SymSize record_size16 = (CV_SymSize)record_size;

  // init header
  CV_SymbolHeader *header = (CV_SymbolHeader *)(buffer + buffer_cursor);
  header->size = record_size16;
  header->kind = symbol->kind;

  // copy symbol data
  U8 *data_dst = (U8 *)(header + 1);
  MemoryCopy(data_dst, symbol->data.str, symbol->data.size);

  // set pad bytes
  U64 pad_size = AlignPadPow2(symbol->data.size, align);
  U8 *pad_dst = data_dst + symbol->data.size;
  MemorySet(&pad_dst[0], 0, pad_size);

  return write_size;
}

internal String8
cv_serialize_symbol(Arena *arena, CV_Symbol *symbol, U64 align)
{
  U64 buffer_size = cv_compute_symbol_record_size(symbol, align);
  U8 *buffer = push_array(arena, U8, buffer_size);
  cv_serialize_symbol_to_buffer(buffer, 0, buffer_size, symbol, align);
  String8 result = str8(buffer, buffer_size);
  return result;
}

internal String8
cv_make_symbol(Arena *arena, CV_SymKind kind, String8 data)
{
  ProfBeginFunction();
  Temp scratch = scratch_begin(&arena, 1);
  AssertAlways((data.size + sizeof(kind)) <= CV_SymSize_Max);
  CV_SymSize symbol_size = (CV_SymSize)data.size + sizeof(kind);
  String8List srl = {0};
  str8_serial_begin(scratch.arena, &srl);
  str8_serial_push_struct(scratch.arena, &srl, &symbol_size);
  str8_serial_push_struct(scratch.arena, &srl, &kind);
  str8_serial_push_string(scratch.arena, &srl, data);
  String8 symbol = str8_serial_end(arena, &srl);
  scratch_end(scratch);
  ProfEnd();
  return symbol;
}

internal String8
cv_make_obj_name(Arena *arena, String8 obj_path, U32 sig)
{
  ProfBeginFunction();
  Temp scratch = scratch_begin(&arena, 1);
  
  CV_SymObjName obj = {0};
  obj.sig = sig;
  
  String8List serial = {0};
  str8_serial_begin(scratch.arena, &serial);
  str8_serial_push_struct(scratch.arena, &serial, &obj);
  str8_serial_push_cstr(scratch.arena, &serial, obj_path);
  String8 result = str8_serial_end(arena, &serial);
  
  scratch_end(scratch);
  ProfEnd();
  return result;
}

internal String8
cv_make_comp3(Arena *arena,
              CV_Compile3Flags flags, CV_Language lang, CV_Arch arch, 
              U16 ver_fe_major, U16 ver_fe_minor, U16 ver_fe_build, U16 ver_feqfe,
              U16 ver_major, U16 ver_minor, U16 ver_build, U16 ver_qfe,
              String8 version_string)
{
  ProfBeginFunction();
  Temp scratch = scratch_begin(&arena, 1);
  
  CV_SymCompile3 comp = {0};
  comp.flags          = flags | lang;
  comp.machine        = arch;
  comp.ver_fe_major   = ver_fe_major;
  comp.ver_fe_minor   = ver_fe_minor;
  comp.ver_fe_build   = ver_fe_build;
  comp.ver_feqfe      = ver_feqfe;
  comp.ver_major      = ver_major;
  comp.ver_minor      = ver_minor;
  comp.ver_build      = ver_build;
  comp.ver_qfe        = ver_qfe;
  
  String8List serial = {0};
  str8_serial_begin(scratch.arena, &serial);
  str8_serial_push_struct(scratch.arena, &serial, &comp);
  str8_serial_push_cstr(scratch.arena, &serial, version_string);
  String8 result = str8_serial_end(arena, &serial);
  
  scratch_end(scratch);
  ProfEnd();
  return result;
}

internal String8
cv_make_envblock(Arena *arena, String8List string_list)
{
  ProfBeginFunction();
  Temp scratch = scratch_begin(&arena, 1);
  String8List serial = {0};
  str8_serial_begin(scratch.arena, &serial);
  CV_SymEnvBlock envblock = {0};
  str8_serial_push_struct(scratch.arena, &serial, &envblock);
  for (String8Node *n = string_list.first; n != NULL; n = n->next) {
    str8_serial_push_cstr(scratch.arena, &serial, n->string);
  }
  String8 result = str8_serial_end(arena, &serial);
  scratch_end(scratch);
  ProfEnd();
  return result;
}

internal CV_Symbol
cv_make_proc_ref(Arena *arena, CV_ModIndex imod, U32 stream_offset, String8 name, B32 is_local)
{
  U64 buffer_size = sizeof(CV_SymRef2) + name.size + 1;
  U8 *buffer      = push_array_no_zero(arena, U8, buffer_size);
  
  CV_SymRef2 *ref = (CV_SymRef2*)buffer;
  ref->suc_name = 0;
  ref->sym_off  = stream_offset;
  ref->imod     = imod + 1; // MSVC adds one
  
  U8 *name_ptr = (U8*)(ref + 1);
  MemoryCopy(name_ptr, name.str, name.size);
  name_ptr[name.size] = '\0';
  
  CV_Symbol symbol;
  symbol.kind   = is_local ? CV_SymKind_LPROCREF : CV_SymKind_PROCREF;
  symbol.data   = str8(buffer, buffer_size);
  symbol.offset = max_U64;
  
  return symbol;
}

internal CV_Symbol
cv_make_pub32(Arena *arena, CV_Pub32Flags flags, U32 off, U16 isect, String8 name)
{
  U64 buffer_size = sizeof(CV_SymPub32) + name.size + 1;
  U8 *buffer      = push_array_no_zero(arena, U8, buffer_size);

  CV_SymPub32 *pub = (CV_SymPub32 *)buffer;
  pub->flags = flags;
  pub->off   = off;
  pub->sec   = isect;
  
  U8 *name_ptr = (U8*)(pub + 1);
  MemoryCopy(name_ptr, name.str, name.size);
  name_ptr[name.size] = '\0';
  
  CV_Symbol symbol;
  symbol.kind = CV_SymKind_PUB32;
  symbol.data = str8(buffer, buffer_size);
  
  return symbol;
}

internal CV_SymbolList
cv_make_proc_refs(Arena *arena, CV_ModIndex imod, CV_SymbolList symbol_list)
{
  CV_SymbolList proc_ref_list = {0};
  for (CV_SymbolNode *symbol_node = symbol_list.first; symbol_node != 0; symbol_node = symbol_node->next) {
    CV_Symbol *symbol = &symbol_node->data;
    if (symbol->kind == CV_SymKind_GPROC32) {
      String8        name          = cv_name_from_symbol(symbol->kind, symbol->data);
      CV_Symbol      ref           = cv_make_proc_ref(arena, imod, safe_cast_u32(symbol->offset), name, /* is_local: */ 0);
      CV_SymbolNode *proc_ref_node = cv_symbol_list_push(arena, &proc_ref_list);
      proc_ref_node->data = ref;
    } else if (symbol->kind == CV_SymKind_LPROC32) {
      String8        name          = cv_name_from_symbol(symbol->kind, symbol->data);
      CV_Symbol      ref           = cv_make_proc_ref(arena, imod, safe_cast_u32(symbol->offset), name, /* is_local */ 1);
      CV_SymbolNode *proc_ref_node = cv_symbol_list_push(arena, &proc_ref_list);
      proc_ref_node->data = ref;
    }
  }
  return proc_ref_list;
}

////////////////////////////////
//~ .debug$S helpers

internal void
cv_parse_debug_s_c13_(Arena *arena, CV_DebugS *debug_s, String8 raw_debug_s)
{
  for (U64 cursor = 0; cursor + sizeof(CV_C13SubSectionHeader) <= raw_debug_s.size; ) {
    // read header
    CV_C13SubSectionHeader header = {0};
    cursor += str8_deserial_read_struct(raw_debug_s, cursor, &header);

    if (~header.kind & CV_C13SubSectionKind_IgnoreFlag) {
      // pick sub-section list
      U64          sub_sect_idx  = cv_c13_sub_section_idx_from_kind(header.kind);
      String8List *sub_sect_list = debug_s->data_list + sub_sect_idx;

      // push data to sub-section
      Rng1U64 sub_sect_range = r1u64(cursor, cursor + header.size);
      String8 sub_sect_data  = str8_substr(raw_debug_s, sub_sect_range);
      str8_list_push(arena, sub_sect_list, sub_sect_data);
    }

    // advance
    cursor += header.size;
    cursor = AlignPow2(cursor, CV_C13SubSectionAlign);
  }
}

internal CV_DebugS
cv_parse_debug_s_c13(Arena *arena, String8 raw_debug_s)
{
  CV_DebugS debug_s = {0};
  cv_parse_debug_s_c13_(arena, &debug_s, raw_debug_s);
  return debug_s;
}

internal CV_DebugS
cv_parse_debug_s_c13_list(Arena *arena, String8List raw_debug_s)
{
  CV_DebugS debug_s = {0};
  for (String8Node *node = raw_debug_s.first; node != 0; node = node->next) {
    cv_parse_debug_s_c13_(arena, &debug_s, node->string);
  }
  return debug_s;
}

internal CV_DebugS 
cv_parse_debug_s(Arena *arena, String8 raw_debug_s)
{
  CV_DebugS result; MemoryZeroStruct(&result);
  if (raw_debug_s.size >= sizeof(CV_Signature)) {
    CV_Signature sig = *(CV_Signature *)raw_debug_s.str;
    switch (sig) {
    case CV_Signature_C13: {
      String8 raw_debug_s_past_sig = str8_substr(raw_debug_s, r1u64(sizeof(sig), raw_debug_s.size));
      result = cv_parse_debug_s_c13(arena, raw_debug_s_past_sig);
    } break;
    case CV_Signature_C6: {
      Assert(!"TODO: handle C6");
    } break;
    case CV_Signature_C7: {
      Assert(!"TODO: handle C7");
    } break;
    case CV_Signature_C11: {
      Assert(!"TODO: handle C11");
    } break;
    default: Assert(!"invalid signature"); break;
    }
  }
  return result;
}

internal void
cv_debug_s_concat_in_place(CV_DebugS *dst, CV_DebugS *src)
{
  for (U64 sub_sect_idx = 0; sub_sect_idx < ArrayCount(dst->data_list); sub_sect_idx += 1) {
    str8_list_concat_in_place(&dst->data_list[sub_sect_idx], &src->data_list[sub_sect_idx]);
  }
}

internal String8List
cv_data_c13_from_debug_s(Arena *arena, CV_DebugS *debug_s, B32 write_sig)
{
  String8List srl = {0};
  str8_serial_begin(arena, &srl);
  
  if (write_sig) {
    CV_Signature sig = CV_Signature_C13;
    str8_serial_push_struct(arena, &srl, &sig);
  }
  
  static CV_C13SubSectionKind layout_arr[] = {
    CV_C13SubSectionKind_Symbols,
    //CV_C13SubSectionKind_Lines,
    CV_C13SubSectionKind_FileChksms,
    CV_C13SubSectionKind_FrameData,
    CV_C13SubSectionKind_InlineeLines,
    CV_C13SubSectionKind_IlLines,
    CV_C13SubSectionKind_CrossScopeImports,
    CV_C13SubSectionKind_CrossScopeExports,
    CV_C13SubSectionKind_FuncMDTokenMap,
    CV_C13SubSectionKind_TypeMDTokenMap,
    CV_C13SubSectionKind_MergedAssemblyInput,
    CV_C13SubSectionKind_CoffSymbolRVA,
    CV_C13SubSectionKind_XfgHashType,
    CV_C13SubSectionKind_XfgHashVirtual,
  };
  
  for (U64 layout_idx = 0; layout_idx < ArrayCount(layout_arr); layout_idx += 1) {
    CV_C13SubSectionKind kind = layout_arr[layout_idx];
    String8List *data = cv_sub_section_ptr_from_debug_s(debug_s, kind);
    if (data->total_size > 0) {
      U32 size32 = safe_cast_u32(data->total_size);
      str8_serial_push_u32(arena, &srl, kind);
      str8_serial_push_u32(arena, &srl, size32);
      str8_serial_push_data_list(arena, &srl, data->first);
      str8_serial_push_align(arena, &srl, 4);
    }
  }
  
  String8List *line_data = cv_sub_section_ptr_from_debug_s(debug_s, CV_C13SubSectionKind_Lines);
  for (String8Node *line_node = line_data->first; line_node != 0; line_node = line_node->next) {
    str8_serial_push_u32(arena, &srl, CV_C13SubSectionKind_Lines);
    str8_serial_push_u32(arena, &srl, safe_cast_u32(line_node->string.size));
    str8_serial_push_string(arena, &srl, line_node->string);
    str8_serial_push_align(arena, &srl, 4);
  }
  
  return srl;
}

internal CV_C13SubSectionIdxKind
cv_c13_sub_section_idx_from_kind(CV_C13SubSectionKind kind)
{
  switch (kind) {
#define X(n,c) case CV_C13SubSectionKind_##n: return CV_C13SubSectionIdxKind_##n;
    CV_C13SubSectionKindXList(X)
#undef X
  }
  return CV_C13SubSectionIdxKind_NULL;
} 

internal String8List *
cv_sub_section_ptr_from_debug_s(CV_DebugS *debug_s, CV_C13SubSectionKind kind)
{
  CV_C13SubSectionIdxKind idx = cv_c13_sub_section_idx_from_kind(kind);
  return &debug_s->data_list[idx];
}

internal String8List
cv_sub_section_from_debug_s(CV_DebugS debug_s, CV_C13SubSectionKind kind)
{
  String8List *list_ptr = cv_sub_section_ptr_from_debug_s(&debug_s, kind);
  return *list_ptr;
}

internal String8
cv_string_table_from_debug_s(CV_DebugS debug_s)
{
  String8List data_list = cv_sub_section_from_debug_s(debug_s, CV_C13SubSectionKind_StringTable);
  String8 string_data = str8(0,0);
  if (data_list.node_count > 0) {
    string_data = data_list.first->string;
  }
  return string_data;
}

internal String8
cv_file_chksms_from_debug_s(CV_DebugS debug_s)
{
  String8List data_list = cv_sub_section_from_debug_s(debug_s, CV_C13SubSectionKind_FileChksms);
  String8 file_chksms = str8(0,0);
  if (data_list.node_count > 0) {
    file_chksms = data_list.first->string;
  }
  return file_chksms;
}

////////////////////////////////
//~ String Table Deduper

internal U64
cv_string_hash_table_hash(String8 string)
{
  return hash_from_str8(string);
}

internal int
cv_string_bucket_is_before(void *raw_a, void *raw_b)
{
  CV_StringBucket **a = raw_a;
  CV_StringBucket **b = raw_b;

  int is_before;

  if ((*a)->u.idx0 == (*b)->u.idx0) {
    is_before = (*a)->u.idx1 < (*b)->u.idx1;
  } else {
    is_before = (*a)->u.idx0 < (*b)->u.idx0;
  }

  return is_before;
}

internal CV_StringBucket *
cv_string_hash_table_insert_or_update(CV_StringBucket **buckets, U64 cap, U64 hash, CV_StringBucket *new_bucket)
{
  CV_StringBucket *result                         = 0;
  B32              was_bucket_inserted_or_updated = 0;

  U64 best_idx = hash % cap;
  U64 idx      = best_idx;

  do {
    retry:;
    CV_StringBucket *curr_bucket = buckets[idx];

    if (curr_bucket == 0) {
      CV_StringBucket *compare_bucket = ins_atomic_ptr_eval_cond_assign(&buckets[idx], new_bucket, curr_bucket);

      if (compare_bucket == curr_bucket) {
        // success, bucket was inserted
        was_bucket_inserted_or_updated = 1;
        break;
      }

      // another thread took the bucket...
      goto retry;
    } else if (str8_match(curr_bucket->string, new_bucket->string, 0)) {
      if (cv_string_bucket_is_before(&curr_bucket, &new_bucket)) {
        // recycle bucket
        result = new_bucket;

        // don't need to update, more recent leaf is in the bucket
        was_bucket_inserted_or_updated = 1;

        break;
      }

      CV_StringBucket *compare_bucket = ins_atomic_ptr_eval_cond_assign(&buckets[idx], new_bucket, curr_bucket);

      if (compare_bucket == curr_bucket) {

        // recycle bucket
        result = compare_bucket;

        // new bucket is in the hash table, exit
        was_bucket_inserted_or_updated = 1;
        break;
      }

      // another thread took the bucket...
      goto retry;
    }

    // advance
    idx = (idx + 1) % cap;
  } while (idx != best_idx);

  // are there enough free buckets?
  Assert(was_bucket_inserted_or_updated);

  return result;
}

internal
THREAD_POOL_TASK_FUNC(cv_count_strings_in_debug_s_arr_task)
{
  ProfBeginFunction();
  CV_DedupStringTablesTask *task          = raw_task;
  CV_StringTableRange      *range_list    = task->range_lists[task_id];

  for (CV_StringTableRange *range_n = range_list; range_n != 0; range_n = range_n->next) {
    CV_DebugS debug_s       = task->arr[range_n->debug_s_idx];
    String8   string_buffer = cv_string_table_from_debug_s(debug_s);

    Assert(range_n->range.min <= range_n->range.max);
    Assert(range_n->range.min <= string_buffer.size);
    Assert(range_n->range.max <= string_buffer.size);

    U64 count = 0;
    for (U64 i = range_n->range.min; i < range_n->range.max; ++i) {
      U8 b = string_buffer.str[i];
      if (b == '\0') {
        count += 1;
      }
    }

    ins_atomic_u64_add_eval(&task->string_counts[range_n->debug_s_idx], count);
  }

  ProfEnd();
}

internal
THREAD_POOL_TASK_FUNC(cv_dedup_strings_in_debug_s_arr_task)
{
  ProfBeginFunction();
  Temp scratch = scratch_begin(&arena, 1);

  U64                       debug_s_idx = task_id;
  CV_DedupStringTablesTask *task        = raw_task;
  CV_DebugS                 debug_s     = task->arr[debug_s_idx];

  String8     string_table = cv_string_table_from_debug_s(debug_s);
  String8List strings_list = str8_split_by_string_chars(scratch.arena, string_table, str8_lit("\0"), 0);

  CV_StringBucket *bucket = 0;

  U64 total_string_size  = 0;
  U64 total_insert_count = 0;

  U64 string_idx = 0;


  for (String8Node *string_n = strings_list.first; string_n != 0; string_n = string_n->next, ++string_idx) {
    if (bucket == 0) {
      bucket = push_array_no_zero(arena, CV_StringBucket, 1);
    }

    bucket->u.idx0 = debug_s_idx;
    bucket->u.idx1 = string_idx;
    bucket->string = string_n->string;

    U64              hash             = cv_string_hash_table_hash(string_n->string);
    CV_StringBucket *insert_or_update = cv_string_hash_table_insert_or_update(task->buckets, task->bucket_cap, hash, bucket);

    if (insert_or_update == 0) {
      total_string_size  += string_n->string.size;
      total_insert_count += 1;
    }

    if (insert_or_update != bucket) {
      bucket = 0;
    }
  }

  ins_atomic_u64_add_eval(&task->total_string_size, total_string_size);
  ins_atomic_u64_add_eval(&task->total_insert_count, total_insert_count);

  scratch_end(scratch);
  ProfEnd();
}

internal CV_StringHashTable
cv_dedup_string_tables(TP_Arena *arena, TP_Context *tp, U64 count, CV_DebugS *arr)
{
  ProfBeginFunction();
  Temp scratch = scratch_begin(arena->v, arena->count);

  ProfBegin("Compute Total Weight");
  U64 total_weight = 0;
  for (U64 i = 0; i < count; ++i) {
    String8 string_table = cv_string_table_from_debug_s(arr[i]);
    total_weight += string_table.size;
  }
  ProfEnd();

  U64                   per_task_weight = CeilIntegerDiv(total_weight, tp->worker_count);
  U64                   task_weight     = 0;
  U64                   task_id         = 0;
  CV_StringTableRange **range_lists     = push_array(scratch.arena, CV_StringTableRange *, tp->worker_count);

  ProfBegin("Divide Work");
  for (U64 debug_s_idx = 0; debug_s_idx < count; ++debug_s_idx) {
    String8 string_table = cv_string_table_from_debug_s(arr[debug_s_idx]);

    for (U64 cursor = 0; cursor < string_table.size; cursor += per_task_weight) {
      if (task_weight >= per_task_weight) {
        task_id     = (task_id + 1) % tp->worker_count;
        task_weight = 0;
      }

      U64 max_range_weight = Min(per_task_weight, string_table.size - cursor);

      CV_StringTableRange *node = push_array(scratch.arena, CV_StringTableRange, 1);
      node->range               = rng_1u64(cursor, cursor + max_range_weight);
      node->debug_s_idx         = debug_s_idx;

      SLLStackPush(range_lists[task_id], node);
      task_weight += max_range_weight;
    }
  }
  ProfEnd();

  ProfBegin("Count");
  CV_DedupStringTablesTask task = {0};
  task.arr                     = arr;
  task.range_lists             = range_lists;
  task.string_counts           = push_array(scratch.arena, U64, count);
  tp_for_parallel(tp, 0, tp->worker_count, cv_count_strings_in_debug_s_arr_task, &task);
  ProfEnd();

  ProfBegin("Dedup");
  U64 total_string_count = sum_array_u64(count, task.string_counts);
  task.bucket_cap = (U64)((F64)total_string_count * 1.3);
  task.buckets    = push_array(arena->v[0], CV_StringBucket *, task.bucket_cap);
  tp_for_parallel(tp, arena, count, cv_dedup_strings_in_debug_s_arr_task, &task);
  ProfEnd();

  CV_StringHashTable string_ht = {0};
  string_ht.total_string_size  = task.total_string_size;
  string_ht.total_insert_count = task.total_insert_count;
  string_ht.bucket_cap         = task.bucket_cap;
  string_ht.buckets            = task.buckets;

  scratch_end(scratch);
  ProfEnd();
  return string_ht;
}

internal void
cv_string_hash_table_assign_buffer_offsets(TP_Context *tp, CV_StringHashTable string_ht)
{
  ProfBeginFunction();
  Temp scratch = scratch_begin(0,0);

  ProfBegin("Count Strings");
  U64 string_count = 0;
  for (U64 i = 0; i < string_ht.bucket_cap; ++i) {
    if (string_ht.buckets[i] != 0) {
      string_count += 1;
    }
  }
  ProfEnd();

  ProfBegin("Push");
  CV_StringBucket **strings = push_array_no_zero(scratch.arena, CV_StringBucket *, string_count);
  ProfEnd();

  ProfBegin("Copy Present Buckets");
  for (U64 i = 0, string_idx = 0; i < string_ht.bucket_cap; ++i) {
    if (string_ht.buckets[i] != 0) {
      strings[string_idx++] = string_ht.buckets[i];
    }
  }
  ProfEnd();

  ProfBegin("Sort");
  radsort(strings, string_count, cv_string_bucket_is_before);
  ProfEnd();

  ProfBegin("Assign Offsets");
  for (U64 i = 0, offset_cursor = 0; i < string_count; ++i) {
    CV_StringBucket *s = strings[i];
    s->u.offset = offset_cursor;
    offset_cursor += s->string.size + 1;
  }
  ProfEnd();

  scratch_end(scratch);
  ProfEnd();
}

internal CV_StringBucket *
cv_string_hash_table_lookup(CV_StringHashTable ht, String8 string)
{
  U64 hash     = cv_string_hash_table_hash(string);
  U64 best_idx = hash % ht.bucket_cap;
  U64 idx      = best_idx;

  do {
    if (ht.buckets[idx] == 0) {
      break;
    }

    if (str8_match(ht.buckets[idx]->string, string, 0)) {
      return ht.buckets[idx];
    }

    idx = (idx + 1 % ht.bucket_cap);
  } while (idx != best_idx);

  return 0;
}

internal
THREAD_POOL_TASK_FUNC(cv_pack_string_hash_table_task)
{
  ProfBeginFunction();
  CV_PackStringHashTableTask *task  = raw_task;
  Rng1U64                     range = task->ranges[task_id];
  for (U64 bucket_idx = range.min; bucket_idx < range.max; ++bucket_idx) {
    CV_StringBucket *bucket = task->buckets[bucket_idx];
    if (bucket) {
      MemoryCopy(task->buffer + bucket->u.offset, bucket->string.str, bucket->string.size);
      task->buffer[bucket->u.offset + bucket->string.size] = '\0';
    }
  }
  ProfEnd();
}

internal String8
cv_pack_string_hash_table(Arena *arena, TP_Context *tp, CV_StringHashTable string_ht)
{
  ProfBeginFunction();
  Temp scratch = scratch_begin(&arena, 1);

  U64  buffer_size = string_ht.total_string_size + /* nulls: */ string_ht.total_insert_count;
  U8  *buffer      = push_array_no_zero(arena, U8, buffer_size);

  CV_PackStringHashTableTask task = {0};
  task.buckets                    = string_ht.buckets;
  task.buffer                     = buffer;
  task.ranges                     = tp_divide_work(scratch.arena, string_ht.bucket_cap, tp->worker_count);
  tp_for_parallel(tp, 0, tp->worker_count, cv_pack_string_hash_table_task, &task);

  String8 result = str8(buffer, buffer_size);
  scratch_end(scratch);
  ProfEnd();
  return result;
}

////////////////////////////////
//~ Symbol Deduper

internal int
cv_symbol_deduper_is_before(void *raw_a, void *raw_b)
{
  return raw_a < raw_b;
}

internal CV_SymbolNode **
cv_symbol_deduper_insert_or_update(CV_SymbolNode ***buckets, U64 cap, U64 hash, CV_SymbolNode **new_bucket)
{
  CV_SymbolNode **result                 = 0;
  B32             is_inserted_or_updated = 0;

  U64 best_idx = hash % cap;
  U64 idx      = best_idx;

  do {
    retry:;
    CV_SymbolNode **curr_bucket = buckets[idx];

    Assert(curr_bucket != new_bucket);

    if (curr_bucket == 0) {
      CV_SymbolNode **compare_bucket = ins_atomic_ptr_eval_cond_assign(&buckets[idx], new_bucket, curr_bucket);

      if (compare_bucket == curr_bucket) {
        // success, bucket was inserted
        is_inserted_or_updated = 1;
        break;
      }

      // another thread took the bucket...
      goto retry;
    } else if ((*curr_bucket)->data.kind == (*new_bucket)->data.kind &&
               (*curr_bucket)->data.data.size == (*new_bucket)->data.data.size &&
               MemoryMatch((*curr_bucket)->data.data.str, (*new_bucket)->data.data.str, (*new_bucket)->data.data.size)) {
      if (cv_symbol_deduper_is_before(curr_bucket, new_bucket)) {
        result = new_bucket;

        is_inserted_or_updated = 1;

        // don't need to update, more recent leaf is in the bucket
        break;
      }

      CV_SymbolNode **compare_bucket = ins_atomic_ptr_eval_cond_assign(&buckets[idx], new_bucket, curr_bucket);
      if (compare_bucket == curr_bucket) {
        result = compare_bucket;

        is_inserted_or_updated = 1;
        break;
      }

      // another thread took the bucket...
      goto retry;
    }

    // advance
    idx = (idx + 1) % cap;
  } while (idx != best_idx);

  Assert(is_inserted_or_updated);

  return result;
}

internal
THREAD_POOL_TASK_FUNC(cv_symbol_deduper_insert_task)
{
  ProfBeginFunction();
  CV_SymbolDeduperTask *task  = raw_task;
  Rng1U64               range = task->ranges[task_id];
  for (U64 symbol_idx = range.min; symbol_idx < range.max; ++symbol_idx) {
    CV_SymbolNode **symbol_node = &task->symbols[symbol_idx];
    U64             hash        = hash_from_cv_symbol(&(*symbol_node)->data);
    cv_symbol_deduper_insert_or_update(task->u.buckets, task->cap, hash, symbol_node);
  }
  ProfEnd();
}

internal
THREAD_POOL_TASK_FUNC(cv_symbol_deduper_deref_buckets_task)
{
  ProfBeginFunction();
  CV_SymbolDeduperTask *task  = raw_task;
  Rng1U64               range = task->ranges[task_id];
  for (U64 bucket_idx = range.min; bucket_idx < range.max; ++bucket_idx) {
    CV_SymbolNode **bucket = task->u.buckets[bucket_idx];
    if (bucket) {
      task->u.deref_buckets[bucket_idx] = *bucket;
    }
  }
  ProfEnd();
}

internal void
cv_dedup_symbol_ptr_array(TP_Context *tp, CV_SymbolPtrArray *symbols)
{
  ProfBeginDynamic("Dedup Symbols [Count %llu]", symbols->count);
  Temp scratch = scratch_begin(0, 0);

  ProfBegin("Setup Task");
  CV_SymbolDeduperTask task = {0};
  task.symbols              = symbols->v;
  task.cap                  = (U64)((F64)symbols->count * 1.3);
  task.u.buckets            = push_array(scratch.arena, CV_SymbolNode **, task.cap);
  ProfEnd();

  ProfBegin("Dedup");
  task.ranges = tp_divide_work(scratch.arena, symbols->count, tp->worker_count);
  tp_for_parallel(tp, 0, tp->worker_count, cv_symbol_deduper_insert_task, &task);
  ProfEnd();

  ProfBegin("Deref Buckets");
  task.ranges = tp_divide_work(scratch.arena, task.cap, tp->worker_count);
  tp_for_parallel(tp, 0, tp->worker_count, cv_symbol_deduper_deref_buckets_task, &task);
  ProfEnd();

  ProfBegin("Copy Extant Buckets");
  U64 unique_symbol_count = 0;
  for (U64 bucket_idx = 0; bucket_idx < task.cap; ++bucket_idx) {
    CV_SymbolNode *bucket = task.u.deref_buckets[bucket_idx];
    if (bucket) {
      symbols->v[unique_symbol_count++] = bucket;
    }
  }
  ProfEnd();

  Assert(unique_symbol_count < symbols->count);
  symbols->count = unique_symbol_count;

  ProfBeginDynamic("Sort [Count %llu]", symbols->count);
  radsort(symbols->v, symbols->count, cv_symbol_deduper_is_before);
  ProfEnd();

  scratch_end(scratch);
  ProfEnd();
}

////////////////////////////////
//~ .debug$T helpers

internal CV_DebugT
cv_debug_t_from_data_arr(Arena *arena, String8Array data_arr, U64 align)
{
  ProfBegin("Upfront parse");
  U64 max_leaf_count = 0;
  for (U64 data_idx = 0; data_idx < data_arr.count; data_idx += 1) {
    String8 data = data_arr.v[data_idx];
    for (U64 cursor = 0; cursor < data.size; ) {
      CV_Leaf leaf;
      cursor += cv_deserial_leaf(data, cursor, align, &leaf);
      max_leaf_count += 1;
    }
  }
  ProfEnd();

  U8 **leaf_arr   = push_array_no_zero(arena, U8 *, max_leaf_count);
  U64  leaf_count = 0;
  for (U64 data_idx = 0; data_idx < data_arr.count; data_idx += 1) {
    String8 data = data_arr.v[data_idx];

    U64 cursor = 0;
    while (cursor < data.size) {
      CV_Leaf leaf;
      U64 read_size = cv_deserial_leaf(data, cursor, align, &leaf);

      Assert(leaf_count < max_leaf_count);
      leaf_arr[leaf_count] = str8_deserial_get_raw_ptr(data, cursor, read_size);
      leaf_count += 1;

      // advance cursor
      cursor += read_size;
    }
  }

  CV_DebugT debug_t = {0};
  debug_t.count     = leaf_count;
  debug_t.v         = leaf_arr;
  return debug_t;
}

internal CV_DebugT
cv_debug_t_from_data(Arena *arena, String8 data, U64 align)
{
  String8Array arr = {0};
  arr.count        = 1;
  arr.v            = &data;
  return cv_debug_t_from_data_arr(arena, arr, align);
}

internal CV_Leaf
cv_debug_t_get_leaf(CV_DebugT debug_t, U64 leaf_idx)
{
  Assert(leaf_idx < debug_t.count);

  U8 *ptr = debug_t.v[leaf_idx];
  String8 data = str8(ptr, max_U64);

  CV_Leaf leaf;
  cv_deserial_leaf(data, 0, 1, &leaf);

  U64 size = cv_header_struct_size_from_leaf_kind(leaf.kind);
  Assert(size <= leaf.data.size);

  return leaf;
}

internal String8
cv_debug_t_get_raw_leaf(CV_DebugT debug_t, U64 leaf_idx)
{
  Assert(leaf_idx < debug_t.count);
  U8          *leaf_ptr   = debug_t.v[leaf_idx];
  CV_LeafSize *size_ptr   = (CV_LeafSize *)leaf_ptr;
  CV_LeafSize  total_size = sizeof(*size_ptr) + *size_ptr;
  String8 raw_leaf = str8(leaf_ptr, total_size);
  return raw_leaf;
}

internal CV_LeafHeader *
cv_debug_t_get_leaf_header(CV_DebugT debug_t, U64 leaf_idx)
{
  Assert(leaf_idx < debug_t.count);
  CV_LeafHeader *leaf_header = (CV_LeafHeader *) debug_t.v[leaf_idx];
  return leaf_header;
}

internal B32
cv_debug_t_is_pch(CV_DebugT debug_t)
{
  if (debug_t.count > 0) {
    CV_Leaf leaf = cv_debug_t_get_leaf(debug_t, 0);
    return cv_is_leaf_pch(leaf.kind);
  }
  return 0;
}

internal B32
cv_debug_t_is_type_server(CV_DebugT debug_t)
{
  if (debug_t.count > 0) {
    CV_Leaf leaf = cv_debug_t_get_leaf(debug_t, 0);
    return cv_is_leaf_type_server(leaf.kind);
  }
  return 0;
}

internal U64
cv_debug_t_array_count_leaves(U64 count, CV_DebugT *arr)
{
  U64 total_leaf_count = 0;
  for (U64 i = 0; i < count; i += 1) {
    total_leaf_count += arr[i].count;
  }
  return total_leaf_count;
}

THREAD_POOL_TASK_FUNC(cv_str8_list_from_debug_t_task)
{
  CV_Str8ListFromDebugT *task = raw_task;
  for (U64 leaf_idx = task->ranges[task_id].min; leaf_idx < task->ranges[task_id].max; ++leaf_idx) {
    String8Node *node = &task->nodes[leaf_idx];
    node->string = cv_debug_t_get_raw_leaf(task->debug_t, leaf_idx);
    str8_list_push_node(&task->lists[task_id], node);
  }
}

internal String8List
cv_str8_list_from_debug_t_parallel(TP_Context *tp, Arena *arena, CV_DebugT debug_t)
{
  ProfBeginFunction();
  Temp scratch = scratch_begin(&arena, 1);

  // build lists in parallel
  CV_Str8ListFromDebugT task = {0};
  task.debug_t = debug_t;
  task.ranges  = tp_divide_work(scratch.arena, debug_t.count, tp->worker_count);
  task.lists   = push_array(scratch.arena, String8List, tp->worker_count);
  task.nodes   = push_array_no_zero(arena, String8Node, debug_t.count);
  tp_for_parallel(tp, 0, tp->worker_count, cv_str8_list_from_debug_t_task, &task);

  // concat output lists
  String8List list = {0};
  for (U64 task_id = 0; task_id < tp->worker_count; ++task_id) {
    str8_list_concat_in_place(&list, &task.lists[task_id]);
  }

  scratch_end(scratch);
  ProfEnd();
  return list;
}

// $$Symbols

internal void
cv_parse_symbol_sub_section(Arena *arena, CV_SymbolList *list, U64 offset_base, String8 data, U64 align)
{
  for (U64 cursor = 0, opl = data.size; cursor < opl; ) {
    // read symbol header
    CV_SymbolHeader header;
    cursor += str8_deserial_read_struct(data, cursor, &header);
    
    // size from header has to be larger than 2 bytes
    if (header.size < sizeof(header.kind)) {
      Assert(!"TODO: error handle invalid symbol data");
      break;
    }
    
    // is there enough bytes in the range?
    U64 symbol_opl = cursor + (header.size - sizeof(header.kind));
    if (symbol_opl > opl) {
      Assert(!"TODO: error handle corrupted symbol data");
      break;
    }
    
    // get symbol data
    Rng1U64 symbol_data_range = r1u64(cursor, symbol_opl);
    String8 symbol_data       = str8_substr(data, symbol_data_range);
    
    // init symbol
    CV_SymbolNode *node = cv_symbol_list_push(arena, list);
    node->data.offset   = offset_base + cursor;
    node->data.kind     = header.kind;
    node->data.data     = symbol_data;
    
    // advance cursor
    cursor = symbol_opl;
    cursor = AlignPow2(cursor, align);
  }
}

internal CV_SymbolList
cv_symbol_list_from_data_list(Arena *arena, String8List data_list, U64 align)
{
  CV_SymbolList symbol_list = {0};
  U64 cursor = 0;
  for (String8Node *sect = data_list.first; sect != 0; cursor += sect->string.size, sect = sect->next) {
    cv_parse_symbol_sub_section(arena, &symbol_list, cursor, sect->string, align);
  }
  return symbol_list;
}

internal void
cv_symbol_list_push_node(CV_SymbolList *list, CV_SymbolNode *node)
{
  node->prev = 0;
  node->next = 0;
  DLLPushBack(list->first, list->last, node);
  list->count += 1;
}

internal CV_SymbolNode *
cv_symbol_list_push(Arena *arena, CV_SymbolList *list)
{
  CV_SymbolNode *node = push_array(arena, CV_SymbolNode, 1);
  cv_symbol_list_push_node(list, node);
  return node;
}

internal CV_SymbolNode *
cv_symbol_list_push_data(Arena *arena, CV_SymbolList *list, CV_SymKind kind, String8 data)
{
  CV_SymbolNode *node = cv_symbol_list_push(arena, list);
  node->data.kind = kind;
  node->data.data = data;
  return node;
}

internal CV_SymbolNode *
cv_symbol_list_push_many(Arena *arena, CV_SymbolList *list, U64 count)
{
  CV_SymbolNode *node_arr = push_array_no_zero(arena, CV_SymbolNode, 1);
  for (U64 node_idx = 0; node_idx < count; node_idx += 1) {
    cv_symbol_list_push_node(list, &node_arr[node_idx]);
  }
  return node_arr;
}

internal void
cv_symbol_list_remove_node(CV_SymbolList *list, CV_SymbolNode *node)
{
  Assert(list->count > 0);
  list->count -= 1;
  DLLRemove(list->first, list->last, node);
}

internal void
cv_symbol_list_concat_in_place(CV_SymbolList *list, CV_SymbolList *to_concat)
{
  SLLConcatInPlace(list, to_concat);
}

internal void
cv_symbol_list_concat_in_place_arr(CV_SymbolList *list, U64 count, CV_SymbolList *to_concat)
{
  SLLConcatInPlaceArray(list, to_concat, count);
}

internal U64
cv_symbol_list_arr_get_count(U64 count, CV_SymbolList *list_arr)
{
  U64 result = 0;
  for (U64 idx = 0; idx < count; idx += 1) {
    result += list_arr[idx].count;
  }
  return result;
}

internal String8List
cv_data_from_symbol_list(Arena *arena, CV_SymbolList symbol_list, U64 align)
{
  String8List data_list = {0};
  for (CV_SymbolNode *node = symbol_list.first; node != 0; node = node->next) {
    String8 data = cv_serialize_symbol(arena, &node->data, align);
    str8_list_push(arena, &data_list, data);
  }
  return data_list;
}

internal
THREAD_POOL_TASK_FUNC(cv_symbol_list_syncer)
{
  ProfBeginFunction();

  CV_SymbolListSyncer *task = raw_task;

  // context shortcuts
  Rng1U64 list_range  = task->list_range_arr[task_id];
  U64     symbol_base = task->symbol_base_arr[task_id];

  for (U64 list_idx = list_range.min, symbol_idx = symbol_base; list_idx < list_range.max; list_idx += 1) {
    // pick up assigned list
    CV_SymbolList list = task->list_arr[list_idx];

    // fill out assigned range in the symbol array
    for (CV_SymbolNode *node = list.first; node != 0; node = node->next, symbol_idx += 1) {
      task->symbol_arr[symbol_idx] = node;
    }
  }

  ProfEnd();
}

internal CV_SymbolPtrArray
cv_symbol_ptr_array_from_list(Arena *arena, TP_Context *tp, U64 count, CV_SymbolList *list_arr)
{
  ProfBeginFunction();
  Temp scratch = scratch_begin(&arena, 1);

  U64 total_count = cv_symbol_list_arr_get_count(count, list_arr);

  CV_SymbolListSyncer task = {0};
  task.list_arr            = list_arr;
  task.symbol_arr          = push_array_no_zero(arena, CV_SymbolNode *, total_count);
  task.symbol_base_arr     = push_array_no_zero(scratch.arena, U64, tp->worker_count);
  task.list_range_arr      = tp_divide_work(scratch.arena, count, tp->worker_count);

  for (U64 thread_idx = 0, symbol_base = 0; thread_idx < tp->worker_count; thread_idx += 1) {
    task.symbol_base_arr[thread_idx] = symbol_base;
    Rng1U64 range = task.list_range_arr[thread_idx];
    for (U64 list_idx = range.min; list_idx < range.max; list_idx += 1) {
      symbol_base += list_arr[list_idx].count;
    }
  }

  tp_for_parallel(tp, 0, tp->worker_count, cv_symbol_list_syncer, &task);

  CV_SymbolPtrArray result = {0};
  result.count             = total_count;
  result.v                 = task.symbol_arr;

  scratch_end(scratch);
  ProfEnd();
  return result;
}

internal CV_Scope *
cv_scope_list_push(Arena *arena, CV_ScopeList *list)
{
  CV_Scope *node = push_array(arena, CV_Scope, 1);
  SLLQueuePush(list->first, list->last, node);
  return node;
}

internal CV_SymbolList
cv_global_scope_symbols_from_list(Arena *arena, CV_SymbolList list)
{
  CV_SymbolList gsym_list = {0};
  S64 scope_depth = 0;
  for (CV_SymbolNode *symbol_n = list.first; symbol_n != 0; symbol_n = symbol_n->next) {
    CV_Symbol symbol = symbol_n->data;
    if (cv_is_global_symbol(symbol.kind) && scope_depth == 0) {
      cv_symbol_list_push_data(arena, &gsym_list, symbol.kind, symbol.data);
    } else if (cv_is_scope_symbol(symbol.kind)) {
      scope_depth += 1;
    } else if (cv_is_end_symbol(symbol.kind)) {
      scope_depth -= 1;
      if (scope_depth < 0) {
        break;
      }
    }
  }
  return gsym_list;
}

internal CV_ScopeList
cv_symbol_tree_from_symbol_list(Arena *arena, CV_SymbolList list)
{
  Temp scratch = scratch_begin(&arena, 1);
  
  CV_ScopeList root = {0};
  
  // setup root frame
  CV_ScopeFrame *stack = push_array(scratch.arena, CV_ScopeFrame, 1);
  stack->list = &root;
  
  for (CV_SymbolNode *symbol_node = list.first; symbol_node != 0; symbol_node = symbol_node->next) {
    // store symbol in current scope
    CV_Scope *scope = cv_scope_list_push(arena, stack->list);
    scope->symbol = symbol_node->data;
    
    // does this symbol define a new scope?
    if (cv_is_scope_symbol(symbol_node->data.kind)) {
      CV_ScopeFrame *frame = push_array(scratch.arena, CV_ScopeFrame, 1);
      frame->list = push_array(arena, CV_ScopeList, 1);
      SLLStackPush(stack, frame);
    }
    // does this symbol end current scope?
    else if (cv_is_end_symbol(symbol_node->data.kind)) {
      CV_ScopeFrame *prev_stack_frame = stack->next;
      if (prev_stack_frame) {
        // set children in parent scope
        CV_Scope *parent_scope = prev_stack_frame->list->last;
        parent_scope->children = stack->list;
      }
      
      // pop frame
      SLLStackPop(stack);
    }
  }
  
  scratch_end(scratch);
  return root;
}

internal CV_SymbolList
cv_build_symbol_tree(Arena *arena, CV_ScopeList symbol_tree, U64 symbol_base, U64 align)
{
  Temp scratch = scratch_begin(&arena, 1);
  
  CV_SymbolList result = {0};

  U64 cursor = symbol_base;

  // setup root frame
  CV_ScopeFrame *stack = push_array(scratch.arena, CV_ScopeFrame, 1);
  stack->list = &symbol_tree;
  stack->curr = stack->list->first;
  stack->symbol_off = cursor;

  for (; stack != 0; ) {
    for (; stack->curr != 0; stack->curr = stack->curr->next) {
      CV_Scope  *scope  = stack->curr;
      CV_Symbol *symbol = &scope->symbol;
      
      // store symbol
      CV_SymbolNode *symbol_node = cv_symbol_list_push_data(arena, &result, symbol->kind, symbol->data);
      symbol_node->data.offset = cursor;
      
      // read & advance cursor
      U64 record_size = cv_compute_symbol_record_size(symbol, align);
      cursor += record_size;
      
      if (scope->children) {
        // in every scoped symbol parent and end offsets follow record header
        U32 *parent_off_ptr = (U32 *)symbol->data.str;
        U32 *end_off_ptr    = (U32 *)(parent_off_ptr + 1);
        
        // write parent symbol offset
        U64 parent_off64 = stack->symbol_off;
        U32 parent_off32 = safe_cast_u32(parent_off64);
        *parent_off_ptr = parent_off32;

        // advance to next node so after stack pop we resume from correct node
        stack->curr = stack->curr->next;
        
        // push new scope frame
        CV_ScopeFrame *frame = push_array(scratch.arena, CV_ScopeFrame, 1);
        frame->symbol_off     = symbol_node->data.offset;
        frame->list           = scope->children;
        frame->curr           = frame->list->first;
        frame->parent_off_ptr = parent_off_ptr;
        frame->end_off_ptr    = end_off_ptr;
        SLLStackPush(stack, frame);
        
        break;
      }
    }
    
    if (stack->curr == 0) {
      // write end symbol offset
      if (stack->end_off_ptr) {
        U64 end_symbol_size = sizeof(CV_SymbolHeader);
        U64 end_off64 = cursor - end_symbol_size;
        U32 end_off32 = safe_cast_u32(end_off64);
        *stack->end_off_ptr = end_off32;
      }
      
      // pop scope
      SLLStackPop(stack);
    }
  }
  
  scratch_end(scratch);
  return result;
}

internal U64
cv_patch_symbol_tree_offsets(CV_SymbolList list, U64 base_offset, U64 align)
{
  Temp scratch = scratch_begin(0, 0);

  struct Stack {
    struct Stack *next;
    CV_Symbol    *symbol;
    U64           offset;
  };
  struct Stack *stack     = 0;
  struct Stack *free_list = 0;

  U64 cursor = base_offset;

  for (CV_SymbolNode *symbol_n = list.first; symbol_n != 0; symbol_n = symbol_n->next) {
    CV_Symbol symbol = symbol_n->data;
    if (cv_is_scope_symbol(symbol.kind)) {
      // NOTE: We don't patch 'next' offset in PROC symbols because
      // it's not used by visual studio and MSVC leaves the offsets
      // zeroed. LLD is on the same page.
      Assert(symbol.data.size >= sizeof(U32)*2);

      // patch symbol parent
      if (stack) {
        U32 *parent_off_ptr = (U32 *)symbol.data.str;
        *parent_off_ptr = stack->offset;
      }

      // reuse/alloc frame
      struct Stack *frame;
      if (free_list) {
        frame = free_list;
        SLLStackPop(free_list);
      } else {
        frame = push_array_no_zero(scratch.arena, struct Stack, 1);
      }

      // push frame to the stack
      frame->symbol = &symbol_n->data;
      frame->offset = cursor;
      SLLStackPush(stack, frame);
    } else if (cv_is_end_symbol(symbol.kind)) {
      // patch symbol end
      U32 *end_off_ptr = (U32 *)stack->symbol->data.str + /* skip parent off */ 1;
      *end_off_ptr = cursor;

      // recycle frame
      struct Stack *free_frame = stack;
      SLLStackPop(stack);
      SLLStackPush(free_list, free_frame);
    }

    // advance cursor
    cursor += cv_compute_symbol_record_size(&symbol, align);
  }

  scratch_end(scratch);
  U64 serial_size = cursor - base_offset;
  return serial_size;
}

// $$FileChksms

#if 0
internal String8
cv_c13_file_chksms_from_sub_sections(String8 c13_data, CV_C13Parsed *ss)
{
  ProfBeginFunction();
  String8 file_chksms = str8(0,0);
  CV_C13SubSectionList file_chksms_list = ss->v[CV_C13SubSectionIdxKind_FileChksms];
  if (file_chksms_list.count > 0) {
    Assert(file_chksms_list.count == 1);
    CV_C13SubSectionNode *file_chksms_node = file_chksms_list.first;
    Assert(file_chksms_node->kind == CV_C13SubSectionKind_FileChksms);
    file_chksms = str8_substr(c13_data, file_chksms_node->range);
  }
  ProfEnd();
  return file_chksms;
}
#endif

internal void
cv_parse_checksum_data(Arena *arena, CV_ChecksumList *list, String8 checksum_data)
{
  for (U64 cursor = 0, cursor_opl = checksum_data.size; cursor < cursor_opl; ) {
    U64 expected_cursor_after_checksum = cursor + sizeof(CV_C13Checksum);
    if (expected_cursor_after_checksum > cursor_opl) {
      break;
    }
    CV_C13Checksum *header = (CV_C13Checksum *)str8_deserial_get_raw_ptr(checksum_data, cursor, sizeof(CV_C13Checksum));
    cursor += sizeof(CV_C13Checksum);
    
    U64 expected_cursor_after_value = cursor + header->len;
    if (expected_cursor_after_value > cursor_opl) {
      break;
    }
    String8 value = str8(0,0);
    cursor += str8_deserial_read_block(checksum_data, cursor, header->len, &value);
    cursor = AlignPow2(cursor, 4);
    
    CV_ChecksumNode *node = push_array(arena, CV_ChecksumNode, 1);
    node->next = 0;
    
    CV_Checksum *data = &node->data;
    data->header = header;
    data->value = value;
    
    SLLQueuePush(list->first, list->last, node);
    list->count += 1;
  }
}

internal CV_ChecksumList
cv_c13_parse_checksum_data_list(Arena *arena, String8List checksum_data_list)
{
  CV_ChecksumList result = {0};
  for (String8Node *node = checksum_data_list.first; node != 0; node = node->next) {
    cv_parse_checksum_data(arena, &result, node->string);
  }
  return result;
}

internal void
cv_c13_patch_string_offsets_in_checksum_list(CV_ChecksumList checksum_list, String8 string_data, U64 string_data_base_offset, CV_StringHashTable string_ht)
{
  for (CV_ChecksumNode *node = checksum_list.first; node != 0; node = node->next) {
    CV_Checksum     *checksum = &node->data;
    CV_C13Checksum  *header   = checksum->header;
    String8          name     = str8_cstring_capped(string_data.str + header->name_off, string_data.str + string_data.size);
    CV_StringBucket *bucket   = cv_string_hash_table_lookup(string_ht, name);

    U64 name_off64 = string_data_base_offset + bucket->u.offset;
    header->name_off = safe_cast_u32(name_off64);
  }
}

internal String8List
cv_c13_collect_source_file_names(Arena *arena, CV_ChecksumList checksum_list, String8 string_data)
{
  String8List source_file_name_list = {0};
  for (CV_ChecksumNode *node = checksum_list.first; node != 0; node = node->next) {
    CV_Checksum *checksum = &node->data;
    CV_C13Checksum *header = checksum->header;
    Assert(header->name_off < string_data.size);
    String8 name = str8_cstring_capped(string_data.str + header->name_off, string_data.str + string_data.size);
    str8_list_push(arena, &source_file_name_list, name);
  }
  return source_file_name_list;
}

// $$Lines

internal CV_C13LinesHeaderList
cv_c13_lines_from_sub_sections(Arena *arena, String8 c13_data, Rng1U64 ss_range)
{
  ProfBeginFunction();

  CV_C13LinesHeaderList parsed_line_list = {0};

  String8 sub_sect_data  = str8_substr(c13_data, ss_range);

  for (U64 cursor = 0; cursor + sizeof(CV_C13SubSecLinesHeader) <= sub_sect_data.size; ) {
    CV_C13SubSecLinesHeader *hdr = (CV_C13SubSecLinesHeader *)(sub_sect_data.str + cursor);
    cursor += sizeof(*hdr);

    // read files
    for (; cursor + sizeof(CV_C13File) <= sub_sect_data.size; ) {
      // grab next file header
      CV_C13File *file = (CV_C13File *)(sub_sect_data.str + cursor);
      cursor += sizeof(CV_C13File);

      // parse lines and columns
      //
      // TODO: export columns
      U64  max_line_count = (sub_sect_data.size - cursor) / sizeof(CV_C13Line);
      U32  line_count     = Min(file->num_lines, max_line_count);

      // TODO(allen): check order correctness here

      U64 line_array_off = cursor;
      //U64 col_array_off  = line_array_off + line_count * sizeof(CV_C13Line);

      // compute line entry size
      U64 line_entry_size = sizeof(CV_C13Line);
      if (hdr->flags & CV_C13SubSecLinesFlag_HasColumns) {
        line_entry_size += sizeof(CV_C13Column);
      }

      // advance past line and column entries
      cursor += line_count * line_entry_size;

      // emit parsed lines
      CV_C13LinesHeaderNode *lines_parsed_node = push_array_no_zero(arena, CV_C13LinesHeaderNode, 1);
      lines_parsed_node->next = 0;

      CV_C13LinesHeader *lines_parsed = &lines_parsed_node->v;
      lines_parsed->sec_idx        = hdr->sec;
      lines_parsed->sec_off_lo     = hdr->sec_off;
      lines_parsed->sec_off_hi     = hdr->sec_off + hdr->len;
      lines_parsed->file_off       = file->file_off;
      lines_parsed->line_count     = line_count;
      lines_parsed->col_count      = 0; // TODO: columns
      lines_parsed->line_array_off = ss_range.min + line_array_off;
      lines_parsed->col_array_off  = 0; // TODO: columns

      SLLQueuePush(parsed_line_list.first, parsed_line_list.last, lines_parsed_node);
      parsed_line_list.count += 1;
    }
  }

  ProfEnd();
  return parsed_line_list;
}

internal CV_LineArray
cv_c13_line_array_from_data(Arena *arena, String8 c13_data, U64 sec_base, CV_C13LinesHeader parsed_lines)
{
  CV_LineArray result;
  result.file_off   = parsed_lines.file_off;
  result.line_count = parsed_lines.line_count;
  result.col_count  = parsed_lines.col_count;
  result.voffs      = push_array_no_zero(arena, U64, parsed_lines.line_count + 1);
  result.line_nums  = push_array_no_zero(arena, U32, parsed_lines.line_count);
  result.col_nums   = 0;

  CV_C13Line *raw_lines = (CV_C13Line *)str8_deserial_get_raw_ptr(c13_data, parsed_lines.line_array_off, parsed_lines.line_count * sizeof(raw_lines[0]));

  for(U64 line_idx = 0; line_idx < parsed_lines.line_count; line_idx += 1)
  {
    CV_C13Line line = raw_lines[line_idx];
    result.voffs[line_idx]     = sec_base + parsed_lines.sec_off_lo + line.off;
    result.line_nums[line_idx] = CV_C13LineFlags_ExtractLineNumber(line.flags);
  }

  // emit voff ender
  result.voffs[result.line_count] = sec_base + parsed_lines.sec_off_hi;

  return result;
}

internal void
cv_c13_patch_checksum_offsets_in_line_data_list(String8List line_data, U64 checksum_rebase)
{
  for(String8Node *node = line_data.first; node != 0; node = node->next)
  {
    String8 raw_data = node->string;
    if(raw_data.size < sizeof(CV_C13SubSecLinesHeader))
    {
      Assert(!"unable to patch checksum in line sub seciton header");
      continue;
    }
    CV_C13File *file_header = (CV_C13File *)(raw_data.str + sizeof(CV_C13SubSecLinesHeader));
    U64 rebased_file_off = file_header->file_off + checksum_rebase;
    file_header->file_off = safe_cast_u32(rebased_file_off);
  }
}

// $$InlineeLines

internal CV_C13InlineeLinesParsedList
cv_c13_inlinee_lines_from_sub_sections(Arena *arena, String8List raw_inlinee_lines)
{
  ProfBeginFunction();

  CV_C13InlineeLinesParsedList inlinee_lines_list = {0};

  for (String8Node *raw_data_node = raw_inlinee_lines.first; raw_data_node != 0; raw_data_node = raw_data_node->next) {
    U64 cursor = 0;

    CV_C13InlineeLinesSig sig = 0;
    cursor += str8_deserial_read_struct(raw_data_node->string, cursor, &sig);

    for (; cursor + sizeof(CV_C13InlineeSourceLineHeader) <= raw_data_node->string.size; ) {
      CV_C13InlineeSourceLineHeader *hdr = (CV_C13InlineeSourceLineHeader *)(raw_data_node->string.str + cursor);
      cursor += sizeof(*hdr);

      CV_C13InlineeLinesParsedNode *inlinee_parsed_node = push_array_no_zero(arena, CV_C13InlineeLinesParsedNode, 1);
      inlinee_parsed_node->next = 0;
      SLLQueuePush(inlinee_lines_list.first, inlinee_lines_list.last, inlinee_parsed_node);
      inlinee_lines_list.count += 1;

      CV_C13InlineeLinesParsed *inlinee_parsed = &inlinee_parsed_node->v;
      inlinee_parsed->inlinee          = hdr->inlinee;
      inlinee_parsed->file_off         = hdr->file_off;
      inlinee_parsed->first_source_ln  = hdr->first_source_ln;
      inlinee_parsed->extra_file_count = 0;
      inlinee_parsed->extra_files      = 0;

      if (sig == CV_C13InlineeLinesSig_EXTRA_FILES) {
        if (cursor + sizeof(U32) <= raw_data_node->string.size) {
          U32 *extra_file_count_ptr = (U32 *)(raw_data_node->string.str + cursor);
          cursor += sizeof(*extra_file_count_ptr);

          U32 max_extra_file_count = (raw_data_node->string.size - cursor) / sizeof(U32);
          U32 extra_file_count     = Min(*extra_file_count_ptr, max_extra_file_count);
          U32 *extra_files         = (U32 *)(raw_data_node->string.str + cursor);
          cursor += sizeof(*extra_files) * extra_file_count;

          inlinee_parsed->extra_file_count = extra_file_count;
          inlinee_parsed->extra_files      = extra_files;
        }
      }
    }
  }

  ProfEnd();
  return inlinee_lines_list;
}

// $$FrameData

internal void
cv_c13_patch_checksum_offsets_in_frame_data_list(String8List frame_data, U32 checksum_rebase)
{
  for(String8Node *node = frame_data.first; node != 0; node = node->next)
  {
    String8 raw_data = node->string;
    U64 count = raw_data.size / sizeof(CV_C13FrameData);
    CV_C13FrameData *arr = (CV_C13FrameData *)raw_data.str;
    CV_C13FrameData *ptr = arr;
    CV_C13FrameData *opl = arr + count;
    for(; ptr < opl; ptr += 1)
    {
      U64 rebased_frame_func = ptr->frame_func + checksum_rebase;
      ptr->frame_func = safe_cast_u32(rebased_frame_func);
    }
  }
}

////////////////////////////////
// $$Lines Accel

int
cv_c13_voff_map_compar(const void *raw_a, const void *raw_b)
{
  CV_Line *a = (CV_Line*)raw_a;
  CV_Line *b = (CV_Line*)raw_b;
  int cmp = a->voff < b->voff ? -1 :
            a->voff > b->voff ? +1 :
            0;
  return cmp;
}

internal CV_LinesAccel *
cv_c13_make_lines_accel(Arena *arena, U64 lines_count, CV_LineArray *lines)
{
  ProfBeginFunction();

  U64 total_voff_count = 0;
  for(U64 arr_idx = 0; arr_idx < lines_count; arr_idx += 1) {
    total_voff_count += lines[arr_idx].line_count + 1;
  }

  CV_Line *map      = push_array_no_zero(arena, CV_Line, total_voff_count);
  U64      map_idx  = 0;

  for(U64 line_idx = 0; line_idx < lines_count; line_idx += 1) {
    CV_LineArray *l = lines + line_idx;
    if (l->line_count > 0) {
      for(U64 voff_idx = 0; voff_idx < l->line_count; voff_idx += 1) {
        map[map_idx].voff     = l->voffs[voff_idx];
        map[map_idx].file_off = l->file_off;
        map[map_idx].line_num = l->line_nums[voff_idx];
        map[map_idx].col_num  = 0; // TODO: columns
        map_idx += 1;
      }

      map[map_idx].voff     = l->voffs[l->line_count];
      map[map_idx].file_off = l->file_off;
      map[map_idx].line_num = 0;
      map[map_idx].col_num  = 0;
      map_idx += 1;
    }
  }
  Assert(map_idx == total_voff_count);

  qsort(map, total_voff_count, sizeof(map[0]), cv_c13_voff_map_compar);

  CV_LinesAccel *accel = push_array(arena, CV_LinesAccel, 1);
  accel->map_count = total_voff_count;
  accel->map       = map;

  ProfEnd();
  return accel;
}

#if 0
internal CV_Line *
cv_line_from_voff(CV_LinesAccel *accel, U64 voff, U64 *out_line_count)
{
  ProfBeginFunction();

  U64      voff_line_count = 0;
  CV_Line *lines           = 0;

  U64 map_idx = bsearch_nearest_u64(accel->map, accel->map_count, voff, sizeof(accel->map[0]), OffsetOf(CV_Line, voff));
  if(map_idx < accel->map_count) {
    U64 near_voff = accel->map[map_idx].voff;

    for (; map_idx > 0; map_idx -= 1) {
      if(accel->map[map_idx - 1].voff != near_voff) {
        break;
      }
    }

    lines = accel->map + map_idx;

    for(; map_idx < (accel->map_count-1); map_idx += 1) {
      if(accel->map[map_idx].voff != near_voff) {
        break;
      }
      voff_line_count += 1;
    }
  }

  *out_line_count = voff_line_count;

  ProfEnd();
  return lines;
}
#endif

////////////////////////////////
// $$InlineeLines Accel

internal U64
cv_c13_inlinee_lines_accel_hash(void *buffer, U64 size)
{
  XXH64_hash_t hash64 = XXH3_64bits(buffer, size);
  return hash64;
}

internal B32
cv_c13_inlinee_lines_accel_push(CV_InlineeLinesAccel *accel, CV_C13InlineeLinesParsed *parsed)
{
  U64 load_factor = accel->bucket_max * 2/3 + 1;  
  if(accel->bucket_count > load_factor) {
    Assert("TODO: increase max count and rehash buckets");
  }

  B32 is_pushed = 0;

  U64 hash     = cv_c13_inlinee_lines_accel_hash(&parsed->inlinee, sizeof(parsed->inlinee));
  U64 best_idx = hash % accel->bucket_max;
  U64 idx      = best_idx;

  do {
    if(accel->buckets[idx] == 0) {
      accel->buckets[idx] = parsed;
      accel->bucket_count += 1;
      is_pushed = 1;
      break;
    }

    idx = (idx + 1) % accel->bucket_max;
  } while(idx != best_idx);

  return is_pushed;
}

internal CV_C13InlineeLinesParsed *
cv_c13_inlinee_lines_accel_find(CV_InlineeLinesAccel *accel, CV_ItemId inlinee)
{
  CV_C13InlineeLinesParsed *match = 0;

  U64 hash     = cv_c13_inlinee_lines_accel_hash(&inlinee, sizeof(inlinee));
  U64 best_idx = hash % accel->bucket_max;
  U64 idx      = best_idx;

  do {
    if(accel->buckets[idx] != 0) {
      if(accel->buckets[idx]->inlinee == inlinee) {
        match = accel->buckets[idx]; 
        break;
      }
    }

    idx = (idx + 1) % accel->bucket_max;
  } while(idx != best_idx);

  return match;
}

internal CV_InlineeLinesAccel *
cv_c13_make_inlinee_lines_accel(Arena *arena, CV_C13InlineeLinesParsedList inlinee_lines)
{
  ProfBeginFunction();

  // alloc hash table
  CV_InlineeLinesAccel *accel = push_array(arena, CV_InlineeLinesAccel, 1);
  accel->bucket_count = 0;
  accel->bucket_max   = (U64)((F64)inlinee_lines.count * 2.5);
  accel->buckets      = push_array(arena, CV_C13InlineeLinesParsed *, accel->bucket_max);

  // push parsed inlinees
  for(CV_C13InlineeLinesParsedNode *inlinee = inlinee_lines.first; inlinee != 0; inlinee = inlinee->next) {
    cv_c13_inlinee_lines_accel_push(accel, &inlinee->v);
  }

  ProfEnd();
  return accel;
}

////////////////////////////////

internal S32
cv_inline_annot_convert_to_signed_operand(U32 value)
{
  if (value & 1) {
    value = -(value >> 1);
  } else {
    value = value >> 1;
  }
  S32 result = (S32)value;
  return result;
}

internal CV_InlineBinaryAnnotsParsed
cv_c13_parse_inline_binary_annots(Arena                    *arena,
                                  U64                       parent_voff,
                                  CV_C13InlineeLinesParsed *inlinee_parsed,
                                  String8                   binary_annots)
{
  Temp scratch = scratch_begin(&arena, 1);

  struct CodeRange {
    struct CodeRange *next;
    Rng1U64 range;
  };
  struct SourceLine {
    struct SourceLine *next;
    U64                voff;
    U64                length;
    U64                ln;
    U64                cn;
    CV_InlineRangeKind kind;
  };
  struct SourceFile {
    struct SourceFile *next;
    struct SourceLine *line_first;
    struct SourceLine *line_last;
    U64                line_count;
    U64                checksum_off;
    Rng1U64            last_code_range;
  };

  Rng1U64List        code_ranges = {0};
  struct SourceFile *file_first  = 0;
  struct SourceFile *file_last   = 0;
  U64                file_count  = 0;

  CV_InlineRangeKind range_kind             = 0; (void)range_kind;
  U32                code_length            = 0;
  U32                code_offset            = 0;
  U32                file_off               = inlinee_parsed->file_off;
  S32                ln                     = (S32)inlinee_parsed->first_source_ln;
  S32                cn                     = 1;
  U64                code_offset_lo         = 0;
  B32                code_offset_changed    = 0;
  B32                code_offset_lo_changed = 0;
  B32                code_length_changed    = 0;
  B32                ln_changed             = 1;
  B32                file_off_changed       = 0;

  for (U64 cursor = 0, keep_running = 1; cursor < binary_annots.size && keep_running; ) {
    U32 op = CV_InlineBinaryAnnotation_Null;
    cursor += cv_decode_inline_annot_u32(binary_annots, cursor, &op);

    switch (op) {
    case CV_InlineBinaryAnnotation_Null: {
      keep_running = 0;
      
      // this is last run, append range with left over code bytes
      code_length         = code_offset - code_offset_lo;
      code_length_changed = 1;
    }break;
    case CV_InlineBinaryAnnotation_CodeOffset: {
      cursor += cv_decode_inline_annot_u32(binary_annots, cursor, &code_offset);
      code_offset_changed = 1;
    }break;
    case CV_InlineBinaryAnnotation_ChangeCodeOffsetBase: {
      AssertAlways(!"TODO: test case");
      // U32 delta = 0;
      // cursor += cv_decode_inline_annot_u32(binary_annots, cursor, &delta);
      // code_offset_base = code_offset;
      // code_offset_end  = code_offset + delta;
      // code_offset += delta;
    }break;
    case CV_InlineBinaryAnnotation_ChangeCodeOffset: {
      U32 delta = 0;
      cursor += cv_decode_inline_annot_u32(binary_annots, cursor, &delta);

      code_offset += delta;

      if (!code_offset_lo_changed) {
        code_offset_lo = code_offset;
        code_offset_lo_changed = 1;
      }
      code_offset_changed = 1;
    }break;
    case CV_InlineBinaryAnnotation_ChangeCodeLength: {
      code_length = 0;
      cursor += cv_decode_inline_annot_u32(binary_annots, cursor, &code_length);
      code_length_changed = 1;
    }break;
    case CV_InlineBinaryAnnotation_ChangeFile: {
      U32 old_file_off = file_off;
      cursor += cv_decode_inline_annot_u32(binary_annots, cursor, &file_off);
      file_off_changed = old_file_off != file_off;
      // Compiler isn't obligated to terminate code sequence before chaning files,
      // so we have to always force emit code range on file change.
      code_length_changed = file_off_changed;
    }break;
    case CV_InlineBinaryAnnotation_ChangeLineOffset: {
      S32 delta = 0;
      cursor += cv_decode_inline_annot_s32(binary_annots, cursor, &delta);

      ln += delta;
      ln_changed = 1;
    }break;
    case CV_InlineBinaryAnnotation_ChangeLineEndDelta: {
      AssertAlways(!"TODO: test case");
      // S32 end_delta = 1;
      // cursor += cv_decode_inline_annot_s32(binary_annots, cursor, &end_delta);
      // ln += end_delta;
    }break;
    case CV_InlineBinaryAnnotation_ChangeRangeKind: {
      AssertAlways(!"TODO: test case");
      // cursor += cv_decode_inline_annot_u32(binary_annots, cursor, &range_kind);
    }break;
    case CV_InlineBinaryAnnotation_ChangeColumnStart: {
      AssertAlways(!"TODO: test case");
      // S32 delta;
      // cursor += cv_decode_inline_annot_s32(binary_annots, cursor, &delta);
      // cn += delta;
    }break;
    case CV_InlineBinaryAnnotation_ChangeColumnEndDelta: {
      AssertAlways(!"TODO: test case");
      // S32 end_delta;
      // cursor += cv_decode_inline_annot_s32(binary_annots, cursor, &end_delta);
      // cn += end_delta;
    }break;
    case CV_InlineBinaryAnnotation_ChangeCodeOffsetAndLineOffset: {
      U32 code_offset_and_line_offset = 0;
      cursor += cv_decode_inline_annot_u32(binary_annots, cursor, &code_offset_and_line_offset);

      S32 line_delta = cv_inline_annot_convert_to_signed_operand(code_offset_and_line_offset >> 4);
      U32 code_delta = (code_offset_and_line_offset & 0xf);

      code_offset += code_delta;
      ln          += line_delta;

      if (!code_offset_lo_changed) {
        code_offset_lo = code_offset;
        code_offset_lo_changed = 1;
      }

      code_offset_changed = 1;
      ln_changed          = 1;
    }break;
    case CV_InlineBinaryAnnotation_ChangeCodeLengthAndCodeOffset: {
      U32 offset_delta = 0;
      cursor += cv_decode_inline_annot_u32(binary_annots, cursor, &code_length);
      cursor += cv_decode_inline_annot_u32(binary_annots, cursor, &offset_delta); 

      code_offset += offset_delta;

      if (!code_offset_lo_changed) {
        code_offset_lo = code_offset;
        code_offset_lo_changed = 1;
      }

      code_offset_changed = 1;
      code_length_changed = 1;
    }break;
    case CV_InlineBinaryAnnotation_ChangeColumnEnd: {
      AssertAlways(!"TODO: test case");
      // U32 column_end = 0;
      // cursor += cv_decode_inline_annot_u32(binary_annots, cursor, &column_end);
    }break;
    }

    U64 line_code_offset = code_offset;

    if (code_length_changed) {
      // compute upper bound of the range
      U64 code_offset_hi = code_offset + code_length;

      // can last code range be extended to cover current sequence too?
      if (code_ranges.last != 0 && code_ranges.last->v.max == parent_voff + code_offset_lo) {
        code_ranges.last->v.max = parent_voff + code_offset_hi;
      } else {
        // append range
        rng_1u64_list_push(arena, &code_ranges, rng_1u64(parent_voff + code_offset_lo, parent_voff + code_offset_hi));

        // update last code range in file
        if (file_last) {
          file_last->last_code_range = code_ranges.last->v;
        }
      }

      // update low offset for next range
      code_offset_lo = code_offset_hi;

      // advance code offset
      code_offset += code_length;

      // reset state
      code_offset_lo_changed = 0;
      code_length_changed    = 0;
      code_length            = 0;
    }

    if (file_off_changed || (file_first == 0)) {
      // append file
      struct SourceFile *file = push_array(scratch.arena, struct SourceFile, 1);
      file->checksum_off = file_off;
      SLLQueuePush(file_first, file_last, file);
      ++file_count;

      // update last code range in file
      if (code_ranges.last) {
        file->last_code_range = code_ranges.last->v;
      }

      // reset state
      file_off_changed = 0;
    }

    if (code_offset_changed && ln_changed) {
      if (file_last->line_last == 0 || file_last->line_last->ln != (U64)ln) {
        // append line
        struct SourceLine *line = push_array(scratch.arena, struct SourceLine, 1);
        line->voff = parent_voff + line_code_offset;
        line->ln   = (U64)ln;
        line->cn   = (U64)cn;
        SLLQueuePush(file_last->line_first, file_last->line_last, line);
        ++file_last->line_count;
      }

      // reset state
      code_offset_changed = 0;
      ln_changed          = 0;
    }
  }

  CV_LineArray *lines = push_array(arena, CV_LineArray, file_count);
  {
    U64 lines_idx = 0;
    for (struct SourceFile *file = file_first; file != 0; file = file->next, lines_idx += 1) {
      CV_LineArray *l = lines + lines_idx;

      l->file_off   = file->checksum_off;
      l->line_count = file->line_count;
      l->col_count  = 0;

      if (file->line_count > 0) {
        l->voffs     = push_array_no_zero(arena, U64, file->line_count + 1);
        l->line_nums = push_array_no_zero(arena, U32, file->line_count);
        l->col_nums  = 0; // TODO: column info 

        U64 line_idx = 0;
        for (struct SourceLine *line = file->line_first; line != NULL; line = line->next, ++line_idx) {
          // emit line voff and line number
          l->voffs[line_idx]     = line->voff;
          l->line_nums[line_idx] = (U32)line->ln;
        }
        Assert(line_idx == file->line_count);
        l->voffs[line_idx] = file->last_code_range.max;
      }
    }
  }

  // fill out result
  CV_InlineBinaryAnnotsParsed result = {0};
  result.lines_count      = file_count;
  result.lines            = lines;
  result.code_ranges      = code_ranges;

  scratch_end(scratch);
  return result;
}

////////////////////////////////

internal CV_EncodedFramePtrReg
cv_pick_fp_encoding(CV_SymFrameproc *frameproc, B32 is_local_param)
{
  CV_EncodedFramePtrReg fp_reg = 0;
  if (is_local_param) {
    fp_reg = CV_FrameprocFlags_ExtractParamBasePointer(frameproc->flags);
  } else {
    fp_reg = CV_FrameprocFlags_ExtractLocalBasePointer(frameproc->flags);
  }
  return fp_reg;
}

internal CV_Reg
cv_decode_fp_reg(CV_Arch arch, CV_EncodedFramePtrReg encoded_reg)
{
  CV_Reg fp_reg = 0;
  switch (arch) {
  case CV_Arch_8086: {
    switch (encoded_reg) {
    case CV_EncodedFramePtrReg_None    : break;
    case CV_EncodedFramePtrReg_StackPtr: AssertAlways(!"TODO(nick): not tested, this is a guess");
                                         fp_reg = CV_Regx86_ESP; break;
    case CV_EncodedFramePtrReg_FramePtr: fp_reg = CV_Regx86_EBP; break;
    case CV_EncodedFramePtrReg_BasePtr : fp_reg = CV_Regx86_EBX; break;
    default: InvalidPath;
    }
  } break;
  case CV_Arch_X64: {
    switch (encoded_reg) {
    case CV_EncodedFramePtrReg_None    : break;
    case CV_EncodedFramePtrReg_StackPtr: fp_reg = CV_Regx64_RSP; break;
    case CV_EncodedFramePtrReg_FramePtr: fp_reg = CV_Regx64_RBP; break;
    case CV_EncodedFramePtrReg_BasePtr : fp_reg = CV_Regx64_R13; break;
    default: InvalidPath;
    }
  } break;
  default: NotImplemented;
  }
  return fp_reg;
}

internal Rng1U64List
cv_make_defined_range_list_from_gaps(Arena *arena, Rng1U64 defrange, CV_LvarAddrGap *gaps, U64 gap_count)
{
  Rng1U64List result = {0};

  if (gap_count == 0) {
    // no gaps, push whole range
    rng_1u64_list_push(arena, &result, defrange);
  } else {
    U64 cursor = defrange.min;
    for (U64 gap_idx = 0; gap_idx < gap_count; ++gap_idx) {
      // make range
      Rng1U64 range = rng_1u64(cursor, cursor + gaps[gap_idx].off);
      rng_1u64_list_push(arena, &result, range);

      // advance
      cursor = defrange.min + gaps[gap_idx].off + gaps[gap_idx].len;
    }


    // emit range past last gap
    if (gap_count > 0) {
      CV_LvarAddrGap  last_gap             = gaps[gap_count - 1];
      U64             last_range_byte_size = dim_1u64(defrange) - (last_gap.off + last_gap.len);
      if (last_range_byte_size) {
        Rng1U64 last_range = rng_1u64(defrange.min + last_gap.off + last_gap.len, defrange.max);
        rng_1u64_list_push(arena, &result, last_range);
      }
    }
  }

  return result;
}

////////////////////////////////

internal U64
cv_size_from_reg_x86(CV_Reg reg)
{
  switch (reg) {
#define X(NAME, CODE, RDI_NAME, BYTE_POS, BYTE_SIZE) case CV_Regx86_##NAME: return BYTE_SIZE;
    CV_Reg_X86_XList(X)
#undef X
  }
  return 0;
}

internal U64
cv_size_from_reg_x64(CV_Reg reg)
{
  switch (reg) {
#define X(NAME, CODE, RDI_NAME, BYTE_POS, BYTE_SIZE) case CV_Regx64_##NAME: return BYTE_SIZE;
  CV_Reg_X64_XList(X)
#undef X 
  }
  return 0;
}

internal U64
cv_size_from_reg(CV_Arch arch, CV_Reg reg)
{
  switch (arch) {
  case CV_Arch_8086: return cv_size_from_reg_x86(reg);
  case CV_Arch_X64 : return cv_size_from_reg_x64(reg);
  }
  return 0;
}

////////////////////////////////

internal CV_Arch
cv_arch_from_coff_machine(COFF_MachineType machine)
{
  CV_Arch arch = 0;
  switch (machine) {
  case COFF_MachineType_X64:       arch = CV_Arch_X64;    break;
  case COFF_MachineType_X86:       arch = CV_Arch_8086;   break;
  case COFF_MachineType_AM33:      arch = CV_Arch_AM33;   break;
  case COFF_MachineType_ARM:       NotImplemented;        break;
  case COFF_MachineType_ARM64:     arch = CV_Arch_ARM64;  break;
  case COFF_MachineType_ARMNT:     arch = CV_Arch_ARMNT;  break;
  case COFF_MachineType_EBC:       arch = CV_Arch_EBC;    break;
  case COFF_MachineType_IA64:      arch = CV_Arch_IA64;   break;
  case COFF_MachineType_M32R:      arch = CV_Arch_M32R;   break;
  case COFF_MachineType_MIPS16:    arch = CV_Arch_MIPS16; break;
  case COFF_MachineType_MIPSFPU:   NotImplemented;        break;
  case COFF_MachineType_MIPSFPU16: NotImplemented;        break;
  case COFF_MachineType_POWERPC:   NotImplemented;        break;
  case COFF_MachineType_POWERPCFP: arch = CV_Arch_PPCFP;  break;
  case COFF_MachineType_R4000:     NotImplemented;        break;
  case COFF_MachineType_RISCV32:   NotImplemented;        break;
  case COFF_MachineType_RISCV64:   NotImplemented;        break;
  case COFF_MachineType_RISCV128:  NotImplemented;        break;
  case COFF_MachineType_SH3:       arch = CV_Arch_SH3;    break;
  case COFF_MachineType_SH3DSP:    arch = CV_Arch_SH3DSP; break;
  case COFF_MachineType_SH4:       arch = CV_Arch_SH4;    break;
  case COFF_MachineType_SH5:       NotImplemented;        break;
  case COFF_MachineType_THUMB:     arch = CV_Arch_THUMB;  break;
  case COFF_MachineType_WCEMIPSV2: NotImplemented;        break;
  }
  return arch;
}

internal String8
cv_string_from_type_index_source(CV_TypeIndexSource ti_source)
{
  switch (ti_source) {
  case CV_TypeIndexSource_NULL: return str8_lit("");    break;
  case CV_TypeIndexSource_TPI:  return str8_lit("TPI"); break;
  case CV_TypeIndexSource_IPI:  return str8_lit("IPI"); break;
  case CV_TypeIndexSource_COUNT: break;
  }
  return str8_zero();
}

