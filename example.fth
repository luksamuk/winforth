( example.fth -- Some Winforth examples       )
( This file is a part of Winforth.            )
( Copyright (c) 2023 Lucas S. Vieira          )

: square ( n -- n )
  dup * ;

: sum-of-squares ( a b -- r )
  square swap square + ;

: fact ( n -- n! )
  dup 1 <= if 1 else dup 1 - fact then * ;


( allocating an array )
variable 'arr
variable #arr
align
here 'arr !
10 4 cells * allot
here 'arr @ - #arr !

: arridx ( idx -- a-addr )
  4 cells * 'arr @ + ;

: !num ( n idx -- )
  arridx ! ;

: @num ( idx -- n )
  arridx @ ;

: ?num ( idx -- n )
  @num . ;

cr ." example.fth was successfully loaded!" cr
