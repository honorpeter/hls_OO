//------------------------------------------------------------------------------
//  SqueezeNetOnFPGA
//------------------------------------------------------------------------------
//
//	File:  output_cache.hpp
//
//  Output Cache Module for FPGA
//
//	(c) David Gschwend, 2016
//
//------------------------------------------------------------------------------

#ifndef OUTPUT_CACHE_HPP_07571FC2
#define OUTPUT_CACHE_HPP_07571FC2

// Data Types for FPGA Implementation
#include "fpga_top.hpp"

// ================
// = Output Cache =
// ================
class OutputCache {
 public:
  OutputCache(const char* name = "OutputCache");
  void accumulateChannel(channel_t c, data_t data);
  data_t getChannel(channel_t c);
  void setChannel(channel_t c, data_t data);
  void reset();
  //data_t *BRAMPointer() { return BRAM; }

 private:
#ifdef EBUG
  const char *_name;  // for debugging / logging
#endif
  data_t BRAM[MAX_NUM_CHOUT];
};

#endif /* end of include guard: OUTPUT_CACHE_HPP_07571FC2 */
