#include <bmp.h>

#include <cstring>

bmp::bmp(const char *name, uint32_t width, uint32_t height) : name(name) {
  uint32_t header_size = 54;
  line_padding = (width * 3) % 4;
  size = header_size + (width * 3 + line_padding) * height;

  uint8_t header[] = {
      // Header
      0x42, 0x4D,             // 'BM'
      0x46, 0x00, 0x00, 0x00, // size (54 + ?)
      0x00, 0x00, 0x00, 0x00, 0x36, 0x00, 0x00,
      0x00, // offset to data, 54 (14 + 40)

      // DIB header
      0x28, 0x00, 0x00, 0x00, // number of bytes in the DIB headers
      0x02, 0x00, 0x00, 0x00, // 2 pixel width
      0x02, 0x00, 0x00, 0x00, // 2 pixels height
      0x01, 0x00,             // one color plane
      0x18, 0x00,             // 24 bits color
      0x00, 0x00, 0x00, 0x00, // no compression
      0x10, 0x00, 0x00, 0x00, // raw size
      0x13, 0x0B, 0x00, 0x00, // dpi
      0x13, 0x0B, 0x00, 0x00, // dpi
      0x00, 0x00, 0x00, 0x00, // no palette
      0x00, 0x00, 0x00, 0x00  // all important colors
  };

  // update size, width and height in the header
  std::memcpy(header + 2, &size, sizeof(uint32_t));
  std::memcpy(header + 18, &width, sizeof(uint32_t));
  std::memcpy(header + 22, &height, sizeof(uint32_t));

  file = fopen(name, "w");

  std::fwrite(header, sizeof(uint8_t), header_size, file);
}

bmp &bmp::operator<<(pixel p) {
  std::fwrite(&p, sizeof(uint8_t), 3, file);
  return *this;
}

bmp &bmp::operator<<(special_pixels p) {
  if (line_padding > 0) {
    std::fseek(file, line_padding, SEEK_CUR);
  }
  return *this;
}

bmp::~bmp() { std::fclose(file); }
