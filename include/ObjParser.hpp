#pragma once
#include "CommonInclude.hpp"

#include "Mesh.hpp"
#include "Material.hpp"

namespace objParser {
	namespace ObjParser {
		objParser::PtError parseFile(std::string fileName, std::vector<Mesh>& meshs, std::vector<objParser::Material>& materials);
		objParser::PtError parseStream(std::istream& stream, std::vector<Mesh>& meshs, std::vector<objParser::Material>& materials);
	};
}