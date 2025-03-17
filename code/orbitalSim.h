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
     Vector3 position;    // Posición actual del cuerpo
     Vector3 velocity;    // Velocidad actual del cuerpo
     Vector3 acceleration; // Aceleración calculada en cada paso
     float mass;          // Masa del cuerpo
     float radius;        // Radio visual del cuerpo
     Color color;         // Color para la representación gráfica
 };
 
 /**
  * @brief Orbital simulation definition
  */
 struct OrbitalSim
 {
     float timeStep;      // Intervalo de tiempo para cada paso de simulación
     float elapsedTime;   // Tiempo total transcurrido desde el inicio
     int bodyCount;       // Número total de cuerpos en la simulación
     OrbitalBody* bodies; // Arreglo dinámico de cuerpos orbitales
 };
 
 
 OrbitalSim *constructOrbitalSim(float timeStep);
 void destroyOrbitalSim(OrbitalSim *sim);
 
 void updateOrbitalSim(OrbitalSim *sim);
 
 #endif