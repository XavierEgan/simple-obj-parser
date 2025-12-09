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
		
		auto material = std::ranges::find_if(materials, materialNameMatches);


		
		return objParser::ErrorType::OK;
	}

	static inline objParser::Error linkMtlFile(std::istringstream& lineStream, const std::filesystem::path& objFileName, std::vector<objParser::Material>& materials) {
		std::string mtlFileName;
		lineStream >> mtlFileName;

		std::filesystem::path mtlFilePath = objFileName / mtlFileName;

		objParser::parseMtlFile(mtlFilePath, materials);

		return objParser::ErrorType::OK;
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

