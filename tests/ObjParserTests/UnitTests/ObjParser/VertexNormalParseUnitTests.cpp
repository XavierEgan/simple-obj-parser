#include <gtest/gtest.h>
#include <sstream>

#include "ObjParser.hpp"
#include "TestHelpers.hpp"

// Parameterized testing help from here:
// https://www.sandordargo.com/blog/2019/04/24/parameterized-testing-with-gtest

struct VertexNormalParseCase {
	std::string objFileContents;
	bool makeDummyMesh;
	bool shouldBeOk;
	glm::vec3 expectedValue = glm::vec3(0.0f, 0.0f, 0.0f);
	objParser::ErrorType expectedError = objParser::ErrorType::OK;

	friend std::ostream& operator<<(std::ostream& os, const VertexNormalParseCase& pc) {
		os << "VertexNormalParseCase struct" << std::endl;
		return os;
	}
};

class VertexNormalParseTestFixture : public ::testing::TestWithParam<VertexNormalParseCase> {
protected:
	std::istringstream testStream;
	std::vector<objParser::Mesh> meshs;
	std::vector<objParser::Material> materials; // this is empty coz were not testing materials
	objParser::PtError error;
public:
	VertexNormalParseTestFixture() {
		const VertexNormalParseCase& testCase = GetParam();
		testStream = std::istringstream(testCase.objFileContents);

		if (testCase.makeDummyMesh) {
			meshs.push_back(objParser::Mesh("t"));
		}
	}
};

TEST_P(VertexNormalParseTestFixture, ParsesVertex) {
	const VertexNormalParseCase& testCase = GetParam();

	error = objParser::ObjParser::parseStream(testStream, meshs, materials);

	if (!testCase.shouldBeOk) {
		ASSERT_NE(error, objParser::ErrorType::OK);
		EXPECT_EQ(error, testCase.expectedError);

		if (meshs.size() != 0) {
			// we should not have modified the vector of vertexNormals (starts at 0, should remain 0)
			EXPECT_EQ(meshs.at(0).vertexNormals.size(), 0);
		}

		return;
	}

	ASSERT_EQ(error, objParser::ErrorType::OK);

	ASSERT_NE(meshs.at(0).vertexNormals.size(), 0);

	EXPECT_TRUE(TestHelpers::check_vec3_close_vec3(meshs.at(0).vertexNormals.at(0), testCase.expectedValue));
}

constexpr float oneSqrtThree = 0.57735026919f;
constexpr glm::vec3 normVec = glm::vec3(oneSqrtThree, oneSqrtThree, oneSqrtThree);

INSTANTIATE_TEST_SUITE_P(
	ObjParser,
	VertexNormalParseTestFixture,
	// ACCEPTED tests should modify the vector
	// REJECTED tests should not modify the vector
	::testing::Values(
		VertexNormalParseCase{ "vn 1 1 1",						true, true,		normVec },		// ACCEPTS no decimal place
		VertexNormalParseCase{ "vn 01 01 01",					true, true,		normVec },		// ACCEPTS leading zero
		VertexNormalParseCase{ "vn 01.0 01.0 01.0",				true, true,		normVec },		// ACCEPTS leading zero and decimal
		VertexNormalParseCase{ "vn -1 -1 -1",					true, true,		-normVec },		// ACCEPTS negative no decimal place
		VertexNormalParseCase{ "vn -01 -01 -01",				true, true,		-normVec },		// ACCEPTS negative leading zero
		VertexNormalParseCase{ "vn -01.0 -01.0 -01.0",			true, true,		-normVec },		// ACCEPTS negative leading zero and decimal

		VertexNormalParseCase{ "vn 1.0 1.0 1.0",				true, true,		normVec },		// ACCEPTS three values
		VertexNormalParseCase{ "vn 1.0 1.0 1.0 1.0",			true, true,		normVec },		// ACCEPTS four values (NOTE: any value past 3 is ignored)
		VertexNormalParseCase{ "vn 1.0 1.0 1.0 1.0 1.0",		true, true,		normVec },		// ACCEPTS five (NOTE: any value past 3 is ignored)
		VertexNormalParseCase{ "vn 1.0 1.0 1.0 1.0 1.0 1.0",	true, true,		normVec },		// ACCEPTS six (NOTE: any value past 3 is ignored)

		VertexNormalParseCase{ "vn 1.0 1.0 1.0 # comment",		true, true,		normVec },		// ACCEPTS ignores comment

		VertexNormalParseCase{ "vn",							true, false,	glm::vec3(0.0f, 0.0f, 0.0f), objParser::ErrorType::FileFormatError },	// REJECTS no values
		VertexNormalParseCase{ "vn 1.0",						true, false,	glm::vec3(0.0f, 0.0f, 0.0f), objParser::ErrorType::FileFormatError },	// REJECTS one value
		VertexNormalParseCase{ "vn 1.0 1.0",					true, false,	glm::vec3(0.0f, 0.0f, 0.0f), objParser::ErrorType::FileFormatError },	// REJECTS two values
		VertexNormalParseCase{ "vn 1.0 1.0 1.0",				false,false,	glm::vec3(0.0f, 0.0f, 0.0f), objParser::ErrorType::FileFormatError },	// REJECTS when there is no object
		VertexNormalParseCase{ "vn t",							true, false,	glm::vec3(0.0f, 0.0f, 0.0f), objParser::ErrorType::FileFormatError }		// REJECTS when there is a letter instead of number
	
		
	)
);