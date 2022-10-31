#ifndef LOCK_CARDS
#  define LOCK_CARDS

#  include "utils.h"
#  include "player.h"

#  define SOLE_FIELDS (3)
#  define FIELDS (7)

#  define CARDS_PER_SUIT (13)
#  define SUITS_PER_DECK (4)

#  define DECKN (CARDS_PER_SUIT * SUITS_PER_DECK)
#  define CARDS_ON_PLAYING AR_SUM_UP_TO(FIELDS-1)
#  define CARDS_ON_STACKING (DECKN - CARDS_ON_PLAYING)

#  define REDSUIT(x) ((x) > 0)
#  define BLACKSUIT(x) ((x) < 0)

/*
  red suits:   positive
  black suits: negative
 */
typedef enum suit {
	DIAMONDS = 1,
	HEARTS = 2,

	CLUBS = -1,
	SPADES = -2,
} Suit;

typedef enum number {
	_A = 0, _2, _3, _4, _5, _6, _7, _8, _9, _10,
	_K,	_Q,	_J,
} Number;

typedef struct {
  Suit suit;
  Number number;
} Card;

typedef struct {
  Card* stack;
  uint am;
  uint off;
} DeckField;

extern Card deck[DECKN];

extern DeckField pfield[FIELDS];
extern DeckField sfield;
extern DeckField ffield[SUITS_PER_DECK];

void fieldspop(void);
void cardsshuffle(void);

bool islegal(Card* act, Card* pas, Field actfield);

int play(Player* p);
int playbottom(Player* p);
int playstack(Player* p);
int playall(Player* p);
int markstack(Player* p);
int prevfield(Player* p);
int prevmark(Player* p);

void chfield(Player* p);
void mark(Player* p);
void take(Player* p);
void drop(Player* p);

#endif
