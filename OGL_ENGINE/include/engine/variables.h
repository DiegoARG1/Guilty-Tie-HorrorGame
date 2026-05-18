#pragma once
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
#include <cstdlib> 
#include <ctime> 
#include "ObjetosJuego.h"
#include "EntidadIA.h"

//:::: ENUMS Y CONFIGURACIÓN DEL MOTOR :::://
enum LightType { FlatColor, AllLights, DirectionalLight, SpotLight, PointLight };
enum Axis { X, Y, Z };
enum TypeActionKeyBoard { GAME, OBJECTS, COLLISION, LIGHTING };
enum TransformObject { MODEL, COLLBOX };

LightType lightType = FlatColor;
TypeActionKeyBoard typemenu = GAME;
TransformObject transformObject = COLLBOX;

// :::: NUEVO: RESOLUCIÓN NATIVA DEL MONITOR (16:9) ::::
const unsigned int SCR_WIDTH = 1920;
const unsigned int SCR_HEIGHT = 1080;

// :::: NUEVO: RESOLUCIÓN INTERNA DEL JUEGO (4:3 PILLARBOX) ::::
const unsigned int VIEW_WIDTH = 1440;
const unsigned int VIEW_HEIGHT = 1080;

// Desfase para centrar el cuadro 4:3 en la ventana moderna
const int VIEW_OFFSET_X = 240;

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
float bateriaLinterna = 50.0f;

//:::: ENTORNO, TERRENO Y CLIMA :::://
Terrain terrain2;
const char** texturePaths;
glm::vec3 skyPos(0);

// :::: SISTEMA DE PARTÍCULAS (LLUVIA DE SANGRE) ::::
Particles* lluviaSangre;
ParticleProps propsLluvia;

// NUESTRA LISTA PARA EL BOSQUE TÉTRICO
std::vector<glm::vec3> posicionesBosque = {
    glm::vec3(15.0f, 13.0f, 32.0f),
    glm::vec3(30.0f, 13.0f, 0.0f),
    glm::vec3(0.0f, 13.0f, 0.0f),
    glm::vec3(-20.0f, 13.0f, -15.0f),
    glm::vec3(-20.0f, 13.0f, 28.0f),
    glm::vec3(-10.0f, 13.0f, 20.0f),
    glm::vec3(0.0f, 13.0f, 26.0f),
    glm::vec3(28.4f, 13.0f, 38.9f),
    glm::vec3(26.5f, 13.0f, 17.0f),
    glm::vec3(8.9f, 13.0f, 16.3f),
    glm::vec3(-9.8f, 13.0f, 6.5f),
    glm::vec3(4.0f, 13.0f, -14.4f),
    glm::vec3(19.9f, 13.0f, -10.7f),
	glm::vec3(-29.9f, 15.0f, 10.8f),// Este es un árbol más alto para que se vea detrás de la cabaña
    glm::vec3(3.39f, 13.0f, 39.8f),
    glm::vec3(12.2f, 13.0f, 48.3f),
};

// :::: CONTROL DE LA HISTORIA ::::
// 0 = Control Xbox, 1 = Oso, 2 = Tocadiscos, 3 = Cabaña (Final)
int etapaHistoria = 0;

// :::: NUEVO: VARIABLES PARA REINICIAR LA CINEMÁTICA ::::
int framesCarga = 0;
float timerInicio = 0.0f;
bool vozInicioSonada = false;

// :::: ETAPA 0: CONTROL XBOX ::::
glm::vec3 posicionControl = glm::vec3(0.0f, 0.0f, 0.0f); // Se llenará al azar

// :::: ANIMACIÓN STOP-MOTION DEL OSO ::::
bool activandoOso = false; // Se vuelve true cuando presionas 'E'
float timerOso = 0.0f;     // Cronómetro para cambiar de modelo
int frameOso = 0;          // Del 0 al 3 (tus 4 poses)
glm::vec3 posicionFijaOso = glm::vec3(0.0f, 0.0f, 0.0f);
bool vozOsoSonada = false;

// :::: IA DEL CAZADOR (BOSQUE) ::::
// Lo hacemos aparecer en una zona profunda del bosque al iniciar
EntidadIA cazadorBosque(glm::vec3(35.0f, 18.0f, -35.0f));

//:::: INTERACCIÓN Y OBJETOS :::://
// 
bool teclaEPulsada = false;
//cabana
bool abrirPuerta = false;
float anguloPuerta = 0.0f;
glm::vec3 posicionEstructura = glm::vec3(-2.0f, 17.4f, 15.0f);

//auto
bool abrirCajuela = false;
float anguloCajuela = 0.0f;
glm::vec3 posicionAuto = glm::vec3(20.0f, 17.8f, 38.0f);
bool bateriaCajuelaRecogida = false;

//Tocadiscos
bool tocadiscosEncendido = false;
float anguloDisco = 0.0f;
float velocidadDisco = 0.0f; // Para que acelere poco a poco
glm::vec3 posicionTocadiscos = glm::vec3(31.0f, 12.0f, -18.0f); // Ajusta según tu terreno
// :::: NUEVO: VARIABLES DE LA CINEMÁTICA DEL TOCADISCOS ::::
bool jugadorCongelado = false;
float timerTocadiscos = 0.0f;
bool loopTocadiscosActivo = false;

//Objetos
glm::vec3 posicionMesa = glm::vec3(-19.0f, 15.3f, 10.0f);
glm::vec3 posicionBanca = glm::vec3(23.0f, 16.0f, 28.0f);
glm::vec3 posicionSaco = glm::vec3(-19.5f, 16.0f, 15.0f);
glm::vec3 posicionBateria = glm::vec3(20.5f, 17.658f, 37.2f);
std::vector<BateriaRecargable> listaBaterias;
glm::vec3 posicionCartel = glm::vec3(19.5f, 17.700f, 37.2f);

// :::: SECUENCIA FINAL (EL SUSTO Y LA CARTA) ::::

// 1. Coordenadas Maestras (Ajusta estas leyendo tu consola en el juego)
glm::vec3 posicionTriggerSusto = glm::vec3(-9.16f, 18.0f, 4.23f); // Donde pisas para que se apague la luz
glm::vec3 posicionEntidadSusto = glm::vec3(-21.19f, 15.0f, 4.19f); // Donde aparece el monstruo
glm::vec3 posicionCarta = glm::vec3(-19.0f, 17.0f, 10.0f); // Sobre la mesa

// :::: NUEVAS VARIABLES DE AUDIO PARA EL FINAL ::::
bool vozCabanaSonada = false;
float timerPuerta = 0.0f;
bool vozCartaSonada = false;

// 2. Máquina de estados del Susto
bool sustoActivado = false;
bool sustoTerminado = false;
bool mostrarEntidad = false;
float timerSusto = 0.0f;

// 3. Estado del final
bool cartaRecogida = false;

// 4. Variables de Muerte
bool jugadorMuerto = false;
float timerMuerte = 0.0f;
int frameMuerte = 0;

// :::: MOTOR DE AUDIO (18 Pistas) ::::

// 1. Efectos de un solo toque (SFX - One Shots)
Audio sfxPuertaCarro;     
Audio sfxLinterna;        
Audio sfxRecogerBateria;  
Audio sfxPuertaCabana;
Audio sfxCajuelaCarro;

// 2. Voces y Diálogos
Audio vozHombre1; // "Tal vez tenga baterias en la cajuela"
Audio vozHombre2; // "Te voy a encontrar"
Audio vozHombre3; // "...que su cancion favorita"

Audio vozMujer1;  // "Juguemos hermanito"
Audio vozMujer2;  // "No sigas"
Audio vozMujer3;  // "Gracias es lo que queria"
Audio vozMujer4;  // "Por que lo haces"
Audio vozMujer5;  // "Que haces aqui"     

// 3. Loops de Movimiento (Se encienden al presionar W,A,S,D)
Audio pasosJugadorBosque; 
Audio pasosJugadorCabana;
Audio pasosEntidad;       

// 4. Jumpscares (Máximo volumen)
Audio jsOso;              
Audio jsEntidad;          
Audio jsCabana;           

// 5. Atmósfera y Loops Constantes
Audio loopAmbiental;      
Audio loopLluvia;
Audio musicaIntroTocadiscos;
Audio loopTocadiscos;     
Audio murmullos;          
Audio sonidoEntidad;      
Audio musicaFinal;        

//Vectores y renderizado
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

   /*
   // Controles de Joystick
   bool isJoyStick = false;
   double xJoy = 0.0; double yJoy = 0.0; double yLeftJoy = 0.0;

   // Variables de Shooters
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

   // Modelosno usados
   glm::vec3 posicion_anakin = glm::vec3(0.0, 2.5f, 5.0f);
   glm::vec3 posicion_trampa = glm::vec3(30.0f, 1.5f, 20.0f);
   bool trampa_move = false;
   glm::vec3 posicion_enemigo1;
   bool destruir_enemigo_1 = false;
   map<int, pair<string, CollisionBox>> collboxes_enemigos;

   // Agua
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

   // Lluvia
   float posicion_lluvia = 10.0f;
   float caida_lluvia = 0.5;
   float gotas_lluviaX[200];
   float gotas_lluviaZ[200];
   float gotas_lluviaY[200];
   float gotas_lluviaY_Inicial[200];
   bool primera_vez = true;
   */