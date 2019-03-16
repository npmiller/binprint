#include <bmp.h>

#include <cstdio>
#include <lua.hpp>

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

  lua_getglobal(L, "width");
  size_t width = lua_tonumber(L, -1);

  lua_getglobal(L, "endl");
  size_t endl = lua_tonumber(L, -1);

  if (3 == argc) {
    std::FILE *file = fopen(argv[1], "r");
    std::fseek(file, 0, SEEK_END);
    std::size_t filesize = std::ftell(file);
    std::fseek(file, 0, SEEK_SET);

    bmp img(argv[2], width);

    char c;
    for (int i = 0; i < filesize; ++i) {
      c = std::fgetc(file);
      if (c == endl) {
        img << bmp::ENDL;
        continue;
      }

      lua_getglobal(L, "compute_color");
      lua_pushinteger(L, c);
      lua_call(L, 1, 3);

      img << bmp::pixel{static_cast<uint8_t>(lua_tonumber(L, -1)),
                        static_cast<uint8_t>(lua_tonumber(L, -2)),
                        static_cast<uint8_t>(lua_tonumber(L, -3))};
    }
    img << bmp::ENDL;
  }

  return 0;
}
