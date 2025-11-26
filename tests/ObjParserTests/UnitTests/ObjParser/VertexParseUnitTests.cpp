#include <gtest/gtest.h>
#include <sstream>

#include "ObjParser.hpp"
#include "TestHelpers.hpp"

// Parameterized testing help from here:
// https://www.sandordargo.com/blog/2019/04/24/parameterized-testing-with-gtest

struct VertexParseCase {
	std::string objFileContents;
	bool makeDummyMesh;
	glm::vec3 expectedValue = glm::vec3(0.0f, 0.0f, 0.0f);
	objParser::ErrorType expectedError = objParser::ErrorType::OK;

	friend std::ostream& operator<<(std::ostream& os, const VertexParseCase& pc) {
		os << "VertexParseCase struct" << std::endl;
		return os;
	}
};

class VertexParseTestFixture : public ::testing::TestWithParam<VertexParseCase> {
protected:
	std::istringstream testStream;
	std::vector<objParser::Mesh> meshs;
	std::vector<objParser::Material> materials; // this is empty coz were not testing materials
	objParser::Error error;
public:
	VertexParseTestFixture() {
		const VertexParseCase& testCase = GetParam();
		testStream = std::istringstream(testCase.objFileContents);

		if (testCase.makeDummyMesh) {
			meshs.push_back(objParser::Mesh("t"));
		}
	}
};

TEST_P(VertexParseTestFixture, ParsesVertex) {
	const VertexParseCase& testCase = GetParam();

	error = objParser::ObjParser::parseStream(testStream, meshs, materials);

	if (testCase.expectedError != objParser::ErrorType::OK) {
		EXPECT_EQ(error, testCase.expectedError);
		
		if (meshs.size() != 0) {
			// we should not have modified the vector of verts (starts at 0, should remain 0)
			EXPECT_EQ(meshs.at(0).vertices.size(), 0);
		}

		return;
	}
	
	ASSERT_EQ(error, objParser::ErrorType::OK);

	ASSERT_NE(meshs.at(0).vertices.size(), 0);

	EXPECT_TRUE(TestHelpers::check_vec3_close_vec3(meshs.at(0).vertices.at(0), testCase.expectedValue));
}

INSTANTIATE_TEST_SUITE_P(
	ObjParser,
	VertexParseTestFixture,
	// ACCEPTED tests should modify the vector
	// REJECTED tests should not
	::testing::Values(
		VertexParseCase{ "v 1 1 1",						true,		glm::vec3(1.0f, 1.0f, 1.0f)},		// ACCEPTS no decimal place
		VertexParseCase{ "v 01 01 01",					true,		glm::vec3(1.0f, 1.0f, 1.0f) },		// ACCEPTS leading zero
		VertexParseCase{ "v 01.0 01.0 01.0",			true,		glm::vec3(1.0f, 1.0f, 1.0f) },		// ACCEPTS leading zero and decimal
		VertexParseCase{ "v -1 -1 -1",					true,		glm::vec3(-1.0f, -1.0f, -1.0f) },	// ACCEPTS negative no decimal place
		VertexParseCase{ "v -01 -01 -01",				true,		glm::vec3(-1.0f, -1.0f, -1.0f) },	// ACCEPTS negative leading zero
		VertexParseCase{ "v -01.0 -01.0 -01.0",			true,		glm::vec3(-1.0f, -1.0f, -1.0f) },	// ACCEPTS negative leading zero and decimal

		VertexParseCase{ "v 1.0 1.0 1.0 -.5",			true,		glm::vec3(-2.0f, -2.0f, -2.0f) },	// ACCEPTS w, which scales by 1/w
		VertexParseCase{ "v 1.0 1.0 1.0 .5 1.0 1.0 1.0",true,		glm::vec3(2.0f, 2.0f, 2.0f) },		// ACCEPTS a bunch of numbers (some programs use them to specify rgb, so its still valid im just ignoring it)
		
		VertexParseCase{ "v a 1.0 1.0 .5",				true,		glm::vec3(), objParser::ErrorType::FileFormatError },	// REJECTS letter instead of number
		VertexParseCase{ "v 1 1.0 b .5",				true,		glm::vec3(), objParser::ErrorType::FileFormatError }		// REJECTS letter instead of number
	)
);