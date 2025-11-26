#pragma once
#include "CommonInclude.hpp"

#include "Material.hpp"

namespace objParser {
	namespace MtlParser {
		objParser::Error parseFile(std::string fileName, std::vector<objParser::Material>& materials);
		objParser::Error parseStream(std::istream& stream, std::vector<objParser::Material>& materials);
	};
}