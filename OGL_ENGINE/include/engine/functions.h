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

    // :::: AVANZAR EN LA PANTALLA FINAL ::::
    if (etapaHistoria == 4 && !cartaLeida) {
        if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS) {
            cartaLeida = true;
        }
    }

    // 2. :::: CONTROLES CUANDO ESTÁS MUERTO (GAME OVER) ::::
    if (jugadorMuerto) {
        // Solo escuchamos la tecla R para reiniciar
        if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
            jugadorMuerto = false;
            jsEntidad.Stop();
            sonidoEntidad.Stop();
            pasosEntidad.Stop();

            // reset
            etapaHistoria = 0;
            bateriaLinterna = 50.0f;
            linternaEncendida = true;
            timerMuerte = 0.0f;
            frameMuerte = 0;

            camera.Position = posicionAuto + glm::vec3(-6.5f, 1.8f, 5.0f);
            camera.PosPersonaje = camera.Position;
            camera.updateCameraVectors();

            cazadorBosque = EntidadIA(glm::vec3(35.0f, 18.0f, -35.0f));

			// reset objetos y triggers
            abrirCajuela = false;
            anguloCajuela = 0.0f;
            bateriaCajuelaRecogida = false;

            activandoOso = false;
            frameOso = 0;
            vozOsoSonada = false;

            tocadiscosEncendido = false;
            velocidadDisco = 0.0f;

            sustoActivado = false;
            sustoTerminado = false;
            mostrarEntidad = false;
            cartaRecogida = false;

            // reset cinematicas
            framesCarga = 0;
            timerInicio = 0.0f;
            vozInicioSonada = false;

            loopAmbiental.Play();
            sfxPuertaCarro.Play();
        }
        return;
    }

    // :::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // :::: SISTEMA DINÁMICO DE PASOS (EXTERIOR VS CABAÑA) :::::
    // :::::::::::::::::::::::::::::::::::::::::::::::::::::::::

    bool intentandoMoverse = (
        glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS ||
        glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS ||
        glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS ||
        glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS
        )&& !jugadorCongelado;

    static bool pasosSonando = false;
    static int tipoSueloActual = -1; // 0 = Exterior, 1 = Cabaña

    if (intentandoMoverse) {
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) camera.ProcessKeyboard(FORWARD, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) camera.ProcessKeyboard(BACKWARD, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) camera.ProcessKeyboard(LEFT, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) camera.ProcessKeyboard(RIGHT, deltaTime);

        // 1. Deducimos si estamos afuera o adentro
        int nuevoTipoSuelo = 0; // Por defecto asumimos Exterior

        // NUEVO CÁLCULO: Zona rectangular exacta de la cabaña basada en tus paredes
        if (etapaHistoria >= 3) {
            // Calculamos dónde estás tú en relación a la bisagra de la puerta
            float relX = camera.Position.x - posicionEstructura.x;
            float relZ = camera.Position.z - posicionEstructura.z;

            // Si estás dentro de los límites de las 4 paredes, pisas madera
            if (relX > -20.6f && relX < 0.1f && relZ > -12.55f && relZ < 1.5f) {
                nuevoTipoSuelo = 1;
            }
        }

        // 2. Si es la primera vez que pisamos, o entramos/salimos de la cabaña
        if (!pasosSonando || tipoSueloActual != nuevoTipoSuelo) {

            // Callamos ambos audios para evitar superposiciones
            pasosJugadorBosque.Stop();
            pasosJugadorCabana.Stop();

            // Reproducimos el correcto en bucle infinito
            if (nuevoTipoSuelo == 0) pasosJugadorBosque.Play();
            else if (nuevoTipoSuelo == 1) pasosJugadorCabana.Play();

            pasosSonando = true;
            tipoSueloActual = nuevoTipoSuelo;
        }
    }
    else {
        // 3. Si soltamos las teclas, apagamos los pasos al instante
        if (pasosSonando) {
            pasosJugadorBosque.Stop();
            pasosJugadorCabana.Stop();
            pasosSonando = false;
        }
    }

    float distanciaCabana = glm::distance(camera.Position, posicionEstructura);
    float distanciaCajuela = glm::distance(camera.Position, posicionAuto);
    float distToca = glm::distance(camera.Position, posicionTocadiscos);

    // :::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // :::: TRIGGER DE PROXIMIDAD: LA NIÑA DEL OSO (ETAPA 1) :::
    // :::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    if (etapaHistoria == 1 && !vozOsoSonada) {
        float distOso = glm::distance(camera.Position, posicionFijaOso);

        if (distOso < 12.0f) {
            vozMujer3.Play();
            vozOsoSonada = true;
        }
    }

    // :::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // :::: TRIGGERS: LA CABAÑA Y LA CARTA (ETAPA 3) :::::::::::
    // :::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    if (etapaHistoria == 3) {

        if (abrirPuerta && !vozCabanaSonada) {
            timerPuerta += deltaTime;

            if (timerPuerta > 1.2f) {
                vozMujer2.Play();
                vozCabanaSonada = true;
            }
        }

        if (sustoTerminado && !vozCartaSonada) {
            float distCarta = glm::distance(camera.Position, posicionCarta);

            if (distCarta < 7.0f) {
                vozMujer5.Play();
                vozCartaSonada = true;
            }
        }
    }

    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
    {
        if (!teclaEPulsada)
        {
            if (distanciaCabana < 5.0f)
            {
                abrirPuerta = !abrirPuerta;
				sfxPuertaCabana.Play();
                teclaEPulsada = true;
                collboxes.erase(105);
            }
            else if (distanciaCajuela < 5.0f)
            {
                if (!abrirCajuela) {
                    abrirCajuela = true;
                    sfxCajuelaCarro.Play();
                    vozHombre2.Play();
                    teclaEPulsada = true;
                }
                else if (!bateriaCajuelaRecogida) {
                    bateriaCajuelaRecogida = true;

                    bateriaLinterna += 35.0f;
                    if (bateriaLinterna > 100.0f) bateriaLinterna = 100.0f;

                    sfxRecogerBateria.Play();
                    vozMujer4.Play();

                    teclaEPulsada = true;
                }
            }
            // :::: RECOGER LA CARTA Y TERMINAR EL JUEGO ::::
            if (etapaHistoria == 3 && sustoTerminado) {
                float distCarta = glm::distance(camera.Position, posicionCarta);
                if (distCarta < 5.0f) {
                    cartaRecogida = true;
                    etapaHistoria = 4;
                    teclaEPulsada = true;

                    // :::: APAGÓN TOTAL DE LA ATMÓSFERA ::::
                    loopLluvia.Stop();
                    loopTocadiscos.Stop();
                    pasosJugadorCabana.Stop();
					musicaIntroTocadiscos.Stop();
					loopTocadiscos.Stop();

                    // :::: MÚSICA DE CRÉDITOS ::::
                    musicaFinal.Play();
                }
            }
            // :::: INTERACCIÓN CON EL TOCADISCOS (ETAPA 2) ::::
            if (etapaHistoria == 2) {
                float distToca = glm::distance(camera.Position, posicionTocadiscos);
                if (distToca < 5.0f) {
                    tocadiscosEncendido = true;
                    etapaHistoria = 3;
                    teclaEPulsada = true;

                    activarColisionesCabana();

                    // :::: BORRAR COLISIONES DE LOS ÁRBOLES DEFORESTADOS ::::
                    for (int i = 0; i < posicionesBosque.size(); i++) {
                        float distACabana = glm::distance(posicionesBosque[i], posicionEstructura);
                        if (distACabana < 16.0f) {
                            collboxes.erase(200 + i);
                        }
                    }

                    // :::: INICIO DE LA CINEMÁTICA DEL TOCADISCOS ::::
                    jugadorCongelado = true;
                    timerTocadiscos = 0.0f;

                    vozHombre3.Play();
                    musicaIntroTocadiscos.Play();
                    sonidoEntidad.Stop();
                    pasosEntidad.Stop();
                    loopAmbiental.Stop();

                    loopLluvia.Play();
                }
            }

            // :::: INTERACCIÓN CON EL CONTROL (ETAPA 0) ::::
            if (etapaHistoria == 0) {
                float distControl = glm::distance(camera.Position, posicionControl);

                if (distControl < 5.0f) {
                    etapaHistoria = 1;
                    teclaEPulsada = true;
                    vozMujer1.Play();
                    sonidoEntidad.SetVolume(0);
                    pasosEntidad.SetVolume(0);
                    sonidoEntidad.Play();
                    pasosEntidad.Play();
                }
            }

            // :::: INTERACCIÓN CON EL OSO ::::
            if (etapaHistoria == 1 && !activandoOso) {
                float distOso = glm::distance(camera.Position, posicionFijaOso);
                if (distOso < 5.0f) {
                    activandoOso = true;
                    teclaEPulsada = true;
                    jsOso.Play();
                }
            }

            // :::: NUEVO: RECOGER BATERÍAS ALEATORIAS ::::
            for (int i = 0; i < listaBaterias.size(); i++) {
                if (listaBaterias[i].isActivo()) {
                    float distBateria = glm::distance(camera.Position, listaBaterias[i].getPosicion());

                    if (distBateria < 5.0f) {
                        listaBaterias[i].interactuar();

                        bateriaLinterna += 35.0f; 
                        if (bateriaLinterna > 100.0f) bateriaLinterna = 100.0f; 
                        sfxRecogerBateria.Play();
                        teclaEPulsada = true;
                        break;
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
            sfxLinterna.Play();
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
	if (jugadorMuerto || jugadorCongelado) return;
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
    // 1. EFECTOS DE SONIDO (Interacciones)
    sfxPuertaCarro.Load("audio/puerta_carro.mp3");
    sfxLinterna.Load("audio/linterna_click.mp3");
    sfxRecogerBateria.Load("audio/recoger_bateria.mp3");
	sfxRecogerBateria.SetVolume(600);
    sfxPuertaCabana.Load("audio/puerta_rechina.mp3");
	sfxPuertaCabana.SetVolume(600);
	sfxCajuelaCarro.Load("audio/cajuela.mp3");
	sfxCajuelaCarro.SetVolume(400);

    // 2. VOCES MASCULINAS
    vozHombre1.Load("audio/Voz_hombre_1.mp3"); // Al aparecer en el juego (batería baja)
    vozHombre1.SetVolume(400);
    vozHombre2.Load("audio/Voz_hombre_2.mp3"); // Cuando abre la cajuela
    vozHombre2.SetVolume(400);
    vozHombre3.Load("audio/Voz_hombre_3.mp3"); // Cuando inicia el tocadiscos
    vozHombre3.SetVolume(400);

    // 3. VOCES FEMENINAS
    vozMujer1.Load("audio/Voz_mujer_1.mp3"); // Toma el control
    vozMujer2.Load("audio/Voz_mujer_2.mp3"); // Entra a la cabaña
    vozMujer3.Load("audio/Voz_mujer_3.mp3"); // Agarra el oso de peluche
    vozMujer4.Load("audio/Voz_mujer_4.mp3"); // Agarra batería de la cajuela
    vozMujer5.Load("audio/Voz_mujer_5.mp3"); // Al entrar a la habitación final

    // 4. MURMULLOS Y SONIDOS DE LA ENTIDAD
    murmullos.Load("audio/murmullos.mp3");
    sonidoEntidad.Load("audio/sonido_entidad.mp3");
    sonidoEntidad.Loop(true);

    // 5. LOOPS DE MOVIMIENTO (Pasos)
    pasosJugadorBosque.Load("audio/pasos_bosque.mp3");
    pasosJugadorBosque.Loop(true);
	pasosJugadorBosque.SetVolume(400);

    pasosJugadorCabana.Load("audio/pasos_madera.mp3");
    pasosJugadorCabana.Loop(true);

    pasosEntidad.Load("audio/pasos_moustro.mp3");
    pasosEntidad.Loop(true);

    // 6. JUMPSCARES
    jsOso.Load("audio/susto_oso.mp3");
    jsEntidad.Load("audio/susto_entidad.mp3");
    jsCabana.Load("audio/susto_final.mp3");

    // 7. ATMÓSFERA CONSTANTE
    loopAmbiental.Load("audio/ambiente_bosque.mp3");
    loopAmbiental.Loop(true);
    loopAmbiental.SetVolume(300);

    loopLluvia.Load("audio/lluvia.mp3");
    loopLluvia.Loop(true);
	loopLluvia.SetVolume(400);

    musicaIntroTocadiscos.Load("audio/musica_distorcionada.mp3");
	musicaIntroTocadiscos.SetVolume(500);

    loopTocadiscos.Load("audio/musica_distorcionada_loop.mp3");
    loopTocadiscos.Loop(true);
	loopTocadiscos.SetVolume(500);

    musicaFinal.Load("audio/musica_final.mp3");
}