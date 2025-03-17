/**
 * @brief Orbital simulation
 * @author Marc S. Ressl
 *
 * @copyright Copyright (c) 2022-2023
 */

// Enables M_PI #define in Windows
#define _USE_MATH_DEFINES

#include <iostream>
using namespace std;

#include <stdlib.h>
#include <math.h>

#include "OrbitalSim.h"
#include "ephemerides.h"

// Factor de conversión interno para números grandes
#define INTERNAL_SCALE 1.0E-10F  // Convertir metros a unidades internas
#define INTERNAL_TIME_SCALE 86400.0F  // Convertir segundos a días

// Ajustar la constante gravitacional para las unidades internas
#define INTERNAL_G (GRAVITATIONAL_CONSTANT * INTERNAL_TIME_SCALE * INTERNAL_TIME_SCALE / INTERNAL_SCALE)

#define GRAVITATIONAL_CONSTANT 6.6743E-11F
#define ASTEROIDS_MEAN_RADIUS 4E11F
#define ASTEROID_COUNT 50

/**
 * @brief Gets a uniform random value in a range
 *
 * @param min Minimum value
 * @param max Maximum value
 * @return The random value
 */
float getRandomFloat(float min, float max)
{
    return min + (max - min) * rand() / (float)RAND_MAX;
}

/**
 * @brief Configures an asteroid
 *
 * @param body An orbital body
 * @param centerMass The mass of the most massive object in the star system
 */
void configureAsteroid(OrbitalBody *body, float centerMass)
{
    // Logit distribution
    float x = getRandomFloat(0, 1);
    float l = logf(x) - logf(1 - x) + 1;

    // Reescalar el radio medio de los asteroides
    float scaledRadius = ASTEROIDS_MEAN_RADIUS * INTERNAL_SCALE;
    
    // Calcular posición
    float r = scaledRadius * sqrtf(fabsf(l));
    float phi = getRandomFloat(0, 2.0F * (float)M_PI);

    // Calcular velocidad orbital (ajustada para unidades internas)
    // v = sqrt(G*M/r) pero con G ajustado para nuestras unidades internas
    float v = sqrtf(INTERNAL_G * centerMass / r) * getRandomFloat(0.6F, 1.2F);
    float vy = getRandomFloat(-1E2F, 1E2F) * INTERNAL_SCALE * INTERNAL_TIME_SCALE;

    body->mass = 1E12F;  // masa en kg
    body->radius = 2E3F; // radio en metros
    body->color = GRAY;
    
    // Posición en unidades internas
    body->position = {r * cosf(phi), 0, r * sinf(phi)};
    
    // Velocidad en unidades internas/día
    body->velocity = {-v * sinf(phi), vy, v * cosf(phi)};
    
    // Aceleración inicial
    body->acceleration = {0.0f, 0.0f, 0.0f};
}

/**
 * @brief Constructs an orbital simulation
 *
 * @param timeStep The time step
 * @return The orbital simulation
 */OrbitalSim *constructOrbitalSim(float timeStep)
{
    // Crear una nueva instancia de OrbitalSim
    OrbitalSim *sim = new OrbitalSim;
    
    // Configurar el timeStep y el tiempo inicial
    // Convertir el timestep de segundos a días para la simulación interna
    sim->timeStep = timeStep / INTERNAL_TIME_SCALE;
    sim->elapsedTime = 0.0f;
    
    // Decidir qué sistema usar
    EphemeridesBody* selectedSystem = solarSystem;
    int systemBodyCount = SOLARSYSTEM_BODYNUM;
    
    // Configurar los cuerpos del sistema
    sim->bodyCount = systemBodyCount + ASTEROID_COUNT;
    sim->bodies = new OrbitalBody[sim->bodyCount];
    
    // Inicializar los cuerpos celestes del sistema
    for (int i = 0; i < systemBodyCount; i++) {
        // Reescalar las posiciones internamente
        sim->bodies[i].position.x = selectedSystem[i].position.x * INTERNAL_SCALE;
        sim->bodies[i].position.y = selectedSystem[i].position.y * INTERNAL_SCALE;
        sim->bodies[i].position.z = selectedSystem[i].position.z * INTERNAL_SCALE;
        
        // Reescalar las velocidades (m/s a unidades internas/día)
        sim->bodies[i].velocity.x = selectedSystem[i].velocity.x * INTERNAL_SCALE * INTERNAL_TIME_SCALE;
        sim->bodies[i].velocity.y = selectedSystem[i].velocity.y * INTERNAL_SCALE * INTERNAL_TIME_SCALE;
        sim->bodies[i].velocity.z = selectedSystem[i].velocity.z * INTERNAL_SCALE * INTERNAL_TIME_SCALE;
        
        // Inicializar aceleración
        sim->bodies[i].acceleration = {0.0f, 0.0f, 0.0f};
        
        // Copiar masa, radio y color sin cambios
        sim->bodies[i].mass = selectedSystem[i].mass;
        sim->bodies[i].radius = selectedSystem[i].radius;
        sim->bodies[i].color = selectedSystem[i].color;
        
        // ? Depuración: mostrar valores reescalados
        //cout << "Cuerpo " << i << " (" << selectedSystem[i].name << "):" << endl;
        //cout << "  Posición reescalada: (" << sim->bodies[i].position.x << ", " 
             //<< sim->bodies[i].position.y << ", " << sim->bodies[i].position.z << ")" << endl;
    }
    
    // Inicializar asteroides con las nuevas unidades internas
    for (int i = 0; i < ASTEROID_COUNT; i++) {
        int index = systemBodyCount + i;
        configureAsteroid(&sim->bodies[index], selectedSystem[0].mass);
    }
    
    return sim;
}

/**
 * @brief Destroys an orbital simulation
 * 
 * @param sim The orbital simulation to destroy
 */
void destroyOrbitalSim(OrbitalSim *sim)
{
    if (sim != NULL) {
        // Liberar la memoria del arreglo de cuerpos
        if (sim->bodies != NULL) {
            delete[] sim->bodies;
            sim->bodies = NULL;
        }
        
        // Liberar la memoria de la estructura principal
        delete sim;
    }
}

/**
 * @brief Simulates a timestep
 *
 * @param sim The orbital simulation
 */void updateOrbitalSim(OrbitalSim *sim)
{
    if (sim == NULL)
        return;
    
    // 1. Calcular las aceleraciones
    for (int i = 0; i < sim->bodyCount; i++) {
        // Reiniciar la aceleración
        sim->bodies[i].acceleration = {0.0f, 0.0f, 0.0f};
        
        // Calcular la fuerza gravitacional de todos los demás cuerpos
        for (int j = 0; j < sim->bodyCount; j++) {
            if (i == j)
                continue;
            
            // Vector de posición relativa
            Vector3 relativePos = Vector3Subtract(sim->bodies[j].position, sim->bodies[i].position);
            
            // Distancia entre los cuerpos
            float distance = Vector3Length(relativePos);
            
            // Evitar divisiones por cero o distancias muy pequeñas
            if (distance < 1e-4f)
                continue;
            
            // Dirección unitaria de la fuerza
            Vector3 direction = Vector3Normalize(relativePos);
            
            // Magnitud de la fuerza usando G ajustado para unidades internas
            float forceMagnitude = INTERNAL_G * sim->bodies[i].mass * sim->bodies[j].mass / 
                                  (distance * distance);
            
            // Aceleración
            float accelerationMagnitude = forceMagnitude / sim->bodies[i].mass;
            
            // Añadir a la aceleración total
            Vector3 acceleration = Vector3Scale(direction, accelerationMagnitude);
            sim->bodies[i].acceleration = Vector3Add(sim->bodies[i].acceleration, acceleration);
        }
        
        // Verificar si la aceleración es válida
        if (isnan(sim->bodies[i].acceleration.x) || isnan(sim->bodies[i].acceleration.y) || 
            isnan(sim->bodies[i].acceleration.z)) {

            // ! ACÁ HAY UN ERROR, SALTA LA ADVERTENCIA PARA TODOS LOS CUERPOS
            // cout << "¡Advertencia! Aceleración inválida para cuerpo " << i << endl;

            sim->bodies[i].acceleration = {0.0f, 0.0f, 0.0f};
        }
    }
    
    // ! OJO Creo que estas no son las ecuaciones que hay que usar
    // Primer medio paso: actualizar velocidades usando la aceleración actual
    for (int i = 0; i < sim->bodyCount; i++) {
        Vector3 halfDeltaV = Vector3Scale(sim->bodies[i].acceleration, sim->timeStep * 0.5f);
        sim->bodies[i].velocity = Vector3Add(sim->bodies[i].velocity, halfDeltaV);
    }
    
    // Actualizar posiciones con las velocidades de medio paso
    for (int i = 0; i < sim->bodyCount; i++) {
        Vector3 deltaX = Vector3Scale(sim->bodies[i].velocity, sim->timeStep);
        sim->bodies[i].position = Vector3Add(sim->bodies[i].position, deltaX);
        
        // Verificar si la posición es válida
        if (isnan(sim->bodies[i].position.x) || isnan(sim->bodies[i].position.y) || 
            isnan(sim->bodies[i].position.z)) {
            cout << "¡Advertencia! Posición inválida para cuerpo " << i << endl;
            if (i == 0) {
                sim->bodies[i].position = {0.0f, 0.0f, 0.0f}; // Origen para el Sol
            } else {
                // Posición segura relativa al Sol
                sim->bodies[i].position = Vector3Add(sim->bodies[0].position, 
                                                    (Vector3){(float)i * 0.1f, 0.0f, 0.0f});
            }
        }
    }
    
    // Recalcular aceleraciones con las nuevas posiciones
    // TODO Repetir el código de arriba para calcular aceleraciones
    
    // Segundo medio paso: completar la actualización de velocidades
    for (int i = 0; i < sim->bodyCount; i++) {
        Vector3 halfDeltaV = Vector3Scale(sim->bodies[i].acceleration, sim->timeStep * 0.5f);
        sim->bodies[i].velocity = Vector3Add(sim->bodies[i].velocity, halfDeltaV);
        
        // Verificar si la velocidad es válida
        if (isnan(sim->bodies[i].velocity.x) || isnan(sim->bodies[i].velocity.y) || 
            isnan(sim->bodies[i].velocity.z)) {
            cout << "¡Advertencia! Velocidad inválida para cuerpo " << i << endl;
            sim->bodies[i].velocity = {0.0f, 0.0f, 0.0f};
        }
    }
    
    // Actualizar el tiempo total (en segundos para interfaz externa)
    sim->elapsedTime += sim->timeStep * INTERNAL_TIME_SCALE;
}