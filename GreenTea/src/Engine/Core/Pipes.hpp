#pragma once

#include "Pipes.h"
#include "GreenTeaWin.h"

#include <thread>

namespace gte::internal {

	template<typename Func>
	inline void ServerPipe::Start(Func&& func) noexcept
	{
		mServerRunning = true;
		mServerThread = std::thread([func, this]()
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

}