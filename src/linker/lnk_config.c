// Copyright (c) 2024 Epic Games Tools
// Licensed under the MIT license (https://opensource.org/license/mit/)

////////////////////////////////
// Enum <-> String

read_only struct
{
  LNK_CmdSwitchType type;
  char             *name;
  char             *args;
  char             *desc;
} g_cmd_switch_map[] = {
  { LNK_CmdSwitch_Null,               "",                     "", ""                                                                                                      },
  { LNK_CmdSwitch_NotImplemented,     "NOT_IMPLEMENTED",      "", ""                                                                                                      },
  { LNK_CmdSwitch_Align,              "ALIGN",                ":#", ""                                                                                                    },
  { LNK_CmdSwitch_AllowBind,          "ALLOWBIND",            "[:NO]", ""                                                                                                 },
  { LNK_CmdSwitch_AllowIsolation,     "ALLOWISOLATION",       "[:NO]", ""                                                                                                 },
  { LNK_CmdSwitch_AlternateName,      "ALTERNATENAME",        "Creates an a symbol alias \"FROM=TO\"."                                                                    },
  { LNK_CmdSwitch_AppContainer,       "APPCONTAINER",         "[:NO]", ""                                                                                                 },
  { LNK_CmdSwitch_NotImplemented,     "ASSEMBLYDEBUG",        "", ""                                                                                                      }, // .NET
  { LNK_CmdSwitch_NotImplemented,     "ASSEMBLYLINKRESOURCE", "", ""                                                                                                      }, // .NET
  { LNK_CmdSwitch_NotImplemented,     "ASSEMBLYMODULE",       "", ""                                                                                                      }, // .NET
  { LNK_CmdSwitch_NotImplemented,     "ASSEMBLYRESOURCE",     "", ""                                                                                                      }, // .NET
  { LNK_CmdSwitch_Base,               "BASE",                 "{ADDRESS[,SIZE]|@FILENAME,KEY}", ""                                                                        },
  { LNK_CmdSwitch_NotImplemented,     "CLRIMAGETYPE",         "", ""                                                                                                      }, // .NET
  { LNK_CmdSwitch_NotImplemented,     "CLRLOADEROPTIMIZATION","", ""                                                                                                      }, // .NET
  { LNK_CmdSwitch_NotImplemented,     "CLRSUPPORTLASTERROR",  "", ""                                                                                                      }, // .NET
  { LNK_CmdSwitch_NotImplemented,     "CLRTHREADATTRIBUTE",   "", ""                                                                                                      }, // .NET
  { LNK_CmdSwitch_NotImplemented,     "CLRUNMANAGEDCODECHECK","", ""                                                                                                      }, // .NET
  { LNK_CmdSwitch_Debug,              "DEBUG",                "[:{FULL|NONE}]", ""                                                                                        },
  { LNK_CmdSwitch_NotImplemented,     "DEF",                  ":FILENAME", ""                                                                                             },
  { LNK_CmdSwitch_DefaultLib,         "DEFAULTLIB",           ":LIBNAME", ""                                                                                              },
  { LNK_CmdSwitch_Delay,              "DELAY",                ":{NOBIND|UNLOAD}", ""                                                                                      },
  { LNK_CmdSwitch_DelayLoad,          "DELAYLOAD",            ":DLL", ""                                                                                                  },
  { LNK_CmdSwitch_NotImplemented,     "DELAYSIGN",            "", ""                                                                                                      },
  { LNK_CmdSwitch_NotImplemented,     "DEPENDENTLOADFLAG",    "", ""                                                                                                      },
  { LNK_CmdSwitch_Dll,                "DLL",                  "", ""                                                                                                      },
  { LNK_CmdSwitch_NotImplemented,     "DRIVER",               "", ""                                                                                                      },
  { LNK_CmdSwitch_DisallowLib,        "DISALLOWLIB",          ":LIBRARY", "",                                                                                             },
  { LNK_CmdSwitch_DynamicBase,        "DYNAMICBASE",          "[:NO]", ""                                                                                                 },
  { LNK_CmdSwitch_NotImplemented,     "EMITVOLATILEMETADATA", "", ""                                                                                                      },
  { LNK_CmdSwitch_Entry,              "ENTRY",                ":FUNCTION", ""                                                                                             },
  { LNK_CmdSwitch_Null,               "ERRORREPORT",          "", "Deprecated starting Windows Vista."                                                                    },
  { LNK_CmdSwitch_NotImplemented,     "EXPORT",               ":SYMBOL", ""                                                                                               },
  { LNK_CmdSwitch_NotImplemented,     "EXPORTADMIN",          "", ""                                                                                                      },
  { LNK_CmdSwitch_FastFail,           "FASTFAIL",             "", "Not used."                                                                                             },
  { LNK_CmdSwitch_NotImplemented,     "FASTGENPROFILE",       "", ""                                                                                                      },
  { LNK_CmdSwitch_FileAlign,          "FILEALIGN",            ":#", ""                                                                                                    },
  { LNK_CmdSwitch_Fixed,              "FIXED",                "[:NO]", ""                                                                                                 },
  { LNK_CmdSwitch_NotImplemented,     "FORCE",                "", ""                                                                                                      },
  { LNK_CmdSwitch_FunctionPadMin,     "FUNCTIONPADMIN",      ":#", "Not Implemented"                                                                                      },
  { LNK_CmdSwitch_NotImplemented,     "GUARD",                "", ""                                                                                                      },
  { LNK_CmdSwitch_NotImplemented,     "GENPROFILE",           "", ""                                                                                                      },
  { LNK_CmdSwitch_Heap,               "HEAP",                 "RESERVE[,COMMIT]", ""                                                                                      },
  { LNK_CmdSwitch_HighEntropyVa,      "HIGHENTROPYVA",        "[:NO]", ""                                                                                                 },
  { LNK_CmdSwitch_NotImplemented,     "IDLOUT",               "", ""                                                                                                      },
  { LNK_CmdSwitch_Ignore,             "IGNORE",               ":#", ""                                                                                                    },
  { LNK_CmdSwitch_NotImplemented,     "IGNOREIDL",            "", ""                                                                                                      },
  { LNK_CmdSwitch_NotImplemented,     "ILK",                  "", ""                                                                                                      },
  { LNK_CmdSwitch_ImpLib,             "IMPLIB",               ":FILENAME", ""                                                                                             },
  { LNK_CmdSwitch_Include,            "INCLUDE",              "", ""                                                                                                      },
  { LNK_CmdSwitch_Incremental,        "INCREMENTAL",          "[:NO]", "Incremental linking is not supported."                                                            },
  { LNK_CmdSwitch_NotImplemented,     "INTEGRITYCHECK",       "", ""                                                                                                      },
  { LNK_CmdSwitch_NotImplemented,     "KERNEL",               "", ""                                                                                                      },
  { LNK_CmdSwitch_NotImplemented,     "KEYCONTAINER",         "", ""                                                                                                      },
  { LNK_CmdSwitch_NotImplemented,     "KEYFILE",              "", ""                                                                                                      },
  { LNK_CmdSwitch_LargeAddressAware,  "LARGEADDRESSAWARE",    "[:NO]", ""                                                                                                 },
  { LNK_CmdSwitch_LibPath,            "LIBPATH",              ":DIR", ""                                                                                                  },
  { LNK_CmdSwitch_NotImplemented,     "LINKERREPO",           "", ""                                                                                                      },
  { LNK_CmdSwitch_NotImplemented,     "LINKERREPOTARGET",     "", ""                                                                                                      },
  { LNK_CmdSwitch_NotImplemented,     "LTCG",                 "", ""                                                                                                      },
  { LNK_CmdSwitch_NotImplemented,     "LTCGOUT",              "", ""                                                                                                      },
  { LNK_CmdSwitch_Machine,            "MACHINE",              ":{X64|X86}", ""                                                                                            },
  { LNK_CmdSwitch_Manifest,           "MANIFEST",             "[:{EMBED[,ID=#]|NO]", ""                                                                                   },
  { LNK_CmdSwitch_ManifestDependency, "MANIFESTDEPENDENCY",   ":\"manifest dependency XML string\"", ""                                                                   },
  { LNK_CmdSwitch_ManifestFile,       "MANIFESTFILE",         ":FILENAME", ""                                                                                             },
  { LNK_CmdSwitch_ManifestInput,      "MANIFESTINPUT",        ":FILENAME", ""                                                                                             },
  { LNK_CmdSwitch_ManifestUac,        "MANIFESTUAC",          ":{NO|{'level'={'asInvoker'|'highestAvailable'|'requireAdministrator'} ['uiAccess'={'true'|'false'}]}}", "" },
  { LNK_CmdSwitch_NotImplemented,     "MAP",                  "", ""                                                                                                      },
  { LNK_CmdSwitch_NotImplemented,     "MAPINFO",              "", ""                                                                                                      },
  { LNK_CmdSwitch_NotImplemented,     "MERGE",                "", ""                                                                                                      },
  { LNK_CmdSwitch_NotImplemented,     "MIDL",                 "", ""                                                                                                      },
  { LNK_CmdSwitch_Natvis,             "NATVIS",               ":FILENAME", ""                                                                                             },
  { LNK_CmdSwitch_NotImplemented,     "NOASSEMBLY",           "", ""                                                                                                      },
  { LNK_CmdSwitch_NoDefaultLib,       "NODEFAULTLIB",         ":LIBNAME", ""                                                                                              },
  { LNK_CmdSwitch_NotImplemented,     "NOENTRY",              "", ""                                                                                                      },
  { LNK_CmdSwitch_NotImplemented,     "NOEXP",                "", ""                                                                                                      },
  { LNK_CmdSwitch_NotImplemented,     "NOIMPLIB",             "", ""                                                                                                      },
  { LNK_CmdSwitch_NoLogo,             "NOLOGO",               "", ""                                                                                                      },
  { LNK_CmdSwitch_NxCompat,           "NXCOMPAT",             "[:NO]", ""                                                                                                 },
  { LNK_CmdSwitch_Opt,                "OPT",                  "", ""                                                                                                      },
  { LNK_CmdSwitch_NotImplemented,     "ORDER",                "", ""                                                                                                      },
  { LNK_CmdSwitch_Out,                "OUT",                  ":FILENAME", ""                                                                                             },
  { LNK_CmdSwitch_Pdb,                "PDB",                  ":FILENAME", ""                                                                                             },
  { LNK_CmdSwitch_NotImplemented,     "PDBSTRIPPED",          "", ""                                                                                                      },
  { LNK_CmdSwitch_PdbPageSize,        "PDBPAGESIZE",          ":#", "Page size must be power of two"                                                                      },
  { LNK_CmdSwitch_NotImplemented,     "PROFILE",              "", ""                                                                                                      },
  { LNK_CmdSwitch_Release,            "RELEASE",              "", ""                                                                                                      },
  { LNK_CmdSwitch_NotImplemented,     "SAFESEH",              "", ""                                                                                                      },
  { LNK_CmdSwitch_NotImplemented,     "SECTION",              "", ""                                                                                                      },
  { LNK_CmdSwitch_NotImplemented,     "SOURCELINK",           "", ""                                                                                                      },
  { LNK_CmdSwitch_Stack,              "STACK",                ":RESERVE[,COMMIT]", ""                                                                                     },
  { LNK_CmdSwitch_NotImplemented,     "STUB",                 "", ""                                                                                                      },
  { LNK_CmdSwitch_SubSystem,          "SUBSYSTEM",            ":{CONSOLE|NATIVE|WINDOWS}[,#[.##]]", ""                                                                    },
  { LNK_CmdSwitch_NotImplemented,     "SWAPRUN",              "", ""                                                                                                      },
  { LNK_CmdSwitch_NotImplemented,     "TLBID",                "", ""                                                                                                      },
  { LNK_CmdSwitch_NotImplemented,     "TLBOUT",               "", ""                                                                                                      },
  { LNK_CmdSwitch_NotImplemented,     "TIME",                 "", ""                                                                                                      },
  { LNK_CmdSwitch_TsAware,            "TSAWARE",              "[:NO]", ""                                                                                                 },
  { LNK_CmdSwitch_NotImplemented,     "USERPROFILE",          "", ""                                                                                                      },
  { LNK_CmdSwitch_NotImplemented,     "VERBOSE",              "", ""                                                                                                      },
  { LNK_CmdSwitch_Version,            "VERSION",              "", ""                                                                                                      },
  { LNK_CmdSwitch_NotImplemented,     "WINMD",                "", ""                                                                                                      },
  { LNK_CmdSwitch_NotImplemented,     "WINMDDELAYSIGN",       "", ""                                                                                                      },
  { LNK_CmdSwitch_NotImplemented,     "WINMDKEYCONTAINER",    "", ""                                                                                                      },
  { LNK_CmdSwitch_NotImplemented,     "WINMDKEYFILE",         "", ""                                                                                                      },
  { LNK_CmdSwitch_NotImplemented,     "WHOLEARCHIVE",         "", ""                                                                                                      },
  { LNK_CmdSwitch_NotImplemented,     "WX",                   "", ""                                                                                                      },

  //- internal switches
  { LNK_CmdSwitch_Rad_Age,                     "RAD_AGE",                         ":#", "Age embeded in EXE and PDB, used to validate incremental build. Default is 1." },
  { LNK_CmdSwitch_Rad_BuildInfo,               "RAD_BUILD_INFO",                  "",   "Print build info and exit."                                                    },
  { LNK_CmdSwitch_Rad_CheckUnusedDelayLoadDll, "RAD_CHECK_UNUSED_DELAY_LOAD_DLL", "[:NO]", ""                                                                           },
  { LNK_CmdSwitch_Rad_Debug,                   "RAD_DEBUG",                       "[:NO]", "Emit RAD debug info file."                                                  },
  { LNK_CmdSwitch_Rad_DebugName,               "RAD_DEBUG_NAME",                  ":FILENAME", "Sets file name for RAD debug info file."                                },
  { LNK_CmdSwitch_Rad_DelayBind,               "RAD_DELAY_BIND",                  "[:NO]", ""                                                                           },
  { LNK_CmdSwitch_Rad_DeleteManifest,          "RAD_DELETE_MANIFEST",             "[:NO]", ""                                                                           },
  { LNK_CmdSwitch_Rad_DoMerge,                 "RAD_DO_MERGE",                    "[:NO]", ""                                                                           },
  { LNK_CmdSwitch_Rad_EnvLib,                  "RAD_ENV_LIB",                     "[:NO]", ""                                                                           },
  { LNK_CmdSwitch_Rad_Exe,                     "RAD_EXE",                         "[:NO]", ""                                                                           },
  { LNK_CmdSwitch_Rad_Guid,                    "RAD_GUID",                        ":{IMAGEBLAKE3|XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXXXXXX}", ""                         },
  { LNK_CmdSwitch_Rad_LargePages,              "RAD_LARGE_PAGES",                 "[:NO]", "Disabled by default on Windows."                                            },
  { LNK_CmdSwitch_Rad_LinkVer,                 "RAD_LINK_VER",                    ":##,##", ""                                                                          },
  { LNK_CmdSwitch_Rad_Log,                     "RAD_LOG",                         ":{ALL,INPUT_OBJ,INPUT_LIB,IO,LINK_STATS,TIMERS}", ""                                 },
  { LNK_CmdSwitch_Rad_MtPath,                  "RAD_MT_PATH",                     ":EXEPATH", "Path to manifest tool."                                                  },
  { LNK_CmdSwitch_Rad_OsVer,                   "RAD_OS_VER",                      ":##,##", ""                                                                          },
  { LNK_CmdSwitch_Rad_PageSize,                "RAD_PAGE_SIZE",                   ":#", "Must be power of two."                                                         },
  { LNK_CmdSwitch_Rad_PathStyle,               "RAD_PATH_STYLE",                  ":{WindowsAbsolute|UnixAbsolute}", ""                                                 },
  { LNK_CmdSwitch_Rad_SectVirtOff,             "RAD_SECT_VIRT_OFF",               ":#", "Set RVA where section data is placed in memory. For internal use only."        },
  { LNK_CmdSwitch_Rad_SuppressError,           "RAD_SUPPRESS_ERROR",              ":#", ""                                                                              },
  { LNK_CmdSwitch_Rad_SymbolTableCapDefined,   "RAD_SYMBOL_TABLE_CAP_DEFINED",    ":#", "Number of buckets allocated in the symbol table for defined symbols."          },
  { LNK_CmdSwitch_Rad_SymbolTableCapInternal,  "RAD_SYMBOL_TABLE_CAP_INTERNAL",   ":#", "Number of buckets allocated in the symbol table for internal symbols."         },
  { LNK_CmdSwitch_Rad_SymbolTableCapWeak,      "RAD_SYMBOL_TABLE_CAP_WEAK",       ":#", "Number of buckets allocated in the symbol table for weak symbols."             },
  { LNK_CmdSwitch_Rad_SymbolTableCapLib,       "RAD_SYMBOL_TABLE_CAP_LIB",        ":#", "Number of buckets allocated in the symbol table for library symbols."          },
  { LNK_CmdSwitch_Rad_TargetOs,                "RAD_TARGET_OS",                   ":{WINDOWS,LINUX,MAC}"                                                                },
  { LNK_CmdSwitch_Rad_TimeStamp,               "RAD_TIME_STAMP",                  ":#", "Time stamp embeded in EXE and PDB."                                            },
  { LNK_CmdSwitch_Rad_Version,                 "RAD_VERSION",                     "", "Print version and exit."                                                         },
  { LNK_CmdSwitch_Rad_Workers,                 "RAD_WORKERS",                     ":#", "Sets number of workers created in the pool. Number is capped at 1024."         },

  { LNK_CmdSwitch_Help,                        "HELP",                            "", ""                                                                                },
  { LNK_CmdSwitch_Help,                        "?",                               "", ""                                                                                },
};

internal String8
lnk_string_from_cmd_switch_type(LNK_CmdSwitchType type)
{
  for (U64 cmd_idx = 0; cmd_idx < ArrayCount(g_cmd_switch_map); cmd_idx += 1) {
    if (g_cmd_switch_map[cmd_idx].type == type) {
      return str8_cstring(g_cmd_switch_map[cmd_idx].name);
    }
  }
  return str8(0,0);
}

internal LNK_CmdSwitchType
lnk_cmd_switch_type_from_string(String8 string)
{
  LNK_CmdSwitchType type = LNK_CmdSwitch_Null;
  for (U64 icmd = 0; icmd < ArrayCount(g_cmd_switch_map); icmd += 1) {
    String8 cmd_name = str8_cstring(g_cmd_switch_map[icmd].name);
    if (str8_match(cmd_name, string, StringMatchFlag_CaseInsensitive)) {
      type = g_cmd_switch_map[icmd].type;
      break;
    }
  }
  
  return type;
}

read_only struct {
  char         *name;
  LNK_InputType type;
} g_input_type_map[] = {
  { "o",    LNK_Input_Obj },
  { "obj",  LNK_Input_Obj },
  { "lib",  LNK_Input_Lib },
  { "rlib", LNK_Input_Lib }, // rust libs
  { "res",  LNK_Input_Res },
};

internal LNK_InputType
lnk_input_type_from_string(String8 string)
{
  for (U64 i = 0; i < ArrayCount(g_input_type_map); i += 1) {
    if (str8_match(str8_cstring(g_input_type_map[i].name), string, StringMatchFlag_CaseInsensitive)) {
      return g_input_type_map[i].type;
    }
  }
  return LNK_Input_Null;
}

read_only struct
{
  char         *name;
  LNK_DebugMode mode;
} g_debug_mode_map[] = {
  { "null",     LNK_DebugMode_Null     },
  { "none",     LNK_DebugMode_None     },
  { "fastlink", LNK_DebugMode_FastLink },
  { "ghash",    LNK_DebugMode_GHash    },
  { "full",     LNK_DebugMode_Full     },
}; 

internal LNK_DebugMode
lnk_debug_mode_from_string(String8 string)
{
  for (U64 i = 0; i < ArrayCount(g_debug_mode_map); i += 1) {
    if (str8_match(str8_cstring(g_debug_mode_map[i].name), string, StringMatchFlag_CaseInsensitive)) {
      return g_debug_mode_map[i].mode;
    }
  }
  return LNK_DebugMode_Null;
}

////////////////////////////////

internal LNK_CmdOption *
lnk_cmd_line_push_option_if_not_presentf(Arena *arena, LNK_CmdLine *cmd_line, LNK_CmdSwitchType cmd_switch_type, char *param_fmt, ...)
{
  LNK_CmdOption *opt = 0;
  String8 cmd_switch_name = lnk_string_from_cmd_switch_type(cmd_switch_type);
  if (!lnk_cmd_line_has_option_string(*cmd_line, cmd_switch_name)) {
    va_list param_args;
    va_start(param_args, param_fmt);
    String8 param_str = push_str8fv(arena, param_fmt, param_args);
    va_end(param_args);

    opt = lnk_cmd_line_push_option_string(arena, cmd_line, cmd_switch_name, param_str);
  }
  return opt;
}

internal LNK_CmdOption *
lnk_cmd_line_push_optionf(Arena *arena, LNK_CmdLine *cmd_line, LNK_CmdSwitchType cmd_switch, char *param_fmt, ...)
{
  va_list param_args;
  va_start(param_args, param_fmt);
  String8 param_str = push_str8fv(arena, param_fmt, param_args);
  va_end(param_args);
  String8 cmd_switch_name = lnk_string_from_cmd_switch_type(cmd_switch);
  LNK_CmdOption *opt = lnk_cmd_line_push_option_string(arena, cmd_line, cmd_switch_name, param_str);
  return opt;
}

internal B32
lnk_cmd_line_has_switch(LNK_CmdLine cmd_line, LNK_CmdSwitchType cmd_switch)
{
  String8 cmd_switch_name = lnk_string_from_cmd_switch_type(cmd_switch);
  return lnk_cmd_line_has_option_string(cmd_line, cmd_switch_name);
}

////////////////////////////////

internal void
lnk_error_cmd_switch(LNK_ErrorCode code, LNK_CmdSwitchType cmd_switch, char *fmt, ...)
{
  Temp scratch = scratch_begin(0,0);

  va_list args;
  va_start(args, fmt);

  String8 switch_name = lnk_string_from_cmd_switch_type(cmd_switch);
  String8 message = push_str8fv(scratch.arena, fmt, args);
  String8 output = push_str8f(scratch.arena, "/%S: %S", switch_name, message);
  lnk_error(code, "%S", output);

  va_end(args);

  scratch_end(scratch);
}

internal void
lnk_error_cmd_switch_invalid_param_count(LNK_ErrorCode code, LNK_CmdSwitchType cmd_switch)
{
  lnk_error_cmd_switch(code, cmd_switch, "invalid number of parameters");
}

internal void
lnk_error_cmd_switch_invalid_param(LNK_ErrorCode code, LNK_CmdSwitchType cmd_switch, String8 param)
{
  lnk_error_cmd_switch(code, cmd_switch, "invalid parameter \"%S\"", param);
}

internal String8
lnk_error_check_and_strip_quotes(LNK_ErrorCode error_code, LNK_CmdSwitchType cmd_switch, String8 string)
{
  String8 result = string;
  B32 starts_with_quote = str8_match(str8_lit("\""), string, StringMatchFlag_RightSideSloppy);
  if (starts_with_quote) {
    if (str8_ends_with(string, str8_lit("\""), 0)) {
      result = str8_skip(result, 1);
      result = str8_chop(result, 1);
    } else {
      lnk_error_cmd_switch(error_code, cmd_switch, "detected unmatched \" in \"%S\"", string);
    }
  }
  return result;
}

internal void
lnk_error_invalid_uac_level_param(LNK_ErrorCode error_code, LNK_CmdSwitchType cmd_switch, String8 input)
{
  lnk_error_cmd_switch(error_code, cmd_switch, "invalid param format, expected \"level={'asInvoker'|'highestAvailable'|'requireAdministrator'}\" but got \"%S\"", input);
}

internal void
lnk_error_invalid_uac_ui_access_param(LNK_ErrorCode error_code, LNK_CmdSwitchType cmd_switch, String8 input)
{
  lnk_error_cmd_switch(error_code, cmd_switch, "invalid param format, expected \"uiAccess={'true'|'false'}\" but got \"%S\"", input);
}

////////////////////////////////

internal U64
lnk_get_base_addr(LNK_Config *config)
{
  U64 base_addr = config->user_base_addr;
  if (base_addr == 0) {
    if (config->file_characteristics & PE_ImageFileCharacteristic_FILE_DLL) {
      base_addr = coff_default_dll_base_from_machine(config->machine);
    } else if (config->file_characteristics & PE_ImageFileCharacteristic_EXE) {
      base_addr = coff_default_exe_base_from_machine(config->machine);
    } else {
      lnk_error(LNK_Error_Cmdl, "image type is not specified.");
    }
  }
  return base_addr;
}

internal Version
lnk_get_default_subsystem_version(PE_WindowsSubsystem subsystem, COFF_MachineType machine)
{
  Version ver = make_version(0,0);
  switch (subsystem) {
  case PE_WindowsSubsystem_WINDOWS_BOOT_APPLICATION: {
    ver = make_version(1,0);
  } break;

  case PE_WindowsSubsystem_WINDOWS_CUI: {
    switch (machine) {
    case COFF_MachineType_X64: 
    case COFF_MachineType_X86: {
      ver = make_version(6,0);
    } break;
    case COFF_MachineType_ARMNT:
    case COFF_MachineType_ARM64:
    case COFF_MachineType_ARM: {
      ver = make_version(6,2);
    } break;
    default: { InvalidPath; } break;
    }
  } break;

  case PE_WindowsSubsystem_WINDOWS_GUI: {
    switch (machine) {
    case COFF_MachineType_X64:
    case COFF_MachineType_X86: {
      ver = make_version(6,0);
    } break;
    case COFF_MachineType_ARMNT:
    case COFF_MachineType_ARM64:
    case COFF_MachineType_ARM: {
      ver = make_version(6,2);
    } break;
    default: { InvalidPath; } break;
    }
  } break;

  case PE_WindowsSubsystem_POSIX_CUI: {
    ver = make_version(19,90);
  } break;

  case PE_WindowsSubsystem_EFI_APPLICATION: 
  case PE_WindowsSubsystem_EFI_BOOT_SERVICE_DRIVER:
  case PE_WindowsSubsystem_EFI_ROM: 
  case PE_WindowsSubsystem_EFI_RUNTIME_DRIVER: {
    ver = make_version(1,0);
  } break;

  case PE_WindowsSubsystem_NATIVE_WINDOWS:
  case PE_WindowsSubsystem_NATIVE: {
    Assert(!"TODO: detect -drive=WDM switch");
  } break;
  }
  return ver;
}

internal Version
lnk_get_min_subsystem_version(PE_WindowsSubsystem subsystem, COFF_MachineType machine)
{
  Version ver = make_version(0,0);
  switch (subsystem) {
  case PE_WindowsSubsystem_WINDOWS_BOOT_APPLICATION: {
    ver = make_version(1,0);
  } break;

  case PE_WindowsSubsystem_WINDOWS_CUI: {
    switch (machine) {
    case COFF_MachineType_X86: {
      ver = make_version(5,1);
    } break;

    case COFF_MachineType_X64: { 
      ver = make_version(5,2);
    } break;

    case COFF_MachineType_ARMNT:
    case COFF_MachineType_ARM64:
    case COFF_MachineType_ARM: {
      ver = make_version(6,2);
    } break;

    default: InvalidPath; break;
    }
  } break;

  case PE_WindowsSubsystem_WINDOWS_GUI: {
    switch (machine) {
    case COFF_MachineType_X86: {
      ver = make_version(5,1);
    } break;

    case COFF_MachineType_X64: {
      ver = make_version(5,2);
    } break;

    case COFF_MachineType_ARMNT:
    case COFF_MachineType_ARM64:
    case COFF_MachineType_ARM: {
      ver = make_version(6,2);
    } break;

    default: InvalidPath; break;
    }
  } break;

  case PE_WindowsSubsystem_POSIX_CUI: {
    ver = make_version(1,0);
  } break;

  case PE_WindowsSubsystem_EFI_APPLICATION: 
  case PE_WindowsSubsystem_EFI_BOOT_SERVICE_DRIVER:
  case PE_WindowsSubsystem_EFI_ROM: 
  case PE_WindowsSubsystem_EFI_RUNTIME_DRIVER: {
    ver = make_version(1,0);
  } break;

  case PE_WindowsSubsystem_NATIVE_WINDOWS:
  case PE_WindowsSubsystem_NATIVE: {
    Assert(!"TODO: detect -drive=WDM switch");
  } break;
  }
  return ver;
}

internal String8
lnk_get_mt_path(Arena *arena)
{
#if OS_WINDOWS
#undef OS_WINDOWS
#pragma comment(lib, "shlwapi.lib")
#include <shlwapi.h>

  String8 mt_path = str8(0,0);
  local_persist wchar_t raw_mt_path[MAX_PATH + 1] = L"mt.exe";
  B32 is_mt_found = PathFindOnPathW(&raw_mt_path[0], 0);
  if (is_mt_found) {
    String16 mt_path_16 = str16_cstring(&raw_mt_path[0]);
    mt_path = str8_from_16(arena, mt_path_16);
    mt_path = path_convert_slashes(arena, mt_path, PathStyle_WindowsAbsolute);
  } else {
    lnk_error(LNK_Error_Cmdl, "mt.exe not found, please specify path with /RAD_MT_PATH or run vcvarsall.bat");
  }

#undef OS_WINDOWS
#define OS_WINDOWS 1
#else
#error "TODO: find llvm-mt"
#endif
  return mt_path;
}

internal B32
lnk_do_debug_info(LNK_Config *config)
{
  B32 do_debug_info = config->rad_debug == LNK_SwitchState_Yes ||
    (config->debug_mode != LNK_DebugMode_None && config->debug_mode != LNK_DebugMode_Null);
  return do_debug_info;
}

////////////////////////////////

internal B32
lnk_cmd_switch_parse_version(String8List value_strings, LNK_CmdSwitchType cmd_switch, Version *ver_out)
{
  Temp scratch = scratch_begin(0,0);
  B32 is_parsed = 0;

  if (value_strings.node_count == 1) {
    String8List split_list = str8_split_by_string_chars(scratch.arena, value_strings.first->string, str8_lit("."), StringSplitFlag_KeepEmpties);

    String8 maj_str = str8_lit("0");
    String8 min_str = str8_lit("0");
    if (split_list.node_count == 1) {
      maj_str = split_list.first->string;
    } else if (split_list.node_count == 2) {
      maj_str = split_list.first->string;
      min_str = split_list.last->string;
    } else {
      lnk_error_cmd_switch(LNK_Error_Cmdl, cmd_switch, "invalid version format, too many dots, expected format: {N[.N]}");
      goto exit;
    }

    U64 maj, min;
    if (try_u64_from_str8_c_rules(maj_str, &maj)) {
      if (try_u64_from_str8_c_rules(min_str, &min)) {
        *ver_out = make_version(maj, min);
        is_parsed = 1;
      } else {
        lnk_error_cmd_switch(LNK_Error_Cmdl, cmd_switch, "unable to parse minor version");
      }
    } else {
      lnk_error_cmd_switch(LNK_Error_Cmdl, cmd_switch, "unable to parse major version");
    }
  } else {
    lnk_error_cmd_switch(LNK_Error_Cmdl, cmd_switch, "invalid number of parameters");
  }

exit:;
  scratch_end(scratch);
  return is_parsed;
}

internal B32
lnk_cmd_switch_parse_tuple(String8List value_strings, LNK_CmdSwitchType cmd_switch, Rng1U64 *tuple_out)
{
  if (value_strings.node_count == 1) {
    U64 value;
    if (try_u64_from_str8_c_rules(value_strings.first->string, &value)) {
      tuple_out->v[0] = value;
      return 1;
    } else {
      lnk_error_cmd_switch(LNK_Error_Cmdl, cmd_switch, "unable to parse the parameter \"%S\"", value_strings.first->string);
    }
  } else if (value_strings.node_count == 2) {
    U64 a,b;
    if (try_u64_from_str8_c_rules(value_strings.first->string, &a)) {
      if (try_u64_from_str8_c_rules(value_strings.last->string, &b)) {
        tuple_out->v[0] = a;
        tuple_out->v[1] = b;
        return 1;
      } else {
        lnk_error_cmd_switch(LNK_Error_Cmdl, cmd_switch, "unable ot parse second parameter \"%S\"", value_strings.last->string);
      }
    } else {
      lnk_error_cmd_switch(LNK_Error_Cmdl, cmd_switch, "unable to parse first parameter \"%S\"", value_strings.first->string);
    }
  } else {
    lnk_error_cmd_switch(LNK_Error_Cmdl, cmd_switch, "invalid number of parameters");
  }
  return 0;
}

internal B32
lnk_try_parse_u64(String8 string, LNK_ParseU64Flags flags, U64 *value_out)
{
  if (try_u64_from_str8_c_rules(string, value_out)) {
    if (flags & LNK_ParseU64Flag_CheckUnder32bit) {
      if (*value_out > max_U32) {
        return 0;
      }
    }

    if (flags & LNK_ParseU64Flag_CheckPow2) {
      if (!IsPow2(*value_out)) {
        return 0;
      }
    }
  }

  return 1;
}

internal B32
lnk_cmd_switch_parse_u64(String8List value_strings, LNK_CmdSwitchType cmd_switch, U64 *value_out, LNK_ParseU64Flags flags)
{
  if (value_strings.node_count != 1) {
    lnk_error_cmd_switch(LNK_Error_Cmdl, cmd_switch, "invalid number of parameters, exepcted integer number as input");
    return 0;
  }
  if (!lnk_try_parse_u64(value_strings.first->string, flags, value_out)) {
    lnk_error_cmd_switch(LNK_Error_Cmdl, cmd_switch, "unable to parse string \"%S\"", value_strings.first->string);
    return 0;
  }
  return 1;
}

internal B32
lnk_cmd_switch_parse_u32(String8List value_strings, LNK_CmdSwitchType cmd_switch, U32 *value_out, LNK_ParseU64Flags flags)
{
  U64 value;
  if (lnk_cmd_switch_parse_u64(value_strings, cmd_switch, &value, flags | LNK_ParseU64Flag_CheckUnder32bit)) {
    *value_out = (U32)value;
    return 1;
  }
  return 0;
}

internal B32
lnk_cmd_switch_parse_u64_list(Arena *arena, String8List value_strings, LNK_CmdSwitchType cmd_switch, U64List *list_out, LNK_ParseU64Flags flags)
{
  for (String8Node *string_n = value_strings.first; string_n != 0; string_n = string_n->next) {
    U64 value;
    if (!lnk_try_parse_u64(string_n->string, flags, &value)) {
      return 0;
    }
    u64_list_push(arena, list_out, value);
  }
  return 1;
}

internal B32
lnk_cmd_switch_parse_flag(String8List value_strings, LNK_CmdSwitchType cmd_switch, LNK_SwitchState *value_out)
{
  B32 is_parsed = 0;
  if (value_strings.node_count > 1) {
    lnk_error_cmd_switch(LNK_Error_Cmdl, cmd_switch, "too many parameters");
  } else if (value_strings.node_count == 1) {
    if (str8_match(value_strings.first->string, str8_lit("no"), StringMatchFlag_CaseInsensitive)) {
      *value_out = LNK_SwitchState_No;
      is_parsed = 1;
    } else if (str8_match(value_strings.first->string, str8_lit("yes"), StringMatchFlag_CaseInsensitive)) {
      *value_out = LNK_SwitchState_Yes;
      is_parsed = 1;
    } else if (value_strings.first->string.size == 0) {
      *value_out = 1;
      is_parsed = 1;
    } else {
      lnk_error_cmd_switch(LNK_Error_Cmdl, cmd_switch, "invalid parameter \"%S\"", value_strings.first->string);
    }
  } else {
    *value_out = LNK_SwitchState_Yes;
    is_parsed = 1;
  }
  return is_parsed;
}

internal void
lnk_cmd_switch_set_flag_inv_16(String8List value_strings, LNK_CmdSwitchType cmd_switch, U16 *flags, U16 bits)
{
  LNK_SwitchState state;
  if (lnk_cmd_switch_parse_flag(value_strings, cmd_switch, &state)) {
    switch (state) {
    case LNK_SwitchState_Null: break;
    case LNK_SwitchState_Yes : *flags |= bits;  break;
    case LNK_SwitchState_No  : *flags &= ~bits; break;
    }
  }
}

internal void
lnk_cmd_switch_set_flag_inv_64(String8List value_strings, LNK_CmdSwitchType cmd_switch, U64 *flags, U64 bits)
{
  LNK_SwitchState state;
  if (lnk_cmd_switch_parse_flag(value_strings, cmd_switch, &state)) {
    switch (state) {
    case LNK_SwitchState_Null: break;
    case LNK_SwitchState_Yes : *flags |= bits;  break;
    case LNK_SwitchState_No  : *flags &= ~bits; break;
    }
  }
}

internal void
lnk_cmd_switch_set_flag_16(String8List value_strings, LNK_CmdSwitchType cmd_switch, U16 *flags, U16 bits)
{
  LNK_SwitchState state;
  if (lnk_cmd_switch_parse_flag(value_strings, cmd_switch, &state)) {
    switch (state) {
    case LNK_SwitchState_Null: break;
    case LNK_SwitchState_Yes : *flags |= bits;  break;
    case LNK_SwitchState_No  : *flags &= ~bits; break;
    }
  }
}

internal void
lnk_cmd_switch_set_flag_32(String8List value_strings, LNK_CmdSwitchType cmd_switch, U32 *flags, U32 bits)
{
  LNK_SwitchState state;
  if (lnk_cmd_switch_parse_flag(value_strings, cmd_switch, &state)) {
    switch (state) {
    case LNK_SwitchState_Null: break;
    case LNK_SwitchState_Yes : *flags |= bits;  break;
    case LNK_SwitchState_No  : *flags &= ~bits; break;
    }
  }
}

internal void
lnk_cmd_switch_set_flag_64(String8List value_strings, LNK_CmdSwitchType cmd_switch, U64 *flags, U64 bits)
{
  LNK_SwitchState state;
  if (lnk_cmd_switch_parse_flag(value_strings, cmd_switch, &state)) {
    switch (state) {
    case LNK_SwitchState_Null: break;
    case LNK_SwitchState_Yes : *flags |= bits;  break;
    case LNK_SwitchState_No  : *flags &= ~bits; break;
    }
  }
}

internal B32
lnk_cmd_switch_parse_string(String8List value_strings, LNK_CmdSwitchType cmd_switch, String8 *string_out)
{
  if (value_strings.node_count == 1) {
    if (value_strings.first->string.size > 0) {
      *string_out = value_strings.first->string;
      return 1;
    } else {
      lnk_error_cmd_switch(LNK_Error_Cmdl, cmd_switch, "empty string is not permitted");
    }
  } else {
    lnk_error_cmd_switch(LNK_Error_Cmdl, cmd_switch, "invalid number of parameters");
  }
  return 0;
}

internal void
lnk_cmd_switch_parse_string_copy(Arena *arena, String8List value_strings, LNK_CmdSwitchType cmd_switch, String8 *string_out)
{
  if (lnk_cmd_switch_parse_string(value_strings, cmd_switch, string_out)) {
    *string_out = push_str8_copy(arena, *string_out);
  }
}

////////////////////////////////

internal B32
lnk_parse_alt_name_directive(Arena *arena, String8 input, LNK_AltNameList *list_out)
{
  Temp scratch = scratch_begin(&arena, 1);
  B32 is_parse_ok = 0;
  String8List pair = str8_split_by_string_chars(scratch.arena, input, str8_lit("="), 0);
  if (pair.node_count == 2) {
    str8_list_push(arena, &list_out->from_list, pair.first->string);
    str8_list_push(arena, &list_out->to_list,   pair.last->string);
    is_parse_ok = 1;
  }
  scratch_end(scratch);
  return is_parse_ok;
}

internal String8 *
lnk_parse_alt_name_directive_list(Arena *arena, String8List list, LNK_AltNameList *list_out)
{
  for (String8Node *str_n = list.first; str_n != 0; str_n = str_n->next) {
    B32 is_parse_ok = lnk_parse_alt_name_directive(arena, str_n->string, list_out);
    if ( ! is_parse_ok) {
      return &str_n->string;
    }
  }
  return 0;
}

////////////////////////////////

internal void
lnk_print_build_info()
{
  fprintf(stdout, "  Compiler: %s\n", COMPILER_STRING);
  fprintf(stdout, "  Mode    : %s\n", BUILD_MODE_STRING);
  fprintf(stdout, "  Date    : %s %s\n", __TIME__, __DATE__);
  fprintf(stdout, "  Version : %s\n", BUILD_VERSION_STRING);
}

internal void
lnk_print_help(void)
{
  Temp scratch = scratch_begin(0,0);

  fprintf(stdout, "--- Help -------------------------------------------------------\n");
  fprintf(stdout, "  %s\n", BUILD_VERSION_STRING);
  fprintf(stdout, "\n");
  fprintf(stdout, "  Usage: rad-link.exe [Options] [Files] [@rsp]\n");
  fprintf(stdout, "\n");

  fprintf(stdout, "  Options:\n");
  for (U64 i = 0; i < ArrayCount(g_cmd_switch_map); ++i) {
    Temp temp = temp_begin(scratch.arena);

    char *name = g_cmd_switch_map[i].name;
    char *args = g_cmd_switch_map[i].args;
    char *desc = g_cmd_switch_map[i].desc;
    LNK_CmdSwitchType type = g_cmd_switch_map[i].type;

    if (strcmp(name, "") == 0 ||
        strcmp(name, "NOT_IMPLEMENTED") == 0 ||
        type == LNK_CmdSwitch_Help) {
      continue;
    }

    String8 name_args = push_str8f(temp.arena, "%s%s", name, args);

    fprintf(stdout, "   /%-32.*s %s%s\n",
            str8_varg(name_args),
            desc,
            type == LNK_CmdSwitch_NotImplemented ? "Not Implemented" : "");

    temp_end(temp);
  }

  fprintf(stdout, "\n");

  scratch_end(scratch);
}

////////////////////////////////

internal LNK_Config *
lnk_config_from_cmd_line(Arena *arena, String8List raw_cmd_line)
{
  ProfBeginFunction();
  Temp scratch = scratch_begin(&arena, 1);
  
  // parse command line
  String8List unwrapped_cmd_line = lnk_unwrap_rsp(scratch.arena, raw_cmd_line);
  LNK_CmdLine cmd_line = lnk_cmd_line_parse_windows_rules(scratch.arena, unwrapped_cmd_line);
  
  // setup default flags
  lnk_cmd_line_push_option_if_not_presentf(scratch.arena, &cmd_line, LNK_CmdSwitch_Align, "%u", KB(4));
  lnk_cmd_line_push_option_if_not_presentf(scratch.arena, &cmd_line, LNK_CmdSwitch_Debug, "none");
  lnk_cmd_line_push_option_if_not_presentf(scratch.arena, &cmd_line, LNK_CmdSwitch_FileAlign, "%u", 512);
  if (!lnk_cmd_line_has_switch(cmd_line, LNK_CmdSwitch_Fixed)) {
    lnk_cmd_line_push_optionf(scratch.arena, &cmd_line, LNK_CmdSwitch_DynamicBase, "");
  }
  if (lnk_cmd_line_has_switch(cmd_line, LNK_CmdSwitch_Dll)) {
    lnk_cmd_line_push_option_if_not_presentf(scratch.arena, &cmd_line, LNK_CmdSwitch_SubSystem, "%S", pe_string_from_subsystem(PE_WindowsSubsystem_WINDOWS_GUI));
  }
  lnk_cmd_line_push_option_if_not_presentf(scratch.arena, &cmd_line, LNK_CmdSwitch_Heap, "%u,%u", MB(1), KB(4));
  lnk_cmd_line_push_option_if_not_presentf(scratch.arena, &cmd_line, LNK_CmdSwitch_HighEntropyVa, "");
  lnk_cmd_line_push_option_if_not_presentf(scratch.arena, &cmd_line, LNK_CmdSwitch_ManifestUac, "\"level='asInvoker' uiAccess='false'\"");
  lnk_cmd_line_push_option_if_not_presentf(scratch.arena, &cmd_line, LNK_CmdSwitch_NxCompat, "");
  lnk_cmd_line_push_option_if_not_presentf(scratch.arena, &cmd_line, LNK_CmdSwitch_LargeAddressAware, "");
  lnk_cmd_line_push_option_if_not_presentf(scratch.arena, &cmd_line, LNK_CmdSwitch_PdbPageSize, "%u", KB(4));
  lnk_cmd_line_push_option_if_not_presentf(scratch.arena, &cmd_line, LNK_CmdSwitch_Stack, "%u,%u", MB(1), KB(1));
  lnk_cmd_line_push_option_if_not_presentf(scratch.arena, &cmd_line, LNK_CmdSwitch_Rad_TimeStamp, "%u", os_get_process_start_time_unix());
  lnk_cmd_line_push_option_if_not_presentf(scratch.arena, &cmd_line, LNK_CmdSwitch_Rad_Age, "%u", 1);
  lnk_cmd_line_push_option_if_not_presentf(scratch.arena, &cmd_line, LNK_CmdSwitch_Rad_CheckUnusedDelayLoadDll, "");
  lnk_cmd_line_push_option_if_not_presentf(scratch.arena, &cmd_line, LNK_CmdSwitch_Rad_DelayBind, "");
  lnk_cmd_line_push_option_if_not_presentf(scratch.arena, &cmd_line, LNK_CmdSwitch_Rad_DoMerge, "");
  lnk_cmd_line_push_option_if_not_presentf(scratch.arena, &cmd_line, LNK_CmdSwitch_Rad_EnvLib, "");
  lnk_cmd_line_push_option_if_not_presentf(scratch.arena, &cmd_line, LNK_CmdSwitch_Rad_Exe, "");
  lnk_cmd_line_push_option_if_not_presentf(scratch.arena, &cmd_line, LNK_CmdSwitch_Rad_Guid, "imageblake3");
  lnk_cmd_line_push_option_if_not_presentf(scratch.arena, &cmd_line, LNK_CmdSwitch_Rad_LargePages, "no");
  lnk_cmd_line_push_option_if_not_presentf(scratch.arena, &cmd_line, LNK_CmdSwitch_Rad_LinkVer, "14.0");
  lnk_cmd_line_push_option_if_not_presentf(scratch.arena, &cmd_line, LNK_CmdSwitch_Rad_OsVer, "6.0");
  lnk_cmd_line_push_option_if_not_presentf(scratch.arena, &cmd_line, LNK_CmdSwitch_Rad_PageSize, "%u", KB(4));
  lnk_cmd_line_push_option_if_not_presentf(scratch.arena, &cmd_line, LNK_CmdSwitch_Rad_PathStyle, "system");
  lnk_cmd_line_push_option_if_not_presentf(scratch.arena, &cmd_line, LNK_CmdSwitch_Rad_SectVirtOff, "0x1000");
  lnk_cmd_line_push_option_if_not_presentf(scratch.arena, &cmd_line, LNK_CmdSwitch_Rad_Workers, "%u", os_get_system_info()->logical_processor_count);
  lnk_cmd_line_push_option_if_not_presentf(scratch.arena, &cmd_line, LNK_CmdSwitch_Manifest, "embed");
  lnk_cmd_line_push_option_if_not_presentf(scratch.arena, &cmd_line, LNK_CmdSwitch_Rad_TargetOs, "windows");
  //lnk_cmd_line_push_option_if_not_presentf(scratch.arena, &cmd_line, LNK_CmdSwitch_Rad_Log, "debug");
  lnk_cmd_line_push_option_if_not_presentf(scratch.arena, &cmd_line, LNK_CmdSwitch_Rad_SymbolTableCapDefined, "0x3ffff");
  lnk_cmd_line_push_option_if_not_presentf(scratch.arena, &cmd_line, LNK_CmdSwitch_Rad_SymbolTableCapInternal, "0x1000");
  lnk_cmd_line_push_option_if_not_presentf(scratch.arena, &cmd_line, LNK_CmdSwitch_Rad_SymbolTableCapWeak, "0x3ffff");
  lnk_cmd_line_push_option_if_not_presentf(scratch.arena, &cmd_line, LNK_CmdSwitch_Rad_SymbolTableCapLib, "0x3ffff");
  lnk_cmd_line_push_option_if_not_presentf(scratch.arena, &cmd_line, LNK_CmdSwitch_Rad_DeleteManifest, "");

#if !BUILD_DEBUG
  //lnk_cmd_line_push_optionf(scratch.arena, &cmd_line, LNK_CmdSwitch_Rad_SuppressError, "37");
#endif

  if (!lnk_cmd_line_has_switch(cmd_line, LNK_CmdSwitch_Rad_MtPath)) {
    String8 mt_path = lnk_get_mt_path(scratch.arena);
    lnk_cmd_line_push_option_if_not_presentf(scratch.arena, &cmd_line, LNK_CmdSwitch_Rad_MtPath, "%S", mt_path);
  }

  LNK_Config *config   = push_array(arena, LNK_Config, 1);
  config->raw_cmd_line = raw_cmd_line;
  config->work_dir     = os_get_current_path(scratch.arena);

  // process command line switches
  for (LNK_CmdOption *cmd = cmd_line.first_option; cmd != 0; cmd = cmd->next) {
    LNK_CmdSwitchType cmd_switch = lnk_cmd_switch_type_from_string(cmd->string);
    switch (cmd_switch) {
    case LNK_CmdSwitch_Null: {
      String8 value = str8_list_join(scratch.arena, &cmd->value_strings, &(StringJoin){.sep=str8_lit_comp(",")});
      lnk_error(LNK_Warning_UnknownSwitch, "unknown switch: \"/%S%s%S\"", cmd->string, value.size ? ":" : "", value);
    } break;

    default: { InvalidPath; } break;

    case LNK_CmdSwitch_NotImplemented: {
      String8 value = str8_list_join(scratch.arena, &cmd->value_strings, &(StringJoin){.sep=str8_lit_comp(",")});
      lnk_not_implemented("switch \"%S\" is not implemented \"%S\"", cmd->string, value);
    } break;

    case LNK_CmdSwitch_Align: {
      lnk_cmd_switch_parse_u64(cmd->value_strings, cmd_switch, &config->sect_align, LNK_ParseU64Flag_CheckPow2);
    } break;

    case LNK_CmdSwitch_AllowBind: {
      lnk_cmd_switch_set_flag_inv_16(cmd->value_strings, cmd_switch, &config->dll_characteristics, PE_DllCharacteristic_NO_BIND);
    } break;

    case LNK_CmdSwitch_AllowIsolation: {
      lnk_cmd_switch_set_flag_inv_16(cmd->value_strings, cmd_switch, &config->dll_characteristics, PE_DllCharacteristic_NO_ISOLATION);
    } break;

    case LNK_CmdSwitch_AlternateName: {
      String8 *error_string = lnk_parse_alt_name_directive_list(arena, cmd->value_strings, &config->alt_name_list);
      if (error_string != 0) {
        lnk_error_cmd_switch(LNK_Error_Cmdl, cmd_switch, "invalid syntax \"%S\", expected format \"FROM=TO\"", *error_string);
      }
    } break;

    case LNK_CmdSwitch_AppContainer: {
      lnk_cmd_switch_set_flag_16(cmd->value_strings, cmd_switch, &config->dll_characteristics, PE_DllCharacteristic_APPCONTAINER);
    } break;

    case LNK_CmdSwitch_Base: {
      if (cmd->value_strings.node_count == 2) {
        String8Node *first_node = cmd->value_strings.first;
        //String8Node *second_node = first_node->next;
        B32 is_response_file = str8_match(str8_lit("@"), first_node->string, StringMatchFlag_RightSideSloppy);
        if (is_response_file) {
          //String8 file_path = first_node->string;
          //String8 tag = second_node->string;
          lnk_not_implemented("Response files are not implemented for /BASE");
        } else {
          Rng1U64 addr_size = {0};
          if (lnk_cmd_switch_parse_tuple(cmd->value_strings, cmd_switch, &addr_size)) {
            config->user_base_addr = addr_size.v[0];
            config->max_image_size = addr_size.v[1];
          }
        }
      } else if (cmd->value_strings.node_count == 1) {
        U64 addr;
        if (lnk_cmd_switch_parse_u64(cmd->value_strings, cmd_switch, &addr, 0)) {
          config->user_base_addr = addr;
        }
      } else if (cmd->value_strings.node_count == 0) {
        lnk_error_cmd_switch(LNK_Error_Cmdl, cmd_switch, "expected at least 1 parameter");
      } else {
        lnk_error_cmd_switch(LNK_Error_Cmdl, cmd_switch, "too many parameters");
      }
    } break;

    case LNK_CmdSwitch_Debug: {
      if (cmd->value_strings.node_count == 0) {
        config->debug_mode = LNK_DebugMode_Full;
      } else if (cmd->value_strings.node_count == 1) {
        LNK_DebugMode debug_mode = lnk_debug_mode_from_string(cmd->value_strings.first->string);
        if (debug_mode == LNK_DebugMode_GHash) {
          config->debug_mode = LNK_DebugMode_Full;
          lnk_error_cmd_switch(LNK_Warning_Cmdl, cmd_switch, "GHASH is not supported, switching to FULL");
        } else if (debug_mode != LNK_DebugMode_Null) {
          config->debug_mode = debug_mode;
        } else {
          lnk_error_cmd_switch(LNK_Error_Cmdl, cmd_switch, "invalid parameter \"%S\"", cmd->value_strings.first->string);
        }
      } else {
        lnk_error_cmd_switch(LNK_Error_Cmdl, cmd_switch, "invalid number of parameters");
      }
    } break;

    case LNK_CmdSwitch_DefaultLib: {
      String8List default_lib_list = str8_list_copy(arena, &cmd->value_strings);
      str8_list_concat_in_place(&config->input_default_lib_list, &default_lib_list);
    } break;

    case LNK_CmdSwitch_Delay: {
      if (cmd->value_strings.node_count == 0 || cmd->value_strings.node_count > 1) {
        lnk_error_cmd_switch(LNK_Error_Cmdl, cmd_switch, "invalid number of parameters");
      } else {
        String8 value = cmd->value_strings.first->string;
        if (str8_match(value, str8_lit("unload"), StringMatchFlag_CaseInsensitive)) {
          config->flags |= LNK_ConfigFlag_DelayUnload;
        } else if (str8_match(value, str8_lit("nobind"), StringMatchFlag_CaseInsensitive)) {
          config->flags &= ~LNK_ConfigFlag_DelayBind;
        } else {
          lnk_error_cmd_switch(LNK_Error_Cmdl, cmd_switch, "unknown parameter \"%S\"", value);
        }
      }
    } break;

    case LNK_CmdSwitch_DelayLoad: {
      String8List delay_load_dll_list = str8_list_copy(arena, &cmd->value_strings);
      str8_list_concat_in_place(&config->delay_load_dll_list, &delay_load_dll_list);
    } break;

    case LNK_CmdSwitch_Dll: {
      config->file_characteristics |= PE_ImageFileCharacteristic_FILE_DLL;
    } break;

    case LNK_CmdSwitch_DisallowLib: {
      lnk_not_implemented("TODO: how is this switch different from /nodefaultlib?");
    } break;

    case LNK_CmdSwitch_DynamicBase: {
      lnk_cmd_switch_set_flag_16(cmd->value_strings, cmd_switch, &config->dll_characteristics, PE_DllCharacteristic_DYNAMIC_BASE);
    } break;

    case LNK_CmdSwitch_Entry: {
      lnk_cmd_switch_parse_string_copy(arena, cmd->value_strings, cmd_switch, &config->user_entry_point_name);
      config->entry_point_name = config->user_entry_point_name;
    } break;

    case LNK_CmdSwitch_FastFail: {
      // do nothing
    } break;

    case LNK_CmdSwitch_FileAlign: {
      lnk_cmd_switch_parse_u64(cmd->value_strings, cmd_switch, &config->file_align, LNK_ParseU64Flag_CheckPow2);
    } break;

    case LNK_CmdSwitch_Fixed: {
      lnk_cmd_switch_set_flag_64(cmd->value_strings, cmd_switch, &config->flags, LNK_ConfigFlag_Fixed);
    } break;

    case LNK_CmdSwitch_FunctionPadMin: {
      lnk_cmd_switch_parse_u64(cmd->value_strings, cmd_switch, &config->function_pad_min, LNK_ParseU64Flag_CheckUnder32bit);
    } break;

    case LNK_CmdSwitch_Heap: {
      Rng1U64 reserve_commit;
      reserve_commit.v[0] = config->heap_reserve;
      reserve_commit.v[1] = config->heap_commit;
      if (lnk_cmd_switch_parse_tuple(cmd->value_strings, cmd_switch, &reserve_commit)) {
        if (reserve_commit.v[0] >= reserve_commit.v[1]) {
          U64 reserve_aligned = AlignPow2(reserve_commit.v[0], 4);
          U64 commit_aligned = AlignPow2(reserve_commit.v[1], 4);
#if 0
          if (reserve_aligned != reserve_commit.v[0]) {
            lnk_error_cmd_switch(LNK_WARNING_CMDL, cmd_switch, "reserve is not power of two, aligned to %u bytes", reserve_aligned);
          }
          if (commit_aligned != reserve_commit.v[1]) {
            lnk_error_cmd_switch(LNK_WARNING_CMDL, cmd_switch, "commit is not power of two, aligned to %u bytes", commit_aligned);
          }
#endif
          config->heap_reserve = reserve_aligned;
          config->heap_commit = commit_aligned;
        } else {
          lnk_error_cmd_switch(LNK_Error_Cmdl, cmd_switch, "commit(%llu) is greater than reserve(%llu)", reserve_commit.v[1], reserve_commit.v[0]);
        }
      }
    } break;

    case LNK_CmdSwitch_HighEntropyVa: {
      lnk_cmd_switch_set_flag_16(cmd->value_strings, cmd_switch, &config->dll_characteristics, PE_DllCharacteristic_HIGH_ENTROPY_VA);
    } break;

    case LNK_CmdSwitch_Ignore: {
      U64 error_code;
      if (lnk_cmd_switch_parse_u64(cmd->value_strings, cmd_switch, &error_code, 0)) {
        switch (error_code) {
        case LNK_MsWarningCode_UnsuedDelayLoadDll: {
          lnk_suppress_error(LNK_Warning_UnusedDelayLoadDll);
        } break;
        case LNK_MsWarningCode_MissingExternalTypeServer: {
          lnk_suppress_error(LNK_Warning_MissingExternalTypeServer);
        } break;
        case LNK_MsWarningCode_SectionFlagsConflict: {
          lnk_suppress_error(LNK_Warning_SectionFlagsConflict);
        } break;
        default: {
          lnk_not_implemented("TODO: /IGNORE:%llu", error_code);
        } break;
        }
      }
    } break;

    case LNK_CmdSwitch_ImpLib: {
      lnk_cmd_switch_parse_string_copy(arena, cmd->value_strings, cmd_switch, &config->imp_lib_name);
    } break;

    case LNK_CmdSwitch_Include: {
      String8List include_symbol_list = str8_list_copy(arena, &cmd->value_strings);
      str8_list_concat_in_place(&config->include_symbol_list, &include_symbol_list);
    } break;

    case LNK_CmdSwitch_Incremental: {
      LNK_SwitchState state;
      if (lnk_cmd_switch_parse_flag(cmd->value_strings, cmd_switch, &state)) {
        if (state == LNK_SwitchState_Yes) {
          lnk_error_cmd_switch(LNK_Warning_Cmdl, cmd_switch, "incremental linkage is not supported");
        }
      }
    } break;

    case LNK_CmdSwitch_LargeAddressAware: {
      lnk_cmd_switch_set_flag_16(cmd->value_strings, cmd_switch, &config->file_characteristics, PE_ImageFileCharacteristic_LARGE_ADDRESS_AWARE);
    } break;

    case LNK_CmdSwitch_LibPath: {
      String8List lib_dir_list = str8_list_copy(arena, &cmd->value_strings);
      for (String8Node *dir_n = lib_dir_list.first; dir_n != 0; dir_n = dir_n->next) {
        if (!os_folder_path_exists(dir_n->string)) {
          lnk_error_cmd_switch(LNK_Warning_Cmdl, cmd_switch, "path doesn't exist %S", dir_n->string);
        }
      }
      str8_list_concat_in_place(&config->lib_dir_list, &lib_dir_list);
    } break;

    case LNK_CmdSwitch_Machine: {
      if (cmd->value_strings.node_count == 1) {
        COFF_MachineType machine = coff_machine_from_string(cmd->value_strings.first->string);
        if (machine != COFF_MachineType_UNKNOWN) {
          config->machine = machine;
        } else {
          lnk_error_cmd_switch(LNK_Error_Cmdl, cmd_switch, "unknown parameter \"%S\"", cmd->value_strings.first->string);
        }
      } else {
        lnk_error_cmd_switch(LNK_Error_Cmdl, cmd_switch, "invalid number of parameters");
      }
    } break;

    case LNK_CmdSwitch_Manifest: {
      if (cmd->value_strings.node_count == 1) {
        String8List  param_list = str8_split_by_string_chars(scratch.arena, cmd->value_strings.first->string, str8_lit(","), 0);
        String8Array param_arr  = str8_array_from_list(scratch.arena, &param_list);
        if (param_arr.count > 0) {
          if (str8_match(param_arr.v[0], str8_lit("embed"), StringMatchFlag_CaseInsensitive)) {
            config->manifest_opt = LNK_ManifestOpt_Embed;
            if (config->delete_manifest == LNK_SwitchState_Null) {
              config->delete_manifest = 1;
            }

            if (param_arr.count == 1) {
              if (lnk_cmd_line_has_switch(cmd_line, LNK_CmdSwitch_Dll)) {
                config->manifest_resource_id = 2;
              } else {
                config->manifest_resource_id = 1;
              }
            } else if (param_arr.count > 1) {
              // parse resource id
              if (str8_match(param_arr.v[1], str8_lit("id="), StringMatchFlag_RightSideSloppy|StringMatchFlag_CaseInsensitive)) {
                String8List res_id_list = str8_split_by_string_chars(scratch.arena, param_arr.v[1], str8_lit("="), 0);
                String8Array res_id_arr = str8_array_from_list(scratch.arena, &res_id_list);
                if (res_id_arr.count == 2) {
                  U64 resource_id;
                  if (try_u64_from_str8_c_rules(res_id_arr.v[1], &resource_id)) {
                    config->manifest_resource_id = resource_id;
                  } else {
                    lnk_error_cmd_switch(LNK_Error_Cmdl, cmd_switch, "unable to parse resource_id \"%S\"", res_id_arr.v[1]);
                  }
                } else {
                  lnk_error_cmd_switch(LNK_Error_Cmdl, cmd_switch, "invalid syntax expected form ID=resource_id but got \"%S\"", param_arr.v[1]);
                }
              } else {
                lnk_error_cmd_switch_invalid_param(LNK_Error_Cmdl, cmd_switch, param_arr.v[0]);
              }
            } else {
              lnk_error_cmd_switch_invalid_param_count(LNK_Error_Cmdl, cmd_switch);
            }
          } else if (str8_match(param_arr.v[0], str8_lit("no"), StringMatchFlag_CaseInsensitive)) {
            config->manifest_opt = LNK_ManifestOpt_No;
          } else {
            lnk_error_cmd_switch_invalid_param(LNK_Error_Cmdl, cmd_switch, param_arr.v[0]);
          }
        } else {
          lnk_error_cmd_switch_invalid_param_count(LNK_Error_Cmdl, cmd_switch);
        }
      } else if (cmd->value_strings.node_count == 0) {
        config->manifest_opt = LNK_ManifestOpt_Embed;
      }
    } break;

    case LNK_CmdSwitch_ManifestDependency: {
      String8List manifest_dependency_list = str8_list_copy(arena, &cmd->value_strings);
      str8_list_concat_in_place(&config->manifest_dependency_list, &manifest_dependency_list);
    } break;

    case LNK_CmdSwitch_ManifestFile: {
      lnk_cmd_switch_parse_string_copy(arena, cmd->value_strings, cmd_switch, &config->manifest_name);
    } break;

    case LNK_CmdSwitch_ManifestInput: {
      // see :manifest_input
    } break;

    case LNK_CmdSwitch_ManifestUac: {
      if (cmd->value_strings.node_count == 1) {
        String8 uac = lnk_error_check_and_strip_quotes(LNK_Error_Cmdl, cmd_switch, cmd->value_strings.first->string);
        String8List  param_list = str8_split_by_string_chars(scratch.arena, uac, str8_lit(" "), 0);
        String8Array param_arr  = str8_array_from_list(scratch.arena, &param_list);
        if (param_arr.count > 0) {
          if (str8_match(str8_lit("level="), param_arr.v[0], StringMatchFlag_RightSideSloppy|StringMatchFlag_CaseInsensitive)) {
            String8 level_param = param_arr.v[0];
            String8List level_list = str8_split_by_string_chars(scratch.arena, level_param, str8_lit("="), 0);
            if (level_list.node_count == 2) {
              if (str8_match(level_list.first->string, str8_lit("level"), StringMatchFlag_CaseInsensitive)) {
                String8 level = level_list.last->string;
                if (str8_match(level, str8_lit("'asInvoker'"), 0) ||
                    str8_match(level, str8_lit("'highestAvailable'"), 0) ||
                    str8_match(level, str8_lit("'requireAdministrator'"), 0)) {
                  // manifest level was parsed!
                  config->manifest_uac = 1;
                  config->manifest_level = push_str8_copy(arena, level);
                  if (param_arr.count > 1) {
                    String8 ui_access_param = param_arr.v[1];
                    String8List ui_access_list = str8_split_by_string_chars(scratch.arena, ui_access_param, str8_lit("="), 0);
                    if (ui_access_list.node_count == 2) {
                      String8 ui_access = ui_access_list.last->string;
                      if (str8_match(ui_access, str8_lit("'true'"), 0) ||
                          str8_match(ui_access, str8_lit("'false'"), 0)) {
                        // ui access was parsed!
                        config->manifest_ui_access = push_str8_copy(arena, ui_access);
                      } else {
                        lnk_error_invalid_uac_ui_access_param(LNK_Error_Cmdl, cmd_switch, ui_access_param);
                      }
                    } else {
                      lnk_error_invalid_uac_ui_access_param(LNK_Error_Cmdl, cmd_switch, ui_access_param);
                    }
                  }
                } else {
                  lnk_error_invalid_uac_level_param(LNK_Error_Cmdl, cmd_switch, level_param);
                }
              } else {
                lnk_error_invalid_uac_level_param(LNK_Error_Cmdl, cmd_switch, level_param);
              }
            } else {
              lnk_error_invalid_uac_level_param(LNK_Error_Cmdl, cmd_switch, level_param);
            }
          } else if (str8_match(str8_lit("no"), param_arr.v[0], StringMatchFlag_CaseInsensitive)) {
            config->manifest_uac = 0;
          } else {
            lnk_error_cmd_switch_invalid_param(LNK_Error_Cmdl, cmd_switch, param_arr.v[0]);
          }
        } else {
          lnk_error_cmd_switch(LNK_Warning_Cmdl, cmd_switch, "empty param string");
        }
      } else {
        lnk_error_cmd_switch_invalid_param_count(LNK_Error_Cmdl, cmd_switch);
      }
    } break;

    case LNK_CmdSwitch_Natvis: {
      // warn about invalid natvis extension
      for (String8Node *node = cmd->value_strings.first; node != 0; node = node->next) {
        String8 ext = str8_skip_last_dot(node->string);
        if (!str8_match(ext, str8_lit("natvis"), StringMatchFlag_CaseInsensitive)) {
          lnk_error_cmd_switch(LNK_Warning_InvalidNatvisFileExt, cmd_switch, "Visual Studio expects .natvis extension: \"%S\"", node->string);
        }
      }

      String8List natvis_list = str8_list_copy(arena, &cmd->value_strings);
      str8_list_concat_in_place(&config->natvis_list, &natvis_list);
    } break;

    case LNK_CmdSwitch_NoDefaultLib: {
      String8List no_default_lib_list = str8_list_copy(arena, &cmd->value_strings);
      str8_list_concat_in_place(&config->disallow_lib_list, &no_default_lib_list);
    } break;

    case LNK_CmdSwitch_NoLogo: {
      // we don't print logo
    } break;

    case LNK_CmdSwitch_NxCompat: {
      lnk_cmd_switch_set_flag_16(cmd->value_strings, cmd_switch, &config->dll_characteristics, PE_DllCharacteristic_NX_COMPAT);
    } break;

    case LNK_CmdSwitch_Opt: {
      for (String8Node *n = cmd->value_strings.first; n != 0; n = n->next) {
        String8 param = n->string;
        if (str8_match(param, str8_lit("ref"), StringMatchFlag_CaseInsensitive)) {
          config->opt_ref = LNK_SwitchState_Yes; 
        } else if (str8_match(param, str8_lit("noref"), StringMatchFlag_CaseInsensitive)) {
          config->opt_ref = LNK_SwitchState_No;
        } else if (str8_match(param, str8_lit("icf"), StringMatchFlag_CaseInsensitive) ||
                   str8_match(param, str8_lit("icf="), StringMatchFlag_CaseInsensitive | StringMatchFlag_RightSideSloppy)) {
          String8List vals = str8_split_by_string_chars(scratch.arena, param, str8_lit("="), 0);
          if (vals.node_count > 2) {
            lnk_error_cmd_switch(LNK_Error_Cmdl, cmd_switch, "too many parameters for iteration");
            continue;
          }
          if (vals.node_count == 2) {
            B32 is_parsed = try_u64_from_str8_c_rules(vals.last->string, &config->opt_iter_count);
            if (!is_parsed) {
              lnk_error_cmd_switch(LNK_Error_Cmdl, cmd_switch, "unable to parse iterations \"%S\"", vals.last->string);
              continue;
            }
          }
          config->opt_icf = LNK_SwitchState_Yes;
        } else if (str8_match(param, str8_lit("noicf"), StringMatchFlag_CaseInsensitive)) {
          config->opt_icf = LNK_SwitchState_No;
        } else if (str8_match(param, str8_lit("lbr"), StringMatchFlag_CaseInsensitive)) {
          config->opt_lbr = LNK_SwitchState_Yes;
        } else if (str8_match(param, str8_lit("nolibr"), StringMatchFlag_CaseInsensitive)) {
          config->opt_lbr = LNK_SwitchState_No;
        } else {
          lnk_error_cmd_switch(LNK_Error_Cmdl, cmd_switch, "unknown option \"%S\"", param);
        }
      }
    } break;

    case LNK_CmdSwitch_Out: {
      lnk_cmd_switch_parse_string_copy(arena, cmd->value_strings, cmd_switch, &config->image_name);
    } break;

    case LNK_CmdSwitch_Pdb: {
      lnk_cmd_switch_parse_string_copy(arena, cmd->value_strings, cmd_switch, &config->pdb_name);
    } break;

    case LNK_CmdSwitch_PdbPageSize: {
      U64 page_size;
      if (lnk_cmd_switch_parse_u64(cmd->value_strings, cmd_switch, &page_size, LNK_ParseU64Flag_CheckPow2)) {
        if (page_size >= MSF_MIN_PAGE_SIZE) {
          if (page_size < MSF_MAX_PAGE_SIZE) {
            config->pdb_page_size = page_size;
          } else {
            lnk_error_cmd_switch(LNK_Error_Cmdl, cmd_switch, "page size must be <= %u bytes", MSF_MAX_PAGE_SIZE);
          }
        } else {
          lnk_error_cmd_switch(LNK_Error_Cmdl, cmd_switch, "page size must be >= %u bytes", MSF_MIN_PAGE_SIZE);
        }
      }
    } break;

    case LNK_CmdSwitch_Release: {
      if (cmd->value_strings.node_count == 0) {
        config->flags |= LNK_ConfigFlag_WriteImageChecksum;
      } else {
        lnk_error_cmd_switch(LNK_Error_Cmdl, cmd_switch, "invalid number of parameters");
      }
    } break;

    case LNK_CmdSwitch_Stack: {
      Rng1U64 reserve_commit;
      reserve_commit.v[0] = config->stack_reserve;
      reserve_commit.v[1] = config->stack_commit;
      if (lnk_cmd_switch_parse_tuple(cmd->value_strings, cmd_switch, &reserve_commit)) {
        if (reserve_commit.v[0] >= reserve_commit.v[1]) {
          U64 reserve_aligned = AlignPow2(reserve_commit.v[0], 4);
          U64 commit_aligned = AlignPow2(reserve_commit.v[1], 4);
#if 0
          if (reserve_aligned != reserve_commit.v[0]) {
            lnk_error_cmd_switch(LNK_Warning_Cmdl, cmd_switch, "reserve is not power of two, aligned to %u", reserve_aligned);
          }
          if (commit_aligned != reserve_commit.v[1]) {
            lnk_error_cmd_switch(LNK_Warning_Cmdl, cmd_switch, "commit is not power of two, aligned to %u", commit_aligned);
          }
#endif
          config->stack_reserve = reserve_aligned;
          config->stack_commit = commit_aligned;
        } else {
          lnk_error_cmd_switch(LNK_Error_Cmdl, cmd_switch, "commit(%llu) is greater than reserve(%llu)", reserve_commit.v[1], reserve_commit.v[0]);
        }
      }
    } break;

    case LNK_CmdSwitch_SubSystem: {
      if (cmd->value_strings.node_count <= 2 && cmd->value_strings.node_count > 0) {
        // set subsystem type
        PE_WindowsSubsystem subsystem = pe_subsystem_from_string(cmd->value_strings.first->string);
        if (subsystem != PE_WindowsSubsystem_UNKNOWN) {
          config->subsystem = subsystem;

          // parse version (optional)
          if (cmd->value_strings.node_count == 2) {
            String8List value_strings = cmd->value_strings;
            str8_list_pop_front(&value_strings); // pop subsystem parameter
            lnk_cmd_switch_parse_version(value_strings, cmd_switch, &config->subsystem_ver);
          }
        } else {
          lnk_error_cmd_switch(LNK_Error_Cmdl, cmd_switch, "invalid subsystem \"%S\"", cmd->value_strings.first->string);
        }
      } else {
        lnk_error_cmd_switch(LNK_Error_Cmdl, cmd_switch, "invalid number of parameters");
      }
    } break;

    case LNK_CmdSwitch_Time: {
    } break;

    case LNK_CmdSwitch_TsAware: {
      lnk_cmd_switch_set_flag_inv_64(cmd->value_strings, cmd_switch, &config->flags, LNK_ConfigFlag_NoTsAware);
    } break;

    case LNK_CmdSwitch_Version: {
      lnk_cmd_switch_parse_version(cmd->value_strings, cmd_switch, &config->image_ver);
    } break;

    case LNK_CmdSwitch_Rad_Age: {
      lnk_cmd_switch_parse_u32(cmd->value_strings, cmd_switch, &config->age, 0);
    } break;

    case LNK_CmdSwitch_Rad_BuildInfo: {
      lnk_print_build_info();
      os_abort(0);
    } break;

    case LNK_CmdSwitch_Rad_CheckUnusedDelayLoadDll: {
      lnk_cmd_switch_set_flag_64(cmd->value_strings, cmd_switch, &config->flags, LNK_ConfigFlag_CheckUnusedDelayLoadDll);
    } break;

    case LNK_CmdSwitch_Rad_Debug: {
      lnk_cmd_switch_parse_flag(cmd->value_strings, cmd_switch, &config->rad_debug);
    } break;
    case LNK_CmdSwitch_Rad_DebugName: {
      lnk_cmd_switch_parse_string_copy(arena, cmd->value_strings, cmd_switch, &config->rad_debug_name);
    } break;

    case LNK_CmdSwitch_Rad_DelayBind: {
      lnk_cmd_switch_set_flag_64(cmd->value_strings, cmd_switch, &config->flags, LNK_ConfigFlag_DelayBind);
    } break;

    case LNK_CmdSwitch_Rad_DeleteManifest: {
      lnk_cmd_switch_parse_flag(cmd->value_strings, cmd_switch, &config->delete_manifest);
    } break;

    case LNK_CmdSwitch_Rad_DoMerge: {
      lnk_cmd_switch_set_flag_64(cmd->value_strings, cmd_switch, &config->flags, LNK_ConfigFlag_Merge);
    } break;

    case LNK_CmdSwitch_Rad_EnvLib: {
      lnk_cmd_switch_set_flag_64(cmd->value_strings, cmd_switch, &config->flags, LNK_ConfigFlag_EnvLib);
    } break;

    case LNK_CmdSwitch_Rad_Exe: {
      lnk_cmd_switch_set_flag_16(cmd->value_strings, cmd_switch, &config->file_characteristics, PE_ImageFileCharacteristic_EXE);
    } break;

    case LNK_CmdSwitch_Rad_Guid: {
      if (cmd->value_strings.node_count == 1) {
        if (str8_match(cmd->value_strings.first->string, str8_lit("imageblake3"), StringMatchFlag_CaseInsensitive)) {
          config->guid_type = Lnk_DebugInfoGuid_ImageBlake3;
        } else if (str8_match(cmd->value_strings.first->string, str8_lit("random"), StringMatchFlag_CaseInsensitive)) {
          config->guid = os_make_guid();
        } else {
          OS_Guid guid;
          if (os_try_guid_from_string(cmd->value_strings.first->string, &guid)) {
            config->guid = guid;
          } else {
            lnk_error_cmd_switch(LNK_Error_Cmdl, cmd_switch, "unable to parse \"%S\"", cmd->value_strings.first->string);
          }
        }
      } else {
        lnk_error_cmd_switch(LNK_Error_Cmdl, cmd_switch, "invalid number of parameters, expected GUID formatted as following: \"0000000-0000-0000-0000-000000000000\"");
      }
    } break;

    case LNK_CmdSwitch_Rad_LargePages: {
      if (cmd->value_strings.node_count == 0) {
        OS_ProcessInfo *process_info = os_get_process_info();
        if (process_info->large_pages_allowed) {
          arena_default_flags |= ArenaFlag_LargePages;
        } else {
          lnk_error_cmd_switch(LNK_Warning_LargePages, cmd_switch, "Large pages aren't enabled on this system.");
#if OS_WINDOWS
          lnk_supplement_error("To enable large pages:");
          lnk_supplement_error("\t- Press Win+R and open \"gpedit.msc\"");
          lnk_supplement_error("\t- Navigate to Local Computer Policy > Computer Configuration > Windows Settings > Security Settings > Local Policies > User Rights And Assignments");
          lnk_supplement_error("\t- Double-click on \"Lock pages in memory\"");
          lnk_supplement_error("\t- Click \"Add User or Group...\"");
          lnk_supplement_error("\t- Type in your user name");
          lnk_supplement_error("\t- Click Oks and reboot the machine");
#endif
        }
      } else if (cmd->value_strings.node_count == 1) {
        if (str8_match(cmd->value_strings.first->string, str8_lit("quiet"), StringMatchFlag_CaseInsensitive)) {
          OS_ProcessInfo *process_info = os_get_process_info();
          if (process_info->large_pages_allowed) {
            arena_default_flags |= ArenaFlag_LargePages;
          }
        } else if (str8_match(cmd->value_strings.first->string, str8_lit("no"), StringMatchFlag_CaseInsensitive)) {
          arena_default_flags &= ~ArenaFlag_LargePages;
        } else {
          lnk_error_cmd_switch(LNK_Error_Cmdl, cmd_switch, "invalid parameter: \"%S\", expected NO or QUIET", cmd->value_strings.first->string);
        }
      } else {
        lnk_error_cmd_switch(LNK_Error_Cmdl, cmd_switch, "invalid number of parameters");
      }
    } break;

    case LNK_CmdSwitch_Rad_LinkVer: {
      lnk_cmd_switch_parse_version(cmd->value_strings, cmd_switch, &config->link_ver);
    } break;

    case LNK_CmdSwitch_Rad_Log: {
      if (cmd->value_strings.node_count == 1) {
        if (str8_match(cmd->value_strings.first->string, str8_lit("all"), StringMatchFlag_CaseInsensitive)) {
          for (U64 ilog = 0; ilog < LNK_Log_Count; ilog += 1) {
            lnk_set_log_status((LNK_LogType)ilog, 1);
          }
        } else {
          LNK_LogType log_type = lnk_log_type_from_string(cmd->value_strings.first->string);
          if (log_type == LNK_Log_Null) {
            lnk_error_cmd_switch(LNK_Error_Cmdl, cmd_switch, "unknown parameter \"%S\"", cmd->value_strings.first->string);
          } else {
            lnk_set_log_status(log_type, 1);
          }
        }
      } else {
        lnk_error_cmd_switch(LNK_Error_Cmdl, cmd_switch, "invalid number of parameters, expected 1");
      }
    } break;

    case LNK_CmdSwitch_Rad_MtPath: {
      lnk_cmd_switch_parse_string_copy(arena, cmd->value_strings, cmd_switch, &config->mt_path);
    } break;

    case LNK_CmdSwitch_Rad_OsVer: {
      lnk_cmd_switch_parse_version(cmd->value_strings, cmd_switch, &config->os_ver);
    } break;

    case LNK_CmdSwitch_Rad_PageSize: {
      lnk_cmd_switch_parse_u64(cmd->value_strings, cmd_switch, &config->page_size, 0);
    } break;

    case LNK_CmdSwitch_Rad_PathStyle: {
      if (cmd->value_strings.node_count == 1) {
        PathStyle path_style = path_style_from_string(str8_list_first(&cmd->value_strings));
        if (path_style != PathStyle_Null) {
          config->path_style = path_style;
        } else {
          lnk_error_cmd_switch(LNK_Error_Cmdl, cmd_switch, "unable to parse parameter \"%S\"", cmd->value_strings.first->string);
        }
      } else {
        lnk_error_cmd_switch(LNK_Error_Cmdl, cmd_switch, "invalid number of parameters");
      }
    } break;

    case LNK_CmdSwitch_Rad_SectVirtOff: {
      U64 sect_virt_off;
      if (lnk_cmd_switch_parse_u64(cmd->value_strings, cmd_switch, &sect_virt_off, LNK_ParseU64Flag_CheckUnder32bit)) {
        if (sect_virt_off >= 0x1000) {
          config->section_virt_off = sect_virt_off;
        } else {
          lnk_error_cmd_switch(LNK_Error_Cmdl, cmd_switch, "section virtual offset must be >= 0x1000");
        }
      }
    } break;

    case LNK_CmdSwitch_Rad_SuppressError: {
      U64List error_code_list = {0};
      if (lnk_cmd_switch_parse_u64_list(scratch.arena, cmd->value_strings, cmd_switch, &error_code_list, 0)) {
        for (U64Node *error_code_n = error_code_list.first; error_code_n != 0; error_code_n = error_code_n->next) {
          if (error_code_n->data < LNK_Error_Count) {
            lnk_suppress_error(error_code_n->data);
          } else {
            lnk_error_cmd_switch(LNK_Warning_Cmdl, cmd_switch, "unknown error code %llu", error_code_n->data);
          }
        }
      }
    } break;

    case LNK_CmdSwitch_Rad_SymbolTableCapDefined: {
      lnk_cmd_switch_parse_u64(cmd->value_strings, cmd_switch, &config->symbol_table_cap_defined, 0);
    } break;
    case LNK_CmdSwitch_Rad_SymbolTableCapInternal: {
      lnk_cmd_switch_parse_u64(cmd->value_strings, cmd_switch, &config->symbol_table_cap_internal, 0);
    } break;
    case LNK_CmdSwitch_Rad_SymbolTableCapWeak: {
      lnk_cmd_switch_parse_u64(cmd->value_strings, cmd_switch, &config->symbol_table_cap_weak, 0);
    } break;
    case LNK_CmdSwitch_Rad_SymbolTableCapLib: {
      lnk_cmd_switch_parse_u64(cmd->value_strings, cmd_switch, &config->symbol_table_cap_lib, 0);
    } break;

    case LNK_CmdSwitch_Rad_TargetOs: {
      if (cmd->value_strings.node_count == 1) {
        String8 os_string = str8_list_first(&cmd->value_strings);
        OperatingSystem target_os = operating_system_from_string(os_string);
        if (target_os != OperatingSystem_Null) {
          config->target_os = target_os;
        } else {
          lnk_error_cmd_switch(LNK_Error_Cmdl, cmd_switch, "unknown operating system type %S", os_string);
        }
      } else {
        lnk_error_cmd_switch(LNK_Warning_Cmdl, cmd_switch, "expected 1 parameter");
      }
    } break;

    case LNK_CmdSwitch_Rad_TimeStamp: {
      lnk_cmd_switch_parse_u32(cmd->value_strings, cmd_switch, &config->time_stamp, 0);
    } break;

    case LNK_CmdSwitch_Rad_Version: {
      fprintf(stdout, "%s\n", BUILD_TITLE);
      os_abort(0);
    } break;

    case LNK_CmdSwitch_Rad_Workers: {
      U64 worker_count;
      if (lnk_cmd_switch_parse_u64(cmd->value_strings, cmd_switch, &worker_count, 0)) {
        config->worker_count = worker_count;
      }
    } break;

    case LNK_CmdSwitch_Help: {
      lnk_print_help();
      os_abort(0);
    } break;
    }
  }

  // :manifest_input
  if (lnk_cmd_line_has_switch(cmd_line, LNK_CmdSwitch_ManifestInput)) {
    switch (config->manifest_opt) {
    case LNK_ManifestOpt_Null: {
      lnk_error_cmd_switch(LNK_Error_Cmdl, LNK_CmdSwitch_ManifestInput, "missing /MANIFEST:EMBED");
    } break;
    case LNK_ManifestOpt_No: {
      lnk_error_cmd_switch(LNK_Warning_Cmdl, LNK_CmdSwitch_ManifestInput, "missing /MANIFEST:EMBED, ignoring inputs");
    } break;
    case LNK_ManifestOpt_Embed: {
      for (LNK_CmdOption *cmd = cmd_line.first_option; cmd != 0; cmd = cmd->next) {
        LNK_CmdSwitchType cmd_switch = lnk_cmd_switch_type_from_string(cmd->string);
        if (cmd_switch == LNK_CmdSwitch_ManifestInput) {
          String8List manifest_list = str8_list_copy(arena, &cmd->value_strings);
          str8_list_concat_in_place(&config->input_list[LNK_Input_Manifest], &manifest_list);
        }
      }
    } break;
    }
  }

  // input files
  for (String8Node *input_node = cmd_line.input_list.first; input_node != 0; input_node = input_node->next) {
    String8 path = push_str8_copy(arena, input_node->string);
    String8 ext = str8_skip_last_dot(path);

    // map file extension to input type
    LNK_InputType input_type = lnk_input_type_from_string(ext);

    // do we support this file format?
    if (input_type == LNK_Input_Null) {
      lnk_error(LNK_Error_Cmdl, "unknown file format \"%S\"", path);
      continue;
    }

    // psuh file path
    str8_list_push(arena, &config->input_list[input_type], path);
  }

  // os version and subsystem are always same?
  if (!lnk_cmd_line_has_switch(cmd_line, LNK_CmdSwitch_Rad_OsVer)) {
    config->os_ver = config->subsystem_ver;
  }
  
  // don't emit bind table with /ALLOWBIND:NO
  if (config->dll_characteristics & PE_DllCharacteristic_NO_BIND) {
    config->flags &= ~LNK_ConfigFlag_DelayBind;
  }
  
  // gather lib paths from enviroment string
  if (config->flags & LNK_ConfigFlag_EnvLib) {
#if OS_WINDOWS
    OS_ProcessInfo *process_info = os_get_process_info();
    for (String8Node *node = process_info->environment.first; node != 0; node = node->next) {
      String8List var_list = str8_split_by_string_chars(scratch.arena, node->string, str8_lit("="), 0);
      if (var_list.node_count != 2) {
        continue;
      }
      String8 key = var_list.first->string;
      String8 val = var_list.last->string;
      if (str8_match(key, str8_lit("Lib"), StringMatchFlag_CaseInsensitive) || 
          str8_match(key, str8_lit("LibPath"), StringMatchFlag_CaseInsensitive)) {
        String8List val_list = str8_split_by_string_chars(scratch.arena, val, str8_lit(";"), 0);
        String8List val_list_copy = str8_list_copy(arena, &val_list);
        str8_list_concat_in_place(&config->lib_dir_list, &val_list_copy);
      }
    }
#endif
  }
  
  // set flags for /OPT
  {
    // these flags remove and merge inline functions and methods defined in class,
    // and makes stepping tougher, in debug mode we don't link with these optimizations
    // unless user specifically orverrides.
    if (config->debug_mode != LNK_DebugMode_None) {
      if (config->opt_ref == LNK_SwitchState_Null) {
        config->opt_ref = LNK_SwitchState_No;
      }
      if (config->opt_icf == LNK_SwitchState_Null) {
        config->opt_icf = LNK_SwitchState_No;
      }
    }
    
    // by default enable all optimizations
    if (config->opt_ref == LNK_SwitchState_Null) {
      config->opt_ref = LNK_SwitchState_Yes;
    }
    if (config->opt_icf == LNK_SwitchState_Null) {
      config->opt_icf = LNK_SwitchState_Yes;
    }
    if (config->opt_lbr == LNK_SwitchState_Null) {
      config->opt_lbr = LNK_SwitchState_Yes;
    }
  }
  
  // error check base address flags
  if (config->flags & LNK_ConfigFlag_Fixed) {
    if (lnk_cmd_line_has_switch(cmd_line, LNK_CmdSwitch_DynamicBase)) {
      B32 is_dynamic_base_set = !!(config->dll_characteristics & PE_DllCharacteristic_DYNAMIC_BASE);
      if (is_dynamic_base_set) {
        lnk_error(LNK_Error_IncomatibleCmdOptions, "unable to link with /FIXED and /DYNAMICBASE at the same time");
      }
    }
  }
  
  // set flags for /FIXED
  if (config->flags & LNK_ConfigFlag_Fixed) {
    config->file_characteristics |= PE_ImageFileCharacteristic_STRIPPED;
    config->dll_characteristics &= ~PE_DllCharacteristic_DYNAMIC_BASE;
  }
  
  // set flag for /guard
  if (config->guard_flags != LNK_Guard_None) {
    config->dll_characteristics |= PE_DllCharacteristic_GUARD_CF;
  }

  // handle empty /OUT
  if (!lnk_cmd_line_has_switch(cmd_line, LNK_CmdSwitch_Out)) {
    String8 name = str8_list_first(&config->input_list[LNK_Input_Obj]);
    String8 ext  = (config->file_characteristics & PE_ImageFileCharacteristic_FILE_DLL) ? str8_lit("dll") : str8_lit("exe");
    config->image_name = make_file_path_with_ext(scratch.arena, name, ext);
  }
  config->image_name = os_make_full_path(arena, config->image_name);

  // handle empty /PDB
  if (!lnk_cmd_line_has_switch(cmd_line, LNK_CmdSwitch_Pdb)) {
    config->pdb_name = make_file_path_with_ext(arena, config->image_name, str8_lit("pdb"));
  }
  config->pdb_name = os_make_full_path(arena, config->pdb_name);

  // handle empty /RAD_DEBUG_NAME
  if (!lnk_cmd_line_has_switch(cmd_line, LNK_CmdSwitch_Rad_DebugName)) {
    config->rad_debug_name = make_file_name_with_ext(arena, config->image_name, str8_lit("rdi"));
  }
  config->rad_debug_name = os_make_full_path(arena, config->rad_debug_name);

  // handle empty /IMPLIB
  if (!lnk_cmd_line_has_switch(cmd_line, LNK_CmdSwitch_ImpLib)) {
    config->imp_lib_name = make_file_name_with_ext(arena, config->image_name, str8_lit("lib"));
  }
  config->imp_lib_name = os_make_full_path(arena, config->imp_lib_name);

  // handle empty /MANIFESTFILE
  if (!lnk_cmd_line_has_switch(cmd_line, LNK_CmdSwitch_ManifestFile)) {
    config->manifest_name = make_file_path_with_ext(arena, config->image_name, str8_lit("manifest"));
  }

  if (lnk_get_log_status(LNK_Log_Debug)) {
    String8 full_cmd_line = str8_list_join(scratch.arena, &raw_cmd_line, &(StringJoin){ .sep = str8_lit_comp(" ") });
    lnk_log(LNK_Log_Debug, "--------------------------------------------------------------------------------");
    lnk_log(LNK_Log_Debug, "Command Line: %S", full_cmd_line);
    lnk_log(LNK_Log_Debug, "Work Dir    : %S", config->work_dir);
    lnk_log(LNK_Log_Debug, "--------------------------------------------------------------------------------");
  }

  scratch_end(scratch);
  ProfEnd();
  return config;
}

internal LNK_Config *
lnk_build_config(Arena *arena, int argc, char **argv)
{
  Temp scratch = scratch_begin(&arena, 1);

  String8List raw_cmd_line = os_string_list_from_argcv(arena, argc, argv);

#if OS_WINDOWS
  // remove exe name first argument
  str8_list_pop_front(&raw_cmd_line); 
#endif

  // init config
  LNK_Config *config = lnk_config_from_cmd_line(arena, raw_cmd_line);

  scratch_end(scratch);
  return config;
}

