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
int Parity(int x, int size);


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
int Parity(int x, int size)
{
  return 0;
}

int Emulator(States *state)
{
  uint8_t *opcode = &state->memory[state->pc];

  state->pc += 1;
  switch(*opcode)
  {
    case 0x00: break;// NOP
    case 0x01: // LXI B,word(2 bytes)
               state->c = opcode[1];
               state->b = opcode[2];
               state-> pc += 2;//Advance by 2 bytes
               break;
    case 0x02: IncompleteInstruction(state); break;
    case 0x03: IncompleteInstruction(state); break;
    case 0x04: IncompleteInstruction(state); break;
    case 0x0f: // RRC
        {
          uint8_t x = state->a;
          state->a = ((x & 1) << 7) | (x >> 1);
          state->cc.cy = (1 == (x&1));
        } break;
    case 0x1f: // RAR
        {
          uint8_t x = state->a;
          state->a = (state->cc.cy << 7) | (x >> 1);
          state->cc.cy = (1 == (x&1));
        } break;
    case 0x2f: // CMA
               state->a = ~state->a;
               break;
    case 0x41: // MOV B,C
               state->b = state->c;
               break;
    case 0x42: // MOV B,D
               state->b = state->d;
               break;
    case 0x43: // MOV B,E
               state->b = state->e;
               break;
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
    case 0xc1: // POP B
        {
          state->c = state->memory[state->sp];
          state->b = state->memory[state->sp+1];
          state->sp += 2;
        } break;
    case 0xc2: // JNZ addr
        {
          if(state->cc.z == 0){
            state->pc = (opcode[2] << 8) | opcode[1];
          } else {
            state->pc += 2;
          }
        } break;
    case 0xc3: // JMP addr
              state->pc = (opcode[2]<<8) | opcode[1];
              break;
    case 0xc5: // PUSH B
        {
          state->memory[state->sp-1] = state->b;
          state->memory[state->sp-2] = state->c;
          state->sp = state->sp - 2;
        } break;
    case 0xc6: // ADI byte
        {
          uint16_t answer = (uint16_t)state->a + (uint16_t)opcode[1];
          state->cc.z = ((answer & 0xff) == 0);
          state->cc.s = ((answer & 0x80) != 0);
          state->cc.cy = (answer > 0xff);
          state->cc.p = Parity(answer & 0xff);
          state->a = answer & 0xff;
        } break;
    case 0xc9: // RET
        {
              state->pc = state->memory[state->sp] |
                          (state->memory[state->sp+1]<<8);
              state->sp += 2;
        } break;
    case 0xcd: // CALL addr
        {
          uint16_t ret = state->pc+2;
          state->memory[state->sp-1] = (ret>>8) & 0xff;
          state->memory[state->sp-2] = (ret & 0xff);
          state->sp = state->sp - 2;
          state->pc = (opcode[2]<<8) | opcode[1];
        } break;
    case 0xe6: // ANI 1 byte
        {
          uint8_t x = state->a & opcode[1];
          state->cc.z = (x == 0);
          state->cc.s = ((x & 0x80) == 0x80);
          state->cc.p = Parity(x, 8);
          state->cc.cy = 0;
          state->a = x;
          state->pc++;
        } break;
    case 0xf1: // POP PSW
        {
          state->a = state->memory[state->sp+1];
          uint8_t psw = state->memory[state->sp];
          state->cc.z = ((psw & 0x01) == 0x01);
          state->cc.s = ((psw & 0x02) == 0x02);
          state->cc.p = ((psw & 0x04) == 0x04);
          state->cc.cy = ((psw & 0x08) == 0x08);
          state->cc.ac = ((psw & 0x10) == 0x10);
          state->sp += 2;
        } break;
    case 0xf5: // PUSH PSW
        {
          state->memory[state->sp-1] = state->a;
          uint8_t psw = (state->cc.z | state->cc.s << 1 | state->cc.p << 2|
                         state->cc.cy << 3 | state->cc.ac << 4);
          state->memory[state->sp-2] = psw;
          state->sp = state->sp - 2;
        } break;
    case 0xfe: // CPI 1 byte
        {
          uint8_t x = state->a - opcode[1];
          state->cc.z = (x == 0):
          state->cc.s = ((x & 0x80) == 0x80);
          state->cc.p = parity(x, 8);
          state->cc.cy = (state->a < opcode[1]);
          state->pc++;
        } break;
    case 0xff: IncompleteInstruction(state); break;
  }
  state->pc += 1;
}
