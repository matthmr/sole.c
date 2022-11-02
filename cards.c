#include "shuffle.h"
#include "cards.h"

/*
  deck layout:

  0x00        >
              >
              >>(playbuf ~ finbuf)
              >
              >
  0xff        >>(stackf)
 */
Card deck[DECKN] = {
  {.suit = SPADES, .number = _A},  {.suit = HEARTS, .number = _A},
  {.suit = SPADES, .number = _2},  {.suit = HEARTS, .number = _2},
  {.suit = SPADES, .number = _3},  {.suit = HEARTS, .number = _3},
  {.suit = SPADES, .number = _4},  {.suit = HEARTS, .number = _4},
  {.suit = SPADES, .number = _5},  {.suit = HEARTS, .number = _5},
  {.suit = SPADES, .number = _6},  {.suit = HEARTS, .number = _6},
  {.suit = SPADES, .number = _7},  {.suit = HEARTS, .number = _7},
  {.suit = SPADES, .number = _8},  {.suit = HEARTS, .number = _8},
  {.suit = SPADES, .number = _9},  {.suit = HEARTS, .number = _9},
  {.suit = SPADES, .number = _10}, {.suit = HEARTS, .number = _10},
  {.suit = SPADES, .number = _K},  {.suit = HEARTS, .number = _K},
  {.suit = SPADES, .number = _Q},  {.suit = HEARTS, .number = _Q},
  {.suit = SPADES, .number = _J},  {.suit = HEARTS, .number = _J},

  {.suit = CLUBS, .number = _A},   {.suit = DIAMONDS, .number = _A},
  {.suit = CLUBS, .number = _2},   {.suit = DIAMONDS, .number = _2},
  {.suit = CLUBS, .number = _3},   {.suit = DIAMONDS, .number = _3},
  {.suit = CLUBS, .number = _4},   {.suit = DIAMONDS, .number = _4},
  {.suit = CLUBS, .number = _5},   {.suit = DIAMONDS, .number = _5},
  {.suit = CLUBS, .number = _6},   {.suit = DIAMONDS, .number = _6},
  {.suit = CLUBS, .number = _7},   {.suit = DIAMONDS, .number = _7},
  {.suit = CLUBS, .number = _8},   {.suit = DIAMONDS, .number = _8},
  {.suit = CLUBS, .number = _9},   {.suit = DIAMONDS, .number = _9},
  {.suit = CLUBS, .number = _10},  {.suit = DIAMONDS, .number = _10},
  {.suit = CLUBS, .number = _K},   {.suit = DIAMONDS, .number = _K},
  {.suit = CLUBS, .number = _Q},   {.suit = DIAMONDS, .number = _Q},
  {.suit = CLUBS, .number = _J},   {.suit = DIAMONDS, .number = _J},
};

DeckField pfield[FIELDS] = {0};
DeckField sfield = {0};
DeckField ffield[SUITS_PER_DECK] = {0};

struct fnum_t {
  uint mod;
  uint zero;
};

static struct fnum_t fnum = {
  .mod = SOLE_FIELDS,
  .zero = 0
};

static void swap(uint i) {
  uint swp = seeded(DECKN);
  uint _swp = (swp + i) % DECKN;

  if (swp == _swp) {
    return;
  }

  Card tmp = deck[_swp];
  deck[_swp] = deck[swp];
  deck[swp] = tmp;
}

static void stackpop(void) {
  sfield.stack = pfield[FIELDS-1].stack+1;
  sfield.am = (&deck[DECKN-1] - sfield.stack) + 1;
  sfield.off = IND(sfield.am);
}

static void playingpop(void) {
  uint i = 0;
  uint off = 0;

  for (; i < FIELDS; i++) {
    pfield[i].am = (i+1);
    pfield[i].off = i;
    pfield[i].stack = (Card*)deck + off;
    off += (i+1);
  }
}

static void finishingpop(void) {
  for (uint i = 0; i < SUITS_PER_DECK; i++) {
    ffield[i].off = 0;
    ffield[i].am = 0;

    /*
      NOTE: the update function *only* checks *this* field
            to be NULL. If it is, it tries to call in on
            memory utilities to fit it inside `pfield'.
     */
    ffield[i].stack = NULL;
  }
}

static void movupdate(Player* p) {
}

static Card* cardonpos(Pos pp, Field pf) {
  Card* ret = NULL;

  switch (pf) {
  case STACKING:
    ret = sfield.stack;
    break;
  case PLAYING:
    ret = (pfield[pp.x].stack + pfield[pp.x].off) + pp.y;
    break;
  case FINISHING:
    ret = (ffield[pp.x].stack + ffield[pp.x].off);
    break;
  }

  return ret;
}

void cardsshuffle(void) {
  for (uint i = 0; i < DECKN; i++) {
    swap(i);
  }
}

void fieldspop(void) {
  playingpop();
  finishingpop();
  stackpop();
}

/* compare current `actlatch' with `paslatch':
   `actlatch'::
          top-most card of the stack that's being moved
   `paslatch'::
          card that the active stack will latch into;
          if NULL, then we're latching into an empty space
 */
bool islegal(Card* actlatch, Card* paslatch, Field actfield) {
  Card act = *actlatch;
  bool fin = (actfield == FINISHING);

  if (!paslatch) {
    return act.number == (fin? _A: _K);
  }
  else {
    Card pas = *paslatch;
    Suit passuit = pas.suit, actsuit = act.suit;
    Number pasnum = pas.number, actnum = act.number;

    return fin?
      ((actsuit == passuit) && (pasnum != _K) && (pasnum - actnum == 1)):
      (((REDSUIT(passuit) && BLACKSUIT(actsuit))
        || (BLACKSUIT(passuit) && REDSUIT(actsuit)))
       && ((pasnum != _A) && (pasnum - actnum == 1)));
  }
}

void take(Player *p) {
  sfield.off--;
  if (sfield.off == -1u) {
    sfield.off = IND(sfield.am);
  }
}

void mark(Player* p) {
  p->event.cmd |= MARK;
  p->event.from = p->curr;
}

void chfield(Player *p) {
  // NOTE: this doesn't assume mark; although it may be the same
  p->prev = p->curr;
  p->curr.field = (p->curr.field % fnum.mod) + fnum.zero;
  //p->pos = updatepos(p->field, p->pos);
}

int play(Player* p) {
  int ret = 0;

  Event pe = p->event;

  PlayerPos actpos = pe.to;
  PlayerPos paspos;

  if ((pe.cmd & KEEP)) {
    paspos = pe.from;
  }
  else {
    paspos = p->curr;
  }

  // can't play a card into the stack
  if (actpos.field == STACKING) {
    ret = -EILLEGAL;
    goto done;
  }

  Card* act = cardonpos(actpos.pos, actpos.field);
  Card* pas = cardonpos(paspos.pos, paspos.field);

  if (islegal(act, pas, actpos.field)) {
    if (!(pe.cmd & KEEP)) {
      pe.cmd &= ~MARK;
    }
    pe.cmd |= PLAY;
    p->event = pe;

    movupdate(p);
  }
  else {
    ret = -EILLEGAL;
  }

done:
  return ret;
}

int playbottom(Player* p) {
  int ret = 0;
  Player vp = *p;

  uint max_y = pfield[vp.curr.pos.x].am;

  if (IND(max_y)) {
    ret = -EBOTNCARD;
    goto done;
  }
  else {
    vp.curr.pos.y = IND(max_y);
  }

  ret = play(&vp);
  *p = vp;

done:
  return ret;
}

// TODO: the finishing algorithm
int playall(Player* p) {
  int ret = 0;
  uint hits = 0;

  if (!hits) {
    ret = -EALLNFIN;
  }

  return ret;
}

void drop(Player* p) {
  p->event.cmd &= ~MARK;
}

int markstack(Player* p) {
  int ret = 0;
  Player vp = *p;

  if (!sfield.am) {
    ret = -ESTKNCARD;
  }
  else {
    vp.event.cmd |= MARK;
    vp.event.from = (PlayerPos) {
      .pos = {
        .x = 0,
        .y = 0,
      },
      .field = STACKING,
    };

    mark(&vp);
    *p = vp;
  }

  return ret;
}

int prevfield(Player *p) {
  int ret = 0;

  if (!p->prev.field) {
    ret = -ENFIELD;
  }
  else {
    PlayerPos tmp = p->curr;
    p->curr = p->prev;
    p->prev = tmp;
  }

  return ret;
}

int prevmark(Player *p)  {
  int ret = 0;

  if (!(p->event.cmd & MARK)) {
    ret = -ENMARK;
  }
  else {
    p->curr = p->event.from;
  }

  return ret;
}

int playstackfin(Player *p)  {
  int ret = 0;
  Player vp = *p;

  if (!sfield.am) {
    ret = -ESTKNCARD;
  }
  else {
    vp.event.cmd |= (MARK | KEEP);
    vp.event.from = (PlayerPos) {
      .pos = {
        .x = 0,
        .y = 0,
      },
      .field = STACKING,
    };
    vp.event.to = (PlayerPos) {
      .pos = {
        .x = (sfield.stack + sfield.off)->suit,
        .y = 0,
      },
      .field = FINISHING,
    };
    
    ret = play(&vp);
    *p = vp;
  }
  
  return ret;
}

int playstackcur(Player *p) {
  int ret = 0;
  Player vp = *p;

  if (!sfield.am) {
    ret = -ESTKNCARD;
  }
  else {
    vp.event.cmd |= (MARK | KEEP);
    vp.event.from = (PlayerPos) {
      .pos = {
        .x = 0,
        .y = 0,
      },
      .field = STACKING,
    };
    vp.event.to = vp.curr;

    ret = play(&vp);
    *p = vp;
  }
  
  return ret;
}
