#include <gtest/gtest.h>
#include <sstream>

#include "MtlParser.hpp"
#include "TestHelpers.hpp"

// tests for Mtl Parser that 

struct MtlParseCaseFloat {
	std::string mtlFileContents;
	bool makeDummyMaterial;
	objParser::ErrorType expectedError = objParser::ErrorType::OK;
	
	float expectedNs = 0.0;
	float expecteddTransparency = 0.0;
	float expectedNi = 0.0;
};

class MtlParseTestFloatFixture : public ::testing::TestWithParam<MtlParseCaseFloat> {
protected:
	std::istringstream testStream;
	std::vector<objParser::Material> materials;
	objParser::PtError error;
public:
	MtlParseTestFloatFixture() {
		const MtlParseCaseFloat& testCase = GetParam();
		testStream = std::istringstream(testCase.mtlFileContents);

		if (testCase.makeDummyMaterial) {
			materials.push_back(objParser::Material("t"));
		}
	}
};

TEST_P(MtlParseTestFloatFixture, MtlParses) {
	const MtlParseCaseFloat& testCase = GetParam();

	error = objParser::MtlParser::parseStream(testStream, materials);

	// were not checking material creation, so its fine
	if (materials.size() != 0) {
		EXPECT_FLOAT_EQ(materials.back().specularExponent, testCase.expectedNs);
		EXPECT_FLOAT_EQ(materials.back().transparent, testCase.expecteddTransparency);
		EXPECT_FLOAT_EQ(materials.back().indexOfRefraction, testCase.expectedNi);
	}

	ASSERT_EQ(error, testCase.expectedError);
}

INSTANTIATE_TEST_SUITE_P(
	MtlParser,
	MtlParseTestFloatFixture,
	::testing::Values(
		// make sure they fail if there is no material
		MtlParseCaseFloat{ "Ns 1.0", false, objParser::ErrorType::FileFormatError },
		MtlParseCaseFloat{ "d 1.0", false, objParser::ErrorType::FileFormatError },
		MtlParseCaseFloat{ "Tr 1.0", false, objParser::ErrorType::FileFormatError },
		MtlParseCaseFloat{ "Ni 1.0", false, objParser::ErrorType::FileFormatError },

		// Ns, specular exponent
		MtlParseCaseFloat{ "Ns -0.1", true, objParser::ErrorType::FileFormatError },
		MtlParseCaseFloat{ "Ns 1000.1", true, objParser::ErrorType::FileFormatError },
		MtlParseCaseFloat{ "Ns 0.0", true, objParser::ErrorType::OK, 0.0},
		MtlParseCaseFloat{ "Ns 500.0", true, objParser::ErrorType::OK, 500.0 },
		MtlParseCaseFloat{ "Ns 1000.0", true, objParser::ErrorType::OK, 1000.0 },

		MtlParseCaseFloat{ "Ns a", true, objParser::ErrorType::FileFormatError },
		MtlParseCaseFloat{ "Ns", true, objParser::ErrorType::FileFormatError },

		// d, dissolve (inverse transparency)
		MtlParseCaseFloat{ "d -0.1", true, objParser::ErrorType::FileFormatError },
		MtlParseCaseFloat{ "d 1.1", true, objParser::ErrorType::FileFormatError },
		MtlParseCaseFloat{ "d 0.0", true, objParser::ErrorType::OK, 0.0, 1.0 },
		MtlParseCaseFloat{ "d 0.2", true, objParser::ErrorType::OK, 0.0, 0.8 },
		MtlParseCaseFloat{ "d 1.0", true, objParser::ErrorType::OK, 0.0, 0.0 },

		MtlParseCaseFloat{ "d a", true, objParser::ErrorType::FileFormatError },
		MtlParseCaseFloat{ "d", true, objParser::ErrorType::FileFormatError },

		// Tr, transparency
		MtlParseCaseFloat{ "Tr -0.1", true, objParser::ErrorType::FileFormatError },
		MtlParseCaseFloat{ "Tr 1.1", true, objParser::ErrorType::FileFormatError },
		MtlParseCaseFloat{ "Tr 0.0", true, objParser::ErrorType::OK, 0.0, 0.0 },
		MtlParseCaseFloat{ "Tr 0.2", true, objParser::ErrorType::OK, 0.0, 0.2 },
		MtlParseCaseFloat{ "Tr 1.0", true, objParser::ErrorType::OK, 0.0, 1.0 },

		MtlParseCaseFloat{ "Tr a", true, objParser::ErrorType::FileFormatError },
		MtlParseCaseFloat{ "Tr", true, objParser::ErrorType::FileFormatError },

		// Ni, index of refraction/optical density
		MtlParseCaseFloat{ "Ni 0.0", true, objParser::ErrorType::FileFormatError },
		MtlParseCaseFloat{ "Ni 10.1", true, objParser::ErrorType::FileFormatError },
		MtlParseCaseFloat{ "Ni 0.001", true, objParser::ErrorType::OK, 0.0, 0.0, 0.001 },
		MtlParseCaseFloat{ "Ni 5.0", true, objParser::ErrorType::OK, 0.0, 0.0, 5.0 },
		MtlParseCaseFloat{ "Ni 10.0", true, objParser::ErrorType::OK, 0.0, 0.0, 10.0 },

		MtlParseCaseFloat{ "Ni a", true, objParser::ErrorType::FileFormatError },
		MtlParseCaseFloat{ "Ni", true, objParser::ErrorType::FileFormatError }
	)
);