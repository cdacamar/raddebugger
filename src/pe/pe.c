// Copyright (c) 2024 Epic Games Tools
// Licensed under the MIT license (https://opensource.org/license/mit/)

////////////////////////////////
//~ rjf: Basic Enum Functions

internal U32
pe_slot_count_from_unwind_op_code(PE_UnwindOpCode opcode)
{
  U32 result = 0;
  switch(opcode)
  {
    case PE_UnwindOpCode_PUSH_NONVOL:     result = 1; break;
    case PE_UnwindOpCode_ALLOC_LARGE:     result = 2; break;
    case PE_UnwindOpCode_ALLOC_SMALL:     result = 1; break;
    case PE_UnwindOpCode_SET_FPREG:       result = 1; break;
    case PE_UnwindOpCode_SAVE_NONVOL:     result = 2; break;
    case PE_UnwindOpCode_SAVE_NONVOL_FAR: result = 3; break;
    case PE_UnwindOpCode_EPILOG:          result = 2; break;
    case PE_UnwindOpCode_SPARE_CODE:      result = 3; break;
    case PE_UnwindOpCode_SAVE_XMM128:     result = 2; break;
    case PE_UnwindOpCode_SAVE_XMM128_FAR: result = 3; break;
    case PE_UnwindOpCode_PUSH_MACHFRAME:  result = 1; break;
  }
  return result;
}

read_only struct
{
  String8             string;
  PE_WindowsSubsystem type;
} g_pe_subsystem_map[] = {
  { str8_lit_comp(""),                         PE_WindowsSubsystem_UNKNOWN                  },
  { str8_lit_comp("native"),                   PE_WindowsSubsystem_NATIVE                   },
  { str8_lit_comp("windows"),                  PE_WindowsSubsystem_WINDOWS_GUI              },
  { str8_lit_comp("console"),                  PE_WindowsSubsystem_WINDOWS_CUI              },
  { str8_lit_comp("os2_cui"),                  PE_WindowsSubsystem_OS2_CUI                  },
  { str8_lit_comp("posix"),                    PE_WindowsSubsystem_POSIX_CUI                },
  { str8_lit_comp("native_windows"),           PE_WindowsSubsystem_NATIVE_WINDOWS           },
  { str8_lit_comp("windows_ce_gui"),           PE_WindowsSubsystem_WINDOWS_CE_GUI           },
  { str8_lit_comp("efi_application"),          PE_WindowsSubsystem_EFI_APPLICATION          },
  { str8_lit_comp("efi_boot_service_driver"),  PE_WindowsSubsystem_EFI_BOOT_SERVICE_DRIVER  },
  { str8_lit_comp("efi_runtime_driver"),       PE_WindowsSubsystem_EFI_RUNTIME_DRIVER       },
  { str8_lit_comp("efi_rom"),                  PE_WindowsSubsystem_EFI_ROM                  },
  { str8_lit_comp("xbox"),                     PE_WindowsSubsystem_XBOX                     },
  { str8_lit_comp("windows_boot_application"), PE_WindowsSubsystem_WINDOWS_BOOT_APPLICATION },
};
StaticAssert(ArrayCount(g_pe_subsystem_map) == PE_WindowsSubsystem_COUNT, g_pe_subsystem_map_count_check);

internal String8
pe_string_from_subsystem(PE_WindowsSubsystem subsystem)
{
  for (U64 i = 0; i < ArrayCount(g_pe_subsystem_map); i += 1) {
    if (g_pe_subsystem_map[i].type == subsystem) {
      return g_pe_subsystem_map[i].string;
    }
  }
  return str8(0,0);
}

internal PE_WindowsSubsystem
pe_subsystem_from_string(String8 string)
{
  for (U64 i = 0; i < ArrayCount(g_pe_subsystem_map); i += 1) {
    if (str8_match(g_pe_subsystem_map[i].string, string, StringMatchFlag_CaseInsensitive)) {
      return g_pe_subsystem_map[i].type;
    }
  }
  return PE_WindowsSubsystem_UNKNOWN;
}

////////////////////////////////
//~ rjf: Parser Functions

internal PE_BinInfo
pe_bin_info_from_data(Arena *arena, String8 data)
{
  PE_BinInfo info = {0};
  B32 valid = 1;
  
  // rjf: read dos header
  PE_DosHeader dos_header = {0};
  str8_deserial_read_struct(data, 0, &dos_header);
  
  // rjf: bad dos magic -> bad
  if(dos_header.magic != PE_DOS_MAGIC)
  {
    valid = 0;
  }
  
  // rjf: read pe magic
  U32 pe_magic = 0;
  if(valid)
  {
    str8_deserial_read_struct(data, dos_header.coff_file_offset, &pe_magic);
  }
  
  // rjf: bad pe magic -> abort
  if(pe_magic != PE_MAGIC)
  {
    valid = 0;
  }
  
  // rjf: read coff header
  U32 coff_header_off = dos_header.coff_file_offset + sizeof(pe_magic);
  COFF_Header coff_header = {0};
  if(valid)
  {
    str8_deserial_read_struct(data, coff_header_off, &coff_header);
  }
  
  // rjf: range of optional extension header ("optional" for short)
  U32 optional_size = coff_header.optional_header_size;
  U64 after_coff_header_off = coff_header_off + sizeof(coff_header);
  U64 after_optional_header_off = after_coff_header_off + optional_size;
  Rng1U64 optional_range = {0};
  if(valid)
  {
    optional_range.min = ClampTop(after_coff_header_off, data.size);
    optional_range.max = ClampTop(after_optional_header_off, data.size);
  }
  
  // rjf: get sections
  U64 sec_array_off = optional_range.max;
  U64 sec_array_raw_opl = sec_array_off + coff_header.section_count*sizeof(COFF_SectionHeader);
  U64 sec_array_opl = ClampTop(sec_array_raw_opl, data.size);
  U64 clamped_sec_count = (sec_array_opl - sec_array_off)/sizeof(COFF_SectionHeader);
  COFF_SectionHeader *sections = (COFF_SectionHeader*)(data.str + sec_array_off);
  
  // rjf: get symbols
  U64 symbol_array_off = coff_header.symbol_table_foff;
  U64 symbol_count = coff_header.symbol_count;
  
  // rjf: get string table
  U64 string_table_off = symbol_array_off + sizeof(COFF_Symbol16) * symbol_count;
  
  // rjf: read optional header
  U16 optional_magic = 0;
  U64 image_base = 0;
  U64 entry_point = 0;
  U32 data_dir_count = 0;
  U64 virt_section_align = 0;
  U64 file_section_align = 0;
  Rng1U64 *data_dir_franges = 0;
  if(valid && optional_size > 0)
  {
    // rjf: read magic number
    str8_deserial_read_struct(data, optional_range.min, &optional_magic);
    
    // rjf: read info
    U32 reported_data_dir_offset = 0;
    U32 reported_data_dir_count = 0;
    switch(optional_magic)
    {
      case PE_PE32_MAGIC:
      {
        PE_OptionalHeader32 pe_optional = {0};
        str8_deserial_read_struct(data, optional_range.min, &pe_optional);
        image_base = pe_optional.image_base;
        entry_point = pe_optional.entry_point_va;
        virt_section_align = pe_optional.section_alignment;
        file_section_align = pe_optional.file_alignment;
        reported_data_dir_offset = sizeof(pe_optional);
        reported_data_dir_count = pe_optional.data_dir_count;
      }break;
      case PE_PE32PLUS_MAGIC:
      {
        PE_OptionalHeader32Plus pe_optional = {0};
        str8_deserial_read_struct(data, optional_range.min, &pe_optional);
        image_base = pe_optional.image_base;
        entry_point = pe_optional.entry_point_va;
        virt_section_align = pe_optional.section_alignment;
        file_section_align = pe_optional.file_alignment;
        reported_data_dir_offset = sizeof(pe_optional);
        reported_data_dir_count = pe_optional.data_dir_count;
      }break;
    }
    
    // rjf: find file ranges of data directories
    U32 data_dir_max = (optional_size - reported_data_dir_offset) / sizeof(PE_DataDirectory);
    data_dir_count = ClampTop(reported_data_dir_count, data_dir_max);
    
    // rjf: convert PE directories to ranges
    data_dir_franges = push_array(arena, Rng1U64, data_dir_count);
    for(U32 dir_idx = 0; dir_idx < data_dir_count; dir_idx += 1)
    {
      U64 dir_offset = optional_range.min + reported_data_dir_offset + sizeof(PE_DataDirectory)*dir_idx;
      PE_DataDirectory dir = {0};
      str8_deserial_read_struct(data, dir_offset, &dir);
      U64 file_off = coff_foff_from_voff(sections, clamped_sec_count, dir.virt_off);
      data_dir_franges[dir_idx] = r1u64(file_off, file_off+dir.virt_size);
    }
  }
  
  // rjf: read info about debug file
  U32 dbg_time = 0;
  U32 dbg_age = 0;
  OS_Guid dbg_guid = {0};
  U64 dbg_path_off = 0;
  U64 dbg_path_size = 0;
  if(valid && PE_DataDirectoryIndex_DEBUG < data_dir_count)
  {
    // rjf: read debug directory
    PE_DebugDirectory dbg_data = {0};
    str8_deserial_read_struct(data, data_dir_franges[PE_DataDirectoryIndex_DEBUG].min, &dbg_data);
    
    // rjf: extract external file info from codeview header
    if(dbg_data.type == PE_DebugDirectoryType_CODEVIEW)
    {
      U64 cv_offset = dbg_data.foff;
      U32 cv_magic = 0;
      str8_deserial_read_struct(data, cv_offset, &cv_magic);
      switch(cv_magic)
      {
        default:break;
        case PE_CODEVIEW_PDB20_MAGIC:
        {
          PE_CvHeaderPDB20 cv = {0};
          str8_deserial_read_struct(data, cv_offset, &cv);
          dbg_time = cv.time;
          dbg_age = cv.age;
          dbg_path_off = cv_offset + sizeof(cv);
        }break;
        case PE_CODEVIEW_PDB70_MAGIC:
        {
          PE_CvHeaderPDB70 cv = {0};
          str8_deserial_read_struct(data, cv_offset, &cv);
          dbg_guid = cv.guid;
          dbg_age = cv.age;
          dbg_path_off = cv_offset + sizeof(cv);
        }break;
      }
      if(dbg_path_off > 0)
      {
        U8 *dbg_path_cstring_base = data.str+dbg_path_off;
        dbg_path_size = cstring8_length(dbg_path_cstring_base);
      }
    }
  }
  
  // rjf: extract tls header
  PE_TLSHeader64 tls_header = {0};
  if(valid && PE_DataDirectoryIndex_TLS < data_dir_count)
  {
    Rng1U64 tls_header_frng = data_dir_franges[PE_DataDirectoryIndex_TLS];
    switch(coff_header.machine)
    {
      default:{}break;
      case COFF_MachineType_X86:
      {
        PE_TLSHeader32 tls_header32 = {0};
        str8_deserial_read_struct(data, tls_header_frng.min, &tls_header32);
        tls_header.raw_data_start    = (U64)tls_header32.raw_data_start;
        tls_header.raw_data_end      = (U64)tls_header32.raw_data_end;
        tls_header.index_address     = (U64)tls_header32.index_address;
        tls_header.callbacks_address = (U64)tls_header32.callbacks_address;
        tls_header.zero_fill_size    = (U64)tls_header32.zero_fill_size;
        tls_header.characteristics   = (U64)tls_header32.characteristics;
      }break;
      case COFF_MachineType_X64:
      {
        str8_deserial_read_struct(data, tls_header_frng.min, &tls_header);
      }break;
    }
  }
  
  // rjf: fill info
  if(valid)
  {
    info.image_base                    = image_base;
    info.entry_point                   = entry_point;
    info.is_pe32                       = (optional_magic == PE_PE32_MAGIC);
    info.virt_section_align            = virt_section_align;
    info.file_section_align            = file_section_align;
    info.section_array_off             = sec_array_off;
    info.section_count                 = clamped_sec_count;
    info.symbol_array_off              = symbol_array_off;
    info.symbol_count                  = symbol_count;
    info.string_table_off              = string_table_off;
    info.dbg_path_off                  = dbg_path_off;
    info.dbg_path_size                 = dbg_path_size;
    info.dbg_guid                      = dbg_guid;
    info.dbg_age                       = dbg_age;
    info.dbg_time                      = dbg_time;
    info.data_dir_franges              = data_dir_franges;
    info.data_dir_count                = data_dir_count;
    switch(coff_header.machine)
    {
      default:{}break;
      case COFF_MachineType_X86:   {info.arch = Arch_x86;}break;
      case COFF_MachineType_X64:   {info.arch = Arch_x64;}break;
      case COFF_MachineType_ARM:   {info.arch = Arch_arm32;}break;
      case COFF_MachineType_ARM64: {info.arch = Arch_arm64;}break;
    }
    MemoryCopyStruct(&info.tls_header, &tls_header);
  }
  return info;
}

////////////////////////////////
//~ rjf: Helpers

internal U64
pe_intel_pdata_off_from_voff__binary_search(String8 data, PE_BinInfo *bin, U64 voff)
{
  U64 result = 0;
  if(bin->arch != Arch_Null && PE_DataDirectoryIndex_EXCEPTIONS < bin->data_dir_count)
  {
    Rng1U64 range = bin->data_dir_franges[PE_DataDirectoryIndex_EXCEPTIONS];
    U64 pdata_off = range.min;
    U64 pdata_count = (range.max - range.min)/sizeof(PE_IntelPdata);
    
    // check if this bin includes a pdata array
    if(pdata_count > 0 && 0 <= pdata_off && pdata_off < data.size)
    {
      PE_IntelPdata *pdata_array = (PE_IntelPdata*)(data.str + pdata_off);
      if(voff >= pdata_array[0].voff_first)
      {
        // binary search:
        //  find max index s.t. pdata_array[index].voff_first <= voff
        //  we assume (i < j) -> (pdata_array[i].voff_first < pdata_array[j].voff_first)
        U64 index = pdata_count;
        U64 min = 0;
        U64 opl = pdata_count;
        for(;;)
        {
          U64 mid = (min + opl)/2;
          PE_IntelPdata *pdata = pdata_array + mid;
          if(voff < pdata->voff_first)
          {
            opl = mid;
          }
          else if(pdata->voff_first < voff)
          {
            min = mid;
          }
          else
          {
            index = mid;
            break;
          }
          if(min + 1 >= opl)
          {
            index = min;
            break;
          }
        }
        
        // if we are in range fill result
        {
          PE_IntelPdata *pdata = pdata_array + index;
          if(pdata->voff_first <= voff && voff < pdata->voff_one_past_last)
          {
            result = pdata_off + index*sizeof(PE_IntelPdata);
          }
        }
      }
    }
  }
  return(result);
}

internal void *
pe_ptr_from_voff(String8 data, PE_BinInfo *bin, U64 voff)
{
  // rjf: get the section for this voff
  U64 sec_count = bin->section_count;
  COFF_SectionHeader *sec_array = (COFF_SectionHeader*)((U8*)data.str + bin->section_array_off);
  COFF_SectionHeader *sec_ptr = sec_array;
  COFF_SectionHeader *sec = 0;
  for(U64 i = 1; i <= sec_count; i += 1, sec_ptr += 1)
  {
    if(sec_ptr->voff <= voff && voff < sec_ptr->voff + sec_ptr->vsize)
    {
      sec = sec_ptr;
      break;
    }
  }
  
  // rjf: adjust to file pointer
  void *result = 0;
  if(sec != 0 && sec_ptr->fsize > 0)
  {
    U64 off = voff - sec->voff + sec->foff;
    if(off < data.size)
    {
      result = data.str + off;
    }
  }
  return result;
}

internal U64
pe_section_num_from_voff(String8 data, PE_BinInfo *bin, U64 voff)
{
  U64 sec_count = bin->section_count;
  COFF_SectionHeader *sec_array = (COFF_SectionHeader*)((U8*)data.str + bin->section_array_off);
  COFF_SectionHeader *sec_ptr = sec_array;
  U64 result = 0;
  for(U64 i = 1; i <= sec_count; i += 1, sec_ptr += 1)
  {
    if(sec_ptr->voff <= voff && voff < sec_ptr->voff + sec_ptr->vsize)
    {
      result = i;
      break;
    }
  }
  return result;
}

internal void *
pe_ptr_from_section_num(String8 data, PE_BinInfo *bin, U64 n)
{
  void *result = 0;
  U64 sec_count = bin->section_count;
  if(1 <= n && n <= sec_count)
  {
    COFF_SectionHeader *sec_array = (COFF_SectionHeader*)((U8*)data.str + bin->section_array_off);
    COFF_SectionHeader *sec = sec_array + n - 1;
    if(sec->fsize > 0)
    {
      result = data.str + sec->foff;
    }
  }
  return(result);
}

internal U64
pe_foff_from_voff(String8 data, PE_BinInfo *bin, U64 voff)
{
  U64 foff = 0;
  COFF_SectionHeader *sections = (COFF_SectionHeader*)(data.str+bin->section_array_off);
  U64 section_count = bin->section_count;
  for(U64 sect_idx = 0; sect_idx < section_count; sect_idx += 1)
  {
    COFF_SectionHeader *sect = &sections[sect_idx];
    if(sect->voff <= voff && voff < sect->voff + sect->vsize)
    {
      if(!(sect->flags & COFF_SectionFlag_CNT_UNINITIALIZED_DATA))
      {
        foff = sect->foff + (voff - sect->voff);
      }
      break;
    }
  }
  return foff;
}

internal PE_BaseRelocBlockList
pe_base_reloc_block_list_from_bin(Arena *arena, String8 data, PE_BinInfo *bin)
{
  PE_BaseRelocBlockList list = {0};
  Rng1U64 base_reloc_range = bin->data_dir_franges[PE_DataDirectoryIndex_BASE_RELOC];
  U64 range_dim = dim_1u64(base_reloc_range);
  for(U64 off = base_reloc_range.min; off < range_dim;)
  {
    // rjf: read next entry
    U32 page_virt_off = 0;
    U32 block_size = 0;
    off += str8_deserial_read_struct(data, off, &page_virt_off);
    off += str8_deserial_read_struct(data, off, &block_size);
    
    // rjf: break on sentinel
    if(block_size == 0)
    {
      break;
    }
    
    // rjf: add node
    PE_BaseRelocBlockNode *node = push_array(arena, PE_BaseRelocBlockNode, 1);
    SLLQueuePush(list.first, list.last, node);
    list.count += 1;
    
    // rjf: fill block
    PE_BaseRelocBlock *block = &node->v;
    U64 entries_size = block_size - (sizeof(block_size) + sizeof(page_virt_off));
    block->page_virt_off = page_virt_off;
    block->entry_count = entries_size / sizeof(U16);
    block->entries = push_array(arena, U16, block->entry_count);
    off += str8_deserial_read_array(data, off, &block->entries[0], entries_size);
  }
  return list;
}

internal Rng1U64
pe_tls_rng_from_bin_base_vaddr(String8 data, PE_BinInfo *bin, U64 base_vaddr)
{
  U64 result_addr = (bin->tls_header.index_address - bin->image_base);
  U64 result_size = sizeof(U32);
  if(bin->arch != Arch_Null)
  {
    U64 addr_size = bit_size_from_arch(bin->arch)/8;
    Temp scratch = scratch_begin(0, 0);
    PE_BaseRelocBlockList relocs = pe_base_reloc_block_list_from_bin(scratch.arena, data, bin);
    for(PE_BaseRelocBlockNode *n = relocs.first; n != 0; n = n->next)
    {
      PE_BaseRelocBlock *block = &n->v;
      for(U64 ientry = 0; ientry < block->entry_count;)
      {
        U32 reloc = block->entries[ientry];
        U16 kind = PE_BaseRelocKindFromEntry(reloc);
        U16 offset = PE_BaseRelocOffsetFromEntry(reloc);
        U64 apply_to_voff = block->page_virt_off + offset;
        U64 apply_to_foff = pe_foff_from_voff(data, bin, apply_to_voff);
        U64 apply_to      = 0;
        str8_deserial_read(data, apply_to_foff, &apply_to, addr_size, 1);
        if(apply_to == bin->tls_header.index_address)
        {
          U64 base_diff = base_vaddr-bin->image_base;
          switch(kind)
          {
            default:
            {
              // NOTE(rjf): these relocs are arm/mips/riscv specific which aren't supported at the moment
            }break;
            case PE_BaseRelocKind_ABSOLUTE:{}break;
            case PE_BaseRelocKind_HIGH:
            {
              // rjf: relocate high 16-bits.
              U64 high_bits = (apply_to & max_U16) << 16;
              result_addr = (high_bits + ((base_diff & max_U32) >> 16)) & max_U16;
            }break;
            case PE_BaseRelocKind_LOW:
            {
              // rjf: relocate low 16-bits.
              U64 low_bits = apply_to & max_U16;
              result_addr = (low_bits + (base_diff & max_U32)) & max_U16;
            }break;
            case PE_BaseRelocKind_HIGHLOW:
            {
              // rjf: relocate 32-bits.
              result_addr = (apply_to & max_U32) + (base_diff & max_U32);
            }break;
            case PE_BaseRelocKind_HIGHADJ:
            {
              if(ientry + 1 >= block->entry_count)
              {
                // NOTE(rjf): malformed relocation, expected two 16-bit entries
                break;
              }
              
              // rjf: relocate high bits and adjust sign bit on lower half.
              U16 adj_offset = PE_BaseRelocOffsetFromEntry(block->entries[ientry + 1]);
              result_addr  = (apply_to & max_U16) << 16;
              result_addr += adj_offset;
              result_addr += (base_diff & max_U32);
              result_addr += 0x8000;
              result_addr  = (result_addr >> 16) & max_U16;
            }break;
            case PE_BaseRelocKind_DIR64:
            {
              // rjf: image base relocation.
              result_addr = apply_to + base_diff;
            }break;
          }
          
          goto dbl_break;
        }
        
        U32 advance = (kind == PE_BaseRelocKind_HIGHADJ) ? 2 : 1;
        ientry += advance;
      }
    }
    dbl_break:;
    scratch_end(scratch);
  }
  Rng1U64 result = r1u64(result_addr, result_addr+result_size);
  return result;
}

internal String8Array
pe_get_entry_point_names(COFF_MachineType            machine,
                         PE_WindowsSubsystem         subsystem,
                         PE_ImageFileCharacteristics file_characteristics)
{
  String8Array entry_point_names = {0};
  
  if (file_characteristics & PE_ImageFileCharacteristic_FILE_DLL) {
    if (machine == COFF_MachineType_X86) {
      read_only static String8 dll_entry_point_arr[] = {
        str8_lit_comp("__DllMainCRTStartup@12"),
      };

      entry_point_names.v = &dll_entry_point_arr[0];
      entry_point_names.count = ArrayCount(dll_entry_point_arr);
    } else {
      read_only static String8 dll_entry_point_arr[] = {
        str8_lit_comp("_DllMainCRTStartup"),
      };

      entry_point_names.v = &dll_entry_point_arr[0];
      entry_point_names.count = ArrayCount(dll_entry_point_arr);
    }
  } else {
    switch (subsystem) {
    case PE_WindowsSubsystem_UNKNOWN: break;
    case PE_WindowsSubsystem_WINDOWS_GUI: {
      read_only static String8 gui_entry_point_arr[] = {
        str8_lit_comp("WinMain"),
        str8_lit_comp("wWinMain"),
        str8_lit_comp("WinMainCRTStartup"),
        str8_lit_comp("wWinMainCRTStartup"),
      };

      entry_point_names.v = &gui_entry_point_arr[0];
      entry_point_names.count = ArrayCount(gui_entry_point_arr);
    } break;
    case PE_WindowsSubsystem_WINDOWS_CUI: {
      read_only static String8 cui_entry_point_arr[] = {
        str8_lit_comp("main"),
        str8_lit_comp("wmain"),
        str8_lit_comp("mainCRTStartup"),
        str8_lit_comp("wmainCRTStartup"),
      };

      entry_point_names.v = &cui_entry_point_arr[0];
      entry_point_names.count = ArrayCount(cui_entry_point_arr);
    } break;
    case PE_WindowsSubsystem_NATIVE:
    case PE_WindowsSubsystem_OS2_CUI:
    case PE_WindowsSubsystem_POSIX_CUI:
    case PE_WindowsSubsystem_NATIVE_WINDOWS:
    case PE_WindowsSubsystem_WINDOWS_CE_GUI:
    case PE_WindowsSubsystem_EFI_APPLICATION:
    case PE_WindowsSubsystem_EFI_BOOT_SERVICE_DRIVER:
    case PE_WindowsSubsystem_EFI_RUNTIME_DRIVER:
    case PE_WindowsSubsystem_EFI_ROM:
    case PE_WindowsSubsystem_XBOX:
    case PE_WindowsSubsystem_WINDOWS_BOOT_APPLICATION: {
      // TODO
	  NotImplemented;
    } break;
    }
  }

  return entry_point_names;
}

////////////////////////////////

internal B32
pe_is_res(String8 data)
{
  U8 magic[sizeof(PE_RES_MAGIC)]; MemoryZeroStruct(&magic);
  str8_deserial_read_struct(data, 0, &magic);
  B32 is_res = MemoryCompare(&PE_RES_MAGIC, &magic, sizeof(magic)) == 0;
  return is_res;
}

internal PE_ResourceNode *
pe_resource_dir_push_dir_node(Arena *arena, PE_ResourceDir *dir, COFF_ResourceID id, U32 characteristics, COFF_TimeStamp time_stamp, U16 major_version, U16 minor_version)
{
  PE_ResourceList *list = 0;
  switch (id.type) {
    default:
    case COFF_ResourceIDType_NULL: break;
    case COFF_ResourceIDType_STRING: list = &dir->named_list; break;
    case COFF_ResourceIDType_NUMBER: list = &dir->id_list;    break;
  }
  
  PE_ResourceNode *res_node = push_array(arena, PE_ResourceNode, 1);
  SLLQueuePush(list->first, list->last, res_node);
  list->count += 1;
  
  PE_ResourceDir *sub_dir = push_array(arena, PE_ResourceDir, 1);
  sub_dir->characteristics = characteristics;
  sub_dir->time_stamp      = time_stamp;
  sub_dir->major_version   = major_version;
  sub_dir->minor_version   = minor_version;
  
  PE_Resource *res = &res_node->data;
  res->id    = id;
  res->kind  = PE_ResDataKind_DIR;
  res->u.dir = sub_dir;
  
  return res_node;
}

internal PE_ResourceNode *
pe_resource_dir_push_entry_node(Arena *arena, PE_ResourceDir *dir, COFF_ResourceID id, COFF_ResourceID type, U32 data_version, U32 version, COFF_ResourceMemoryFlags memory_flags, String8 data)
{
  PE_ResourceList *list = NULL;
  switch (id.type) {
    default:
    case COFF_ResourceIDType_NULL: break;
    case COFF_ResourceIDType_STRING: list = &dir->named_list; break;
    case COFF_ResourceIDType_NUMBER: list = &dir->id_list;    break;
  }
  
  PE_ResourceNode *res_node = push_array(arena, PE_ResourceNode, 1);
  SLLQueuePush(list->first, list->last, res_node);
  list->count += 1;
  
  PE_Resource *res = &res_node->data;
  res->id   = id;
  res->kind = PE_ResDataKind_COFF_RESOURCE;
  res->u.coff_res.type         = type;
  res->u.coff_res.data_version = data_version;
  res->u.coff_res.version      = version;
  res->u.coff_res.memory_flags = memory_flags;
  res->u.coff_res.data         = data;
  
  return res_node;
}

internal PE_Resource *
pe_resource_dir_push_entry(Arena *arena, PE_ResourceDir *dir, COFF_ResourceID id, COFF_ResourceID type, U32 data_version, U32 version, COFF_ResourceMemoryFlags memory_flags, String8 data)
{
  PE_ResourceNode *node = pe_resource_dir_push_entry_node(arena, dir, id, type, data_version, version, memory_flags, data);
  return &node->data;
}

internal PE_Resource *
pe_resource_dir_push_dir(Arena *arena, PE_ResourceDir *dir, COFF_ResourceID id, U32 characteristics, COFF_TimeStamp time_stamp, U16 major_version, U16 minor_version)
{
  PE_ResourceNode *dir_node = pe_resource_dir_push_dir_node(arena, dir, id, characteristics, time_stamp, major_version, minor_version); 
  return &dir_node->data;
}

internal PE_ResourceNode *
pe_resource_dir_search_node(PE_ResourceDir *dir, COFF_ResourceID id)
{
  for (PE_ResourceNode *i = dir->id_list.first; i != NULL; i = i->next) {
    if (coff_resource_id_is_equal(i->data.id, id)) {
      return i;
    }
  }
  return NULL;
}

internal PE_Resource *
pe_resource_dir_search(PE_ResourceDir *dir, COFF_ResourceID id)
{
  PE_ResourceNode *node = pe_resource_dir_search_node(dir, id);
  return node ? &node->data : NULL;
}

internal PE_ResourceArray
pe_resource_list_to_array(Arena *arena, PE_ResourceList *list)
{
  PE_ResourceArray result;
  result.count = 0;
  result.v = push_array(arena, PE_Resource, list->count);
  for (PE_ResourceNode *n = list->first; n != NULL; n = n->next) {
    result.v[result.count++] = n->data;
  }
  return result;
}

internal void
pe_resource_dir_push_res_file(Arena *arena, PE_ResourceDir *root_dir, String8 res_file)
{
  // parse file into resource list
  String8 res_data = str8_substr(res_file, rng_1u64(sizeof(PE_RES_MAGIC), res_file.size));
  COFF_ResourceList list = coff_resource_list_from_data(arena, res_data);
  
  // move resources to directories based on type
  for (COFF_ResourceNode *res_node = list.first; res_node != NULL; res_node = res_node->next) {
    COFF_Resource *res = &res_node->data;
    
    // search existing directories
    PE_Resource *dir_res = pe_resource_dir_search(root_dir, res->type);
    
    // create new directory
    if (dir_res == NULL) {
      dir_res = pe_resource_dir_push_dir(arena, root_dir, res->type, 0, 0, 0, 0);
    }
    PE_ResourceDir *dir = dir_res->u.dir;
    
    // check for name collisions
    PE_Resource *check_res = pe_resource_dir_search(dir, res->name);
    if (check_res != NULL) {
      // TODO: how do we handle name conflicts?
      Assert(!"name collision");
      continue;
    }
    
    // push entry
    PE_Resource *sub_dir_res = pe_resource_dir_push_dir(arena, dir, res->name, 0, 0, 0, 0);
    COFF_ResourceID id;
    id.type = COFF_ResourceIDType_NUMBER;
    id.u.number = res->language_id;
    pe_resource_dir_push_entry(arena, sub_dir_res->u.dir, id, res->type, res->data_version, res->version, res->memory_flags, res->data);
  }
}

internal PE_ResourceDir *
pe_resource_table_from_directory_data(Arena *arena, String8 data)
{
  struct stack_s {
    struct stack_s *next;
    U64 table_offset;
    U64 name_base_offset;
    U64 id_base_offset;
    PE_ResourceDir *table;
    PE_ResourceDir **directory_ptr;
    U64 name_ientry;
    U64 id_ientry;
    U64 name_entry_count;
    U64 id_entry_count;
  };
  
  Temp scratch = scratch_begin(&arena,1);
  struct stack_s *bottom_frame = push_array(scratch.arena, struct stack_s, 1);
  struct stack_s *stack = bottom_frame;
  
  while (stack) {
    if (stack->table == NULL) {
      COFF_ResourceDirTable coff_table = {0};
      str8_deserial_read_struct(data, stack->table_offset, &coff_table);
      
      PE_ResourceDir *table = push_array(arena, PE_ResourceDir, 1);
      table->characteristics = coff_table.characteristics;
      table->time_stamp = coff_table.time_stamp;
      table->major_version = coff_table.major_version;
      table->minor_version = coff_table.minor_version;
      
      stack->table = table;
      stack->name_base_offset = stack->table_offset + sizeof(COFF_ResourceDirTable);
      stack->id_base_offset = stack->table_offset + sizeof(COFF_ResourceDirTable) + sizeof(COFF_ResourceDirEntry) * coff_table.name_entry_count;
      stack->name_entry_count = coff_table.name_entry_count;
      stack->id_entry_count = coff_table.id_entry_count;
      
      if (stack->directory_ptr) {
        *stack->directory_ptr = table;
      }
    }
    
    while (stack->name_ientry < stack->name_entry_count) {
      U64 entry_offset = stack->name_base_offset + stack->name_ientry * sizeof(COFF_ResourceDirEntry);
      ++stack->name_ientry;
      
      PE_ResourceNode *named_node = push_array(arena, PE_ResourceNode, 1);
      SLLQueuePush(stack->table->named_list.first, stack->table->named_list.last, named_node);
      ++stack->table->named_list.count;
      PE_Resource *entry = &named_node->data;
      
      COFF_ResourceDirEntry coff_entry = {0};
      str8_deserial_read_struct(data, entry_offset, &coff_entry);
      
      // NOTE: this is not documented on MSDN but high bit here is set for some reason
      U32 name_offset = coff_entry.name.offset & ~COFF_RESOURCE_SUB_DIR_FLAG;
      U16 name_size = 0;
      str8_deserial_read_struct(data, name_offset, &name_size);
      
      String8 name_block;
      str8_deserial_read_block(data,  name_offset + sizeof(name_size), name_size*sizeof(U16), &name_block);
      String16 name16 = str16((U16*)name_block.str, name_size);
      
      B32 is_dir = !!(coff_entry.id.data_entry_offset & COFF_RESOURCE_SUB_DIR_FLAG);
      
      entry->id.type = COFF_ResourceIDType_STRING;
      entry->id.u.string = str8_from_16(arena, name16);
      entry->kind = is_dir ? PE_ResDataKind_DIR : PE_ResDataKind_COFF_LEAF;
      
      if (is_dir) {
        struct stack_s *frame = push_array(scratch.arena, struct stack_s, 1);
        frame->table_offset = coff_entry.id.sub_dir_offset & ~COFF_RESOURCE_SUB_DIR_FLAG;
        frame->directory_ptr = &entry->u.dir;
        SLLStackPush(stack, frame);
        goto yeild;
      } else {
        str8_deserial_read_struct(data, coff_entry.id.data_entry_offset, &entry->u.leaf);
      }
    }
    
    while (stack->id_ientry < stack->id_entry_count) {
      U64 entry_offset = stack->id_base_offset + stack->id_ientry * sizeof(COFF_ResourceDirEntry);
      ++stack->id_ientry;
      
      PE_ResourceNode *id_node = push_array(arena, PE_ResourceNode, 1);
      SLLQueuePush(stack->table->id_list.first, stack->table->id_list.last, id_node);
      ++stack->table->id_list.count;
      PE_Resource *entry = &id_node->data;
      
      COFF_ResourceDirEntry coff_entry = {0};
      str8_deserial_read_struct(data, entry_offset, &coff_entry);
      
      B32 is_dir = !!(coff_entry.id.sub_dir_offset & COFF_RESOURCE_SUB_DIR_FLAG);
      
      entry->id.type = COFF_ResourceIDType_NUMBER;
      entry->id.u.number = coff_entry.name.id;
      entry->kind = is_dir ? PE_ResDataKind_DIR : PE_ResDataKind_COFF_LEAF;
      
      if (is_dir) {
        struct stack_s *frame = push_array(scratch.arena, struct stack_s, 1);
        frame->table_offset = coff_entry.id.sub_dir_offset & ~COFF_RESOURCE_SUB_DIR_FLAG;
        frame->directory_ptr = &entry->u.dir;
        SLLStackPush(stack, frame);
        goto yeild;
      } else {
        str8_deserial_read_struct(data, coff_entry.id.sub_dir_offset, &entry->u.leaf);
      }
    }
    
    SLLStackPop(stack);
    
    yeild:;
  }
  
  scratch_end(scratch);
  return bottom_frame->table;
}

////////////////////////////////
//~ Debug Directory

internal String8
pe_make_debug_header_pdb70(Arena *arena, OS_Guid guid, U32 age, String8 pdb_path)
{
  Temp scratch = scratch_begin(&arena, 1);
  
  PE_CvHeaderPDB70 header = {0};
  header.magic 			  = PE_CODEVIEW_PDB70_MAGIC;
  header.guid  			  = guid;
  header.age   			  = age;
  
  String8List cv_list = {0};
  str8_serial_begin(scratch.arena, &cv_list);
  str8_serial_push_struct(scratch.arena, &cv_list, &header);
  str8_serial_push_cstr(scratch.arena, &cv_list, pdb_path);
  
  String8 cv_data = str8_serial_end(arena, &cv_list);

  scratch_end(scratch);
  return cv_data;
}

internal String8
pe_make_debug_header_rdi(Arena *arena, OS_Guid guid, String8 rdi_path)
{
  Temp scratch = scratch_begin(&arena,1);

  PE_CvHeaderRDI header = {0};
  header.magic          = PE_CODEVIEW_RDI_MAGIC;
  header.guid           = guid;

  String8List list = {0};
  str8_serial_begin(scratch.arena, &list);
  str8_serial_push_struct(scratch.arena, &list, &header);
  str8_serial_push_cstr(scratch.arena, &list, rdi_path);

  String8 cv_data = str8_serial_end(arena, &list);

  scratch_end(scratch);
  return cv_data;
}

////////////////////////////////
//~ Image Checksum

internal U32 
pe_compute_checksum(U8 *buffer, U64 buffer_size)
{
  // https://bytepointer.com/resources/microsoft_pe_checksum_algo_distilled.htm
  U32 hash = 0;
  for (U16 *ptr16 = (U16*)buffer, *opl16 = (U16*)(buffer + buffer_size);
       ptr16 < opl16;
       ptr16 += 1) {
    hash += *ptr16;
    hash = (hash >> 16) + (hash & 0xffff);
  }
  hash = (U16)(((hash >> 16) + hash) & 0xffff);
  hash += buffer_size;
  return hash;
}

