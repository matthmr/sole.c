define(`rec', `ifelse($1, `', ,`$1 rec(shift($@))')')dnl
define(`ifrec', `ifelse($1, `', ,`ifelse($2, `1', `$1 ifrec(shift(shift($@)))', )')')dnl
dnl
define(`target', `define(`__target__', `$1')')dnl
define(`target_obj', `define(`__target_objects__', `rec($@)')')dnl
define(`target_opt_obj', `define(`__target_opt_objects__', `ifrec($@)')')dnl
define(`target_gen', `dnl
__target__: __target_objects__ __target_opt_objects__')dnl
