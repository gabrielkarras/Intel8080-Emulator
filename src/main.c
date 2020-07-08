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
int Parity(uint16_t x);


int main(int argc, char **argv){

  return 0;
}


/*
  For any unimplmented instruction in the Intel8080 instruction set
  We will simply exit the emulation(halt)
*/
void IncompleteInstruction(States *state)
{
  state->pc -= 1; // undoing PC increment
  printf("Error: Incomplete Instrction\n");
  EXIT_FAILURE;
}

/*
  Tests wether an input is even or odd parity bit
*/
int Parity(uint16_t x)
{
  return 0;
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
    case 0x80: // ADD B
        {
          uint16_t answer = (uint16_t)state->a + (uint16_t)state->b;
          state->cc.z = ((answer & 0xff) == 0);// If result is zero, set flag
          state->cc.s = ((answer & 0x80) != 0);// If bit 7 is set, set sign flag
          state->cc.cy = (answer > 0xff);// Set carry flag if exceeds 2 bytes
          state->cc.p = Parity(answer & 0xff);// Parity flag is set if even
          state->a = answer & 0xff;
        } break;
    case 0x81: // ADD C
        {
          uint16_t answer = (uint16_t)state->a + (uint16_t)state->c;
          state->cc.z = ((answer & 0xff) == 0);
          state->cc.s = ((answer & 0x80) != 0);
          state->cc.cy = (answer > 0xff);
          state->cc.p = Parity(answer & 0xff);
          state->a = answer & 0xff;
        } break;
    case 0x86: // ADD M
        {
          uint16_t offset = (state->h<<8) | (state->l);
          uint16_t answer = (uint16_t)state->a +
                            (uint16_t)state->memory[offset];
          state->cc.z = ((answer & 0xff) == 0);
          state->cc.s = ((answer & 0x80) != 0);
          state->cc.cy = (answer > 0xff);
          state->cc.p = Parity(answer & 0xff);
          state->a = answer & 0xff;
        } break;
    case 0xc2:
    case 0xc6: // ADI byte
        {
          uint16_t answer = (uint16_t)state->a + (uint16_t)opcode[1];
          state->cc.z = ((answer & 0xff) == 0);
          state->cc.s = ((answer & 0x80) != 0);
          state->cc.cy = (answer > 0xff);
          state->cc.p = Parity(answer & 0xff);
          state->a = answer & 0xff;
        } break;
    case 0xfe: IncompleteInstruction(state); break;
    case 0xff: IncompleteInstruction(state); break;
  }
  state->pc += 1;
}
