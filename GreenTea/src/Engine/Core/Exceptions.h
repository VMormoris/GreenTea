#pragma once

#include "Engine.h"
#include "GreenTeaWin.h"

#include <exception>
#include <string>

namespace gte {

	class ENGINE_API RuntimeException : public std::exception {
	public:
		RuntimeException(void) noexcept
			: mErrorCode(0) {}
		RuntimeException(uint32 errorCode)
			: mErrorCode(errorCode) {}

		uint32 GetErrorCode(void) const noexcept { return mErrorCode; }

	private:
		const uint32 mErrorCode;
	};

	class ENGINE_API AssertException : public std::exception {
	public:
		AssertException(void) noexcept = default;
	};

	namespace internal {
	
		class ENGINE_API ScopedSETranslator {
		public:
			ScopedSETranslator(_se_translator_function se_translator) noexcept
				: mOldSETranslator(_set_se_translator(se_translator)) {}
			~ScopedSETranslator(void) noexcept { _set_se_translator(mOldSETranslator); }
		private:
			const _se_translator_function mOldSETranslator;
		};

		ENGINE_API [[nodiscard]] std::string FormatErrorMessage(uint32 errorCode);

		void TranslateFunction(uint32 errorCode, EXCEPTION_POINTERS* ptr);
	}
}