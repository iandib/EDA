/**
 * @brief Orbital simulation
 * @author Marc S. Ressl
 *
 * @copyright Copyright (c) 2022-2023
 */

// OrbitalSim.h
#ifndef ORBITALSIM_H
#define ORBITALSIM_H

#include "raylib.h"
#include "raymath.h"

// Estructura para representar un cuerpo orbital
typedef struct {
    const char *name;
    float mass;       // [kg]
    float radius;     // [m]
    Color color;      // Color en Raylib
    Vector3 position; // [m]
    Vector3 velocity; // [m/s]
} OrbitalBody;

// Estructura para representar nodo del árbol octante (Barnes-Hut)
typedef struct OctreeNode {
    Vector3 centerOfMass;  // Centro de masa del nodo
    float totalMass;       // Masa total del nodo
    Vector3 position;      // Posición central del nodo
    float size;            // Tamaño del nodo
    struct OctreeNode* children[8]; // 8 octantes
    OrbitalBody* body;     // Referencia al cuerpo (solo para nodos hoja)
    bool isLeaf;           // Indica si es un nodo hoja
} OctreeNode;

// Estructura para representar la simulación
typedef struct {
    float timeStep;        // Paso temporal [s]
    float time;            // Tiempo transcurrido [s]
    int bodyCount;         // Número de cuerpos
    OrbitalBody* bodies;   // Arreglo de cuerpos
    float theta;           // Parámetro de precisión para Barnes-Hut (típicamente 0.5)
} OrbitalSim;

// Funciones principales
OrbitalSim* constructOrbitalSim(float timeStep);
void destroyOrbitalSim(OrbitalSim* sim);
void updateOrbitalSim(OrbitalSim* sim);

// Configura un asteroide (implementada en el archivo fuente)
void configureAsteroid(OrbitalBody* body, float centerMass);

#endif // ORBITALSIM_H