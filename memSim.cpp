#include "memSim.h"

using namespace std;

void init(){
  initTLB();
  initPageTable();
  initPhysMem();  
}

void initTLB(){
  for(int i = 0; i < TLB_SIZE; i++)
    TLB.push_back(new TLBEntry(0,0)); 
}

void initPageTable(){
  for(int i = 0; i < PAGE_TABLE_SIZE; i++)
    pageTable.push_back(new PageTableEntry(0,0,0));
}

void initPhysMem(){
  FILE *disk;
  if((disk = fopen(DISK,"r")) == NULL){
    cout << "Could not read disk. Exiting program." << endl;
    exit(EXIT_FAILURE);
  }
  for(int i = 0; i < frames; i++){
    physMem.push_back(new PhysMemFrame());
  }
  fclose(disk);
}

void cleanup(){
  cleanTLB();
  cleanPageTable();
  cleanPhysMem();
}

void cleanTLB(){
  for(int i = 0; i < TLB_SIZE; i++)
    delete TLB[i];
}

void cleanPageTable(){
  for(int i = 0; i < PAGE_TABLE_SIZE; i++)
    delete pageTable[i];
}

void cleanPhysMem(){
  for(int i = 0; i < frames; i++)
    delete physMem[i];
}

int main(int argc, char** argv){
  //command line parsing
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
  FILE *addrs; //address file
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

  cleanup();

  return 0;
}