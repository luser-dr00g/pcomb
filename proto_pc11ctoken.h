#define PC11CTOKEN_H
#if  ! PC11PARSER_H
  #include "pc11parser.h"
#endif

#define Each_Symbolic(_) \
  _("int", kw_int) _("char", kw_char) \
  _("float", kw_float) _("double", kw_double) _("struct", kw_struct) \
  _("auto", kw_auto) _("extern", kw_extern) \
  _("register", kw_register) _("static", kw_static) \
  _("goto", kw_goto) _("return", kw_return) _("sizeof", kw_sizeof) \
  _("break", kw_break) _("continue", kw_continue) \
  _("if", kw_if) _("else", kw_else) \
  _("for", kw_for) _("do", kw_do) _("while", kw_while) \
  _("switch", kw_switch) _("case", kw_case) _("default", kw_default) \
  /*_("entry", kw_entry) */ \
  _("*", op_star) _("++", op_inc) _("+", op_plus) _(".", op_dot) \
  _("->", op_arrow) _("--", op_dec) _("-", op_minus) \
  _("!=", op_neq) _("!", op_bang) _("~", op_tilde) \
  _("&&", op_andand) _("&", op_and) _("==", op_eqeq) _("=", op_eq) \
  _("^", op_caret) _("||", op_oror) _("|", op_or) \
  _("/", op_slant) _("%", op_percent) \
  _("<<", op_left) _("<=", op_leq) _("<", op_lt) \
  _(">>", op_right) _(">=", op_geq) _(">", op_gt) \
  _("(", lpar) _(")", rpar) \
  _(",", comma) _(";", semi) _(":", colon) _("?", quest) \
  _("{", lbrace) _("}", rbrace) _("[", lbrack) _("]", rbrack) \
//end Each_Symbolic
