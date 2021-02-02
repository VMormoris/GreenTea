#pragma once

#include <GreenTea/Core/Ref.h>
#include "Asset.h"

namespace GTE {

	/**
	* @brief AssetManager is the proper way to request Asset Handles using GreenTea engine
	* @details AssetManger is responsible for the asynchronous loading 
	*	of a given an Asset to the apropriate target.
	*/
	class ENGINE_API AssetManager {
	public:

		/**
		* @brief Initialization of thread used for Asset Loading
		*/
		static void Init(void);

		/**
		* @brief Stops the threads
		*/
		static void Shutdown(void);

		/**
		* @brief Queries for a Asset handle of Texture using the default Texture settings
		* @param filepath A C-style string containg the filepath (the filepath is also used for the key on the underlying map)
		* @returns A Reference to an Asset
		* @warnings To properly used the returned reference you must check the Type
		*	if the given AssetType is LOADING then you should continue Requesting
		*/
		static Ref<Asset> RequestTexture(const char* filepath);

		/**
		* @brief Queiries for a Asset handle of a Mesh
		* @param filepath  A C-style string containg the filepath (the filepath is also used for the key on the underlying map)
		* @returns A Reference to an Asset
		* @warnings To properly used the returned reference you must check the Type
		*	if the given AssetType is LOADING then you should continue Requesting
		*/
		static Ref<Asset> RequestMesh(const char* filepath);

		/**
		* @brief Destroyes a Texture from GPU memory as well and it's cached data on RAM
		* @param A C-style string containg the filepath (the filepath is also used for the key on the underlying map)
		*/
		static void DeleteTexture(const char* filepath);

		/**
		* @brief Destroys every Asset that is not used by anyone both from RAM and GPU memory
		*/
		static void Clean(void);

		/**
		* @brief Destroys every Asset from RAM and GPU memory
		*/
		static void Clear(void);

	};


}