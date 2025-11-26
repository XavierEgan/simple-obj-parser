#include "CommonInclude.hpp"
#include "MtlParser.hpp"

namespace MtlParserHelpers {
	static objParser::PtError ensureMaterialExists(const std::vector<objParser::Material>& materials) {
		if (materials.size() == 0) {
			return objParser::PtError(objParser::ErrorType::FileFormatError, "Trying to read data before any meshs have been defined");
		}
		return objParser::ErrorType::OK;
	}

	static objParser::PtError newMaterial(std::istream& lineStream, std::vector<objParser::Material>& materials) {
		std::string materialName;
		lineStream >> materialName;

		materials.emplace_back(materialName);

		return objParser::ErrorType::OK;
	}

	static objParser::PtError setAmbient(std::istream& lineStream, std::vector<objParser::Material>& materials) {
		float x, y, z;

		if (!(lineStream >> x >> y >> z)) {
			return objParser::PtError(objParser::ErrorType::FileFormatError, "Reading in ambient failed");
		}

		// range check
		if (x < 0.0f || y < 0.0f || z < 0.0f) {
			return objParser::PtError(objParser::ErrorType::FileFormatError, "Value of ambient is less than 0");
		}
		if (x > 1.0f || y > 1.0f || z > 1.0f) {
			return objParser::PtError(objParser::ErrorType::FileFormatError, "Value of ambient is greater than 1");
		}

		materials.back().ambientColor = glm::vec3(x, y, z);

		return objParser::ErrorType::OK;
	}

	static objParser::PtError setDiffuse(std::istream& lineStream, std::vector<objParser::Material>& materials) {
		float x, y, z;

		if (!(lineStream >> x >> y >> z)) {
			return objParser::PtError(objParser::ErrorType::FileFormatError, "Reading in diffuse failed");
		}
		
		// range check
		if (x < 0.0f || y < 0.0f || z < 0.0f) {
			return objParser::PtError(objParser::ErrorType::FileFormatError, "Value of diffuse is less than 0");
		}
		if (x > 1.0f || y > 1.0f || z > 1.0f) {
			return objParser::PtError(objParser::ErrorType::FileFormatError, "Value of diffuse is greater than 1");
		}

		materials.back().diffuseColor = glm::vec3(x, y, z);

		return objParser::ErrorType::OK;
	}

	static objParser::PtError setSpecular(std::istream& lineStream, std::vector<objParser::Material>& materials) {
		float x, y, z;

		if (!(lineStream >> x >> y >> z)) {
			return objParser::PtError(objParser::ErrorType::FileFormatError, "Reading in specular failed");
		}

		// range check
		if (x < 0.0f || y < 0.0f || z < 0.0f) {
			return objParser::PtError(objParser::ErrorType::FileFormatError, "Value of specular is less than 0");
		}
		if (x > 1.0f || y > 1.0f || z > 1.0f) {
			return objParser::PtError(objParser::ErrorType::FileFormatError, "Value of specular is greater than 1");
		}

		materials.back().specularColor = glm::vec3(x, y, z);

		return objParser::ErrorType::OK;
	}

	static objParser::PtError setSpecularExponent(std::istream& lineStream, std::vector<objParser::Material>& materials) {
		float x;

		if (!(lineStream >> x)) {
			return objParser::PtError(objParser::ErrorType::FileFormatError, "Reading in specular exponent failed");
		}

		// range check
		if (x < 0.0f) {
			return objParser::PtError(objParser::ErrorType::FileFormatError, "Value of specular exponent less than 0");
		}
		if (x > 1000.0f) {
			return objParser::PtError(objParser::ErrorType::FileFormatError, "Value of specular exponent greater than 1000");
		}

		materials.back().specularExponent = x;

		return objParser::ErrorType::OK;
	}

	static objParser::PtError setTransparent(std::istream& lineStream, std::vector<objParser::Material>& materials) {
		float x;

		if (!(lineStream >> x)) {
			return objParser::PtError(objParser::ErrorType::FileFormatError, "Reading in transparent failed");
		}

		// range check
		if (x < 0.0f) {
			return objParser::PtError(objParser::ErrorType::FileFormatError, "Value of transparent less than 0");
		}
		if (x > 1.0f) {
			return objParser::PtError(objParser::ErrorType::FileFormatError, "Value of transparent greater than 1");
		}

		materials.back().transparent = x;

		return objParser::ErrorType::OK;
	}

	static objParser::PtError setInverseTransparent(std::istream& lineStream, std::vector<objParser::Material>& materials) {
		float x;

		if (!(lineStream >> x)) {
			return objParser::PtError(objParser::ErrorType::FileFormatError, "Reading in transparent failed");
		}
		
		// since its inverse
		x = 1 - x;

		// range check
		if (x < 0.0f) {
			return objParser::PtError(objParser::ErrorType::FileFormatError, "Value of transparent less than 0");
		}
		if (x > 1.0f) {
			return objParser::PtError(objParser::ErrorType::FileFormatError, "Value of transparent greater than 1");
		}

		materials.back().transparent = x;

		return objParser::ErrorType::OK;
	}

	static objParser::PtError setTransmissionFilter(std::istream& lineStream, std::vector<objParser::Material>& materials) {
		float x, y, z;

		if (!(lineStream >> x >> y >> z)) {
			return objParser::PtError(objParser::ErrorType::FileFormatError, "Reading in transmission filter failed");
		}

		// range check
		if (x < 0.0f || y < 0.0f || z < 0.0f) {
			return objParser::PtError(objParser::ErrorType::FileFormatError, "Value of transmission filter is less than 0");
		}
		if (x > 1.0f || y > 1.0f || z > 1.0f) {
			return objParser::PtError(objParser::ErrorType::FileFormatError, "Value of transmission filter is greater than 1");
		}

		materials.back().transmissionFilter = glm::vec3(x, y, z);

		return objParser::ErrorType::OK;
	}

	static objParser::PtError setIndexRefraction(std::istream& lineStream, std::vector<objParser::Material>& materials) {
		float x;

		if (!(lineStream >> x)) {
			return objParser::PtError(objParser::ErrorType::FileFormatError, "Reading in optical density/index of refraction failed");
		}

		// range check
		if (x < 0.001f) {
			return objParser::PtError(objParser::ErrorType::FileFormatError, "Value of index of recfraction less than 0.001");
		}
		if (x > 10.0f) {
			return objParser::PtError(objParser::ErrorType::FileFormatError, "Value of index of recfraction greater than 10.0");
		}

		materials.back().indexOfRefraction = x;

		return objParser::ErrorType::OK;
	}
}

objParser::PtError objParser::MtlParser::parseFile(std::string fileName, std::vector<objParser::Material>& materials) {
	std::ifstream inFS(fileName);

	if (!inFS.is_open() || !inFS.good()) {
		std::ostringstream errorStream;
		errorStream << "error reading material file '" << fileName << "'";
		return objParser::PtError(objParser::ErrorType::FileFormatError, errorStream.str());
	}

	objParser::PtError error = objParser::MtlParser::parseStream(inFS, materials);

	return error;
}

objParser::PtError objParser::MtlParser::parseStream(std::istream& stream, std::vector<objParser::Material>& materials) {
	std::string line;
	std::getline(stream, line);

	std::istringstream lineStream(line);

	while (!stream.fail()) {
		std::string prefix;
		lineStream >> prefix;

		if (prefix == "Ka") {
			objParser::PtError error = MtlParserHelpers::ensureMaterialExists(materials);

			if (error != objParser::ErrorType::OK) {
				return error;
			}

			error = MtlParserHelpers::setAmbient(lineStream, materials);

			if (error != objParser::ErrorType::OK) {
				return error;
			}
		} else if (prefix == "Kd") {
			objParser::PtError error = MtlParserHelpers::ensureMaterialExists(materials);

			if (error != objParser::ErrorType::OK) {
				return error;
			}

			error = MtlParserHelpers::setDiffuse(lineStream, materials);

			if (error != objParser::ErrorType::OK) {
				return error;
			}

		} else if (prefix == "Ks") {
			objParser::PtError error = MtlParserHelpers::ensureMaterialExists(materials);

			if (error != objParser::ErrorType::OK) {
				return error;
			}

			error = MtlParserHelpers::setSpecular(lineStream, materials);

			if (error != objParser::ErrorType::OK) {
				return error;
			}

		} else if (prefix == "Ns") {
			objParser::PtError error = MtlParserHelpers::ensureMaterialExists(materials);

			if (error != objParser::ErrorType::OK) {
				return error;
			}

			error = MtlParserHelpers::setSpecularExponent(lineStream, materials);

			if (error != objParser::ErrorType::OK) {
				return error;
			}

		} else if (prefix == "d") {
			objParser::PtError error = MtlParserHelpers::ensureMaterialExists(materials);

			if (error != objParser::ErrorType::OK) {
				return error;
			}

			error = MtlParserHelpers::setInverseTransparent(lineStream, materials);

			if (error != objParser::ErrorType::OK) {
				return error;
			}

		} else if (prefix == "Tr") {
			objParser::PtError error = MtlParserHelpers::ensureMaterialExists(materials);

			if (error != objParser::ErrorType::OK) {
				return error;
			}

			error = MtlParserHelpers::setTransparent(lineStream, materials);

			if (error != objParser::ErrorType::OK) {
				return error;
			}

		} else if (prefix == "Tf") {
			objParser::PtError error = MtlParserHelpers::ensureMaterialExists(materials);

			if (error != objParser::ErrorType::OK) {
				return error;
			}

			error = MtlParserHelpers::setTransmissionFilter(lineStream, materials);

			if (error != objParser::ErrorType::OK) {
				return error;
			}

		} else if (prefix == "Ni") {

			objParser::PtError error = MtlParserHelpers::ensureMaterialExists(materials);

			if (error != objParser::ErrorType::OK) {
				return error;
			}

			error = MtlParserHelpers::setIndexRefraction(lineStream, materials);

			if (error != objParser::ErrorType::OK) {
				return error;
			}

		} else if (prefix == "map_Ka") {

		} else if (prefix == "map_Kd") {

		} else if (prefix == "map_Ks") {

		} else if (prefix == "map_Ns") {

		} else if (prefix == "map_d") {

		} else if (prefix == "map_bump") {

		} else if (prefix == "bump") {

		} else if (prefix == "disp") {

		} else if (prefix == "decal") {

		} else if (prefix == "illum") {

		} else if (prefix == "newmtl") {
			MtlParserHelpers::newMaterial(lineStream, materials);
		}
		
		getline(stream, line);
		lineStream = std::istringstream(line);
	}

	return objParser::ErrorType::OK;
}