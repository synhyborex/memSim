#include "memSim.h"

using namespace std;

int main() {
  unsigned int address, mask = 0xFF;
  unsigned char pageNum, offset;
  const char* binFile = "./addresses.txt\0";
  FILE *infile;

  infile = fopen(binFile, "r");
  fscanf(infile, "%d", &address);

  pageNum = address >> BYTE_SIZE & mask;
  offset = address & mask;

  //cout << "page number: " << pageNum << "\noffset: " << offset << "\n";
  printf("page number:%d offset:%d\n", pageNum, offset);

  return 0;
}
