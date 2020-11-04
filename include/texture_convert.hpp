#pragma once

#include <filesystem>
#include <iostream>
namespace fs = std::filesystem;

bool convert_texture(fs::path const& input, fs::path const& output, std::ostream& log);