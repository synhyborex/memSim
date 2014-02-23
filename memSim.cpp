#include "memSim.h"

using namespace std;

void leastRecentlyUsed() {

}

void firstInFirstOut() {

}

void optimal() {
}

void init() {
  FILE *disk;

  // init physical memory
  if(!(disk = fopen(DISK,"r"))) {
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
    pageTable.push_back(new PageTableEntry(0,0));
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

  page_faults++;
  return rtn;
}

TLBEntry* getTLBEntry() {
  unsigned int i = 0;
  while (i < TLB.size() && TLB[i]->log_page != 0) {
    i++;
  }
  if (i == TLB.size()) {
    int removed = 0;
    unsigned int high_priority = 0;
    for (i = 0; i < TLB.size(); i++) {
      if(TLB[i]->priority > high_priority) {
        removed = i;
      }
    }
  }
  if (pra == LRU) {
    TLB[i]->priority = 0;
  }
  return TLB[i];
}

PageTableEntry* getPageTableEntry() {
  unsigned int i = 0;
  while (i < pageTable.size() && pageTable[i]->log_page != 0) {
    i++;
  }
  if (i == pageTable.size()) {
    int removed = 0;
    unsigned int high_priority = 0;
    for (i = 0; i < pageTable.size(); i++) {
      if(pageTable[i]->priority > high_priority) {
        removed = i;
      }
    }
  }
  if (pra == LRU) {
    pageTable[i]->priority = 0;
  }
  return pageTable[i];
}

void updateTLB(Address* addr) {
  TLBEntry* my_TLB = getTLBEntry();
  my_TLB->physFrame = addr->frameNum;
  my_TLB->logicalPage = addr->page;
}

void updatePageTable(Address* addr) {
  PageTableEntry* my_entry = getPageTableEntry();
  my_entry->physFrame = addr->frameNum;
  my_entry->logicalPage = addr->page;
}

void pageFaultHandler(int index) {
  FILE *disk;
  Address* addr = addresses[index];
  if ((disk = fopen(DISK,"r")) == NULL) {
    char* diskPage = (char*)malloc(PAGE_SIZE*sizeof(char));
    //go to page in disk
    fseek(disk,addr->page*PAGE_SIZE,SEEK_SET);
    //read in page
    char nextByte;
    for(int a = 0; a < PAGE_SIZE; a++){
      fread(&nextByte,1,1,disk);
      diskPage[a] = nextByte;
    }
    //set frame in address
    memmove(addr->frame,diskPage,PAGE_SIZE);
    //need to update TLB and page table
    updatePageTable(addr);
    updateTLB(addr);
    fclose(disk);
  }
}

void findAddresses(){
  for(unsigned int i = 0; i < addresses.size(); i++){
    if(checkTLB(addresses[i])) {
    }
    else if(checkPageTable(addresses[i])) {
    }
    else {
      pageFaultHandler(i);
    }
    for (unsigned int i = 0; i < TLB.size(); i++) {
      TLB[i]->priority++;
    }
    for (unsigned int i = 0; i < pageTable.size(); i++) {
      pageTable[i]->priority++;
    }
  }
}

void print() {
  for(int i = 0; i < addresses.size(); i++) {
    printf("%d, %d, %d\n", addresses[i]->address, addresses[i]->value,
      addresses[i]->frameNum);
  }
  printf("Page Faults: %d Page Fault Rate: %f\n", page_faults, page_fault_rate);
  printf("TLB Hits: %d TLB Misses: %d TLB Miss Rate: %f\n", tlb_hits, tlb_misses, tlb_miss_rate);
}

FILE* openAddressFile(char* addressName) {
  FILE *addressFile;
  if (!(addressFile = fopen(addressName,"r"))) {
    cout << "Invalid reference file." << endl;
    exit(EXIT_FAILURE);
  }
  return addressFile;
}

int main(int argc, char** argv) {
  unsigned int address;
  unsigned char page, offset;
  FILE* addressFile;

  switch(argc){
    case 2:
      frames = NUM_FRAMES;
      //pra = FIFO;
      pageReplacementAlgorithm = &firstInFirstOut;
      break;

    case 3:
      frames = strtol(argv[2], NULL, 10);
      //pra = FIFO;
      pageReplacementAlgorithm = &firstInFirstOut;
      break;

    case 4:
      frames = strtol(argv[2], NULL, 10);
      if(!strcmp(argv[3], "fifo")) {
        //pra = FIFO;
        pageReplacementAlgorithm = &firstInFirstOut;
      }
      else if(!strcmp(argv[3], "lru")) {
        //pra = LRU;
        pageReplacementAlgorithm = &leastRecentlyUsed;
      }
      else if(!strcmp(argv[3], "opt")) {
        //pra = OPT;
        pageReplacementAlgorithm = &optimal;
      }
      else{
        cout << "Invalid PRA." << endl;
        //pra = FIFO;
        pageReplacementAlgorithm = &firstInFirstOut;
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

  findAddresses();
  print();
  cleanup();

  return 0;
}
