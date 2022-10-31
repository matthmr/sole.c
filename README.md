# sole.c - solitaire clone

Patience-like card game for the CLI written in C.

## Building

Run `./configure` then `make sole` to build the program. `make`
variables are:

- `CC` : the C compiler (cc)
- `CFLAGS` : flags to pass to the compiler (-Wall)
- `CFLAGSADD` : additional flags to pass to the compiler ()

`./configure` flags are the same as `make` variables but in
**lowercase** (for example `./configure --cc=clang`) plus the
following include flags:

- `[-/+]animation` : ommit/permit displaying ending animation (permit)

No further dependencies are needed besides `libc`. This project uses
the `./configure` building system from
[cproj](https://github.com/matthmr/cproj).

## Playing

`sole` plays similar to solitaire. There are three sections to the game:

- stacking field
- playing field
- finishing field

The **stacking field** is the section at the *top-right*. It contains
cards that can be revealed one-by-one.

The **playing field** is the section at the *bottom*. It consists of
seven columns, each of which can hold any number of cards. By default,
they hold as many cards as their numerical index (i.e. the first holds
one, the second holds two, etc). 

The **finishing field** is the section at the *top-right*. It consists
of four columns, each of which holds as many cards as there are in
each suit. Each column is mapped to a suit following the order:

> Spades, Hearts, Clubs, Diamonds

The objective of the game is to fill out the finishing field with all
cards of all suits following solitaire ordering:

> A, 2, 3, 4, 5, 6, 7, 8, 9, 10, K, Q, J

You can stack out cards from the playing field as long as they follow
the **reverse** of the numerical order above and are using alternating
suit color. The colors are set as:

- **RED**: spades, diamonds
- **BLACK**: hearts, clubs

So a sequence of `(9 spades, 8 hearts)` is valid and a sequence of `(9
diamonds, 8 hearts)` is not.

If a column of the playing field is vacant, only a stack starting with
`K` will be able to occupy that space. Cards may be taken in and out
of the finishing space at any moment.

### Interface

Here are the default keybindings broken down by category:

#### Movement

- `h`: go left
- `j`: go down
- `k`: go up
- `l`: go right

#### Mark

- `m`: mark the current visible stack card
- `RET`: mark card
- `c`: cancel mark

#### Play

- `a`: send all possible cards to the finishing stack maintaining
       the mark as intact as possible
- `b`: play the bottom card of the current stack into the finishing
       stack (keeps mark, if possible)
- `s`: send the visible stack card into the finishing stack
- `p`: act on mark (send marked stack to current position, if possible)

#### Fields

- `TAB`: cycle fields
- `q`: go to the previous field, preserving the previous cursor
       position (keeps mark, if possible)
- `t`: go to the passive mark

#### Misc

- `SPC`: take a card from the stack

Invalid plays (acts) will **not** cancel the mark.

## License

This repository is licensed under the [MIT License](https://opensource.org/licenses/MIT).
