( hello world )

|sys.inc
=0

#msg puts halt.

:msg "Hi! [ 5 + 2 * 2 - 4 ] 0

:puts
  { dup @ dup jz: ->
    !chout +1 jmp: <- }
  drop: 2
  ret.

