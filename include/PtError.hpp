#pragma once
#include <ostream>
#include <istream>
#include <string>

namespace pt {
	enum PtErrorType {
		OK,
		FileFormatError
	};

	std::ostream& operator<<(std::ostream& oss, const pt::PtErrorType& error) noexcept;

	struct PtError {
		PtErrorType errorType;
		std::string message;

		PtError() noexcept;
		PtError(const PtErrorType& error_t) noexcept;
		PtError(PtErrorType error_t, std::string message) noexcept;

		bool operator==(const PtError& other) const noexcept;
		bool operator!=(const PtError& other) const noexcept;

		bool operator==(const PtErrorType& other) const noexcept;
		bool operator!=(const PtErrorType& other) const noexcept;

		friend std::ostream& operator<<(std::ostream& oss, const PtError& error) noexcept;
	};
}