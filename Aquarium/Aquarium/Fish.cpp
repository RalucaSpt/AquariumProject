#include "Fish.h"

Fish::Fish(const glm::vec3& initialPos, Model* model)
{
	m_position = initialPos;
	m_fish = model;
	grounded = true;

	//UpdateFishVectors();
}

void Fish::SetPos(const glm::vec3& pos)
{
	m_position = pos;
}

glm::vec3 Fish::GetPos() const
{
	return m_position;
}

void Fish::setModel(Model* model)
{
	m_fish = model;
}

Model* Fish::getModel()
{
	return m_fish;
}

void Fish::Move(EFishMovementType direction)
{
	switch (direction)
	{
	case EFishMovementType::FORWARD:
		currentSpeed = 0.001f;
		break;
	case EFishMovementType::BACKWARD:
		currentSpeed = -0.001f;
		break;
	case EFishMovementType::RIGHT:
		if (currentSpeed >= 0.1f)
		{
			m_yaw += 0.1f;
			if (m_yaw >= 360)
				m_yaw -= 360;

			currentYaw = 0.1f;
		}
		break;
	case EFishMovementType::LEFT:
		if (currentSpeed >= 0.1f)
		{
			m_yaw -= 0.1f;
			if (m_yaw < 0)
				m_yaw = 360;

			currentYaw = -0.1f;
		}
		break;
	case EFishMovementType::UP:
		if (currentSpeed >= 0.001f)
		{
			m_pitch = std::min(m_pitch + 0.1f, 45.f);
		}

		break;
	case EFishMovementType::DOWN:
		if (currentSpeed >= 0.001f)
		{
			m_pitch = std::max(m_pitch - 0.1f, -45.f);
		}
		break;
	}
	UpdateFishVectors();
}

float Fish::GetYaw() const
{
	return m_yaw;
}

void Fish::MoveFish(float deltaTime)
{
	float velocity = (float)(deltaTime);

	m_position += m_forward * velocity * currentSpeed;

	m_position += m_right * velocity * currentYaw;

	//m_position += m_up  * m_pitch;

	currentYaw = 0.f;
}

void Fish::UpdateFishVectors()
{
	float runwayz1 = 650, runwayz2 = 2650;
	float runwayx1 = -11100, runwayx2 = -10960;
	float runwayy = 1770;

	if (runwayx1 <= m_position.x && m_position.x <= runwayx2 &&
		runwayz1 <= m_position.z && m_position.z <= runwayz2 &&
		abs(runwayy - m_position.y) < 10)
	{
		if (glfwGetTime() - takeoffTimer >= takeoffCooldown)
		{
			grounded = true;
			m_position.y = runwayy - 0.5;
			m_pitch = 0;
		}
	}
	else
	{
		if (grounded)
			grounded = false;
	}

	float x = glm::radians(m_yaw);
	float y = glm::radians(m_pitch);
	float z = glm::radians(m_roll);

	m_forward.x = cos(x) * cos(y);
	m_forward.y = sin(y);
	m_forward.z = sin(x) * cos(y);
	m_forward = glm::normalize(m_forward);

	m_right = glm::normalize(glm::cross(m_forward, m_worldUp));
	m_up = glm::normalize(glm::cross(m_right, m_forward));
}

float Fish::GetPitch() const
{
	return m_pitch;
}

float Fish::GetRoll() const
{
	return m_roll;
}

float Fish::GetSpeed() const
{
	return currentSpeed;
}

void Fish::SetSpeed(float speed)
{
	currentSpeed = speed;
}