#ifndef BMP_INCLUDED_H
#define BMP_INCLUDED_H

#include <cstdint>
#include <cstdio>

struct bmp final {

  bmp(const char *name, uint32_t width);
  ~bmp();

#pragma pack(push, 1)
  struct pixel {
    uint8_t r;
    uint8_t g;
    uint8_t b;
  };
#pragma pack(pop)

  enum special_pixels { ENDL };

  bmp &operator<<(pixel p);
  bmp &operator<<(special_pixels p);

  const char *name;
  std::FILE *file;

  uint32_t width;
  uint32_t height;
  uint32_t line_padding;
  uint32_t size;
  uint32_t line;
  const uint32_t header_size = 54;
};

#endif
