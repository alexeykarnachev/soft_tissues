#pragma once

#include <string>

namespace soft_tissues::serialization {

void save(std::string file_path);
void load(std::string file_path);

}  // namespace soft_tissues::serialization
