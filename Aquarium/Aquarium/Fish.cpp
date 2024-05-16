#include "Fish.h"

// Constructor for copying an existing Fish
Fish::Fish(const Fish& other)
{
    m_position = other.m_position;
    m_fish = other.m_fish;
    m_forward = other.m_forward;
    m_right = other.m_right;
    m_up = other.m_up;
    m_worldUp = other.m_worldUp;
    m_yaw = other.m_yaw;
    m_pitch = other.m_pitch;
    m_roll = other.m_roll;
    currentSpeed = other.currentSpeed;
    currentYaw = other.currentYaw;
    grounded = other.grounded;
    takeoffTimer = other.takeoffTimer;
    takeoffCooldown = other.takeoffCooldown;
}

// Constructor for initializing a new Fish
Fish::Fish(const glm::vec3& initialPos, Model* model)
{
    m_position = initialPos;
    m_fish = model;
    grounded = true;
    m_worldUp = glm::vec3(0.0f, 1.0f, 0.0f); // Ensure m_worldUp is initialized
    m_yaw = 0.0f;
    m_pitch = 0.0f;
    m_roll = 0.0f;
    currentSpeed = 0.0f;
    currentYaw = 0.0f;
    UpdateFishVectors(); // Initialize direction vectors
}

// Assignment operator
Fish& Fish::operator=(const Fish& other)
{
    if (this != &other)
    {
        m_position = other.m_position;
        m_fish = other.m_fish;
        m_forward = other.m_forward;
        m_right = other.m_right;
        m_up = other.m_up;
        m_worldUp = other.m_worldUp;
        m_yaw = other.m_yaw;
        m_pitch = other.m_pitch;
        m_roll = other.m_roll;
        currentSpeed = other.currentSpeed;
        currentYaw = other.currentYaw;
        grounded = other.grounded;
        takeoffTimer = other.takeoffTimer;
        takeoffCooldown = other.takeoffCooldown;
    }
    return *this;
}

// Set position
void Fish::SetPos(const glm::vec3& pos)
{
    m_position = pos;
}

// Get position
glm::vec3 Fish::GetPos() const
{
    return m_position;
}

// Set model
void Fish::setModel(Model* model)
{
    m_fish = model;
}

// Get model
Model* Fish::getModel()
{
    return m_fish;
}

// Move fish based on direction
void Fish::Move(EFishMovementType direction)
{
    switch (direction)
    {
    case EFishMovementType::FORWARD:
        currentSpeed = 0.0001f; // Increase speed to be more noticeable
        break;
    case EFishMovementType::BACKWARD:
        currentSpeed = -0.0001f;
        break;
    case EFishMovementType::RIGHT:
        m_yaw += 1.0f;
        if (m_yaw >= 360.0f) m_yaw -= 360.0f;
        break;
    case EFishMovementType::LEFT:
        m_yaw -= 1.0f;
        if (m_yaw < 0.0f) m_yaw += 360.0f;
        break;
    case EFishMovementType::UP:
        m_pitch = std::min(m_pitch + 1.0f, 45.0f);
        break;
    case EFishMovementType::DOWN:
        m_pitch = std::max(m_pitch - 1.0f, -45.0f);
        break;
    }
    UpdateFishVectors();
}

// Get yaw
float Fish::GetYaw() const
{
    return m_yaw;
}

// Move fish based on the current speed and yaw
//void Fish::MoveFish(float deltaTime)
//{
//    float velocity = currentSpeed * deltaTime;
//    m_position += m_forward * velocity;
//    // Reset speed after moving
//    currentSpeed *= 0.05f;
//}
void Fish::MoveFish(float deltaTime) {
    // Check if the fish is currently in motion
    if (currentSpeed != 0.0f) {
        // Calculate the distance the fish should move in this frame
        //float distance = currentSpeed * deltaTime;

        // Update the fish position based on the direction and distance
        m_position = m_forward * (m_timeLeft);

        // Reduce the remaining movement time
        m_timeLeft -= deltaTime;

        // If the remaining movement time becomes non-positive, stop the fish
        if (m_timeLeft <= 0.0f) {
            currentSpeed = 0.0f; // Reset speed to zero
        }
    }
}


void Fish::SetDirection(EFishMovementType direction)
{
    switch (direction) {
    case EFishMovementType::FORWARD:
        m_forward = glm::vec3(0.0f, 0.0f, 1.0f);
        break;
    case EFishMovementType::UP:
        m_forward = glm::vec3(0.0f, 1.0f, 0.0f);
        break;
    case EFishMovementType::DOWN:
        m_forward = glm::vec3(0.0f, -1.0f, 0.0f);
        break;
    case EFishMovementType::LEFT:
        m_forward = glm::vec3(-1.0f, 0.0f, 0.0f);
        break;
    case EFishMovementType::RIGHT:
        m_forward = glm::vec3(1.0f, 0.0f, 0.0f);
        break;
    }
}

void Fish::StartNewMovement(float totalTime) {
    m_fishMovementTimer = totalTime;
    m_timeLeft = totalTime;

    // Optionally, set a new direction
    int direction = rand() % 6;
    switch (direction) {
    case 0:
    case 1:
        SetDirection(EFishMovementType::FORWARD);
        break;
    case 2:
        SetDirection(EFishMovementType::UP);
        break;
    case 3:
        SetDirection(EFishMovementType::DOWN);
        break;
    case 4:
        SetDirection(EFishMovementType::LEFT);
        break;
    case 5:
        SetDirection(EFishMovementType::RIGHT);
        break;
    }

    // Set a new speed
    SetSpeed(0.001f);
}


// Get fish size
float Fish::GetFishSize() const
{
    return m_fishSize;
}

// Get fish movement timer
float Fish::GetFishMovementTimer() const
{
    return m_fishMovementTimer;
}

// Set fish size
void Fish::SetFishSize(float size)
{
    m_fishSize = size;
}

// Set fish movement timer
void Fish::SetFishMovementTimer(float timer)
{
    m_fishMovementTimer = timer;
}

// Update direction vectors based on yaw and pitch
void Fish::UpdateFishVectors()
{
    float x = glm::radians(m_yaw);
    float y = glm::radians(m_pitch);

    m_forward.x = cos(x) * cos(y);
    m_forward.y = sin(y);
    m_forward.z = sin(x) * cos(y);
    m_forward = glm::normalize(m_forward);

    m_right = glm::normalize(glm::cross(m_forward, m_worldUp));
    m_up = glm::normalize(glm::cross(m_right, m_forward));
}

// Get pitch
float Fish::GetPitch() const
{
    return m_pitch;
}

// Get roll
float Fish::GetRoll() const
{
    return m_roll;
}

// Get speed
float Fish::GetSpeed() const
{
    return currentSpeed;
}

// Set speed
void Fish::SetSpeed(float speed)
{
    currentSpeed = speed;
}
