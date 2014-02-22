#include "memSim.h"

using namespace std;

void init(){
  FILE *disk;
  // init physical memory
  if((disk = fopen(DISK,"r")) == NULL){
    cout << "Could not read disk. Exiting program." << endl;
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

  //init tlb table
  for(int i = 0; i < TLB_SIZE; i++) {
    TLB.push_back(new TLBEntry(66,0));
  }

  //init page table
  for(int i = 0; i < PAGE_TABLE_SIZE; i++) {
    pageTable.push_back(new PageTableEntry(0,0,0));
  }

  page_hits = 0;
  page_faults = 0;
  tlb_hits = 0;
  tlb_misses = 0;
}

void addressOps(char* address_file) {
  int address;
  char page, offset;
  FILE* addrs = openAddrFile(address_file);
  while (!feof(addrs)) {
    fscanf(addrs,"%d",&address);
    page = (address & 0xFF00) >> BYTE_SIZE;
    offset = address & 0xFF;
    addresses.push_back(new Address(address, page, offset));
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

void printResults() {
  unsigned int index = 0;
  while (index < addresses.size()) {
    printf("%d, %d, %d, %s\n", addresses[index]->address, addresses[index]->value,
      addresses[index]->frameNum,addresses[index]->frame);
    //printf("full address; value; phsymem frame number; content of entire frame;\n");
    index++;
  }
  page_fault_rate = (float)(page_faults/(page_hits+page_faults));
  tlb_miss_rate = (float)(tlb_misses/(tlb_hits+tlb_misses));
  printf("Page Faults: %d Page Fault Rate: %f\n", page_faults, page_fault_rate);
  printf("TLB Hits: %d TLB Misses: %d TLB Miss Rate: %f\n", tlb_hits, tlb_misses, tlb_miss_rate);
}

void cleanup(){
  //clean physical memory
  for (int i = 0; i < frames; i++) {
    delete physMem[i];
  }

  //clean tlb
  for (int i = 0; i < TLB_SIZE; i++) {
    delete TLB[i];
  }

  //clean page table
  for (int i = 0; i < PAGE_TABLE_SIZE; i++) {
    delete pageTable[i];
  }
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

bool checkTLB(Address* addr){
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
      return true;
    }
  }

  //leaving for loop means it didn't find a match
  tlb_misses++;
  return false;
}

bool checkPageTable(Address* addr){
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
      return true;
    }
  }

  //leaving for loop means it didn't find a match
  page_faults++;
  return false;
}

int main(int argc, char** argv){
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
  runAddrs();
  printResults();
  cleanup();
  return 0;
}
