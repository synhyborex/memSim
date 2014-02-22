#include "memSim.h"

using namespace std;

FILE* openAddressFile(char* addressName) {
  FILE *addressFile;
  if (((addressFile = fopen(addressName,"r")) == NULL)) {
    cout << "Could not open reference sequence file. Exiting program." << endl;
    exit(EXIT_FAILURE);
  }
  return addressFile;
}

void init() {
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

void cleanup() {
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

void addressOps(char* addressName) {
  unsigned int address;
  unsigned char page, offset;
  FILE* addressFile;

  addressFile = openAddressFile(addressName);
  while (!feof(addressFile)) {
    fscanf(addressFile, "%d", &address);
    page = address >> BYTE_SIZE & 0xFF;
    offset = address & 0xFF;
    addresses.push_back(new Address(address, page, offset));
  }
  fclose(addressFile);
}

void print() {
  for(int i = 0; i < addresses.size(); i++) {
    printf("%d %d %d\n", addresses[i]->address, addresses[i]->page,
      addresses[i]->offset);
  }
  printf("Page Faults: %d Page Fault Rate: %f\n", page_faults, page_fault_rate);
  printf("TLB Hits: %d TLB Misses: %d TLB Miss Rate: %f\n", tlb_hits, tlb_misses, tlb_miss_rate);
}

int main(int argc, char** argv) {
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
  init();
  addressOps(argv[1]);
  print();
  cleanup();
  return 0;
}
