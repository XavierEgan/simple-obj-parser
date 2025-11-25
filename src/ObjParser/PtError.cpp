#include "PtError.hpp"

/*
 * The error works like this:
 * PtError contains a PtErrorType and a string describing the error
 * PtErrorType has an implicit conversion to PtError with an empty string
 *     This means you can return PtErrorType::{some error} and it will convert
 * PtError can be compared and only the PtErrorType will be compared
 *     This means that ptError(OK, "all good") == ptError(OK, "") will return true
 */

pt::PtError::PtError() noexcept : errorType(pt::PtErrorType::OK) {}
pt::PtError::PtError(const pt::PtErrorType& errorType) noexcept : errorType(errorType) {}
pt::PtError::PtError(pt::PtErrorType errorType, std::string message) noexcept : errorType(errorType), message(message) {}

bool pt::PtError::operator==(const pt::PtError& other) const noexcept {
	return errorType == other.errorType;
}

bool pt::PtError::operator!=(const pt::PtError& other) const noexcept {
	return errorType != other.errorType;
}

bool pt::PtError::operator==(const pt::PtErrorType& other) const noexcept {
	return errorType == other;
}

bool pt::PtError::operator!=(const pt::PtErrorType& other) const noexcept {
	return errorType != other;
}

std::ostream& pt::operator<<(std::ostream& oss, const pt::PtErrorType& error) noexcept {
	switch (error) {
	case(pt::PtErrorType::OK):
		oss << "OK";
		break;
	case(pt::PtErrorType::FileFormatError):
		oss << "FileFormatError";
		break;
	default:
		break;
	}

	return oss;
}

std::ostream& pt::operator<<(std::ostream& oss, const pt::PtError& error) noexcept {
	oss << "PtError: " << error.errorType << " - Message: " << error.message << std::endl;
	return oss;
}