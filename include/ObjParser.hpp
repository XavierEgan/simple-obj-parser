#pragma once
#include "CommonInclude.hpp"

#include "Mesh.hpp"
#include "Material.hpp"

namespace objParser {
	namespace ObjParser {
		objParser::Error parseFile(std::string fileName, std::vector<Mesh>& meshs, std::vector<objParser::Material>& materials);
		objParser::Error parseStream(std::istream& stream, std::vector<Mesh>& meshs, std::vector<objParser::Material>& materials);
	};
}