/* *****************************************************************
    * FILE INFORMATION *
   ***************************************************************** */
   
/// @brief Orbital simulation
/// @author Marc S. Ressl, Ian A. Dib, Luciano S. Cordero
/// @copyright Copyright (c) 2022-2023


/* *****************************************************************
    * HEADER CONFIGURATION *
   ***************************************************************** */

   #ifndef ORBITALSIM_H
   #define ORBITALSIM_H

   //* NECESSARY LIBRARIES
   #include <raylib.h>
   #include <raymath.h>

    //* CONFIGURATION

    #define SOLAR_SYSTEM 1
    #define ALPHA_CENTAURI 0
    #define BLACKHOLE 0
    #define MASIVE_JUPITER 0
    #define NUM_ASTEROIDS 100


    //* CONSTANTS & STRUCTURES
   
    /// @brief Orbital body definition
    struct OrbitalBody
    {
        const char *name;
        float mass;                 // [kg]
        float radius;               // [m]
        Color color;                // Raylib color
        Vector3 position;           // [m]
        Vector3 previousPosition;   // [m]
        Vector3 velocity;           // [m/s]
    };


    /// @brief Orbital simulation definition
    struct OrbitalSim
    {
        float timeStep;     // [s]
        float time;         // Total elapsed time [s]
        int bodyCount;
        OrbitalBody* bodies;
    };


    //* PUBLIC FUNCTIONS PROTOTYPES

    OrbitalSim *constructOrbitalSim(float timeStep);
    void destroyOrbitalSim(OrbitalSim *sim);
    void updateOrbitalSim(OrbitalSim *sim);


    #endif // ORBITALSIM_H





