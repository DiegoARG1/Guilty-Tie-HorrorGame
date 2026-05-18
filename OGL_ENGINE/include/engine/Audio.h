#ifndef Audio_H
#define Audio_H
#include <Windows.h>
#include <mmsystem.h>
#include <iostream>
#include <string>

#pragma comment(lib,"winmm.lib")

using namespace std;

class Audio
{
private:
    string alias;
    string ruta;
    bool isLoop;

    // Generamos un nombre interno único para que Windows pueda reproducir
    // múltiples audios simultáneamente sin que se pisen entre ellos.
    string generarAlias() {
        static int contador = 0;
        return "pista_audio_" + to_string(contador++);
    }

public:
    Audio() {
        alias = generarAlias();
        isLoop = false;
    }

    // :::: METODO PARA CARGAR EL ARCHIVO ::::
    void Load(string path) {
        ruta = path;

        string cmdClose = "close " + alias;
        mciSendStringA(cmdClose.c_str(), NULL, 0, NULL);

        // CORRECCIÓN: Le quitamos el 'type waveaudio' para que Windows 
        // detecte automáticamente que son .mp3 y los reproduzca sin problemas.
        string cmdOpen = "open \"" + ruta + "\" alias " + alias;
        mciSendStringA(cmdOpen.c_str(), NULL, 0, NULL);
    }

    // :::: METODO PARA ACTIVAR EL BUCLE (Lluvia, Viento, Pasos) ::::
    void Loop(bool loop) {
        isLoop = loop;
    }

    // :::: METODO PARA REPRODUCIR (Permite audios simultáneos) ::::
    void Play() {
        // Regresamos el audio al segundo 0 por si ya se había reproducido antes (Ej. clicks de linterna)
        string cmdSeek = "seek " + alias + " to start";
        mciSendStringA(cmdSeek.c_str(), NULL, 0, NULL);

        string cmdPlay = "play " + alias;
        if (isLoop) {
            cmdPlay += " repeat";
        }
        mciSendStringA(cmdPlay.c_str(), NULL, 0, NULL);
    }

    // :::: METODO PARA DETENER ::::
    void Stop() {
        string cmdStop = "stop " + alias;
        mciSendStringA(cmdStop.c_str(), NULL, 0, NULL);
    }
    // :::: METODO PARA CONTROLAR EL VOLUMEN ::::
    // La escala de Windows va de 0 (Silencio) a 1000 (Máximo)
    void SetVolume(int volumen) {
        // 1. Protegemos el código para que no le pases valores inválidos
        if (volumen < 0) volumen = 0;
        if (volumen > 1000) volumen = 1000;

        // 2. Le enviamos la orden exacta a Windows
        string cmdVol = "setaudio " + alias + " volume to " + to_string(volumen);
        mciSendStringA(cmdVol.c_str(), NULL, 0, NULL);
    }
};

#endif