#ifndef _PANELS
#define _PANELS

#include <GreenTea.h>

namespace GTE {

	void RenderLogPanel(void);
	void RenderPropertiesPanel(Entity entity);
	void RenderMaterialsPanel(std::vector<Material>& materials, int32& index);

}
#endif