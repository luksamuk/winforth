: square ( n -- n )
  dup * ;

: sum-of-squares ( a b -- r )
  square swap square + ;

: fact ( n -- n! )
  dup 1 <= if 1 else dup 1 - fact then * ;

: cr ( -- )
  10 emit ;
