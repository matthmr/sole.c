# sole.c - solitaire clone

Patience-like card game for the CLI written in C.

## Building

Run `make sole` to build the program. `make` variables are:

- `CC` : the C compiler (cc)
- `CFLAGS` : flags to pass to the compiler ()
- `CFLAGSADD` : additional flags to pass to the compiler ()

No further dependencies are needed besides `libc`.

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

Press `TAB` to cycle between sections. Each section defines a *mode of
operation* that changes the keybindings so that we can reuse them.

Here are the keybindings *per mode*:

- **stacking**:
  - `ENTER`: mark card as active<*>
  - `SPACE`: take another cards
- **playing**:
  - `ENTER`: mark card/(sub)stack as active<*>
  - `h`: go left
  - `j`: go down
  - `k`: go up
  - `l`: go right
  - `s`: send bottom card to the finishing section
  - `SPACE`: send as many cards as possible to the finishing section
- **finishing**:
  - `ENTER`: mark card as active<*>

<*> an active stack/card is marked as movable, any other mark
    operation tries to join the stack/card with the stack under the
    cursor. The `c` key cancels the mark.

Invalid plays will **not** cancel the marking.

## License

This repository is licensed under the [MIT License](https://opensource.org/licenses/MIT).
