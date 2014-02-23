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
      age = -1;
    }
    ~TLBEntry(){}

    unsigned char logicalPage;
    unsigned char physFrame;
    int age;
};

class PageTableEntry {
  public:
    PageTableEntry(char val, char logical, char phys){
      valid = val;
      logicalPage = logical;
      physFrame = phys;
      age = -1;
    }
    ~PageTableEntry(){}

    unsigned char valid;
    unsigned char logicalPage;
    unsigned char physFrame;
    int age;
};

class PhysMemFrame {
  public:
    PhysMemFrame(unsigned char* fr){
      frame = (unsigned char*)malloc(PAGE_SIZE*sizeof(char));
      memmove(frame,fr,PAGE_SIZE);
      age = -1;
    }
    ~PhysMemFrame(){
      free(frame);
    }

    unsigned char* frame; //fixed size 256 bytes
    int age;
};

//functions
extern void parseCommandLine(int,char**);
extern void init(char*);
extern void initTLB();
extern void initPageTable();
extern void initPhysMem();
extern FILE* openAddrFile(char*);
extern void getAddrsFromFile(char*);
extern void cleanup();
extern void cleanAddrs();
extern void cleanTLB();
extern void cleanPageTable();
extern void cleanPhysMem();
extern bool checkTLB(Address*);
extern bool checkPageTable(Address*);
extern void handlePageFault(Address*);
extern int updatePhysMem(Address*);
extern void updatePageTable(Address*,int);
extern void updateTLB(Address*,int);
extern void printResults();
extern bool checkTLB(Address*);
extern bool checkPageTable(Address*);
extern void ageTLB();
extern void agePageTable();
extern void agePhysMem();

std::vector<TLBEntry*> TLB;
std::vector<PageTableEntry*> pageTable;
std::vector<PhysMemFrame*> physMem;
std::vector<Address*> addresses;
int frames; //the number of frames in physical memory
int pra; //the page replacement algorithm
float page_hits; //number of page table hits
float page_faults; // total number of page faults
float page_fault_rate; // percentage page fault rate
float tlb_hits; // total number of tlb hits
float tlb_misses; // total number of tlb misses
float tlb_miss_rate; // percentage tlb misses
