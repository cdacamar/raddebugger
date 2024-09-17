// Copyright (c) 2024 Epic Games Tools
// Licensed under the MIT license (https://opensource.org/license/mit/)

//- GENERATED CODE

#ifndef CTRL_META_H
#define CTRL_META_H

typedef enum CTRL_EntityKind
{
CTRL_EntityKind_Null,
CTRL_EntityKind_Root,
CTRL_EntityKind_Machine,
CTRL_EntityKind_Process,
CTRL_EntityKind_Thread,
CTRL_EntityKind_Module,
CTRL_EntityKind_EntryPoint,
CTRL_EntityKind_DebugInfoPath,
CTRL_EntityKind_COUNT,
} CTRL_EntityKind;

typedef enum CTRL_ExceptionCodeKind
{
CTRL_ExceptionCodeKind_Null,
CTRL_ExceptionCodeKind_Win32CtrlC,
CTRL_ExceptionCodeKind_Win32CtrlBreak,
CTRL_ExceptionCodeKind_Win32WinRTOriginateError,
CTRL_ExceptionCodeKind_Win32WinRTTransformError,
CTRL_ExceptionCodeKind_Win32RPCCallCancelled,
CTRL_ExceptionCodeKind_Win32DatatypeMisalignment,
CTRL_ExceptionCodeKind_Win32AccessViolation,
CTRL_ExceptionCodeKind_Win32InPageError,
CTRL_ExceptionCodeKind_Win32InvalidHandle,
CTRL_ExceptionCodeKind_Win32NotEnoughQuota,
CTRL_ExceptionCodeKind_Win32IllegalInstruction,
CTRL_ExceptionCodeKind_Win32CannotContinueException,
CTRL_ExceptionCodeKind_Win32InvalidExceptionDisposition,
CTRL_ExceptionCodeKind_Win32ArrayBoundsExceeded,
CTRL_ExceptionCodeKind_Win32FloatingPointDenormalOperand,
CTRL_ExceptionCodeKind_Win32FloatingPointDivisionByZero,
CTRL_ExceptionCodeKind_Win32FloatingPointInexactResult,
CTRL_ExceptionCodeKind_Win32FloatingPointInvalidOperation,
CTRL_ExceptionCodeKind_Win32FloatingPointOverflow,
CTRL_ExceptionCodeKind_Win32FloatingPointStackCheck,
CTRL_ExceptionCodeKind_Win32FloatingPointUnderflow,
CTRL_ExceptionCodeKind_Win32IntegerDivisionByZero,
CTRL_ExceptionCodeKind_Win32IntegerOverflow,
CTRL_ExceptionCodeKind_Win32PrivilegedInstruction,
CTRL_ExceptionCodeKind_Win32StackOverflow,
CTRL_ExceptionCodeKind_Win32UnableToLocateDLL,
CTRL_ExceptionCodeKind_Win32OrdinalNotFound,
CTRL_ExceptionCodeKind_Win32EntryPointNotFound,
CTRL_ExceptionCodeKind_Win32DLLInitializationFailed,
CTRL_ExceptionCodeKind_Win32FloatingPointSSEMultipleFaults,
CTRL_ExceptionCodeKind_Win32FloatingPointSSEMultipleTraps,
CTRL_ExceptionCodeKind_Win32AssertionFailed,
CTRL_ExceptionCodeKind_Win32ModuleNotFound,
CTRL_ExceptionCodeKind_Win32ProcedureNotFound,
CTRL_ExceptionCodeKind_Win32SanitizerErrorDetected,
CTRL_ExceptionCodeKind_Win32SanitizerRawAccessViolation,
CTRL_ExceptionCodeKind_Win32DirectXDebugLayer,
CTRL_ExceptionCodeKind_COUNT,
} CTRL_ExceptionCodeKind;

C_LINKAGE_BEGIN
extern String8 ctrl_entity_kind_display_string_table[8];
extern U32 ctrl_exception_code_kind_code_table[38];
extern String8 ctrl_exception_code_kind_display_string_table[38];
extern String8 ctrl_exception_code_kind_lowercase_code_string_table[38];
extern B8 ctrl_exception_code_kind_default_enable_table[38];

C_LINKAGE_END

#endif // CTRL_META_H
