enum EntityType { PLAYER, PLATFORM, ENEMY, WEAPON };
enum AIState { IDLE, PATROLING, CHASING };
enum AIType { FREDDY, BONNIE, CHICA, FOXY };
enum PlayerState { WALK, SPRINT, SNEAK };

#include "Map.h";

class Entity {
private:
    // position and tranformation variables
    glm::vec3 m_position;
    glm::mat4 m_model_matrix;

    // physics variables
    glm::vec3 m_velocity;
    glm::vec3 m_acceleration;
    glm::vec3 m_movement;

    // specific to different entity types
    float m_walk_speed;
    float m_sprint_speed;
    float m_sneak_speed;
    float current_speed;
    float m_jumping_power = 8.0f;
    float m_width = 1;
    float m_height = 1;

    EntityType m_entity_type; // type of entity - treat as NAME

    bool m_is_active = true;

    // PLAYER MOVEMENT STATE
    PlayerState movement_state;

    // ENEMY AI
    AIType     m_ai_type;
    AIState    m_ai_state;

public:
    GLuint m_texture_id; // texture

    // physics - collision for all directions
    bool m_collided_top = false;
    bool m_collided_bottom = false;
    bool m_collided_left = false;
    bool m_collided_right = false;

    bool is_facing_right = true;
    bool is_dead = false;
    float ability_timer = 2.0f;
    bool m_is_jumping = false;

    // default constructor
    Entity();

    void update(float delta_time, Entity* player, Entity* objects, int object_count, Map* map);
    void render(ShaderProgram* program);

    // collisions - both in the x and y axis
    bool const check_collision(Entity* other) const;
    void const check_collision_y(Entity* collidable_entities, int collidable_entity_count);
    void const check_collision_y(Map* map);
    void const check_collision_x(Entity* collidable_entities, int collidable_entity_count);
    void const check_collision_x(Map* map);

    // ai scripts -- also located at bottom of .cpp file
    void ai_activate(Entity* player, float delta_time);
    void ai_teleport(Entity* player, float delta_time); // freddy
    void ai_patrol(Entity* player, float delta_time); // bonnie
    void ai_stealth_activate(Entity* player); // chica
    void ai_peekaboo(Entity* player); // foxy

    void activate() { m_is_active = true; };
    void deactivate() { m_is_active = false; };

    // movement
    void move_left() { m_movement.x = -1.0f; }
    void move_right() { m_movement.x = 1.0f; }

    // GETTERS
    EntityType const get_entity_type()    const { return m_entity_type; };
    glm::vec3  const get_position()       const { return m_position; };
    glm::vec3  const get_movement()       const { return m_movement; };
    glm::vec3  const get_velocity()       const { return m_velocity; };
    glm::vec3  const get_acceleration()   const { return m_acceleration; };
    int        const get_width()          const { return m_width; };
    int        const get_height()         const { return m_height; };
    PlayerState const get_player_state() const { return movement_state; }
    AIType     const get_ai_type()        const { return m_ai_type; };
    AIState    const get_ai_state()       const { return m_ai_state; };

    // SETTLERS
    void const set_entity_type(EntityType new_entity_type) { m_entity_type = new_entity_type; };
    void const set_position(glm::vec3 new_position) { m_position = new_position; };
    void const set_movement(glm::vec3 new_movement) { m_movement = new_movement; };
    void const set_velocity(glm::vec3 new_velocity) { m_velocity = new_velocity; };
    void const set_speeds(float new_walk, float new_sprint, float new_sneak)
    {
        m_walk_speed = new_walk;
        m_sprint_speed = new_sprint;
        m_sneak_speed = new_sneak;
    }
    void const set_acceleration(glm::vec3 new_acceleration) { m_acceleration = new_acceleration; };
    void const set_width(float new_width) { m_width = new_width; };
    void const set_height(float new_height) { m_height = new_height; };
    void const set_movement_state(PlayerState new_player_state) { movement_state = new_player_state; };
    void const set_ai_type(AIType new_ai_type) { m_ai_type = new_ai_type; };
    void const set_ai_state(AIState new_state) { m_ai_state = new_state; };
};