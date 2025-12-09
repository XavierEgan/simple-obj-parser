#pragma once
#include "CommonInclude.hpp"

namespace objParser {
    struct Material {
        std::string name;
		glm::vec3 ambientColor = glm::vec3(0.0f, 0.0f, 0.0f);		// Ka
		glm::vec3 diffuseColor = glm::vec3(0.0f, 0.0f, 0.0f);		// Kd
		glm::vec3 specularColor = glm::vec3(0.0f, 0.0f, 0.0f);		// Ks
		float specularExponent = 0.0f;								// Ns
		float transparent = 0.0f;									// d or Tr
		glm::vec3 transmissionFilter = glm::vec3(0.0f, 0.0f, 0.0f);	// Tf
		float indexOfRefraction = 0.0f;								// Ni / index of refraction


        Material(const std::string& name);
    };
}