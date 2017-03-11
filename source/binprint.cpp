#include <bmp.h>

#include <cstdio>

int main(int argc, char *argv[]) {

  if (3 == argc) {
    std::FILE *file = fopen(argv[1], "r");
    std::fseek(file, 0, SEEK_END);
    std::size_t filesize = std::ftell(file);
    std::fseek(file, 0, SEEK_SET);

    uint32_t width = 200;
    uint32_t height = filesize / (width);

    bmp img(argv[2], width, height);

    for (size_t i = 0; i < height; ++i) {
      for (size_t j = 0; j < width; ++j) {
        int8_t prev = (int8_t)std::fgetc(file);
        int8_t cur = (int8_t)std::fgetc(file);
        int8_t next = (int8_t)std::fgetc(file);

        // put current char and next one back for the next pixel
        std::ungetc(cur, file);
        std::ungetc(next, file);

        img << bmp::pixel{static_cast<uint8_t>(prev - cur),
                          static_cast<uint8_t>(cur),
                          static_cast<uint8_t>(cur - next)};
      }
      img << bmp::ENDL;
    }
  }

  return 0;
}
