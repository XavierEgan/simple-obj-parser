#include <gtest/gtest.h>
#include <sstream>
#include <filesystem>

#include "../release/obj_parser.hpp"

TEST(ObjParserReadsFile, rejectsWrongFilePath) {
	std::vector<objParser::Mesh> meshs;
	std::vector<objParser::Material> materials;
	objParser::Error error = objParser::parseObjFile("", meshs, materials);

	ASSERT_EQ(error, objParser::ErrorType::FileNotFound);
}

TEST(ObjParserReadsFile, readsBasicFile) {
	std::vector<objParser::Mesh> meshs;
	std::vector<objParser::Material> materials;

	objParser::Error error = objParser::parseObjFile("../tests/TestAssets/objTest1.obj", meshs, materials);
	
	ASSERT_EQ(error, objParser::ErrorType::OK);
	ASSERT_EQ(meshs.size(), 1);

	EXPECT_EQ(meshs.at(0).name, "t");
	EXPECT_EQ(meshs.at(0).vertexIndexes, std::vector<int>({ 0,0,2,1,2,1 }));
	EXPECT_EQ(meshs.at(0).vertexTextureCoordinatesIndexes, std::vector<int>({ 0,1,1,2,0,2 }));
	EXPECT_EQ(meshs.at(0).vertexNormalsIndexes, std::vector<int>({ 0,2,0,1,1,0 }));
	
	EXPECT_EQ(meshs.at(0).vertices, std::vector<glm::vec3>({ { 1,2,3 }, { 4,5,6 }, { 7,8,9 } }));
	EXPECT_EQ(meshs.at(0).vertexTextureCoordinates, std::vector<glm::vec3>({ { 1,0,.5 }, { .5,0,1 }, { 1,0,.5 } }));
	EXPECT_EQ(meshs.at(0).vertexNormals, std::vector<glm::vec3>({ { 0,1,0 }, { 1,0,0 }, { 0,0,1 } }));
}