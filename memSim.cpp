#include "memSim.h"

using namespace std;

FILE* openAddressFile(char* addressName) {
  FILE *addressFile;
  if (!addressFile = fopen(addressName,"r")) {
    cout << "Invalid reference file." << endl;
    exit(EXIT_FAILURE);
  }
  return addressFile;
}

void init() {
  FILE *disk;

  // init physical memory
  if(!disk = fopen(DISK,"r")) {
    cout << "Invalid disk location." << endl;
    exit(EXIT_FAILURE);
  }

  unsigned char* nextFrame = (unsigned char*)malloc(PAGE_SIZE*sizeof(char));
  unsigned char nextByte;

  for(int i = 0; i < frames; i++){
    for(int j = 0; j < PAGE_SIZE; j++){
      fread(&nextByte,1,1,disk);
      nextFrame[j] = nextByte;
    }
    physMem.push_back(new PhysMemFrame(nextFrame));
  }
  free(nextFrame);
  fclose(disk);

  //init tlb
  for(int i = 0; i < TLB_SIZE; i++)
    TLB.push_back(new TLBEntry(0,0)); 

  //init page table
  for(int i = 0; i < PAGE_TABLE_SIZE; i++)
    pageTable.push_back(new PageTableEntry(0,0,0));
}

void cleanup() {
  //clean physical memory
  for(int i = 0; i < frames; i++)
    delete physMem[i];

  //clean tlb
  for(int i = 0; i < TLB_SIZE; i++)
    delete TLB[i];

  //clean page table
  for(int i = 0; i < PAGE_TABLE_SIZE; i++)
    delete pageTable[i];
}

void runAddrs(){
  for(unsigned int i = 0; i < addresses.size(); i++){
    if(!checkTLB(addresses[i])){
      if(!checkPageTable(addresses[i])){
        //page fault!
        //need to go to disk
      }
      //if found in page table, will be taken care of inside checkPageTable()
    }
    //if found in TLB, will be taken care of inside checkTLB()
  }
}

bool checkTLB(Address* addr) {
  bool rtn = false;

  for(unsigned int i = 0; i < TLB.size(); i++){
    if(addr->page == TLB[i]->logicalPage){
      //TLB hit
      //frame number in physical memory
      addr->frameNum = TLB[i]->physFrame;
      //get value at the offset in physical memory
      addr->value = *((physMem[TLB[i]->physFrame]->frame)+addr->offset);
      //copy the frame over
      memmove(addr->frame,physMem[TLB[i]->physFrame]->frame,PAGE_SIZE);
      tlb_hits++;
      rtn = true;
    }
  }

  //leaving for loop means it didn't find a match
  tlb_misses++;
  return rtn;
}

bool checkPageTable(Address* addr) {
  bool rtn = false;

  for(unsigned int i = 0; i < pageTable.size(); i++){
    if((addr->page == pageTable[i]->logicalPage)
        && pageTable[i]->valid){
      //page table hit
      //frame number in physical memory
      addr->frameNum = pageTable[i]->physFrame;
      //get value at the offset in physical memory
      addr->value = *((physMem[pageTable[i]->physFrame]->frame)+addr->offset);
      //copy the frame over
      memmove(addr->frame,physMem[pageTable[i]->physFrame]->frame,PAGE_SIZE);
      page_hits++;
      rtn = true;
    }
  }

  //leaving for loop means it didn't find a match
  page_faults++;
  return rtn;
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
  unsigned int address;
  unsigned char page, offset;
  FILE* addressFile;

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
  addressFile = openAddressFile(argv[1]);

  while (!feof(addressFile)) {
    fscanf(addressFile, "%d", &address);
    page = address >> BYTE_SIZE & 0xFF;
    offset = address & 0xFF;
    addresses.push_back(new Address(address, page, offset));
  }
  fclose(addressFile);

  print();
  cleanup();

  return 0;
}
