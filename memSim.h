#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <stdlib.h>
#include <string.h>

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
      address = addr_num;
      page = page_num;
      offset = offset_num;
    }
    ~Address(){}
    int page;
    int offset;
    int address;
  private:
};

class TLBEntry {
  public:
    TLBEntry(char logical, char phys){
      logicalPage = logical;
      physFrame = phys;
    }
    ~TLBEntry(){}

  private:
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
  private:
    unsigned char valid;
    unsigned char logicalPage;
    unsigned char physFrame;

};

class PhysMemFrame {
  public:
    PhysMemFrame(){
      frame = (unsigned char*)malloc(PAGE_SIZE*sizeof(char));
    }
    ~PhysMemFrame(){
      free(frame);
    }
  private:
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
extern void printResults();

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
