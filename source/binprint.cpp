#include <bmp.h>

#include <cstdio>
#include <lua.hpp>
#include <vector>

int main(int argc, char *argv[]) {

  // setup lua env
  lua_State *L = luaL_newstate();
  luaL_openlibs(L);

  int luaerr = LUA_OK;

  // load config file
  luaerr = luaL_dofile(L, "config.lua");
  if (LUA_OK != luaerr) {
    lua_close(L);
    fprintf(stderr, "error while loading config file");
    return 1;
  }

  lua_getglobal(L, "window_size");
  if (!lua_isnumber(L, -1)) {
    lua_close(L);
    fprintf(stderr, "error window_size must be a number");
    return 1;
  }
  size_t window_size = lua_tonumber(L, -1);

  lua_getglobal(L, "width");
  size_t width = lua_tonumber(L, -1);

  if (3 == argc) {
    std::FILE *file = fopen(argv[1], "r");
    std::fseek(file, 0, SEEK_END);
    std::size_t filesize = std::ftell(file);
    std::fseek(file, 0, SEEK_SET);

    size_t height = filesize / width;

    bmp img(argv[2], width, height);

    std::vector<uint8_t> points(window_size);

    for (size_t i = 0; i < height; ++i) {
      for (size_t j = 0; j < width; ++j) {

        lua_getglobal(L, "compute_color");

        for (size_t w = 0; w < window_size; ++w) {
          points[w] = static_cast<uint8_t>(std::fgetc(file));
          lua_pushinteger(L, points[w]);
        }

        lua_call(L, window_size, 3);

        img << bmp::pixel{static_cast<uint8_t>(lua_tonumber(L, -1)),
                          static_cast<uint8_t>(lua_tonumber(L, -2)),
                          static_cast<uint8_t>(lua_tonumber(L, -3))};

        for (size_t w = window_size - 1; w > 0; --w) {
          std::ungetc(points[w], file);
        }
      }
      img << bmp::ENDL;
    }
  }

  return 0;
}
