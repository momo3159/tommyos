#include <cstdint>
#include <array>
#include "paging.hpp"
#include "asmfunc.hpp"

namespace {
  const uint64_t kPageSize4K = 4096;
  const uint64_t kPageSize2M = 512 * kPageSize4K;
  const uint64_t kPageSize1G = 512 * kPageSize2M;

  alignas(kPageSize4K) std::array<uint64_t, 512> pml4_table;
  alignas(kPageSize4K) std::array<uint64_t, 512> pdp_table;
  alignas(kPageSize4K) std::array<std::array<uint64_t, 512>, kPageDirectoryCount> page_directory;
}

void SetupIdentityPageTable() {
  pml4_table[0] = reinterpret_cast<uint64_t>(&pdp_table[0]) | 0x003;
  for (int i_pdp=0;i_pdp<page_directory.size();i_pdp++) {
    pdp_table[i_pdp] = reinterpret_cast<uint64_t>(&page_directory[i_pdp]) | 0x003;
    for (int i_pd=0;i_pd<512;i_pd++) {
      page_directory[i_pdp][i_pd] =  i_pdp * kPageSize1G + i_pd * kPageSize2M | 0x083; // 0x80 と 0x03
    }
  }

  SetCR3(reinterpret_cast<uint64_t>(&pml4_table[0]));
}

void InitializePaging() {
  SetupIdentityPageTable();
}