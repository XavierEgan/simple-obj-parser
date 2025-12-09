#pragma once
#include "CommonInclude.hpp"

#include "Mesh.hpp"
#include "Material.hpp"

#include <cctype>
#include <filesystem>
#include <algorithm>

namespace objParser {
	objParser::Error parseObjFile(std::filesystem::path fileName, std::vector<Mesh>& meshs, std::vector<objParser::Material>& materials);
	objParser::Error parseObjStream(std::istream& stream, const std::filesystem::path& objPath, std::vector<Mesh>& meshs, std::vector<objParser::Material>& materials);
}