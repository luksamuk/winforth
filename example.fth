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
here 'arr !
10 cells allot
here 'arr @ - #arr !

( getters, setters, utils )
: arridx ( idx -- a-addr )
  cell * 'arr @ + ;

: !num ( n idx -- )
  arridx ! ;

: @num ( idx -- n )
  arridx @ ;

: ?num ( idx -- n )
  @num . ;

( arithmetic example )
90 0 !num 30 1 !num
0 @num 1 @num + 2 !num
2 ?num

cr ." example.fth was successfully loaded!" cr
