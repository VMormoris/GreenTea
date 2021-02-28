#ifndef _PLAYER
#define _PLAYER

#include <GreenTea.h>

class GAME_API Player : public GTE::ScriptableEntity {
public:

	Player();

	virtual void Start(void) override;
	virtual void Destroy(void) override;

	virtual void FixedUpdate(void) override;
	virtual void Update(float dt) override;

	bool onMouseButtonDown(GTE::MouseButtonType btnType);
	bool onMouseButtonUp(GTE::MouseButtonType btnType);
	bool onMouseMove(int x, int y);

private:

	float m_Velocity = 5.0f;
	float m_AngularFactor = 0.0005f;
	glm::vec2 m_LookAngleDest = { 0.0f, 0.0f };
	glm::vec2 m_CursorPos = { -1.0f, -1.0f };

};
#endif