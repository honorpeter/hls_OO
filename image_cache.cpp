//------------------------------------------------------------------------------
//  SqueezeNetOnFPGA
//------------------------------------------------------------------------------
//
//	File:  image_cache.cpp
//
//  Image Cache Module for FPGA
//
//	(c) David Gschwend, 2016
//
//------------------------------------------------------------------------------

#include "image_cache.hpp"

// ===============
// = Image Cache =
// ===============
ImageCache::ImageCache(){};
void ImageCache::reset() {
  LOG("ImageCache: Reset Write Pointers.\n");

  curr_img_cache_line = 0;
  next_img_cache_addr = 0;
}

void ImageCache::setNextChannel(data_t value) {
  if (LOG_DETAILS)
    LOG("ImageCache: setNextChannel         ICACHE[%3d] <- %.2f.\n",
        (int)next_img_cache_addr, value);

  BRAM[next_img_cache_addr] = value;
  next_img_cache_addr++;
  if (next_img_cache_addr == line_width * NUM_IMG_CACHE_LINES)
    next_img_cache_addr = 0;
}

void ImageCache::preloadPixelFromDRAM(MemoryController *DRAM) {
#pragma HLS inline
  LOG("ImageCache: preloadPixelFromDRAM (%2d channels)\n", (int)ch_in);

  LOG_LEVEL_INCR;

L_PRELOAD_PIXEL_CHANNELS:
  for (channel_t ci = 0; ci < ch_in; ci++) {
#pragma HLS LOOP_TRIPCOUNT min=3 max=1024 avg=237
#pragma HLS pipeline II=1 rewind

    if (loads_left == 0) {
      LOG("ImageCache: NO MORE PIXELS LEFT IN DRAM\n");
      // break;
    } else {
      data_t px = DRAM->loadNextChannel();
      setNextChannel(px);
      loads_left--;
    }
  }

  LOG_LEVEL_DECR;
}

void ImageCache::preloadRowFromDRAM(MemoryController *DRAM) {
#pragma HLS inline off
  LOG("ImageCache: preloadRowFromDRAM (%2d pixels, each %2d channels)\n",
      (int)width_in, (int)ch_in);

  LOG_LEVEL_INCR;

L_DRAM_PRELOADROW_X:
  for (coordinate_t x = 0; x < width_in; x++) {
#pragma HLS LOOP_TRIPCOUNT min=8 max=256 avg=45
    preloadPixelFromDRAM(DRAM);
  }

  LOG_LEVEL_DECR;
}

void ImageCache::setLayerConfig(layer_t &layer) {
  width_in = layer.width;
  ch_in = layer.channels_in;
  line_width = ch_in * width_in;
  loads_left = line_width * layer.height;

  LOG("ImageCache: setLayerConfig\n");
  LOG(" - width_in        = %d\n", (int)width_in);
  LOG(" - ch_in           = %d\n", (int)ch_in);
  LOG(" - line_width      = %d\n", (int)line_width);
  LOG(" - height_in       = %d\n", (int)layer.height);
  LOG(" - loads_left      = %d\n", (int)loads_left);

  reset();
}

data_t ImageCache::getPixel(coordinate_t y, coordinate_t x, channel_t ci) {
  assert(((x >= 0) & (x < width_in)) && "ImageCache: ILLEGAL PIXEL REQUESTED");

  cacheline_t req_line = (y) % NUM_IMG_CACHE_LINES;
  imgcacheaddr_t addr = req_line * width_in * ch_in + x * ch_in + ci;

  /*if (LOG_DETAILS)
    LOG(
        "ImageCache: getPixel( y: %d, x: %d, ci: %d ) "
        "= ICACHE[%3d] (cache line %d) -> %.2f\n",
        (int)y, (int)x, (int)ci, (int)addr, (int)req_line, BRAM[addr]);*/

  return BRAM[addr];
}
