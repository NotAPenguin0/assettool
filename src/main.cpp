#include <assetlib/versions.hpp>

#include <texture_convert.hpp>
#include <mesh_convert.hpp>
#include <mipgen/mipgen.hpp>
#include <stb_image.h>

#include <chrono>
#include <string>
#include <iostream>
#include <filesystem>

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

static void process_directory(fs::path const& directory, std::ostream& log) {
	uint32_t tex_count = 0;
	uint32_t mesh_count = 0;
	auto start = std::chrono::high_resolution_clock::now();
	mipgen::Context mipgen_ctx(mipgen::GenerationMethod::ComputeBicubic);
	for (fs::directory_entry const& entry : fs::recursive_directory_iterator(directory)) {
		fs::path const& path = entry.path();
		if (is_texture(path)) {
			log << "Found texture to process: " << path.generic_string() << "\n";

			fs::path new_path = path;
			new_path.replace_extension(".tx");

			auto in = plib::binary_input_stream::from_file(path.generic_string().c_str());
			auto out = plib::binary_output_stream::from_file(new_path.generic_string().c_str());

			bool success = convert_texture(mipgen_ctx, in, out, log);
			++tex_count;
			if (!success) {
				log << "Conversion of texture " << path.generic_string() << " failed.\n";
			}
		}

		if (is_mesh(path)) {
			log << "Found mesh to process: " << path.generic_string() << "\n";

			fs::path new_path = path;
			new_path.replace_extension(".mesh");

			auto in = plib::binary_input_stream::from_file(path.generic_string().c_str());
			auto out = plib::binary_output_stream::from_file(new_path.generic_string().c_str());

			bool success = convert_mesh(in, out, log);
			++mesh_count;
			if (!success) {
				log << "Conversion of mesh " << path.generic_string() << " failed.\n";
			}
		}
	}
	auto end = std::chrono::high_resolution_clock::now();

	auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
	std::cout << "Converted and preprocessed " << tex_count << " textures and " << mesh_count << " meshes in " << ms << "ms\n.";
}

// Usage: assettool dir_to_process
int main(int argc, char** argv) {
	std::ostream& log = std::cout;

	if (argc != 2) {
		log << "Error: Invalid argument count. Usage: assettool dir_to_process" << std::endl;
		return -1;
	}

	log_asset_versions(log);
	fs::path dir(argv[1]);
	log << "Processing directory " << dir.generic_string() << "\n";

	stbi_set_flip_vertically_on_load(true);
	process_directory(dir, log);
}