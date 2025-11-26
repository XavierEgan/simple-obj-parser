#include "PtError.hpp"

/*
 * The error works like this:
 * PtError contains a ErrorType and a string describing the error
 * ErrorType has an implicit conversion to PtError with an empty string
 *     This means you can return ErrorType::{some error} and it will convert
 * PtError can be compared and only the ErrorType will be compared
 *     This means that ptError(OK, "all good") == ptError(OK, "") will return true
 */

objParser::PtError::PtError() noexcept : errorType(objParser::ErrorType::OK) {}
objParser::PtError::PtError(const objParser::ErrorType& errorType) noexcept : errorType(errorType) {}
objParser::PtError::PtError(objParser::ErrorType errorType, std::string message) noexcept : errorType(errorType), message(message) {}

bool objParser::PtError::operator==(const objParser::PtError& other) const noexcept {
	return errorType == other.errorType;
}

bool objParser::PtError::operator!=(const objParser::PtError& other) const noexcept {
	return errorType != other.errorType;
}

bool objParser::PtError::operator==(const objParser::ErrorType& other) const noexcept {
	return errorType == other;
}

bool objParser::PtError::operator!=(const objParser::ErrorType& other) const noexcept {
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

std::ostream& objParser::operator<<(std::ostream& oss, const objParser::PtError& error) noexcept {
	oss << "PtError: " << error.errorType << " - Message: " << error.message << std::endl;
	return oss;
} 