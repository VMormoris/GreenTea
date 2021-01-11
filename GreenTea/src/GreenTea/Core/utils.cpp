#include "utils.h"
#include <fstream>

namespace GTE::utils {

	std::string strip_path(const std::string& filepath)
	{
		size_t offset = filepath.find_last_of("/\\");
		if (offset == filepath.size()) return filepath;
		else return filepath.substr(offset + 1);
	}

	const char* readfile(const char* filepath)
	{
		std::ifstream in(filepath, std::ifstream::ate | std::ifstream::binary);
		if (!in.is_open())
			return nullptr;

		size_t length = in.tellg();
		in.seekg(0, in.beg);

		char* buffer = new char[length + 1];
		in.read(buffer, length);
		buffer[length] = '\0';

		if (!in) {
			delete[]buffer;
			buffer = nullptr;
		}

		in.close();
		return buffer;
	}

	void writefile(const char* filepath, const std::string& str)
	{
		std::ofstream out(filepath);
		if (!out.is_open())
			return;
		out.write(str.c_str(), str.size());
		out.close();
	}

}