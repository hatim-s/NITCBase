#include "Buffer/StaticBuffer.h"
#include "Cache/OpenRelTable.h"
#include "Disk_Class/Disk.h"
#include "FrontendInterface/FrontendInterface.h"

// local headers
#include <iostream>

void printBuffer (unsigned char buffer[], int size) {
  for (int i = 0; i < size; i++) {
    std::cout << (int) buffer[i] << " ";
    if (i % 64 == 63) std::cout << "\n";
  }
  std::cout << "\n";
}

int main(int argc, char *argv[]) {
  Disk disk_run;

  // unsigned char buffer[BLOCK_SIZE];
  // Disk::readBlock(buffer, 7000);
  // char message[] = "hello";
  // memcpy(buffer + 20, message, 6);
  // Disk::writeBlock(buffer, 7000);

  // unsigned char buffer2[BLOCK_SIZE];
  // char message2[6];
  // Disk::readBlock(buffer2, 7000);
  // memcpy(message2, buffer2 + 20, 6);
  // std::cout << message2;

  unsigned char buffer [BLOCK_SIZE];
  for (int blockNum = 0; blockNum <= 3; blockNum++) {
    Disk::readBlock(buffer, blockNum);
    printBuffer(buffer, BLOCK_SIZE);
  }

  return 0;
}