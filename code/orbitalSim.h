/**
 * @brief Orbital simulation
 * @author Marc S. Ressl
 *
 * @copyright Copyright (c) 2022-2023
 */

#ifndef ORBITALSIM_H
#define ORBITALSIM_H

#include "raylib.h"
#include "raymath.h"

/**
 * @brief Orbital body definition
 */
struct OrbitalBody
{
    // * Todo esto ya está en ephemerides.h pero (entiendo) que no se usan en el código, es solo una base
    float mass;		  // [kg]
    float radius;	  // [m]
    Color color;	  // Raylib color
    Vector3 position; // [m]
    Vector3 velocity; // [m/s]
};

/**
 * @brief Orbital simulation definition
 */
struct OrbitalSim
{
    float timeStep;
    int cronometer;
    int numBodies;
    OrbitalBody *bodies;
};


OrbitalSim *constructOrbitalSim(float timeStep);
void destroyOrbitalSim(OrbitalSim *sim);

void updateOrbitalSim(OrbitalSim *sim);

#endif
