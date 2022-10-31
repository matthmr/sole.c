#!/usr/bin/env bash
case $1 in
	'-h'|'--help')
		printf 'Usage:       scripts/inline.sh
Description: Inline meta-programatic data in C sources
Variables:   SED="sed-like command"
Note:        Make sure to call this script from the repository root
'
		exit 1
		;;
esac

[[ -z $SED ]] && SED=sed

echo "[ INFO ] SED=$SED"

echo "[ .. ] Inlining to \`draw.h'"

SIZE="$(scripts/errmsglen.awk draw.c)"

echo "[ INFO ] [draw.h] ERRMSGLEN=$SIZE"

$SED -i \
	-e "s:^\(# *define ERRMSGLEN\).*$:\1 $SIZE:" \
	-e 's:^# *undef ERRMSGLEN$://&:' \
	draw.h

echo "[ OK ] Done"
