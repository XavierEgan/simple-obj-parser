#pragma once
#include "CommonInclude.hpp"

#include "Material.hpp"

namespace objParser {
	objParser::Error parseMtlFile(std::string fileName, std::vector<objParser::Material>& materials);
	objParser::Error parseMtlStream(std::istream& stream, const std::string& fileName, std::vector<objParser::Material>& materials);
}