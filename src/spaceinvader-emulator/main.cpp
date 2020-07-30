#include <iostream>
#include "CPU_emulator.h"

/* Function Declaration */
void ReadIntoMemory(States *state, char *filename, uint32_t offset);


int main(int argc, char **argv)
{

  int EOI = 0; // End Of Instruction

  /* Allocate and initialize memory */
  States *state = static_cast<States*>( calloc(1, sizeof(States)) );
  /* Allocate for 16bits address/64Kbytes */
  state->memory = static_cast<uint8_t*>( malloc(0x10000) );

  /*
    Read Space Invader ROM
  */
  ReadIntoMemory(state, (char*)"invaders.h", 0x0);
  ReadIntoMemory(state, (char*)"invaders.g", 0x800);
  ReadIntoMemory(state, (char*)"invaders.f", 0x1000);
  ReadIntoMemory(state, (char*)"invaders.e", 0x1800);


  /*
    Loop until end of program
    Or until emulator reads an incomplete instruction
  */
  while (EOI == 0){
    EOI = Emulator(state);
  }

  return 0;
}


/* Helper Function */

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
