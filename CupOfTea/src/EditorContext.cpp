#include "EditorContext.h"
#include "OBJLoader.h"

static EditorContext* sContext = nullptr;

EditorContext* CreateEditorContext(void)
{
	sContext = new EditorContext();

	gte::Image img("../Assets/Icons/Transparency.png");
	sContext->TransparencyGrid = gte::GPU::Texture2D::Create(img);
	
	sContext->SphereGeometry = OBJLoader{}.Load("../Assets/Shapes/Sphere.obj");
	return sContext;
}

[[nodiscard]] EditorContext* GetEditorContext(void) { return sContext; }
void DestroyEditorContext(void) { if (sContext) delete sContext; sContext = nullptr; }
