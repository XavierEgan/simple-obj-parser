#include <gtest/gtest.h>
#include <sstream>

#include "MtlParser.hpp"
#include "ObjParser.hpp"
#include "TestHelpers.hpp"

TEST(MtlandObjIntegrationTests, rejectsWrongMtlFilePath) {
	std::vector<objParser::Mesh> meshs;
	std::vector<objParser::Material> materials;

	objParser::Error error = objParser::parseObjFile("../../../tests/TestAssets/objTest2.obj", meshs, materials);

	ASSERT_EQ(error, objParser::ErrorType::FileNotFound);
}

TEST(MtlandObjIntegrationTests, linksMtlFile) {
	std::vector<objParser::Mesh> meshs;
	std::vector<objParser::Material> materials;

	objParser::Error error = objParser::parseObjFile("../../../tests/TestAssets/objTest3.obj", meshs, materials);

	ASSERT_EQ(error, objParser::ErrorType::OK);
	ASSERT_EQ(meshs.size(), 1);
	ASSERT_EQ(materials.size(), 2);

	EXPECT_EQ(meshs.at(0).mtlIndex, 1);
}