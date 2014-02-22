#include "memSim.h"

using namespace std;

void init(){
  //init tlb
  for(int i = 0; i < TLB_SIZE; i++)
    TLB.push_back(new TLBEntry(0,0)); 

  //init page table
  for(int i = 0; i < PAGE_TABLE_SIZE; i++)
    pageTable.push_back(new PageTableEntry(0,0,0));

  //init physical memory
  for(int i = 0; i < frames; i++)
    physMem.push_back(new PhysMemFrame());
}

void cleanup(){
  //clean tlb
  for(int i = 0; i < TLB_SIZE; i++)
    delete TLB[i];

  //clean page table
  for(int i = 0; i < PAGE_TABLE_SIZE; i++)
    delete pageTable[i];

  //clean physical memory
  for(int i = 0; i < frames; i++)
    delete physMem[i];
}

int main(int argc, char** argv){
  int address, tlbMiss = 0;
  char page, offset;
  bool tlbFull = false;

  //command line parsing
  switch(argc){
    case 2:
      frames = NUM_FRAMES;
      pra = FIFO;
      break;
    case 3:
      frames = strtol(argv[2], NULL, 10);
      pra = FIFO;
      break;
    case 4:
      frames = strtol(argv[2], NULL, 10);
      if(!strcmp(argv[3], "fifo"))
        pra = FIFO;
      else if(!strcmp(argv[3], "lru"))
        pra = LRU;
      else if(!strcmp(argv[3], "opt"))
        pra = OPT;
      else{
        cout << "Invalid PRA. Algorithm defaulted to FIFO." << endl;
        pra = FIFO;
      }
      break;
    default:
      cout << "Invalid command line input." << endl;
      exit(EXIT_SUCCESS);
  }

  FILE *addressFile;
  if(((addressFile = fopen(argv[1], "r")) == NULL)){
    cout << "Could not open address file." << endl;
    exit(EXIT_SUCCESS);
  }

  init();

  fscanf(addressFile, "%d", &address);
  page = address >> BYTE_SIZE & MASK;
  offset = address & MASK;

  fclose(addressFile);
  cleanup();

  return 0;
}