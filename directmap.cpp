#include <iostream>
#include <fstream>
#include <vector>
#include <tuple>
#include <cmath>
#include <iomanip>

//max ross

using namespace std;

struct CacheEntry {
  int tag;
  int valid;
};

int main(int argc, char * argv[]) {

  // declare sizes and trace
   int cache_size = 512;
   int block_size = 32;
   string trace_file = "gcc.trace";

  // calculate the number of blocks, offset bits and index bits
   int num_blocks = cache_size / block_size;
   int offset_bits = log2(block_size);
   int index_bits = log2(num_blocks);

  // create the cache
  vector < CacheEntry > cache(num_blocks);

  // open the trace file
  ifstream trace(trace_file);
  if (!trace) {
    cerr << "Failed to open trace file" << endl;
    return 1;
  }

  // initialize counters for hits and misses
  int hits = 0;
  int misses = 0;

  // process each memory access in the trace file
  char op;
  unsigned int addr;
  int size;
  while (trace >> op >> hex >> addr >> size) {
    // calculate the offset, index and tag
     int offset = addr & ((1 << offset_bits) - 1);
     int index = (addr >> offset_bits) & ((1 << index_bits) - 1);
     int tag = addr >> (offset_bits + index_bits);

    // check if the cache entry is valid and has the same tag as the current address
    CacheEntry & entry = cache[index];
    if (entry.valid && entry.tag == tag) {
      ++hits;
    } else {
      // update the cache entry and count the miss
      ++misses;
      entry.valid = 1;
      entry.tag = tag;
    }
  }

  // print the hit rate
  cout << fixed << setprecision(8);
  cout << "Hit rate: " << static_cast < double > (hits) / (hits + misses) << endl;

  return 0;
}