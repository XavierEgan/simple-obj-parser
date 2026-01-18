#include "../../include/ObjParser.hpp"
#include "../../include/MtlParser.hpp"


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

	static objParser::Error newVertex(std::istringstream& lineStream, std::vector<glm::vec3>& vertices) {
		// we will ignore w
		float x = 0, y = 0, z = 0, w = 1.0;
		if (!(lineStream >> x >> y >> z)) {
			return objParser::Error(objParser::ErrorType::FileFormatError, "Reading in a vertex failed");
		}
		// read in w, but its not an error if its not there
		lineStream >> w;

		vertices.emplace_back(x / w, y / w, z / w);

		return objParser::ErrorType::OK;
	}

	static objParser::Error newVertexNormal(std::istringstream& lineStream, std::vector<glm::vec3>& vertexNormals) {
		float x = 0, y = 0, z = 0;
		if (!(lineStream >> x >> y >> z)) {
			return objParser::Error(objParser::ErrorType::FileFormatError, "Reading in a vertex normal failed");
		}

		// its not necessarily normalized, so normalize it to make sure
		glm::vec3 vec(x, y, z);
		vec = glm::normalize(vec);

		vertexNormals.emplace_back(vec.x, vec.y, vec.z);

		return objParser::ErrorType::OK;
	}

	static objParser::Error newVertexTexture(std::istringstream& lineStream, std::vector<glm::vec3>& vertexTextureCoordinates) {
		// last two are optional, but default to zero so this should be fine
		float x = 0, y = 0, z = 0;
		if (!(lineStream >> x)) {
			return objParser::Error(objParser::ErrorType::FileFormatError, "Reading in vertex texture (uv) coords failed");
		}

		lineStream >> y >> z;

		vertexTextureCoordinates.emplace_back(x, y, z);

		return objParser::ErrorType::OK;
	}
	
	enum FaceElementType {
		notSet,
		v,
		vvt,
		vvn,
		vvtvn
	};

	static objParser::Error newFace(
		std::istringstream& lineStream, 
		std::vector<objParser::Mesh>& meshs, 
		const std::vector<glm::vec3>& globalVertices, 
		const std::vector<glm::vec3>& globalVertexTextureCoordinates, 
		const std::vector<glm::vec3>& globalVertexNormals
	) {
		// f v1/vt1/vn1 v2/vt2/vn2 v3/vt3/vn3
		std::array<std::string, 3> faces;

		if (!(lineStream >> faces[0] >> faces[1] >> faces[2])) {
			return objParser::Error(objParser::ErrorType::FileFormatError, "Face cant have less that 3 verts.");
		}
		
		std::string temp;
		if (lineStream >> temp) {
			return objParser::Error(objParser::ErrorType::FileFormatError, "Face cant have more that 3 verts. Triangulate your mesh before exporting");
		}

		FaceElementType inputTypeAllVerts = FaceElementType::notSet;

		for (std::string& face : faces) {
			size_t firstSlashIndex = face.find_first_of('/');
			size_t secondSlashIndex = face.find_last_of('/');

			if (firstSlashIndex != std::string::npos) {
				face.at(firstSlashIndex) = ' ';
				face.at(secondSlashIndex) = ' ';
			}

			FaceElementType thisType = FaceElementType::notSet;

			std::istringstream faceStream(face);

			if (firstSlashIndex == std::string::npos) {
				thisType = FaceElementType::v;
			} else  if (firstSlashIndex == secondSlashIndex) {
				thisType = FaceElementType::vvt;
			} else if (firstSlashIndex == secondSlashIndex - 1) {
				thisType = FaceElementType::vvn;
			} else {
				thisType = FaceElementType::vvtvn;
			}

			// check the type is correct
			if (inputTypeAllVerts == FaceElementType::notSet) {
				inputTypeAllVerts = thisType;
			} else if (inputTypeAllVerts != thisType) {
				return objParser::Error(objParser::ErrorType::FileFormatError, "Error reading face, must be all the same type of input (for example, all v//vn)");
			}

			constexpr int NO_INDEX = std::numeric_limits<int>::min();
			int v = NO_INDEX, vt = NO_INDEX, vn = NO_INDEX;
			switch (inputTypeAllVerts) {
				case FaceElementType::v:
					faceStream >> v;
					break;
				case FaceElementType::vvt:
					faceStream >> v >> vt;
					break;
				case FaceElementType::vvn:
					faceStream >> v >> vn;
					break;
				case FaceElementType::vvtvn:
					faceStream >> v >> vt >> vn;
					break;
			}

			if (v != NO_INDEX && (v < 1 || v > globalVertices.size())) {
				return objParser::Error(objParser::ErrorType::FileFormatError, "Vertex index in face is out of range, got " + std::to_string(v) + " but there are only " + std::to_string(globalVertices.size()) + " vertices");
			}
			if (vt != NO_INDEX && (vt < 1 || vt > globalVertexTextureCoordinates.size())) {
				return objParser::Error(objParser::ErrorType::FileFormatError, "Vertex texture coordinate index in face is out of range, got " + std::to_string(vt) + " but there are only " + std::to_string(globalVertexTextureCoordinates.size()) + " vertex texture coordinates");
			}
			if (vn != NO_INDEX && (vn < 1 || vn > globalVertexNormals.size())) {
				return objParser::Error(objParser::ErrorType::FileFormatError, "Vertex normal index in face is out of range, got " + std::to_string(vn) + " but there are only " + std::to_string(globalVertexNormals.size()) + " vertex normals");
			}
			
			objParser::Mesh& mesh = meshs.back();

			if (v != NO_INDEX) {
				mesh.vertices.push_back(globalVertices[v - 1]);
				mesh.vertexIndexes.push_back(static_cast<int>(mesh.vertices.size() - 1));
			}
			if (vt != NO_INDEX) {
				mesh.vertexTextureCoordinates.push_back(globalVertexTextureCoordinates[vt - 1]);
				mesh.vertexTextureCoordinatesIndexes.push_back(static_cast<int>(mesh.vertexTextureCoordinates.size() - 1));
			}
			if (vn != NO_INDEX) {
				mesh.vertexNormals.push_back(globalVertexNormals[vn - 1]);
				mesh.vertexNormalsIndexes.push_back(static_cast<int>(mesh.vertexNormals.size() - 1));
			}
		}

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

	std::vector<glm::vec3> globalVertices;
	std::vector<glm::vec3> globalVertexTextureCoordinates;
	std::vector<glm::vec3> globalVertexNormals;


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
			objParser::Error error = ObjParserHelpers::newVertex(lineStream, globalVertices);

			if (error != objParser::ErrorType::OK) {
				return error;
			}

		} else if (elementType == "vn") {
			// vertex normal
			objParser::Error error = ObjParserHelpers::newVertexNormal(lineStream, globalVertexNormals);

			if (error != objParser::ErrorType::OK) {
				return error;
			}

		} else if (elementType == "vt") {
			// vertex texture
			objParser::Error error = ObjParserHelpers::newVertexTexture(lineStream, globalVertexTextureCoordinates);

			if (error != objParser::ErrorType::OK) {
				return error;
			}

		} else if (elementType == "f") {
			// face
			objParser::Error error = ObjParserHelpers::ensureObjExists(meshs);

			if (error != objParser::ErrorType::OK) {
				return error;
			}

			error = ObjParserHelpers::newFace(lineStream, meshs, globalVertices, globalVertexTextureCoordinates, globalVertexNormals);

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

