#ifndef Audio_H
#define Audio_H
#include <Windows.h>
#include <mmsystem.h>
#include <iostream>
#include <string>


#pragma comment(lib,"winmm.lib")

using namespace std;

class Cancion {

public:
    LPCWSTR Nombre;
    LPCWSTR Dir;
};

enum ListaMusica {
    Pista1,
    Pista2,
    Pista3
};

class Audio
{
public:
    Cancion lista_sonidos[3];
    char* name;
    // Constructor with vectors
    Audio()
    {
        lista_sonidos[0].Dir = L"audio//hq-explosion-6288.wav";

        lista_sonidos[1].Dir = L"audio//rain-and-thunder-16705";

        lista_sonidos[2].Dir = L"";

        
    }

    Audio(string dir1, string dir2, string dir3)
    {
        lista_sonidos[0].Dir = (LPCWSTR)dir1.c_str();

        lista_sonidos[1].Dir = (LPCWSTR)dir2.c_str();

        lista_sonidos[2].Dir = (LPCWSTR)dir3.c_str();


    }
   
public:
    void ReproducirAudio(ListaMusica pista) {
        
        switch (pista) {
        case Pista1:
            PlaySound(lista_sonidos[0].Dir, NULL, SND_ASYNC);
            break;
        case Pista2:
            PlaySound(lista_sonidos[1].Dir, NULL, SND_ASYNC|SND_LOOP);
            break;
        case Pista3:
            PlaySound(lista_sonidos[2].Dir, NULL, SND_ASYNC);
            break;
        }
       
    }

    void StopAudio( ) {
       
            PlaySound(0, 0, 0);
    }

    void operator()(ListaMusica pista) {
        ReproducirAudio(pista);
    }
   
};

#endif