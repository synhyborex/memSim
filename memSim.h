#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <stdlib.h>
#include <cstring>
#include <cctype>

#define DISK "BACKING_STORE.bin"
#define BYTE_SIZE 8

//page replacement
#define FIFO 0
#define LRU 1
#define OPT 2

//sizes for tables
#define TLB_SIZE 16
#define PAGE_TABLE_SIZE 256
#define PAGE_SIZE 256

class Address {
  public:
    Address(int addr_num, int page_num, int offset_num) {
      address = (unsigned)addr_num;
      page = (unsigned)page_num;
      offset = (unsigned)offset_num;
      value = 0;
      frameNum = 0;
      frame = (unsigned char*)malloc(PAGE_SIZE*sizeof(char));
    }
    ~Address(){
      free(frame);
    }
    unsigned char page;
    unsigned char offset;
    unsigned int address;
    signed int value;
    unsigned int frameNum;
    unsigned char* frame;
  private:
};

class TLBEntry {
  public:
    TLBEntry(char logical, char phys){
      logicalPage = logical;
      physFrame = phys;
    }
    ~TLBEntry(){}

    unsigned char logicalPage;
    unsigned char physFrame;
};

class PageTableEntry {
  public:
    PageTableEntry(char val, char logical, char phys){
      valid = val;
      logicalPage = logical;
      physFrame = phys;
    }
    ~PageTableEntry(){}

    unsigned char valid;
    unsigned char logicalPage;
    unsigned char physFrame;
};

class PhysMemFrame {
  public:
    PhysMemFrame(unsigned char* fr){
      frame = (unsigned char*)malloc(PAGE_SIZE*sizeof(char));
      memmove(frame,fr,PAGE_SIZE);
    }
    ~PhysMemFrame(){
      free(frame);
    }

    unsigned char* frame; //fixed size 256 bytes
};

//functions
extern void parseCommandLine(int argc, char* argv[]);
extern void init();
extern void initTLB();
extern void initPageTable();
extern void initPhysMem();
extern void cleanup();
extern void cleanTLB();
extern void cleanPageTable();
extern void cleanPhysMem();
extern FILE* openAddrFile(char* address_file);
extern void addressOps(char* address_file);
extern void my_run();
extern bool checkTLB(Address* addr);
extern bool checkPageTable(Address* addr);
extern void pageFault(int index);
extern void updatePageTable(Address* addr);
extern void updateTLB(Address* addr);
extern TLBEntry* getTLBEntry();
extern PageTableEntry* getPageTableEntry();
extern void printResults();
extern bool checkTLB(Address*);
extern bool checkPageTable(Address*);

std::vector<TLBEntry*> TLB;
std::vector<PageTableEntry*> pageTable;
std::vector<PhysMemFrame*> physMem;
std::vector<Address*> addresses;
int frames; //the number of frames in physical memory
int pra; //the page replacement algorithm
int page_hits; //number of page table hits
int page_faults; // total number of page faults
float page_fault_rate; // percentage page fault rate
int tlb_hits; // total number of tlb hits
int tlb_misses; // total number of tlb misses
float tlb_miss_rate; // percentage tlb misses
