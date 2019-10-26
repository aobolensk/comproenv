#ifndef INCLUDE_CONST_H
#define INCLUDE_CONST_H
#include <string>
#include "fs.h"

namespace comproenv {

const static std::string env_prefix = (fs::path("data") / "env_").string();
const static std::string task_prefix = "task_";
const static std::string cache_file_name = ".comproenv_cache";
const static std::string application_name = "comproenv";

}  // namespace comproenv

#endif  // INCLUDE_CONST_H
