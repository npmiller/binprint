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

struct window final {
  struct iterator final {
    size_t pos;
    const size_t size;
    const uint8_t *data;

    iterator &operator++() {
      pos++;
      pos %= size;
      return *this;
    }

    iterator operator+(int idx) {
      size_t new_pos = pos + idx;
      new_pos %= size;
      return iterator(new_pos, size, data);
    }

    bool operator==(iterator other) const { return pos == other.pos; }
    bool operator!=(iterator other) const { return pos != other.pos; }
    uint8_t operator*() const { return data[pos]; }

    iterator(size_t pos, const size_t size, const uint8_t *data)
        : pos(pos), size(size), data(data) {}
  };

  iterator begin() {
    return iterator(begin_idx, size + 1, data);
  }

  iterator end() {
    return iterator(end_idx, size + 1, data);
  }

  window(size_t size) : size(size), begin_idx(0), end_idx(size) {
    data = new uint8_t[size + 1];
  }

  ~window() {
    delete[] data;
  }

  void push(uint8_t value) {
    data[end_idx] = value;

    ++end_idx;
    end_idx %= size + 1;

    ++begin_idx;
    begin_idx %= size + 1;
  }

  void set(iterator it, uint8_t value) {
    data[it.pos] = value;
  }

  uint8_t *data;
  size_t size;
  size_t begin_idx;
  size_t end_idx;
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

  lua_pushinteger(L, bmp::ENDL);
  lua_setglobal(L, "ENDL");

  lua_pushinteger(L, bmp::SKIP);
  lua_setglobal(L, "SKIP");

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
  lua_pop(L, 1);

  size_t window = 1;
  lua_getglobal(L, "window");
  if (!lua_isnil(L, -1)) {
    window = lua_tonumber(L, -1);
    if (window == 0 || window % 2 != 1) {
      fprintf(stderr,
              "binprint: invalid window in plugin '%s', window must be a "
              "non-zero odd positive number\n",
              plugin.c_str());
    }
  }
  lua_pop(L, 1);

  uint8_t padding = 0;
  lua_getglobal(L, "window_padding");
  if (!lua_isnil(L, -1)) {
    padding = lua_tonumber(L, -1);
  }
  lua_pop(L, 1);

  size_t prefetch = window - (window / 2) - 1;
  binprint::window w(window);

  for (int i = 0; i < (window / 2) + 1; ++i) {
    w.set(w.begin() + i, padding);
  }

  for (int i = 0; i < prefetch; ++i) {
    w.set(w.begin() + i, std::fgetc(file));
  }

  bmp img(args.output, width);

  for (int i = 0; i < (filesize + prefetch); ++i) {
    if (i < filesize) {
      w.push(std::fgetc(file));
    } else {
      w.push(padding);
    }

    lua_getglobal(L, "compute_color");
    for (uint8_t c : w) {
      lua_pushinteger(L, c);
    }

    luaerr = lua_pcall(L, w.size, 3, 0);
    if (LUA_OK != luaerr) {
      const char *err = lua_tostring(L, -1);
      lua_close(L);
      std::fclose(file);
      fprintf(stderr, "binprint: %s\n", err);
      return -1;
    }

    if (lua_isnil(L, -1)) {
      bmp::special_pixels code =
          static_cast<bmp::special_pixels>(lua_tonumber(L, -2));
      switch (code) {
      case bmp::ENDL:
      case bmp::SKIP:
        img << code;
        lua_pop(L, 3);
        break;
      default:
        fprintf(stderr, "binprint: invalid special pixel code\n");
      }
      continue;
    }

    img << bmp::pixel{static_cast<uint8_t>(lua_tonumber(L, -1)),
                      static_cast<uint8_t>(lua_tonumber(L, -2)),
                      static_cast<uint8_t>(lua_tonumber(L, -3))};
    lua_pop(L, 3);
  }
  img << bmp::ENDL;

  std::fclose(file);

  return 0;
}
