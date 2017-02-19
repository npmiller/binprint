
#include <cstdint>
#include <cstdio>
#include <cstring>


struct bmp {
	bmp(const char* name, uint32_t width, uint32_t height) : name(name) {
		line_padding = (width * 3) % 4;
		size = header_size + (width * 3 + line_padding) * height;

		uint8_t header[] = {
			// Header
			0x42, 0x4D,             // 'BM'
			0x46, 0x00, 0x00, 0x00, // size (54 + ?)
			0x00, 0x00,
			0x00, 0x00,
			0x36, 0x00, 0x00, 0x00, // offset to data, 54 (14 + 40)

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

	~bmp() {
		std::fclose(file);
	}

#pragma pack(push, 1)
	struct pixel {
		uint8_t r;
		uint8_t g;
		uint8_t b;
	};
#pragma pack(pop)

	enum special_pixels {
		ENDL
	};

	bmp& operator<<(pixel p) {
		std::fwrite(&p, sizeof(uint8_t), 3, file);
		return *this;
	}

	bmp& operator<<(special_pixels p) {
		if (line_padding > 0) {
			std::fseek(file, line_padding, SEEK_CUR);
		}
		return *this;
	}


	const char* name;
	static const uint32_t header_size = 54;
	/* uint8_t header[header_size]; */
	std::FILE *file;

	uint32_t line_padding;
	uint32_t size;

};

int main(int argc, char *argv[])
{

	if (3 == argc) {
		std::FILE* file = fopen(argv[1], "r");
		std::fseek(file, 0, SEEK_END);
		std::size_t filesize = std::ftell(file);
		std::fseek(file, 0, SEEK_SET);

		uint32_t width = 200;
		uint32_t height = filesize / (width);

		bmp img(argv[2], width, height);

		for (size_t i = 0; i < height; ++i) {
			for (size_t j = 0; j < width; ++j) {
				uint8_t c = (uint8_t)std::fgetc(file);
				img << bmp::pixel{c, c, c};
			}
			img << bmp::ENDL;
		}
	} else {
		bmp img("img.bmp", 4, 4);
		img << bmp::pixel{0,0,0} << bmp::pixel{255,255,255} << bmp::pixel{0,0,0} << bmp::pixel{255,255,255} << bmp::ENDL;
		img << bmp::pixel{255,255,255} << bmp::pixel{0,0,0} << bmp::pixel{255,255,255} << bmp::pixel{0,0,0} << bmp::ENDL;
		img << bmp::pixel{0,0,0} << bmp::pixel{255,255,255} << bmp::pixel{0,0,0} << bmp::pixel{255,255,255} << bmp::ENDL;
		img << bmp::pixel{255,255,255} << bmp::pixel{0,0,0} << bmp::pixel{255,255,255} << bmp::pixel{0,0,0} << bmp::ENDL;
	}
		

	/* std::FILE *bmp = fopen("img.bmp", "w"); */

	/* // bitmap header in bytes */
	/* constexpr uint32_t header_size = 2 + 4 + 2 + 2 + 4; */
	/* constexpr uint32_t offset = 0; */
	/* constexpr uint32_t size = header_size + 12; // 4 pixels */


	/* uint8_t header_data[header_size]; */
	/* uint8_t* header = header_data; */
	/* /1* = { *1/ */
	/* /1* 	66, 65, *1/ */
	/* /1* 	size, *1/ */
	/* /1* }; *1/ */

	/* uint8_t byte = 66; */
	/* std::memcpy(header, &byte, 1); */
	/* byte = 65; */
	/* header++; */
	/* std::memcpy(header, &byte, 1); */
	/* header++; */
	/* std::memcpy(header, &size, 4); */
	/* header += 4; */

	/* // skip app id */
	/* header += 4; */

	/* // offset */
	/* std::memcpy(header, &offset, 4); */

	// write header

	/* const uint8_t data[] = { */
	/* 	0x00, 0x00, 0x00, */
	/* 	0xFF, 0xFF, 0xFF, */ 
	/* 	0x00, 0x00, */
	/* 	0x00, 0x00, 0x00, */
	/* 	0xFF, 0xFF, 0xFF, */
	/* 	0x00, 0x00 */
	/* }; */

	/* std::fwrite(header, sizeof(uint8_t), 16, bmp); */

	// 4 pixels
	/* uint8_t data[12]; */
	/* uint8_t* data_ptr = data; */

	/* uint8_t color = 0; */
	/* std::memcpy(data_ptr, &color, 1); */
	/* data_ptr++; */
	/* std::memcpy(data_ptr, &color, 1); */
	/* data_ptr++; */
	/* std::memcpy(data_ptr, &color, 1); */
	/* data_ptr++; */

	/* color = 255; */
	/* std::memcpy(data_ptr, &color, 1); */
	/* data_ptr++; */
	/* std::memcpy(data_ptr, &color, 1); */
	/* data_ptr++; */
	/* std::memcpy(data_ptr, &color, 1); */
	/* data_ptr++; */
	
	/* color = 0; */
	/* std::memcpy(data_ptr, &color, 1); */
	/* data_ptr++; */
	/* std::memcpy(data_ptr, &color, 1); */
	/* data_ptr++; */
	/* std::memcpy(data_ptr, &color, 1); */
	/* data_ptr++; */

	/* color = 255; */
	/* std::memcpy(data_ptr, &color, 1); */
	/* data_ptr++; */
	/* std::memcpy(data_ptr, &color, 1); */
	/* data_ptr++; */
	/* std::memcpy(data_ptr, &color, 1); */
	/* data_ptr++; */

	// write data
	/* std::fwrite(data, sizeof(uint8_t), 12, bmp); */

	return 0;
}
