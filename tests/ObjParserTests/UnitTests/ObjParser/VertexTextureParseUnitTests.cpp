#include <gtest/gtest.h>
#include <sstream>

#include "ObjParser.hpp"
#include "TestHelpers.hpp"

// Parameterized testing help from here:
// https://www.sandordargo.com/blog/2019/04/24/parameterized-testing-with-gtest

struct VertexTextureParseCase {
	std::string objFileContents;
	bool makeDummyMesh;
	glm::vec3 expectedValue = glm::vec3(0.0f, 0.0f, 0.0f);
	objParser::ErrorType expectedError = objParser::ErrorType::OK;

	friend std::ostream& operator<<(std::ostream& os, const VertexTextureParseCase& pc) {
		os << "VertexTextureParseCase struct" << std::endl;
		return os;
	}
};

class VertexTextureParseTestFixture : public ::testing::TestWithParam<VertexTextureParseCase> {
protected:
	std::istringstream testStream;
	std::vector<objParser::Mesh> meshs;
	std::vector<objParser::Material> materials; // this is empty coz were not testing materials
	objParser::Error error;
public:
	VertexTextureParseTestFixture() {
		const VertexTextureParseCase& testCase = GetParam();
		testStream = std::istringstream(testCase.objFileContents);

		if (testCase.makeDummyMesh) {
			meshs.push_back(objParser::Mesh("t"));
		}
	}
};

TEST_P(VertexTextureParseTestFixture, ParsesVertex) {
	const VertexTextureParseCase& testCase = GetParam();

	error = objParser::ObjParser::parseStream(testStream, meshs, materials);

	if (testCase.expectedError != objParser::ErrorType::OK) {
		ASSERT_NE(error, objParser::ErrorType::OK);
		EXPECT_EQ(error, testCase.expectedError);

		if (meshs.size() != 0) {
			// we should not have modified the vector of verts (starts at 0, should remain 0)
			EXPECT_EQ(meshs.at(0).vertexTextureCoordinates.size(), 0);
		}

		return;
	}

	ASSERT_EQ(error, objParser::ErrorType::OK);

	ASSERT_NE(meshs.at(0).vertexTextureCoordinates.size(), 0);

	EXPECT_TRUE(TestHelpers::check_vec3_close_vec3(meshs.at(0).vertexTextureCoordinates.at(0), testCase.expectedValue));
}

INSTANTIATE_TEST_SUITE_P(
	ObjParser,
	VertexTextureParseTestFixture,
	// ACCEPTED tests should modify the vector
	// REJECTED tests should not
	::testing::Values(
		VertexTextureParseCase{ "vt 1 1 1",						true,		glm::vec3(1.0f, 1.0f, 1.0f) },		// ACCEPTS no decimal place
		VertexTextureParseCase{ "vt 01 01 01",					true,		glm::vec3(1.0f, 1.0f, 1.0f) },		// ACCEPTS leading zero
		VertexTextureParseCase{ "vt 01.0 01.0 01.0",			true,		glm::vec3(1.0f, 1.0f, 1.0f) },		// ACCEPTS leading zero and decimal
		VertexTextureParseCase{ "vt -1 -1 -1",					true,		glm::vec3(-1.0f, -1.0f, -1.0f) },	// ACCEPTS negative no decimal place
		VertexTextureParseCase{ "vt -01 -01 -01",				true,		glm::vec3(-1.0f, -1.0f, -1.0f) },	// ACCEPTS negative leading zero
		VertexTextureParseCase{ "vt -01.0 -01.0 -01.0",			true,		glm::vec3(-1.0f, -1.0f, -1.0f) },	// ACCEPTS negative leading zero and decimal

		VertexTextureParseCase{ "vt 1.0 1.0 1.0",				true,		glm::vec3(1.0f, 1.0f, 1.0f) },		// ACCEPTS three values
		VertexTextureParseCase{ "vt 1.0 1.0",					true,		glm::vec3(1.0f, 1.0f, 0.0f) },		// ACCEPTS two values (optional defaults to 0)
		VertexTextureParseCase{ "vt 1.0",						true,		glm::vec3(1.0f, 0.0f, 0.0f) },		// ACCEPTS one values (optional defaults to 0)

		VertexTextureParseCase{ "vt",							true,	glm::vec3(0.0f, 0.0f, 0.0f), objParser::ErrorType::FileFormatError }	// REJECTS no values
	)
);
