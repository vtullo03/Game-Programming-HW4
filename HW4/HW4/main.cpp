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
#define LOG(argument) std::cout << argument << '\n'
#define GL_GLEXT_PROTOTYPES 1
#define FIXED_TIMESTEP 0.0166666f
#define LEVEL1_WIDTH 14
#define LEVEL1_HEIGHT 5
#define ENEMY_COUNT 4

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_mixer.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "stb_image.h"
#include "cmath"
#include <ctime>
#include <vector>
#include <cstdlib>
#include "Entity.h"
#include "Map.h"

struct GameState
{
	Entity* player;
	Entity* enemies;
	Entity* weapons;

	Map* map;
};

// CONSTS
// window dimensions + viewport
const int WINDOW_WIDTH = 640 * 2,
WINDOW_HEIGHT = 480 * 2;

const int VIEWPORT_X = 0,
VIEWPORT_Y = 0,
VIEWPORT_WIDTH = WINDOW_WIDTH,
VIEWPORT_HEIGHT = WINDOW_HEIGHT;

// background color -- CHANGE LATER
const float BG_RED = 0.133f,
BG_BLUE = 0.133f,
BG_GREEN = 0.133f,
BG_OPACITY = 1.0f;

// shaders
const char V_SHADER_PATH[] = "shaders/vertex_textured.glsl",
F_SHADER_PATH[] = "shaders/fragment_textured.glsl";

// texture filepaths
// ENTITIES
const char FREDDY_FILEPATH[] = "Freddy_Placeholder.png",
BONNIE_FILEPATH[] = "Bonnie_Placeholder.png",
CHICA_FILEPATH[] = "Chica_Placeholder.png",
FOXY_FILEPATH[] = "Foxy_Placeholder.png",
PLAYER_FILEPATH[] = "Guard_Placeholder.png",
TRAP_FILEPATH[] = "Trap.png",
// MAPS
MAP_TILESET_FILEPATH[] = "Tileset_Placeholder.png",
// FONT
FONT_FILEPATH[] = "font.png";

// texture constants
const int NUMBER_OF_TEXTURES = 1;
const GLint LEVEL_OF_DETAIL = 0;
const GLint TEXTURE_BORDER = 0;

// math + physics constants
const float MILLISECONDS_IN_SECOND = 1000.0;

// text constants
const int FONTBANK_SIZE = 16;

// GLOBAL
// game state and finished status
GameState g_state;
bool game_completed = false;

SDL_Window* g_display_window;
bool g_game_is_running = true;

ShaderProgram g_shader_program;
glm::mat4 g_view_matrix, g_projection_matrix;

float g_previous_ticks = 0.0f;
float g_accumulator = 0.0f;

// weapon variables
bool trap_placed = false;

unsigned int LEVEL_1_DATA[] =
{
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	3, 3, 3, 3, 3, 3, 3, 3, 2, 2, 2, 2, 2, 2
};

// helpers
GLuint load_texture(const char* filepath);
void init_platform(Entity& entity, glm::vec3 position,
	EntityType type, GLuint& texture);
void init_enemy(Entity& enemy, AIType animatronic,
	const char texture_name[], glm::vec3 position);
void draw_text(ShaderProgram* program, GLuint font_texture_id, std::string text,
	float screen_size, float spacing, glm::vec3 position);
// for game program
void initialise();
void process_input();
void update();
void render();
void shutdown();

// ––––– GAME LOOP ––––– //
int main(int argc, char* argv[])
{
	initialise(); // initailize all game objects and code -- runs ONCE

	while (g_game_is_running)
	{
		process_input(); // get input from player
		update(); // update the game state, run every frame
		render(); // show the game state (after update to show changes in game state)
	}

	shutdown(); // close game safely
	return 0;
}

/*
* Loads a texture to be used for each sprite
* 
* @param filepath, an array of chars that represents the text name
  of the filepath of the texture for the sprite
*/
GLuint load_texture(const char* filepath)
{
	// Load image file from filepath
	int width, height, number_of_components;
	unsigned char* image = stbi_load(filepath, &width, &height, &number_of_components, STBI_rgb_alpha);

	// Throw error if no image found at filepath
	if (image == NULL)
	{
		LOG(" Unable to load image. Make sure the path is correct.");
		assert(false);
	}

	// Generate and bind texture ID to image
	GLuint textureID;
	glGenTextures(NUMBER_OF_TEXTURES, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexImage2D(GL_TEXTURE_2D, LEVEL_OF_DETAIL, GL_RGBA, width, height, TEXTURE_BORDER, GL_RGBA, GL_UNSIGNED_BYTE, image);

	// Setting up texture filter parameters
	// NEAREST better for pixel art
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	// Release from memory and return texture id
	stbi_image_free(image);

	return textureID;
}

/*
* Initialises an ENEMY ENTITY object
* 
* @param enemy, the ENEMY ENTITY object
* @param AITYPE, what animatronic they are
* @param texture_name[], the name of the sprite they have
* @param position, the position the enemy is going to spawn at
*/
void init_enemy(Entity& enemy, AIType animatronic, 
	const char texture_name[], glm::vec3 position)
{
	enemy.set_entity_type(ENEMY);
	enemy.set_ai_type(animatronic);
	enemy.m_texture_id = load_texture(texture_name);
	enemy.set_position(position);
	enemy.set_movement(glm::vec3(0.0f));
	enemy.set_speeds(.50f, 2.0f, 0.25f);
	enemy.set_acceleration(glm::vec3(0.0f, -9.81f, 0.0f));
	enemy.set_ai_state(IDLE);
}

/*
* Initialises all objects in the game -- only runs the first frame
*/
void initialise()
{
	// create window
	SDL_Init(SDL_INIT_VIDEO);
	g_display_window = SDL_CreateWindow("HW 4!!!!!!",
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		WINDOW_WIDTH, WINDOW_HEIGHT,
		SDL_WINDOW_OPENGL);

	SDL_GLContext context = SDL_GL_CreateContext(g_display_window);
	SDL_GL_MakeCurrent(g_display_window, context);

	// for windows machines
#ifdef _WINDOWS
	glewInit();
#endif

	glViewport(VIEWPORT_X, VIEWPORT_Y, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);
	g_shader_program.load(V_SHADER_PATH, F_SHADER_PATH);

	g_view_matrix = glm::mat4(1.0f);
	g_projection_matrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);

	g_shader_program.set_projection_matrix(g_projection_matrix);
	g_shader_program.set_view_matrix(g_view_matrix);
	g_view_matrix = glm::translate(g_view_matrix, glm::vec3(-5.0f, 0.75f, 0.0f));

	glUseProgram(g_shader_program.get_program_id());

	glClearColor(BG_RED, BG_BLUE, BG_GREEN, BG_OPACITY);

	// MAP
	GLuint map_texture_id = load_texture(MAP_TILESET_FILEPATH);
	g_state.map = new Map(LEVEL1_WIDTH, LEVEL1_HEIGHT, LEVEL_1_DATA, map_texture_id, 1.0f, 3, 1);

	// ENEMIES
	g_state.enemies = new Entity[ENEMY_COUNT];
	init_enemy(g_state.enemies[0], BONNIE, BONNIE_FILEPATH, glm::vec3(7.75f, 0.0f, 0.0f));
	init_enemy(g_state.enemies[1], CHICA, CHICA_FILEPATH, glm::vec3(7.75f, -2.75f, 0.0f));
	init_enemy(g_state.enemies[2], FOXY, FOXY_FILEPATH, glm::vec3(12.0f, -2.75f, 0.0f));
	init_enemy(g_state.enemies[3], FREDDY, FREDDY_FILEPATH, glm::vec3(0.0f, 0.0f, 0.0f));

	// PLAYER
	g_state.player = new Entity();
	g_state.player->set_entity_type(PLAYER);
	g_state.player->set_position(glm::vec3(3.0f, -3.0f, 0.0f));
	g_state.player->set_movement(glm::vec3(0.0f, 0.0f, 0.0f));
	g_state.player->set_speeds(1.5f, 4.0f, 0.5f);
	g_state.player->set_acceleration(glm::vec3(0.0f, -9.81f, 0.0f)); // gravity
	g_state.player->m_texture_id = load_texture(PLAYER_FILEPATH);
	g_state.player->is_facing_right = true;

	// WEAPON
	g_state.weapons = new Entity[2];

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

// Reads all the inputs on player's machine
// This includes keyboard, mouse, and window close
void process_input()
{
	// reset player movement vector
	g_state.player->set_movement(glm::vec3(0.0f));

	SDL_Event event;
	// check if game is quit
	while (SDL_PollEvent(&event))
	{
		switch (event.type) {
			// End game
		case SDL_QUIT:
		case SDL_WINDOWEVENT_CLOSE:
			g_game_is_running = false;
			break;

		case SDL_KEYDOWN:
			switch (event.key.keysym.sym) {
			case SDLK_q:
				// Quit the game with a keystroke
				g_game_is_running = false;
				break;

			case SDLK_SPACE:
				// Jump
				if (g_state.player->m_collided_bottom)
				{
					g_state.player->m_is_jumping = true;
				}
				break;
			}
		}
	}

	const Uint8* key_state = SDL_GetKeyboardState(NULL);

	if (key_state[SDL_SCANCODE_A])
	{
		// If holding either shift enter into sprint mode
		if (key_state[SDL_SCANCODE_LSHIFT] || key_state[SDL_SCANCODE_RSHIFT])
		{
			g_state.player->set_movement_state(SPRINT);
		}
		// If holding either control enter into sprint mode
		if (key_state[SDL_SCANCODE_LCTRL] || key_state[SDL_SCANCODE_RCTRL])
		{
			g_state.player->set_movement_state(SNEAK);
		}
		else g_state.player->set_movement_state(WALK); // otherwise normal speed

		g_state.player->move_left();
		g_state.player->is_facing_right = false;
	}
	else if (key_state[SDL_SCANCODE_D])
	{
		// If holding either shift enter into sprint mode
		if (key_state[SDL_SCANCODE_LSHIFT] || key_state[SDL_SCANCODE_RSHIFT])
		{
			g_state.player->set_movement_state(SPRINT);
		}
		// If holding either control enter into sprint mode
		if (key_state[SDL_SCANCODE_LCTRL] || key_state[SDL_SCANCODE_RCTRL])
		{
			g_state.player->set_movement_state(SNEAK);
		}
		else g_state.player->set_movement_state(WALK); // otherwise normal speed

		g_state.player->is_facing_right = true;
		g_state.player->move_right();
	}
	if (key_state[SDL_SCANCODE_F])
	{
		if (!trap_placed)
		{
			trap_placed = true;
			// Trap Placement
			g_state.weapons[0].set_entity_type(WEAPON);
			g_state.weapons[0].m_texture_id = load_texture(TRAP_FILEPATH);
			g_state.weapons[0].set_movement(glm::vec3(0.0f));
			g_state.weapons[0].set_speeds(0.0f, 0.0f, 0.0f);
			g_state.weapons[0].set_acceleration(glm::vec3(0.0f));
		}
		if (g_state.player->is_facing_right)
		{
			g_state.weapons[0].set_position(g_state.player->get_position() + glm::vec3(1.0f, 0.0f, 0.0f));
		}
		else g_state.weapons[0].set_position(g_state.player->get_position() + glm::vec3(-1.0f, 0.0f, 0.0f));
	}
}

/*
* Updates all objects in the game every second
*/
void update()
{
	float ticks = (float)SDL_GetTicks() / MILLISECONDS_IN_SECOND;
	float delta_time = ticks - g_previous_ticks;
	g_previous_ticks = ticks;

	delta_time += g_accumulator;

	if (delta_time < FIXED_TIMESTEP)
	{
		g_accumulator = delta_time;
		return;
	}

	while (delta_time >= FIXED_TIMESTEP)
	{
		g_state.player->update(FIXED_TIMESTEP, g_state.player, g_state.player, 1, g_state.map);
		for (size_t i = 0; i < ENEMY_COUNT; ++i)
		{
			g_state.enemies[i].update(FIXED_TIMESTEP, g_state.player, g_state.player, 1, g_state.map);
		}
		if (trap_placed)
		{
			g_state.weapons[0].update(FIXED_TIMESTEP, g_state.player, g_state.enemies, ENEMY_COUNT, g_state.map);
		}
		delta_time -= FIXED_TIMESTEP;
	}

	g_accumulator = delta_time;

	g_view_matrix = glm::mat4(1.0f);
	g_view_matrix = glm::translate(g_view_matrix, glm::vec3(-g_state.player->get_position().x, 0.75f, 0.0f));
}

/*
* Renders all objects in the game, called every frame
* Responsible for calling the entity's render function and drawing text
*/
void render()
{
	g_shader_program.set_view_matrix(g_view_matrix);

	glClear(GL_COLOR_BUFFER_BIT);

	g_state.player->render(&g_shader_program);
	g_state.map->render(&g_shader_program);
	if (trap_placed)
	{
		g_state.weapons[0].render(&g_shader_program);
	}
	for (size_t i = 0; i < ENEMY_COUNT; ++i)
	{
		g_state.enemies[i].render(&g_shader_program);
	}

	if (g_state.player->is_dead == true)
	{
		draw_text(&g_shader_program, load_texture(FONT_FILEPATH), "you lose", 0.5f,
			-0.2f, glm::vec3(g_state.player->get_position().x, 0.0f, 0.0f));
	}
	
	int death_count = 0;
	for (size_t i = 0; i < ENEMY_COUNT; ++i)
	{
		if (g_state.enemies[i].is_dead) death_count += 1;
	}
	if (death_count == ENEMY_COUNT)
	{
		draw_text(&g_shader_program, load_texture(FONT_FILEPATH), "you win", 0.5f,
			-0.2f, glm::vec3(g_state.player->get_position().x, 0.0f, 0.0f));
	}

	SDL_GL_SwapWindow(g_display_window);
}

/*
* Shuts down the game safely
*/
void shutdown()
{
	SDL_Quit();

	// free from memory
	delete g_state.enemies;
	delete g_state.player;
	delete g_state.map;
}

/*
* Draws all the game's text
* 
* @param program, a reference to the shader program that the game is using
* @param font_texture_id, the image that contains our font
* @param text, the text to be rendered
* @param screen_size, size of the game's display
* @param spacing, spacing between characters
* @param position, position of the text
*/
void draw_text(ShaderProgram* program, GLuint font_texture_id, std::string text, float screen_size, float spacing, glm::vec3 position)
{
	// Scale the size of the fontbank in the UV-plane
	// We will use this for spacing and positioning
	float width = 1.0f / FONTBANK_SIZE;
	float height = 1.0f / FONTBANK_SIZE;

	// Instead of having a single pair of arrays, we'll have a series of pairs—one for each character
	// Don't forget to include <vector>!
	std::vector<float> vertices;
	std::vector<float> texture_coordinates;

	// For every character...
	for (int i = 0; i < text.size(); i++) {
		// 1. Get their index in the spritesheet, as well as their offset (i.e. their position
		//    relative to the whole sentence)
		int spritesheet_index = (int)text[i];  // ascii value of character
		float offset = (screen_size + spacing) * i;

		// 2. Using the spritesheet index, we can calculate our U- and V-coordinates
		float u_coordinate = (float)(spritesheet_index % FONTBANK_SIZE) / FONTBANK_SIZE;
		float v_coordinate = (float)(spritesheet_index / FONTBANK_SIZE) / FONTBANK_SIZE;

		// 3. Inset the current pair in both vectors
		vertices.insert(vertices.end(), {
			offset + (-0.5f * screen_size), 0.5f * screen_size,
			offset + (-0.5f * screen_size), -0.5f * screen_size,
			offset + (0.5f * screen_size), 0.5f * screen_size,
			offset + (0.5f * screen_size), -0.5f * screen_size,
			offset + (0.5f * screen_size), 0.5f * screen_size,
			offset + (-0.5f * screen_size), -0.5f * screen_size,
			});

		texture_coordinates.insert(texture_coordinates.end(), {
			u_coordinate, v_coordinate,
			u_coordinate, v_coordinate + height,
			u_coordinate + width, v_coordinate,
			u_coordinate + width, v_coordinate + height,
			u_coordinate + width, v_coordinate,
			u_coordinate, v_coordinate + height,
			});
	}

	// 4. And render all of them using the pairs
	glm::mat4 model_matrix = glm::mat4(1.0f);
	model_matrix = glm::translate(model_matrix, position);

	program->set_model_matrix(model_matrix);
	glUseProgram(program->get_program_id());

	glVertexAttribPointer(program->get_position_attribute(), 2, GL_FLOAT, false, 0, vertices.data());
	glEnableVertexAttribArray(program->get_position_attribute());
	glVertexAttribPointer(program->get_tex_coordinate_attribute(), 2, GL_FLOAT, false, 0, texture_coordinates.data());
	glEnableVertexAttribArray(program->get_tex_coordinate_attribute());

	glBindTexture(GL_TEXTURE_2D, font_texture_id);
	glDrawArrays(GL_TRIANGLES, 0, (int)(text.size() * 6));

	glDisableVertexAttribArray(program->get_position_attribute());
	glDisableVertexAttribArray(program->get_tex_coordinate_attribute());
}