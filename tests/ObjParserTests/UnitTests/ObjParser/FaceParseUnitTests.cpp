#include <gtest/gtest.h>
#include <sstream>

#include "ObjParser.hpp"
#include "TestHelpers.hpp"

struct FaceParseCase {
	std::string objFileContents;
	bool makeFilledMeshs;

	int expectedLenVertexIndices;
	int expectedLenVertexTextureIndices;
	int expectedLenVertexNormalIndices;

	std::vector<int> expectedVertexIndices;
	std::vector<int> expectedVertexTextureIndices;
	std::vector<int> expectedVertexNormalIndices;

	objParser::ErrorType expectedError = objParser::ErrorType::OK;
	

	friend std::ostream& operator<<(std::ostream& os, const FaceParseCase& pc) {
		os << "FaceParseCase struct" << std::endl;
		return os;
	}
};

class FaceParseTestFixture : public ::testing::TestWithParam<FaceParseCase> {
protected:
	std::istringstream testStream;
	std::vector<objParser::Mesh> meshs;
	std::vector<objParser::Material> materials;
	objParser::Error error;
public:
	FaceParseTestFixture() {
		const FaceParseCase& testCase = GetParam();

		// make a filled mesh vector manually so were ONLY testing faces and not verts/object creation etc
		if (testCase.makeFilledMeshs) {
			objParser::Mesh testMesh("t");
			testMesh.vertices.push_back(glm::vec3(1.0f, 1.0f, 1.0f));
			testMesh.vertices.push_back(glm::vec3(2.0f, 2.0f, 2.0f));

			testMesh.vertexNormals.push_back(glm::vec3(1.1f, 1.1f, 1.1f));
			testMesh.vertexNormals.push_back(glm::vec3(2.1f, 2.1f, 2.1f));

			testMesh.vertexTextureCoordinates.push_back(glm::vec3(1.2f, 1.2f, 1.2f));
			testMesh.vertexTextureCoordinates.push_back(glm::vec3(2.2f, 2.2f, 2.2f));

			meshs.push_back(testMesh);
		}

		testStream = std::istringstream(testCase.objFileContents);
	}
};

TEST_P(FaceParseTestFixture, ParsesFace) {
	const FaceParseCase& testCase = GetParam();

	error = objParser::ObjParser::parseStream(testStream, meshs, materials);

	if (testCase.expectedError != objParser::ErrorType::OK) {
		EXPECT_EQ(error, testCase.expectedError);

		if (meshs.size() > 0) {
			EXPECT_EQ(meshs.at(0).vertexIndexes.size(), testCase.expectedLenVertexIndices);
			EXPECT_EQ(meshs.at(0).vertexNormalsIndexes.size(), testCase.expectedLenVertexNormalIndices);
			EXPECT_EQ(meshs.at(0).vertexTextureCoordinatesIndexes.size(), testCase.expectedLenVertexTextureIndices);

			EXPECT_EQ(meshs.at(0).vertexIndexes, testCase.expectedVertexIndices);
			EXPECT_EQ(meshs.at(0).vertexNormalsIndexes, testCase.expectedVertexNormalIndices);
			EXPECT_EQ(meshs.at(0).vertexTextureCoordinatesIndexes, testCase.expectedVertexTextureIndices);
		}
		
		return;
	}

	ASSERT_EQ(error, objParser::ErrorType::OK);

	ASSERT_NE(meshs.size(), 0);

	EXPECT_EQ(meshs.at(0).vertexIndexes.size(), testCase.expectedLenVertexIndices);
	EXPECT_EQ(meshs.at(0).vertexNormalsIndexes.size(), testCase.expectedLenVertexNormalIndices);
	EXPECT_EQ(meshs.at(0).vertexTextureCoordinatesIndexes.size(), testCase.expectedLenVertexTextureIndices);

	EXPECT_EQ(meshs.at(0).vertexIndexes, testCase.expectedVertexIndices);
	EXPECT_EQ(meshs.at(0).vertexNormalsIndexes, testCase.expectedVertexNormalIndices);
	EXPECT_EQ(meshs.at(0).vertexTextureCoordinatesIndexes, testCase.expectedVertexTextureIndices);
}

INSTANTIATE_TEST_SUITE_P(
	ObjParser,
	FaceParseTestFixture,
	::testing::Values(
		/*
		FaceParseCase{ objFileContents, 
			makeFilledMeshs, shouldBeOk, 
			expectedLenVertexIndices, expectedLenVertexTextureIndices, expectedLenVertexNormalIndices, 
			expectedVertexIndices, expectedVertexTe9xtureIndices, expectedVertexNormalIndices, 
			expectedError }
		*/
		// NOTE for these testcases, its assumed that 2 verts, normals and texture coords have been defined (meaning 3 is out of range)
		FaceParseCase{ "f 1 1 1",				true, 3, 0, 0, { 0, 0, 0 }, {},			{}			},	// ACCEPTS verts
		FaceParseCase{ "f 1/1 1/1 1/1",			true, 3, 3, 0, { 0, 0, 0 }, { 0, 0, 0 },	{}			},	// ACCEPTS verts and vert texture
		FaceParseCase{ "f 1//1 1//1 1//1",		true, 3, 0, 3, { 0, 0, 0 }, {},			{ 0, 0, 0 } },	// ACCEPTS verts and vert normals
		FaceParseCase{ "f 1/1/1 1/1/1 1/1/1",	true, 3, 3, 3, { 0, 0, 0 }, { 0, 0, 0 },	{ 0, 0, 0 } },	// ACCEPTS verts, vert texture and vert normals
		FaceParseCase{ "f 1 2 2",				true, 3, 0, 0, { 0, 1, 1 }, {},			{} },			// ACCEPTS verts
		FaceParseCase{ "f 1/2 2/1 2/1",			true, 3, 3, 0, { 0, 1, 1 }, { 1, 0, 0 },	{} },			// ACCEPTS verts and vert texture
		FaceParseCase{ "f 2//1 2//1 2//1",		true, 3, 0, 3, { 1, 1, 1 }, {},			{ 0, 0, 0 } },	// ACCEPTS verts and vert normals
		FaceParseCase{ "f 1/2/1 1/2/1 2/1/2",	true, 3, 3, 3, { 0, 0, 1 }, { 1, 1, 0 },	{ 0, 0, 1 } },	// ACCEPTS verts, vert texture and vert normals

		FaceParseCase{ "f -1 -1 -1",			true, 3, 0, 0, { 1, 1, 1 }, {},	{} },	// ACCEPTS negatives
		FaceParseCase{ "f -2 -2 -2",			true, 3, 0, 0, { 0, 0, 0 }, {},	{} },	// ACCEPTS negatives

		FaceParseCase{ "f 3 3 3",				true, 0, 0, 0, {}, {}, {}, objParser::ErrorType::FileFormatError },	// REJECTS verts, index out of range
		FaceParseCase{ "f 3/3 3/3 3/3",			true, 0, 0, 0, {}, {}, {}, objParser::ErrorType::FileFormatError },	// REJECTS verts and vert texture, index out of range
		FaceParseCase{ "f 3//3 3//3 3//3",		true, 0, 0, 0, {}, {}, {}, objParser::ErrorType::FileFormatError },	// REJECTS verts and vert normals, index out of range
		FaceParseCase{ "f 3/3/3 3/3/3 3/3/3",	true, 0, 0, 0, {}, {}, {}, objParser::ErrorType::FileFormatError },	// REJECTS verts and vert texture and vert normals, index out of range
		FaceParseCase{ "f 3 1 2",				true, 0, 0, 0, {}, {}, {}, objParser::ErrorType::FileFormatError },	// REJECTS verts, index out of range with some correct values (this actually caught a bug lmao)
		FaceParseCase{ "f 3/2 1/3 2/3",			true, 0, 0, 0, {}, {}, {}, objParser::ErrorType::FileFormatError },	// REJECTS verts and vert texture, index out of range with some correct values
		FaceParseCase{ "f 2//3 3//1 2//3",		true, 0, 0, 0, {}, {}, {}, objParser::ErrorType::FileFormatError },	// REJECTS verts and vert normals, index out of range with some correct values
		FaceParseCase{ "f 1/2/3 3/1/2 2/3/1",	true, 0, 0, 0, {}, {}, {}, objParser::ErrorType::FileFormatError },	// REJECTS verts and vert texture and vert normals, index out of range with some correct values

		FaceParseCase{ "f 0 0 0",				true, 0, 0, 0, {}, {}, {}, objParser::ErrorType::FileFormatError },	// REJECTS verts, index equal to zero
		FaceParseCase{ "f 0/0 0/0 0/0",			true, 0, 0, 0, {}, {}, {}, objParser::ErrorType::FileFormatError },	// REJECTS verts, index equal to zero
		FaceParseCase{ "f 0//0 0//0 0//0",		true, 0, 0, 0, {}, {}, {}, objParser::ErrorType::FileFormatError },	// REJECTS verts, index equal to zero
		FaceParseCase{ "f 0/0/0 0/0/0 0/0/0",	true, 0, 0, 0, {}, {}, {}, objParser::ErrorType::FileFormatError },	// REJECTS verts, index equal to zero
		FaceParseCase{ "f 0 1 2",				true, 0, 0, 0, {}, {}, {}, objParser::ErrorType::FileFormatError },	// REJECTS verts, index equal to zero with some correct value
		FaceParseCase{ "f 0/2 0/1 2/0",			true, 0, 0, 0, {}, {}, {}, objParser::ErrorType::FileFormatError },	// REJECTS verts, index equal to zero with some correct value
		FaceParseCase{ "f 2//0 0//1 0//2",		true, 0, 0, 0, {}, {}, {}, objParser::ErrorType::FileFormatError },	// REJECTS verts, index equal to zero with some correct value
		FaceParseCase{ "f 2/0/1 0/1/0 0/0/2",	true, 0, 0, 0, {}, {}, {}, objParser::ErrorType::FileFormatError },	// REJECTS verts, index equal to zero with some correct value

		FaceParseCase{ "f a 1 1",				true, 0, 0, 0, {}, {}, {}, objParser::ErrorType::FileFormatError },	// REJECTS verts, not an int
		FaceParseCase{ "f 1/1 b/1 1/1",			true, 0, 0, 0, {}, {}, {}, objParser::ErrorType::FileFormatError },	// REJECTS verts, not an int
		FaceParseCase{ "f a//1 1//1 1//1",		true, 0, 0, 0, {}, {}, {}, objParser::ErrorType::FileFormatError },	// REJECTS verts, not an int
		FaceParseCase{ "f 1/1/1 1/1/1 #/1/1",	true, 0, 0, 0, {}, {}, {}, objParser::ErrorType::FileFormatError },	// REJECTS verts, not an int

		FaceParseCase{ "f 1 1/1 1",				true, 0, 0, 0, {}, {}, {}, objParser::ErrorType::FileFormatError },	// REJECTS verts, face types are not the same
		FaceParseCase{ "f 1/1 1/1/2 1",			true, 0, 0, 0, {}, {}, {}, objParser::ErrorType::FileFormatError },	// REJECTS verts, face types are not the same
		FaceParseCase{ "f 1//1 1/1/1 1//1",		true, 0, 0, 0, {}, {}, {}, objParser::ErrorType::FileFormatError },	// REJECTS verts, face types are not the same
		FaceParseCase{ "f 1/1/1 1/1/1 1/1",		true, 0, 0, 0, {}, {}, {}, objParser::ErrorType::FileFormatError },	// REJECTS verts, face types are not the same
		
		FaceParseCase{ "f 1 1",					true, 0, 0, 0, {}, {}, {}, objParser::ErrorType::FileFormatError },	// REJECTS verts, not enough verts
		FaceParseCase{ "f 1/1 1/1",				true, 0, 0, 0, {}, {}, {}, objParser::ErrorType::FileFormatError },	// REJECTS verts, not enough verts
		FaceParseCase{ "f 1//1 1//1",			true, 0, 0, 0, {}, {}, {}, objParser::ErrorType::FileFormatError },	// REJECTS verts, not enough verts
		FaceParseCase{ "f 1/1/1 1/1/1",			true, 0, 0, 0, {}, {}, {}, objParser::ErrorType::FileFormatError },	// REJECTS verts, not enough verts

		FaceParseCase{ "f 1 1 1 1",						true, 0, 0, 0, {}, {}, {}, objParser::ErrorType::FileFormatError },	// REJECTS verts, too many verts
		FaceParseCase{ "f 1/1 1/1 1/1 1/1",				true, 0, 0, 0, {}, {}, {}, objParser::ErrorType::FileFormatError },	// REJECTS verts, too many verts
		FaceParseCase{ "f 1//1 1//1 1//1 1//1",			true, 0, 0, 0, {}, {}, {}, objParser::ErrorType::FileFormatError },	// REJECTS verts, too many verts
		FaceParseCase{ "f 1/1/1 1/1/1 1/1/1 1/1/1",		true, 0, 0, 0, {}, {}, {}, objParser::ErrorType::FileFormatError },	// REJECTS verts, too many verts

		FaceParseCase{ "f 1 1 1",					false, 0, 0, 0, {}, {}, {}, objParser::ErrorType::FileFormatError },	// REJECTS there are no meshs
		FaceParseCase{ "f 1/1 1/1 1/1",				false, 0, 0, 0, {}, {}, {}, objParser::ErrorType::FileFormatError },	// REJECTS there are no meshs
		FaceParseCase{ "f 1//1 1//1 1//1",			false, 0, 0, 0, {}, {}, {}, objParser::ErrorType::FileFormatError },	// REJECTS there are no meshs
		FaceParseCase{ "f 1/1/1 1/1/1 1/1/1",		false, 0, 0, 0, {}, {}, {}, objParser::ErrorType::FileFormatError }	// REJECTS there are no meshs
	)
);
