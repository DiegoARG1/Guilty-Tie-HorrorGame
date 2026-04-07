#include <GLFW/glfw3.h>
#include <engine/Billboard.h>
#include <engine/CollisionBox.h>
#include <engine/Objectives.h>
#include <engine/Particles.h>
#include <engine/Plane.h>
#include <engine/QuadTexture.h>
#include <engine/RigidModel.h>
#include <engine/Terrain.h>
#include <engine/shader_m.h>
#include <engine/skybox.h>
#include <engine/textrenderer.h>
#include <engine/Audio.h>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <string>
#include <vector>

//:::: ENUMS Y CONFIGURACIÓN DEL MOTOR :::://
enum LightType { FlatColor, AllLights, DirectionalLight, SpotLight, PointLight };
enum Axis { X, Y, Z };
enum TypeActionKeyBoard { GAME, OBJECTS, COLLISION, LIGHTING };
enum TransformObject { MODEL, COLLBOX };

LightType lightType = FlatColor;
TypeActionKeyBoard typemenu = GAME;
TransformObject transformObject = COLLBOX;

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

float deltaTime = 0.0f;
float lastFrame = 0.0f;
bool firstMouse = true;
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;

//:::: EL JUGADOR Y LA CÁMARA :::://
Camera camera(glm::vec3(21.0f, 3.0f, 27.0f));

// Movimiento y Gravedad
float posicion_y = 3.0f;
float posicion_suelo = 3.0f;
float maxima_altura = 5.0f;
bool saltar = false;

//:::: LINTERNA Y VISIÓN :::://
bool linternaEncendida = true;
bool teclaFPulsada = false;
float bateriaLinterna = 100.0f;

//:::: ENTORNO, TERRENO Y CLIMA :::://
Terrain terrain2;
const char** texturePaths;
glm::vec3 skyPos(0);

//:::: INTERACCIÓN Y OBJETOS (CABAÑA) :::://
bool abrirPuerta = false;
float anguloPuerta = 0.0f;

//:::: AUDIO :::://
Audio efecto1;
Audio efecto2;
bool sonar_ambiente = false;

//:::::::::::::: VECTORES Y RENDERIZADO :::::::::::::://
vector<glm::vec3> pointLightPositions;
vector<glm::vec3> physicsObjectsPositions;
vector<RigidModel> rigidModels;
vector<RigidModel> rbmodels = rigidModels;
vector<Model> models;

map<int, pair<string, CollisionBox>> collboxes;
map<int, pair<string, CollisionBox>> lightcubes;
CollisionBox* cb = new CollisionBox();

// Configuración de Render
float initScale = 0.2f;
bool renderCollBox = true;
bool renderLightingCubes = true;

// Físicas Base
rbEnvironment physicsEnviroment;
rbRigidBody piso, pared;


/* =========================================================================
   :::: ZONA DE CUARENTENA (VARIABLES QUE NO SE USARÁN EN ESTE JUEGO) ::::
   Todo esto está comentado. Si al compilar tu main.cpp te marca error
   en alguna de estas palabras, significa que tienes código viejo que
   también debes borrar en tu main.cpp.
   ========================================================================= */

   /*
   // Controles de Joystick (A menos que planees usar control de Xbox)
   bool isJoyStick = false;
   double xJoy = 0.0; double yJoy = 0.0; double yLeftJoy = 0.0;

   // Variables de Shooters (Balas, torretas, daño)
   int cargador = 5;
   float velocidad_bala = 3.0f;
   bool crear_bala = false;
   glm::vec3 actual_posicion_bala = glm::vec3(0.0f, 0.0f, 0.0f);
   glm::vec3 actual_direccion_bala = glm::vec3(0.0f, 0.0f, 0.0f);
   glm::vec3 posicion_torreta = glm::vec3(30.0f, 3.0f, 10.0f);
   glm::vec3 posicion_bala = glm::vec3(30.0f, 3.0f, 20.0f);
   float salud = 101.0f;
   bool control_salud = true;
   float retroceso = 5.0f;
   float trayecto_retroceso = 5.0f;
   bool golpe = true;

   // Modelos específicos no usados
   glm::vec3 posicion_anakin = glm::vec3(0.0, 2.5f, 5.0f);
   glm::vec3 posicion_trampa = glm::vec3(30.0f, 1.5f, 20.0f);
   bool trampa_move = false;
   glm::vec3 posicion_enemigo1;
   bool destruir_enemigo_1 = false;
   map<int, pair<string, CollisionBox>> collboxes_enemigos;

   // Mecánicas de Agua
   bool waterout = false;
   glm::vec3 posicion_agua = glm::vec3(35.0f, 0.5f, 35.0f);
   float mov_agua_x = 0.0f;
   float mov_agua_y = 0.0f;

   // Variables basura de prueba
   glm::vec3 posicion_2 = glm::vec3(-35.0f, 0.5f, -35.0f);
   glm::vec3 posicion_eliminacion = glm::vec3(0.0f, -100.0f, 0.0f);
   bool direccion_1 = true;
   bool direccion_2 = false;
   float rodandoY = 0.0f;
   float rodandoYmenos = 0.0f;
   float posicion_trayecto = 0.0f;
   float posicion_final_trayecto = 10.0f;
   bool choco = false;
   bool choco_torre = false;
   int limite_choque = 0;

   // Lluvia (Descomentar si planeas añadir tormenta después)
   float posicion_lluvia = 10.0f;
   float caida_lluvia = 0.5;
   float gotas_lluviaX[200];
   float gotas_lluviaZ[200];
   float gotas_lluviaY[200];
   float gotas_lluviaY_Inicial[200];
   bool primera_vez = true;
   */