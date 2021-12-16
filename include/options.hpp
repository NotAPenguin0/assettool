#pragma once

#include <assetlib/texture.hpp>
#include <assetlib/mesh.hpp>
#include <filesystem>

namespace fs = std::filesystem;

struct Options {
    fs::path file;
    assetlib::ColorSpace colorspace;
    int channels;
};

extern Options g_options;