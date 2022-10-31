include(make/m4/defaults.m4)dnl
include(make/m4/makefile.m4)dnl
target(`sole')dnl
target_obj(`sole.o',dnl
`cards.o', `draw.o', `drawcore.o', `shuffle.o')dnl
target_opt_obj(`opt/anim.o', `M4FLAG_include_ANIMATION')dnl
target_gen
