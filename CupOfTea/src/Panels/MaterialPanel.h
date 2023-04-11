#pragma once

#include <GreenTea.h>

class MaterialPanel {
public:

	MaterialPanel(void) = default;

	void Draw(void);

	void SetMaterial(const gte::uuid& id);

private:

	void DrawProperty(const char* label, gte::Ref<gte::Asset>& asset);
	void DrawProperty(const char* label, gte::Ref<gte::Asset>& asset, float& value, const char* help = "");
	void DrawProperty(const char* label, gte::Ref<gte::Asset>& asset, glm::vec3& color, const char* help = "", float resetVal = 1.0f);
	void DrawImageControl(gte::Ref<gte::Asset>& asset);
	void SaveMaterial(void);

private:

	gte::uuid mMaterialID;
	gte::Material mMaterial;
	glm::vec3 mRotation = { 0.0f, 0.0f, 0.0f };
};