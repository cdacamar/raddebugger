// Copyright (c) 2024 Epic Games Tools
// Licensed under the MIT license (https://opensource.org/license/mit/)

#ifndef EVAL_BUNDLES_H
#define EVAL_BUNDLES_H

////////////////////////////////
//~ rjf: Bundled Evaluation Path Types

typedef struct E_Eval E_Eval;
struct E_Eval
{
  E_Value value;
  E_Mode mode;
  E_Space space;
  E_TypeKey type_key;
  E_InterpretationCode code;
  E_MsgList msgs;
  U64 advance;
};

////////////////////////////////
//~ rjf: Bundled Evaluation Functions

internal E_Eval e_eval_from_string(Arena *arena, String8 string);
internal E_Eval e_autoresolved_eval_from_eval(E_Eval eval);
internal E_Eval e_dynamically_typed_eval_from_eval(E_Eval eval);
internal E_Eval e_value_eval_from_eval(E_Eval eval);
internal E_Eval e_element_eval_from_array_eval_index(E_Eval eval, U64 index);

#endif // EVAL_BUNDLES_H
