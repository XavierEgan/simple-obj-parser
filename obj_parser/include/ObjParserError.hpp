#pragma once
#include <ostream>
#include <istream>
#include <string>

namespace objParser {
	enum ErrorType {
		OK,
		FileFormatError,
		FileNotFound
	};

	std::ostream& operator<<(std::ostream& oss, const objParser::ErrorType& error) noexcept;

	struct Error {
		ErrorType errorType;
		std::string message;

		Error() noexcept;
		Error(const ErrorType& error_t) noexcept;
		Error(ErrorType error_t, std::string message) noexcept;

		bool operator==(const Error& other) const noexcept;
		bool operator!=(const Error& other) const noexcept;

		bool operator==(const ErrorType& other) const noexcept;
		bool operator!=(const ErrorType& other) const noexcept;

		friend std::ostream& operator<<(std::ostream& oss, const Error& error) noexcept;
	};
}