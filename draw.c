#include <stdlib.h>
#include <unistd.h>
#include <termios.h>

#include "drawcore.h"
#include "cards.h"
#include "draw.h"
#include "cli.h"

#define HEADER_INIT_MAGIC (18)
#define HEADER_ALIGN_MAGIC (21)
#define HEADER_PAD_MAGIC (3)
#define HEADER_CURSOR_PAD_MAGIC (2)
#define HEADER_CARD_MAGIC (7)

typedef struct termios Termios;

/*
   draw stack:

     - 20 -                    - ♠ --   - ♥ --   - ♦ --   - ♣ -- <<<
     [10 ♥]                    [4  ♥]   [Q  ♦]   [2  ♣]     **   <<<
                                                                 <<< (draw_header)
     - 2 --   > ** <  [    ]   [    ]   [    ]   [    ]   [    ] <<<
    ~[    ]~                                                     <<<
    ~[    ]~                                                     <<<
                                                                 <<< (draw_playing)
    <error>                                                      <<< (draw_error)
 */

Display display = {0};
Termios oset, nset;

static const schar numpm[] = {
  [_A]  = "A  ",
  [_2]  = "2  ",
  [_3]  = "3  ",
  [_4]  = "4  ",
  [_5]  = "5  ",
  [_6]  = "6  ",
  [_7]  = "7  ",
  [_8]  = "8  ",
  [_9]  = "9  ",
  [_10] = "10 ",
  [_K]  = "K  ",
  [_Q]  = "Q  ",
  [_J]  = "J  ",
};

static const wchar suitpm[] = {
  [DIAMONDS] = "♦",
  [CLUBS]    = "♣",
  [HEARTS]   = "♥",
  [SPADES]   = "♠",
};

// get the compiler to spill the deeds using `sizeof'
typedef struct {
  const char* string;
  const uint size;
} String;

static const String errmsg[] = {
  [IND(EILLEGAL)]  = {MSG("Illegal play")},
  [IND(EBOTNFIN)]  = {MSG("Bottom card is not finishable")},
  [IND(EBOTNCARD)] = {MSG("No cards in this column")},
  [IND(EALLNFIN)]  = {MSG("No legal finishable play")},
  [IND(ESTKNCARD)] = {MSG("No cards in the stack")},
  [IND(ENMARK)]    = {MSG("Mark is not set")},
  [IND(ENFIELD)]   = {MSG("No previous field")},
};

static bool err = false;

enum cur_t {
  NO_CUR = -1,

  ACT_CARD = 0,
  ACT_STACK,

  PAS_CARD,
  PAS_STACK,
};

static const Cur selcur[] = {
  [ACT_CARD] = {
    .left  = STYLE("92;1", ">"),
    .right = STYLE("92;1", "<"),
  },
  [ACT_STACK] = {
    .left  = STYLE("92", "~"),
    .right = STYLE("92", "~"),
  },

  [PAS_CARD] = {
    .left  = STYLE("91", "~"),
    .right = STYLE("91", "~"),
  },
  [PAS_STACK] = {
    .left  = STYLE("91", "~"),
    .right = STYLE("91", "~"),
  },
};

static const Cur markcur = {
  .left  = STYLE("91;1", ">"),
  .right = STYLE("91;1", "<"),
};

static inline void termset(void) {
  tcgetattr(0, &oset);
  nset = oset;
  nset.c_lflag &= (~ICANON & ~ECHO);
  tcsetattr(0, TCSANOW, &nset);
  write(1, "\033[?25l", 6);
}

static inline void termrest(void) {
  tcsetattr(0, TCSANOW, &oset);
  write(1, "\033[?25h", 6);
}

static inline char* card_color(Suit suit) {
  return REDSUIT(suit)?
    "\x1b[91m":
    "\x1b[90m";
}

static char* draw_card(char* buf, Card* card) {
  if (card) {
    buf = drawn(buf, '[', 1);
    buf = draw(buf, card_color(card->suit));
    buf = draw(buf, (char*) numpm[card->number]);
    buf = draw(buf, (char*) suitpm[card->suit]);
    buf = draw(buf, __RESET__ "]");
  }
  else {
    buf = draw(buf, "  **  ");
  }

  return buf;
}

static inline char* draw_card_cur(char* buf, Card* card, enum cur_t cur) {
  buf = draw(buf, (char*) selcur[cur].left);
  buf = draw_card(buf, card);
  buf = draw(buf, (char*) selcur[cur].right);

  return buf;
}

static inline char* draw_header_suit(char* buf, Suit suit) {
  // NOTE: this is the *only one* to have an extra space because
  //       this is only called once
  buf = draw(buf, "- ");
  buf = draw(buf, card_color(suit));
  buf = draw(buf, (char*) suitpm[suit]);
  buf = draw(buf, __RESET__ " --");

  return buf;
}

static inline char* draw_header_num(char* buf, uint num) {
  buf = draw(buf, "- ");
  char* _buf = buf;
  buf = draw_num(buf, num);
  buf = draw(buf, ((buf - _buf == 1)? " --": " -"));

  return buf;
}

static inline char* draw_header_top_stacking(char* buf) {
  buf = drawn(buf, ' ', HEADER_INIT_MAGIC);

  Suit order[] = {SPADES, HEARTS, CLUBS, DIAMONDS};

  for (uint i = 0; i < ARRLEN(order); i++) {
    buf = drawn(buf, ' ', HEADER_PAD_MAGIC);
    buf = draw_header_suit(buf, order[i]);
  }

  buf = drawn(buf, '\n', 1);

  return buf;
}

static char* draw_cursor_cur(char* buf, enum cur_t cur) {
  if (cur == NO_CUR) {
  }
  else {
  }

  return buf;
}

static char* draw_cursor_forward_by_card(char* buf) {
  // TODO: stub
  return buf;
}

static inline char* draw_cursor_change(char* buf, enum cur_t cur, uint am) {
  for (uint i = 0; i < am; i++) {
    buf = draw_cursor_cur(buf, cur);
    buf = draw_cursor_forward_by_card(buf);
    buf = draw_cursor_cur(buf, cur);
  }

  return buf;
}

// -- BEGIN: draw stack -- //
static char* draw_header_top(char* buf) {
  buf = drawn(buf, ' ', HEADER_PAD_MAGIC);

  buf = draw_header_num(buf, sfield.off);
  buf = draw_header_top_stacking(buf);

  return buf;
}

static char* draw_header_stacking(char* buf, Player* p) {

  /*
    NOTE: here, `off' means the cards *over* the stack
          and `am` means all the stack cards at once
   */
  uint off = sfield.off;
  uint am = sfield.am;

  Card* top = sfield.stack;
  uint align = HEADER_ALIGN_MAGIC;

  enum cur_t cur;

#define SET_CURSOR_STACKING(a,b,c)              \
  (a.b) == STACKING && (cur = (c), true)

  bool is_cursor_here = (
    (SET_CURSOR_STACKING(p->curr, field, ACT_CARD))
    || (SET_CURSOR_STACKING(p->event, cmd & MARK, PAS_CARD)));


  if (am) {
    if (is_cursor_here) {
      buf = drawn(buf, ' ', HEADER_CURSOR_PAD_MAGIC);
      buf = draw_card_cur(buf, (top + off), cur);
      align--;
      goto done;
    }

    buf = drawn(buf, ' ', HEADER_PAD_MAGIC);

    if (off) {
      buf = draw_card(buf, (top + off));
    }
    else {
      buf = draw(buf, "  **  ");
    }
  }
  else {
    buf = draw(buf, "  **  ");
  }

done:
  // trailing space before the finishing field
  buf = drawn(buf, ' ', align);

  return buf;
}
static char* draw_header_finishing(char* buf, Player* p) {
  enum cur_t cur;
  uint here;

#define SET_CURSOR_FINISHING(a,b,c,d)                   \
  (a.b) == FINISHING && (cur = (d), here = (a.c), true)

  // dirty C hack, may not work on compilers that don't short circuit booleans
  // also, the macro looks dirty but it *should* work on your preprocessor
  bool is_cursor_here = (
    (SET_CURSOR_FINISHING(p->curr, field, pos.x, ACT_CARD))
    || (SET_CURSOR_FINISHING(p->event, cmd & MARK, from.pos.x, PAS_CARD)));

  for (uint i = 0; i < SUITS_PER_DECK; i++) {
    DeckField ff = ffield[i];

    if (is_cursor_here) {
      // wait for the cursor state
      if (here == i) {
        buf = draw_card_cur(buf, (ff.stack + ff.off), cur);
        goto _draw_pad;
      }
      else {
        goto _draw_card;
      }
    }
    else {
_draw_card:
      if (ff.am) {
        buf = draw_card(buf, (ff.stack + ff.off));
      }
      else {
        buf = draw(buf, "  **  ");
      }
    }
_draw_pad:
    buf = drawn(buf, ' ', HEADER_PAD_MAGIC);
  }

  return buf;
}

static char* draw_header(char* buf, Player* p) {
  buf = draw_header_top(buf);
  buf = draw_header_stacking(buf, p);
  buf = draw_header_finishing(buf, p);
  buf = drawn(buf, '\n', 2);

  return buf;
}

static char* draw_playing_header(char* buf, Player* p) {
  enum cur_t cur;
  uint here;

#define SET_CURSOR_PLAYING_HEADER(a,b,c,d)                     \
  (a.b) == PLAYING && (cur = (d), here = (a.c), true)

  bool is_cursor_here = (
    (SET_CURSOR_PLAYING_HEADER(p->curr, field, pos.x, ACT_CARD))
    || (SET_CURSOR_PLAYING_HEADER(p->event, cmd & MARK, from.pos.x, PAS_CARD)));

  for (uint i = 0; i < FIELDS; i++) {
    uint off = pfield[i].off;

    if (off) {
      if (is_cursor_here && (here == i || (here + 1) == i)) {
        buf = drawn(buf, ' ', HEADER_CURSOR_PAD_MAGIC);
      }
      else {
        buf = drawn(buf, ' ', HEADER_PAD_MAGIC);
      }

      buf = draw_header_num(buf, off);

      continue;
    }

    else if (is_cursor_here) {
      if (here == i) {
        buf = drawn(buf, ' ', HEADER_CURSOR_PAD_MAGIC);
        buf = draw_card_cur(buf, (pfield[i].stack + pfield[i].off), cur);
        continue;
      }
      else if ((here + 1) == i) {
        buf = drawn(buf, ' ',  HEADER_CURSOR_PAD_MAGIC);
      }
      else {
        buf = drawn(buf, ' ', HEADER_PAD_MAGIC);
      }
    }
    else {
      buf = drawn(buf, ' ', HEADER_PAD_MAGIC);
    }

    buf = draw_card(buf, (pfield[i].stack + pfield[i].off));
  }

  buf = drawn(buf, '\n', 1);

  return buf;
}

static char* draw_playing(char* buf, Player* p) {
  buf = draw_playing_header(buf, p);

  enum cur_t cur;
  Pos here;

#define SET_CURSOR_PLAYING(a,b,c,d)             \
  (a.b) == PLAYING && (cur = (d), here.x = (a.c.x), here.y = (a.c.y), true)

  bool is_cursor_here = (
    (SET_CURSOR_PLAYING(p->curr, field, pos, ACT_CARD))
    || (SET_CURSOR_PLAYING(p->event, cmd & MARK, from.pos, PAS_CARD)));

  for (uint i = 0; i < FIELDS; i++) {
    DeckField pf = pfield[i];

    // we already drew you
    if (pf.am == 1) {
      buf = drawn(buf, ' ', HEADER_PAD_MAGIC + HEADER_CARD_MAGIC);
      continue;
    }

    else {
      uint max_y = (pf.am - pf.off) + 1;
      for (uint j = 1; j < max_y; j++) {

        if (here.x == i) {
          buf = drawn(buf, ' ', HEADER_CURSOR_PAD_MAGIC);
          if (here.y == j) {
            buf = draw_card_cur(buf, (pf.stack + pf.off + j), cur);
          }
          else {
            // NOTE: `+1' essentially gets the `_STACK' version of the cursor
            buf = draw_card_cur(buf, (pf.stack + pf.off + j), (cur+1));
          }
          continue;
        }
        else if ((here.x+1) == i) {
          buf = drawn(buf, ' ', HEADER_CURSOR_PAD_MAGIC);
        }
        else {
          buf = drawn(buf, ' ', HEADER_PAD_MAGIC);
        }

        buf = draw_card(buf, (pf.stack + pf.off + j));
      }
    }
  }

  return buf;
}
// -- END: draw stack -- //

static void display_compsize(Display* dpy, DeckField* pf) {
  uint ret = 0;

  struct d_t {
    uint x;
    uint y;
  };

  // backtrace filling algorithm
  for (uint x = 1; x < FIELDS; x++) {
    uint prevvis = (pf[x-1].am - pf[x-1].off);
    uint currvis = (pf[x].am   - pf[x].off);

    if (x == 1) {
      if (currvis > prevvis) {
        ret += (currvis - prevvis);
      }
      continue;
    }
    if (currvis <= prevvis) {
      continue;
    }

    struct d_t d = {.x = 0, .y = 0};
    uint bndvis = prevvis;

    /*
        the `COUNT` form:
               .____________________________.
               | d.y*(d.x+(d.x-1)*CARDSIZE) |
               `----------------------------´

        used below follows that:

        1. `d.y` counts how many columns we got
        2. `d.x` counts pure whitespaces
        3. `(d.x-1)*CARDSIZE` counts the cards in between
    */
#define COUNT(v) (v) += (d.y*(d.x+(d.x-1)*CARDSIZE))

    for (uint tx = (x-2); tx > 0; tx--) {
      d.x = (x - tx);
      uint tstvis = (pf[tx].am - pf[tx].off);

      if (tstvis < currvis) {
        if (tstvis > bndvis) {
          d.y = (tstvis - bndvis);
          COUNT(ret);
          bndvis = tstvis;
        }
        else {
          continue;
        }
      }
      else {
        break;
      }
    }

    d.y = (currvis - bndvis);
    COUNT(ret);
  }

  // account for the newlines
  ret += FIELDS;
}

static void display_alloc(Display* dpy) {
  uint obufs = dpy->bufs;
  display_compsize(dpy, pfield);

  if (dpy->bufs != obufs) {
    dpy->buf = realloc(dpy->buf, obufs*sizeof(*dpy->buf));
  }
}

static void render(Display dpy, char* buf) {
  char* dpybuf = dpy.buf;
  uint dpybufs = buf - dpybuf;

  *buf = '\0';

  write(1, dpybuf, dpybufs);
}

static char* set_new_frame(char* buf, EventCmd eventcmd) {
  // TODO: clear the previous frame

  display_alloc(&display);
  buf = display.buf;
  return buf;
}

static char* put_cursor(char* buf, Player* p) {
  return buf;
}

static void draw_stack(Player* p) {
  char* buf = display.buf;

  buf = set_new_frame(buf, p->event.cmd);

  // -- BEGIN: draw stack -- //
  buf = draw_header(buf, p);
  buf = draw_playing(buf, p);
  // -- END: draw stack -- //

  buf = drawn(buf, '\n', 2);
  buf = put_cursor(buf, p);

  render(display, buf);

#if DEBUG == 1
#  ifdef DEBUG_DRAW_STACK
  termrest();
  exit(0);
#  endif
#endif

}

static void draw_stack_init(Player* p) {
  char* buf = NULL;
  uint bufs = 0;

  uint errmsglen = 0;

  for (uint i = 0; i < ARRLEN(errmsg); i++) {
    uint msglen = errmsg[i].size;
    errmsglen = (msglen > errmsglen)? msglen: errmsglen;
  }

  // -- BEGIN: init display -- //
  // static alloc, later alloc's will be more dynamic than this
  bufs += 0;
  bufs += NUMSIZE + 1 + CARDSIZE;      // 24 [10 ♥]
  bufs += SUITS_PER_DECK*(CARDSIZE+1); // [4 ♥] [Q ♦] [2 ♣]  **
  bufs += 1;
  bufs += 1+FIELDS*(CARDSIZE+1)+1;     // -2--  **  [  ] [  ]
  bufs += 2*(ESCSIZE+1)+1;             // > ... < (anata... baka!)
  bufs += 1+errmsglen+1;               // <error>
  display.bufs = bufs;

#if DEBUG == 1
  buf = malloc(1024*sizeof(*display.buf));
#else
  buf = malloc(display.bufs*sizeof(*display.buf));
#endif

  display.buf = buf;
  // -- END: init display -- //

  draw_stack(p);
  render(display, buf);

#if DEBUG == 1
#  ifdef DEBUG_DRAW_STACK_INIT
  termrest();
  exit(0);
#  endif
#endif

}

static void draw_error(char* msg, bool err) {
}

static void clear_error(void) {
}


void displayinit(void) {
  termset();

  // NOTE: this is the same as in `gameinit'
  Player vp;

  vp.event.cmd = 0;
  vp.curr = (PlayerPos) {
    .field = PLAYING,
    .pos = {
      .x = 0,
      .y = 0,
    },
  };

  draw_stack_init(&vp);
}

void displayclear(void) {
  termrest();
  free(display.buf);
}

void displayupdate(Cmd* cmd) {
  if (err) {
    clear_error();
    err ^= true;
  }

  draw_stack(&cmd->p);
}

void displayerr(enum err ecode) {
  int code = -(int) ecode;
  const char* msg = errmsg[HUM(code)].string;
  draw_error(msg, err);

  err ^= true;
}
