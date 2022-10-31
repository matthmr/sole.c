define(`debug', `-DDEBUG=$(DEBUG)')dnl
define(`animation', `-DANIMATION=$(ANIMATION)')dnl
define(`file', `define(`M4FLAG_include_$1', `$2')')dnl
dnl
file(`sole', debug animation)dnl
file(`draw', debug)dnl
undefine(`file')dnl
undefine(`header')dnl
include(make/Objects.m4)
