#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <stdlib.h>
#include <cstring>

#define DISK "BACKING_STORE.bin"
#define BYTE_SIZE 8
#define MASK 0xFF

//page replacement
#define FIFO 0
#define LRU 1
#define OPT 2

//sizes for tables
#define TLB_SIZE 16
#define PAGE_TABLE_SIZE 256
#define PAGE_SIZE 256
#define NUM_FRAMES 256

class Address {
  public:
    int page;
    int offset;
    int address;

    Address(int addr_num, int page_num, int offset_num) {
      address = addr_num;
      page = page_num;
      offset = offset_num;
    }
    
    ~Address() {}

  private:
};

class TLBEntry {
  public:
    unsigned char logicalPage;
    unsigned char physFrame;

    TLBEntry(char logical, char phys) {
      logicalPage = logical;
      physFrame = phys;
    }

    ~TLBEntry() {}

  private:
};

class PageTableEntry {
  public:
    bool valid;
    unsigned char logicalPage;
    unsigned char physFrame;

    PageTableEntry(char logical, char phys) {
      valid = false;
      logicalPage = logical;
      physFrame = phys;
    }

    ~PageTableEntry() {}

  private:
};

class PhysMemFrame {
  public:
    unsigned char* frame;

    PhysMemFrame(unsigned char* fr) {
      frame = (unsigned char*)malloc(PAGE_SIZE*sizeof(char));
      memmove(frame,fr,PAGE_SIZE);
    }

    ~PhysMemFrame() {
      free(frame);
    }

  private:
};

//functions
extern void init();
extern void cleanup();
extern FILE* openAddressFile(char* address_file);
extern void print();
extern void runAddrs();
extern bool checkTLB(Address*);
extern bool checkPageTable(Address*);

std::vector<TLBEntry*> TLB;
std::vector<PageTableEntry*> pageTable;
std::vector<PhysMemFrame*> physMem;
std::vector<Address*> addresses;
int frames; //the number of frames in physical memory
int pra; //the page replacement algorithm
int page_faults; // total number of page faults
float page_fault_rate; // percentage page fault rate
int tlb_hits; // total number of tlb hits
int tlb_misses; // total number of tlb misses
float tlb_miss_rate; // percentage tlb misses
