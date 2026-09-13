#include <std_include.hpp>
#include "workshop.hpp"

#include <game/game.hpp>
#include <utils/io.hpp>
#include <utils/string.hpp>

namespace workshop {
std::atomic<bool> downloading_workshop_item{false};
std::atomic<bool> launcher_downloading{false};

bool is_any_download_active() { return false; }

int get_workshop_retry_attempts() { return 0; }

std::string get_usermap_publisher_id(const std::string &) { return {}; }

std::string get_mod_publisher_id() { return {}; }

std::string get_mod_resized_name() { return {}; }

bool check_valid_usermap_id(const std::string &, const std::string &,
                            const std::string &, const std::string &) {
  return true;
}

bool check_valid_mod_id(const std::string &, const std::string &) {
  return true;
}

bool mod_switch_requires_fs_reinitialization(const std::string &,
                                             const std::string &) {
  return false;
}

bool mod_load_requires_fs_reinitialization(std::string &) { return false; }

void setup_same_mod_as_host(game::LocalClientNum_t, const std::string &,
                            const std::string &, bool) {}

namespace {
std::string pending_mod_reconnect{};
std::string pending_download_reconnect{};
} // namespace

void set_pending_mod_reconnect(const std::string &address) {
  pending_mod_reconnect = address;
}

std::string get_pending_mod_reconnect() { return pending_mod_reconnect; }

void set_pending_download_reconnect(const std::string &address) {
  pending_download_reconnect = address;
}

std::string get_pending_download_reconnect() {
  return pending_download_reconnect;
}

std::uint64_t compute_folder_size_bytes(const std::filesystem::path &) {
  return 0;
}

std::string human_readable_size(std::uint64_t bytes) {
  return std::to_string(bytes) + " B";
}

workshop_info get_steam_workshop_info(const std::string &) { return {}; }

void load_workshop_data(game::ugc::WorkshopData *) {}

void supplement_mods_from_disk() {}

void supplement_ugc_from_workshop(game::ZoneType) {}

const char *va_mods_path(const char *fmt, const char *root_dir,
                         const char *mods_dir, const char *dir_name) {
  const auto original_path =
      utils::string::va(fmt, root_dir, mods_dir, dir_name);
  if (utils::io::directory_exists(original_path)) {
    return original_path;
  }
  return utils::string::va("%s/%s/%s", root_dir, mods_dir, dir_name);
}

const char *va_user_content_path(const char *fmt, const char *root_dir,
                                 const char *user_content_dir) {
  const auto original_path = utils::string::va(fmt, root_dir, user_content_dir);
  if (utils::io::directory_exists(original_path)) {
    return original_path;
  }
  return utils::string::va("%s/%s", root_dir, user_content_dir);
}
} // namespace workshop
