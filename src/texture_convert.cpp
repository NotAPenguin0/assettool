#include <texture_convert.hpp>
#include <assetlib/texture.hpp>
#include <stb_image.h>

#include <cassert>
#include <fstream>
#include <unordered_map>

namespace png {

	static bool check_signature(std::ifstream& file) {
		// From the PNG spec (https://www.w3.org/TR/2003/REC-PNG-20031110/#5PNG-file-signature)
		// The first eight bytes of a PNG datastream always contain the following (decimal) values:
		//  137 80 78 71 13 10 26 10
		constexpr unsigned char signature[]{ 137, 80, 78, 71, 13, 10, 26, 10 };
		constexpr size_t signature_size = sizeof(signature); // dividing by sizeof(byte) would be a bit silly
		unsigned char buffer[signature_size];
		// Attempt to read, and if not enough bytes were read, return an error
		if (!file.read(reinterpret_cast<char*>(&buffer), signature_size)) {
			return false;
		}

		// Verify that the signature matches
		if (memcmp(signature, buffer, signature_size) != 0) {
			return false;
		}

		return true;
	}

	enum class ChunkType {
		// These chunks are required to be present in a valid PNG file
		IHDR, // Header
		PLTE, // Palette
		IDAT, // Data
		IEND, // End
		// These chunks are optional
		sRGB, // srgb information
		gAma, // gamma information
		pHYs, // Intended pixel size/aspect ratio. Ignored when parsing
		Invalid // Invalid or unsupported chunk type
	};

	struct ChunkInfo {
		ChunkType type = ChunkType::Invalid;
		uint32_t size = 0;

		// Amount of bytes the ChunkInfo struct takes up in a file
		static constexpr size_t file_bytes = 8;
	};

	struct ChunkHeader {
		static constexpr size_t size = 4;
		const unsigned char data[4];
	};

	static std::unordered_map<ChunkType, ChunkHeader> chunk_headers{
		{ ChunkType::IHDR, {{ 73, 72, 68, 82 }} },
		{ ChunkType::sRGB, {{ 115, 82, 71, 66 }} },
		{ ChunkType::gAma, {{ 103, 65, 77, 65 }} },
		{ ChunkType::pHYs, {{ 112, 72, 89, 115 }} },
		{ ChunkType::IDAT, {{ 73, 68, 65, 84 }} }
	};

	static ChunkInfo next_chunk_info(std::ifstream& file) {
		ChunkInfo info;
		if (!file.read(reinterpret_cast<char*>(&info.size), sizeof(uint32_t))) {
			return info;
		}

		unsigned char header_buffer[ChunkHeader::size];
		// Attempt to read the chunk header.
		if (!file.read(reinterpret_cast<char*>(&header_buffer), ChunkHeader::size)) {
			return info; // type is initialized to invalid
		}

		for (auto const& [type, header] : chunk_headers) {
			if (memcmp(header_buffer, header.data, ChunkHeader::size) == 0) {
				info.type = type;
				return info;
			}
		}

		return info;
	}

	// Ignores count bytes from the file
	static bool ignore_bytes(std::ifstream& file, size_t count) {
		return file.seekg(count, std::ios::cur).good();
	}

	static void skip_chunk_contents(std::ifstream& file, ChunkInfo const& info) {
		ignore_bytes(file, info.size);
	}

	static assetlib::ColorSpace get_color_space(std::ifstream& file) {
		bool valid_png = check_signature(file);
		assert(valid_png && "Corrupt PNG file detected.");

		// Skip through chunks until we find an sRGB chunk.
		ChunkInfo next_chunk = next_chunk_info(file);
		while (next_chunk.type != ChunkType::Invalid) {
			if (next_chunk.type == ChunkType::sRGB) {
				return assetlib::ColorSpace::sRGB;
			}
			skip_chunk_contents(file, next_chunk);
			next_chunk = next_chunk_info(file);
		}
		return assetlib::ColorSpace::RGB;
	}

} // namespace png

bool convert_texture(mipgen::Context& mipgen_ctx, plib::binary_input_stream& in, plib::binary_output_stream& out, std::ostream& log) {
	int width, height, channels;

	uint32_t const file_size = in.size();
	unsigned char* in_file = new unsigned char[file_size];
	in.read(in_file, file_size);

	unsigned char* pixels = stbi_load_from_memory(in_file, file_size, &width, &height, &channels, STBI_rgb_alpha);
	if (pixels == nullptr) {
		log << "Error: Failed to read texture" << std::endl;
		delete[] in_file;
		return false;
	}
	mipgen::ImageInfo img_info;
	img_info.extents[0] = width;
	img_info.extents[1] = height;
	img_info.format = mipgen::ImageFormat::sRGBA8;
	img_info.pixels = pixels;

	uint32_t output_byte_size = mipgen::output_buffer_size(img_info);
	unsigned char* pixels_with_mipmaps = new unsigned char[output_byte_size];
	mipgen_ctx.generate_mipmap(img_info, pixels_with_mipmaps);

	assetlib::TextureInfo info;
	info.byte_size = output_byte_size;
	info.extents[0] = width;
	info.extents[1] = height;
	info.mip_levels = mipgen::get_mip_count(img_info);
	info.compression = assetlib::CompressionMode::LZ4;
	info.format = assetlib::TextureFormat::RGBA8;
	assetlib::AssetFile converted = assetlib::pack_texture(info, pixels_with_mipmaps);
	delete[] pixels_with_mipmaps;
	delete[] in_file;
	stbi_image_free(pixels);

	return assetlib::save_binary_file(out, converted);
}