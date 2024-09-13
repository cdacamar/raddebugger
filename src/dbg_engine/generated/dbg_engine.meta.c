// Copyright (c) 2024 Epic Games Tools
// Licensed under the MIT license (https://opensource.org/license/mit/)

//- GENERATED CODE

C_LINKAGE_BEGIN
String8 d_cfg_src_string_table[4] =
{
str8_lit_comp("user"),
str8_lit_comp("project"),
str8_lit_comp("command_line"),
str8_lit_comp("transient"),
};

D_ViewRuleSpecInfo d_core_view_rule_spec_info_table[21] =
{
{str8_lit_comp("default"), str8_lit_comp("Default"), str8_lit_comp(""), str8_lit_comp(""), (D_ViewRuleSpecInfoFlag_Inherited*0)|(D_ViewRuleSpecInfoFlag_Expandable*0)|(D_ViewRuleSpecInfoFlag_ExprResolution*0)|(D_ViewRuleSpecInfoFlag_VizBlockProd*1), },
{str8_lit_comp("array"), str8_lit_comp("Array"), str8_lit_comp("x:{expr}"), str8_lit_comp("Specifies that a pointer points to N elements, rather than only 1."), (D_ViewRuleSpecInfoFlag_Inherited*0)|(D_ViewRuleSpecInfoFlag_Expandable*0)|(D_ViewRuleSpecInfoFlag_ExprResolution*1)|(D_ViewRuleSpecInfoFlag_VizBlockProd*0), },
{str8_lit_comp("slice"), str8_lit_comp("Slice"), str8_lit_comp(""), str8_lit_comp("Specifies that a pointer within a struct, also containing an integer, points to the number of elements encoded by the integer."), (D_ViewRuleSpecInfoFlag_Inherited*0)|(D_ViewRuleSpecInfoFlag_Expandable*0)|(D_ViewRuleSpecInfoFlag_ExprResolution*1)|(D_ViewRuleSpecInfoFlag_VizBlockProd*0), },
{str8_lit_comp("list"), str8_lit_comp("List"), str8_lit_comp("x:{member}"), str8_lit_comp("Specifies that some struct, union, or class forms the top of a linked list, and the member which points at the following element in the list."), (D_ViewRuleSpecInfoFlag_Inherited*0)|(D_ViewRuleSpecInfoFlag_Expandable*0)|(D_ViewRuleSpecInfoFlag_ExprResolution*0)|(D_ViewRuleSpecInfoFlag_VizBlockProd*1), },
{str8_lit_comp("bswap"), str8_lit_comp("Byte Swap"), str8_lit_comp(""), str8_lit_comp("Specifies that all integral evaluations should be byte-swapped, such that their endianness is reversed."), (D_ViewRuleSpecInfoFlag_Inherited*1)|(D_ViewRuleSpecInfoFlag_Expandable*0)|(D_ViewRuleSpecInfoFlag_ExprResolution*1)|(D_ViewRuleSpecInfoFlag_VizBlockProd*0), },
{str8_lit_comp("cast"), str8_lit_comp("Cast"), str8_lit_comp("x:{type}"), str8_lit_comp("Specifies that the expression to which the view rule is applied should be casted to the provided type."), (D_ViewRuleSpecInfoFlag_Inherited*0)|(D_ViewRuleSpecInfoFlag_Expandable*0)|(D_ViewRuleSpecInfoFlag_ExprResolution*1)|(D_ViewRuleSpecInfoFlag_VizBlockProd*0), },
{str8_lit_comp("dec"), str8_lit_comp("Decimal Base (Base 10)"), str8_lit_comp(""), str8_lit_comp("Specifies that all integral evaluations should appear in base-10 form."), (D_ViewRuleSpecInfoFlag_Inherited*1)|(D_ViewRuleSpecInfoFlag_Expandable*0)|(D_ViewRuleSpecInfoFlag_ExprResolution*0)|(D_ViewRuleSpecInfoFlag_VizBlockProd*0), },
{str8_lit_comp("bin"), str8_lit_comp("Binary Base (Base 2)"), str8_lit_comp(""), str8_lit_comp("Specifies that all integral evaluations should appear in base-2 form."), (D_ViewRuleSpecInfoFlag_Inherited*1)|(D_ViewRuleSpecInfoFlag_Expandable*0)|(D_ViewRuleSpecInfoFlag_ExprResolution*0)|(D_ViewRuleSpecInfoFlag_VizBlockProd*0), },
{str8_lit_comp("oct"), str8_lit_comp("Octal Base (Base 8)"), str8_lit_comp(""), str8_lit_comp("Specifies that all integral evaluations should appear in base-8 form."), (D_ViewRuleSpecInfoFlag_Inherited*1)|(D_ViewRuleSpecInfoFlag_Expandable*0)|(D_ViewRuleSpecInfoFlag_ExprResolution*0)|(D_ViewRuleSpecInfoFlag_VizBlockProd*0), },
{str8_lit_comp("hex"), str8_lit_comp("Hexadecimal Base (Base 16)"), str8_lit_comp(""), str8_lit_comp("Specifies that all integral evaluations should appear in base-16 form."), (D_ViewRuleSpecInfoFlag_Inherited*1)|(D_ViewRuleSpecInfoFlag_Expandable*0)|(D_ViewRuleSpecInfoFlag_ExprResolution*0)|(D_ViewRuleSpecInfoFlag_VizBlockProd*0), },
{str8_lit_comp("only"), str8_lit_comp("Only Specified Members"), str8_lit_comp("x:{member}"), str8_lit_comp("Specifies that only the specified members should appear in struct, union, or class evaluations."), (D_ViewRuleSpecInfoFlag_Inherited*1)|(D_ViewRuleSpecInfoFlag_Expandable*0)|(D_ViewRuleSpecInfoFlag_ExprResolution*0)|(D_ViewRuleSpecInfoFlag_VizBlockProd*1), },
{str8_lit_comp("omit"), str8_lit_comp("Omit Specified Members"), str8_lit_comp("x:{member}"), str8_lit_comp("Omits a list of member names from appearing in struct, union, or class evaluations."), (D_ViewRuleSpecInfoFlag_Inherited*1)|(D_ViewRuleSpecInfoFlag_Expandable*0)|(D_ViewRuleSpecInfoFlag_ExprResolution*0)|(D_ViewRuleSpecInfoFlag_VizBlockProd*1), },
{str8_lit_comp("no_addr"), str8_lit_comp("Disable Address Values"), str8_lit_comp(""), str8_lit_comp("Displays only what pointers point to, if possible, without the pointer's address value."), (D_ViewRuleSpecInfoFlag_Inherited*1)|(D_ViewRuleSpecInfoFlag_Expandable*0)|(D_ViewRuleSpecInfoFlag_ExprResolution*0)|(D_ViewRuleSpecInfoFlag_VizBlockProd*0), },
{str8_lit_comp("checkbox"), str8_lit_comp("Checkbox"), str8_lit_comp(""), str8_lit_comp("Displays simple integer values as checkboxes, encoding zero or nonzero values."), (D_ViewRuleSpecInfoFlag_Inherited*0)|(D_ViewRuleSpecInfoFlag_Expandable*0)|(D_ViewRuleSpecInfoFlag_ExprResolution*0)|(D_ViewRuleSpecInfoFlag_VizBlockProd*0), },
{str8_lit_comp("color_rgba"), str8_lit_comp("Color (RGBA)"), str8_lit_comp(""), str8_lit_comp("Displays as a color, interpreting the data as encoding R, G, B, and A values."), (D_ViewRuleSpecInfoFlag_Inherited*0)|(D_ViewRuleSpecInfoFlag_Expandable*1)|(D_ViewRuleSpecInfoFlag_ExprResolution*0)|(D_ViewRuleSpecInfoFlag_VizBlockProd*1), },
{str8_lit_comp("text"), str8_lit_comp("Text"), str8_lit_comp("x:{'lang':lang, 'size':expr}"), str8_lit_comp("Displays as text."), (D_ViewRuleSpecInfoFlag_Inherited*0)|(D_ViewRuleSpecInfoFlag_Expandable*1)|(D_ViewRuleSpecInfoFlag_ExprResolution*0)|(D_ViewRuleSpecInfoFlag_VizBlockProd*1), },
{str8_lit_comp("disasm"), str8_lit_comp("Disassembly"), str8_lit_comp("x:{'arch':arch, 'size':expr}"), str8_lit_comp("Displays as disassembled instructions, interpreting the data as raw machine code."), (D_ViewRuleSpecInfoFlag_Inherited*0)|(D_ViewRuleSpecInfoFlag_Expandable*1)|(D_ViewRuleSpecInfoFlag_ExprResolution*0)|(D_ViewRuleSpecInfoFlag_VizBlockProd*1), },
{str8_lit_comp("memory"), str8_lit_comp("Memory"), str8_lit_comp("x:{'size':expr}"), str8_lit_comp("Displays as a raw memory grid."), (D_ViewRuleSpecInfoFlag_Inherited*0)|(D_ViewRuleSpecInfoFlag_Expandable*1)|(D_ViewRuleSpecInfoFlag_ExprResolution*0)|(D_ViewRuleSpecInfoFlag_VizBlockProd*1), },
{str8_lit_comp("graph"), str8_lit_comp("Graph"), str8_lit_comp(""), str8_lit_comp("Displays as a pointer graph, visualizing nodes and edges formed by pointers directly."), (D_ViewRuleSpecInfoFlag_Inherited*0)|(D_ViewRuleSpecInfoFlag_Expandable*1)|(D_ViewRuleSpecInfoFlag_ExprResolution*0)|(D_ViewRuleSpecInfoFlag_VizBlockProd*1), },
{str8_lit_comp("bitmap"), str8_lit_comp("Bitmap"), str8_lit_comp("x:{'w':expr, 'h':expr, 'fmt':tex2dformat}"), str8_lit_comp("Displays as a bitmap, interpreting the data as raw pixel data."), (D_ViewRuleSpecInfoFlag_Inherited*0)|(D_ViewRuleSpecInfoFlag_Expandable*1)|(D_ViewRuleSpecInfoFlag_ExprResolution*0)|(D_ViewRuleSpecInfoFlag_VizBlockProd*1), },
{str8_lit_comp("geo3d"), str8_lit_comp("Geometry (3D)"), str8_lit_comp("x:{'count':expr, 'vtx':expr, 'vtx_size':expr}"), str8_lit_comp("Displays as geometry, interpreting the data as index or vertex data."), (D_ViewRuleSpecInfoFlag_Inherited*0)|(D_ViewRuleSpecInfoFlag_Expandable*1)|(D_ViewRuleSpecInfoFlag_ExprResolution*0)|(D_ViewRuleSpecInfoFlag_VizBlockProd*1), },
};

C_LINKAGE_END

