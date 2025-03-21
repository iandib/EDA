#define ORBITALSIM_H/**
/* * @brief Orbital simulation
 * @author Marc S. Ressl
 *
 * @copyright Copyright (c) 2022-2023
 */

#ifndef ORBITALSIM_H



#include "raylib.h"
#include "raymath.h"
/**
 * @brief Orbital body definition
 */
struct OrbitalBody
{
    Vector3 position;
    Vector3 speed;
    Vector3 acceleration;
    float mass;
    float radius;
    Color color;

};

/**
 * @brief Orbital simulation definition
 */
struct OrbitalSim
{
    // Fill in your code here...
    float timeStep;
    float bodyCounter;
    OrbitalBody (*body)[9];

};

OrbitalSim *constructOrbitalSim(float timeStep);
void destroyOrbitalSim(OrbitalSim *sim);

void updateOrbitalSim(OrbitalSim *sim);

#endif



