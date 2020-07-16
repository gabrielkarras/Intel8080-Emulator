# Intel8080-Emulator
An emulation of the Intel 8080 CPU architecture

## References
[Intel8080 OpCode Reference List](http://emulator101.com/)

[Intel8080 User's Manual](http://www.nj7p.info/Manuals/PDFs/Intel/9800153B.pdf)

## Disassembler
With the help of the references above, I fully implemented a disassembler for the Intel 8080 instruction set

If you wish to run it:
1. cd /src/disassembler/ (cd into the correct folder)
2. gcc disassembler.c -o disassembler (run gcc compiler)
3. ./disassembler

Your results should look like the screenshot below. However, if you wish to have a complete & thorough comparison use this reference to the [complete Space Invaders' code](http://computerarcheology.com/Arcade/SpaceInvaders/Code.html)

![disassembler_result](https://user-images.githubusercontent.com/30480951/87622306-d834a180-c6f0-11ea-85ff-22a0546c3db6.png)

## Emulator-Space Invaders(only 50 OpCodes)
Similarly, I implemented an emulator for the Intel8080 CPU architecture. At this stage I've implemented the 50 suggested opcodes that will read and run Space Invaders.

If you wish to run it:
1. cd /src/ (cd into the correct folder)
2. gcc main.c -o main (run gcc compiler)
3. ./main

Your results should look like the screenshot below. I've used this [Javascript based emulator](https://bluishcoder.co.nz/js8080/) for step by step analysis.
![IntelCPU50OpCode](https://user-images.githubusercontent.com/30480951/87625254-b38ff800-c6f7-11ea-8408-72d8c7c09241.png)
