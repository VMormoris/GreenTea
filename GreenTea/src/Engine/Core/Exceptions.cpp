#include "Exceptions.h"

//#include <Windows.h>
#include <DbgHelp.h>
#include <sstream>

namespace gte::internal {

	[[nodiscard]] std::string FormatErrorMessage(uint32 errorCode)
	{
		LPSTR errorMessageBuffer = nullptr;
		DWORD messageSize = FormatMessageA
		(
			FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			nullptr,
			errorCode,
			MAKELANGID(LANG_ENGLISH, SUBLANG_DEFAULT),
			(LPSTR)&errorMessageBuffer,
			0,
			nullptr
		);

		if (messageSize == 0)//Not system-defined error
		{
			switch (errorCode)
			{
            case EXCEPTION_ACCESS_VIOLATION:        return "Access violation";
            case EXCEPTION_DATATYPE_MISALIGNMENT:   return "Data type misalignment";
            case EXCEPTION_BREAKPOINT:              return "Breakpoint";
            case EXCEPTION_SINGLE_STEP:             return "Single Step";
            case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:   return "Array bounds exceeded";
            case EXCEPTION_FLT_DENORMAL_OPERAND:    return "Floating point number denormal operand";
            case EXCEPTION_FLT_DIVIDE_BY_ZERO:      return "Floating point number division by zero";
            case EXCEPTION_FLT_INEXACT_RESULT:      return "Floating point number inexact result";
            case EXCEPTION_FLT_INVALID_OPERATION:   return "Floating point number invalid operation";
            case EXCEPTION_FLT_OVERFLOW:            return "Floating point number overflow";
            case EXCEPTION_FLT_STACK_CHECK:         return "Floating point number stack check";
            case EXCEPTION_FLT_UNDERFLOW:           return "Floating point number underflow";
            case EXCEPTION_INT_DIVIDE_BY_ZERO:      return "Integer division by zero";
            case EXCEPTION_INT_OVERFLOW:            return "Integer overflow";
            case EXCEPTION_PRIV_INSTRUCTION:        return "Privileged instruction";
            case EXCEPTION_IN_PAGE_ERROR:           return "In-page error";
            case EXCEPTION_ILLEGAL_INSTRUCTION:     return "Illegal instruction";
            case EXCEPTION_STACK_OVERFLOW:          return "Stack overflow";
            case EXCEPTION_INVALID_DISPOSITION:     return "Invalid disposition";
            case EXCEPTION_GUARD_PAGE:              return "Guard page violation";
            case EXCEPTION_INVALID_HANDLE:          return "Invalid handle";
            case ERROR_FILE_NOT_FOUND:              return "The system cannot find the file specified.";
            case ERROR_PATH_NOT_FOUND:              return "The system cannot find the path specified.";
            case ERROR_ACCESS_DENIED:               return "Access is denied.";
            case ERROR_INVALID_PARAMETER:           return "The parameter is incorrect.";
            case ERROR_INVALID_HANDLE:              return "The handle is invalid.";
            case ERROR_NOT_ENOUGH_MEMORY:           return "Not enough storage is available to complete this operation.";
            case ERROR_OUTOFMEMORY:                 return "Not enough storage is available to complete this operation.";
            case ERROR_INVALID_FUNCTION:            return "Incorrect function.";
            case ERROR_CALL_NOT_IMPLEMENTED:        return "This function is not supported on this system.";
            case ERROR_INVALID_MODULETYPE:          return "The specified module type is invalid.";
                //TODO(Vasilis): Add more errors as we go
            default:                                return "An unknown error has occured.";
			}
		}

        std::string errorMessage(errorMessageBuffer);
        // Free the buffer that was allocated by FormatMessage
        LocalFree(errorMessageBuffer);
        return errorMessage;
	}

    void TranslateFunction(uint32 errorCode, EXCEPTION_POINTERS* ptr)
    {
        std::stringstream stream;
        stream << "Exception raised (" << std::hex << errorCode << "): " << FormatErrorMessage(errorCode) << '\n';

        // Initialize the symbol engine
        SymInitialize(GetCurrentProcess(), NULL, TRUE);

        // Obtain the call stack
        HANDLE process = GetCurrentProcess();
        HANDLE thread = GetCurrentThread();
        CONTEXT context = *ptr->ContextRecord;

        STACKFRAME64 stackFrame;
        memset(&stackFrame, 0, sizeof(stackFrame));

        DWORD machineType = IMAGE_FILE_MACHINE_AMD64;
        stackFrame.AddrPC.Offset = context.Rip;
        stackFrame.AddrPC.Mode = AddrModeFlat;
        stackFrame.AddrFrame.Offset = context.Rsp;
        stackFrame.AddrFrame.Mode = AddrModeFlat;
        stackFrame.AddrStack.Offset = context.Rsp;
        stackFrame.AddrStack.Mode = AddrModeFlat;

        stream << "Stack trace:\n";
        for (;;)
        {
            if (!StackWalk64(machineType, process, thread, &stackFrame, &context, NULL, SymFunctionTableAccess64, SymGetModuleBase64, NULL))
                break;
            if (stackFrame.AddrPC.Offset == 0)
                break;

            DWORD64 displacement64;
            IMAGEHLP_SYMBOL64* symbol = (IMAGEHLP_SYMBOL64*)malloc(sizeof(IMAGEHLP_SYMBOL64) + MAX_SYM_NAME);
            symbol->SizeOfStruct = sizeof(IMAGEHLP_SYMBOL64);
            symbol->MaxNameLength = MAX_SYM_NAME;
            if (SymGetSymFromAddr64(process, stackFrame.AddrPC.Offset, &displacement64, symbol))
                stream << '\t' << symbol->Name << " + 0x" << std::hex << displacement64 << " (0x" << std::hex << stackFrame.AddrPC.Offset << ")" << '\n';
            else
                stream << "\tUnable to obtain symbol information for current location (0x" << std::hex << stackFrame.AddrPC.Offset << ")" << '\n';

            free(symbol);
        }

        GTE_ERROR_LOG(stream.str());
        throw RuntimeException(errorCode);
    }

}