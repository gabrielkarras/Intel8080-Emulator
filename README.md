# Intel8080-Emulator
An emulation of the Intel 8080 CPU architecture

## References
[Intel8080 OpCode Reference List](http://emulator101.com/)

[Intel8080 User's Manual](http://www.nj7p.info/Manuals/PDFs/Intel/9800153B.pdf)

## Disassembler
WIth the help of the references above, I fully implemented a disassembler for the Intel 8080 instruction set

If you wish to run it:
1. cd /src/disassembler/ (cd into the correct folder)
2. gcc disassembler.c -o disassembler (run gcc compiler)
3. ./disassembler

Your results should look like the screenshot below

![disassembler_result](https://user-images.githubusercontent.com/30480951/87622306-d834a180-c6f0-11ea-85ff-22a0546c3db6.png)
