#include "memSim.h"

using namespace std;

void init(){
  initTLB();
  initPageTable();
  initPhysMem();

  page_hits = 0;
  page_faults = 0;
  tlb_hits = 0;
  tlb_misses = 0;
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
  //try to open disk for reading
  FILE *disk;
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
      //cout << nextFrame[j];
    }
    physMem.push_back(new PhysMemFrame(nextFrame));
  }
  free(nextFrame);
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
    printf("%d, %d, %d, ", addresses[index]->address, addresses[index]->value,
      addresses[index]->frameNum);
    //need to print frame contents
    /*for(int i = 0; i < PAGE_SIZE; i++){
      printf("%c",addresses[index]->frame[i]);
    }*/
    printf("\n");
    //printf("full address; value; phsymem frame number; content of entire frame;\n");
    index++;
  }
  page_fault_rate = (float)(page_faults/(page_hits+page_faults));
  tlb_miss_rate = (float)(tlb_misses/(tlb_hits+tlb_misses));
  printf("Page Faults: %d Page Fault Rate: %f\n", page_faults, page_fault_rate);
  printf("TLB Hits: %d TLB Misses: %d TLB Miss Rate: %f\n", tlb_hits, tlb_misses, tlb_miss_rate);
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

void runAddrs(){
  for(unsigned int i = 0; i < addresses.size(); i++){
    if(!checkTLB(addresses[i])){
      if(!checkPageTable(addresses[i])){
        //page fault!
        //need to go to disk
        FILE *disk;
        if((disk = fopen(DISK,"r")) == NULL){
          unsigned char* diskPage = (unsigned char*)malloc(PAGE_SIZE*sizeof(char));
          //go to page in disk
          fseek(disk,addresses[i]->page*PAGE_SIZE,SEEK_SET);
          //read in page
          char nextByte;
          for(int a = 0; a < PAGE_SIZE; a++){
            fread(&nextByte,1,1,disk);
            //cout << nextByte;
            diskPage[a] = nextByte;
            //cout << diskPage[a];
          }
          //for(int a = 0; a <)
          //cout << diskPage << endl;
          //set frame in address
          memmove(addresses[i]->frame,diskPage,PAGE_SIZE);

          //need to update TLB and page table

          fclose(disk);
        }
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
  parseCommandLine(argc, argv);
  init();
  addressOps(argv[1]);
  runAddrs();
  printResults();
  cleanup();
  return 0;
}
