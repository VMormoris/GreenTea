#pragma once

#include "Engine.h"

#include <thread>
#include <atomic>

namespace gte::internal {
	/**
	* @brief Wrapper that enables you to read & write into pipes
	*/
	class ENGINE_API PipeStream {
	public:
		// Constructor(s) & Destructor
		virtual ~PipeStream(void) = default;
		PipeStream(void) = default;
		PipeStream(void* pipe)
			: mPipe(pipe) {}

		/**
		* @brief Sends data through the pipe
		* @param data Pointer to the data buffer that will be send
		* @param size Number of bytes that will be send
		* @param offset How many bytes away from the start should we start sending
		* @return The number of bytes that were actually send
		*/
		uint32 Send(const void* data, uint32 size, uint32 offset = 0) noexcept;

		/**
		* @brief Receives data from from the pipe
		* @param[out] data Pointer to the data that will be received
		* @param[in] length Number of bytes that will be received
		* @param[in] offset How many bytes away from the start should we start receiving
		* @return The number of bytes that were actually received
		*/
		uint32 Receive(void* data, uint32 size, uint32 offset = 0) noexcept;

	protected:
		void* mPipe = (void*)-1;
	};

	/**
	* @brief A wrapper for a named pipe server
	*/
	class ServerPipe {
		using CommunicationHandler = void (*)(PipeStream);
	public:
		// Destructor
		~ServerPipe(void) noexcept;

		/**
		* @brief Creates an instance of a named pipe server
		* @warning This pipe is not ready for use before calling Open
		* @sa Open
		*/
		ServerPipe(void) = default;
		/**
		* @brief Creates an instance of the named pipe server
		* @param name Name that will be used to open the pipe
		*/
		ServerPipe(const std::string& name) noexcept;

		/**
		* @brief Opens the pipe using the specified named
		* @param name Name that will be used to open the pipe
		*/
		void Open(const std::string& name) noexcept;

		/**
		* @brief Closes the pipe if it's already opened
		*/
		void Close(void) noexcept;

		/**
		* @brief Start a new thread that listens for new connection
		* @tparam Func Lamda with signature: void (PipeStream stream)
		* @param func Function that will be used to handle communication for new clients
		*/
		template<typename Func>
		void Start(Func&& func) noexcept;

		/**
		* @brief Signals the Server that it should close
		* @warning Server will actually close after the next connection attempt
		*/
		void Stop(void) noexcept { mServerRunning = false; }

		/**
		* @brief Start a new thread that listens for new connection
		* @param func Function that will be used to handle communication for new clients
		*/
		void Start(CommunicationHandler func) noexcept;

		// Not copying allowed
		ServerPipe(const ServerPipe&) = delete;
		ServerPipe& operator=(const ServerPipe&) = delete;

	private:
		void* mPipe = (void*)-1;
		std::thread mServerThread;
		std::atomic<bool> mServerRunning = false;
	};

	/**
	* @brief A wrapper for a named pipe client
	*/
	class ClientPipe final : public PipeStream {
	public:
		// Destructor
		~ClientPipe(void);
		
		/**
		* @brief Creates an instance of a named pipe client
		* @warning This pipe is not ready for use before calling Open
		* @sa Open
		*/
		ClientPipe(void) = default;
		/**
		* @brief Creates an instance of the named pipe client
		* @param name Name that will be used to open the pipe
		*/
		ClientPipe(const std::string& name) noexcept;

		/**
		* @brief Opens the pipe using the specified named
		* @param name Name that will be used to open the pipe
		*/
		void Open(const std::string& name) noexcept;

		/**
		* @brief Closes the pipe if it's already opened
		*/
		void Close(void) noexcept;

		// Not copying allowed
		ClientPipe(const ClientPipe&) = delete;
		ClientPipe& operator=(const ClientPipe&) = delete;
	};

}

#include "Pipes.hpp"