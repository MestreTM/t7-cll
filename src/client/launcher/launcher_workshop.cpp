#include <std_include.hpp>
#include "launcher_workshop.hpp"

namespace launcher::workshop {
CComVariant utf8_variant(const std::string &utf8_str) {
  return CComVariant(utf8_str.c_str());
}

void register_callbacks(html_frame *) {}

void try_refresh_workshop_content() {}

std::map<std::string, uint64_t>
batch_get_time_updated(const std::vector<std::string> &) {
  return {};
}

std::map<std::string, workshop_item_meta>
batch_get_workshop_meta(const std::vector<std::string> &) {
  return {};
}
} // namespace launcher::workshop
