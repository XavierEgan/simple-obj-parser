#pragma once
#include "CommonInclude.hpp"

#include "Material.hpp"
#include <filesystem>

namespace objParser {
	objParser::Error parseMtlFile(std::filesystem::path fileName, std::vector<objParser::Material>& materials);
	objParser::Error parseMtlStream(std::istream& stream, const std::filesystem::path& fileName, std::vector<objParser::Material>& materials);
}