#include <mesh_convert.hpp>
#include <assetlib/mesh.hpp>

#include <cassert>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

bool convert_mesh(plib::binary_input_stream& in, plib::binary_output_stream& out, std::ostream& log) {
	assetlib::MeshInfo info;
	info.compression = assetlib::CompressionMode::LZ4;
	info.format = assetlib::VertexFormat::PNTV32;
	info.index_bits = 32; // We will be using 32-bit indices everywhere

	Assimp::Importer importer;
	uint32_t file_size = in.size();
	unsigned char* file_mem = new unsigned char[file_size];
	in.read_bytes(file_mem, file_size);
	aiScene const* scene = importer.ReadFileFromMemory(file_mem, file_size, aiProcess_CalcTangentSpace | aiProcess_GenNormals | aiProcess_Triangulate);

	// Temporary importer only converts first mesh in the scene.
	aiMesh const* mesh = scene->mMeshes[0];

	std::vector<assetlib::PNTV32Vertex> vertices;
	vertices.resize(mesh->mNumVertices);
	std::vector<uint32_t> indices;
	indices.resize(mesh->mNumFaces * 3);

	for (size_t i = 0; i < mesh->mNumVertices; ++i) {
		assetlib::PNTV32Vertex& dst = vertices[i];
		static_assert(sizeof(aiVector3D) == 3 * sizeof(float), "cannot memcpy aiVector3D if data is not correctly sized"); // add fail path for when this doesn't match?
		memcpy(dst.position, &mesh->mVertices[i], 3 * sizeof(float));
		memcpy(dst.normal, &mesh->mNormals[i], 3 * sizeof(float));
		memcpy(dst.tangent, &mesh->mTangents[i], 3 * sizeof(float));
		memcpy(dst.uv, &mesh->mTextureCoords[0][i], 2 * sizeof(float));
	}

	for (size_t i = 0; i < mesh->mNumFaces; ++i) {
		uint32_t* dst_start = &indices[3 * i];
		aiFace& face = mesh->mFaces[i];
		static_assert(sizeof(unsigned int) == sizeof(uint32_t), "cannot safely memcpy indices"); // Maybe add fail path for when this does not match
		assert(face.mNumIndices == 3 && "Mesh not triangulated properly");
		memcpy(dst_start, face.mIndices, 3 * sizeof(uint32_t));
	}

	info.vertex_count = vertices.size();
	info.index_count = indices.size();

	delete[] file_mem;

	assetlib::AssetFile file = assetlib::pack_mesh(info, vertices.data(), indices.data());
	return assetlib::save_binary_file(out, file);
}