#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <stdlib.h>
#include <cstring>

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
    PhysMemFrame(unsigned char* fr){
      frame = (unsigned char*)malloc(PAGE_SIZE*sizeof(char));
      memmove(frame,fr,PAGE_SIZE);
    }
    ~PhysMemFrame(){
      free(frame);
    }

  private:
    unsigned char* frame; //fixed size 256 bytes
};

//functions
extern void init();
extern void initTLB();
extern void initPageTable();
extern void initPhysMem();
extern void cleanup();
extern void cleanTLB();
extern void cleanPageTable();
extern void cleanPhysMem();

std::vector<TLBEntry*> TLB;
std::vector<PageTableEntry*> pageTable;
std::vector<PhysMemFrame*> physMem;
int frames; //the number of frames in physical memory
int pra; //the page replacement algorithm