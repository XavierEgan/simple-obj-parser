#include <gtest/gtest.h>
#include <sstream>

#include "MtlParser.hpp"
#include "TestHelpers.hpp"

// tests for Mtl Parser that 
struct MtlParseCaseVec {
	std::string mtlFileContents;
	bool makeDummyMaterial;
	pt::PtErrorType expectedError = pt::PtErrorType::OK;
	
	glm::vec3 expectedKa = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 expectedKd = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 expectedKs = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 expectedTf = glm::vec3(0.0f, 0.0f, 0.0f);
};

class MtlParseTestVecFixture : public ::testing::TestWithParam<MtlParseCaseVec> {
protected:
	std::istringstream testStream;
	std::vector<pt::Material> materials;
	pt::PtError error;
public:
	MtlParseTestVecFixture() {
		const MtlParseCaseVec& testCase = GetParam();
		testStream = std::istringstream(testCase.mtlFileContents);

		if (testCase.makeDummyMaterial) {
			materials.push_back(pt::Material("t"));
		}
	}
};

TEST_P(MtlParseTestVecFixture, MtlParses) {
	const MtlParseCaseVec& testCase = GetParam();

	error = pt::MtlParser::parseStream(testStream, materials);

	if (materials.size() != 0) {
		EXPECT_TRUE(TestHelpers::check_vec3_close_vec3(testCase.expectedKa, materials.back().ambientColor));
		EXPECT_TRUE(TestHelpers::check_vec3_close_vec3(testCase.expectedKd, materials.back().diffuseColor));
		EXPECT_TRUE(TestHelpers::check_vec3_close_vec3(testCase.expectedKs, materials.back().specularColor));
		EXPECT_TRUE(TestHelpers::check_vec3_close_vec3(testCase.expectedTf, materials.back().transmissionFilter));
	}

	ASSERT_EQ(error, testCase.expectedError);
}

static constexpr glm::vec3 zero_vec = glm::vec3(0.0f, 0.0f, 0.0f);

INSTANTIATE_TEST_SUITE_P(
	MtlParser,
	MtlParseTestVecFixture,
	::testing::Values(
		// make sure they fail if there is no material
		MtlParseCaseVec{ "Ka 1.0 1.0 1.0", false, pt::PtErrorType::FileFormatError },
		MtlParseCaseVec{ "Kd 1.0 1.0 1.0", false, pt::PtErrorType::FileFormatError },
		MtlParseCaseVec{ "Ks 1.0 1.0 1.0", false, pt::PtErrorType::FileFormatError },
		MtlParseCaseVec{ "Tf 1.0 1.0 1.0", false, pt::PtErrorType::FileFormatError },

		// Ka, ambient color
		MtlParseCaseVec{ "Ka -0.1 0.0 0.0", true, pt::PtErrorType::FileFormatError },
		MtlParseCaseVec{ "Ka 0.0 -0.1 0.0", true, pt::PtErrorType::FileFormatError },
		MtlParseCaseVec{ "Ka 0.0 0.0 -0.1", true, pt::PtErrorType::FileFormatError },
		MtlParseCaseVec{ "Ka 1.1 0.0 0.0", true, pt::PtErrorType::FileFormatError },
		MtlParseCaseVec{ "Ka 0.0 1.1 0.0", true, pt::PtErrorType::FileFormatError },
		MtlParseCaseVec{ "Ka 0.0 0.0 1.1", true, pt::PtErrorType::FileFormatError },

		MtlParseCaseVec{ "Ka 0.0 0.0 0.0", true, pt::PtErrorType::OK, glm::vec3(0.0f, 0.0f, 0.0f) },
		MtlParseCaseVec{ "Ka 1.0 1.0 1.0", true, pt::PtErrorType::OK, glm::vec3(1.0f, 1.0f, 1.0f) },

		// Kd, diffuse color
		MtlParseCaseVec{ "Kd -0.1 0.0 0.0", true, pt::PtErrorType::FileFormatError },
		MtlParseCaseVec{ "Kd 0.0 -0.1 0.0", true, pt::PtErrorType::FileFormatError },
		MtlParseCaseVec{ "Kd 0.0 0.0 -0.1", true, pt::PtErrorType::FileFormatError },
		MtlParseCaseVec{ "Kd 1.1 0.0 0.0", true, pt::PtErrorType::FileFormatError },
		MtlParseCaseVec{ "Kd 0.0 1.1 0.0", true, pt::PtErrorType::FileFormatError },
		MtlParseCaseVec{ "Kd 0.0 0.0 1.1", true, pt::PtErrorType::FileFormatError },

		MtlParseCaseVec{ "Kd 0.0 0.0 0.0", true, pt::PtErrorType::OK, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f) },
		MtlParseCaseVec{ "Kd 1.0 1.0 1.0", true, pt::PtErrorType::OK, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f) },

		// Ks, specular color
		MtlParseCaseVec{ "Ks -0.1 0.0 0.0", true, pt::PtErrorType::FileFormatError },
		MtlParseCaseVec{ "Ks 0.0 -0.1 0.0", true, pt::PtErrorType::FileFormatError },
		MtlParseCaseVec{ "Ks 0.0 0.0 -0.1", true, pt::PtErrorType::FileFormatError },
		MtlParseCaseVec{ "Ks 1.1 0.0 0.0", true, pt::PtErrorType::FileFormatError },
		MtlParseCaseVec{ "Ks 0.0 1.1 0.0", true, pt::PtErrorType::FileFormatError },
		MtlParseCaseVec{ "Ks 0.0 0.0 1.1", true, pt::PtErrorType::FileFormatError },

		MtlParseCaseVec{ "Ks 0.0 0.0 0.0", true, pt::PtErrorType::OK, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f) },
		MtlParseCaseVec{ "Ks 1.0 1.0 1.0", true, pt::PtErrorType::OK, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f) },

		// Tf, Transmission Filter Color
		MtlParseCaseVec{ "Tf -0.1 0.0 0.0", true, pt::PtErrorType::FileFormatError },
		MtlParseCaseVec{ "Tf 0.0 -0.1 0.0", true, pt::PtErrorType::FileFormatError },
		MtlParseCaseVec{ "Tf 0.0 0.0 -0.1", true, pt::PtErrorType::FileFormatError },
		MtlParseCaseVec{ "Tf 1.1 0.0 0.0", true, pt::PtErrorType::FileFormatError },
		MtlParseCaseVec{ "Tf 0.0 1.1 0.0", true, pt::PtErrorType::FileFormatError },
		MtlParseCaseVec{ "Tf 0.0 0.0 1.1", true, pt::PtErrorType::FileFormatError },

		MtlParseCaseVec{ "Tf 0.0 0.0 0.0", true, pt::PtErrorType::OK, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f) },
		MtlParseCaseVec{ "Tf 1.0 1.0 1.0", true, pt::PtErrorType::OK, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f) }
	)
);