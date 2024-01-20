
( test tile graphics )

|sys.inc
=0

#$0000c0 !tile_palette
#$ffffff ![ tile_palette + 1 ]
#$a0a0a0 ![ tile_palette + 2 ]

#tilea fill

#update !vupdate
#keydown !vkeydown

{ jmp: <- }

:check 1

:keydown
  drop: 1
  @check if #tileb #0 else #tilea #1
  then !check fill
  ret.

:scroll
  dup @ +1 @check if and: 7 then
  swap: 1 ! ret.

:update
  #tile_xoffset scroll
  @tile_xoffset and: 1 if
    #tile_yoffset scroll
  then
  ret.

:fill
  #[ 49 * 29 - 1 ] >r {
    dup i. +tile_screen !
    next: <-
  } drop: 1 ret.

:tilea
  $00000000
  $01111100
  $12000210
  $10000010
  $11111110
  $10000010
  $10000010
  $10000010

:tileb
  $00000000
  $11111100
  $10000010
  $10000210
  $11111100
  $10000210
  $10000010
  $11111100

