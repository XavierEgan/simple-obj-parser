#pragma once
#include "CommonInclude.hpp"

#include "Mesh.hpp"
#include "Material.hpp"

namespace pt {
	namespace ObjParser {
		pt::PtError parseFile(std::string fileName, std::vector<Mesh>& meshs, std::vector<pt::Material>& materials);
		pt::PtError parseStream(std::istream& stream, std::vector<Mesh>& meshs, std::vector<pt::Material>& materials);
	};
}