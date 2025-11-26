#pragma once
#include "CommonInclude.hpp"

#include "Mesh.hpp"
#include "Material.hpp"

namespace objParser {
	objParser::Error parseObjFile(std::string fileName, std::vector<Mesh>& meshs, std::vector<objParser::Material>& materials);
	objParser::Error parseObjStream(std::istream& stream, const std::string& fileName, std::vector<Mesh>& meshs, std::vector<objParser::Material>& materials);
}