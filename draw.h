#ifndef LOCK_DRAW
#  define LOCK_DRAW

#  include "utils.h"
#  include "player.h"

#  define ESCSIZE (8)                      // \033[..;..m
#  define SYMSIZE (12)                     // ♥       (NOTE: plus color)
#  define NUMSIZE (2)                      // 10
#  define CARDSIZE (1+NUMSIZE+1+SYMSIZE+1) // [10 ♥]
#  define CURSORSIZE (SYMSIZE+2+SIMSIZE)   // > ... < (kono baka senpai...)

#  define STACKINGSIZE (NUMSIZE+1+CARDSIZE)
#  define FINISHINGSIZE (FINISHING_COL*(CARDSIZE+1))
#  define PLAYINGSIZE (PLAYING_COL*(CARDSIZE+NUMSIZE+1))

#  define HEADERSIZE (STACKINGSIZE + 3 + FINISHINGSIZE + 1)
#  define BODYSIZE (1 + PLAYINGSIZE + 1) + (CURSORSIZE)

#  define SUIT(x) ((x)+2)
#  define STYLE(x,y) "\x1b[" x "m" y "\x1b[0m"

#  define ERRMSGLEN 29
//#  undef ERRMSGLEN

#  ifndef ERRMSGLEN
#    error ERRMSGLEN is not defined yet. Did you run ./configure?
#  endif

typedef char schar[4];
typedef char wchar[4];

typedef struct {
  char* left;
  char* right;
} Cur;

typedef struct {
  char* buf;
  uint bufs;
  uint wsps;
} Display;

extern Display display;

void displayinit(void);
void displayclear(void);
void displayupdate(Cmd* cmd);

void displayerr(enum err ecode);

#endif
