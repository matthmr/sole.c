#!/usr/bin/env bash

case $1 in
	'-h'|'--help')
		printf 'Usage:       scripts/make-deps.sh
Description: Makes the dependencies for objects
Variables:   CC=[C compiler]
             M4=[m4-like command]
	     SED=[sed-like command]
Note:        Make sure to call this script from the repository root
'
		exit 1
		;;
esac

echo '[ .. ] Generating dependencies'

[[ -z $CC ]]  && CC=cc
[[ -z $M4 ]]  && M4=m4
[[ -z $SED ]] && SED=sed

echo "[ INFO ] CC = $CC"

# fix-up for local development branch: all test files have `-test' prefix
find . -name '*.c' -and -not -name '*-test*' -type f | xargs $CC -MM > make/Autodep.mk

# fix up files in nested directories
awk '
$2 ~ /.*\/.*/ {
  basedir=$2
  sub(/\/.*?$/, "", basedir);
  printf "%s/%s\n", basedir, $0;
  next
}
{
  print
}' make/Autodep.mk > make/Deps.mk

# get object files
cat make/Deps.mk |\
  cut -d: -f1 |\
  tr '\n' ' ' |\
  $SED 's: $:\n:' |\
  $SED -e 's/^/OBJECTS:=&/' >> make/Deps.mk

# set up future template for `make-makefile'
awk -F: '
BEGIN {
  cmsg="";
  ccmd="";
}
/^OBJECTS/ {
  print
  next
}
{
  base=$1;
  sub(/\.o/, "", base);
  base_include=base;

  if (base ~ /.*\/.*/) {
    sub(/\//, "I", base_include);
  }

  cmsg = "@echo \"CC " base ".c\"";
  ccmd = "@$(CC) -c M4FLAG_include_" base_include " $(CFLAGS) $(CFLAGSADD) $< -o $@";
  printf "%s\n\t%s\n\t%s\n", $0, cmsg, ccmd;
}
' make/Deps.mk > make/Objects.m4

echo '[ .. ] Generating objects'
eval "$M4 $M4FLAGS make/Include.m4" | $SED 's/M4FLAG_include_[A-Za-z]*/ /g' > make/Objects.mk
eval "$M4 $M4FLAGS make/Targets.m4" > make/Targets.mk

echo '[ .. ] Generating targets'
cat make/Targets.mk |\
  cut -d: -f1 |\
  tr '\n' ' ' |\
  $SED 's: $:\n:' |\
  $SED -e 's/^/TARGETS:=&/g' >> make/Targets.mk

echo '[ OK ] Done'
