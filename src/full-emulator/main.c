/*
  Author: Gabriel Karras
  Date: 08/06/2020
  License: DOWHATEVERYOUWANT
  Contact: gavrilkarras@hotmail.com

  A complete emulation of all the opcode of the Intel 8080 CPU architecture

  We perform CPU diagnostic test to verify the validity of every opcode
  The test is part of binary file named cpudiag.bin
*/
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>


/* Definitions */
#define FILE_NAME "cpudiag.bin"


/* Struct definitions */
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
void ReadIntoMemory(States *state, char *filename, uint32_t offset);
int Disassembler(uint8_t *codebuffer, int pc);
int Emulator(States *state);


int main(int argc, char **argv)
{

  int EOI = 0; // End Of Instruction

  /* Allocate and initialize memory */
  States *state = calloc(1, sizeof(States));
  /* Allocate for 16bits address/64Kbytes */
  state->memory = malloc(0x10000);

  /*
    Read cpudiag binary starting from 0x100
    Avoids the instruction 'JMP $0100'
  */
  ReadIntoMemory(state, FILE_NAME, 0x100);

  /* Fix 'JMP 0x100' instruction */
  state->memory[0] = 0xc3;
  state->memory[1] = 0;
  state->memory[2] = 0x01;

  /*
    Fix SP from 0x6ad to 0x7ad
    Byte #112(0x70) + offset of 256 bytes(0x100) = total offset 368(0x170)
   */
  state->memory[368] = 0x7;

  /* Skip DAA test */
  state->memory[0x59c] = 0xc3; // JMP
  state->memory[0x59] = 0xc2;
  state->memory[0x59e] = 0x05;

  /*
    Loop until end of program
    Or until emulator reads incomplete instruction
  */
  while ( EOI == 0 ){
    EOI = Emulator(state);
  }

  return 0;
}


/* Function implementation */

/*
 * Function:  IncompleteInstruction
 * --------------------------------
 *  Halts emulation for any incomplete or uninplemented instruction
 *
 *  state: state of Intel8080 machine
 *
 *  returns: void - exits emulation
 */
void IncompleteInstruction(States *state)
{
  state->pc -= 1; // undoing PC increment
  printf("Error: Incomplete Instruction-Emulation Halted\n");
  Disassembler(state->memory, state->pc);
  exit(EXIT_FAILURE);
}

/*
 * Function: Parity8b
 * ------------------
 *  Tests wether an input is even or odd parity bit
 *
 *  x: 8 bit interger
 *
 *  returns: 1 if even parity, else
 *           0
 */
int Parity8b(int x)
{
  x ^= x >> 4;
  x ^= x >> 1;
  x ^= x >> 2;
  return (~x) & 1;
}

/*
 * Function: Parity16b
 * -------------------
 *  Tests wether an input is even or odd parity bit
 *
 *  x: 16 bit integer
 *
 *  returns: 1 if even parity, else
 *           0
 */
int Parity16b(int x)
{
  x ^= x >> 8;
  x ^= x >> 4;
  x ^= x >> 1;
  x ^= x >> 2;
  return (~x) & 1;
}

/*
 * Function:  ReadIntoMemory
 * -------------------------
 *  State machine of emulator
 *  Will read from ROM and store into memory
 *
 *  state: state of Intel8080 machine
 *
 *  returns: void - exits emulation
 */
void ReadIntoMemory(States *state, char *filename, uint32_t offset)
{
  // Open file and verify status
  FILE *fp = fopen(filename, "rb");
  if(fp == NULL){
    printf("Can't open %s\n", filename);
    exit(EXIT_FAILURE);
  }

  // Get file size
  fseek(fp, 0L, SEEK_END);
  int fsize = ftell(fp);

  // Return to beginning of file to read into memory buffer
  fseek(fp, 0L, SEEK_SET);
  uint8_t *buffer = &state->memory[offset];

  // Read file into buffer
  fread(buffer, fsize, 1, fp);
  fclose(fp);
}

/*
 * Function: Disassembler
 * ----------------------
 *  Reads machine code and disassembles it to 8080 assembly instruction code
 *
 *  codebuffer: pointer to the 8080 assembly ROM from a memory buffer
 *  pc: the current program counter
 *
 *  returns: returns the number of bytes
 *           required from the OpCode(to increment PC)
 */
int Disassembler(uint8_t *codebuffer, int pc)
{
  uint8_t *code = &codebuffer[pc];
  int opbytes = 1; // Default OpCode size in bytes

  printf("%04x ", pc); // Prints current instruction location
  switch(*code)
  {
    case 0x00: printf("NOP"); break;
    case 0x01: printf("LXI B,$%02x%02x", code[2], code[1]);
               opbytes = 3;
               break;
    case 0x02: printf("STAX B"); break;
    case 0x03: printf("INX B"); break;
    case 0x04: printf("INR B"); break;
    case 0x05: printf("DCR B"); break;
    case 0x06: printf("MVI B,$%02x", code[1]);
               opbytes = 2;
               break;
    case 0x07: printf("RLC"); break;
    case 0x08: break; // Free OpCode
    case 0x09: printf("DAD B"); break;
    case 0x0a: printf("LDAX B"); break;
    case 0x0b: printf("DCX B"); break;
    case 0x0c: printf("INR C"); break;
    case 0x0d: printf("DCR C"); break;
    case 0x0e: printf("MCI C,$%02x", code[1]);
               opbytes = 2;
               break;
    case 0x0f: printf("RRC"); break;
    case 0x10: break; // Free OpCode
    case 0x11: printf("LXI D,$%02x%02x", code[2], code[1]);
               opbytes = 3;
               break;
    case 0x12: printf("STAX D"); break;
    case 0x13: printf("INX D"); break;
    case 0x14: printf("INR D"); break;
    case 0x15: printf("DCR D"); break;
    case 0x16: printf("MVI D,$%02x", code[1]);
               opbytes = 2;
               break;
    case 0x17: printf("RAL"); break;
    case 0x18: break; // Free OpCode
    case 0x19: printf("DAD D"); break;
    case 0x1a: printf("LDAX D"); break;
    case 0x1b: printf("DCX D"); break;
    case 0x1c: printf("INR E"); break;
    case 0x1d: printf("DCR E"); break;
    case 0x1e: printf("MVI E,$%02x", code[1]);
               opbytes = 2;
               break;
    case 0x1f: printf("RAR"); break;
    case 0x20: break; // Free OpCode
    case 0x21: printf("LXI H,$%02x%02x", code[2], code[1]);
               opbytes = 3;
               break;
    case 0x22: printf("SHLD $%02x%02x", code[2], code[1]);
               opbytes = 3;
               break;
    case 0x23: printf("INX H"); break;
    case 0x24: printf("INR H"); break;
    case 0x25: printf("DCR H"); break;
    case 0x26: printf("MVI H,$%02x", code[1]);
               opbytes = 2;
               break;
    case 0x27: printf("DAA"); // Allows decimal arithmetic
    case 0x28: break; // Free OpCode
    case 0x29: printf("DAD H"); break;
    case 0x2a: printf("LHLD $%02x%02x", code[2], code[1]);
               opbytes = 3;
               break;
    case 0x2b: printf("DCX H"); break;
    case 0x2c: printf("INR L"); break;
    case 0x2d: printf("DCR L"); break;
    case 0x2e: printf("MVI L,$%02x", code[1]);
               opbytes = 2;
               break;
    case 0x2f: printf("CMA"); break;
    case 0x30: break; // Free OpCode
    case 0x31: printf("LXI SP,$%02x%02x", code[2], code[1]);
               opbytes = 3;
               break;
    case 0x32: printf("STA $%02x%02x", code[2], code[1]);
               opbytes = 3;
               break;
    case 0x33: printf("INX SP"); break;
    case 0x34: printf("INR M"); break;
    case 0x35: printf("DCR M"); break;
    case 0x36: printf("MVI M,$%02x", code[1]);
               opbytes = 2;
               break;
    case 0x37: printf("STC"); break;
    case 0x38: break; // Free OpCode
    case 0x39: printf("DAD SP"); break;
    case 0x3a: printf("LDA $%02x%02x", code[2], code[1]);
               opbytes = 3;
               break;
    case 0x3b: printf("DCX SP"); break;
    case 0x3c: printf("INR A"); break;
    case 0x3d: printf("DCR A"); break;
    case 0x3e: printf("MVI A, $%02x", code[1]);
               opbytes = 2;
               break;
    case 0x3f: printf("CMC"); break;
    case 0x40: printf("MOV B,B"); break;
    case 0x41: printf("MOV B,C"); break;
    case 0x42: printf("MOV B,D"); break;
    case 0x43: printf("MOV B,E"); break;
    case 0x44: printf("MOV B,H"); break;
    case 0x45: printf("MOV B,L"); break;
    case 0x46: printf("MOV B,M"); break;
    case 0x47: printf("MOV B,A"); break;
    case 0x48: printf("MOV C,B"); break;
    case 0x49: printf("MOV C,C"); break;
    case 0x4a: printf("MOV C,D"); break;
    case 0x4b: printf("MOV C,E"); break;
    case 0x4c: printf("MOV C,H"); break;
    case 0x4d: printf("MOV C,L"); break;
    case 0x4e: printf("MOV C,M"); break;
    case 0x4f: printf("MOV C,A"); break;
    case 0x50: printf("MOV D,B"); break;
    case 0x51: printf("MOV D,C"); break;
    case 0x52: printf("MOV D,D"); break;
    case 0x53: printf("MOV D,E"); break;
    case 0x54: printf("MOV D,H"); break;
    case 0x55: printf("MOV D,L"); break;
    case 0x56: printf("MOV D,M"); break;
    case 0x57: printf("MOV D,A"); break;
    case 0x58: printf("MOV E,B"); break;
    case 0x59: printf("MOV E,C"); break;
    case 0x5a: printf("MOV E,D"); break;
    case 0x5b: printf("MOV E,E"); break;
    case 0x5c: printf("MOV E,H"); break;
    case 0x5d: printf("MOV E,L"); break;
    case 0x5e: printf("MOV E,M"); break;
    case 0x5f: printf("MOV E,A"); break;
    case 0x60: printf("MOV H,B"); break;
    case 0x61: printf("MOV H,C");	break;
    case 0x62: printf("MOV H,D"); break;
    case 0x63: printf("MOV H,E"); break;
    case 0x64: printf("MOV H,H"); break;
    case 0x65: printf("MOV H,L"); break;
    case 0x66: printf("MOV H,M"); break;
    case 0x67: printf("MOV H,A"); break;
    case 0x68: printf("MOV L,B"); break;
    case 0x69: printf("MOV L,C"); break;
    case 0x6a: printf("MOV L,D"); break;
    case 0x6b: printf("MOV L,E"); break;
    case 0x6c: printf("MOV L,H"); break;
    case 0x6d: printf("MOV L,L"); break;
    case 0x6e: printf("MOV L,M"); break;
    case 0x6f: printf("MOV L,A"); break;
    case 0x70: printf("MOV M,B"); break;
    case 0x71: printf("MOV M,C"); break;
    case 0x72: printf("MOV M,D"); break;
    case 0x73: printf("MOV M,E"); break;
    case 0x74: printf("MOV M,H"); break;
    case 0x75: printf("MOV M,L"); break;
    case 0x76: printf("HLT"); break;
    case 0x77: printf("MOV M,A"); break;
    case 0x78: printf("MOV A,B"); break;
    case 0x79: printf("MOV A,C"); break;
    case 0x7a: printf("MOV A,D"); break;
    case 0x7b: printf("MOV A,E"); break;
    case 0x7c: printf("MOV A,H"); break;
    case 0x7d: printf("MOV A,L"); break;
    case 0x7e: printf("MOV A,M"); break;
    case 0x7f: printf("MOV A,A"); break;
    case 0x80: printf("ADD B"); break;
    case 0x81: printf("ADD C"); break;
    case 0x82: printf("ADD D"); break;
    case 0x83: printf("ADD E"); break;
    case 0x84: printf("ADD H"); break;
    case 0x85: printf("ADD L"); break;
    case 0x86: printf("ADD M"); break;
    case 0x87: printf("ADD A"); break;
    case 0x88: printf("ADC B"); break;
    case 0x89: printf("ADC C"); break;
    case 0x8a: printf("ADC D"); break;
    case 0x8b: printf("ADC E"); break;
    case 0x8c: printf("ADC H"); break;
    case 0x8d: printf("ADC L"); break;
    case 0x8e: printf("ADC M"); break;
    case 0x8f: printf("ADC A"); break;
    case 0x90: printf("SUB B"); break;
    case 0x91: printf("SUB C"); break;
    case 0x92: printf("SUB D"); break;
    case 0x93: printf("SUB E"); break;
    case 0x94: printf("SUB H"); break;
    case 0x95: printf("SUB L"); break;
    case 0x96: printf("SUB M"); break;
    case 0x97: printf("SUB A"); break;
    case 0x98: printf("SBB B"); break;
    case 0x99: printf("SBB C"); break;
    case 0x9a: printf("SBB D"); break;
    case 0x9b: printf("SBB E"); break;
    case 0x9c: printf("SBB H"); break;
    case 0x9d: printf("SBB L"); break;
    case 0x9e: printf("SBB M"); break;
    case 0x9f: printf("SBB A"); break;
    case 0xa0: printf("ANA B"); break;
    case 0xa1: printf("ANA C"); break;
    case 0xa2: printf("ANA D"); break;
    case 0xa3: printf("ANA E"); break;
    case 0xa4: printf("ANA H"); break;
    case 0xa5: printf("ANA L"); break;
    case 0xa6: printf("ANA M"); break;
    case 0xa7: printf("ANA A"); break;
    case 0xa8: printf("XRA B"); break;
    case 0xa9: printf("XRA C"); break;
    case 0xaa: printf("XRA D"); break;
    case 0xab: printf("XRA E"); break;
    case 0xac: printf("XRA H"); break;
    case 0xad: printf("XRA L"); break;
    case 0xae: printf("XRA M"); break;
    case 0xaf: printf("XRA A"); break;
    case 0xb0: printf("ORA B"); break;
    case 0xb1: printf("ORA C"); break;
    case 0xb2: printf("ORA D"); break;
    case 0xb3: printf("ORA E"); break;
    case 0xb4: printf("ORA H"); break;
    case 0xb5: printf("ORA L"); break;
    case 0xb6: printf("ORA M"); break;
    case 0xb7: printf("ORA A"); break;
    case 0xb8: printf("CMP B"); break;
    case 0xb9: printf("CMP C"); break;
    case 0xba: printf("CMP D"); break;
    case 0xbb: printf("CMP E"); break;
    case 0xbc: printf("CMP H"); break;
    case 0xbd: printf("CMP L"); break;
    case 0xbe: printf("CMP M"); break;
    case 0xbf: printf("CMP A"); break;
    case 0xc0: printf("RNZ"); break;
    case 0xc1: printf("POP B"); break;
    case 0xc2: printf("JNZ $%02x%02x", code[2], code[1]);
               opbytes = 3;
               break;
    case 0xc3: printf("JMP $%02x%02x", code[2], code[1]);
               opbytes = 3;
               break;
    case 0xc4: printf("CNZ $%02x%02x", code[2], code[1]);
               opbytes = 3;
               break;
    case 0xc5: printf("PUSH B"); break;
    case 0xc6: printf("ADI $%02x", code[1]);
               opbytes = 2;
               break;
    case 0xc7: printf("RST 0"); break;
    case 0xc8: printf("RZ"); break;
    case 0xc9: printf("RET"); break;
    case 0xca: printf("JZ $%02x%02x", code[2], code[1]);
               opbytes = 3;
               break;
    case 0xcb: break; // Free Opcode
    case 0xcc: printf("CZ $%02x%02x", code[2], code[1]);
               opbytes = 3;
               break;
    case 0xcd: printf("CALL $%02x%02x", code[2], code[1]);
    	         opbytes = 3;
               break;
    case 0xce: printf("ACI $%02x", code[1]);
        	     opbytes = 2;
               break;
    case 0xcf: printf("RST 1"); break;
    case 0xd0: printf("RNC"); break;
    case 0xd1: printf("POP D"); break;
    case 0xd2: printf("JNC $%02x%02x", code[2], code[1]);
               opbytes = 3;
               break;
    case 0xd3: printf("OUT $%02x", code[1]);
               opbytes = 2;
               break;
    case 0xd4: printf("CNC $%02x%02x", code[2], code[1]);
               opbytes = 3;
               break;
    case 0xd5: printf("PUSH D"); break;
    case 0xd6: printf("SUI D8");
               opbytes = 2;
               break;
    case 0xd7: printf("RST 2"); break;
    case 0xd8: printf("RC"); break;
    case 0xd9: break; // Free OpCode
    case 0xda: printf("JC $%02x%02x", code[2], code[1]);
               opbytes = 3;
               break;
    case 0xdb: printf("IN $%02x", code[1]);
               opbytes = 2;
               break;
    case 0xdc: printf("CC $%02x%02x", code[2], code[1]);
               opbytes = 3;
               break;
    case 0xdd: break; // Free OpCode
    case 0xde: printf("SBI $%02x", code[1]);
               opbytes = 2;
               break;
    case 0xdf: printf("RST 3"); break;
    case 0xe0: printf("RPO"); break;
    case 0xe1: printf("POP H"); break;
    case 0xe2: printf("JPO $%02x%02x", code[2], code[1]);
               opbytes = 3;
               break;
    case 0xe3: printf("XTHL"); break;
    case 0xe4: printf("CPO $%02x%02x", code[2], code[1]);
               opbytes = 3;
               break;
    case 0xe5: printf("PUSH H"); break;
    case 0xe6: printf("ANI $%02x", code[1]);
               opbytes = 2;
               break;
    case 0xe7: printf("RST 4"); break;
    case 0xe8: printf("RPE"); break;
    case 0xe9: printf("PCHL"); break;
    case 0xea: printf("JPE $%02x%02x", code[2], code[1]);
               opbytes = 3;
               break;
    case 0xeb: printf("XCHG"); break;
    case 0xec: printf("CPE $%02x%02x", code[2], code[1]);
               opbytes = 3;
               break;
    case 0xed: break; // Free OpCode
    case 0xee: printf("XRI $%02x", code[1]);
               opbytes = 2;
               break;
    case 0xef: printf("RST 5"); break;
    case 0xf0: printf("RP"); break;
    case 0xf1: printf("POP PSW"); break;
    case 0xf2: printf("JP $%02x%02x", code[2], code[1]);
               opbytes = 3;
               break;
    case 0xf3: printf("DI"); break;
    case 0xf4: printf("CP $%02x%02x", code[2], code[1]);
               opbytes = 3;
               break;
    case 0xf5: printf("PUSH PSW"); break;
    case 0xf6: printf("ORI $%02x", code[1]);
               opbytes = 2;
               break;
    case 0xf7: printf("RST 6");
    case 0xf8: printf("RM"); break;
    case 0xf9: printf("SPHL"); break;
    case 0xfa: printf("JM $%02x%02x", code[2], code[1]);
               opbytes = 3;
               break;
    case 0xfb: printf("EI"); break;
    case 0xfc: printf("CM $%02x%02x", code[2], code[1]);
               opbytes = 3;
               break;
    case 0xfd: break; // Free OpCode
    case 0xfe: printf("CPI $%02x", code[1]);
               opbytes = 2;
               break;
    case 0xff: printf("RST 7"); break;
    default: break;
  }

  printf("\n");

  return opbytes;
}

/*
 * Function: Emulator
 * --------------------
 *  Emulates the Intel8080 CPU architecture from given instructions
 *
 *  state: pointer to current state of machine
 *
 *  returns: 0 if instruction is processed
 */
int Emulator(States *state)
{
  uint8_t *opcode = &state->memory[state->pc];
  Disassembler(state->memory, state->pc);

  state->pc += 1;
  switch(*opcode)
  {
    case 0x00: break;// NOP
    case 0x01: // LXI B,D16
        {
          state->c = opcode[1];
          state->b = opcode[2];
          state-> pc += 2;//Advance by 2 bytes
        } break;
    case 0x02: // STAX B
        {
          uint16_t addr = ((state->b) << 8) | (state->c);
          state->memory[addr] = state->a;
        } break;
    case 0x03: // INX B
        {
          uint16_t answer = ( ((state->b) << 8) | (state->c) ) + 1;
          state->b = (answer >> 8) & 0xff;
          state->c = answer & 0xff;
        } break;
    case 0x04: // INR B
        {
          uint8_t answer = state->b + 1;
          state->cc.z = (answer == 0);
          state->cc.s = ((answer&0x80) == 0x80);
          state->cc.p = Parity8b(answer);
          // state->cc.ac; - unsure
          state->b = answer;
        } break;
    case 0x05: //DCR B
        {
          uint8_t answer = state->b - 1;
          state->cc.z = (answer == 0);// If result is zero, set flag
          state->cc.s = ((answer & 0x80) == 0x80);// If bit 7 is set, set sign flag
          state->cc.p = Parity8b(answer);// Parity flag is set if even
          state->b = answer;
        } break;
    case 0x06: // MVI B,D8
        {
          state->b = opcode[1];
          state->pc++;
        } break;
    case 0x07: // RLC
        {
          uint8_t x = state->a;
          state->a = ((x&0x80) >> 7) | (x << 1);
          state->cc.cy = ((x&0x80) == 0x80);
        }break;
    case 0x08: break; // NOP
    case 0x09: // DAD B
        {
          uint32_t rp = ((state->b)<< 8) | (state->c); // set B to MSByte
          uint32_t hl = ((state->h)<< 8) | (state->l); // set H to MSByte
          uint32_t answer = hl + rp; // Add HL + BC
          state->cc.cy = (answer > 0xffff);
          /* move MSByte to LSByte and clear upper half*/
          state->h = ( (answer>>8) & 0xff);
          state->l = (answer & 0xff); // Clear upper half
        } break;
    case 0x0a: // LDAX B
        {
          uint16_t rp_addr = ((state->b) << 8) | (state->c);
          state->a = state->memory[rp_addr];
        } break;
    case 0x0b: // DCX B
        {
          uint16_t answer = ( ((state->b) << 8) | (state->c)) - 1;
          state->b = (answer >> 8) & 0xff;
          state->c = answer&0xff;
        } break;
    case 0x0c: // INR C
        {
          uint8_t answer = state->c + 1;
          state->cc.z = (answer == 0);
          state->cc.s = ((answer&0x80) == 0x80);
          state->cc.p = Parity8b(answer);
          // state->cc.ac; - unsure
          state->c = answer;
        } break;
    case 0x0d: // DCR C
        {
          uint8_t answer = state->c - 1;
          state->cc.z = (answer == 0);
          state->cc.s = ((answer & 0x80) == 0x80);
          state->cc.p = Parity8b(answer);
          state->c = answer;
        } break;
    case 0x0e: // MVI C,D8
        {
          state->c = opcode[1];
          state->pc++;
        } break;
    case 0x0f: // RRC
        {
          uint8_t x = state->a;
          state->a = ((x & 1) << 7) | (x >> 1);
          state->cc.cy = (1 == (x&1));
        } break;
    case 0x10: break; // NOP
    case 0x11: // LXI D,D16
        {
          state->e = opcode[1];
          state->d = opcode[2];
          state-> pc += 2;
        } break;
    case 0x12: // STAX D
        {
          uint16_t addr = ((state->d) << 8) | (state->e);
          state->memory[addr] = state->a;
        } break;
    case 0x13: // INX D
        {
          uint16_t answer = ( ((state->d) << 8) | (state->e) ) + 1;
          state->d = ((answer >> 8) & 0xff);
          state->e = (answer & 0xff);
        } break;
    case 0x14: // INR D
        {
          uint8_t answer = state->d + 1;
          state->cc.z = (answer == 0);
          state->cc.s = ((answer&0x80) == 0x80);
          state->cc.p = Parity8b(answer);
          // state->cc.ac; - unsure
          state->d = answer;
        } break;
    case 0x15: // DCR D
        {
          uint8_t answer = state->d - 1;
          state->cc.z = (answer == 0);
          state->cc.s = ((answer & 0x80) == 0x80);
          state->cc.p = Parity8b(answer);
          state->d = answer;
        } break;
    case 0x16: // MVI D,D8
        {
          state->d = opcode[1];
          state->pc++;
        } break;
    case 0x17: // RAL
        {
          uint8_t x = state->a;
          state->a = ((state->cc.cy)&0x01) | (x << 1);
          state->cc.cy = ((x&0x80) == 0x80);
        } break;
    case 0x18: IncompleteInstruction(state); break;
    case 0x19: // DAD D
        {
          uint32_t rp = ((state->d)<< 8) | (state->e);
          uint32_t hl = ((state->h)<< 8) | (state->l);
          uint32_t answer = hl + rp; // Add HL + DE
          state->cc.cy = (answer > 0xffff);
          state->h = ( (answer>>8) & 0xff);
          state->l = (answer & 0xff);
        } break;
    case 0x1a: // LDAX D
        {
          uint16_t rp_addr = ((state->d) << 8) | (state->e);
          state->a = state->memory[rp_addr];
        }break;
    case 0x1b: IncompleteInstruction(state); break;
    case 0x1c: IncompleteInstruction(state); break;
    case 0x1d: IncompleteInstruction(state); break;
    case 0x1e: IncompleteInstruction(state); break;
    case 0x1f: // RAR
        {
          uint8_t x = state->a;
          state->a = ((state->cc.cy) << 7) | (x >> 1);
          state->cc.cy = (1 == (x&1));
        } break;
    case 0x20: IncompleteInstruction(state); break;
    case 0x21: // LXI H,D16
        {
          state->l = opcode[1];
          state->h = opcode[2];
          state-> pc += 2;
        } break;
    case 0x22: IncompleteInstruction(state); break;
    case 0x23: // INX H
        {
          uint16_t rp = ((state->h) << 8) | (state->l);
          uint16_t answer = rp + 1;
          state->h = ((answer >> 8) & 0xff);
          state->l = (answer & 0xff);
        } break;
    case 0x24: IncompleteInstruction(state); break;
    case 0x25: IncompleteInstruction(state); break;
    case 0x26: // MVI H,D8
        {
          state->h = opcode[1];
          state->pc++;
        } break;
    case 0x27: IncompleteInstruction(state); break;
    case 0x28: IncompleteInstruction(state); break;
    case 0x29: // DAD H
        {
          uint16_t rp = ((state->h)<< 8) | (state->l);
          uint16_t answer = rp + rp; // Add HL + HL
          state->cc.cy = (answer > 0xffff);
          state->h = ( (answer>>8) & 0xff);
          state->l = (answer & 0xff);
        } break;
    case 0x2a: IncompleteInstruction(state); break;
    case 0x2b: IncompleteInstruction(state); break;
    case 0x2c: IncompleteInstruction(state); break;
    case 0x2d: IncompleteInstruction(state); break;
    case 0x2e: IncompleteInstruction(state); break;
    case 0x2f: // CMA
        {
          state->a = ~state->a;
        } break;
    case 0x30: IncompleteInstruction(state); break;
    case 0x31: // LXI SP,D16
        {
          state->sp = ((opcode[2] << 8) | opcode[1]);
          state->pc += 2;
        } break;
    case 0x32: // STA addr
        {
          uint16_t addr = ((opcode[2] << 8) | opcode[1]); // Form address
          state->memory[addr] = state->a; // Load Acc to addr location
          state->pc += 2;
        } break;
    case 0x33: IncompleteInstruction(state); break;
    case 0x34: IncompleteInstruction(state); break;
    case 0x35: IncompleteInstruction(state); break;
    case 0x36: // MVI M,D8
        {
          uint16_t addr = ((state->h) << 8) | (state->l);
          state->memory[addr] = opcode[1];
          state->pc++;
        } break;
    case 0x37: IncompleteInstruction(state); break;
    case 0x38: IncompleteInstruction(state); break;
    case 0x39: IncompleteInstruction(state); break;
    case 0x3a: // LDA addr
        {
          uint16_t addr = ((opcode[2] << 8) | opcode[1]);
          state->a = state->memory[addr];
          state->pc +=2;
        } break;
    case 0x3b: IncompleteInstruction(state); break;
    case 0x3c: IncompleteInstruction(state); break;
    case 0x3d: IncompleteInstruction(state); break;
    case 0x3e: // MVI A,D8
        {
          state->a = opcode[1];
          state->pc++;
        } break;
    case 0x3f: IncompleteInstruction(state); break;
    case 0x40: IncompleteInstruction(state); break;
    case 0x41: // MOV B,C
        {
          state->b = state->c;
        } break;
    case 0x42: // MOV B,D
        {
          state->b = state->d;
        } break;
    case 0x43: // MOV B,E
        {
          state->b = state->e;
        } break;
    case 0x44: IncompleteInstruction(state); break;
    case 0x45: IncompleteInstruction(state); break;
    case 0x46: IncompleteInstruction(state); break;
    case 0x47: IncompleteInstruction(state); break;
    case 0x48: IncompleteInstruction(state); break;
    case 0x49: IncompleteInstruction(state); break;
    case 0x4a: IncompleteInstruction(state); break;
    case 0x4b: IncompleteInstruction(state); break;
    case 0x4c: IncompleteInstruction(state); break;
    case 0x4d: IncompleteInstruction(state); break;
    case 0x4e: IncompleteInstruction(state); break;
    case 0x4f: IncompleteInstruction(state); break;
    case 0x50: IncompleteInstruction(state); break;
    case 0x51: IncompleteInstruction(state); break;
    case 0x52: IncompleteInstruction(state); break;
    case 0x53: IncompleteInstruction(state); break;
    case 0x54: IncompleteInstruction(state); break;
    case 0x55: IncompleteInstruction(state); break;
    case 0x56: // MOV D,M
        {
          uint16_t addr = ((state->h) << 8) | (state->l);
          state->d = state->memory[addr];
        } break;
    case 0x57: IncompleteInstruction(state); break;
    case 0x58: IncompleteInstruction(state); break;
    case 0x59: IncompleteInstruction(state); break;
    case 0x5a: IncompleteInstruction(state); break;
    case 0x5b: IncompleteInstruction(state); break;
    case 0x5c: IncompleteInstruction(state); break;
    case 0x5d: IncompleteInstruction(state); break;
    case 0x5e: // MOV E,M
        {
          uint16_t addr = ((state->h) << 8) | (state->l);
          state->e = state->memory[addr];
        } break;
    case 0x5f: IncompleteInstruction(state); break;
    case 0x60: IncompleteInstruction(state); break;
    case 0x61: IncompleteInstruction(state); break;
    case 0x62: IncompleteInstruction(state); break;
    case 0x63: IncompleteInstruction(state); break;
    case 0x64: IncompleteInstruction(state); break;
    case 0x65: IncompleteInstruction(state); break;
    case 0x66: // MOV H,M
        {
          uint16_t addr = ((state->h) << 8) | (state->l);
          state->h = state->memory[addr];
        } break;
    case 0x67: IncompleteInstruction(state); break;
    case 0x68: IncompleteInstruction(state); break;
    case 0x69: IncompleteInstruction(state); break;
    case 0x6a: IncompleteInstruction(state); break;
    case 0x6b: IncompleteInstruction(state); break;
    case 0x6c: IncompleteInstruction(state); break;
    case 0x6d: IncompleteInstruction(state); break;
    case 0x6e: IncompleteInstruction(state); break;
    case 0x6f: // MOV L,A
        {
          state->l = state->a;
        } break;
    case 0x70: IncompleteInstruction(state); break;
    case 0x71: IncompleteInstruction(state); break;
    case 0x72: IncompleteInstruction(state); break;
    case 0x73: IncompleteInstruction(state); break;
    case 0x74: IncompleteInstruction(state); break;
    case 0x75: IncompleteInstruction(state); break;
    case 0x76: IncompleteInstruction(state); break;
    case 0x77: // MOV M,A
        {
          uint16_t addr = ((state->h) << 8) | (state->l);
          state->memory[addr] = state->a;
        } break;
    case 0x78: IncompleteInstruction(state); break;
    case 0x79: IncompleteInstruction(state); break;
    case 0x7a: // MOV A,D
        {
          state->a = state->d;
        } break;
    case 0x7b: // MOV A,E
        {
          state->a = state->e;
        } break;
    case 0x7c: // MOV A,H
        {
          state->a = state->h;
        } break;
    case 0x7d: IncompleteInstruction(state); break;
    case 0x7e: // MOV A,M
        {
          uint16_t addr = ((state->h) << 8) | (state->l);
          state->a = state->memory[addr];
        } break;
    case 0x7f: IncompleteInstruction(state); break;
    case 0x80: // ADD B
        {
          uint16_t answer = (uint16_t)state->a + (uint16_t)state->b;
          state->cc.z = ((answer & 0xff) == 0);
          state->cc.s = ((answer & 0x80) != 0);
          state->cc.cy = (answer > 0xff);
          state->cc.p = Parity16b(answer & 0xff);
          state->a = answer & 0xff;
        } break;
    case 0x81: // ADD C
        {
          uint16_t answer = (uint16_t)state->a + (uint16_t)state->c;
          state->cc.z = ((answer & 0xff) == 0);
          state->cc.s = ((answer & 0x80) != 0);
          state->cc.cy = (answer > 0xff);
          state->cc.p = Parity16b(answer & 0xff);
          state->a = answer & 0xff;
        } break;
    case 0x82: IncompleteInstruction(state); break;
    case 0x83: IncompleteInstruction(state); break;
    case 0x84: IncompleteInstruction(state); break;
    case 0x85: IncompleteInstruction(state); break;
    case 0x86: // ADD M
        {
          uint16_t offset = (state->h<<8) | (state->l);
          uint16_t answer = (uint16_t)state->a +
                            (uint16_t)state->memory[offset];
          state->cc.z = ((answer & 0xff) == 0);
          state->cc.s = ((answer & 0x80) != 0);
          state->cc.cy = (answer > 0xff);
          state->cc.p = Parity16b(answer & 0xff);
          state->a = answer & 0xff;
        } break;
    case 0x87: IncompleteInstruction(state); break;
    case 0x88: IncompleteInstruction(state); break;
    case 0x89: IncompleteInstruction(state); break;
    case 0x8a: IncompleteInstruction(state); break;
    case 0x8b: IncompleteInstruction(state); break;
    case 0x8c: IncompleteInstruction(state); break;
    case 0x8d: IncompleteInstruction(state); break;
    case 0x8e: IncompleteInstruction(state); break;
    case 0x8f: IncompleteInstruction(state); break;
    case 0x90: IncompleteInstruction(state); break;
    case 0x91: IncompleteInstruction(state); break;
    case 0x92: IncompleteInstruction(state); break;
    case 0x93: IncompleteInstruction(state); break;
    case 0x94: IncompleteInstruction(state); break;
    case 0x95: IncompleteInstruction(state); break;
    case 0x96: IncompleteInstruction(state); break;
    case 0x97: IncompleteInstruction(state); break;
    case 0x98: IncompleteInstruction(state); break;
    case 0x99: IncompleteInstruction(state); break;
    case 0x9a: IncompleteInstruction(state); break;
    case 0x9b: IncompleteInstruction(state); break;
    case 0x9c: IncompleteInstruction(state); break;
    case 0x9d: IncompleteInstruction(state); break;
    case 0x9e: IncompleteInstruction(state); break;
    case 0x9f: IncompleteInstruction(state); break;
    case 0xa0: IncompleteInstruction(state); break;
    case 0xa1: IncompleteInstruction(state); break;
    case 0xa2: IncompleteInstruction(state); break;
    case 0xa3: IncompleteInstruction(state); break;
    case 0xa4: IncompleteInstruction(state); break;
    case 0xa5: IncompleteInstruction(state); break;
    case 0xa6: IncompleteInstruction(state); break;
    case 0xa7: // ANA A
        {
          uint8_t answer = (state->a) & (state->a); // A AND A
          state->cc.z = (answer == 0);
          state->cc.s = ((answer & 0x80) == 0x80);
          state->cc.p = Parity8b(answer);
          state->cc.cy = 0; // Cleared
          state->a = answer;
        } break;
    case 0xa8: IncompleteInstruction(state); break;
    case 0xa9: IncompleteInstruction(state); break;
    case 0xaa: IncompleteInstruction(state); break;
    case 0xab: IncompleteInstruction(state); break;
    case 0xac: IncompleteInstruction(state); break;
    case 0xad: IncompleteInstruction(state); break;
    case 0xae: IncompleteInstruction(state); break;
    case 0xaf: // XRA A
        {
          uint8_t answer = (state->a) ^ (state->a); // A XOR A
          state->cc.z = (answer == 0);
          state->cc.s = ((answer & 0x80) == 0x80);
          state->cc.p = Parity8b(answer);
          state->cc.cy = 0; // Cleared
          state->cc.ac = 0; // Cleared
          state->a = answer;
        } break;
    case 0xb0: IncompleteInstruction(state); break;
    case 0xb1: IncompleteInstruction(state); break;
    case 0xb2: IncompleteInstruction(state); break;
    case 0xb3: IncompleteInstruction(state); break;
    case 0xb4: IncompleteInstruction(state); break;
    case 0xb5: IncompleteInstruction(state); break;
    case 0xb6: IncompleteInstruction(state); break;
    case 0xb7: IncompleteInstruction(state); break;
    case 0xb8: IncompleteInstruction(state); break;
    case 0xb9: IncompleteInstruction(state); break;
    case 0xba: IncompleteInstruction(state); break;
    case 0xbb: IncompleteInstruction(state); break;
    case 0xbc: IncompleteInstruction(state); break;
    case 0xbd: IncompleteInstruction(state); break;
    case 0xbe: IncompleteInstruction(state); break;
    case 0xbf: IncompleteInstruction(state); break;
    case 0xc0: IncompleteInstruction(state); break;
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
        {
          state->pc = (opcode[2]<<8) | opcode[1];
        } break;
    case 0xc4: IncompleteInstruction(state); break;
    case 0xc5: // PUSH B
        {
          state->memory[state->sp-1] = state->b;
          state->memory[state->sp-2] = state->c;
          state->sp = state->sp - 2;
        } break;
    case 0xc6: // ADI D8
        {
          uint16_t answer = (uint16_t)state->a + (uint16_t)opcode[1];
          state->cc.z = ((answer & 0xff) == 0);
          state->cc.s = ((answer & 0x80) != 0);
          state->cc.cy = (answer > 0xff);
          state->cc.p = Parity16b(answer & 0xff);
          state->a = answer & 0xff;
          state->pc++;
        } break;
    case 0xc7: IncompleteInstruction(state); break;
    case 0xc8: IncompleteInstruction(state); break;
    case 0xc9: // RET
        {
              state->pc = state->memory[state->sp] |
                          (state->memory[state->sp+1]<<8);
              state->sp += 2;
        } break;
    case 0xca: IncompleteInstruction(state); break;
    case 0xcb: IncompleteInstruction(state); break;
    case 0xcc: IncompleteInstruction(state); break;
    case 0xcd: // CALL addr
    /*
      CPU diagnotic test makes call and
      prints to console with the help of CP/M OS
    */
    #ifdef CPUDIAG
          if ( ((opcode[2] << 8) | opcode[1]) == 5 ){
            if (state->c == 9){
              uint16_t addr = (state->d << 8) | (state->e);
              char *str = &state->memory[addr + 3];

              while(*str != '$'){
                printf("%c", *str++);
              }
              printf("\n");
            }
            else if(state->c == 2){
              printf("Print routine called\n");
            }
          }
          else if( ((opcode[2] << 8) | opcode[1]) == 0){
              exit(EXIT_SUCCESS);
          }
          else
    #endif
        {
          uint16_t ret = state->pc+2;
          state->memory[state->sp-1] = (ret>>8) & 0xff;
          state->memory[state->sp-2] = (ret & 0xff);
          state->sp = state->sp - 2;
          state->pc = (opcode[2]<<8) | opcode[1];
        } break;
    case 0xce: IncompleteInstruction(state); break;
    case 0xcf: IncompleteInstruction(state); break;
    case 0xd0: IncompleteInstruction(state); break;
    case 0xd1: // POP D
        {
          state->e = state->memory[state->sp];
          state->d = state->memory[state->sp+1];
          state->sp += 2;
        } break;
    case 0xd2: IncompleteInstruction(state); break;
    case 0xd3: // OUT D8
        {
          // need to verify user manual
          // state->a
          state->pc++;
        } break;
    case 0xd4: IncompleteInstruction(state); break;
    case 0xd5: // PUSH D
        {
          state->memory[state->sp-1] = state->d;
          state->memory[state->sp-2] = state->e;
          state->sp = state->sp - 2;
        } break;
    case 0xd6: IncompleteInstruction(state); break;
    case 0xd7: IncompleteInstruction(state); break;
    case 0xd8: IncompleteInstruction(state); break;
    case 0xd9: IncompleteInstruction(state); break;
    case 0xda: IncompleteInstruction(state); break;
    case 0xdb: IncompleteInstruction(state); break;
    case 0xdc: IncompleteInstruction(state); break;
    case 0xdd: IncompleteInstruction(state); break;
    case 0xde: IncompleteInstruction(state); break;
    case 0xdf: IncompleteInstruction(state); break;
    case 0xe0: IncompleteInstruction(state); break;
    case 0xe1: // POP H
        {
          state->l = state->memory[state->sp];
          state->h = state->memory[state->sp+1];
          state->sp += 2;
        } break;
    case 0xe2: IncompleteInstruction(state); break;
    case 0xe3: IncompleteInstruction(state); break;
    case 0xe4: IncompleteInstruction(state); break;
    case 0xe5: // PUSH H
        {
          state->memory[state->sp-1] = state->h;
          state->memory[state->sp-2] = state->l;
          state->sp = state->sp - 2;
        } break;
    case 0xe6: // ANI D8
        {
          uint8_t x = state->a & opcode[1];
          state->cc.z = (x == 0);
          state->cc.s = ((x & 0x80) == 0x80);
          state->cc.p = Parity8b(x);
          state->cc.cy = 0;
          state->cc.ac = 0;
          state->a = x;
          state->pc++;
        } break;
    case 0xe7: IncompleteInstruction(state); break;
    case 0xe8: IncompleteInstruction(state); break;
    case 0xe9: IncompleteInstruction(state); break;
    case 0xea: IncompleteInstruction(state); break;
    case 0xeb: // XCHG
        {
          uint8_t rh_temp = state->h; // Temp for higher register
          uint8_t rl_temp = state->l; // Temp for lower register
          state->h = state->d; // Swap H for D
          state->d = rh_temp;
          state->l = state->e; // Swap L for E
          state->e = rl_temp;
        } break;
    case 0xec: IncompleteInstruction(state); break;
    case 0xed: IncompleteInstruction(state); break;
    case 0xee: IncompleteInstruction(state); break;
    case 0xef: IncompleteInstruction(state); break;
    case 0xf0: IncompleteInstruction(state); break;
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
    case 0xf2: IncompleteInstruction(state); break;
    case 0xf3: IncompleteInstruction(state); break;
    case 0xf4: IncompleteInstruction(state); break;
    case 0xf5: // PUSH PSW
        {
          state->memory[state->sp-1] = state->a;
          uint8_t psw = (state->cc.z | state->cc.s << 1 | state->cc.p << 2|
                         state->cc.cy << 3 | state->cc.ac << 4);
          state->memory[state->sp-2] = psw;
          state->sp = state->sp - 2;
        } break;
    case 0xf6: IncompleteInstruction(state); break;
    case 0xf7: IncompleteInstruction(state); break;
    case 0xf8: IncompleteInstruction(state); break;
    case 0xf9: IncompleteInstruction(state); break;
    case 0xfa: IncompleteInstruction(state); break;
    case 0xfb: // EI
        {
          state->int_enable = 1;
        } break;
    case 0xfc: IncompleteInstruction(state); break;
    case 0xfd: IncompleteInstruction(state); break;
    case 0xfe: // CPI D8
        {
          uint8_t x = state->a - opcode[1];
          state->cc.z = (x == 0);
          state->cc.s = ((x & 0x80) == 0x80);
          state->cc.p = Parity8b(x);
          state->cc.cy = (state->a < opcode[1]);
          state->pc++;
        } break;
    case 0xff: IncompleteInstruction(state); break;
  }

  // Print out condition flag content
  printf("C = %d\t"    "P = %d\t"   "S = %d\t"   "Z = %d\n",
         state->cc.cy, state->cc.p, state->cc.s, state->cc.z);
  // Print out register content
  printf("A : $%02x\t"
         "B : $%02x\t"
         "C : $%02x\t"
         "D : $%02x\t"
         "E : $%02x\t"
         "H : $%02x\t"
         "L : $%02x\t"
         "SP : $%04x\n",
         state->a,
         state->b,
         state->c,
         state->d,
         state->e,
         state->h,
         state->l,
         state->sp);

  return 0;
}
