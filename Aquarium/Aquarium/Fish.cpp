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
    currentSpeed = 0.01f;
    InitFishMovements();
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
    currentSpeed = 0.01f;
    currentYaw = 0.0f;
    m_timeLeft = 3.0f;
    InitFishMovements();
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
        currentSpeed = 0.001f; // Increase speed to be more noticeable
        break;
    case EFishMovementType::BACKWARD:
        currentSpeed = -0.001f;
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
    // Check if the fish is currently in motion and if there's time left to move
    if (currentSpeed != 0.0f && m_timeLeft > 0.0f) {
        // Calculate the distance the fish should move in the current frame
        float distance = currentSpeed * deltaTime;
        glm::vec3 movement = m_forward * distance;

        // Update the fish's position
        m_position += movement;

        // Reduce the remaining movement time
        m_timeLeft -= deltaTime;

        // If the remaining movement time becomes non-positive, stop the fish
        if (m_timeLeft <= 0.0f) {
            currentSpeed = 0.0f; // Reset speed to zero
            m_timeLeft = 0.0f; // Ensure time left is not negative
        }
    }
}

void Fish::SetDirection(EFishMovementType direction) {
    switch (direction) {
    case EFishMovementType::FORWARD:
        // Set forward direction based on current yaw and pitch
        m_forward = glm::vec3(0.0f, 0.0f, 1.0f);
        break;
    case EFishMovementType::UP:
        m_pitch = std::min(m_pitch + 1.0f, 45.0f);
        m_forward = glm::vec3(0.0f, 1.0f, 0.0f);
        break;
    case EFishMovementType::DOWN:
        m_forward = glm::vec3(0.0f, -1.0f, 0.0f);
        break;
    case EFishMovementType::LEFT:
        m_yaw -= 1.0f;
        if (m_yaw < 0.0f) m_yaw += 360.0f;
        UpdateFishVectors();
        break;
    case EFishMovementType::RIGHT:
        m_yaw += 1.0f;
        if (m_yaw >= 360.0f) m_yaw -= 360.0f;
        UpdateFishVectors();
        break;
    }

    // Update direction vectors based on the yaw and pitch for all directions
    UpdateFishVectors();
}



void Fish::StartNewMovement(float totalTime, EFishMovementType direction) {
    m_fishMovementTimer = totalTime;
    m_timeLeft = totalTime;

    // Set the direction based on the input parameter
    SetDirection(direction);

    // Set a new speed
    SetSpeed(0.1f); // You can adjust the speed as needed
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

void Fish::InitFishMovements()
{
    int prev = -1;
    for(int i = 0; i< 20; i++)
    {
        int move;
        do
        {
            move = rand() % 5;
            if (move == 1 && prev != 2)
                break;
            if (move == 2 && prev != 1)
                break;
            if (move == 3 && prev != 4)
                break;
            if (move == 4 && prev != 3)
                break;
        } while (true);
        prev = move;
        switch(move)
        {
        case 0:
            m_fishMovements.push_back(EFishMovementType::FORWARD);
            break;
        case 1:
            m_fishMovements.push_back(EFishMovementType::LEFT);
            break;
        case 2:
            m_fishMovements.push_back(EFishMovementType::RIGHT);
            break;
        case 3:
            m_fishMovements.push_back(EFishMovementType::UP);
            break;
        case 4:
            m_fishMovements.push_back(EFishMovementType::DOWN);
            break;
        }
    }
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

EFishMovementType Fish::GetMove(int index)
{
    return m_fishMovements[index % 20];
}
