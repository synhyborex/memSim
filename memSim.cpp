#include "memSim.h"

int main() {
  unsigned int address, mask = 0xFF;
  unsigned char pageNum, offset;
  FILE *store;
  char* binFile = "./addresses.txt\0";
  store = fopen(binFile);

  fscanf("%c", &address);
  pageNum = address >> BYTE_SIZE & mask;
  offset = address & mask;

  cout << "page number: " << pageNum << "\noffset: " << offset << "\n";

  return 0;
}
