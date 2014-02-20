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

class PhysMemPage {
  public:
  private:
    unsigned int page; //need to fix to take 256 bytes
};

//functions
extern void init();

std::vector<TLBEntry*> TLB;
std::vector<PageTableEntry*> pageTable;