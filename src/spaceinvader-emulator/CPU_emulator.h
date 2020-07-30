/*
  Author: Gabriel Karras
  Date: 08/06/2020
  License: DOWHATEVERYOUWANT
  Contact: gavrilkarras@hotmail.com

  A complete emulation of all the opcode of the Intel 8080 CPU architecture

*/
#include<iostream>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>


/*
  The Flag register holds 6 flags

  Zero: If the result of an instruction has the value of 0
        This flag is set; otherwise it is reset
  Sign: If the most significant bit(MSB) of the result in an operation
        This flag is set; otherwise it is reset
  Parity: If the modulo 2 sum of the bits of the result in an operation
          is 0(even parity), this flag is set; otherwise it is reset(odd parity)
  Carry: If the instruction resulted in a carry or borrow out of the MSB
         This flag is set; otherwise it is reset
  Auxilliary Carry: If the instruction caused a carry out
                    of bit 3 and into bit 4 of the resulting
                    value, the auxiliary carry is set; otherwise
                    it is reset. This flag is affected by single
                    precision additions, subtractions, increments,
                    decrements, comparisons, and logical operations,
                    but is principally used with additions and increments 
                    preceding a DAA (Decimal Adjust Accumulator) instruction.               
*/

typedef struct ConditionFlags {
  uint8_t z:1; // Zero condition bit
  uint8_t s:1; // Sign condtion bit
  uint8_t p:1; // Parity condtion bit
  uint8_t cy:1; // Carry condtion bit
  uint8_t ac:1; // Auxiliary carry condtion bit
  uint8_t pad:3; // Padding bits
} ConditionFlags;

typedef struct States {
  uint8_t a; // Accumulator register
  uint8_t b; // Register B(register pair BC)
  uint8_t c; // Register C
  uint8_t d; // Register D(register pair DE)
  uint8_t e; // Register E
  uint8_t h; // Register H(register pair HL)
  uint8_t l; // Register L
  uint16_t sp; // Stack pointer
  uint16_t pc; // Program counter
  uint8_t *memory;
  struct ConditionFlags cc;
  uint8_t int_enable; // Enable feature(for particular OpCodes)
} States;


/* Function declarations */
void IncompleteInstruction(States *state);
int Parity8b(int x);
int Parity16b(int x);
int Disassembler(uint8_t *codebuffer, int pc);
int Emulator(States *state);
