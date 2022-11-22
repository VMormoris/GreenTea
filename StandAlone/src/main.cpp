#include "StandAlone.h"

#define GREEN_TEA_ENTRY_POINT
#include <Engine/Core/EntryPoint.h>

gte::Application* CreateApplication(int argc, char** argv) { return new StandAlone(); }