#include "memSim.h"

using namespace std;


/* initialize TLB, page table, and physical memory.
 * also gets addresses from file that need to be read
 */
void init(char* file){
  getAddrsFromFile(file);
  initTLB();
  initPageTable();
  initPhysMem();

  page_hits = 0;
  page_faults = 0;
  tlb_hits = 0;
  tlb_misses = 0;
}

/* initialize TLB */
void initTLB(){
  for(int i = 0; i < TLB_SIZE; i++){
    TLB.push_back(new TLBEntry(0,0));
  }
}

/* initialize page table */
void initPageTable(){
  for(int i = 0; i < PAGE_TABLE_SIZE; i++){
    pageTable.push_back(new PageTableEntry(0,0,0));
  }
}

/* initialize physical memory */
void initPhysMem(){
  FILE *disk;
  if((disk = fopen(DISK,"r")) == NULL){
    cout << "Could not read disk. Exiting program." << endl;
    exit(EXIT_FAILURE);
  }

  //the next frame we're putting into memory
  unsigned char* nextFrame = (unsigned char*)malloc(PAGE_SIZE*sizeof(char));
  unsigned char nextByte; //the next byte we will read in

  for(int i = 0; i < frames; i++){
    for(int j = 0; j < PAGE_SIZE; j++){
      fread(&nextByte,1,1,disk);
      nextFrame[j] = nextByte;
    }
    physMem.push_back(new PhysMemFrame(nextFrame));
  }

  free(nextFrame);
  fclose(disk);
}

/* gets the list of addresses from the address file */
void getAddrsFromFile(char* address_file){
  unsigned int address;
  unsigned char page, offset;
  FILE* addrs = openAddrFile(address_file);
  while(fscanf(addrs, "%d", &address) && !feof(addrs)){
    page = (address & 0xFF00) >> BYTE_SIZE;
    offset = address & 0xFF;
    addresses.push_back(new Address(address, page, offset));
  }
  fclose(addrs);

  //how long until each address will be run
  if(pra == OPT){
    for(unsigned int i = 0; i < addresses.size(); i++){
      addresses[i]->timeTillUse = i;
    }
  }
}

/* opens the given address file */
FILE* openAddrFile(char* address_file){
  FILE *addrs;
  if(((addrs = fopen(address_file,"r")) == NULL)){
    cout << "Could not open reference sequence file. Exiting program." << endl;
    exit(EXIT_FAILURE);
  }
  return addrs;
}

/* deals with page faults */
void handlePageFault(Address* addr){
  FILE *disk;
  if((disk = fopen(DISK,"r")) == NULL){
    cout << "Could not read disk. Exiting program." << endl;
    exit(EXIT_FAILURE);
  }
  char* diskPage = (char*)malloc(PAGE_SIZE*sizeof(char));
  //go to page in disk
  fseek(disk,addr->page*PAGE_SIZE,SEEK_SET);
  char nextByte; //the next byte we'll read in
  //read in page
  for(int a = 0; a < PAGE_SIZE; a++){
    fread(&nextByte,1,1,disk);
    //set next byte of frame
    diskPage[a] = nextByte;
  }

  //set frame in address
  memmove(addr->frame,diskPage,PAGE_SIZE);
  addr->value = diskPage[addr->offset];

  //need to update TLB and page table
  int newFrame = updatePhysMem(addr);
  addr->frameNum = newFrame;
  updatePageTable(addr,newFrame);
  updateTLB(addr,newFrame);

  free(diskPage);
  fclose(disk);
}

/* updates physical memory with the page from disk */
int updatePhysMem(Address* addr){
  int highIdx = 0;
  //check replacement algorithm
  if(pra == OPT){
    for(unsigned int i = addresses.size()-1; i >= 0 && i < addresses.size(); i--){
      for(unsigned int j = 0; j < (unsigned)frames; j++){
        if(addresses[i]->page == pageTable[j]->logicalPage){
          if(addresses[i]->timeTillUse > (unsigned)frames/2)
            highIdx = j;
        }
      }
    }
  }
  else{
    int high = 0;
    for(unsigned int i = 0; i < physMem.size(); i++){
      //see if it's unused
      if(physMem[i]->age == -1){
        highIdx = i;
        break;
      }
      //if not unused, we have to keep going
      if(physMem[i]->age > high){
        high = physMem[i]->age;
        highIdx = i;
      }
    }
  }
  //replace page with one from disk
  memmove(physMem[highIdx]->frame,addr->frame,PAGE_SIZE);
  //reset age
  physMem[highIdx]->age = 0;

  return highIdx;
}

/* updates page table with the new entry in memory */
void updatePageTable(Address* addr, int physFrame){
  int highIdx = 0;
  //check replacement algorithm
  if(pra == OPT){
    for(unsigned int i = addresses.size()-1; i >= 0 && i < addresses.size(); i--){
      for(unsigned int j = 0; j < PAGE_TABLE_SIZE; j++){
        if(addresses[i]->page == pageTable[j]->logicalPage){
          if(addresses[i]->timeTillUse > PAGE_TABLE_SIZE/2)
            highIdx = j;
        }
      }
    }
  }
  else{
    //set invalid bit for all other references to that frame in phys mem
    for(unsigned int i = 0; i < pageTable.size(); i++){
      if(pageTable[i]->physFrame == (char)physFrame){
        pageTable[i]->valid = 0;
      }
    }

    //set new entry
    int high = 0;
    for(unsigned int i = 0; i < pageTable.size(); i++){
      //see if it's unused
      if(pageTable[i]->age == -1){
        highIdx = i;
        break;
      }
      //if not unused, we have to keep going
      if(pageTable[i]->age > high){
        high = pageTable[i]->age;
        highIdx = i;
      }
    }
  }
  //replace page with one from disk
  pageTable[highIdx]->logicalPage = addr->page;
  pageTable[highIdx]->physFrame = (char)physFrame;
  pageTable[highIdx]->valid = 1;
  //reset age
  pageTable[highIdx]->age = 0;
}

/* updates TLB with new entry from page table */
void updateTLB(Address* addr, int physFrame){
  int highIdx = 0;
  //check replacement algorithm
  if(pra == OPT){
    //go bottom to top in address list
    for(unsigned int i = addresses.size()-1; i >= 0 && i < addresses.size(); i--){
      //go top to bottom in TLB
      for(unsigned int j = 0; j < TLB_SIZE; j++){
        //if match, it means not used for a long time
        if(addresses[i]->page == TLB[j]->logicalPage){
          if(addresses[i]->timeTillUse > TLB_SIZE/2)
            highIdx = j;
        }
      }
    }
  }
  //FIFO and LRU
  else{
    int high = 0;
    //find index with highest age
    for(unsigned int i = 0; i < TLB.size(); i++){
      //see if it's unused
      if(TLB[i]->age == -1){
        highIdx = i;
        break;
      }
      //if not unused, we have to keep going
      if(TLB[i]->age > high){
        high = TLB[i]->age;
        highIdx = i;
      }
    }
  }
  //replace page with one from disk
  TLB[highIdx]->logicalPage = addr->page;
  TLB[highIdx]->physFrame = (char)physFrame;
  //reset age
  TLB[highIdx]->age = 0;
}

/* checks TLB for address */
bool checkTLB(Address* addr){
  for(unsigned int i = 0; i < TLB.size(); i++){
    if(addr->page == TLB[i]->logicalPage){
      addr->frameNum = TLB[i]->physFrame;
      addr->value = *((physMem[addr->frameNum]->frame)+addr->offset);
      memmove(addr->frame,physMem[addr->frameNum]->frame,PAGE_SIZE);
      if(pra == LRU){
        TLB[i]->age = 0;
        physMem[addr->frameNum]->age = 0;
      }
      tlb_hits++;
      return true;
    }
  }
  tlb_misses++;
  return false;
}

/* checks page table for address */
bool checkPageTable(Address* addr){
  for(unsigned int i = 0; i < pageTable.size(); i++){
    if((addr->page == pageTable[i]->logicalPage)
        && pageTable[i]->valid){
      addr->frameNum = pageTable[i]->physFrame;
      addr->value = *((physMem[addr->frameNum]->frame)+addr->offset);
      memmove(addr->frame,physMem[addr->frameNum]->frame,PAGE_SIZE);
      if(pra == LRU){
        pageTable[i]->age = 0;
        physMem[addr->frameNum]->age = 0;
      }
      page_hits++;
      return true;
    }
  }
  page_faults++;
  return false;
}

/* prints results for each address in the specified format */
void printResults(){
  for(unsigned int i = 0; i < addresses.size(); i++){
    Address* addr = addresses[i];
    printf("%d, %d, %d, ", addr->address, addr->value, addr->frameNum);
    for(int i = 0; i < PAGE_SIZE; i++){
      printf("%x",(int)(*(unsigned char*)(&addr->frame[i])));
    }
    printf("\n");
  }
  page_fault_rate = (page_faults/(page_hits+page_faults+tlb_hits));
  tlb_miss_rate = (tlb_misses/(tlb_hits+tlb_misses));
  printf("Page Faults: %.0f Page Fault Rate: %f\n", page_faults, page_fault_rate);
  printf("TLB Hits: %.0f TLB Misses: %.0f TLB Miss Rate: %f\n", tlb_hits, tlb_misses, tlb_miss_rate);
}

/* cleans up address list, TLB, page table, and physical memory */
void cleanup(){
  cleanAddrs();
  cleanTLB();
  cleanPageTable();
  cleanPhysMem();
}

/* cleans up address list */
void cleanAddrs(){
  for(unsigned int i = 0; i < addresses.size(); i++){
    delete addresses[i];
  }
}

/* cleans up TLB */
void cleanTLB(){
  for(int i = 0; i < TLB_SIZE; i++) {
    delete TLB[i];
  }
}

/* cleans up page table */
void cleanPageTable(){
  for(int i = 0; i < PAGE_TABLE_SIZE; i++) {
    delete pageTable[i];
  }
}

/* cleans up physical memory */
void cleanPhysMem(){
  for(int i = 0; i < frames; i++) {
    delete physMem[i];
  }
}

/* parses the command line input */
void parseCommandLine(int argc, char* argv[]){
  switch(argc){
    case 2:
      frames = 256;
      pra = FIFO;
      break;
    case 3:
      if(frames > 256 || frames < 0){
        cout << "Invalid number of frames. Using default value of 256." << endl;
        frames = 256;
      }
      else frames = strtol(argv[2],NULL,10);
      pra = FIFO;
      break;
    case 4:
      if(frames > 256 || frames < 0){
        cout << "Invalid number of frames. Using default value of 256." << endl;
        frames = 256;
      }
      else frames = strtol(argv[2],NULL,10);
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
        cout << "Invalid PRA type. Using default of FIFO." << endl;
        pra = FIFO;
      }
      break;
    default:
      cout << "usage: memSim <reference-sequence-file.txt> <FRAMES> <PRA>" << endl;
      exit(EXIT_FAILURE);
  }
}

/* ages values in the TLB */
void ageTLB(){
  for(int i = 0; i < TLB_SIZE; i++) {
    if(TLB[i]->age >= 0) //make sure it has been used
      TLB[i]->age++;
  }
}

/* ages values in the page table */
void agePageTable(){
  for(int i = 0; i < PAGE_TABLE_SIZE; i++) {
    if(pageTable[i]->age >= 0) //make sure it has been used
      pageTable[i]->age++;
  }
}

/* ages values in physical memory */
void agePhysMem(){
  for(int i = 0; i < frames; i++) {
    if(physMem[i]->age >= 0) //make sure it has been used
      physMem[i]->age++;
  }
}

/* runs list of addresses through TLB and page table */
void runAddrs(){
  //update ages
  ageTLB();
  agePageTable();
  agePhysMem();

  //check addrs
  for(unsigned int i = 0; i < addresses.size(); i++){
    if(pra == OPT){
      //everything is now one closer to use
      for(unsigned int j = 0; j < addresses.size(); j++){
        addresses[i]->timeTillUse--;
      }
    }

    //check TLB and page table
    if(!checkTLB(addresses[i])){
      if(!checkPageTable(addresses[i])){
        //page fault!
        handlePageFault(addresses[i]);
      }
      //if found in page table, will be taken care of inside checkPageTable()
    }
    //if found in TLB, will be taken care of inside checkTLB()
  }
}

int main(int argc, char** argv){
  parseCommandLine(argc, argv);
  init(argv[1]);
  runAddrs();
  printResults();
  cleanup();
  return 0;
}