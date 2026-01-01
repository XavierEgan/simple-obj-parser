// unity build of all the tests

#define OBJ_PARSER_IMPLEMENTATION
#include "../obj_parser/obj_parser.hpp"
#include "TestHelpers.hpp"
#include <gtest/gtest.h>

#include "ObjParserTests/IntegrationTests/MtlandObjIntegrationTests.cpp"

#include "ObjParserTests/UnitTests/MtlParser/MtlParserUnitTestsFloat.cpp"
#include "ObjParserTests/UnitTests/MtlParser/MtlParserUnitTestsVec.cpp"

#include "ObjParserTests/UnitTests/ObjParser/FaceParseUnitTests.cpp"
#include "ObjParserTests/UnitTests/ObjParser/ReadsFile.cpp"
#include "ObjParserTests/UnitTests/ObjParser/VertexNormalParseUnitTests.cpp"
#include "ObjParserTests/UnitTests/ObjParser/VertexParseUnitTests.cpp"
#include "ObjParserTests/UnitTests/ObjParser/VertexTextureParseUnitTests.cpp"