#include "ObjParserError.hpp"

/*
 * The error works like this:
 * Error contains a ErrorType and a string describing the error
 * ErrorType has an implicit conversion to Error with an empty string
 *     This means you can return ErrorType::{some error} and it will convert
 * Error can be compared and only the ErrorType will be compared
 *     This means that ptError(OK, "all good") == ptError(OK, "") will return true
 */

objParser::Error::Error() noexcept : errorType(objParser::ErrorType::OK) {}
objParser::Error::Error(const objParser::ErrorType& errorType) noexcept : errorType(errorType) {}
objParser::Error::Error(objParser::ErrorType errorType, std::string message) noexcept : errorType(errorType), message(message) {}

bool objParser::Error::operator==(const objParser::Error& other) const noexcept {
	return errorType == other.errorType;
}

bool objParser::Error::operator!=(const objParser::Error& other) const noexcept {
	return errorType != other.errorType;
}

bool objParser::Error::operator==(const objParser::ErrorType& other) const noexcept {
	return errorType == other;
}

bool objParser::Error::operator!=(const objParser::ErrorType& other) const noexcept {
	return errorType != other;
}

std::ostream& objParser::operator<<(std::ostream& oss, const objParser::ErrorType& error) noexcept {
	switch (error) {
	case(objParser::ErrorType::OK):
		oss << "OK";
		break;
	case(objParser::ErrorType::FileFormatError):
		oss << "FileFormatError";
		break;
	default:
		break;
	}

	return oss;
}

std::ostream& objParser::operator<<(std::ostream& oss, const objParser::Error& error) noexcept {
	oss << "Error: " << error.errorType << " - Message: " << error.message << std::endl;
	return oss;
} 