#pragma once
#include <glm/glm.hpp>
#include <string>
#include <iostream>

// 1. ABSTRACCIÓN: Creamos el concepto general de un "Objeto" que se puede recoger
class ObjetoInteractuable {
    // 2. ENCAPSULAMIENTO: Protegemos las variables para que no se modifiquen por accidente
protected:
    glm::vec3 posicion;
    bool activo; // Si es true, se dibuja en el mapa. Si es false, ya lo recogiste.
    std::string nombre;

public:
    ObjetoInteractuable(std::string _nombre, glm::vec3 _pos) {
        nombre = _nombre;
        posicion = _pos;
        activo = true;
    }

    // Métodos Get/Set obligatorios para el encapsulamiento
    glm::vec3 getPosicion() { return posicion; }
    void setPosicion(glm::vec3 _pos) { posicion = _pos; }
    bool isActivo() { return activo; }
    void setActivo(bool _activo) { activo = _activo; }
    std::string getNombre() { return nombre; }

    // 4. POLIMORFISMO: Este método hará algo distinto dependiendo si es Batería o Reliquia
    virtual void interactuar() {
        if (activo) {
            activo = false; // Al interactuar, desaparece
        }
    }
};

// 3. HERENCIA: La Batería es un "hijo" de ObjetoInteractuable
class BateriaRecargable : public ObjetoInteractuable {
private:
    float energiaAportada;
public:
    BateriaRecargable(std::string _nombre, glm::vec3 _pos, float _energia)
        : ObjetoInteractuable(_nombre, _pos) {
        energiaAportada = _energia;
    }

    // POLIMORFISMO en acción
    void interactuar() override {
        if (activo) {
            // Aquí luego le sumaremos a tu variable bateriaLinterna
            std::cout << "Recogiste: " << nombre << ". Energia recuperada!" << std::endl;
            activo = false;
        }
    }
};

// 3. HERENCIA: La Reliquia (como el tocadiscos u oso de peluche) es otro "hijo"
class Reliquia : public ObjetoInteractuable {
public:
    Reliquia(std::string _nombre, glm::vec3 _pos) : ObjetoInteractuable(_nombre, _pos) {}

    // POLIMORFISMO en acción
    void interactuar() override {
        if (activo) {
            std::cout << "Encontraste un objeto maldito: " << nombre << std::endl;
            // Aquí luego sumaremos al contador para que aparezca la cabaña
            activo = false;
        }
    }
};