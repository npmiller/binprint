#include <bmp.h>

#include <cstdio>
#include <string>
#include <lua.hpp>
#include <vector>
#include <sstream>
#include <fstream>
#include <math.h>

namespace binprint {
struct args final {
  args(int argc, char **argv)
      : input(nullptr), output(nullptr), argc(argc), argv(argv){};

  bool parse() {
    for (int i = 1; i < argc; ++i) {
      std::string arg(argv[i]);

      if (arg == "-h" || arg == "--help") {
        fprintf(stdout, "%s", help);
        exit(0);
      }

      if (arg == "-p" || arg == "--plugin") {
        if (!plugin.empty()) {
          fprintf(
              stderr,
              "binprint: option '-p', '--plugin' can only be specified once\n");
          return false;
        }

        i++;
        if (i >= argc) {
          fprintf(stderr,
                  "binprint: option '-p', '--plugin' requires an argument\n");
          return false;
        }
        plugin.assign(argv[i]);
        continue;
      }

      if (arg == "-L") {
        i++;
        if (i >= argc) {
          fprintf(stderr, "binprint: option '-L' requires an argument\n");
          return false;
        }

        paths.push_back(argv[i]);
        continue;
      }

      if (input == nullptr) {
        input = argv[i];
        continue;
      }

      if (output == nullptr) {
        output = argv[i];
        continue;
      }

      fprintf(stderr, "binprint: unexpected positional argument '%s'\n",
              argv[i]);
      return false;
    }

    // default plugin
    if (plugin.empty()) {
      plugin.assign("code");
    }

    // add default plugin search paths
    const char *home = std::getenv("HOME");
    if (nullptr != home) {
      std::stringstream s;
      s << home << "/.config/binprint/plugins";
      paths.push_back(s.str());
    }
    paths.push_back("/etc/binprint/plugins");
    paths.push_back("/usr/share/binprint/plugins");

    return true;
  }

  std::string find_plugin_path() {
    for (std::string &path : paths) {
      std::stringstream s;
      s << path << "/" << plugin << ".lua";

      std::ifstream f(s.str().c_str());
      if (f.good()) {
        return s.str();
      }
    }

    return std::string();
  }

  std::string plugin;
  char *input;
  char *output;
  std::vector<std::string> paths;

private:
  int argc;
  char **argv;
  static const constexpr char *help =
      R"help(Usage: binprint [OPTION] <input file> <output file>
  -h, --help                Print this message
  -p <arg>, --plugin <arg>  Name of the plugin to use
  -L <dir>                  Add directory to plugin search path
)help";
};
} // namespace binprint

int main(int argc, char *argv[]) {

  binprint::args args(argc, argv);

  if (!args.parse()) {
    return -1;
  }

  std::string plugin = args.find_plugin_path();
  if (plugin.empty()) {
    fprintf(stderr, "binprint: couldn't find plugin '%s' in search path\n",
            args.plugin.c_str());
    return -1;
  }

  // setup lua env
  lua_State *L = luaL_newstate();
  luaL_openlibs(L);

  // load plugin
  int luaerr = LUA_OK;
  luaerr = luaL_dofile(L, plugin.c_str());
  if (LUA_OK != luaerr) {
    const char* err = lua_tostring(L, -1);
    lua_close(L);
    fprintf(stderr, "binprint: %s\n", err);
    return -1;
  }

  // get the size of the input file
  std::FILE *file = fopen(args.input, "r");
  std::fseek(file, 0, SEEK_END);
  std::size_t filesize = std::ftell(file);
  std::fseek(file, 0, SEEK_SET);

  // figure out the width of the output picture
  size_t width;
  lua_getglobal(L, "width");
  if (lua_isnil(L, -1)) {
    // pick default width for roughly square image
    width = sqrt(filesize) + 1;
  } else {
    // get width from plugin
    width = lua_tonumber(L, -1);
    if (width == 0) {
      fprintf(stderr,
              "binprint: invalid width in plugin '%s', width must be "
              "a non-zero positive number\n",
              plugin.c_str());
      std::fclose(file);
      return -1;
    }
  }

  bmp img(args.output, width);

  for (int i = 0; i < filesize; ++i) {
    char c = std::fgetc(file);

    lua_getglobal(L, "compute_color");
    lua_pushinteger(L, c);

    luaerr = lua_pcall(L, 1, 3, 0);
    if (LUA_OK != luaerr) {
      const char *err = lua_tostring(L, -1);
      lua_close(L);
      std::fclose(file);
      fprintf(stderr, "binprint: %s\n", err);
      return -1;
    }

    // nil values from compute_color mean line jump
    if (lua_isnil(L, -1)) {
      img << bmp::ENDL;
      lua_pop(L, 3);
      continue;
    }

    img << bmp::pixel{static_cast<uint8_t>(lua_tonumber(L, -1)),
                      static_cast<uint8_t>(lua_tonumber(L, -2)),
                      static_cast<uint8_t>(lua_tonumber(L, -3))};
  }
  img << bmp::ENDL;

  std::fclose(file);

  return 0;
}
