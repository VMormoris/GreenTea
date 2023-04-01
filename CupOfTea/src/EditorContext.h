#pragma once

#include <GreenTea.h>

struct EditorContext {

	gte::Geometry* SphereGeometry = nullptr;
	gte::GPU::Texture* TransparencyGrid = nullptr;
};

[[nodiscard]] EditorContext* GetEditorContext(void);
EditorContext* CreateEditorContext(void);
void DestroyEditorContext(void);