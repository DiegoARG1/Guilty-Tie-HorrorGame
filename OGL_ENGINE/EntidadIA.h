#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <engine/Terrain.h>
#include <cstdlib>
#include <cmath> 

class EntidadIA {
private:
    glm::vec3 posicion;
    float velocidad;
    bool alertaVisible;
    float timerAnimacion;
    int frameActual;

    bool oculto;
    float timerOculto;
    float tiempoParaAparecer;

    // :::: NUEVO: CRONÓMETRO DE LUZ ::::
    float timerIluminado;

public:
    EntidadIA(glm::vec3 posInicial) {
        oculto = true;
        timerOculto = 0.0f;
        tiempoParaAparecer = 5.0f + (rand() % 10);
        posicion = glm::vec3(1000.0f, -1000.0f, 1000.0f);
        velocidad = 5.0f;
        alertaVisible = false;
        timerAnimacion = 0.0f;
        frameActual = 0;

        timerIluminado = 0.0f; // Inicializamos a 0
    }

    void actualizar(float dt, glm::vec3 posJugador, glm::vec3 camFront, bool linternaOn, Terrain* terreno) {

        if (oculto) {
            timerOculto += dt;
            alertaVisible = false;
            if (timerOculto >= tiempoParaAparecer) {
                oculto = false;
                teletransportar(posJugador, terreno);
            }
            return;
        }

        float distancia = glm::distance(posicion, posJugador);
        glm::vec3 direccionDesdeJugador = glm::normalize(posicion - posJugador);

        float alineacionVista = glm::dot(camFront, direccionDesdeJugador);
        bool estaIluminado = (linternaOn && alineacionVista > 0.85f);

        // :::: NUEVO: SISTEMA ANTI-GIROS RÁPIDOS ::::
        if (estaIluminado) {
            timerIluminado += dt; // Suma tiempo si lo miras
        }
        else {
            // Si te pones nervioso y apartas la luz, el progreso se pierde rápido
            if (timerIluminado > 0.0f) timerIluminado -= dt * 2.0f;
        }

        // LÓGICA DE SUPERVIVENCIA ACTUALIZADA
        // Ahora requiere iluminarlo continuamente por 1.2 segundos para que huya
        if (distancia < 10.0f && timerIluminado > 1.2f) {
            oculto = true;
            timerOculto = 0.0f;
            tiempoParaAparecer = 10.0f + (rand() % 15);
            posicion = glm::vec3(1000.0f, -1000.0f, 1000.0f);
            alertaVisible = false;
            timerIluminado = 0.0f; // Reiniciamos el cronómetro de luz
            return;
        }

        alertaVisible = true;
        glm::vec3 direccionHaciaJugador = glm::normalize(posJugador - posicion);
        direccionHaciaJugador.y = 0.0f;
        posicion += direccionHaciaJugador * velocidad * dt;

        timerAnimacion += dt;
        if (timerAnimacion > 0.15f) {
            timerAnimacion = 0.0f;
            frameActual++;
            if (frameActual > 3) frameActual = 0;
        }

        float alturaSuelo = (terreno->Superficie(posicion.x, posicion.z) * 300.0f) - 2.5f;
        posicion.y = alturaSuelo;
    }

    void teletransportar(glm::vec3 posJugador, Terrain* terreno) {
        float anguloAleatorio = (rand() % 360) * (3.14159f / 180.0f);
        float distanciaSegura = 30.0f;

        float nuevoX = posJugador.x + (cos(anguloAleatorio) * distanciaSegura);
        float nuevoZ = posJugador.z + (sin(anguloAleatorio) * distanciaSegura);

        if (nuevoX > 48.0f) nuevoX = 48.0f;
        if (nuevoX < -48.0f) nuevoX = -48.0f;
        if (nuevoZ > 48.0f) nuevoZ = 48.0f;
        if (nuevoZ < -48.0f) nuevoZ = -48.0f;

        posicion = glm::vec3(nuevoX, 0.0f, nuevoZ);
        float alturaSuelo = (terreno->Superficie(posicion.x, posicion.z) * 300.0f) - 2.5f;
        posicion.y = alturaSuelo;
    }

    glm::vec3 getPosicion() { return posicion; }
    bool mostrarAlerta() { return alertaVisible; }
    int getFrameAnimacion() { return frameActual; }
    bool estaOculto() { return oculto; }
};