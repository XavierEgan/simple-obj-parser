
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
#include <vector>
#include <string>
#include <array>
#include <iostream>
#include <fstream>
#include <sstream>


#include "ext/glm/glm.hpp"

//#include "ext/stb/stb_image.h"
//#include "ext/stb/stb_image_write.h"

#include <filesystem>

namespace objParser {
	objParser::Error parseMtlFile(std::filesystem::path fileName, std::vector<objParser::Material>& materials);
	objParser::Error parseMtlStream(std::istream& stream, const std::filesystem::path& fileName, std::vector<objParser::Material>& materials);
}


#include <cctype>
#include <filesystem>
#include <algorithm>

namespace objParser {
	objParser::Error parseObjFile(std::filesystem::path fileName, std::vector<Mesh>& meshs, std::vector<objParser::Material>& materials); 
	objParser::Error parseObjStream(std::istream& stream, const std::filesystem::path& objPath, std::vector<Mesh>& meshs, std::vector<objParser::Material>& materials);
}
#include <ostream>
#include <istream>
#include <string>

namespace objParser {
	enum ErrorType {
		OK,
		FileFormatError,
		FileNotFound
	};

	std::ostream& operator<<(std::ostream& oss, const objParser::ErrorType& error) noexcept;

	struct Error {
		ErrorType errorType;
		std::string message;

		Error() noexcept;
		Error(const ErrorType& error_t) noexcept;
		Error(ErrorType error_t, std::string message) noexcept;

		bool operator==(const Error& other) const noexcept;
		bool operator!=(const Error& other) const noexcept;

		bool operator==(const ErrorType& other) const noexcept;
		bool operator!=(const ErrorType& other) const noexcept;

		friend std::ostream& operator<<(std::ostream& oss, const Error& error) noexcept;
	};
}

namespace objParser {
	struct Mesh {
		std::vector<glm::vec3> vertices;
		std::vector<glm::vec3> vertexTextureCoordinates;
		std::vector<glm::vec3> vertexNormals;

		std::vector<int> vertexIndexes;
		std::vector<int> vertexTextureCoordinatesIndexes;
		std::vector<int> vertexNormalsIndexes;
		
		size_t mtlIndex;
		std::string name;

		Mesh(std::string name);
	};
}
#if defined(OBJ_PARSER_IMPLEMENTATION)
#include "ObjParser.hpp"
#include "MtlParser.hpp"


namespace ObjParserHelpers {
	static objParser::Error ensureObjExists(std::vector<objParser::Mesh>& meshs) {
		if (meshs.size() == 0) {
			return objParser::Error(objParser::ErrorType::FileFormatError, "Trying to read data before any objects have been defined");
		}

		return objParser::ErrorType::OK;
	}

	static objParser::Error newObject(std::istringstream& lineStream, std::vector<objParser::Mesh>& meshs) {
		std::string name;
		lineStream >> name;
		meshs.emplace_back(name);

		return objParser::ErrorType::OK;
	}

	static objParser::Error newVertex(std::istringstream& lineStream, std::vector<objParser::Mesh>& meshs) {
		// we will ignore w
		float x = 0, y = 0, z = 0, w = 1.0;
		if (!(lineStream >> x >> y >> z)) {
			return objParser::Error(objParser::ErrorType::FileFormatError, "Reading in a vertex failed");
		}
		// read in w, but its not an error if its not there
		lineStream >> w;

		meshs.back().vertices.emplace_back(x / w, y / w, z / w);

		return objParser::ErrorType::OK;
	}

	static objParser::Error newVertexNormal(std::istringstream& lineStream, std::vector<objParser::Mesh>& meshs) {
		float x = 0, y = 0, z = 0;
		if (!(lineStream >> x >> y >> z)) {
			return objParser::Error(objParser::ErrorType::FileFormatError, "Reading in a vertex normal failed");
		}

		// its not necessarily normalized, so normalize it to make sure
		glm::vec3 vec(x, y, z);
		vec = glm::normalize(vec);

		meshs.back().vertexNormals.emplace_back(vec.x, vec.y, vec.z);

		return objParser::ErrorType::OK;
	}

	static objParser::Error newVertexTexture(std::istringstream& lineStream, std::vector<objParser::Mesh>& meshs) {
		// last two are optional, but default to zero so this should be fine
		float x = 0, y = 0, z = 0;
		if (!(lineStream >> x)) {
			return objParser::Error(objParser::ErrorType::FileFormatError, "Reading in vertex texture (uv) coords failed");
		}

		lineStream >> y >> z;

		meshs.back().vertexTextureCoordinates.emplace_back(x, y, z);

		return objParser::ErrorType::OK;
	}
	
	enum FaceElementType {
		notSet,
		v,
		vvt,
		vvn,
		vvtvn
	};

	static objParser::Error newFace(std::istringstream& lineStream, std::vector<objParser::Mesh>& meshs) {
		// f v1/vt1/vn1 v2/vt2/vn2 v3/vt3/vn3
		std::array<std::string, 3> faces;

		if (!(lineStream >> faces[0] >> faces[1] >> faces[2])) {
			return objParser::Error(objParser::ErrorType::FileFormatError, "Must be exactly 3 verts");
		}
		
		std::string temp;
		if (lineStream >> temp) {
			return objParser::Error(objParser::ErrorType::FileFormatError, "Face cant have more that 3 verts. Triangulate your mesh before exporting");
		}

		int typeInput = FaceElementType::notSet;

		std::vector<int> tempVertexIndexes;
		std::vector<int> tempVertexTextureCoordinatesIndexes;
		std::vector<int> tempVertexNormalsIndexes;

		for (std::string face : faces) {
			constexpr int noIndex = 0;
			int v = noIndex, vt = noIndex, vn = noIndex;

			int firstSlashIndex = face.find_first_of('/');
			int secondSlashIndex = face.find_last_of('/');

			if (firstSlashIndex != std::string::npos) {
				face.at(firstSlashIndex) = ' ';
				face.at(secondSlashIndex) = ' ';
			}

			int thisType = FaceElementType::notSet;

			std::istringstream faceStream(face);

			if (firstSlashIndex == std::string::npos) {
				// v
				if (!(faceStream >> v) || v == 0) {
					return objParser::Error(objParser::ErrorType::FileFormatError, "Error reading face, format: v");
				}
				thisType = FaceElementType::v;
			} else  if (firstSlashIndex == secondSlashIndex) {
				// v/vt
				if (!(faceStream >> v >> vt) || v == 0 || vt == 0) {
					return objParser::Error(objParser::ErrorType::FileFormatError, "Error reading face, format: v/vt");
				}
				thisType = FaceElementType::vvt;
			} else if (firstSlashIndex == secondSlashIndex - 1) {
				// v//vn
				if (!(faceStream >> v >> vn) || v == 0 || vn == 0) {
					return objParser::Error(objParser::ErrorType::FileFormatError, "Error reading face, format: v//vn");
				}
				thisType = FaceElementType::vvn;
			} else {
				// v/vt/vn
				if (!(faceStream >> v >> vt >> vn) || v == 0 || vt == 0 || vn == 0) {
					return objParser::Error(objParser::ErrorType::FileFormatError, "Error reading face, format: v/vt/vn");
				}
				thisType = FaceElementType::vvtvn;
			}

			if (typeInput == FaceElementType::notSet) {
				typeInput = thisType;
			} else if (typeInput != thisType) {
				return objParser::Error(objParser::ErrorType::FileFormatError, "Error reading face, must be all the same type of input (for example, all v//vn)");
			}
			
			if (v != noIndex) {
				if (v < 0) {
					v = meshs.back().vertices.size() + v;
				} else {
					v -= 1;
				}

				if (v > (int)meshs.back().vertices.size() - 1) {
					std::ostringstream oss;
					oss << "Vertex '" << vt << "' out of range. Expected less than '" << meshs.back().vertexNormals.size();
					return objParser::Error(objParser::ErrorType::FileFormatError, oss.str());
				}

				tempVertexIndexes.push_back(v);
			}

			if (vt != noIndex) {
				if (vt < 0) {
					vt = meshs.back().vertexTextureCoordinates.size() + vt;
				} else {
					vt -= 1;
				}

				if (vt > (int)meshs.back().vertices.size() - 1) {
					std::ostringstream oss;
					oss << "Vertex Texture '" << vt << "' out of range. Expected less than '" << meshs.back().vertexNormals.size();
					return objParser::Error(objParser::ErrorType::FileFormatError, oss.str());
				}

				tempVertexTextureCoordinatesIndexes.push_back(vt);
			}

			if (vn != noIndex) {
				if (vn < 0) {
					vn = meshs.back().vertexTextureCoordinates.size() + vn;
				} else {
					vn -= 1;
				}

				if (vn > (int)meshs.back().vertices.size() - 1) {
					std::ostringstream oss;
					oss << "Vertex Normal '" << vt << "' out of range. Expected less than '" << meshs.back().vertexNormals.size();
					return objParser::Error(objParser::ErrorType::FileFormatError, oss.str());
				}

				tempVertexNormalsIndexes.push_back(vn);
			}
		}

		meshs.back().vertexIndexes.insert(meshs.back().vertexIndexes.end(), tempVertexIndexes.begin(), tempVertexIndexes.end());
		meshs.back().vertexTextureCoordinatesIndexes.insert(meshs.back().vertexTextureCoordinatesIndexes.end(), tempVertexTextureCoordinatesIndexes.begin(), tempVertexTextureCoordinatesIndexes.end());
		meshs.back().vertexNormalsIndexes.insert(meshs.back().vertexNormalsIndexes.end(), tempVertexNormalsIndexes.begin(), tempVertexNormalsIndexes.end());

		return objParser::ErrorType::OK;
	}

	static inline objParser::Error setMaterial(std::istringstream& lineStream, std::vector<objParser::Mesh>& meshs, std::vector<objParser::Material>& materials) {
		std::string materialName;
		lineStream >> materialName;
		
		auto materialNameMatches = [materialName](objParser::Material mat) -> bool {
			return materialName == mat.name;
		};
		
		

		if (auto matIterator = std::ranges::find_if(materials, materialNameMatches); matIterator != materials.end()) {
			meshs.back().mtlIndex = matIterator - materials.begin();

			return objParser::ErrorType::OK;
		} else {
			return objParser::Error(objParser::ErrorType::FileFormatError, "Material '" + materialName + "' not found");
		}
	}

	static inline objParser::Error linkMtlFile(std::istringstream& lineStream, const std::filesystem::path& objFilePath, std::vector<objParser::Material>& materials) {
		std::string mtlFileName;
		lineStream >> mtlFileName;

		std::filesystem::path mtlFilePath = objFilePath / mtlFileName;

		objParser::Error error = objParser::parseMtlFile(mtlFilePath, materials);

		return error;
	}
}

objParser::Error objParser::parseObjFile(std::filesystem::path fileName, std::vector<objParser::Mesh>& meshs, std::vector<objParser::Material>& materials) {
	std::ifstream inFS(fileName);

	if (!inFS.is_open() || !inFS.good()) {
		std::ostringstream errorStream;
		errorStream << "could not find file '" << fileName << "'";
		return objParser::Error(objParser::ErrorType::FileNotFound, errorStream.str());
	}

	objParser::Error error = parseObjStream(inFS, fileName.parent_path(), meshs, materials);

	return error;
}

objParser::Error objParser::parseObjStream(std::istream& stream, const std::filesystem::path &objFilePath, std::vector<objParser::Mesh>& meshs, std::vector<objParser::Material>& materials) {
	std::string line;
	getline(stream, line);

	std::istringstream lineStream(line);

	while (!stream.fail()) {
		std::string elementType;
		lineStream >> elementType;

		if (elementType == "o") {
			// new object
			objParser::Error error = ObjParserHelpers::newObject(lineStream, meshs);
			
			if (error != objParser::ErrorType::OK) {
				return error;
			}

		} else if (elementType == "v") {
			// vertex
			objParser::Error error = ObjParserHelpers::ensureObjExists(meshs);

			if (error != objParser::ErrorType::OK) {
				return error;
			}

			error = ObjParserHelpers::newVertex(lineStream, meshs);

			if (error != objParser::ErrorType::OK) {
				return error;
			}

		} else if (elementType == "vn") {
			// vertex normal
			objParser::Error error = ObjParserHelpers::ensureObjExists(meshs);

			if (error != objParser::ErrorType::OK) {
				return error;
			}

			error = ObjParserHelpers::newVertexNormal(lineStream, meshs);

			if (error != objParser::ErrorType::OK) {
				return error;
			}

		} else if (elementType == "vt") {
			// vertex texture
			objParser::Error error = ObjParserHelpers::ensureObjExists(meshs);

			if (error != objParser::ErrorType::OK) {
				return error;
			}

			error = ObjParserHelpers::newVertexTexture(lineStream, meshs);

			if (error != objParser::ErrorType::OK) {
				return error;
			}

		} else if (elementType == "f") {
			// face
			objParser::Error error = ObjParserHelpers::ensureObjExists(meshs);

			if (error != objParser::ErrorType::OK) {
				return error;
			}

			error = ObjParserHelpers::newFace(lineStream, meshs);

			if (error != objParser::ErrorType::OK) {
				return error;
			}

		} else if (elementType == "usemtl") {
			objParser::Error error = ObjParserHelpers::setMaterial(lineStream, meshs, materials);

			if (error != objParser::ErrorType::OK) {
				return error;
			}

		} else if (elementType == "mtllib") {
			objParser::Error error = ObjParserHelpers::linkMtlFile(lineStream, objFilePath, materials);

			if (error != objParser::ErrorType::OK) {
				return error;
			}

		} else if (elementType == "#") {
			
		} else if (elementType == "") {

		} else {
			std::ostringstream oss;
			oss << "Unexpected Line start '" << elementType << "'" << std::endl;
			return objParser::Error(objParser::ErrorType::FileFormatError, oss.str());
		}

		getline(stream, line);
		lineStream = std::istringstream(line);
	}

	return objParser::ErrorType::OK;
}


#include "ObjParserError.hpp"

/*
 * The error works like this:
 * Error contains a ErrorType and a string describing the error
 * ErrorType has an implicit conversion to Error with an empty string
 *     This means you can return ErrorType::{some error} and it will convert
 * Error can be compared and only the ErrorType will be compared
 *     This means that ptError(OK, "all good") == ptError(OK, "") will return true
 */

objParser::Error::Error() noexcept : errorType(objParser::ErrorType::OK) {}
objParser::Error::Error(const objParser::ErrorType& errorType) noexcept : errorType(errorType) {}
objParser::Error::Error(objParser::ErrorType errorType, std::string message) noexcept : errorType(errorType), message(message) {}

bool objParser::Error::operator==(const objParser::Error& other) const noexcept {
	return errorType == other.errorType;
}

bool objParser::Error::operator!=(const objParser::Error& other) const noexcept {
	return errorType != other.errorType;
}

bool objParser::Error::operator==(const objParser::ErrorType& other) const noexcept {
	return errorType == other;
}

bool objParser::Error::operator!=(const objParser::ErrorType& other) const noexcept {
	return errorType != other;
}

std::ostream& objParser::operator<<(std::ostream& oss, const objParser::ErrorType& error) noexcept {
	switch (error) {
	case(objParser::ErrorType::OK):
		oss << "OK";
		break;
	case(objParser::ErrorType::FileFormatError):
		oss << "FileFormatError";
		break;
	case(objParser::ErrorType::FileNotFound):
		oss << "FileNotFound";
		break;
	default:
		break;
	}

	return oss;
}

std::ostream& objParser::operator<<(std::ostream& oss, const objParser::Error& error) noexcept {
	oss << "Error: " << error.errorType << " - Message: " << error.message << std::endl;
	return oss;
} 
#include "CommonInclude.hpp"
#include "MtlParser.hpp"

namespace MtlParserHelpers {
	static objParser::Error ensureMaterialExists(const std::vector<objParser::Material>& materials) {
		if (materials.size() == 0) {
			return objParser::Error(objParser::ErrorType::FileFormatError, "Trying to read data before any meshs have been defined");
		}
		return objParser::ErrorType::OK;
	}

	static objParser::Error newMaterial(std::istream& lineStream, std::vector<objParser::Material>& materials) {
		std::string materialName;
		lineStream >> materialName;

		materials.emplace_back(materialName);

		return objParser::ErrorType::OK;
	}

	static objParser::Error setAmbient(std::istream& lineStream, std::vector<objParser::Material>& materials) {
		float x, y, z;

		if (!(lineStream >> x >> y >> z)) {
			return objParser::Error(objParser::ErrorType::FileFormatError, "Reading in ambient failed");
		}

		// range check
		if (x < 0.0f || y < 0.0f || z < 0.0f) {
			return objParser::Error(objParser::ErrorType::FileFormatError, "Value of ambient is less than 0");
		}
		if (x > 1.0f || y > 1.0f || z > 1.0f) {
			return objParser::Error(objParser::ErrorType::FileFormatError, "Value of ambient is greater than 1");
		}

		materials.back().ambientColor = glm::vec3(x, y, z);

		return objParser::ErrorType::OK;
	}

	static objParser::Error setDiffuse(std::istream& lineStream, std::vector<objParser::Material>& materials) {
		float x, y, z;

		if (!(lineStream >> x >> y >> z)) {
			return objParser::Error(objParser::ErrorType::FileFormatError, "Reading in diffuse failed");
		}
		
		// range check
		if (x < 0.0f || y < 0.0f || z < 0.0f) {
			return objParser::Error(objParser::ErrorType::FileFormatError, "Value of diffuse is less than 0");
		}
		if (x > 1.0f || y > 1.0f || z > 1.0f) {
			return objParser::Error(objParser::ErrorType::FileFormatError, "Value of diffuse is greater than 1");
		}

		materials.back().diffuseColor = glm::vec3(x, y, z);

		return objParser::ErrorType::OK;
	}

	static objParser::Error setSpecular(std::istream& lineStream, std::vector<objParser::Material>& materials) {
		float x, y, z;

		if (!(lineStream >> x >> y >> z)) {
			return objParser::Error(objParser::ErrorType::FileFormatError, "Reading in specular failed");
		}

		// range check
		if (x < 0.0f || y < 0.0f || z < 0.0f) {
			return objParser::Error(objParser::ErrorType::FileFormatError, "Value of specular is less than 0");
		}
		if (x > 1.0f || y > 1.0f || z > 1.0f) {
			return objParser::Error(objParser::ErrorType::FileFormatError, "Value of specular is greater than 1");
		}

		materials.back().specularColor = glm::vec3(x, y, z);

		return objParser::ErrorType::OK;
	}

	static objParser::Error setSpecularExponent(std::istream& lineStream, std::vector<objParser::Material>& materials) {
		float x;

		if (!(lineStream >> x)) {
			return objParser::Error(objParser::ErrorType::FileFormatError, "Reading in specular exponent failed");
		}

		// range check
		if (x < 0.0f) {
			return objParser::Error(objParser::ErrorType::FileFormatError, "Value of specular exponent less than 0");
		}
		if (x > 1000.0f) {
			return objParser::Error(objParser::ErrorType::FileFormatError, "Value of specular exponent greater than 1000");
		}

		materials.back().specularExponent = x;

		return objParser::ErrorType::OK;
	}

	static objParser::Error setTransparent(std::istream& lineStream, std::vector<objParser::Material>& materials) {
		float x;

		if (!(lineStream >> x)) {
			return objParser::Error(objParser::ErrorType::FileFormatError, "Reading in transparent failed");
		}

		// range check
		if (x < 0.0f) {
			return objParser::Error(objParser::ErrorType::FileFormatError, "Value of transparent less than 0");
		}
		if (x > 1.0f) {
			return objParser::Error(objParser::ErrorType::FileFormatError, "Value of transparent greater than 1");
		}

		materials.back().transparent = x;

		return objParser::ErrorType::OK;
	}

	static objParser::Error setInverseTransparent(std::istream& lineStream, std::vector<objParser::Material>& materials) {
		float x;

		if (!(lineStream >> x)) {
			return objParser::Error(objParser::ErrorType::FileFormatError, "Reading in transparent failed");
		}
		
		// since its inverse
		x = 1 - x;

		// range check
		if (x < 0.0f) {
			return objParser::Error(objParser::ErrorType::FileFormatError, "Value of transparent less than 0");
		}
		if (x > 1.0f) {
			return objParser::Error(objParser::ErrorType::FileFormatError, "Value of transparent greater than 1");
		}

		materials.back().transparent = x;

		return objParser::ErrorType::OK;
	}

	static objParser::Error setTransmissionFilter(std::istream& lineStream, std::vector<objParser::Material>& materials) {
		float x, y, z;

		if (!(lineStream >> x >> y >> z)) {
			return objParser::Error(objParser::ErrorType::FileFormatError, "Reading in transmission filter failed");
		}

		// range check
		if (x < 0.0f || y < 0.0f || z < 0.0f) {
			return objParser::Error(objParser::ErrorType::FileFormatError, "Value of transmission filter is less than 0");
		}
		if (x > 1.0f || y > 1.0f || z > 1.0f) {
			return objParser::Error(objParser::ErrorType::FileFormatError, "Value of transmission filter is greater than 1");
		}

		materials.back().transmissionFilter = glm::vec3(x, y, z);

		return objParser::ErrorType::OK;
	}

	static objParser::Error setIndexRefraction(std::istream& lineStream, std::vector<objParser::Material>& materials) {
		float x;

		if (!(lineStream >> x)) {
			return objParser::Error(objParser::ErrorType::FileFormatError, "Reading in optical density/index of refraction failed");
		}

		// range check
		if (x < 0.001f) {
			return objParser::Error(objParser::ErrorType::FileFormatError, "Value of index of recfraction less than 0.001");
		}
		if (x > 10.0f) {
			return objParser::Error(objParser::ErrorType::FileFormatError, "Value of index of recfraction greater than 10.0");
		}

		materials.back().indexOfRefraction = x;

		return objParser::ErrorType::OK;
	}
}

objParser::Error objParser::parseMtlFile(std::filesystem::path fileName, std::vector<objParser::Material>& materials) {
	std::ifstream inFS(fileName);

	if (!inFS.is_open() || !inFS.good()) {
		std::ostringstream errorStream;
		errorStream << "error reading material file '" << fileName << "'";
		return objParser::Error(objParser::ErrorType::FileNotFound, errorStream.str());
	}

	objParser::Error error = objParser::parseMtlStream(inFS, fileName, materials);

	return error;
}

objParser::Error objParser::parseMtlStream(std::istream& stream, const std::filesystem::path& fileName, std::vector<objParser::Material>& materials) {
	std::string line;
	std::getline(stream, line);

	std::istringstream lineStream(line);

	while (!stream.fail()) {
		std::string prefix;
		lineStream >> prefix;

		if (prefix == "Ka") {
			objParser::Error error = MtlParserHelpers::ensureMaterialExists(materials);

			if (error != objParser::ErrorType::OK) {
				return error;
			}

			error = MtlParserHelpers::setAmbient(lineStream, materials);

			if (error != objParser::ErrorType::OK) {
				return error;
			}
		} else if (prefix == "Kd") {
			objParser::Error error = MtlParserHelpers::ensureMaterialExists(materials);

			if (error != objParser::ErrorType::OK) {
				return error;
			}

			error = MtlParserHelpers::setDiffuse(lineStream, materials);

			if (error != objParser::ErrorType::OK) {
				return error;
			}

		} else if (prefix == "Ks") {
			objParser::Error error = MtlParserHelpers::ensureMaterialExists(materials);

			if (error != objParser::ErrorType::OK) {
				return error;
			}

			error = MtlParserHelpers::setSpecular(lineStream, materials);

			if (error != objParser::ErrorType::OK) {
				return error;
			}

		} else if (prefix == "Ns") {
			objParser::Error error = MtlParserHelpers::ensureMaterialExists(materials);

			if (error != objParser::ErrorType::OK) {
				return error;
			}

			error = MtlParserHelpers::setSpecularExponent(lineStream, materials);

			if (error != objParser::ErrorType::OK) {
				return error;
			}

		} else if (prefix == "d") {
			objParser::Error error = MtlParserHelpers::ensureMaterialExists(materials);

			if (error != objParser::ErrorType::OK) {
				return error;
			}

			error = MtlParserHelpers::setInverseTransparent(lineStream, materials);

			if (error != objParser::ErrorType::OK) {
				return error;
			}

		} else if (prefix == "Tr") {
			objParser::Error error = MtlParserHelpers::ensureMaterialExists(materials);

			if (error != objParser::ErrorType::OK) {
				return error;
			}

			error = MtlParserHelpers::setTransparent(lineStream, materials);

			if (error != objParser::ErrorType::OK) {
				return error;
			}

		} else if (prefix == "Tf") {
			objParser::Error error = MtlParserHelpers::ensureMaterialExists(materials);

			if (error != objParser::ErrorType::OK) {
				return error;
			}

			error = MtlParserHelpers::setTransmissionFilter(lineStream, materials);

			if (error != objParser::ErrorType::OK) {
				return error;
			}

		} else if (prefix == "Ni") {

			objParser::Error error = MtlParserHelpers::ensureMaterialExists(materials);

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
#include "Mesh.hpp"

objParser::Mesh::Mesh(std::string name) : name(name) {}
#include "Material.hpp"

objParser::Material::Material(const std::string& name) : name(name) {} 

#endif
