#include <std_include.hpp>

#include "game.hpp"
#include "log.hpp"

#include <utils/flags.hpp>
#include <utils/finally.hpp>
#include <utils/nt.hpp>
#include <utils/io.hpp>

#include <combaseapi.h>

namespace game {
void show_error(const std::string &text, const std::string &title) {
  if (quiet_crash()) {
    fflush(stdout);
    fflush(stderr);

    fprintf(stderr, "%s\n%s\n", title.c_str(), text.c_str());

    fflush(stderr);

    trace("[Error]{}\n{}\n", title, text);
  } else if (is_headless()) {
    puts(text.data());
  } else {
    MessageBoxA(nullptr, text.data(), title.data(),
                MB_ICONERROR | MB_SETFOREGROUND | MB_TOPMOST);
  }
}

std::filesystem::path get_appdata_path() {
  static const std::filesystem::path appdata_path =
      []() -> std::filesystem::path {
    const utils::nt::library self{};
    const auto local = self.get_folder() / L"boiii";
    utils::io::create_directory(local);
    utils::io::create_directory(local / L"data");
    utils::io::create_directory(local / L"user");
    return local;
  }();

  return appdata_path;
}

std::filesystem::path get_game_path() {
  return std::filesystem::current_path();
}
} // namespace game