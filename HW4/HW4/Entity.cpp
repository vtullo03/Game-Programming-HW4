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
    m_speed = 0;
}

/*
* Update function specifically for the ENTITY class
* Checks collision in all cardinal directions
* Then calculates physics
* Then updates transformations
* 
* @param delta_time, float that's the value of real-life time in seconds
* @param player, the player ENTITY -- mainly used by enemies
* @param collidable_entities, an array of entities that this ENTITY can collide with
* @param collidable_entity_count, size of the array mentioned above
*/
void Entity::update(float delta_time, Entity* player, Entity* collidable_entities, int collidable_entity_count)
{
    // if not active -- then can't update, treat like deletion
    if (!m_is_active) return;

    m_collided_top = false;
    m_collided_bottom = false;
    m_collided_left = false;
    m_collided_right = false;

    // velocity equation in gaming
    m_velocity += m_acceleration * delta_time;

    // must be calculated seperatedly for seperate collisions
    m_position.y += m_velocity.y * delta_time;
    check_collision_y(collidable_entities, collidable_entity_count);

    m_position.x += m_velocity.x * delta_time;
    check_collision_x(collidable_entities, collidable_entity_count);

    // reset model before every change
    m_model_matrix = glm::mat4(1.0f);
    m_model_matrix = glm::translate(m_model_matrix, m_position);
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
* Checks for collisions in the y-axis
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
* Render function specifically for the ENTITY class
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