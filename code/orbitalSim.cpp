/* *****************************************************************
    * FILE INFORMATION *
   ***************************************************************** */
   
/// @brief Orbital simulation
/// @author Marc S. Ressl, Ian A. Dib, Luciano S. Cordero
/// @copyright Copyright (c) 2022-2023


/* *****************************************************************
    * FILE CONFIGURATION *
   ***************************************************************** */

    //* NECESSARY LIBRARIES

    #include <stdlib.h>
    #include <math.h>
   
   
    //* NECESSARY HEADERS

    #include "OrbitalSim.h"
    #include "ephemerides.h"

    
    //* CONSTANTS

    #define SECONDS_PER_DAY 86400
    
    // Enables M_PI #define in Windows
    #define _USE_MATH_DEFINES

    #define GRAVITATIONAL_CONSTANT 6.6743E-11F
    #define ASTEROIDS_MEAN_RADIUS 4E11F


/* *****************************************************************
    * LOGIC MODULES *
   ***************************************************************** */

    //* GENERAL FUNCTIONALITY MODULES

    /// @brief Gets a uniform random value in a range
    /// @param min Minimum value
    /// @param max Maximum value
    /// @return The random value
    float getRandomFloat(float min, float max)
    {
        return min + (max - min) * rand() / (float)RAND_MAX;
    }


    /// @brief Configures an asteroid
    /// @param body An orbital body
    /// @param centerMass The mass of the most massive object in the star system
    void configureAsteroid(OrbitalBody *body, float centerMass)
    {
        // Logit distribution
        float x = getRandomFloat(0, 1);
        float l = logf(x) - logf(1 - x) + 1;

        /// @cite https://mathworld.wolfram.com/DiskPointPicking.html
        float r = ASTEROIDS_MEAN_RADIUS * sqrtf(fabsf(l));
        float phi = getRandomFloat(0, 2.0F * (float)M_PI);

        //! The Easter Egg is that phi is set to 0, so all asteroids are in the same plane
        // phi = 0;

        /// @cite https://en.wikipedia.org/wiki/Circular_orbit#Velocity
        float v = sqrtf(GRAVITATIONAL_CONSTANT * centerMass / r) * getRandomFloat(0.6F, 1.2F);
        float vy = getRandomFloat(-1E2F, 1E2F);

        // Fill in with your own fields:
        // body->mass = 1E12F;  // Typical asteroid weight: 1 billion tons
        // body->radius = 2E3F; // Typical asteroid radius: 2km
        // body->color = GRAY;
        // body->position = {r * cosf(phi), 0, r * sinf(phi)};
        // body->velocity = {-v * sinf(phi), vy, v * cosf(phi)};
    }


    //* ORBITAL SIMULATION MANAGEMENT MODULES

    /// @brief Constructs an orbital simulation
    /// @param timeStep 
    /// @return The orbital simulation
    OrbitalSim *constructOrbitalSim(float timeStep)
    {
        // TODO complete this function



        return NULL; // This should return the orbital sim
    }


    /// @brief Destroys an orbital simulation
    /// @param sim The orbital simulation
    void destroyOrbitalSim(OrbitalSim *sim)
    {
        // TODO complete this function


    }


    /// @brief Simulates a timestep
    /// @param sim The orbital simulation
    void updateOrbitalSim(OrbitalSim *sim)
    {
        // TODO complete this function


    }
