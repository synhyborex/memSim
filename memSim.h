#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <stdlib.h>
#include <string.h>

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

class TLBEntry {
  public:
    unsigned char logicalPage;
    unsigned char physFrame;

    TLBEntry(char logical, char phys){
      logicalPage = logical;
      physFrame = phys;
    }
    ~TLBEntry(){}

  private:
};

class PageTableEntry {
  public:
    bool valid;
    unsigned char logicalPage;
    unsigned char physFrame;

    PageTableEntry(char logical, char phys){
      valid = false;
      logicalPage = logical;
      physFrame = phys;
    }
    ~PageTableEntry(){}
  private:
};

class PhysMemFrame {
  public:
    unsigned char* frame;
    
    PhysMemFrame(){
      frame = (unsigned char*)malloc(PAGE_SIZE*sizeof(char));
    }
    ~PhysMemFrame(){
      free(frame);
    }
  private:
};

//functions
extern void init();
extern void cleanup();

std::vector<TLBEntry*> TLB;
std::vector<PageTableEntry*> pageTable;
std::vector<PhysMemFrame*> physMem;
int frames; //the number of frames in physical memory
int pra; //the page replacement algorithm