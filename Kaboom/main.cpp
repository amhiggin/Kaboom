/*
	@author Amber Higgins, 13327954
	CS4052 Lab 6: Game Project
	Game name: Kaboom
	Advanced features:
		-collision detection (diamonds, bombs)
		-simulated dynamics (jumping)
		-complex movement of bombs
		-synchronised sound effects
		-high score tracking, game states and text display on-screen
		-great models that I made myself - I made all models


*/

#include <windows.h>
#include <mmsystem.h>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <iostream>
#include "maths_funcs.h"
#include <time.h>
#ifdef WIN32
#endif
#include <windows.h>
#include <conio.h>

// Assimp includes
#pragma comment(lib, "assimp.lib") 
#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h> 
#include <stdio.h>
#include <math.h>
#include <vector> 

#include "text.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <irrKlang.h>
#pragma comment(lib, "irrKlang.lib") // link with irrKlang.dll
#include <ik_ISoundEngine.h>
using namespace irrklang;
ISoundEngine *SoundEngine = createIrrKlangDevice();


#define GROUND_MESH "../Models/ground.dae"			//the floor mesh
#define OBSTACLE_MESH "../Models/pillar.dae"		//the pillar mesh
#define DIAMOND_MESH "../Models/green_diamond.dae"	//the diamond mesh
#define BOMB_MESH "../Models/bomb.dae"				//the bomb mesh
#define CHARACTER_MESH "../Models/character.dae"	//the character mesh
#define GROUND_TEXTURE  "../Images/floor.gif"				//the floor texture
#define OBSTACLE_TEXTURE "../Images/scifi.jpg"				//the pillar texture
#define CHARACTER_TEXTURE "../Images/green.jpg"			//the character texture
#define DIAMOND_TEXTURE "../Images/diamond.jpg"			//the diamond texture

#define NUM_BOMBS 4
#define NUM_OBSTACLES 25

float thresh = 0.5;	//this should be the distance from the centre of an object at which we have collided with it
std::vector<float> g_vp, g_vn, g_vt;
//counters for object vertices
int g_point_count = 0;
int ground_vertex_count = 0, obstacle_vertex_count = 0;
int diamond_vertex_count = 0, bomb_vertex_count = 0, character_vertex_count = 0;
//object and texture ids
GLuint GROUND_ID = 0, DIAMOND_ID = 2, BOMB_ID = 3, OBSTACLE_ID = 4, CHARACTER_ID = 5;
GLuint GROUND_TEXTURE_ID = 0, DIAMOND_TEXTURE_ID = 2, BOMB_TEXTURE_ID = 3, OBSTACLE_TEXTURE_ID = 4, CHARACTER_TEXTURE_ID = 5;


// Macro for indexing vertex buffer
#define BUFFER_OFFSET(i) ((char *)NULL + (i))
using namespace std;
GLuint shaderProgramID;

unsigned int mesh_vao = 0;
int width = 1000;
int height = 1000;

/*
CAMERA GLOBALS
*/
//transformations
GLuint loc1, loc2, loc3;
GLfloat rotate_camera_x = 1.5f, rotate_camera_y = 0.5f;
GLfloat move_camera_x = 0.0f, move_camera_y = 0.0f, move_camera_z = 0.0f;
GLfloat speed = 0.02f;
GLfloat camera_dist = 2.0f;
//for mouse 
int prev_x, prev_y;
bool mouse = true;
bool upkey = false, downkey = false, leftkey = false, rightkey = false;
bool i_key = false, k_key = false, j_key = false, l_key = false;

/*
GAMEPLAY VARIABLES
*/
#define HOMESCREEN 0
#define PLAYING 1
#define YOU_DIED 2
bool gameover = false;
int home_screen_text[1], score_text;
int state = HOMESCREEN;
bool reached_edge[4] = { false,false, false, false };
bool dead = false;
float high_scores[11] = { 0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f, 0.0f };
float highest = 0.0f;
int curr_score = 0.0;
GLfloat character_position_x = 0.0f, character_position_y = 0.0f, character_position_z = -8.0f;
GLfloat bomb_position_x[4] = { 199.0, -100.0, -199.0, 100.0 };
GLfloat bomb_position_z[4] = { -100.0, 250.0, 300.0, -300.0 };
GLfloat obstacle_pos_x[] = { -9.0f, -4.5f, 0.0f, 4.5f, 9.0f,
-9.0f, -4.5f, 0.0f, 4.5f, 9.0f,
-9.0f, -4.5f, 0.0f, 4.5f, 9.0f,
-9.0f, -4.5f, 0.0f, 4.5f, 9.0f,
-9.0f, -4.5f, 0.0f, 4.5f, 9.0f };
GLfloat obstacle_pos_z[] = { -9.0f, -9.0f, -9.0f, -9.0f, -9.0f,
4.5f, 4.5f, 4.5f, 4.5f, 4.5f,
-4.5f, -4.5f, -4.5f, -4.5f, -4.5f,
0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
9.0f, 9.0f, 9.0f, 9.0f, 9.0f };
GLfloat diamond_pos[] = { 0.0f, 0.0f };
GLfloat dist = 0.0;
GLfloat character_translation = 0.0f;
GLfloat diamond_rotation = 0.0f;
GLfloat bomb_rotation = 0.0f;
// possible positions for the diamonds
GLfloat possibilities[] = { -7.5f, 6.0f, 8.5f,-2.5f, 2.5f, 7.5f, -3.65f, -1.39f, 8.2f, -4.0f };
GLfloat diamond_count = 0.0f;
GLfloat bomb_count = 0.0f;
GLfloat game_counter = 0.0f;
bool ypressed = false, npressed = false;

GLfloat jump_size = 0.0f;
bool jump = false;



#pragma region MESH LOADING

bool load_mesh(const char* file_name) {
	const aiScene* scene = aiImportFile(file_name, aiProcess_Triangulate);
	if (!scene) {
		return false;
	}


	for (unsigned int m_i = 0; m_i < scene->mNumMeshes; m_i++) {
		const aiMesh* mesh = scene->mMeshes[m_i];

		g_vp.clear();
		g_vn.clear();
		g_vt.clear();

		//get the num of vertices for each mesh
		if (file_name == GROUND_MESH)
			ground_vertex_count = mesh->mNumVertices;
		else if (file_name == OBSTACLE_MESH)
			obstacle_vertex_count = mesh->mNumVertices;
		else if (file_name == DIAMOND_MESH)
			diamond_vertex_count = mesh->mNumVertices;
		else if (file_name == BOMB_MESH)
			bomb_vertex_count = mesh->mNumVertices;
		else if (file_name == CHARACTER_MESH)
			character_vertex_count = mesh->mNumVertices;
		else
			g_point_count = mesh->mNumVertices;


		for (unsigned int v_i = 0; v_i < mesh->mNumVertices; v_i++) {
			if (mesh->HasPositions()) {
				const aiVector3D* vp = &(mesh->mVertices[v_i]);
				g_vp.push_back(vp->x);
				g_vp.push_back(vp->y);
				g_vp.push_back(vp->z);
			}
			if (mesh->HasNormals()) {
				const aiVector3D* vn = &(mesh->mNormals[v_i]);
				g_vn.push_back(vn->x);
				g_vn.push_back(vn->y);
				g_vn.push_back(vn->z);
			}
			if (mesh->HasTextureCoords(0)) {
				const aiVector3D* vt = &(mesh->mTextureCoords[0][v_i]);
				g_vt.push_back(vt->x);
				g_vt.push_back(vt->y);
			}
		}
	}

	aiReleaseImport(scene);
	return true;
}


#pragma endregion MESH LOADING

#pragma region SHADER_FUNCTIONS
#pragma warning(disable : 4996) 

char* readShaderSource(const char* shaderFile) {
	FILE* fp = fopen(shaderFile, "rb");

	if (fp == NULL) { return NULL; }

	fseek(fp, 0L, SEEK_END);
	long size = ftell(fp);

	fseek(fp, 0L, SEEK_SET);
	char* buf = new char[size + 1];
	fread(buf, 1, size, fp);
	buf[size] = '\0';

	fclose(fp);

	return buf;
}


static void AddShader(GLuint ShaderProgram, const char* pShaderText, GLenum ShaderType)
{
	GLuint ShaderObj = glCreateShader(ShaderType);

	if (ShaderObj == 0) {
		fprintf(stderr, "Error creating shader type %d\n", ShaderType);
		exit(0);
	}
	const char* pShaderSource = readShaderSource(pShaderText);

	glShaderSource(ShaderObj, 1, (const GLchar**)&pShaderSource, NULL);
	glCompileShader(ShaderObj);
	GLint success;
	glGetShaderiv(ShaderObj, GL_COMPILE_STATUS, &success);
	if (!success) {
		GLchar InfoLog[1024];
		glGetShaderInfoLog(ShaderObj, 1024, NULL, InfoLog);
		fprintf(stderr, "Error compiling shader type %d: '%s'\n", ShaderType, InfoLog);
	}
	glAttachShader(ShaderProgram, ShaderObj);
}

//adds a texture using the stb_image library ( (c) Dr Anton Gerdelan)
void addTexture(GLuint& texture_id, char* file_name) {
	int width, height, n;
	unsigned char* image = stbi_load(file_name, &width, &height, &n, STBI_rgb);

	glGenTextures(1, &texture_id);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture_id);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);

	//wrap around texture repeated
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glBindTexture(GL_TEXTURE_2D, 0);
}


GLuint CompileShaders()
{
	shaderProgramID = glCreateProgram();
	if (shaderProgramID == 0) {
		fprintf(stderr, "Error creating shader program\n");
		exit(1);
	}

	// Create vertex and fragment shader
	AddShader(shaderProgramID, "../Shaders/simpleVertexShader.txt", GL_VERTEX_SHADER);
	AddShader(shaderProgramID, "../Shaders/simpleFragmentShader.txt", GL_FRAGMENT_SHADER);

	GLint Success = 0;
	GLchar ErrorLog[1024] = { 0 };

	glLinkProgram(shaderProgramID);
	glGetProgramiv(shaderProgramID, GL_LINK_STATUS, &Success);
	if (Success == 0) {
		glGetProgramInfoLog(shaderProgramID, sizeof(ErrorLog), NULL, ErrorLog);
		fprintf(stderr, "Error linking shader program: '%s'\n", ErrorLog);
		exit(1);
	}

	// program has been successfully linked but needs to be validated 
	glValidateProgram(shaderProgramID);

	glGetProgramiv(shaderProgramID, GL_VALIDATE_STATUS, &Success);
	if (!Success) {
		glGetProgramInfoLog(shaderProgramID, sizeof(ErrorLog), NULL, ErrorLog);
		fprintf(stderr, "Invalid shader program: '%s'\n", ErrorLog);
		exit(1);
	}

	glUseProgram(shaderProgramID);
	return shaderProgramID;
}
#pragma endregion SHADER_FUNCTIONS

#pragma region VBO_FUNCTIONS

//give the vao, filename and the num of  vertices, generate the mesh for the object
void generateObjectBufferMesh(GLuint &vao, const char* f_name, int &count) {
	load_mesh(f_name);
	unsigned int vp_vbo = 0;
	loc1 = glGetAttribLocation(shaderProgramID, "vertex_position");
	loc2 = glGetAttribLocation(shaderProgramID, "vertex_normal");
	loc3 = glGetAttribLocation(shaderProgramID, "vertex_texture");

	glGenBuffers(1, &vp_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vp_vbo);
	glBufferData(GL_ARRAY_BUFFER, count * 3 * sizeof(float), &g_vp[0], GL_STATIC_DRAW);
	unsigned int vn_vbo = 0;
	glGenBuffers(1, &vn_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vn_vbo);
	glBufferData(GL_ARRAY_BUFFER, count * 3 * sizeof(float), &g_vn[0], GL_STATIC_DRAW);

	unsigned int vt_vbo = 0;
	glGenBuffers(1, &vt_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vt_vbo);
	glBufferData(GL_ARRAY_BUFFER, count * 2 * sizeof(float), &g_vt[0], GL_STATIC_DRAW);

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glEnableVertexAttribArray(loc1);
	glBindBuffer(GL_ARRAY_BUFFER, vp_vbo);
	glVertexAttribPointer(loc1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(loc2);
	glBindBuffer(GL_ARRAY_BUFFER, vn_vbo);
	glVertexAttribPointer(loc2, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	glEnableVertexAttribArray(loc3);
	glBindBuffer(GL_ARRAY_BUFFER, vt_vbo);
	glVertexAttribPointer(loc3, 2, GL_FLOAT, GL_FALSE, 0, NULL);
	glBindVertexArray(0);
}


#pragma endregion VBO_FUNCTIONS

void displayInstructions() {
	cout << "-----------------------------\n";
	cout << "\tKABOOM" << endl;
	cout << "Rules: " << endl;
	cout << "\t - To win, collect as many diamonds as you can. " << endl;
	cout << "\t - You will have to navigate around obstacles with the arrow keys." << endl;
	cout << "\t - Use the space-bar to jump over the bombs." << endl;
	cout << "\t - Use i, j, k, l keys to rotate around." << endl;
	cout << "\t - Use the mouse wheel to zoom." << endl;
	cout << "\t - If you hit a bomb, the game is over. " << endl;
	cout << "\t - Your highscore will be saved. " << endl;
	cout << "Press Y to start playing, or N to exit." << endl << endl;
	cout << "------------------------------\n";
}


//generates random diamonds 
void diamond_logic() {
	diamond_pos[0] = possibilities[rand() % 10];
	diamond_pos[1] = possibilities[rand() % 10];
}

//translates the bombs around the scene so that they have to be avoided by the character
void bomb_logic() {
	//bomb 1 - rotating around the scene pseudo-randomly
	if (bomb_position_x[0] == 1000.0) reached_edge[0] = true;
	else if (bomb_position_x[0] == -100.0) reached_edge[0] = false;
	if (reached_edge[0] == true) {
		bomb_position_x[0] -= 1.0;
	}
	else if (reached_edge[0] == false) {
		bomb_position_x[0] += 1.0;
	}

	//bomb 2 - rotating around the scene pseudo-randomly
	if (bomb_position_x[1] == -1000.0) reached_edge[1] = true;
	else if (bomb_position_x[1] == 1000.0) reached_edge[1] = false;
	if (reached_edge[1] == true) {
		bomb_position_x[1] += 1.0;
	}
	else if (reached_edge[1] == false) {
		bomb_position_x[1] -= 1.0;
	}

	//bomb 3 - moving over and back
	if (bomb_position_x[2] == -800.0) reached_edge[2] = true;
	else if (bomb_position_x[2] == 800.0) reached_edge[2] = false;
	if (reached_edge[2] == true) {
		bomb_position_x[2] += 1.0;
	}
	else if (reached_edge[2] == false) {
		bomb_position_x[2] -= 1.0;
	}

	//bomb 4 - moving over and back
	if (bomb_position_x[3] == -800.0) reached_edge[3] = true;
	else if (bomb_position_x[3] == 800.0) reached_edge[3] = false;
	if (reached_edge[3] == true) {
		bomb_position_x[3] += 1.0;
	}
	else if (reached_edge[3] == false) {
		bomb_position_x[3] -= 1.0;
	}
}

void initialiseText()
{
	home_screen_text[0] = add_text("KABOOM", -0.58, 0.75, 90.0f, 0.388, 1.0, 0.125, 0.8);
	score_text = add_text("Score: 0", -0.3, 0.9, 50.0f, 1.0, 1.0, 1.0, 0.0);
}

void calculate_diamond_position() {
	//generates random diamonds 
	diamond_pos[0] = possibilities[rand() % 10];
	diamond_pos[1] = possibilities[rand() % 10];
}


//checks to see if we have hit a diamond
bool check_diamond_collision(float _x, float _z) {
	//this is where we increment our score
	float x = _x - diamond_pos[0];
	float z = _z - diamond_pos[1];
	if (sqrt((x*x) + (z*z)) <= 1.0f) {
		//we scored!!
		curr_score++;
		cout << "Score: " << curr_score << endl;
		SoundEngine->play2D("../Audio/beep.mp3", false);
		//should move the diamond somewhere else
		calculate_diamond_position();
		return true;
	}
	if (sqrt((_x*_x) + (_z*_z)) < thresh || ((sqrt((_x*_x) + (_z*_z)) > thresh * 2)
		&& (sqrt((_x*_x) + (_z*_z)) < thresh * 3) && (_x > 0.36f || _x < -0.36f)))
		return true;
	return false;
}

//determines how the character should move depending on objects in the scene
void character_move() {
	if (upkey == true) {
		//move forward as long as we aren't colliding with a diamond or a pillar
		if (!check_diamond_collision(character_position_x, character_position_z - speed * cos(rotate_camera_x)))
			character_position_z -= speed * cos(rotate_camera_x);
		if (!check_diamond_collision(character_position_x - speed * sin(rotate_camera_x), character_position_z))
			character_position_x -= speed * sin(rotate_camera_x);
	}
	if (downkey == true) {
		//move backwards as long as we aren't colliding with a diamond or a pillar
		if (!check_diamond_collision(character_position_x, character_position_z + speed * cos(rotate_camera_x)))
			character_position_z += speed * cos(rotate_camera_x);
		if (!check_diamond_collision(character_position_x + speed * sin(rotate_camera_x), character_position_z))
			character_position_x += speed * sin(rotate_camera_x);
	}
	if (leftkey == true) {
		//move left as long as we aren't colliding with a diamond or a pillar
		if (!check_diamond_collision(character_position_x - speed * sin(rotate_camera_x + 1.57f), character_position_z))
			character_position_x -= speed * sin(rotate_camera_x + 1.57f);
		if (!check_diamond_collision(character_position_x, character_position_z - speed * cos(rotate_camera_x + 1.57f)))
			character_position_z -= speed * cos(rotate_camera_x + 1.57f);
	}
	if (rightkey == true) {
		//move right as long as we aren't colliding with a diamond or a pillar
		if (!check_diamond_collision(character_position_x + speed * sin(rotate_camera_x + 1.57f), character_position_z))
			character_position_x += speed * sin(rotate_camera_x + 1.57f);
		if (!check_diamond_collision(character_position_x, character_position_z + speed * cos(rotate_camera_x + 1.57f)))
			character_position_z += speed * cos(rotate_camera_x + 1.57f);
	}


	//jumping
	if (jump == true) {
		jump_size += 0.008f;
		character_position_y = 3.0f * sin(jump_size);
		if (sin(jump_size) <= 0.0f) {
			character_position_y = 0.0f;
			jump_size = 0.0f;
			jump = false;
		}
	}

	//keep the character within the bounds of the game area
	if (character_position_x >= 11.5f) character_position_x = 11.5f;
	if (character_position_x <= -11.5f) character_position_x = -11.5f;
	if (character_position_z >= 11.5f) character_position_z = 11.5f;
	if (character_position_z <= -11.5f) character_position_z = -11.5f;
}


//make sure that camera is following the character
void updateCamera() {
	if (i_key) {
		rotate_camera_y += 0.01f;
		if (rotate_camera_y > 1.56f)
			rotate_camera_y = 1.56f;
	}
	if (k_key) {
		rotate_camera_y -= 0.01f;
		if (rotate_camera_y < -0.05f)
			rotate_camera_y = -0.05f;
	}
	if (j_key) {
		rotate_camera_x -= 0.01f;
	}
	if (l_key) {
		rotate_camera_x += 0.01f;
	}
	move_camera_x = (5.0f * character_position_x) + (camera_dist * cos(rotate_camera_y) * sin(rotate_camera_x));
	move_camera_y = camera_dist * sin(rotate_camera_y);
	move_camera_z = (5.0f * character_position_z) + (camera_dist * cos(rotate_camera_y) * cos(rotate_camera_x));
}

//zoom in and out on the character
void mouseWheel(int button, int direction, int x, int y) {
	if (direction > 0)
		camera_dist -= 0.5f;
	else
		camera_dist += 0.5f;
}






void init() {
	// Set up the shaders
	GLuint shaderProgramID = CompileShaders();
	diamond_logic();
	state = HOMESCREEN;//homescreen
	init_text_rendering("..\\freemono.png", "..\\freemono.meta", width, height);
	initialiseText();

	// load meshes for the objects into a vertex buffer array
	generateObjectBufferMesh(GROUND_ID, GROUND_MESH, ground_vertex_count);
	generateObjectBufferMesh(OBSTACLE_ID, OBSTACLE_MESH, obstacle_vertex_count);
	generateObjectBufferMesh(DIAMOND_ID, DIAMOND_MESH, diamond_vertex_count);
	generateObjectBufferMesh(BOMB_ID, BOMB_MESH, bomb_vertex_count);
	generateObjectBufferMesh(CHARACTER_ID, CHARACTER_MESH, character_vertex_count);
	//generate the textures for these objects
	addTexture(GROUND_TEXTURE_ID, GROUND_TEXTURE);
	addTexture(OBSTACLE_TEXTURE_ID, OBSTACLE_TEXTURE);
	addTexture(CHARACTER_TEXTURE_ID, CHARACTER_TEXTURE);
	addTexture(DIAMOND_TEXTURE_ID, DIAMOND_TEXTURE);

	//start playing music on repeat
	SoundEngine->play2D("../Audio/breakout.mp3", true);
	//homescreen state - display the instructions
	if (state == HOMESCREEN)	displayInstructions();

}

void updateHighScore() {
	if (highest < curr_score) {
		highest = curr_score;
	}
	cout << "--------------------------------" << endl;
	cout << "YOUR SCORE: \t" << curr_score << endl;
	cout << "HIGH SCORE: \t" << highest << endl;
	if (highest == curr_score&& curr_score != 0) {
		cout << "CONGRATULATIONS! YOU BEAT YOUR HIGH SCORE!" << endl;
	}
	cout << "--------------------------------" << endl;
	cout << "Press Y to replay or N to exit." << endl;
}

//ensures we know what happens when we stop pressing a key
void releaseNormalKeys(unsigned char key, int x, int y) {
	if (state == PLAYING) {
		switch (key) {
		case 'i': //Up
			i_key = false;
			break;
		case 'j': //Left
			j_key = false;
			break;
		case 'k': //Down
			k_key = false;
			break;
		case 'l': //Right
			l_key = false;
			break;
		}
	}
	else if (state == YOU_DIED) {
		switch (key) {
		case 'n':
			npressed = false;		//don't want to play again
			break;
		case 'y':
			ypressed = false;		//play again
			break;
		}
	}
	else if (state == HOMESCREEN) {
		switch (key) {
		case 'n':
			npressed = false;
			break;
		case 'y':
			ypressed = false;
			break;
		}
	}
	glutPostRedisplay();
}



// Control the translation of the camera using the arrow keys
void processSpecialKeys(int key, int x, int y) {
	if (state == PLAYING) {
		switch (key) {
		case GLUT_KEY_UP:
			//character moves forward
			upkey = true;
			break;
		case GLUT_KEY_DOWN:
			//moves backwards
			downkey = true;
			break;
		case GLUT_KEY_LEFT:
			//moves left
			leftkey = true;
			break;
		case GLUT_KEY_RIGHT:
			//moves right
			rightkey = true;
			break;
		}
		glutPostRedisplay();
	}
}


//update things when the key is up
void releaseKeys(int key, int x, int y) {
	if (state == PLAYING) {
		switch (key) {
		case GLUT_KEY_UP:
			upkey = false;
			break;
		case GLUT_KEY_DOWN:
			downkey = false;
			break;
		case GLUT_KEY_LEFT:
			leftkey = false;
			break;
		case GLUT_KEY_RIGHT:
			rightkey = false;
			break;
		}
		glutPostRedisplay();
	}
}



//move the camera using the mouse
void processMouse(int x, int y) {
	//maybe will use this for rotations
	if (mouse) {
		prev_x = x;
		prev_y = y;
		mouse = false;
	}

	//look at horizontal plane
	if (x > prev_x) {
		rotate_camera_x -= 0.02f;
	}
	else if (x < prev_x) {
		rotate_camera_x += 0.02f;
	}

	//look at vertical plane
	if (y > prev_y) {
		rotate_camera_x -= 0.02f;
	}
	else if (y < prev_y) {
		rotate_camera_x += 0.02f;
	}

	//update the prev variables for next time function is called
	prev_x = x;
	prev_y = y;

	glutPostRedisplay();
}





//reset the game state to start over
void reset_game() {
	dead = false;
	upkey = false, downkey = false, rightkey = false, leftkey = false, jump = true;
	i_key = false, k_key = false, j_key = false, l_key = false;
	// reset vars
	curr_score = 0.0;
	rotate_camera_x = 0.0f, rotate_camera_y = 0.0f;
	game_counter += 1.0;
	diamond_rotation = 0.0f;
	bomb_rotation = 0.0f;
	move_camera_x = 0.0f, move_camera_z = -4.0f;
	//reset the character starting point
	character_position_x = -4.0f;
	character_position_y = 0.0f;
	character_position_z = -4.0f;
	camera_dist = 2.0f;
	dist = 0.0;
	//reset other variables
	game_counter += 1.0f;
	bomb_position_x[0] = 199.0, bomb_position_x[1] = -120.0, bomb_position_x[2] - 199.0, bomb_position_x[3] = 120.0;
	bomb_position_z[0] = -120.0, bomb_position_z[1] = 250.0, bomb_position_z[2] = 300.0, bomb_position_z[3] = -300.0;
	bomb_logic();
	diamond_logic();
	updateCamera();

}
//let the user decide what they want to do
void changeGameState(int next) {

	if (state == HOMESCREEN) {
		change_text_colour(home_screen_text[0], 0.5, 0.5, 0.5, 0.0);


		if (next == PLAYING) {
			change_text_colour(score_text, 0.5, 0.5, 0.5, 0.8);
			state = PLAYING;
			return;
		}
	}
	else if (state == YOU_DIED) {
		//update the scores

		if (curr_score > highest) {
			//new high score
			highest = curr_score;
		}

		if (next = PLAYING && ypressed == true) {
			//the user said yes to playing again
			state = PLAYING;
			change_text_colour(score_text, 0.8, 0.1, 0.6, 0.8);
			reset_game();
		}
	}
}

//checks to see if you've hit a bomb - if you have, you die
void check_bomb_collisions() {
	for (int i = 0; i < 4; i++) {
		GLfloat x_pos = character_position_x - bomb_position_x[i];
		GLfloat z_pos = character_position_z - bomb_position_z[i];
		dist = sqrt((x_pos*x_pos) + (z_pos*z_pos));
		if (dist < 115.0) {
			if (character_position_y < 1.0) {
				//game over - we hit a bomb!!!
				SoundEngine->play2D("../Audio/explosion.mp3", false);
				cout << "YOU DIED" << endl;
				state = YOU_DIED;
				updateHighScore();
			}
		}
	}
	//else we didn't collide with a bomb
	dist = 0.0;
}

void pressNormalKeys(unsigned char key, int x, int y) {
	if (state == PLAYING) {
		switch (key) {
		case 'i': //Up
			i_key = true;
			break;
		case 'j': //Left
			j_key = true;
			break;
		case 'k': //Down
			k_key = true;
			break;
		case 'l': //Right
			l_key = true;
			break;
		case 32:
			if (jump == false) {
				jump = true;
				SoundEngine->play2D("../Audio/boing.mp3");
			}
			break;
		}
	}
	else if (state == YOU_DIED) {
		switch (key) {
		case 'n':
			npressed = true;		//don't want to play again
			exit(0);
			break;
		case 'y':
			ypressed = true;		//play again
			changeGameState(PLAYING);
			break;
		}
	}
	else if (state == HOMESCREEN) {
		switch (key) {
		case 'y':
			ypressed = true;
			changeGameState(PLAYING);
			break;
		case 'n':
			exit(0);
		}
	}
	glutPostRedisplay();
}

void updateScene() {
	static DWORD  last_time = 0;
	DWORD  curr_time = timeGetTime();
	float  delta = (curr_time - last_time) * 0.001f;
	if (delta > 0.03f)
		delta = 0.03f;
	last_time = curr_time;


	if (state == PLAYING) {
		//keep the score updating
		string score_string = "Score: ";
		score_string.append(to_string((int)curr_score));
		update_text(score_text, score_string.c_str());

		//keep rotating going
		bomb_rotation += 0.2f;
		diamond_rotation += 1.0f;
		//move the character
		character_move();
		//check that we are still alive
		check_bomb_collisions();
		//keep moving the bombs
		bomb_logic();
		//make sure the camera state is updated correctly
		updateCamera();
	}
	// Draw the next frame
	glutPostRedisplay();
}



void display() {

	if (state == HOMESCREEN || state == YOU_DIED)
	{
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		//display the text for the homescreen (KABOOM and instructions)
		draw_texts();
	}

	else if (state == PLAYING) {
		//not dead - game on!
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);
		glClearColor(0.5f, 0.1f, 0.5f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glUseProgram(shaderProgramID);

		int matrix_location = glGetUniformLocation(shaderProgramID, "model");
		int view_mat_location = glGetUniformLocation(shaderProgramID, "view");
		int proj_mat_location = glGetUniformLocation(shaderProgramID, "proj");
		int texture_location = glGetUniformLocation(shaderProgramID, "tex");


		//Draw the ground - the root object
		mat4 persp_proj = perspective(90.0, (float)width / (float)height, 0.1, 1000.0);
		mat4 floor_local = identity_mat4();
		floor_local = scale(floor_local, vec3(4.75, 4.75, 4.75));
		mat4 floor_global = floor_local;
		//camera - look at character
		mat4 cam_view = look_at(vec3(move_camera_x, 5.0f * character_position_y + 3.0f + move_camera_y, move_camera_z),
			vec3(5.0f * character_position_x, 5.0f * character_position_y + 3.0f, 5.0f * character_position_z),
			vec3(0.0f, 1.0f, 0.0f));
		glUniformMatrix4fv(proj_mat_location, 1, GL_FALSE, persp_proj.m);
		glUniformMatrix4fv(view_mat_location, 1, GL_FALSE, cam_view.m);
		glUniformMatrix4fv(matrix_location, 1, GL_FALSE, floor_global.m);
		//add texture to ground
		glBindTexture(GL_TEXTURE_2D, GROUND_TEXTURE_ID);
		glUniform1i(texture_location, 0);
		glBindVertexArray(GROUND_ID);
		glDrawArrays(GL_TRIANGLES, 0, ground_vertex_count);


		//pillars
		glBindTexture(GL_TEXTURE_2D, OBSTACLE_TEXTURE_ID);
		glUniform1i(texture_location, 0);
		glBindVertexArray(OBSTACLE_ID);
		mat4 obstacle_local[NUM_OBSTACLES];
		mat4 obstacle_global[NUM_OBSTACLES];
		for (int i = 0; i < NUM_OBSTACLES; i++)
		{
			obstacle_local[i] = identity_mat4();
			obstacle_local[i] = scale(obstacle_local[i], vec3(0.08, 0.25, 0.2));
			obstacle_local[i] = rotate_x_deg(obstacle_local[i], -90.0);
			obstacle_local[i] = translate(obstacle_local[i], vec3(obstacle_pos_x[i], 0.0f, obstacle_pos_z[i] - 2.0));
			obstacle_global[i] = floor_global * obstacle_local[i];
			glUniformMatrix4fv(matrix_location, 1, GL_FALSE, obstacle_global[i].m);
			glDrawArrays(GL_TRIANGLES, 0, obstacle_vertex_count);
		}

		//bombs
		glBindVertexArray(BOMB_ID);
		mat4 bomb_local[4], bomb_global[4];
		for (int j = 0; j < 4; j++) {
			bomb_local[j] = identity_mat4();
			bomb_local[j] = rotate_x_deg(bomb_local[j], -90.0);
			bomb_local[j] = rotate_y_deg(bomb_local[j], bomb_rotation);
			if (j == 0 || j == 1)
				bomb_local[j] = rotate_y_deg(translate(bomb_local[j], vec3(bomb_position_x[j], 10.0,
					bomb_position_z[j])), bomb_rotation);
			else
				bomb_local[j] = translate(bomb_local[j], vec3(bomb_position_x[j], 10.0, bomb_position_z[j]));

			bomb_local[j] = scale(bomb_local[j], vec3(0.01, 0.01, 0.01f));
			bomb_global[j] = floor_global * bomb_local[j];
			glUniformMatrix4fv(matrix_location, 1, GL_FALSE, bomb_global[j].m);
			glDrawArrays(GL_TRIANGLES, 0, bomb_vertex_count);
		}


		//character
		mat4 character_local_bottom = identity_mat4();
		character_local_bottom = scale(character_local_bottom, vec3(0.01f, 0.01f, 0.01f));
		character_local_bottom = rotate_y_deg(character_local_bottom, rotate_camera_y);
		character_local_bottom = translate(character_local_bottom, vec3(character_position_x,
			character_position_y + 0.35f + character_translation, character_position_z));
		mat4 character_global = floor_global * character_local_bottom;
		// draw
		glBindTexture(GL_TEXTURE_2D, CHARACTER_TEXTURE_ID);
		glUniform1i(texture_location, 0);
		glUniformMatrix4fv(matrix_location, 1, GL_FALSE, character_global.m);
		glBindVertexArray(CHARACTER_ID);
		glDrawArrays(GL_TRIANGLES, 0, character_vertex_count);
		//top
		mat4 character_local_top = identity_mat4();
		character_local_top = translate(character_local_top, vec3(0.0f, 20.0f, 0.0f));
		mat4 character_global1 = character_global * character_local_top;
		// draw
		glBindTexture(GL_TEXTURE_2D, CHARACTER_TEXTURE_ID);
		glUniform1i(texture_location, 0);
		glUniformMatrix4fv(matrix_location, 1, GL_FALSE, character_global1.m);
		glBindVertexArray(CHARACTER_ID);
		glDrawArrays(GL_TRIANGLES, 0, character_vertex_count);

		//diamond
		glBindTexture(GL_TEXTURE_2D, DIAMOND_TEXTURE_ID);
		glUniform1i(texture_location, 0);
		glBindVertexArray(DIAMOND_ID);
		mat4 diamond_local = identity_mat4();
		diamond_local = scale(diamond_local, vec3(0.015f, 0.015f, 0.015f));
		diamond_local = rotate_x_deg(diamond_local, -90.0);
		diamond_local = rotate_y_deg(diamond_local, diamond_rotation);
		diamond_local = translate(diamond_local, vec3(diamond_pos[0], 1.0f, diamond_pos[1]));
		mat4 diamond_global = floor_global * diamond_local;

		// draw
		glUniformMatrix4fv(matrix_location, 1, GL_FALSE, diamond_global.m);
		glDrawArrays(GL_TRIANGLES, 0, diamond_vertex_count);

		draw_texts();
	}
	glutSwapBuffers();
}




int main(int argc, char** argv) {
	srand(time(NULL));
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowSize(width, height);
	diamond_logic();

	glutCreateWindow("KABOOM!");

	glutDisplayFunc(display);
	glutIdleFunc(updateScene);
	glutKeyboardFunc(pressNormalKeys);
	glutKeyboardUpFunc(releaseNormalKeys);
	glutSpecialFunc(processSpecialKeys);
	glutSpecialUpFunc(releaseKeys);
	glutPassiveMotionFunc(processMouse);
	glutMouseWheelFunc(mouseWheel);

	GLenum res = glewInit();
	if (res != GLEW_OK) {
		fprintf(stderr, "Error: '%s'\n", glewGetErrorString(res));
		return 1;
	}

	init();
	glutMainLoop();
	return 0;
}