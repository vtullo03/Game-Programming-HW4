#define GL_SILENCE_DEPRECATION
#define STB_IMAGE_IMPLEMENTATION
#define LOG(argument) std::cout << argument << '\n'
#define GL_GLEXT_PROTOTYPES 1
#define FIXED_TIMESTEP 0.0166666f
#define ENEMY_COUNT 10

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

// CONSTS
// window dimensions + viewport
const int WINDOW_WIDTH = 640 * 2,
WINDOW_HEIGHT = 480 * 2;

const int VIEWPORT_X = 0,
VIEWPORT_Y = 0,
VIEWPORT_WIDTH = WINDOW_WIDTH,
VIEWPORT_HEIGHT = WINDOW_HEIGHT;

// background color -- CHANGE LATER
const float BG_RED = 0.424f,
BG_BLUE = 0.102f,
BG_GREEN = 0.11f,
BG_OPACITY = 1.0f;

// shaders
const char V_SHADER_PATH[] = "shaders/vertex_textured.glsl",
F_SHADER_PATH[] = "shaders/fragment_textured.glsl";

// texture filepaths
const char FREDDY_FILEPATH[] = "Freddy_Placeholder",
BONNIE_FILEPATH[] = "Bonnie_Placeholder",
CHICA_FILEPATH[] = "Chica_Placeholder",
FOXY_FILEPATH[] = "Foxy_Placeholder",
PLAYER_FILEPATH[] = "Guard_Placeholder";

// texture constants
const int NUMBER_OF_TEXTURES = 1;
const GLint LEVEL_OF_DETAIL = 0;
const GLint TEXTURE_BORDER = 0;