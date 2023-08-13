( init.fth -- Forth bootstrapping definitions )
( This file is a part of Winforth.            )
( Copyright (c) 2023 Lucas S. Vieira          )

: cr         10 emit 13 emit ;
: ?          @ . ;
: cells      cell * ;
: ,          here cell allot ! ;
: variable   align here 0 , constant ;
