#ifndef INCLUDE_FS_H
#define INCLUDE_FS_H

#ifndef EXP_FS
#include <filesystem>
namespace fs = std::filesystem;
#else
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
#endif  // EXP_FS

#endif  // INCLUDE_FS_H
