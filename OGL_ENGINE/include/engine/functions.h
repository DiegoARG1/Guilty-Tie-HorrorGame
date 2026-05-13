#pragma once
#include <engine/utils.h>
#include <engine/variables.h>

//:::: CALLBACKS  Y FUNCIONES :::://
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void collidedObject_callback(string nameCollidedObject);
void collidedObject_callback(string nameCollidedObject, string nameCollidedObject2);

void joystick_callback(int jid, int event);
void processInput(GLFWwindow* window);
void actionKeys(GLFWwindow* window);

void setMultipleLight(Shader* shader, vector<glm::vec3> pointLightPositions);
void drawModels(Shader* shader, glm::mat4 view, glm::mat4 projection);
// :::: NUESTRAS FUNCIONES MODULARES DE DIBUJADO ::::
void dibujarLinterna(Shader* shader);
void dibujarCarro(Shader* shader);
void dibujarBosque(Shader* shader);
void dibujarBateriasAleatorias(Shader* shader);
void dibujarControlXbox(Shader* shader);
void dibujarOsoStopMotion(Shader* shader);
void dibujarTocadiscos(Shader* shader);
void dibujarCabanaFinal(Shader* shader);
void dibujarEntidadSusto(Shader* shader);
void dibujarCarta(Shader* shader);
void dibujarCazadorBosque(Shader* shader);
void dibujarJumpscareMuerte(Shader* shader);
// :::: COLISIONES ::::
void activarColisionesCabana();
void loadEnviroment(Terrain* terrain, SkyBox* sky, glm::mat4 view, glm::mat4 projection);
void initScene(Shader ourShader);
void collisions();

// :::: CONTROL DE MANDO  ::::
void joystick_callback(int jid, int event) {}

// :::: TECLADO PRINCIPAL ::::
void processInput(GLFWwindow* window)
{
    // Cerrar Juego
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // 2. :::: CONTROLES CUANDO ESTÁS MUERTO (GAME OVER) ::::
    if (jugadorMuerto) {
        // Solo escuchamos la tecla R para reiniciar
        if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
            jugadorMuerto = false;
            etapaHistoria = 0;
            bateriaLinterna = 100.0f;
            linternaEncendida = true;
            timerMuerte = 0.0f;
            frameMuerte = 0;

            camera.Position = posicionAuto + glm::vec3(-6.5f, 1.8f, 5.0f);
            camera.PosPersonaje = camera.Position;
            camera.updateCameraVectors();

            cazadorBosque = EntidadIA(glm::vec3(35.0f, 18.0f, -35.0f));

            abrirCajuela = false;
            anguloCajuela = 0.0f;
            activandoOso = false;
            frameOso = 0;
            tocadiscosEncendido = false;
            sustoActivado = false;
            sustoTerminado = false;
            mostrarEntidad = false;

        }
        return;
    }

    // Movimiento WASD
   // if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
     //   camera.MovementSpeed = 15.0f; // Velocidad de carrera
    //else
     //   camera.MovementSpeed = 3.0f;  // Velocidad normal de caminata

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);

    float distanciaCabana = glm::distance(camera.Position, posicionEstructura);
    float distanciaCajuela = glm::distance(camera.Position, posicionAuto);
    float distToca = glm::distance(camera.Position, posicionTocadiscos);

    // Interaccion:(Tecla E)
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
    {
        if (!teclaEPulsada)
        {
            if (distanciaCabana < 5.0f)
            {
                abrirPuerta = !abrirPuerta;
                teclaEPulsada = true;
                collboxes.erase(105);
            }
            else if (distanciaCajuela < 5.0f)
            {
                abrirCajuela = !abrirCajuela;
                teclaEPulsada = true;
            }
            // :::: RECOGER LA CARTA Y TERMINAR EL JUEGO ::::
            if (etapaHistoria == 3 && sustoTerminado) {
                float distCarta = glm::distance(camera.Position, posicionCarta);
                if (distCarta < 5.0f) {
                    cartaRecogida = true;
                    etapaHistoria = 4;
                    teclaEPulsada = true;

                    // Futuro: musicaFinal.Play();
                }
            }
            // :::: INTERACCIÓN CON EL TOCADISCOS (ETAPA 2) ::::
            if (etapaHistoria == 2) {
                float distToca = glm::distance(camera.Position, posicionTocadiscos);
                if (distToca < 5.0f) {
                    tocadiscosEncendido = true; // Empieza a girar
                    etapaHistoria = 3; // ¡Inicia el final! (Aparición de cabaña y lluvia)
                    teclaEPulsada = true;

					activarColisionesCabana(); // Activamos las colisiones de la cabaña para el final

                    // :::: NUEVO: BORRAR COLISIONES DE LOS ÁRBOLES DEFORESTADOS ::::
                    for (int i = 0; i < posicionesBosque.size(); i++) {
                        float distACabana = glm::distance(posicionesBosque[i], posicionEstructura);
                        if (distACabana < 16.0f) { // El mismo radio de "tala" que en main.cpp
                            collboxes.erase(200 + i);
                        }
                    }

                    std::cout << "Tocadiscos activado. ¡Empieza la lluvia de sangre!" << std::endl;
                    // Aquí conectaremos el audio de la canción distorsionada
                }
            }

            // :::: INTERACCIÓN CON EL CONTROL (ETAPA 0) ::::
            if (etapaHistoria == 0) {
                float distControl = glm::distance(camera.Position, posicionControl);

                if (distControl < 5.0f) {
                    etapaHistoria = 1; // ¡Avanzamos a la etapa del oso!
                    teclaEPulsada = true;

                    std::cout << "Control recogido. 'Hermano, juegas conmigo?'" << std::endl;
                    std::cout << "¡La entidad ha despertado! Busca el oso." << std::endl;

                    // Aquí más adelante pondremos: efecto1.Play(); 
                }
            }

            // :::: INTERACCIÓN CON EL OSO ::::
            if (etapaHistoria == 1 && !activandoOso) {
                float distOso = glm::distance(camera.Position, posicionFijaOso);
                if (distOso < 5.0f) {
                    activandoOso = true; // ¡Inicia la animación de Stop-Motion!
                    teclaEPulsada = true;
                    // Aquí luego pondremos tu Audio de tensión
                }
            }

            // :::: NUEVO: RECOGER BATERÍAS ALEATORIAS ::::
            for (int i = 0; i < listaBaterias.size(); i++) {
                // Verificamos que la batería siga activa (que no la hayas recogido ya)
                if (listaBaterias[i].isActivo()) {
                    float distBateria = glm::distance(camera.Position, listaBaterias[i].getPosicion());

                    if (distBateria < 5.0f) {
                        listaBaterias[i].interactuar(); // El polimorfismo hace que desaparezca

                        bateriaLinterna += 35.0f; // Recargamos la variable de la linterna
                        if (bateriaLinterna > 100.0f) bateriaLinterna = 100.0f; // Límite máximo

                        std::cout << "Bateria recogida! Nivel de linterna: " << bateriaLinterna << "%" << std::endl;

                        teclaEPulsada = true;
                        break; // Solo recogemos una a la vez
                    }
                }
            }

        }
    }
    else
    {
        teclaEPulsada = false;
    }

	//Linterna:(Tecla F)
    if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS)
    {
        if (!teclaFPulsada)
        {
            linternaEncendida = !linternaEncendida;
            teclaFPulsada = true;
        }
    }
    else if (glfwGetKey(window, GLFW_KEY_F) == GLFW_RELEASE)
    {
        teclaFPulsada = false;
    }
}



// :::: EDITOR EN TIEMPO DE EJECUCION::::
void actionKeys(GLFWwindow* window) {}

// :::: RESOLUCIÓN DE PANTALLA ::::
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(VIEW_OFFSET_X, 0, VIEW_WIDTH, VIEW_HEIGHT);
}

// :::: CÁMARA CON EL MOUSE ::::
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (jugadorMuerto) return;
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

// :::: CLICS DEL MOUSE ::::
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {}

// :::: ZOOM ::::
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(yoffset);
}

// :::: EVENTOS DE COLISIÓN ::::
void collidedObject_callback(string nameCollidedObject)
{

}

void collidedObject_callback(string nameCollidedObject, string nameCollidedObject2) {}

void activarColisionesCabana() {
    // RECUERDA: El vector de escala es la MITAD del tamaño real.
    // Si pones vec3(5.0, ...), la pared medirá 10 metros de largo.

    // 1. Pared del Fondo (Roja) - Larga en X, delgada en Z
    CollisionBox paredFondo(posicionEstructura + glm::vec3(-20.6f, 0.0f, -5.6f), glm::vec3(0.2f, 3.0f, 7.2f), glm::vec4(1.0f, 0.0f, 0.0f, 1.0f), false, false);

    // 2. Pared Izquierda (Verde) - Delgada en X, Larga en Z
    CollisionBox paredIzq(posicionEstructura + glm::vec3(-10.3f, 0.0f, 1.5f), glm::vec3(10.5f, 3.0f, 0.2f), glm::vec4(0.0f, 1.0f, 0.0f, 1.0f), false, false);

    // 3. Pared Derecha (Azul) - Delgada en X, Larga en Z
    CollisionBox paredDer(posicionEstructura + glm::vec3(-10.2f, 0.0f, -12.55f), glm::vec3(10.5f, 3.0f, 0.2f), glm::vec4(0.0f, 0.0f, 1.0f, 1.0f), false, false);

    // 4. Pared Frontal Izquierda (Amarilla) - Para dejar el hueco de la puerta
    CollisionBox paredFrente1(posicionEstructura + glm::vec3(0.1f, 0.0f, 0.8f), glm::vec3(0.2f, 3.0f, 0.78f), glm::vec4(1.0f, 1.0f, 0.0f, 1.0f), false, false);

    // 5. Pared Frontal Derecha (Morada) - Para dejar el hueco de la puerta
    CollisionBox paredFrente2(posicionEstructura + glm::vec3(0.1f, 0.0f, -7.35f), glm::vec3(0.2f, 3.0f, 5.4f), glm::vec4(1.0f, 0.0f, 1.0f, 1.0f), false, false);

	// 6. Pared de la Puerta (Naranja) - Para la puerta que se abre
    CollisionBox paredPuerta(posicionEstructura + glm::vec3(0.1f, 0.0f, -1.0f), glm::vec3(0.2f, 3.0f, 1.0f), glm::vec4(1.0f, 0.5f, 0.0f, 1.0f), false, false);

    collboxes.insert({ 100, {"ParedFondo", paredFondo} });
    collboxes.insert({ 101, {"ParedIzq", paredIzq} });
    collboxes.insert({ 102, {"ParedDer", paredDer} });
    collboxes.insert({ 103, {"ParedFrente1", paredFrente1} });
    collboxes.insert({ 104, {"ParedFrente2", paredFrente2} });
    collboxes.insert({ 105, {"Puerta", paredPuerta} });
}
void cargarAudios() {
    // Aquí le pasas la ruta de tu archivo .wav o .mp3 a cada variable
    // Ejemplo (depende de tu librería de audio):
    //sfxPuertaCarro.Load("audio/puerta_carro.wav");
    //vozCajuela.Load("audio/voz_cajuela.wav");
    //loopAmbiental.Load("audio/ambiente_bosque.wav");
    // ... cargas los 18 aquí

    // Configuras los que deben estar en bucle infinito desde el inicio
    //loopAmbiental.Loop(true);
    //loopAmbiental.Play();
}