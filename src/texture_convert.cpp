#include <texture_convert.hpp>
#include <assetlib/texture.hpp>
#include <stb_image.h>

bool convert_texture(fs::path const& input, fs::path const& output, std::ostream& log) {
	int width, height, channels;
	unsigned char* pixels = stbi_load(input.generic_string().c_str(), &width, &height, &channels, STBI_rgb_alpha);
	if (pixels == nullptr) { 
		log << "Error: Failed to read texture " << input.generic_string() << ".\n";
		return false; 
	}
	int byte_size = width * height * 4;
	
	assetlib::TextureInfo info;
	info.byte_size = byte_size;
	info.extents[0] = width;
	info.extents[1] = height;
	info.color_space = assetlib::ColorSpace::SRGB; // TODO, proper rgb/srgb detection
	info.format = assetlib::TextureFormat::RGBA8;

	assetlib::AssetFile converted = assetlib::pack_texture(info, pixels);
	stbi_image_free(pixels);
	return assetlib::save_binary_file(output.generic_string(), converted);
}