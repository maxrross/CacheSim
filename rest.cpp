#include <iostream>
#include <fstream>
#include <vector>
#include <tuple>
#include <cmath>
#include <iomanip>
#include <unordered_map>
#include <queue>

// max ross

using namespace std;

struct cache_entry {
  int tag;
  bool valid;
  int counter;
  int fifo_time;
};

int main(int argc, char * argv[]) {

  // declare vars for the cache configuration
   int cache_size = 512;
   int block_size = 32;
   string associativity_str = "8";
   string replacement_policy_str = "FIFO";
   string trace_file = "gcc.trace";

  // calculating associativity
  int associativity;
  if (associativity_str == "fully") {
    associativity = cache_size / block_size;
  } else {
    associativity = stoi(associativity_str);
  }

  // calculate the number of blocks, offset bits, index bits, and tag bits
   int num_blocks = cache_size / block_size;
   int offset_bits = log2(block_size);
   int index_bits = log2(num_blocks / associativity);
   int tag_bits = 32 - offset_bits - index_bits;

  // create the cache
  vector < vector < cache_entry > > cache(num_blocks / associativity, vector < cache_entry > (associativity));
  // make a map to keep track of the FIFO times for each tag
  unordered_map < int, int > fifo_times;
  // counter for LRU replacement policy
  int counter = 0;

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

    // get the set of cache entries for the current index
    vector < cache_entry > & set = cache[index];
    // keep track of whether or not we had a hit
    bool hit = false;

    // if associativity is 1, then we can just check the first entry in the set
    if (associativity == 1) {
      cache_entry & entry = set[0];
      // if the cache block is valid and has the same tag as the current address, it's a hit
      if (entry.valid && entry.tag == tag) {
        hit = true;
        // updating cache data based on replacement policy
        if (replacement_policy_str == "LRU") {
          entry.counter = counter++;
        } else if (replacement_policy_str == "FIFO") {
          entry.fifo_time = fifo_times[tag];
        }
      } else {
        // if it's a miss, replace the cache block with the new tag and mark it as valid
        entry.valid = true;
        entry.tag = tag;
        if (replacement_policy_str == "LRU") {
          entry.counter = counter++;
        } else if (replacement_policy_str == "FIFO") {
          entry.fifo_time = fifo_times[tag] = misses;
        }
      }
    } else {
      // if associativity is greater than 1, then we need to search the entire set
      for (cache_entry & entry: set) {
        // if the cache block is valid and has the same tag as the current address, it's a hit
        if (entry.valid && entry.tag == tag) {
          hit = true;
          if (replacement_policy_str == "LRU") {
            entry.counter = counter++;
          } else if (replacement_policy_str == "FIFO") {
            entry.fifo_time = fifo_times[tag];
          }
          break;
        }
      }

      // if it's a miss find victim cache block based on the replacement policy
      if (!hit) {
        int min_counter = INT_MAX;
        int min_index = -1;
        int min_fifo_time = INT_MAX;
        // search the entire set for a victim cache block
        for (int i = 0; i < associativity; ++i) {
          cache_entry & entry = set[i];
          if (!entry.valid) {
            min_index = i;
            break;
          }
          // update the victim cache block based on the replacement policy
          if (replacement_policy_str == "LRU") {
            if (entry.counter < min_counter) {
              min_counter = entry.counter;
              min_index = i;
            }
          } else if (replacement_policy_str == "FIFO") {
            if (entry.fifo_time < min_fifo_time) {
              min_fifo_time = entry.fifo_time;
              min_index = i;
            }
          }
        }

        // replace the victim cache block with the new tag and mark it as valid
        cache_entry & entry = set[min_index];
        entry.valid = true;
        entry.tag = tag;
        if (replacement_policy_str == "LRU") {
          entry.counter = counter++;
        } else if (replacement_policy_str == "FIFO") {
          entry.fifo_time = fifo_times[tag] = misses;
        }
        ++misses;
      } else {
        ++hits;
      }
    }
  }

  cout << fixed << setprecision(8);
  cout << "Hit rate: " << static_cast < double > (hits) / (hits + misses) << endl;

  return 0;
}