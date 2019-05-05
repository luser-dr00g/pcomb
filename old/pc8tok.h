//pc8tok.h Tokenizer
#include "pc8par.h"

#define Each_Symbolic(_) \
  _("int", k_int) _("char", k_char) _("float", k_float) _("double", k_double) _("struct", k_struct) \
  _("auto", k_auto) _("extern", k_extern) _("register", k_register) _("static", k_static) \
  _("goto", k_goto) _("return", k_return) _("sizeof", k_sizeof) \
  _("break", k_break) _("continue", k_continue) \
  _("if", k_if) _("else", k_else) \
  _("for", k_for) _("do", k_do) _("while", k_while) \
  _("switch", k_switch) _("case", k_case) _("default", k_default) \
  _("entry", k_entry) \
  _("*", o_star) _("++", o_plusplus) _("+", o_plus) _(".", o_dot) \
  _("->", o_arrow) _("--", o_minusminus) _("-", o_minus) _("!=", o_ne) _("!", o_bang) _("~", o_tilde) \
  _("&&", o_ampamp) _("&", o_amp) _("==", o_equalequal) _("=", o_equal) \
  _("^", o_caret) _("||", o_pipepipe) _("|", o_pipe) \
  _("/", o_slant) _("%", o_percent) \
  _("(", lparen) _(")", rparen) _(",", comma) _(";", semi) _(":", colon) _("?", quest) \
  _("{", lbrace) _("}", rbrace) _("[", lbrack) _("]", rbrack) \
  _("<<", o_ltlt) _("<=", o_le) _("<", o_lt) _(">>", o_gtgt) _(">=", o_ge) _(">", o_gt) \
  _("=+", o_eplus) _("=-", o_eminus) _("=*", o_estar) _("=/", o_eslant) _("=%", o_epercent) \
  _("=>>", o_egtgt) _("=<<", o_eltlt) _("=&", o_eamp) _("=^", o_ecaret) _("=|", o_epipe) \
//End Symbolic
#define Enum_name(x,y) y ,

enum token_symbols {
  t_id = SYM2,
  c_int, c_float, c_char, c_string,
  Each_Symbolic( Enum_name )
  SYM2_, SYM3 = NEXT10(SYM2_)
};

list tokens_from_chars( void *v );
