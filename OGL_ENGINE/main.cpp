
#include <GLFW/glfw3.h>
#include <engine/Billboard.h>
#include <engine/CollisionBox.h>
#include <engine/Objectives.h>
#include <engine/Particles.h>
#include <engine/Plane.h>
#include <engine/QuadTexture.h>
#include <engine/RigidModel.h>
#include <engine/Terrain.h>
#include <engine/functions.h>
#include <engine/shader_m.h>
#include <engine/skybox.h>
#include <engine/textrenderer.h>
#include <engine/Audio.h>
#include <glad/glad.h>
#include <iostream>
#include <thread>

int main()
{
    //:::: INICIALIZAMOS GLFW CON LA VERSIÓN 3.3 :::://
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Guilty Tie", glfwGetPrimaryMonitor(), NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetJoystickCallback(joystick_callback);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    Shader ourShader("shaders/multiple_lighting.vs", "shaders/multiple_lighting.fs");
    initScene(ourShader);

    Terrain terrain("textures//terrenus_guilty2.png", texturePaths);
    SkyBox sky(1.0f, "6");
    SkyBox skySangre(1.0f, "sangre");

    // :::: INICIALIZAR EL HUD DE TEXTO ::::
    TextRenderer Text(VIEW_WIDTH, VIEW_HEIGHT);
    Text.Load("fonts/fuente.ttf", 24);

    // :::: PEGAR OBJETOS ALEATORIOS AL SUELO ::::
    for (int i = 0; i < listaBaterias.size(); i++) {
        glm::vec3 posActual = listaBaterias[i].getPosicion();
        float alturaReal = (terrain.Superficie(posActual.x, posActual.z) * 300.0f) - 2.5f;

        posActual.y = alturaReal + 0.2f;

        listaBaterias[i].setPosicion(posActual);
    }

    // :::: GENERAR EL CONTROL ::::
    std::vector<glm::vec3> posiblesSpawns = {
        glm::vec3(12.0f, 18.0f, 32.0f),   
        //glm::vec3(-20.0f, 18.0f, 5.0f),   
        //glm::vec3(25.0f, 18.0f, -25.0f),  
        //glm::vec3(0.0f, 18.0f, 10.0f)     
    };

    int indiceSpawn = rand() % posiblesSpawns.size();
    posicionControl = posiblesSpawns[indiceSpawn];

    float alturaControl = (terrain.Superficie(posicionControl.x, posicionControl.z) * 300.0f) - 2.5f;
    posicionControl.y = alturaControl + 0.3f;

    // :::: GENERAR EL OSO ::::
    std::vector<glm::vec3> posiblesSpawnsOso = {
        glm::vec3(18.77f, 15.4f, -23.5f),  
        //glm::vec3(-15.0f, 18.0f, -30.0f),  
        //glm::vec3(35.0f, 18.0f, 15.0f),     
        //glm::vec3(-25.0f, 18.0f, 25.0f)     
    };

    int indiceOso = rand() % posiblesSpawnsOso.size();
    posicionFijaOso = posiblesSpawnsOso[indiceOso];

    float alturaOso = (terrain.Superficie(posicionFijaOso.x, posicionFijaOso.z) * 300.0f) - 2.5f;
    posicionFijaOso.y = alturaOso + 0.1f;

    //CorreccionCamara
    float alturaInicial = terrain.Superficie(camera.Position.x, camera.Position.z) * 300.0f;
    camera.PosPersonaje.y = alturaInicial;
    camera.Position.y = camera.PosPersonaje.y + 1.8f;

    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = glfwGetTime();
        deltaTime = (currentFrame - lastFrame);
        lastFrame = currentFrame;
        processInput(window);
        //std::cout << "X: " << camera.Position.x << " Y: " << camera.Position.y << " Z: " << camera.Position.z << std::endl;

        // :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
        // :::: TIMELINE DE LA CINEMÁTICA DEL TOCADISCOS :::::::::::::
        // :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
        if (etapaHistoria >= 3 && tocadiscosEncendido) {
            timerTocadiscos += deltaTime;

            if (jugadorCongelado && timerTocadiscos > 20.0f) {
                jugadorCongelado = false;
            }

            float duracionIntro = 296.9f;

            if (timerTocadiscos > duracionIntro && !loopTocadiscosActivo) {
                loopTocadiscos.Play();
                loopTocadiscosActivo = true;
            }
        }

       // :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
        // :::: SECUENCIA CINEMÁTICA DE INICIO :::::::::::::::::::::::
        // :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

        if (!vozInicioSonada && etapaHistoria == 0) {
            framesCarga++;

            if (framesCarga > 10) {
                timerInicio += deltaTime;

                if (timerInicio > 1.5f) {
                    vozHombre1.Play();
                    vozInicioSonada = true;
                }
            }
        }

        glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // 2. :::: APLICAR EL PILLARBOX 4:3 ::::
        glViewport(VIEW_OFFSET_X, 0, VIEW_WIDTH, VIEW_HEIGHT);

        ourShader.use();
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)VIEW_WIDTH / (float)VIEW_HEIGHT, 0.1f, 1000.0f);
        glm::mat4 view = camera.GetViewMatrix();
        ourShader.setMat4("projection", projection);
        ourShader.setMat4("view", view);

        drawModels(&ourShader, view, projection);
        
        if (etapaHistoria < 3) {
            loadEnviroment(&terrain, &sky, view, projection);
        }
        else {
            loadEnviroment(&terrain, &skySangre, view, projection);
        }

        // ::::LIMITES DEL MAPA::::
        if (camera.PosPersonaje.x > 49.0f) camera.PosPersonaje.x = 49.0f;
        if (camera.PosPersonaje.x < -49.0f) camera.PosPersonaje.x = -49.0f;
        if (camera.PosPersonaje.z > 49.0f) camera.PosPersonaje.z = 49.0f;
        if (camera.PosPersonaje.z < -49.0f) camera.PosPersonaje.z = -49.0f;
        // Camara estricta de 1ra persona
        camera.Position.x = camera.PosPersonaje.x;
        camera.Position.z = camera.PosPersonaje.z;

        //Calculamos donde esta el suelo
        float altura_matematica = terrain.Superficie(camera.Position.x, camera.Position.z);
        float altura_objetivo = (altura_matematica * 300.0f);

        // Ajuste lento
        float velocidad_suavizado = 5.0f * deltaTime;
        camera.PosPersonaje.y = camera.PosPersonaje.y + (altura_objetivo - camera.PosPersonaje.y) * velocidad_suavizado;

        camera.Position.y = camera.PosPersonaje.y + 1.8f;

        // :::: SUSTO EN LA CABAÑA ::::
        if (etapaHistoria == 3 && !sustoTerminado) {

            if (!sustoActivado) {
                float distTrigger = glm::distance(camera.Position, posicionTriggerSusto);

                if (distTrigger < 1.5f) {
                    sustoActivado = true;
                    mostrarEntidad = true;
                    linternaEncendida = false;
                    jsCabana.Play();
                }
            }
            else {
                timerSusto += deltaTime;

                if (timerSusto < 0.5f) {
                    linternaEncendida = false;
                    mostrarEntidad = true;
                }
                else if (timerSusto >= 0.5f && timerSusto < 0.8f) {
                    linternaEncendida = true;
                    mostrarEntidad = true;
                }
                else if (timerSusto >= 0.8f && timerSusto < 1.5f) {
                    linternaEncendida = false;
                    mostrarEntidad = false;
                }
                else if (timerSusto >= 1.5f) {
                    linternaEncendida = true;
                    mostrarEntidad = false;
                    sustoTerminado = true;
                }
            }
        }

        // :::: DRENAJE DE BATERIA ::::
        if (linternaEncendida) {
            // Se gasta 2% por cada segundo real
            bateriaLinterna -= 0.4f * deltaTime;

            if (bateriaLinterna <= 0.0f) {
                bateriaLinterna = 0.0f;
                linternaEncendida = false;
            }
        }

        // :::: REPRODUCTOR DE ANIMACION STOP-MOTION (OSO) ::::
        if (activandoOso) {
            timerOso += deltaTime;

            if (timerOso > 0.15f) {
                timerOso = 0.0f;
                frameOso++;

                if (frameOso > 3) {
                    activandoOso = false;
                    etapaHistoria = 2; 
                }
            }
        }

        // :::: ACTUALIZADOR DE PARTICULAS ::::
        // El sistema se actualiza en cada frame basándose en el tiempo
        if (lluviaSangre != nullptr) {
            lluviaSangre->OnUpdate(deltaTime);
        }

        // :::: EMISOR DE LLUVIA DE SANGRE  ::::
        if (etapaHistoria >= 3) {
            for (int i = 0; i < 15; i++) {

                float offsetX = (static_cast<float>(rand()) / RAND_MAX) * 60.0f - 30.0f;
                float offsetZ = (static_cast<float>(rand()) / RAND_MAX) * 60.0f - 30.0f;

                propsLluvia.Position = camera.Position + glm::vec3(offsetX, 10.0f, offsetZ);

                lluviaSangre->Emit(propsLluvia);
            }
        }

        // :::: CEREBRO DE LA IA ::::
        if (etapaHistoria == 1 || etapaHistoria == 2) {
            cazadorBosque.actualizar(deltaTime, camera.Position, camera.Front, linternaEncendida, &terrain);

            float distIA = glm::distance(camera.Position, cazadorBosque.getPosicion());

            float distMinima = 12.0f;
            float distMaxima = 45.0f;

            int volMax = 1000;
            int volMin = 500;

            int volIA = volMax;

            if (distIA > distMinima) {
                float porcentaje = (distIA - distMinima) / (distMaxima - distMinima);
                if (porcentaje > 1.0f) porcentaje = 1.0f;
                volIA = volMax - (int)(porcentaje * (volMax - volMin));
            }

                // :::: SOLO SUENA CUANDO APARECE LA ALERTA DE PELIGRO ::::
                if (cazadorBosque.mostrarAlerta()) {
                    sonidoEntidad.SetVolume(volIA);
                    pasosEntidad.SetVolume(volIA);
                }
                else {
                    sonidoEntidad.SetVolume(0);
                    pasosEntidad.SetVolume(0);
                }

            // CONDICION DE MUERTE
            if (distIA < 5.0f) {
                jugadorMuerto = true;
                etapaHistoria = 99;
                linternaEncendida = false;
                sonidoEntidad.Stop();
                pasosEntidad.Stop();
                pasosJugadorBosque.Stop();
                loopAmbiental.Stop();
                loopLluvia.Stop();
                loopTocadiscos.Stop();

                jsEntidad.Play();
            }
        }

        // :::: CRONÓMETRO DEL JUMPSCARE  ::::
        if (jugadorMuerto) {
            timerMuerte += deltaTime;

            if (frameMuerte < 5) {
                if (timerMuerte > 0.2f) { 
                    timerMuerte = 0.0f;
                    frameMuerte++;
                }
            }
        }

        collisions();

        // =================================================================
        // :::::::::::::: SISTEMA MODULAR DE HUD Y TEXTOS ::::::::::::::::::
        // =================================================================
        // Al estar en 4:3 Pillarbox, nuestro lienzo de texto mide exactamente 
        // VIEW_WIDTH (1440) x VIEW_HEIGHT (1080). Todas las coordenadas usan este lienzo.

        float centroX = VIEW_WIDTH / 2.0f;  
        float centroY = VIEW_HEIGHT / 2.0f; 

        // 1. CONTROLES DE LA BARRA DE BATERÍA
        float batX = 40.0f;
        float batY = 40.0f;
        float batEscala = 1.0f;

        // 2. CONTROLES DE ALERTA "¡CUIDADO!" 
        float alertaX = centroX - 190.0f;   
        float alertaY = VIEW_HEIGHT - 80.0f;
        float alertaEscala = 1.5f;

        // 3. CONTROLES DE OBJETIVOS (Abajo al centro)
        float objX = centroX - 190.0f;
        float objY = VIEW_HEIGHT - 80.0f;
        float objEscala = 1.3f;

        // 4. CONTROLES DE GAME OVER
        float goTituloX = centroX - 450;
        float goTituloY = centroY;
        float goTituloEscala = 3.0f;

        float goSubX = centroX - 300.0f;
        float goSubY = centroY + 200.0f;
        float goSubEscala = 1.2f;

        // 5. CONTROLES DE CINEMÁTICA FINAL 
        float cartaX = centroX - 150.0f;
        float cartaYInicial = centroY - 100.0f;
        float cartaEspaciadoY = 60.0f;
        float cartaEscala = 1.2f;

        // =================================================================
        // :::::::::::::::::::: RENDERIZADO DEL HUD ::::::::::::::::::::::::
        // =================================================================

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDisable(GL_DEPTH_TEST);

        // :::: A) CINEMÁTICA FINAL (ETAPA 4) ::::
        if (etapaHistoria == 4) {
            glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            Text.RenderText("Querida familia...", cartaX, cartaYInicial, cartaEscala, glm::vec3(1.0f, 1.0f, 1.0f));
            Text.RenderText("Lo siento mucho.", cartaX, cartaYInicial + cartaEspaciadoY, cartaEscala, glm::vec3(1.0f, 1.0f, 1.0f));
        }
        // :::: B) PANTALLA DE GAME OVER ::::
        else if (jugadorMuerto && frameMuerte >= 5) {
            Text.RenderText("P E R D I S T E", goTituloX, goTituloY, goTituloEscala, glm::vec3(1.0f, 0.0f, 0.0f));
            Text.RenderText("Presiona R para reintentar", goSubX, goSubY, goSubEscala, glm::vec3(1.0f, 1.0f, 1.0f));
        }
        // :::: C) HUD NORMAL DE JUEGO (ETAPAS 0 AL 3) ::::
        else {
            // 1. Cálculo y dibujado de la Batería
            int numRayitas = (int)((bateriaLinterna / 100.0f) * 20.0f);
            std::string barraVisual = "[";
            for (int i = 0; i < 20; i++) {
                if (i < numRayitas) barraVisual += "|";
                else barraVisual += " ";
            }
            barraVisual += "]";

            std::string textoBateria = "Bateria " + barraVisual + " " + std::to_string((int)bateriaLinterna) + "%";
            Text.RenderText(textoBateria, batX, batY, batEscala, glm::vec3(1.0f, 1.0f, 1.0f));

            // 2. Alerta de Peligro
            if ((etapaHistoria == 1 || etapaHistoria == 2) && cazadorBosque.mostrarAlerta()) {
                Text.RenderText("¡ C U I D A D O !", alertaX, alertaY, alertaEscala, glm::vec3(1.0f, 0.0f, 0.0f));
            }

            // 3. Objetivos en Pantalla (Etapa 0)
            if (etapaHistoria == 0) {
                if (!abrirCajuela) {
                    Text.RenderText("Revisa la cajuela", objX, objY, objEscala, glm::vec3(0.8f, 0.8f, 0.8f));
                }
                else {
                    Text.RenderText("E N C U E N T R A L A", objX - 20.0f, objY, objEscala, glm::vec3(1.0f, 0.0f, 0.0f));
                }
            }
        }
        glEnable(GL_DEPTH_TEST);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    delete[] texturePaths;
	delete lluviaSangre;
	sky.Release();
    skySangre.Release();
    terrain.Release();
    glfwTerminate();

    return 0;
}

void initScene(Shader ourShader)
{
    //POSICION INICIAL DEL JUGADOR
    camera.Position = posicionAuto + glm::vec3(-6.5f, 1.8f, 5.0f);
	camera.PosPersonaje = camera.Position;

    camera.Yaw = 0.0f;
	camera.Pitch = -15.0f;

    camera.updateCameraVectors();
    camera.PosPersonaje = camera.Position;

    //TEXTURAS DEL SUELO
    texturePaths = new const char* [4];
    texturePaths[0] = "textures/multitexturaGT.jpg";
    texturePaths[1] = "textures/Lodo2.jpg";
    texturePaths[2] = "textures/Bosque.jpg";
    texturePaths[3] = "textures/Grava.jpg";

    //LUCES DEL MAPA
    pointLightPositions.push_back(glm::vec3(20.3f, 5.2f, 20.0f));
    pointLightPositions.push_back(glm::vec3(20.3f, 2.0f, 30.0f));
    pointLightPositions.push_back(glm::vec3(1.0f, 9.3f, -7.0f));
    pointLightPositions.push_back(glm::vec3(0.0f, 10.0f, -3.0f));

    models.push_back(Model("Linterna", "models/Linterna/Flashlight.obj", glm::vec3(0.0f), glm::vec3(0.0f), 0.0f, 0.05f));//1
    models.push_back(Model("Cabana", "models/Cabana/Cabana.obj", glm::vec3(0.0f), glm::vec3(0.0f), 0.0f, 1.0f));//2
    models.push_back(Model("Puerta", "models/Cabana/Puerta.obj", glm::vec3(0.0f), glm::vec3(0.0f), 0.0f, 1.0f));//3
    models.push_back(Model("Pino1", "models/Pinos/Pino1.obj", glm::vec3(0.0f), glm::vec3(0.0f), 0.0f, 1.0f));//4
    models.push_back(Model("Pino2", "models/Pinos/Pino2.obj", glm::vec3(0.0f), glm::vec3(0.0f), 0.0f, 1.0f));//5
    models.push_back(Model("Pino3", "models/Pinos/Pino3.obj", glm::vec3(0.0f), glm::vec3(0.0f), 0.0f, 1.0f));//6
    models.push_back(Model("Auto", "models/Carro/Carro.obj", glm::vec3(0.0f), glm::vec3(0.0f), 0.0f, 1.0f));//7
	models.push_back(Model("Cajuela", "models/Carro/Cajuela.obj", glm::vec3(0.0f), glm::vec3(0.0f), 0.0f, 1.0f));//8
    models.push_back(Model("Toca_Base", "models/TocaDiscos/Toca_Base.obj", glm::vec3(0.0f), glm::vec3(0.0f), 0.0f, 1.0f));//9
    models.push_back(Model("Toca_Disco", "models/TocaDiscos/Toca_Disco.obj", glm::vec3(0.0f), glm::vec3(0.0f), 0.0f, 1.0f));//10
    models.push_back(Model("Cartel", "models/Cartel/Missing_Poster.obj", glm::vec3(0.0f), glm::vec3(0.0f), 0.0f, 1.0f));//11
    models.push_back(Model("Bateria", "models/Bateria/Bateria.obj", glm::vec3(0.0f), glm::vec3(0.0f), 0.0f, 1.0f));//12
    models.push_back(Model("Mesa", "models/Mesa/Mesa.obj", glm::vec3(0.0f), glm::vec3(0.0f), 0.0f, 1.0f));//13
    models.push_back(Model("Banca", "models/Banca/Banca.obj", glm::vec3(0.0f), glm::vec3(0.0f), 0.0f, 1.0f));//14
    models.push_back(Model("Saco", "models/SacoDormir/SacoDormir.obj", glm::vec3(0.0f), glm::vec3(0.0f), 0.0f, 1.0f));//15
    models.push_back(Model("Oso_F0", "models/Oso/Oso_Pose1.obj", glm::vec3(0.0f), glm::vec3(0.0f), 0.0f, 1.0f));//16
    models.push_back(Model("Oso_F1", "models/Oso/Oso_Pose2.obj", glm::vec3(0.0f), glm::vec3(0.0f), 0.0f, 1.0f));//17
    models.push_back(Model("Oso_F2", "models/Oso/Oso_Pose3.obj", glm::vec3(0.0f), glm::vec3(0.0f), 0.0f, 1.0f));//18
    models.push_back(Model("Oso_F3", "models/Oso/Oso_Pose4.obj", glm::vec3(0.0f), glm::vec3(0.0f), 0.0f, 1.0f));//19
    models.push_back(Model("Control", "models/Control/Control.obj", glm::vec3(0.0f), glm::vec3(0.0f), 0.0f, 1.0f));//20
    models.push_back(Model("Entidad_Est", "models/Entidad/Entidad_Estatica.obj", glm::vec3(0.0f), glm::vec3(0.0f), 0.0f, 1.0f)); // 21
    models.push_back(Model("Entidad_C1", "models/Entidad/Caminado/Entidad_F1.obj", glm::vec3(0.0f), glm::vec3(0.0f), 0.0f, 1.0f)); // 22
    models.push_back(Model("Entidad_C2", "models/Entidad/Caminado/Entidad_F2.obj", glm::vec3(0.0f), glm::vec3(0.0f), 0.0f, 1.0f)); // 23
    models.push_back(Model("Entidad_C3", "models/Entidad/Caminado/Entidad_F3.obj", glm::vec3(0.0f), glm::vec3(0.0f), 0.0f, 1.0f)); // 24
    models.push_back(Model("Entidad_C4", "models/Entidad/Caminado/Entidad_F4.obj", glm::vec3(0.0f), glm::vec3(0.0f), 0.0f, 1.0f)); // 25
    models.push_back(Model("Entidad_JS1", "models/Entidad/JumpScare/Entidad_JS_1.obj", glm::vec3(0.0f), glm::vec3(0.0f), 0.0f, 1.0f)); // 26
    models.push_back(Model("Entidad_JS2", "models/Entidad/JumpScare/Entidad_JS_2.obj", glm::vec3(0.0f), glm::vec3(0.0f), 0.0f, 1.0f)); // 27
    models.push_back(Model("Entidad_JS3", "models/Entidad/JumpScare/Entidad_JS_3.obj", glm::vec3(0.0f), glm::vec3(0.0f), 0.0f, 1.0f)); // 28
    models.push_back(Model("Entidad_JS4", "models/Entidad/JumpScare/Entidad_JS_4.obj", glm::vec3(0.0f), glm::vec3(0.0f), 0.0f, 1.0f)); // 29
    models.push_back(Model("Entidad_JS5", "models/Entidad/JumpScare/Entidad_JS_5.obj", glm::vec3(0.0f), glm::vec3(0.0f), 0.0f, 1.0f)); // 30
    models.push_back(Model("Entidad_JS6", "models/Entidad/JumpScare/Entidad_JS_6.obj", glm::vec3(0.0f), glm::vec3(0.0f), 0.0f, 1.0f)); // 31
    models.push_back(Model("Carta", "models/Carta/Carta.obj", glm::vec3(0.0f), glm::vec3(0.0f), 0.0f, 1.0f)); // 32


    // :::: GENERADOR DE BATERIAS ALEATORIAS ::::
    srand(static_cast<unsigned int>(time(0)));

    for (int i = 0; i < 5; i++) {

        float randX = -40.0f + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / 80.0f));
        float randZ = -40.0f + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / 80.0f));
        glm::vec3 posAleatoria = glm::vec3(randX, 18.0f, randZ);

        // Usamos la clase BateriaRecargable que creamos
        listaBaterias.push_back(BateriaRecargable("Bateria_" + std::to_string(i), posAleatoria, 25.0f));
    }

    // :::: INICIALIZAR SISTEMA DE LLUVIA DE SANGRE ::::
    // Asegúrate de tener esta textura en tu carpeta
    lluviaSangre = new Particles("textures/gota_sangre.png");

    // Configuración física de la gota de sangre
    propsLluvia.ColorBegin = glm::vec4(0.8f, 0.0f, 0.0f, 1.0f); // Rojo oscuro al nacer
    propsLluvia.ColorEnd = glm::vec4(0.4f, 0.0f, 0.0f, 0.0f);   // Rojo más oscuro y transparente al morir
    propsLluvia.SizeBegin = 0.15f; // Tamaño inicial
    propsLluvia.SizeEnd = 0.15f;   // Tamaño final
    propsLluvia.SizeVariation = 0.05f;

    // Configuración de la caída
    propsLluvia.Velocity = glm::vec3(0.0f, -50.0f, 0.0f); // Caen directo hacia abajo rápido
    propsLluvia.VelocityVariation = glm::vec3(2.0f, 3.0f, 2.0f); // Ligera variación por el viento
    propsLluvia.LifeTime = 10.0f; // Duran 1.5 segundos antes de desaparecer al tocar el suelo

    // :::: COLISIONES DEL ENTORNO ::::

    // 1. EL CARRO
    CollisionBox cajaCarro(posicionAuto + glm::vec3(0.0f, 0.0f, 5.0f), glm::vec3(2.5f, 3.0f, 7.0f), glm::vec4(1.0f, 0.5f, 0.0f, 1.0f), false, false);
    collboxes.insert({ 10, {"Carro", cajaCarro} });

    // 2. LOS ARBOLES
    for (int i = 0; i < posicionesBosque.size(); i++) {
        float escalaPino = 2.0f + ((i % 5) * 0.4f);

        float radioTronco = 0.42f * escalaPino;

        CollisionBox cajaArbol(posicionesBosque[i], glm::vec3(radioTronco, 8.0f, radioTronco), glm::vec4(0.0f, 1.0f, 0.0f, 1.0f), false, false);
        collboxes.insert({ 200 + i, {"Arbol_" + std::to_string(i), cajaArbol} });
    }

    glEnable(GL_DEPTH_TEST);
    camera.setCollBox();
    ourShader.use();

    cargarAudios();       
    loopAmbiental.Play();
    sfxPuertaCarro.Play();
}

void loadEnviroment(Terrain* terrain, SkyBox* sky, glm::mat4 view, glm::mat4 projection)
{
    //luces y configuraciones de terreno
    terrain->getShader()->use();
    setMultipleLight(terrain->getShader(), pointLightPositions);
    terrain->getShader()->setFloat("shininess", 10.0f);

    //dibujar terreno
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(0.0, -2.5f, 0.0f));
    model = glm::scale(model, glm::vec3(100.5f, 300.0f, 100.5f));
    terrain->draw(model, view, projection);

    //luces cielo
    sky->getShader()->use();
    setMultipleLight(sky->getShader(), pointLightPositions);
    sky->getShader()->setFloat("shininess", 10.0f);

    //dibujar cielo
    glm::mat4 skyModel = glm::mat4(1.0f);
    skyModel = glm::translate(skyModel, glm::vec3(0.0f, 0.0f, 0.0f));
    skyModel = glm::scale(skyModel, glm::vec3(200.0f, 200.0f, 200.0f));
    sky->draw(skyModel, view, projection, skyPos);
}

void drawModels(Shader* shader, glm::mat4 view, glm::mat4 projection)
{
    // Configuraciones de luz base
    shader->setFloat("material.shininess", 10.0f);
    setMultipleLight(shader, pointLightPositions);

    // 1. OBJETOS GLOBALES (Siempre se dibujan sin importar la historia)
    dibujarLinterna(shader);
    dibujarBosque(shader);
    dibujarBateriasAleatorias(shader);
	dibujarCarro(shader);

    switch (etapaHistoria) {
    case 0:
        // Etapa inicial: Buscar el control
        dibujarControlXbox(shader);
        break;
    case 1:
        // Etapa de tensión: El susto del oso
        dibujarOsoStopMotion(shader);
		dibujarCazadorBosque(shader);
        break;
    case 2:
        // Etapa de descubrimiento: El Tocadiscos apagado
        dibujarTocadiscos(shader);
        dibujarCazadorBosque(shader);
        break;
    case 3:
        // Etapa Final: Lluvia de Sangre y Cabaña
        dibujarTocadiscos(shader);  // Sigue girando
        dibujarCabanaFinal(shader); // Cabaña, Auto, Muebles y Secretos
		dibujarEntidadSusto(shader); // La entidad que aparece en la cabaña
		dibujarCarta(shader); // La carta que aparece al fondo del mapa

        // :::: DIBUJAR LA LLUVIA DE SANGRE ::::
        if (lluviaSangre != nullptr) {
            lluviaSangre->Draw(glm::vec3(0.0f), view, projection);
        }

        break;

    case 99:
        dibujarJumpscareMuerte(shader);
        break;
    }
    for (auto& item : collboxes) {
        item.second.second.draw(view, projection);
    }
}

void setMultipleLight(Shader* shader, vector<glm::vec3> pointLightPositions)
{
    shader->setVec3("viewPos", camera.Position);

    // :::: LUZ AMBIENTAL DINÁMICA (LUNA O SANGRE) ::::
    if (etapaHistoria < 3) {
        shader->setVec3("dirLights[0].direction", glm::vec3(-0.2f, -1.0f, -0.3f));
        shader->setVec3("dirLights[0].ambient", 0.02f, 0.025f, 0.04f); // ANTES 0.04, 0.045, 0.07
        shader->setVec3("dirLights[0].diffuse", 0.01f, 0.015f, 0.02f); // ANTES 0.025, 0.03, 0.04
    }
    else {
        shader->setVec3("dirLights[0].direction", glm::vec3(-0.2f, -1.0f, -0.3f));
        shader->setVec3("dirLights[0].ambient", 0.045f, 0.008f, 0.008f);
        shader->setVec3("dirLights[0].diffuse", 0.05f, 0.01f, 0.01f);
    }
    shader->setVec3("dirLights[0].specular", 0.0f, 0.0f, 0.0f);

    // :::: APAGADO ABSOLUTO DE LUCES EXTRAS ::::
    for (int i = 1; i < 4; i++) {
        string num = std::to_string(i);

        // Limpiar DirLights
        shader->setVec3("dirLights[" + num + "].direction", glm::vec3(0.0f, -1.0f, 0.0f));
        shader->setVec3("dirLights[" + num + "].ambient", 0.0f, 0.0f, 0.0f);
        shader->setVec3("dirLights[" + num + "].diffuse", 0.0f, 0.0f, 0.0f);
        shader->setVec3("dirLights[" + num + "].specular", 0.0f, 0.0f, 0.0f);

        // Limpiar PointLights
        shader->setVec3("pointLights[" + num + "].position", glm::vec3(0.0f, -10.0f, 0.0f));
        shader->setVec3("pointLights[" + num + "].ambient", 0.0f, 0.0f, 0.0f);
        shader->setVec3("pointLights[" + num + "].diffuse", 0.0f, 0.0f, 0.0f);
        shader->setVec3("pointLights[" + num + "].specular", 0.0f, 0.0f, 0.0f);
        shader->setFloat("pointLights[" + num + "].constant", 1.0f);

        // Limpiar SpotLights
        shader->setVec3("spotLights[" + num + "].direction", glm::vec3(0.0f, -1.0f, 0.0f));
        shader->setVec3("spotLights[" + num + "].position", glm::vec3(0.0f, -10.0f, 0.0f));
        shader->setVec3("spotLights[" + num + "].ambient", 0.0f, 0.0f, 0.0f);
        shader->setVec3("spotLights[" + num + "].diffuse", 0.0f, 0.0f, 0.0f);
        shader->setVec3("spotLights[" + num + "].specular", 0.0f, 0.0f, 0.0f);
        shader->setFloat("spotLights[" + num + "].constant", 1.0f);
    }

    if (linternaEncendida)
    {
        glm::vec3 puntaLinterna = camera.Position + (camera.Right * 0.45f) + (camera.Up * -0.35f) + (camera.Front * 0.2f);

        shader->setVec3("pointLights[0].position", puntaLinterna);
        shader->setVec3("pointLights[0].ambient", 0.0f, 0.0f, 0.0f);
        shader->setVec3("pointLights[0].diffuse", 1.0f, 1.0f, 0.8f);
        shader->setVec3("pointLights[0].specular", 1.0f, 1.0f, 1.0f);
        shader->setFloat("pointLights[0].constant", 1.0f);
        shader->setFloat("pointLights[0].linear", 2.0f);
        shader->setFloat("pointLights[0].quadratic", 5.0f);
        shader->setVec3("spotLights[0].position", camera.Position + (camera.Right * 0.45f) + (camera.Up * -0.35f));
        shader->setVec3("spotLights[0].direction", camera.Front);
        shader->setVec3("spotLights[0].ambient", 0.0f, 0.0f, 0.0f);
        shader->setVec3("spotLights[0].diffuse", 2.0f, 2.0f, 1.8f);
        shader->setVec3("spotLights[0].specular", 1.0f, 1.0f, 1.0f);
        shader->setFloat("spotLights[0].constant", 1.0f);
        shader->setFloat("spotLights[0].linear", 0.24f);
        shader->setFloat("spotLights[0].quadratic", 0.05f);
        shader->setFloat("spotLights[0].cutOff", glm::cos(glm::radians(10.0f)));
        shader->setFloat("spotLights[0].outerCutOff", glm::cos(glm::radians(15.0f)));
    }
    else
    {
        shader->setVec3("pointLights[0].ambient", 0.0f, 0.0f, 0.0f);
        shader->setVec3("pointLights[0].diffuse", 0.0f, 0.0f, 0.0f);
        shader->setVec3("pointLights[0].specular", 0.0f, 0.0f, 0.0f);
        shader->setFloat("pointLights[0].constant", 1.0f);

        shader->setVec3("spotLights[0].direction", glm::vec3(0.0f, -1.0f, 0.0f));
        shader->setVec3("spotLights[0].ambient", 0.0f, 0.0f, 0.0f);
        shader->setVec3("spotLights[0].diffuse", 0.0f, 0.0f, 0.0f);
        shader->setVec3("spotLights[0].specular", 0.0f, 0.0f, 0.0f);
        shader->setFloat("spotLights[0].constant", 1.0f);
    }

    shader->setInt("lightType", 1);
    shader->setInt("maxRenderLights", 4);
}

void collisions()
{
    float radioCam = 0.2f;
    glm::vec3 camMin = camera.Position - glm::vec3(radioCam);
    glm::vec3 camMax = camera.Position + glm::vec3(radioCam);

    for (auto const& item : collboxes) {

        // item.second es el 'pair' que guarda el string y la caja.
        // Por lo tanto, item.second.second es la CollisionBox.
        CollisionBox box = item.second.second;

        // Verificamos si los "cubos" se están entrelazando en los 3 ejes espaciales
        bool colX = camMax.x > box.min.x && camMin.x < box.max.x;
        bool colY = camMax.y > box.min.y && camMin.y < box.max.y;
        bool colZ = camMax.z > box.min.z && camMin.z < box.max.z;

        if (colX && colY && colZ) {
            // ¡Chocaste! Calculamos cuántos centímetros penetraste la pared por cada lado
            float penIzq = camMax.x - box.min.x;
            float penDer = box.max.x - camMin.x;
            float penAtras = camMax.z - box.min.z;
            float penFrente = box.max.z - camMin.z;

            // Encontramos la cara de la caja que está más cerca de ti
            // (Calculamos el mínimo manualmente para evitar conflictos con Windows)
            float minX = (penIzq < penDer) ? penIzq : penDer;
            float minZ = (penAtras < penFrente) ? penAtras : penFrente;

            // Te expulsamos de la caja exactamente por donde entraste
            if (minX < minZ) {
                camera.Position.x += (penIzq < penDer) ? -minX : minX;
            }
            else {
                camera.Position.z += (penAtras < penFrente) ? -minZ : minZ;
            }

            // Sincronizamos la variable interna de tu cámara para que el motor no pelee con nuestro código
            camera.PosPersonaje.x = camera.Position.x;
            camera.PosPersonaje.z = camera.Position.z;
        }
    }
}
// =================================================================
// ::::::::::::: MODULOS DE RENDERIZADO :::::::::::::::
// =================================================================

void dibujarLinterna(Shader* shader) {
    glm::mat4 modelLinterna = glm::mat4(1.0f);
    modelLinterna = glm::translate(modelLinterna, camera.Position);
    modelLinterna = glm::rotate(modelLinterna, glm::radians(-camera.Yaw - 90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    modelLinterna = glm::rotate(modelLinterna, glm::radians(camera.Pitch), glm::vec3(1.0f, 0.0f, 0.0f));
    modelLinterna = glm::rotate(modelLinterna, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    modelLinterna = glm::translate(modelLinterna, glm::vec3(0.20f, -0.2f, 0.5f));
    modelLinterna = glm::scale(modelLinterna, glm::vec3(0.07f));

    if (!models.empty()) {
        shader->setVec3("dirLights[0].ambient", 0.1f, 0.1f, 0.15f);
        shader->setVec3("dirLights[0].diffuse", 0.0f, 0.0f, 0.0f);
        models[0].Draw(*shader, modelLinterna);
        shader->setVec3("dirLights[0].ambient", 0.03f, 0.03f, 0.05f);
        shader->setVec3("dirLights[0].diffuse", 0.02f, 0.02f, 0.03f);
    }
}

void dibujarBosque(Shader* shader) {
    for (int i = 0; i < posicionesBosque.size(); i++) {
        bool dibujarArbol = true;

        // Deforestación Mágica en la Etapa 3
        if (etapaHistoria >= 3) {
            float distACabana = glm::distance(posicionesBosque[i], posicionEstructura);
            if (distACabana < 16.0f) dibujarArbol = false;
        }

        if (dibujarArbol) {
            glm::mat4 modelPino = glm::mat4(1.0f);
            modelPino = glm::translate(modelPino, posicionesBosque[i]);
            int tipoPino = 3 + (i % 3);
            float escalaPino = 2.0f + ((i % 5) * 0.4f);
            modelPino = glm::scale(modelPino, glm::vec3(escalaPino));
            modelPino = glm::rotate(modelPino, glm::radians(i * 45.0f), glm::vec3(0.0f, 1.0f, 0.0f));

            if (models.size() > tipoPino) models[tipoPino].Draw(*shader, modelPino);
        }
    }
}

void dibujarBateriasAleatorias(Shader* shader) {
    for (int i = 0; i < listaBaterias.size(); i++) {
        if (listaBaterias[i].isActivo()) {
            glm::mat4 modelBat = glm::mat4(1.0f);
            modelBat = glm::translate(modelBat, listaBaterias[i].getPosicion());
            modelBat = glm::scale(modelBat, glm::vec3(4.0f));
            if (models.size() > 11) models[11].Draw(*shader, modelBat);
        }
    }
}

void dibujarControlXbox(Shader* shader) {
    glm::mat4 modelControl = glm::mat4(1.0f);
    modelControl = glm::translate(modelControl, posicionControl);
    modelControl = glm::rotate(modelControl, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    modelControl = glm::scale(modelControl, glm::vec3(0.3f));
    if (models.size() > 19) models[19].Draw(*shader, modelControl);
}

void dibujarOsoStopMotion(Shader* shader) {
    glm::mat4 modelOso = glm::mat4(1.0f);
    modelOso = glm::translate(modelOso, posicionFijaOso);
    modelOso = glm::rotate(modelOso, glm::radians(50.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    modelOso = glm::scale(modelOso, glm::vec3(0.3f));

    int indiceOso = 15 + frameOso;
    if (models.size() > indiceOso) models[indiceOso].Draw(*shader, modelOso);
}

void dibujarTocadiscos(Shader* shader) {
    if (tocadiscosEncendido) {
        if (velocidadDisco < 200.0f) velocidadDisco += 50.0f * deltaTime;
    }
    else {
        if (velocidadDisco > 0.0f) velocidadDisco -= 30.0f * deltaTime;
        if (velocidadDisco < 0.0f) velocidadDisco = 0.0f;
    }
    anguloDisco += velocidadDisco * deltaTime;

    glm::mat4 modelBase = glm::mat4(1.0f);
    modelBase = glm::translate(modelBase, posicionTocadiscos);
    modelBase = glm::scale(modelBase, glm::vec3(0.5f));
    if (models.size() > 8) models[8].Draw(*shader, modelBase);

    glm::mat4 modelDisco = glm::mat4(1.0f);
    modelDisco = glm::translate(modelDisco, posicionTocadiscos);
    modelDisco = glm::rotate(modelDisco, glm::radians(anguloDisco), glm::vec3(0.0f, 1.0f, 0.0f));
    modelDisco = glm::scale(modelDisco, glm::vec3(0.5f));
    if (models.size() > 9) models[9].Draw(*shader, modelDisco);
}

void dibujarCabanaFinal(Shader* shader) {
    float gradosRotacion = 180.0f;

    // CABAÑA Y PUERTA
    glm::mat4 modelCabana = glm::mat4(1.0f);
    modelCabana = glm::translate(modelCabana, posicionEstructura);
    modelCabana = glm::rotate(modelCabana, glm::radians(gradosRotacion), glm::vec3(0.0f, 1.0f, 0.0f));
    modelCabana = glm::scale(modelCabana, glm::vec3(1.0f));
    if (models.size() > 1) models[1].Draw(*shader, modelCabana);

    if (abrirPuerta && anguloPuerta < 90.0f) anguloPuerta += 45.0f * deltaTime;
    glm::mat4 modelPuerta = glm::mat4(1.0f);
    modelPuerta = glm::translate(modelPuerta, posicionEstructura);
    modelPuerta = glm::rotate(modelPuerta, glm::radians(gradosRotacion + anguloPuerta), glm::vec3(0.0f, 1.0f, 0.0f));
    modelPuerta = glm::scale(modelPuerta, glm::vec3(1.0f));
    if (models.size() > 2) models[2].Draw(*shader, modelPuerta);
    // MUEBLES CABAÑA
    glm::mat4 modelMesa = glm::mat4(1.0f);
    modelMesa = glm::translate(modelMesa, posicionMesa);
    modelMesa = glm::scale(modelMesa, glm::vec3(0.5f));
    if (models.size() > 12) models[12].Draw(*shader, modelMesa);

    glm::mat4 modelBanca = glm::mat4(1.0f);
    modelBanca = glm::translate(modelBanca, posicionBanca);
    modelBanca = glm::rotate(modelBanca, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    modelBanca = glm::scale(modelBanca, glm::vec3(3.0f));
    if (models.size() > 13) models[13].Draw(*shader, modelBanca);

    glm::mat4 modelSaco = glm::mat4(1.0f);
    modelSaco = glm::translate(modelSaco, posicionSaco);
    modelSaco = glm::rotate(modelSaco, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    modelSaco = glm::scale(modelSaco, glm::vec3(0.2f));
    if (models.size() > 14) models[14].Draw(*shader, modelSaco);
}

void dibujarCarro(Shader* shader) {
    // AUTO Y CAJUELA
    float orientacionAuto = 270.0f;
    glm::mat4 modelAuto = glm::mat4(1.0f);
    modelAuto = glm::translate(modelAuto, posicionAuto);
    modelAuto = glm::rotate(modelAuto, glm::radians(orientacionAuto), glm::vec3(0.0f, 1.0f, 0.0f));
    modelAuto = glm::scale(modelAuto, glm::vec3(3.1f));
    if (models.size() > 6) models[6].Draw(*shader, modelAuto);

    if (abrirCajuela && anguloCajuela < 60.0f) anguloCajuela += 45.0f * deltaTime;
    else if (!abrirCajuela && anguloCajuela > 0.0f) anguloCajuela -= 45.0f * deltaTime;
    if (anguloCajuela > 60.0f) anguloCajuela = 60.0f;
    if (anguloCajuela < 0.0f) anguloCajuela = 0.0f;

    glm::mat4 modelCajuela = glm::mat4(1.0f);
    modelCajuela = glm::translate(modelCajuela, posicionAuto);
    modelCajuela = glm::rotate(modelCajuela, glm::radians(orientacionAuto), glm::vec3(0.0f, 1.0f, 0.0f));
    modelCajuela = glm::rotate(modelCajuela, glm::radians(-anguloCajuela), glm::vec3(0.0f, 0.0f, 1.0f));
    modelCajuela = glm::scale(modelCajuela, glm::vec3(3.1f));
    if (models.size() > 7) models[7].Draw(*shader, modelCajuela);

    // :::: OBJETOS EN CAJUELA  ::::
    if (anguloCajuela > 20.0f) {
		// CARTEL
        glm::mat4 modelCartel = glm::mat4(1.0f);
        modelCartel = glm::translate(modelCartel, posicionAuto);
        modelCartel = glm::rotate(modelCartel, glm::radians(orientacionAuto), glm::vec3(0.0f, 1.0f, 0.0f));


        modelCartel = glm::translate(modelCartel, glm::vec3(-0.8f, 0.0f, 0.5f));

        modelCartel = glm::rotate(modelCartel, glm::radians(10.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        modelCartel = glm::rotate(modelCartel, glm::radians(5.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        modelCartel = glm::scale(modelCartel, glm::vec3(0.5f));
        if (models.size() > 10) models[10].Draw(*shader, modelCartel);

        // BATERÍA
        if (!bateriaCajuelaRecogida)
        {
            glm::mat4 modelBateria = glm::mat4(1.0f);
            modelBateria = glm::translate(modelBateria, posicionAuto);
            modelBateria = glm::rotate(modelBateria, glm::radians(orientacionAuto), glm::vec3(0.0f, 1.0f, 0.0f));

            modelBateria = glm::translate(modelBateria, glm::vec3(-0.8f, -0.14f, -0.5f));

            modelBateria = glm::rotate(modelBateria, glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
            modelBateria = glm::scale(modelBateria, glm::vec3(7.0f));
            if (models.size() > 11) models[11].Draw(*shader, modelBateria);
        }
    }
}
void dibujarEntidadSusto(Shader* shader) {
    if (mostrarEntidad) {
        glm::mat4 modelEnt = glm::mat4(1.0f);
        modelEnt = glm::translate(modelEnt, posicionEntidadSusto);
        modelEnt = glm::rotate(modelEnt, glm::radians(95.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        modelEnt = glm::scale(modelEnt, glm::vec3(3.5f));
        if (models.size() > 20) models[20].Draw(*shader, modelEnt);
    }
}

void dibujarCarta(Shader* shader) {
    if (!cartaRecogida && etapaHistoria >= 3) {
        glm::mat4 modelCarta = glm::mat4(1.0f);
        modelCarta = glm::translate(modelCarta, posicionCarta);
		modelCarta = glm::rotate(modelCarta, glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        modelCarta = glm::scale(modelCarta, glm::vec3(0.2f));
        if (models.size() > 31) models[31].Draw(*shader, modelCarta);
    }
}
void dibujarJumpscareMuerte(Shader* shader) {
    if (jugadorMuerto) {
        glm::mat4 modelJS = glm::mat4(1.0f);

        // :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
        // :::: ZONA DE AJUSTE MANUAL DEL DIRECTOR (JUMPSCARE) :::::::
        // :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

        // 1. PROFUNDIDAD (Z):
        float alejarDeCamara = 4.7f;

        // 2. ALTURA (Y):
        float bajarCuerpo = 4.0f;

        // 3. HORIZONTAL (X):
        float moverLado = 0.5f;

        // :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

        glm::vec3 posFrente = camera.Position;
        posFrente += camera.Front * alejarDeCamara;
        posFrente += camera.Right * moverLado;
        posFrente.y -= bajarCuerpo;

        modelJS = glm::translate(modelJS, posFrente);

        modelJS = glm::rotate(modelJS, glm::radians(-camera.Yaw - 90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        modelJS = glm::rotate(modelJS, glm::radians(camera.Pitch), glm::vec3(1.0f, 0.0f, 0.0f));

        modelJS = glm::scale(modelJS, glm::vec3(3.5f));

        shader->setVec3("dirLights[0].ambient", 0.4f, 0.05f, 0.05f);
        shader->setVec3("dirLights[0].diffuse", 0.8f, 0.3f, 0.3f);
        shader->setFloat("material.shininess", 16.0f);

        int indiceJS = 25 + frameMuerte;
        if (models.size() > indiceJS) models[indiceJS].Draw(*shader, modelJS);
    }
}
void dibujarCazadorBosque(Shader* shader) {
    if ((etapaHistoria == 1 || etapaHistoria == 2) && !cazadorBosque.estaOculto()) {
        glm::mat4 modelCazador = glm::mat4(1.0f);
        modelCazador = glm::translate(modelCazador, cazadorBosque.getPosicion());

        // :::: ROTACIÓN HACIA EL JUGADOR ::::
        float deltaX = camera.Position.x - cazadorBosque.getPosicion().x;
        float deltaZ = camera.Position.z - cazadorBosque.getPosicion().z;

        float anguloHaciaJugador = atan2(deltaX, deltaZ);

        modelCazador = glm::rotate(modelCazador, anguloHaciaJugador, glm::vec3(0.0f, 1.0f, 0.0f));

        modelCazador = glm::scale(modelCazador, glm::vec3(3.5f));

        int indiceModelo = 21 + cazadorBosque.getFrameAnimacion();
        if (models.size() > indiceModelo) models[indiceModelo].Draw(*shader, modelCazador);
    }
}