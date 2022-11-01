#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "shuffle.h"
#include "cards.h"
#include "draw.h"
#include "sole.h"

#if ANIMATION == 1
#  include "opt/anim.h"
#endif

static Cmd cmd;

static void gameinit(Cmd* cmd) {
  // NOTE: this is the same as in `displayinit'
  Player vp;

  vp.event.cmd = 0;
  vp.curr = (PlayerPos) {
    .field = PLAYING,
    .pos = {
      .x = 0,
      .y = 0,
    },
  };

  cmd->stat = COK;
  cmd->p = vp;

  uint t = time(NULL);
  seed = *(uint*) &vp;
  seed = (seeded(t) << 4) ^ t;

  cardsshuffle();
  fieldspop();
}

static inline void init(Cmd* cmd) {
  gameinit(cmd);
  displayinit();
}

static inline void clear(void) {
  displayclear();
}

static Pos pos_justify(Pos cpp, Field cpf) {
  Pos jpp = cpp;

  // there's no reason to justify any field other
  // than the playing one
  if (cpf == PLAYING) {
    if (pfield[cpp.x].am) {
      uint max_y = (pfield[cpp.x].am - pfield[cpp.x].off);
      if (max_y < cpp.y) {
        jpp.y = max_y;
      }
    }
    else {
      jpp.y = 0;
    }
  }

  return jpp;
}

static void mov(Player* p, CmdMov mov) {
  Pos pp = p->curr.pos;
  Field pf = p->curr.field;

  Pos diff = {.x = 0, .y = 0};
  Pos max = {.x = 0, .y = 0};

  switch (pf) {
  case STACKING:
    // where do you think you're going?
    return;
  case PLAYING: {
    DeckField dpf = pfield[pp.x];
    max.y = ((dpf.am - !!dpf.am) - dpf.off);
    max.x = IND(FIELDS);
  }
    break;
  case FINISHING:
    max.x = IND(SUITS_PER_DECK);
    break;
  }

  switch (mov) {
  case CMOVDOWN:
    if (pf == FINISHING) {
      // force overwrap
      diff.x = -1;
    }
    else {
      diff.y = -1;
    }
    break;
  case CMOVUP:
    if (pf == FINISHING) {
      // force underwrap
      diff.x = 1;
    }
    else {
      diff.y = 1;
    }
    break;
  case CMOVLEFT:
    diff.x = -1;
    break;
  case CMOVRIGHT:
    diff.x = 1;
    break;
  }

  pp.x += diff.x;
  pp.y += diff.y;

  // wrap x
  if (pp.x > max.x) {
    pp.x = 0;
  }
  else if (pp.x < 0) {
    pp.x = max.x;
  }

  // wrap y
  if (pp.y > max.y) {
    pp.y = 0;
  }
  else if (pp.y < 0) {
    pp.y = max.y;
  }

  p->event.cmd |= MOV;
  p->curr.pos = pos_justify(pp, pf);
}

static int gameupdate(Cmd* cmd) {
  int ret = 0;
  int ecode = 0;

  char c;
  Player* p = &cmd->p;

  c = getchar();

  if (c == EOF) {
    cmd->stat = CEXIT;
    return GEND;
  }

  if (cmd->stat == CEXIT || c == 'q') {
    return GEND;
  }

  /*
    NOTE: anything that isn't wrapped with `MAYBE(...)`
          cannot:

          1. error
          2. send an error/info message
   */
  switch (c) {

// -- BEGIN: mov-like commands -- //
  case KEY_MOVEDOWN:
    mov(p, CMOVDOWN);
    break;
  case KEY_MOVEUP:
    mov(p, CMOVUP);
    break;
  case KEY_MOVELEFT:
    mov(p, CMOVLEFT);
    break;
  case KEY_MOVERIGHT:
    mov(p, CMOVRIGHT);
    break;
// -- END: mov-like commands -- //

// -- BEGIN: mark-like commands -- //
  case KEY_MARK:
    mark(p);
    break;
  case KEY_MARKSTACK:
    MAYBE(markstack(p));
    break;
  case KEY_DROP:
    drop(p);
    break;
// -- END: mark-like commands -- //

// -- BEGIN: play-like commands  -- //
  case KEY_PLAY:
    MAYBE(play(p));
    break;
  case KEY_PLAYBOTTOM:
    MAYBE(playbottom(p));
    break;
  case KEY_PLAYALL:
    MAYBE(playall(p));
    break;
  case KEY_PLAYSTACK:
    MAYBE(playstack(p));
    break;
// -- END: play-like commands  -- //

// -- BEGIN: field-like commands -- //
  case KEY_CHFIELD:
    chfield(p);
    break;
  case KEY_PREVFIELD:
    MAYBE(prevfield(p));
    break;
  case KEY_PREVMARK:
    MAYBE(prevmark(p));
    break;
// -- END: field -- //

// -- BEGIN: misc commands -- //
  case KEY_TAKE:
    take(p);
    break;
// -- END: mic commands -- //
  }

  return ret;
}

static int gameloop(void) {
  int ret = 0;

  init(&cmd);

  for (;;) {
    int gstat = gameupdate(&cmd);
    if (gstat == GEND) {
#if ANIMATION == 1
      endanimation();
#endif
      break;
    }
    displayupdate(&cmd);
  }

  clear();
  return ret;
}

static int argp(int argc, char** argv) {
  char* farg = NULL;

  if (argc > 1) {
    farg = argv[1];
    if (farg[0] == '-') {
      switch (farg[1]) {
      case 'v':
        puts("sole " VERSION);
        return 1;
      case 'h':
      default:
        fputs(
          "Usage: " PROG "\n"
          "Note:  type `q' to quit at any moment\n", stderr);
        return 1;
      }
    }
  }

  return 0;
}

int main(int argc, char** argv) {
  int ret = argp(argc, argv);

  if (!ret) {
    ret = gameloop();
  }

  return ret;
}
