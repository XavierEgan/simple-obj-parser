#pragma once
#include "CommonInclude.hpp"

#include "Material.hpp"

namespace pt {
	namespace MtlParser {
		pt::PtError parseFile(std::string fileName, std::vector<pt::Material>& materials);
		pt::PtError parseStream(std::istream& stream, std::vector<pt::Material>& materials);
	};
}