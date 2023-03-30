#include "Geometry.h"

namespace gte {
	
	int32 Geometry::FindMaterial(const std::string& name)
	{
		for (int32 i = 0; i < (int32)Materials.size(); i++)
		{
			let& mat = Materials[i];
			if (mat.Name.compare(name) == 0)
				return i;
		}
		return -1;
	}

}