// champsim/replacement/hybrid_policy.cc

#include "replacement_state.h"
#include <vector>
#include <map>

namespace
{
// Threshold for classifying a set as high-contention
constexpr uint32_t HIGH_CONTENTION_THRESHOLD = 500; 

// Ship++ related constants and structures
constexpr int SH_CTR_MAX = 7;
enum class PREFETCH_TYPE {
  TABLE,
  STREAM,
  NONE
};
struct SHCT_ENTRY {
  uint32_t pc = 0;
  uint32_t signature = 0;
  int counter = 1;
  PREFETCH_TYPE p_type = PREFETCH_TYPE::NONE;
};

// Data structures for our hybrid policy
std::map<CACHE*, std::vector<uint32_t>> eviction_counters;
std::map<CACHE*, std::vector<SHCT_ENTRY>> shct_tables; // For Ship++
std::map<CACHE*, std::vector<uint64_t>> rrpv; // For Ship++
} // namespace

void CACHE::initialize_replacement()
{
  // Initialize eviction counters for each set to zero
  ::eviction_counters[this] = std::vector<uint32_t>(this->NUM_SET, 0);

  // Initialize Ship++ data structures
  ::shct_tables[this] = std::vector<SHCT_ENTRY>(this->NUM_SET * this->NUM_WAY);
  ::rrpv[this] = std::vector<uint64_t>(this->NUM_SET * this->NUM_WAY, 0);
}

uint32_t CACHE::find_victim(uint32_t triggering_cpu, uint64_t instr_id, uint32_t set, const BLOCK* current_set, uint64_t ip, uint64_t full_addr, uint32_t type)
{
  auto& set_eviction_count = ::eviction_counters[this][set];

  // Increment eviction counter for this set upon a miss that requires a victim
  set_eviction_count++;

  // Check if the set is high-contention
  if (set_eviction_count > HIGH_CONTENTION_THRESHOLD) {
    // HIGH-CONTENTION SET: Use Ship++ policy
    // Find a victim whose RRPV is 3 (max value)
    for (uint32_t i = 0; i < this->NUM_WAY; ++i) {
      if (::rrpv[this][set * this->NUM_WAY + i] == 3) {
        return i;
      }
    }
    // If no such block is found, increment all RRPVs and try again
    for (uint32_t i = 0; i < this->NUM_WAY; ++i) {
      ::rrpv[this][set * this->NUM_WAY + i]++;
    }
    // Retry finding a victim with RRPV of 3
    for (uint32_t i = 0; i < this->NUM_WAY; ++i) {
      if (::rrpv[this][set * this->NUM_WAY + i] == 3) {
        return i;
      }
    }
    // Should not happen, but as a fallback
    return 0;

  } else {
    // LOW-CONTENTION SET: Use standard LRU policy
    uint32_t lru_victim = 0;
    uint64_t min_lru = current_set[0].lru;
    for (uint32_t i = 1; i < this->NUM_WAY; ++i) {
      if (current_set[i].lru < min_lru) {
        min_lru = current_set[i].lru;
        lru_victim = i;
      }
    }
    return lru_victim;
  }
}

void CACHE::update_replacement_state(uint32_t triggering_cpu, uint32_t set, uint32_t way, uint64_t full_addr, uint64_t ip, uint64_t victim_addr, uint32_t type,
                                     uint8_t hit)
{
  auto& set_eviction_count = ::eviction_counters[this][set];

  // Update LRU state for all blocks in the set (used by LRU part of the policy)
  for (uint32_t i = 0; i < this->NUM_WAY; i++) {
    if (this->block[set][i].lru > this->block[set][way].lru) {
      this->block[set][i].lru--;
    }
  }
  this->block[set][way].lru = this->NUM_WAY - 1;

  // If the set is high-contention, update Ship++ state
  if (set_eviction_count > HIGH_CONTENTION_THRESHOLD) {
      if (hit) {
          // On a hit, set RRPV to 0 (promote)
          ::rrpv[this][set * this->NUM_WAY + way] = 0;
      } else {
          // On a fill, set RRPV to 2
          ::rrpv[this][set * this->NUM_WAY + way] = 2;
      }
  }
}

void CACHE::replacement_final_stats() {}