#pragma once

#include <GreenTea.h>

using namespace gte;

class StandAlone : public Application {
public:
	StandAlone(void);
	~StandAlone(void);

	virtual void Update(float dt) override;

private:

	void RenderLogo(void);

private:

	float mAccumulator = 0.0f;

};