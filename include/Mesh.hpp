#pragma once
#include "CommonInclude.hpp"

namespace pt {
	struct Mesh {
		std::vector<glm::vec3> vertices;
		std::vector<glm::vec3> vertexTextureCoordinates;
		std::vector<glm::vec3> vertexNormals;

		std::vector<int> vertexIndexes;
		std::vector<int> vertexTextureCoordinatesIndexes;
		std::vector<int> vertexNormalsIndexes;

		std::string name;

		Mesh(std::string name);
	};
}