#ifndef LOCK_SOLE
#  define LOCK_SOLE

#  include "player.h"

#  define VERSION "v0.2.2"
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
  KEY_MOVEDOWN    = 'j',
  KEY_MOVEUP      = 'k',
  KEY_MOVELEFT    = 'h',
  KEY_MOVERIGHT   = 'l',

  KEY_MARK        = '\n',
  KEY_MARKSTACK   = 'm',
  KEY_DROP        = 'c',

  KEY_PLAY        = 'p',
  KEY_PLAYBOTTOM  = 'b',
  KEY_PLAYALL     = 'a',
  KEY_PLAYSTACK   = 's',

  KEY_CHFIELD     = '\t',
  KEY_PREVFIELD   = 'q',
  KEY_PREVMARK    = 't',

  KEY_TAKE        = ' ',
};

#endif
