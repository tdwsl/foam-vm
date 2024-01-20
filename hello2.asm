( hello world, again! )

|sys.inc
=0

#10 and: 20
#msg puts
#1020 putn #10 !chout
#-679 putn #10 !chout
#$f23456 /: 256 and: $fffff puth #10 !chout
halt.

:msg "Hello, 32 "world! 10 0

:.digit
  +"0 !chout ret.

:.digith
  dup <: 10 jnz: .digit
  +[ "a - 10 ] !chout ret.

:pmsk $10000000
:puth
  dup /: 16 mod: pmsk ^ dup if dup puth then drop: 1
  and: 15 jmp: .digith

:putn
  dup <: 0 if xor: $ffffff +1 #"- !chout then
  { dup /: 10 dup if dup <- then drop: 1 }
  mod: 10 jmp: .digit

:puts
  dup @ dup 0if ret. then
  !chout +1 jmp: puts

