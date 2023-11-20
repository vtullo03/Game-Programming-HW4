/**
* Author: Vitoria Tullo
* Assignment: Rise of the AI
* Date due: 2023-11-18, 11:59pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/

#define GL_SILENCE_DEPRECATION
#define STB_IMAGE_IMPLEMENTATION

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#define GL_GLEXT_PROTOTYPES 1
#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "Entity.h"


/*
    The ENTITY Class's Default constructor
*/
Entity::Entity()
{
    // position and tranformation variables
    m_position = glm::vec3(0.0f);
    m_model_matrix = glm::mat4(1.0f);

    // physics variables
    m_velocity = glm::vec3(0.0f);
    m_acceleration = glm::vec3(0.0f);
    m_movement = glm::vec3(0.0f);

    // ENTITY-specific variables
    current_speed = 0;
}

/*
* Update function specifically for the ENTITY class
* Checks collision in all cardinal directions
* Then calculates physics
* Then updates transformations
* 
* @param delta_time, float that's the value of real-life time in seconds
* @param player, the player ENTITY -- mainly used by enemies
* @param objects, an array of entities that this ENTITY can collide with
* @param object_count, size of the array mentioned above
* @param map, the level's MAP object that the entity can collide with
*/
void Entity::update(float delta_time, Entity* player, Entity* objects, int object_count, Map* map)
{
    // if not active -- then can't update, treat like deletion
    if (!m_is_active) return;
    if (m_entity_type == ENEMY) ai_activate(player, delta_time);

    m_collided_top = false;
    m_collided_bottom = false;
    m_collided_left = false;
    m_collided_right = false;

    m_velocity.x = m_movement.x * current_speed;
    m_velocity += m_acceleration * delta_time; // velocity equation implemented in code

    // must be calculated seperatedly for seperate collisions
    m_position.y += m_velocity.y * delta_time;
    check_collision_y(objects, object_count);
    check_collision_y(map);

    m_position.x += m_velocity.x * delta_time;
    check_collision_x(objects, object_count);
    check_collision_x(map);

    // reset model before every change
    m_model_matrix = glm::mat4(1.0f);
    m_model_matrix = glm::translate(m_model_matrix, m_position);

    // ––––– JUMPING ––––– //
    if (m_is_jumping)
    {
        m_is_jumping = false;
        m_velocity.y += m_jumping_power;
    }

    switch (movement_state)
    {
    case WALK:
        current_speed = m_walk_speed;
        break;
    case SPRINT:
        current_speed = m_sprint_speed;
        break;
    case SNEAK:
        current_speed = m_sneak_speed;
        break;
    default:
        break;
    };
}


/*
* Checks for collisions in the y-axis
* Iterates through all the entities that are collidable and checks if
* touching above or below.
* 
* @param collidable_entities, an array of all entities that this ENTITY can collide with
* @param collidable_entity_count, size of the array above
* 
* TREAT LIKE ON_COLLISION_ENTER
*/
void const Entity::check_collision_y(Entity* collidable_entities, int collidable_entity_count)
{
    for (int i = 0; i < collidable_entity_count; i++)
    {
        Entity* collidable_entity = &collidable_entities[i];

        if (check_collision(collidable_entity))
        {
            float y_distance = fabs(m_position.y - collidable_entity->get_position().y);
            float y_overlap = fabs(y_distance - (m_height / 2.0f) - (collidable_entity->get_height() / 2.0f));
            if (m_velocity.y > 0) {
                m_position.y -= y_overlap;
                m_velocity.y = 0;
                m_collided_top = true;
            }
            else if (m_velocity.y < 0) {
                m_position.y += y_overlap;
                m_velocity.y = 0;
                m_collided_bottom = true;
            }
        }
    }
}

/*
* Check for collisions with the map in the y-axis
*
* @param map, MAP object that the ENTITY object is colliding with
*/
void const Entity::check_collision_y(Map* map)
{
    // Check all tiles above, including left and right for corner interaction
    glm::vec3 top = glm::vec3(m_position.x, m_position.y + (m_height / 2), m_position.z);
    glm::vec3 top_left = glm::vec3(m_position.x - (m_width / 2), m_position.y + (m_height / 2), m_position.z);
    glm::vec3 top_right = glm::vec3(m_position.x + (m_width / 2), m_position.y + (m_height / 2), m_position.z);

    // Check all tiles belove, including left and right for corner interaction
    glm::vec3 bottom = glm::vec3(m_position.x, m_position.y - (m_height / 2), m_position.z);
    glm::vec3 bottom_left = glm::vec3(m_position.x - (m_width / 2), m_position.y - (m_height / 2), m_position.z);
    glm::vec3 bottom_right = glm::vec3(m_position.x + (m_width / 2), m_position.y - (m_height / 2), m_position.z);

    float penetration_x = 0;
    float penetration_y = 0;

    // Logic if tiles are detected, stop all velocity and flag collision
    if (map->is_solid(top, &penetration_x, &penetration_y) && m_velocity.y > 0)
    {
        m_position.y -= penetration_y;
        m_velocity.y = 0;
        m_collided_top = true;
    }
    else if (map->is_solid(top_left, &penetration_x, &penetration_y) && m_velocity.y > 0)
    {
        m_position.y -= penetration_y;
        m_velocity.y = 0;
        m_collided_top = true;
    }
    else if (map->is_solid(top_right, &penetration_x, &penetration_y) && m_velocity.y > 0)
    {
        m_position.y -= penetration_y;
        m_velocity.y = 0;
        m_collided_top = true;
    }

    if (map->is_solid(bottom, &penetration_x, &penetration_y) && m_velocity.y < 0)
    {
        m_position.y += penetration_y;
        m_velocity.y = 0;
        m_collided_bottom = true;
    }
    else if (map->is_solid(bottom_left, &penetration_x, &penetration_y) && m_velocity.y < 0)
    {
        m_position.y += penetration_y;
        m_velocity.y = 0;
        m_collided_bottom = true;
    }
    else if (map->is_solid(bottom_right, &penetration_x, &penetration_y) && m_velocity.y < 0)
    {
        m_position.y += penetration_y;
        m_velocity.y = 0;
        m_collided_bottom = true;

    }
}

/*
* Checks for collisions with other ENTITY objects in the x-axis
* Iterates through all the entities that are collidable and checks if
* touching to left or right.
*
* @param collidable_entities, an array of all entities that this ENTITY can collide with
* @param collidable_entity_count, size of the array above
* 
* TREAT LIKE ON_COLLISION_ENTER
*/
void const Entity::check_collision_x(Entity* collidable_entities, int collidable_entity_count)
{
    for (int i = 0; i < collidable_entity_count; i++)
    {
        Entity* collidable_entity = &collidable_entities[i];

        if (check_collision(collidable_entity))
        {
            if (collidable_entity->get_entity_type() == PLAYER && m_entity_type == ENEMY)
            {
                collidable_entity->is_dead = true;
                collidable_entity->m_is_active = false;
            }
            if (collidable_entity->get_entity_type() == ENEMY && m_entity_type == WEAPON)
            {
                collidable_entity->is_dead = true;
                collidable_entity->m_is_active = false;
            }
            float x_distance = fabs(m_position.x - collidable_entity->get_position().x);
            float x_overlap = fabs(x_distance - (m_width / 2.0f) - (collidable_entity->get_width() / 2.0f));
            if (m_velocity.x > 0) {
                m_position.x -= x_overlap;
                m_velocity.x = 0;
                m_collided_right = true;
            }
            else if (m_velocity.x < 0) {
                m_position.x += x_overlap;
                m_velocity.x = 0;
                m_collided_left = true;
            }
        }
    }
}

/*
* Check for collisions with the map in the x-axis
* 
* @param map, MAP object that the ENTITY object is colliding with
*/
void const Entity::check_collision_x(Map* map)
{
    // Check if touching tile
    glm::vec3 left = glm::vec3(m_position.x - (m_width / 2), m_position.y, m_position.z);
    glm::vec3 right = glm::vec3(m_position.x + (m_width / 2), m_position.y, m_position.z);

    float penetration_x = 0;
    float penetration_y = 0;

    if (map->is_solid(left, &penetration_x, &penetration_y) && m_velocity.x < 0)
    {
        m_position.x += penetration_x;
        m_velocity.x = 0;
        m_collided_left = true;
    }
    if (map->is_solid(right, &penetration_x, &penetration_y) && m_velocity.x > 0)
    {
        m_position.x -= penetration_x;
        m_velocity.x = 0;
        m_collided_right = true;
    }
}

/*
* Render function specifically for the ENTITY class
* 
* @param program, reference to the SHADERPROGRAM class -- to use it's functions
*/
void Entity::render(ShaderProgram* program)
{
    program->set_model_matrix(m_model_matrix);

    // if not active -- then can't render, treat like deletion
    if (!m_is_active) { return; }

    float vertices[] = { -0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5 };
    float tex_coords[] = { 0.0,  1.0, 1.0,  1.0, 1.0, 0.0,  0.0,  1.0, 1.0, 0.0,  0.0, 0.0 };

    glBindTexture(GL_TEXTURE_2D, m_texture_id);

    glVertexAttribPointer(program->get_position_attribute(), 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(program->get_position_attribute());
    glVertexAttribPointer(program->get_tex_coordinate_attribute(), 2, GL_FLOAT, false, 0, tex_coords);
    glEnableVertexAttribArray(program->get_tex_coordinate_attribute());

    glDrawArrays(GL_TRIANGLES, 0, 6);

    glDisableVertexAttribArray(program->get_position_attribute());
    glDisableVertexAttribArray(program->get_tex_coordinate_attribute());
}

/*
* General check collision function for static object
* 
* @param other, the other ENTITY class that is being collided with
* 
* TREAT LIKE NORMAL PHYSICS COLLISION -- STOPS OBJECTS
*/
bool const Entity::check_collision(Entity* other) const
{
    if (other == this) return false;
    // If either entity is inactive, there shouldn't be any collision
    if (!m_is_active || !other->m_is_active) return false;

    float x_distance = fabs(m_position.x - other->m_position.x) - ((m_width + other->m_width) / 2.0f);
    float y_distance = fabs(m_position.y - other->m_position.y) - ((m_height + other->m_height) / 2.0f);

    return x_distance < 0.0f && y_distance < 0.0f;
}

// AI SCRIPTS HERE

/*
* Activates each enemies' AI
* 
* @param player, the ENTITY object that represents the player
* so that enemies can affect the player (follow, kill,etc.)
* @param delta_time, the real life time in seconds
* for enemies that incorporate cooldowns
*/
void Entity::ai_activate(Entity* player, float delta_time)
{
    switch (m_ai_type)
    {
    case FREDDY:
        ai_teleport(player, delta_time);
        break;

    case BONNIE:
        ai_patrol(player, delta_time);
        break;

    case CHICA:
        ai_stealth_activate(player);
        break;

    case FOXY:
        ai_peekaboo(player);
        break;

    default:
        break;
    }
}

/*
* Used by the BONNIE enemy
* Immiedately goes into the patroling state and starts a countdown
* When the countdown ends switch directions and reset countdown
* When the player gets into line of sight, enter the chasing state
* 
* @param player, the player ENTITY object
* @param delta_time, real life time in seconds
*/
void Entity::ai_patrol(Entity* player, float delta_time)
{
    switch (m_ai_state)
    {
    case IDLE:
        is_facing_right = true;
        m_ai_state = PATROLING;

    case PATROLING:
        movement_state = WALK;
        current_speed = m_walk_speed;
        if (is_facing_right)
        {
            m_movement = glm::vec3(1.0f, 0.0f, 0.0f);
        }
        else
        {
            m_movement = glm::vec3(-1.0f, 0.0f, 0.0f);
        }

        ability_timer -= delta_time;
        if (ability_timer <= 0.0f)
        {
            ability_timer = 2.0f;
            is_facing_right = !is_facing_right;
        }

        if ((glm::abs(m_position.x - player->get_position().x) < 0.25f) 
            && (m_position.y == player->get_position().y) &&
            is_facing_right == player->is_facing_right)
        {
            m_ai_state = CHASING;
        }

    case CHASING:
        movement_state = SPRINT;
        current_speed = m_sprint_speed;
        if (is_facing_right)
        {
            m_movement = glm::vec3(1.0f, 0.0f, 0.0f);
        }
        else
        {
            m_movement = glm::vec3(-1.0f, 0.0f, 0.0f);
        }
    }
}

/*
* Used by the CHICA enemy
* Idle until the player is within range
* Even when the player is within range - stay idle if the player is sneaking
* Otherwise enter the chasing state
* 
* @param player, the player ENTITY object
*/
void Entity::ai_stealth_activate(Entity* player)
{
    switch (m_ai_state)
    {
    case IDLE: 
        if ((glm::abs(m_position.x - player->get_position().x) < 2.0f)
            && (m_position.y == player->get_position().y))
        {
            if (player->get_player_state() == SNEAK) break;
            else m_ai_state = CHASING;
        }
        break;

    case CHASING:
        movement_state = SPRINT;
        current_speed = m_sprint_speed;
        if (m_position.x > player->get_position().x) {
            m_movement = glm::vec3(-1.0f, 0.0f, 0.0f);
        }
        else {
            m_movement = glm::vec3(1.0f, 0.0f, 0.0f);
        }
        break;
    }    
}

/*
* Used by the FOXY enemy
* If the player is moving in Foxy's direction then stay idle
* When the player is moving away from Foxy, than Foxy states moving
* in the player's direction
* Inspired by the Boos from Mario
*/
void Entity::ai_peekaboo(Entity* player)
{
    switch (m_ai_state)
    {
    case IDLE: 
        m_movement = glm::vec3(0.0f, 0.0f, 0.0f);
        if (player->is_facing_right == false) m_ai_state = CHASING;
        break;

    case CHASING:
        movement_state = SPRINT;
        current_speed = m_sprint_speed;
        m_movement = glm::vec3(-1.0f, 0.0f, 0.0f);
        if (player->is_facing_right == true) m_ai_state = IDLE;
        break;
    }
}

/*
* Used by the Freddy enemy
* Immediately go into the patroling state
* When patroling, start a countdown
* When the countdown is over teleport randomly to one of the set locations
* Restart countdown
* Inspired by Freddy's movement in the original FNAF
*/
void Entity::ai_teleport(Entity* player, float delta_time)
{
    std::vector<glm::vec3> positions =
    { glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(7.75f, 0.0f, 0.0f), glm::vec3(12.0f, 0.0f, 0.0f) };

    switch (m_ai_state)
    {
    case IDLE:
        m_ai_state = PATROLING;
        break;
    case PATROLING:
        ability_timer -= delta_time;
        if (ability_timer <= 0.0f)
        {
            int random_position = rand() % 3;
            set_position(positions[random_position]);
            std::cout << get_position().x << std::endl;
            ability_timer = 2.0f;
        }
    default:
        break;
    }
}