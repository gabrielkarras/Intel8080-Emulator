#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>


#define FILE_NAME "invaders.h"


int Disassembler(uint8_t *codebuffer, int pc);


int main(int argc, char **argv){

  // Open file and verify status
  FILE *fp = fopen(FILE_NAME, "rb");
  if(fp == NULL){
    printf("Can't open %s\n", FILE_NAME);
    exit(EXIT_FAILURE);
  }

  // Get file size
  fseek(fp, 0L, SEEK_END);
  int fsize = ftell(fp);

  // Return to beginning of file to read into memory buffer
  fseek(fp, 0L, SEEK_SET);
  uint8_t *buffer = malloc(fsize);

  // Read file into buffer
  fread(buffer, fsize, 1, fp);
  fclose(fp);

  // Dissamble machine code until PC reaches end of code
  int pc = 0;
  while(pc < fsize){
    pc += Disassembler(buffer, pc);
  }

  return 0;
}


/*
  Reads machine code and dissables it to 8080 assembly code

  input:
  codebuffer points to the 8080 assembly codebuffer
  pc is the current program counter

  output:
  returns the number of bytes of the OpCode
*/
int Disassembler(uint8_t *codebuffer, int pc)
{
  uint8_t *code = &codebuffer[pc];
  int opbytes = 1;

  printf("%04x ", pc);
  switch(*code)
  {
    case 0x00: printf("NOP"); break;
    case 0x01: printf("LXI B,#$%02x%02x", code[2], code[1]);
               opbytes = 3;
               break;
    case 0x02: printf("STAX B"); break;
    case 0x03: printf("INX B"); break;
    case 0x04: printf("INR B"); break;
    case 0x05: printf("DCR B"); break;
    case 0x06: printf("MVI B,#$%02x", code[1]);
               opbytes = 2;
               break;
    case 0x07: printf("RLC"); break;
    case 0x08: break; // Free OpCode
    case 0x09: printf("DAD B"); break;
    case 0x0a: printf("LDAX B"); break;
    case 0x0b: printf("DCX B"); break;
    case 0x0c: printf("INR C"); break;
    case 0x0d: printf("DCR C"); break;
    case 0x0e: printf("MCI C,#$%02x", code[1]);
               opbytes = 2;
               break;
    case 0x0f: printf("RRC"); break;
    case 0x10: break; // Free OpCode
    case 0x11: printf("LXI D,#$%02x%02x", code[2], code[1]);
               opbytes = 3;
               break;
    case 0x12: printf("STAX D"); break;
    case 0x13: printf("INX D"); break;
    case 0x14: printf("INR D"); break;
    case 0x15: printf("DCR D"); break;
    case 0x16: printf("MVI D,#$%02x", code[1]);
               opbytes = 2;
               break;
    case 0x17: printf("RAL"); break;
    case 0x18: break; // Free OpCode
    case 0x19: printf("DAD D"); break;
    case 0x1a: printf("LDAX D"); break;
    case 0x1b: printf("DCX D"); break;
    case 0x1c: printf("INR E"); break;
    case 0x1d: printf("DCR E"); break;
    case 0x1e: printf("MVI E,#$%02x", code[1]);
               opbytes = 2;
               break;
    case 0x1f: printf("RAR"); break;
    case 0x20: break; // Free OpCode
    case 0x21: printf("LXI H,#$%02x%02x", code[2], code[1]);
               opbytes = 3;
               break;
    case 0x22: printf("SHLD $%02x%02x", code[2], code[1]);
               opbytes = 3;
               break;
    case 0x23: printf("INX H"); break;
    case 0x24: printf("INR H"); break;
    case 0x25: printf("DCR H"); break;
    case 0x26: printf("MVI H,#$%02x", code[1]);
               opbytes = 2;
               break;
    case 0x27: printf("DAA"); // Allows decimal arithmetic
    case 0x28: break; // Free OpCode
    case 0x29: printf("DAD H"); break;
    case 0x2a: printf("LHLD $%02x%02x", code[2], code[1]);
               opbytes = 3;
               break;

    default: break;
  }

  printf("\n");

  return opbytes;
}
