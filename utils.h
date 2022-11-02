#ifndef LOCK_UTILS
#  define LOCK_UTILS

#  define PLAYING_COL 7
#  define FINISHING_COL 4

#  define BIT(x) (1 << (x))
#  define ONES(x) (~(~(1<<(x))+1))
#  define SUMUPTO(x) (((x)*(x+1))/2)
#  define MSG(x) (x), (sizeof (x))

#  define ITOA(x)   ((x) + 0x30)
#  define ATOI(x)   ((x) - 0x30)
#  define IND(x)    ((x) - 1)
#  define HUM(x)    ((x) + 1)
#  define ARRLEN(x) ((sizeof((x)))/(sizeof(*(x))))

#  ifndef NULL
#    define NULL ((void*)0)
#  endif

#  define false '\0'
#  define true '\1'

typedef unsigned int uint;
typedef unsigned char bool;

#endif
