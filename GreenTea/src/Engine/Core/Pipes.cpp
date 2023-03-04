#include "Pipes.h"
#include "GreenTeaWin.h"

#include <thread>

namespace gte::internal {

	uint32 PipeStream::Send(const void* data, uint32 size, uint32 offset) noexcept
	{
		// Go to the specified offset
		const void* ptr = (char*)data + offset;
		// Try write into pipe
		DWORD bytes;
		int32 result = WriteFile(mPipe, ptr, size, &bytes, nullptr);

		if (!result)// Check if the write operation was succeful
			return 0;

		return bytes;
	}

	uint32 PipeStream::Receive(void* data, uint32 size, uint32 offset) noexcept
	{
		// Go to the specified offset
		void* ptr = (char*)data + offset;
		// Try read from the pipe
		DWORD bytes;
		int32 result = ReadFile(mPipe, ptr, size, &bytes, nullptr);

		if (!result)// Check if the read operation was succesful
			return 0;

		return bytes;
	}

	void ServerPipe::Start(CommunicationHandler func) noexcept
	{
		mServerThread = std::thread([&]()
		{
			while (mServerRunning)
			{
				//Wait for new client to connect
				if (!ConnectNamedPipe(mPipe, nullptr))
				{
					GTE_ERROR_LOG("Incoming connection failed with error: ", GetLastError());
					break;
				}
				func(PipeStream(mPipe));//Call user defined function to handle client communication
				DisconnectNamedPipe(mPipe);//Release pipe for next connection
			}
		});
	}

	void ServerPipe::Open(const std::string& name) noexcept
	{
		Close();
		mPipe = CreateNamedPipeA
		(
			name.c_str(),
			PIPE_ACCESS_DUPLEX,
			PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT,
			PIPE_UNLIMITED_INSTANCES,
			0, 0,
			100,
			nullptr
		);
		//TODO(Vasilis): Add assert/error
	}

	void ServerPipe::Close(void) noexcept
	{
		if (mPipe != INVALID_HANDLE_VALUE)
		{
			mServerThread.join();
			CloseHandle(mPipe);
			mPipe = INVALID_HANDLE_VALUE;
		}
	}

	ServerPipe::~ServerPipe(void) noexcept { Close(); }

	ServerPipe::ServerPipe(const std::string& name) noexcept { Open(name); }

	void ClientPipe::Open(const std::string& name) noexcept
	{
		Close();
		mPipe = CreateFileA
		(
			name.c_str(),
			GENERIC_READ | GENERIC_WRITE,
			0,
			nullptr,
			OPEN_EXISTING,
			0,
			nullptr
		);
	}

	void ClientPipe::Close() noexcept
	{
		if (mPipe != INVALID_HANDLE_VALUE)
		{
			CloseHandle(mPipe);
			mPipe = INVALID_HANDLE_VALUE;
		}
	}

	ClientPipe::~ClientPipe(void) { Close(); }

	ClientPipe::ClientPipe(const std::string& name) noexcept 
		: PipeStream() { Open(name); }

}