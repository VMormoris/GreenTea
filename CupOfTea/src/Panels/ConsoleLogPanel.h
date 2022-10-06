#pragma once

#include <GreenTea.h>

class ConsoleLogPanel {
public:
	void Draw(void);
private:
	void DrawLine(gte::internal::Logger::Type type, const std::string& msg);
private:
	gte::internal::Logger::Type mFilter = gte::internal::Logger::Type::NONE;
};