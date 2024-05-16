#pragma once
#include "Model.h"
#include "glfw3.h"

enum class EFishMovementType
{
	UNKNOWN,
	FORWARD,
	BACKWARD,
	LEFT,
	RIGHT,
	UP,
	DOWN
};


class Fish
{
public:
	float m_timeLeft;

	Fish()=default;
	Fish(const Fish& other);
	Fish(const glm::vec3& initialPos, Model* model = nullptr);
	//override equal operator
	Fish& operator=(const Fish& other);

	void SetPos(const glm::vec3& pos);
	glm::vec3 GetPos() const;

	void setModel(Model* model);
	Model* getModel();


	void Move(EFishMovementType direction);

	void MoveFish(float deltaTime);

	void SetDirection(EFishMovementType direction);
	void StartNewMovement(float totalTime);

	float GetFishSize() const;
	float GetFishMovementTimer() const;

	void SetFishSize(float size);
	void SetFishMovementTimer(float timer);

	float GetYaw() const;

	float GetPitch() const;

	float GetRoll() const;

	float GetSpeed() const;

	void SetSpeed(float speed);
private:
	void UpdateFishVectors();


	Model* m_fish;
	glm::vec3 m_position;
	glm::vec3 m_forward{ 0.f, 0.f, 1.f };
	glm::vec3 m_right{ 0.f, 1.f, 0.f };
	glm::vec3 m_up{ 0.f, 0.f, 1.f };
	glm::vec3 m_worldUp = { 0.f, 1.f, 0.f };

	float m_yaw = 90.0f;
	float m_pitch = 0.0f;
	float m_roll = 0.0f;
	const float rollMoveRight = 0.5f;

	float currentSpeed = 0.0f;
	float currentYaw = 0.0f;

	bool grounded;

	float takeoffTimer;
	float takeoffCooldown = 2.f;

	float m_fishMovementTimer = 0.f;

	float m_fishSize = 1.f;
};



