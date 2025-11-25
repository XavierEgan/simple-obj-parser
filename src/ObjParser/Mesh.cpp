#include "Mesh.hpp"

pt::Mesh::Mesh(std::string name) : name(name) {}

bool pt::Mesh::operator==(const Mesh& other) const {
	return vertices == other.vertices &&
		vertexTextureCoordinates == other.vertexTextureCoordinates &&
		vertexNormals == other.vertexNormals &&
		vertexIndexes == other.vertexIndexes &&
		vertexTextureCoordinatesIndexes == other.vertexTextureCoordinatesIndexes &&
		vertexNormalsIndexes == other.vertexNormalsIndexes;
}

bool pt::Mesh::operator!=(const Mesh& other) const {
	return vertices != other.vertices &&
		vertexTextureCoordinates != other.vertexTextureCoordinates &&
		vertexNormals != other.vertexNormals &&
		vertexIndexes != other.vertexIndexes &&
		vertexTextureCoordinatesIndexes != other.vertexTextureCoordinatesIndexes &&
		vertexNormalsIndexes != other.vertexNormalsIndexes;
}