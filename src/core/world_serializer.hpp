#pragma once

#include <string>

namespace soft_tissues::world_serializer {

void save(const std::string &file_path);
void load(const std::string &file_path);

}  // namespace soft_tissues::world_serializer
