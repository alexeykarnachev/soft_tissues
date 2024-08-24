#pragma once

#include <string>

namespace soft_tissues::serialization {

void save_scene(std::string file_path);
void load_scene(std::string file_path);

}  // namespace soft_tissues::serialization
