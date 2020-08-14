#define PC9TOK_H
#ifndef PC9PAR_H
  #include "pc9par.h"
#endif

// Symbol objects for these names are returned by parsers matching these strings
#define Each_Symbolic(_) \
  _("int", k_int) _("char", k_char)                                     \
  _("float", k_float) _("double", k_double) _("struct", k_struct)       \
  _("auto", k_auto) _("extern", k_extern)                               \
  _("register", k_register) _("static", k_static)		        \
  _("goto", k_goto) _("return", k_return) _("sizeof", k_sizeof)         \
  _("break", k_break) _("continue", k_continue)                         \
  _("if", k_if) _("else", k_else)                                       \
  _("for", k_for) _("do", k_do) _("while", k_while)                     \
  _("switch", k_switch) _("case", k_case) _("default", k_default)       \
  /*_("entry", k_entry)*/                                               \
  _("*", o_star) _("++", o_plusplus) _("+", o_plus) _(".", o_dot)       \
  _("->", o_arrow) _("--", o_minusminus) _("-", o_minus)                \
  _("!=", o_ne) _("!", o_bang) _("~", o_tilde)				\
  _("&&", o_ampamp) _("&", o_amp) _("==", o_equalequal) _("=", o_equal) \
  _("^", o_caret) _("||", o_pipepipe) _("|", o_pipe)                    \
  _("/", o_slant) _("%", o_percent)                                     \
  _("<<", o_ltlt) _("<=", o_le) _("<", o_lt)                            \
  _(">>", o_gtgt) _(">=", o_ge) _(">", o_gt)				\
  _("(", lparen) _(")", rparen)                                         \
  _(",", comma) _(";", semi) _(":", colon) _("?", quest)                \
  _("{", lbrace) _("}", rbrace) _("[", lbrack) _("]", rbrack)           \
//End Each_Symbolic

#define Each_assignop(_) \
  _("+=", o_pluse) _("-=", o_minuse)                                  \
  _("*=", o_stare) _("/=", o_slante) _("%=", o_percente)		\
  _(">>=", o_gtgte) _("<<=", o_ltlte)                                 \
  _("&=", o_ampe) _("^=", o_carete) _("|=", o_pipee)

#define Each_C75_assignop(_) \
  _("=+", o_eplus) _("=-", o_eminus)                                  \
  _("=*", o_estar) _("=/", o_eslant) _("=%", o_epercent)		\
  _("=>>", o_egtgt) _("=<<", o_eltlt)                                 \
  _("=&", o_eamp) _("=^", o_ecaret) _("=|", o_epipe)


#define Enum_name(x,y) y ,

enum token_symbols {
  t_id = SYM2,
  c_int, c_float, c_char, c_string,
  Each_Symbolic( Enum_name )
  Each_assignop( Enum_name )
  Each_C75_assignop( Enum_name )
  SYM3
};

typedef enum language { C75, C78, C90, C99, C11, C2X } language;

list tokens_from_chars( language lang, object v );

int tok_main( void );
