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
      frame = (char*) malloc(PAGE_SIZE*sizeof(char));
    }
    ~Address(){
      free(frame);
    }

    unsigned int page;
    unsigned int offset;
    unsigned int address;
    int value;
    int frame_index;
    char* frame;
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
      priority = 0;
    }
    ~PhysMemFrame(){
      free(frame);
    }

    unsigned char* frame; //fixed size 256 bytes
    unsigned int priority;
};

extern void parseCommandLine(int argc, char* argv[]);
extern void init();
extern void initTLB();
extern void initPageTable();
extern void initPhysMem();
extern void clean();
extern FILE* openAddrFile(char* address_file);
extern void fillAddresses(char* address_file);
extern void lookupAddress();
extern bool isInTLB(Address* addr);
extern bool isInPageTable(Address* addr);
extern void pageFault(int index);
extern void updatePageTable(Address* addr);
extern void updateTLB(Address* addr);
extern TLBEntry* getTLBEntry();
extern PageTableEntry* getPageTableEntry();
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
