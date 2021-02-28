#pragma once
#include "GreenTea.h" 

using namespace GTE;

class GAME_API HorizontalMovingPlatform : public ScriptableEntity {

public: 

	virtual void Start(void) override;

	virtual void FixedUpdate(void) override;

};