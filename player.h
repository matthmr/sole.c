#ifndef LOCK_PLAYER
#  define LOCK_PLAYER

#  include "utils.h"

enum err {
  EILLEGAL=1,
  EBOTNFIN,
  EBOTNCARD,
  EALLNFIN,
  ESTKNCARD,
};

/*
  the coordinates follow:

  - the leftmost column(x) is 0
  - the highest row(y) is 0

              x
      <-----------------> 
   ^   0,0
   |   v
y  |   .    .    .
   |        .    .
   |             . < 2,2
   v

  unary fields only use `::x'
 */
typedef struct {
  int x;
  int y;
} Pos;

typedef enum {
  STACKING = 1,
  PLAYING,
  FINISHING,
} Field;

/*
  NOTE: this mark act as the 'passive' mark when
  running a command that checks against two
  existing marks.
*/
typedef struct {
  Pos pos;
  Field field;
} PlayerPos;

typedef enum {
  MOV  = BIT(0),
  TAKE = BIT(1),
  MARK = BIT(2),
  PLAY = BIT(3),
} EventCmd;

typedef struct {
  EventCmd cmd;
  PlayerPos from;
  PlayerPos to;
} Event;

//typedef struct {
//  PlayerPos act;
//  PlayerPos pas;
//} PlayerPosCurr;

typedef struct {
  //PlayerPosCurr curr;
  Event event;
  PlayerPos curr;
  PlayerPos prev;
} Player;

typedef enum {
  COK = 0,
  CEXIT,
} CmdStat;

// TODO: make a callback field that can be read by `displayupdate'
typedef struct {
  Player p;
  CmdStat stat;
} Cmd;

#  define ERR(x) ((x) < 0)

#endif
