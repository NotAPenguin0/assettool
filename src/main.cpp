#include <assetlib/versions.hpp>
#include <assetlib/texture.hpp>
#include <assetlib/mesh.hpp>
#include <texture_convert.hpp>
#include <mesh_convert.hpp>
#include <options.hpp>
#include <mipgen/mipgen.hpp>
#include <stb_image.h>
#include <argumentum/argparse.h>
#include <chrono>
#include <string>
#include <iostream>
#include <filesystem>

// define the options global
Options g_options = {};

namespace fs = std::filesystem;

constexpr auto LINE_HORIZONTAL = "-----------------------------------------------\n";

static std::string version_string(uint8_t major, uint8_t minor, uint8_t patch) {
	return std::to_string(major) + "." + std::to_string(minor) + "." + std::to_string(patch);
}

static void log_asset_versions(std::ostream& out) {
	out << LINE_HORIZONTAL;
	out << "Asset tool using ITEX asset version: " <<
		version_string(
			assetlib::major_version(assetlib::itex_version),
			assetlib::minor_version(assetlib::itex_version),
			assetlib::patch_version(assetlib::itex_version)
		) << "\n";
	out << "Asset tool using MESH asset version: " <<
		version_string(
			assetlib::major_version(assetlib::mesh_version),
			assetlib::minor_version(assetlib::mesh_version),
			assetlib::patch_version(assetlib::mesh_version)
		) << "\n";
	out << LINE_HORIZONTAL;
}

static bool is_texture(fs::path const& path) {
	if (path.extension() == ".png"
		|| path.extension() == ".bmp"
		|| path.extension() == ".jpg"
		|| path.extension() == ".tga") {
		return true;
	}
	return false;
}

static bool is_mesh(fs::path const& path) {
	if (path.extension() == ".obj") {
		return true;
		// TODO: add more formats, etc
	}
	return false;
}

static void process_file(fs::path const& path, std::ostream& log) {
    auto start = std::chrono::high_resolution_clock::now();
    mipgen::Context mipgen_ctx(mipgen::GenerationMethod::ComputeBicubic);

    if (is_texture(path)) {
        fs::path new_path = path;
        new_path.replace_extension(".tx");

        auto in = plib::binary_input_stream::from_file(path.generic_string().c_str());
        auto out = plib::binary_output_stream::from_file(new_path.generic_string().c_str());

        bool success = convert_texture(mipgen_ctx, in, out, log);
        if (!success) {
            log << "Conversion of texture " << path.generic_string() << " failed.\n";
        }
    }

    else if (is_mesh(path)) {
        fs::path new_path = path;
        new_path.replace_extension(".mesh");

        auto in = plib::binary_input_stream::from_file(path.generic_string().c_str());
        auto out = plib::binary_output_stream::from_file(new_path.generic_string().c_str());

        bool success = convert_mesh(in, out, log);
        if (!success) {
            log << "Conversion of mesh " << path.generic_string() << " failed.\n";
        }
    }

    auto end = std::chrono::high_resolution_clock::now();

    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    std::cout << "Converted and preprocessed " << path.generic_string() << " in " << ms << "ms.\n";
}

static void process_directory(fs::path const& directory, std::ostream& log) {
	mipgen::Context mipgen_ctx(mipgen::GenerationMethod::ComputeBicubic);
	for (fs::directory_entry const& entry : fs::recursive_directory_iterator(directory)) {
		process_file(entry.path(), log);
	}
}

// Functions used to parse arguments
template<>
struct argumentum::from_string<assetlib::ColorSpace> {
    [[nodiscard]] static assetlib::ColorSpace convert(std::string const& s) {
        if (s == "RGB") return assetlib::ColorSpace::RGB;
        else if (s == "sRGB") return assetlib::ColorSpace::sRGB;
        else return assetlib::ColorSpace::Unknown;
    }
};

// Usage: assettool filename
int main(int argc, char** argv) {
	std::ostream& log = std::cout;

    auto parser = argumentum::argument_parser{};
    auto params = parser.params();
    parser.config().program(argv[0]).description("Asset converter tool for Andromeda.");
    params.add_parameter(g_options.file, "--file", "-f")
        .nargs(1)
        .required()
        .help("Path to the unprocessed asset file.");
    params.add_parameter(g_options.colorspace, "--colorspace", "-s")
        .nargs(1)
        .choices({"RGB", "sRGB"})
        .absent(assetlib::ColorSpace::RGB)
        .help("If the asset is an image file, this is the color space of the image.");
    params.add_parameter(g_options.channels, "--channels", "-c")
        .nargs(1)
        .absent(4)
        .help("If the asset is an image file, this is the amount of channels the final processed image must have.");

    if (!parser.parse_args(argc, argv)) return -1;

    log_asset_versions(log);
	stbi_set_flip_vertically_on_load(true);
	process_file(g_options.file, log);
}