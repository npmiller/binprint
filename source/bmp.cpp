#include <bmp.h>

#include <cstring>

bmp::bmp(const char *name, uint32_t width) : name(name), width(width), height(0), line(0) {
  line_padding = (width * 3) % 4;
  size = header_size;
  file = fopen(name, "w");

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
  std::fwrite(header, sizeof(uint8_t), header_size, file);
}

bmp &bmp::operator<<(pixel p) {
  if (line >= width) {
    std::fseek(file, line_padding, SEEK_CUR);
    line = 0;
    height++;
  }

  std::fwrite(&p, sizeof(uint8_t), 3, file);
  line++;
  return *this;
}

bmp &bmp::operator<<(special_pixels p) {
  for (int i = line; i < width; ++i) {
    (*this) << pixel{255, 255, 255};
  }
  return *this;
}

bmp::~bmp() {
  size += ((3 * width) + line_padding) * height;

  std::fseek(file, 2, SEEK_SET);
  std::fwrite(&size, sizeof(uint32_t), 1, file);

  std::fseek(file, 18, SEEK_SET);
  std::fwrite(&width, sizeof(uint32_t), 1, file);

  int32_t height_flipped = -height;
  std::fseek(file, 22, SEEK_SET);
  std::fwrite(&height_flipped, sizeof(int32_t), 1, file);

  std::fclose(file);
}
