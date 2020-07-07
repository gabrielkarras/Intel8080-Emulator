#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>


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
  uint8_t int_enable;
} States;


void IncompleteInstruction(States *state);
int Emulator(States *state);


int main(int argc, char **argv){

  return 0;
}


void IncompleteInstruction(States *state)
{
  state->pc -= 1; // undoing PC increment
  printf("Error: Incomplete Instrction\n");
  EXIT_FAILURE;
}

int Emulator(States *state)
{
  uint8_t *opcode = &state->memory[state->pc];

  switch(*opcode)
  {
    case 0x00: break;// NOP
    case 0x01: state->c = opcode[1];// LXI B,word(16bits/2bytes)
               state->b = opcode[2];
               state-> pc += 2;//Advance by 2 bytes
               break;
    case 0x02: IncompleteInstruction(state); break;
    case 0x03: IncompleteInstruction(state); break;
    case 0x04: IncompleteInstruction(state); break;
    /* ... */
    case 0x41: state->b = state->c; break;// MOVE B,C
    case 0x42: state->b = state->d; break;// MOV B,D
    case 0x43: state->b = state->e; break;// MOVE B,E
    /* ... */
    case 0xFE: IncompleteInstruction(state); break;
    case 0xFF: IncompleteInstruction(state); break;
  }
  state->pc += 1;
}
