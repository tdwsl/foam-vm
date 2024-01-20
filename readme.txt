[Foam VM - tdwsl 2024]

Foam VM is a simple stack-based virtual machine, with fixed-width encoding of
instructions. It can be programmed using a FORTH-like assembler with unique
syntax. It's main stand-out feature is that most instructions can take literals
as operands instead of always using the stack, and DUP can be encoded into the
beginning of each instruction.

Foam VM has 26 instructions:

  CALL LIT  JMP  NEXT
  JZ   JNZ  RET  I
  RPH  RPL  DROP PICK
  ADD  SUB  AND  OR
  XOR  MUL  DIV  MOD
  GTN  LTN  STW  LDW
  SWAP --   --   --
  --   --   --   HALT

[Instruction formatting]

Instructions are encoded in the top 5 bits of each opcode. The subsequent 3
bits encode the DUP, POP and IND flags, respectively. The DUP flag executes the
DUP stack operation before the execution of the instruction. The POP flag
determines whether the value used (or ignored) during the instruction is popped
from the top of the stack, or is encoded within the instruction. The IND flag
determines whether to use the previous value as-is, or to load the value from
the memory location it points to.  The last 24 bits of the instruction is a
signed 24 bit immediate value.

Instruction code
|     POP flag
|     | DUP flag
|     | | IND flag
|     | | | 24-bit signed immediate value
|     | | | |
v     v v v v
XXXXX P D I ######## ######## ########

The stack:

Instructions in Foam VM make use of two stacks, the return stack and the data
stack. Neither of these stacks reside in memory. Each stack is 256 words deep,
and will wrap around in the case of overflow - there is no way to get the
current value of either stack pointer.

[Memory]

Memory in Foam VM is organised into 32 bit words as the smallest addressable
unit. Foam VM is lightweight, but it isn't particularly dense.

[Instruction set]

In the following instructions, "A" refers to either the value popped from the
top of the data stack, or the immediate value encoded within the instruction.

00 CALL   --    Push PC to return stack, jump to A
01 LIT    -- n  Push A to the data stack
02 JMP    --    Jump to A
03 NEXT   --    Decrement top of RS. If >= 0, jump to A, else drop top of RS
04 JZ   n --    Pop top of the data stack, if zero then jump to A
05 JNZ  n --    Pop top of the data stack, if not zero then jump to A
06 RET    --    Pop the top of the return stack to PC
07 I      -- n  Pick A deep value of return stack, where I 0 = old PC
08 RPH    --    Push A to the return stack
09 RPL    -- n  Pop the top of the return stack to the data stack
0A DROP   --    Drop the top A values from the return stack
0B PICK   -- n  Pick A deep value of the data stack, where PICK 0 = DUP
0C ADD    --    Add A to top of data stack
0D SUB    --    Subtract A from top of data stack
0E AND    --    Bitwise and top of data stack with A
0F OR     --    Bitwise or top of data stack with A
10 XOR    --    Bitwise xor top of data stack with A
11 MUL    --    Multiply top of data stack by A
12 DIV    --    Divide top of data stack by A, signed
13 MOD    --    Get modulo/unsigned remainder of top of data stack and A
14 GTN    --    If TOS > A, set TOS to -1, else set TOS to 0
15 LTN    --    If TOS < A, set TOS to -1, else set TOS to 0
16 STW    --    Store top of data stack at memory location A
17 LDW    --    Push value at memory location A to data stack
18 SWAP   --    Swap TOS and the value A deep, where SWAP 0 does nothing
19 --     --
1A --     --
1B --     --
1C --     --
1D --     --
1E --     --
1F HALT   --    Halt execution of Foam VM

