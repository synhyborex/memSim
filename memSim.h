#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <stdlib.h>
#include <cstring>

#define DISK "BACKING_STORE.bin"
#define BYTE_SIZE 8

//page replacement algorithm
#define FIFO 0
#define LRU 1
#define OPT 2

//sizes
#define TLB_SIZE 16
#define PAGE_TABLE_SIZE 256
#define PAGE_SIZE 256

class Address {
  public:
    Address(int addr_num, unsigned int page_num, unsigned int offset_num) {
      address = addr_num;
      page = page_num;
      offset = offset_num;
      value = 0;
      frame_index = 0;
      frame = (unsigned char*) malloc(PAGE_SIZE*sizeof(char));
    }
    ~Address(){
      free(frame);
    }

    unsigned int page;
    unsigned int offset;
    unsigned int address;
    int value;
    int frame_index;
    unsigned char* frame;
};

class TLBEntry {
  public:
    TLBEntry(char logical, char phys){
      log_page = logical;
      phys_frame = phys;
      priority = 0;
    }
    ~TLBEntry(){}

    unsigned char log_page;
    unsigned char phys_frame;
    unsigned int priority;
};

class PageTableEntry {
  public:
    PageTableEntry(char val, char logical, char phys){
      valid = val;
      log_page = logical;
      phys_frame = phys;
      priority = 0;
    }
    ~PageTableEntry(){}

    unsigned char valid;
    unsigned char log_page;
    unsigned char phys_frame;
    unsigned int priority;
};

class PhysMemFrame {
  public:
    PhysMemFrame(unsigned char* fr){
      frame = (unsigned char*)malloc(PAGE_SIZE*sizeof(char));
      memmove(frame,fr,PAGE_SIZE);
      priority = -1;
    }
    ~PhysMemFrame(){
      free(frame);
    }

    unsigned char* frame; //fixed size 256 bytes
    int priority;
};

extern void parseCommandLine(int argc, char* argv[]); // read command line args
extern void init(); // allocate memory
extern void initPhysMem(); // allocate physical memory
extern void clean(); // free memory
extern FILE* openAddrFile(char* address_file); // open address file
extern void fillAddresses(char* address_file); // parse address file
extern void lookupAddress();  // check for address in existing entry
extern bool isInTLB(Address* addr);  // check for address in existing entry
extern bool isInPageTable(Address* addr); // check for address in existing entry
extern void pageFault(int index); // handle page faults
extern void updatePageTable(Address* addr); // insert address into page table
extern void updateTLB(Address* addr); // insert address into tlb
extern void updatePhysMem(Address* addr); // insert address into phys mem
extern int getPhysMemFrame(); // returns index of the phys mem frame
extern TLBEntry* getTLBEntry(); // use pru to get next entry
extern PageTableEntry* getPageTableEntry(); // use pru to get next entry
extern void printAddress(Address* my_addr);
extern void printResults();

std::vector<TLBEntry*> TLB;
std::vector<PageTableEntry*> pageTable;
std::vector<PhysMemFrame*> physMem;
std::vector<Address*> addresses;
int frames; //the number of frames in physical memory
int pra; //the page replacement algorithm
int page_faults; // total number of page faults
int page_hits; // total number of page hits
float page_fault_rate; // percentage page fault rate
int tlb_hits; // total number of tlb hits
int tlb_misses; // total number of tlb misses
float tlb_miss_rate; // percentage tlb misses
