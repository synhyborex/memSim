#include "memSim.h"

using namespace std;

void init(){
  initTLB();
  initPageTable();
  initPhysMem();
}

void initTLB(){
  for(int i = 0; i < TLB_SIZE; i++) {
    TLB.push_back(new TLBEntry(0,0));
  }
}

void initPageTable(){
  for(int i = 0; i < PAGE_TABLE_SIZE; i++) {
    pageTable.push_back(new PageTableEntry(0,0,0));
  }
}

void initPhysMem(){
  FILE *disk;
  if((disk = fopen(DISK,"r")) == NULL) {
    cout << "Could not read disk. Exiting program." << endl;
    exit(EXIT_FAILURE);
  }
  for(int i = 0; i < frames; i++) {
    physMem.push_back(new PhysMemFrame());
  }
  fclose(disk);
}

void addressOps(char* address_file) {
  int address;
  char page, offset;
  FILE* addrs = openAddrFile(address_file);
  while (!feof(addrs)) {
    fscanf(addrs,"%d",&address);
    page = (address & 0xFF00) >> BYTE_SIZE;
    offset = address & 0xFF;
    //printf("%d %d %d\n", address, page, offset);
  }

  fclose(addrs);
}

FILE* openAddrFile(char* address_file) {
  FILE *addrs;
  if (((addrs = fopen(address_file,"r")) == NULL)) {
    cout << "Could not open reference sequence file. Exiting program." << endl;
    exit(EXIT_FAILURE);
  }
  return addrs;
}

void cleanup(){
  cleanTLB();
  cleanPageTable();
  cleanPhysMem();
}

void cleanTLB(){
  for (int i = 0; i < TLB_SIZE; i++) {
    delete TLB[i];
  }
}

void cleanPageTable(){
  for (int i = 0; i < PAGE_TABLE_SIZE; i++) {
    delete pageTable[i];
  }
}

void cleanPhysMem(){
  for (int i = 0; i < frames; i++) {
    delete physMem[i];
  }
}

void parseCommandLine(int argc, char* argv[]) {
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
      if (!strcmp(argv[3],"fifo")) {
        pra = FIFO;
      }
      else if (!strcmp(argv[3],"lru")) {
        pra = LRU;
      }
      else if (!strcmp(argv[3],"opt")) {
        pra = OPT;
      }
      else {
        cout << "Invalid PRA type. Argument treated as not present." << endl;
        pra = FIFO;
      }
      break;
    default:
      cout << "usage: memSim <reference-sequence-file.txt> <FRAMES> <PRA>" << endl;
      exit(EXIT_FAILURE);
  }
}

int main(int argc, char** argv){
  parseCommandLine(argc, argv);
  init();
  addressOps(argv[1]);
  cleanup();
  return 0;
}
