#ifndef LOCK_SOLE
#  define LOCK_SOLE

#  include "player.h"

#  define VERSION "v0.2.3"
#  define PROG "sole"

#  define GEND (~0)

#  define MAYBE(x)                              \
  ecode = (x);                                  \
  if (ERR(ecode)) {                             \
    displayerr(ecode);                          \
  }

typedef enum {
  CMOVUP,
  CMOVDOWN,
  CMOVLEFT,
  CMOVRIGHT,
} CmdMov;

/*
  NOTE: use this enum to change the keybindings.
        composite (aka C-, M- prefixed) keybindings
        are not possible at the moment
 */
enum keybindings {
  // -- BEGIN: mov-like -- //
  KEY_MOVEDOWN     = 'j',
  KEY_MOVEUP       = 'k',
  KEY_MOVELEFT     = 'h',
  KEY_MOVERIGHT    = 'l',
  // -- END: mov-like -- //

  // -- BEGIN: mark-like -- //
  KEY_MARK         = '\n',
  KEY_MARKSTACK    = 'm',
  KEY_DROP         = 'c',
  // -- END: mark-like -- //

  // -- BEGIN: play-like -- //
  KEY_PLAY         = 'p',
  KEY_PLAYBOTTOM   = 'b',
  KEY_PLAYALL      = 'a',
  KEY_PLAYSTACKCUR = 's',
  KEY_PLAYSTACKFIN = 'e',
  // -- END: play-like -- //

  // -- BEGIN: chfield-like -- //
  KEY_CHFIELD      = '\t',
  KEY_PREVFIELD    = 'f',
  KEY_PREVMARK     = 'r',
  // -- END: chfield-like -- //

  // -- BEGIN: take-like -- //
  KEY_TAKE         = ' ',
  // -- END: take-like -- //

  // -- BEGIN: quit-like -- //
  KEY_QUIT         = 'q',
  // -- END: quit-like -- //
};

#endif
