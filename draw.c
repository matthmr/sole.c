#include <stdlib.h>
#include <unistd.h>
#include <termios.h>

#include "drawcore.h"
#include "cards.h"
#include "draw.h"
#include "cli.h"

typedef struct termios Termios;

/*
   draw stack:

             (draw_header_finishing)       <<<
    (draw_header_stacking) |||||||||       <<<
    |||||||||   ||||||||||||||||||||       <<<
    vvvvvvvvv   vvvvvvvvvvvvvvvvvvvv       <<<
    24 [10 ♥]   [4  ♥] [Q  ♦] [2  ♣]  **   <<< (draw_header)
                                           <<<
    --2---  **  [  ] [  ] [  ] [  ] [  ]   <<< (draw_playing_header)
   >[    ]<                                <<<
   ~[   ]~                                 <<< (draw_playing)
    <error>                                <<< (draw_error)
 */

Display display = {0};
Termios oset, nset;

typedef struct {
  bool play[FIELDS];
  bool fini[SUITS_PER_DECK];
} HeaderMask;

static HeaderMask mask = {0};

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
	[SUIT(DIAMONDS)] = "♦",
	[SUIT(CLUBS)]    = "♣",
	[SUIT(HEARTS)]   = "♥",
	[SUIT(SPADES)]   = "♠",
};

static const char* errmsg[] = {
  [IND(EILLEGAL)]  = "Illegal play",
  [IND(EBOTNFIN)]  = "Bottom card is not finishable",
  [IND(EBOTNCARD)] = "No cards in this column",
  [IND(EALLNFIN)]  = "No legal finishable play",
  [IND(ESTKNCARD)] = "No cards in the stack",
};

static bool preverr = false;

enum cur_t {
  ACT_CARD,
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

static void termset(void) {
  tcgetattr(0, &oset);
  nset = oset;
  nset.c_lflag &= (~ICANON & ~ECHO);
  tcsetattr(0, TCSANOW, &nset);
  write(1, "\033[?25l", 6);
}

static void termrest(void) {
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
    buf = draw(buf, (char*) suitpm[SUIT(card->suit)]);
    buf = draw(buf, __RESET__ "]");
  }
  else {
    buf = draw(buf, " ** ");
  }

  return buf;
}

static char* draw_card_cur(char* buf, Card* card, enum cur_t cur) {
  buf = draw(buf, (char*) selcur[cur].left);
  buf = draw_card(buf, card);
  buf = draw(buf, (char*) selcur[cur].right);

  return buf;
}

struct align_t {
  uint stack;
  uint fini;
};

static struct align_t aligncomp(Player* p, HeaderMask mask) {
  struct align_t ret = {.stack = 0, .fini = 0};
  uint ph = 0;

  for (uint i = 0; i < FIELDS; i++) {
    if (pfield[i].am) {
      ph++;
    }
  }

  ret.fini = ph;

  return ret;
}

// -- draw stack top -- //
static char* draw_header_stacking(char* buf, Player* p, uint align) {
  uint am = sfield.am;
  uint off = sfield.off;
  Card* top = sfield.stack;

  if (am) {
    if (p->curr.field == STACKING) {
      buf = draw_num(buf, off);
      buf = draw_card_cur(buf, top + off, ACT_CARD);
    }
    else if (off) {
      buf = draw_num(buf, off);
      buf = drawn(buf, ' ', 2);
    }
    else {
      buf = draw(buf, " ** ");
      buf = drawn(buf, ' ', 4);
    }
  }
  else {
    buf = draw(buf, " ** ");
    buf = drawn(buf, ' ', 5);
  }

  if (off) {
    buf = draw_card(buf, top + off);
  }
  else {
    buf = draw(buf, " ** ");
  }

  buf = drawn(buf, ' ', align);

  return buf;
}
static char* draw_header_finishing(char* buf, Player* p, uint align) {
  uint mark = -1u;
  uint i = 0;

  if (p->curr.field == FINISHING) {
    // we preemptively write a space from our sibbling
    mark = p->curr.pos.x;
    buf--;
  }

  i++;

  for (; i < SUITS_PER_DECK; i++) {
    if (i == mark) {
      buf = draw_card_cur(buf, (ffield[i].stack + ffield[i].off), ACT_CARD);
      mark++;
    }
    else {
      // don't draw over the right border of a mark
      if (i != mark) {
        buf = drawn(buf, ' ', 1);
      }
      buf = draw_card(buf, (ffield[i].stack + ffield[i].off));
    }
  }

  return buf;
}

static char* draw_header(char* buf, Player* p) {
  struct align_t align = aligncomp(p, mask);

  buf = draw_header_stacking(buf, p, align.stack);
  buf = draw_header_finishing(buf, p, align.fini);
  buf = drawn(buf, '\n', 2);

  return buf;
}

static char* draw_playing_header(char* buf, Player* p) {
  buf = drawn(buf, ' ', 1);

  for (uint i = 0; i < FIELDS; i++) {
    uint off = pfield[i].off;

    if (i) {
      buf = drawn(buf, ' ', 1);
    }

    // there are no hidden cards: draw the top card from
    // the playing field
    if (!off) {
      buf = draw_card(buf, (pfield[i].stack + pfield[i].off));
    }
    else {
      /*
        NOTE: this field can only decrease and the default
              is *single digited*
       */
      buf = draw(buf, " --");
      buf = draw_num(buf, off);
      buf = draw(buf, "---");
    }
  }

  buf = drawn(buf, ' ', 1);
  buf = drawn(buf, '\n', 1);

  return buf;
}

static char* draw_playing(char* buf, Player* p) {
  buf = draw_playing_header(buf, p);

  for (uint i = 0; i < FIELDS; i++) {
    // we already drew you on `draw_playing_header'-time
    if (pfield[i].am == 1) {
      continue;
    }
    else {
      
    }
  }

  return buf;
}
// -- draw stack bottom -- //

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

static char* set_new_frame(char* buf) {
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

  //struct align_t align = draw_update_event(p->event);

  buf = set_new_frame(buf);

  // -- draw stack top -- //
  buf = draw_header(buf, p);
  buf = draw_playing(buf, p);
  // -- draw stack bottom -- //

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
  // static alloc, later alloc's will be more dynamic than this
  display.bufs += NUMSIZE + 1 + CARDSIZE;      // 24 [10 ♥]
  // TODO: this is not 5, it should be dynamic
  //display.bufs += 5;
  display.bufs += SUITS_PER_DECK*(CARDSIZE+1); // [4 ♥] [Q ♦] [2 ♣]  **
  display.bufs += 1;
  display.bufs += 1+7*(CARDSIZE+1)+1;          // -2--  **  [  ] [  ]
  display.bufs += 2*(ESCSIZE+1)+1;             // > ... < (anata... baka!)
  display.bufs += ERRMSGLEN+1;                 // <error>

  display.buf = malloc(display.bufs*sizeof(*display.buf));
  draw_stack(p);
}

void displayinit(void) {
  termset();

  // NOTE: this is the same as in `gameinit'
  Player vp;

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
  draw_stack(&cmd->p);
}

void displayerr(enum err ecode) {
  int code = -(int) ecode;

  const char* msg = errmsg[HUM(code)];
}
