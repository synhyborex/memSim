#include "memSim.h"

using namespace std;

void init() {
  initTLB();
  initPageTable();
  initPhysMem();
  page_hits = 0;
  page_faults = 0;
  tlb_hits = 0;
  tlb_misses = 0;
}

void initTLB() {
  for (int i = 0; i < TLB_SIZE; i++) {
    TLB.push_back(new TLBEntry(0, 0));
  }
}

void initPageTable() {
  for (int i = 0; i < PAGE_TABLE_SIZE; i++) {
    pageTable.push_back(new PageTableEntry(0, 0, 0));
  }
}

void initPhysMem() {
  FILE *disk;
  if ((disk = fopen(DISK,"r")) == NULL) {
    cout << "Could not read disk. Exiting program." << endl;
    exit(EXIT_FAILURE);
  }

  unsigned char* nextFrame = (unsigned char*)malloc(PAGE_SIZE*sizeof(char));
  unsigned char nextByte;

  for (int i = 0; i < frames; i++) {
    for (int j = 0; j < PAGE_SIZE; j++) {
      fread(&nextByte, 1, 1, disk);
      nextFrame[j] = nextByte;
    }
    physMem.push_back(new PhysMemFrame(nextFrame));
  }
  free(nextFrame);
  fclose(disk);
}

void fillAddresses(char* address_file) {
  int address;
  unsigned int page, offset;
  FILE* addrs = openAddrFile(address_file);
  while (fscanf(addrs, "%d", &address) > 0 || !feof(addrs)) {
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

void lookupAddress() {
  for (unsigned int i = 0; i < addresses.size(); i++) {
    if (!isInTLB(addresses[i])) {
      if (!isInPageTable(addresses[i])) {
        pageFault(i);
      }
    }
  }
}

void pageFault(int index) {
  FILE *disk;
  Address* addr = addresses[index];
  if ((disk = fopen(DISK,"r")) != NULL) {
    char* page = (char*)malloc(PAGE_SIZE*sizeof(char));
    fseek(disk, addr->page*PAGE_SIZE, SEEK_SET);
    char nextByte;
    for (int a = 0; a < PAGE_SIZE; a++) {
      fread(&nextByte, 1, 1, disk);
      page[a] = nextByte;
    }
    memmove(addr->frame, page, PAGE_SIZE);
    addr->value = page[addr->offset];

    updatePageTable(addr);
    updateTLB(addr);
    fclose(disk);
  }
}

TLBEntry* getTLBEntry() {
  int i = 0;
  while (TLB[i]->log_page != 0) {
    i++;
  }
  return TLB[i];
}

void updateTLB(Address* addr) {
  TLBEntry* my_TLB = getTLBEntry();
  my_TLB->phys_frame = addr->frame_index;
  my_TLB->log_page = addr->page;
}

PageTableEntry* getPageTableEntry() {
  int i = 0;
  while (pageTable[i]->log_page != 0) {
    i++;
  }
  return pageTable[i];
}

void updatePageTable(Address* addr) {
  PageTableEntry* my_entry = getPageTableEntry();
  my_entry->phys_frame = addr->frame_index;
  my_entry->log_page = addr->page;
}

bool isInTLB(Address* addr) {
  for (unsigned int i = 0; i < TLB.size(); i++) {
    if (addr->page == TLB[i]->log_page) {
      addr->frame_index = TLB[i]->phys_frame;
      addr->value = *((physMem[TLB[i]->phys_frame]->frame)+addr->offset);
      memmove(addr->frame,physMem[TLB[i]->phys_frame]->frame,PAGE_SIZE);
      tlb_hits++;
      return true;
    }
  }
  tlb_misses++;
  return false;
}

bool isInPageTable(Address* addr) {
  for (unsigned int i = 0; i < pageTable.size(); i++) {
    if ((addr->page == pageTable[i]->log_page)
        && pageTable[i]->valid) {
      addr->frame_index = pageTable[i]->phys_frame;
      addr->value = *((physMem[pageTable[i]->phys_frame]->frame)+addr->offset);
      memmove(addr->frame,physMem[pageTable[i]->phys_frame]->frame,PAGE_SIZE);
      page_hits++;
      return true;
    }
  }
  page_faults++;
  return false;
}


void printResults() {
  page_fault_rate = page_faults / (page_faults + page_hits);
  tlb_miss_rate = tlb_misses / (tlb_hits + tlb_misses);
  unsigned int index = 0;
  while (index < addresses.size()) {
    Address* my_addr = addresses[index];
    //printf("%d %d %d\n", my_addr->address, my_addr->page,
    //  my_addr->offset);
    //printf("full address; value; phsymem frame number; content of entire frame;\n");
    printf("%d, %d, %d, ", my_addr->address, my_addr->value, my_addr->frame_index);
    //for (int i = 0; i < PAGE_SIZE; i++) {
    //  printf("%x", (int) (*(unsigned char*) my_addr->frame+i));
    //}
    printf("\n");
    index++;
  }
  printf("Page Faults: %d, Page Fault Rate: %f\n", page_faults, page_fault_rate);
  printf("TLB Hits: %d, TLB Misses: %d, TLB Miss Rate: %f\n", tlb_hits, tlb_misses, tlb_miss_rate);
}

void cleanup() {
  cleanTLB();
  cleanPageTable();
  cleanPhysMem();
}

void cleanTLB() {
  for (int i = 0; i < TLB_SIZE; i++) {
    delete TLB[i];
  }
}

void cleanPageTable() {
  for (int i = 0; i < PAGE_TABLE_SIZE; i++) {
    delete pageTable[i];
  }
}

void cleanPhysMem() {
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
      frames = strtol(argv[2], NULL, 10);
      pra = FIFO;
      break;
    case 4:
      frames = strtol(argv[2], NULL, 10);
      if (!strcmp(argv[3], "fifo")) {
        pra = FIFO;
      }
      else if (!strcmp(argv[3], "lru")) {
        pra = LRU;
      }
      else if (!strcmp(argv[3], "opt")) {
        pra = OPT;
      }
      else {
        cout << "Invalid PRA type. Argument treated as not present." << endl;
        pra = FIFO;
      }
      break;
    default:
      cout << "Usage: memSim <reference-sequence-file.txt> <FRAMES> <PRA>" << endl;
      exit(EXIT_FAILURE);
  }
}

int main(int argc, char** argv) {
  parseCommandLine(argc, argv);
  init();
  fillAddresses(argv[1]);
  lookupAddress();
  printResults();
  cleanup();
}
