#include "memSim.h"

using namespace std;

void init(){
  //initialize TLB
  for(int i = 0; i < TLB_SIZE; i++)
    TLB.push_back(new TLBEntry(0,0));

  //initialize page table
  for(int i = 0; i < PAGE_TABLE_SIZE; i++)
    pageTable.push_back(new PageTableEntry(0,0,0));
}

int main(int argc, char** argv){
  //command line parsing
  FILE *addrs; //address file
  int frames; //the number of frames in physical memory
  int pra; //the page replacement algorithm

  //no FRAMES or PRA
  switch(argc){
    case 2:
      frames = 256;
      pra = FIFO;
      break;
    case 3:
      frames = strtol(argv[2],NULL,10);
      pra = FIFO;
      break;
    case 4:
      frames = strtol(argv[2],NULL,10);
      if(!strcmp(argv[3],"fifo"))
        pra = FIFO;
      else if(!strcmp(argv[3],"lru"))
        pra = LRU;
      else if(!strcmp(argv[3],"opt"))
        pra = OPT;
      else{
        cout << "Invalid PRA type. Argument treated as not present." << endl;
        pra = FIFO;
      }
      break;
    default:
      cout << "usage: memSim <reference-sequence-file.txt> <FRAMES> <PRA>" << endl;
      exit(EXIT_FAILURE);
  }

  //try to open addresses file
  if(((addrs = fopen(argv[1],"r")) == NULL)){
    cout << "Could not open reference sequence file. Exiting program." << endl;
    exit(EXIT_FAILURE);
  }

  //initialize data structures
  init();

  //get address to look for
  int address;
  char page, offset;
  //will probably need in loop 
  fscanf(addrs,"%d",&address);
  page = (address & 0xFF00) >> BYTE_SIZE; //get page number
  offset = address & 0xFF; //get offset


  fclose(addrs);

  //addrs = fopen(ADDRESSES, "r");
  //fscanf(addrs, "%d", &address);

  //cout << "page number: " << pageNum << "\noffset: " << offset << "\n";
  //printf("page number:%d offset:%d\n", pageNum, offset);

  return 0;
}