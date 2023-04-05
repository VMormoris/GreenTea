#pragma once

#include <GreenTea.h>

class MaterialPanel {
public:

	MaterialPanel(void) = default;

	void Draw(void);

	void SetMaterial(const gte::uuid& id);

private:

	void SaveMaterial(void);

private:

	gte::uuid mMaterialID;
	gte::Material mMaterial;
	glm::vec3 mRotation = { 0.0f, 0.0f, 0.0f };
};