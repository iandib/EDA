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

    #define GRAVITATIONAL_CONSTANT (6.6743E-11L)
    #define ASTEROIDS_MEAN_RADIUS 4E11F


/* *****************************************************************
    * LOGIC MODULES *
   ***************************************************************** */

    //* ASTEROID CONFIGURATION

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
    /// @cite https://academia-lab.com/enciclopedia/cinturon-de-asteroides/
    void configureAsteroid(OrbitalBody *body, float centerMass) 
    {
        // Logit distribution
        float x = getRandomFloat(0, 1);
        float l = logf(x) - logf(1 - x) + 1;

        // Generate a random number to determine the region of the asteroid
        float regionSelector = getRandomFloat(0, 1);

        // Define radius based on region
        float r;

        // Supposing 70% of asteroids are between Mars and Jupiter
        if (regionSelector < 0.7)
        {
            // Radius of (Distance to Mars; Distance to Jupiter)
            r = getRandomFloat(2.28E11F, 7.79E11F);
            body->color = GRAY;
        }

        // Supposing 20% of asteroids are around Jupiter
        else if (regionSelector >= 0.7 && regionSelector < 0.9)
        {
            // Radius of ±20% from Jupiter's orbit
            float jupiterDistance = 7.79E11F;        
            r = jupiterDistance * getRandomFloat(0.8F, 1.2F);
            body->color = DARKGRAY;
        }

        // Spawining the remaining 10% of asteroids in any region
        else
        {
            // Original logit distribution for radius
            r = ASTEROIDS_MEAN_RADIUS * sqrtf(fabsf(l));
            body->color = LIGHTGRAY;
        }

        /// @cite https://mathworld.wolfram.com/DiskPointPicking.html
        float phi = getRandomFloat(0, 2.0F * (float)M_PI);

        /// @cite https://en.wikipedia.org/wiki/Circular_orbit#Velocity
        float v = sqrtf(GRAVITATIONAL_CONSTANT * centerMass / r) * getRandomFloat(0.6F, 1.2F);
        float vy = getRandomFloat(-1E2F, 1E2F);

        body->mass = 1E12F;
        body->radius = 2E3F;
        body->position = {r * cosf(phi), vy, r * sinf(phi)};
        body->velocity = {-v * sinf(phi), 0, v * cosf(phi)};
    }


    //* GRAVITATIONAL FORCE AND ACCELERATION CALCULATION

    /// @brief Calculates the gravitational force between two bodies
    /// @param pos1 Position of first body
    /// @param mass1 Mass of first body
    /// @param pos2 Position of second body
    /// @param mass2 Mass of second body
    /// @return Gravitational force vector
    Vector3 calculateGravitationalForce(Vector3 pos1, float mass1, Vector3 pos2, float mass2) 
    {
        // Calculate direction vector from pos1 to pos2
        Vector3 direction = Vector3Subtract(pos2, pos1);
        
        // Calculate distance
        float distance = Vector3Length(direction);
        
        // Avoid division by zero
        if (distance < 1.0f)
        {
            return {0, 0, 0};
        }
        
        Vector3 unitDirection = Vector3Normalize(direction);
        
        // Calculate force magnitude using Newton's law of gravitation
        // F = G * m1 * m2 / r^2
        float forceMagnitude = GRAVITATIONAL_CONSTANT * mass1 * mass2 / (distance * distance);
        
        // Return force vector with negative sign to make it attractive
        auto ret = Vector3Scale(unitDirection, forceMagnitude);

        return ret;
    }


    /// @brief Calculates the acceleration of a group of bodies due to the gravitational force from another group
    /// @param sim The orbital simulation
    /// @param accelerations Array to store the resulting accelerations for each body
    /// @param sourceStartIndex Starting index of the group of bodies whose accelerations will be calculated
    /// @param sourceEndIndex Ending index (exclusive) of the source bodies group
    /// @param targetStartIndex Starting index of the group of bodies that influence the source bodies' accelerations
    /// @param targetEndIndex Ending index (exclusive) of the target bodies group
    void calculateAccelerations(OrbitalSim* sim, Vector3* accelerations, int startIndex, int endIndex, 
                                int targetStartIndex, int targetEndIndex)
    {
        for (int i = startIndex; i < endIndex; i++)
        {
            for (int j = targetStartIndex; j < targetEndIndex; j++)
            { 
                Vector3 force = calculateGravitationalForce(sim->bodies[i].position,
                                                            sim->bodies[i].mass, 
                                                            sim->bodies[j].position,
                                                            sim->bodies[j].mass);    

                // Calculate accelerations using Newton's second law, a = F / m
                accelerations[i] = Vector3Add(accelerations[i],
                                    Vector3Scale(force, 1.0f / sim->bodies[i].mass));
            }
        }

        return;
    }

    
    //* ORBITAL SIMULATION MANAGEMENT

    /// @brief Constructs an orbital simulation
    /// @param timeStep Time step for integration
    /// @return The orbital simulation
    OrbitalSim* constructOrbitalSim(float timeStep)
    {
        // Allocate memory for the simulation structure
        OrbitalSim* sim = new OrbitalSim;

        // Initialize fields
        sim->timeStep = timeStep;
        sim->time = 0.0f;

        // Total number of bodies in the simulation
        sim->bodyCount = SOLARSYSTEM_BODYNUM * SOLAR_SYSTEM + ALPHACENTAURISYSTEM_BODYNUM * ALPHA_CENTAURI 
                        + NUM_ASTEROIDS + BLACKHOLE;

        int TotalBodyNum = sim->bodyCount - 1;

        // Allocate memory for the bodies
        sim->bodies = new OrbitalBody[sim->bodyCount];

        // Copy solar system bodies from ephemerides
        if (SOLAR_SYSTEM)
        {
            for (int i = 0; i < SOLARSYSTEM_BODYNUM; i++)
            {
                sim->bodies[TotalBodyNum].velocity = solarSystem[i].velocity;
                sim->bodies[TotalBodyNum].position = solarSystem[i].position;
                sim->bodies[TotalBodyNum].color = solarSystem[i].color;

                if (MASIVE_JUPITER && (i == 5))
                {
                    sim->bodies[TotalBodyNum].mass = ((solarSystem[i].mass) * 1000.0);
                }

                else
                {
                    sim->bodies[TotalBodyNum].mass = solarSystem[i].mass;
                }

                sim->bodies[TotalBodyNum].radius = solarSystem[i].radius;
                sim->bodies[TotalBodyNum].name = solarSystem[i].name;
                TotalBodyNum--;
            }
        }

        // Copy Alpha Centauri system bodies from ephemerides
        if (ALPHA_CENTAURI)
        {
            for (int j = 0; j < ALPHACENTAURISYSTEM_BODYNUM; j++)
            {
                sim->bodies[TotalBodyNum].velocity = alphaCentauriSystem[j].velocity;
                sim->bodies[TotalBodyNum].position = alphaCentauriSystem[j].position;
                sim->bodies[TotalBodyNum].color = alphaCentauriSystem[j].color;
                sim->bodies[TotalBodyNum].mass = alphaCentauriSystem[j].mass;
                sim->bodies[TotalBodyNum].radius = alphaCentauriSystem[j].radius;
                sim->bodies[TotalBodyNum].name = alphaCentauriSystem[j].name;
                TotalBodyNum--;
            }
        }

        // Intermediate mass black hole setup
        /// @cite https://en.wikipedia.org/wiki/Intermediate-mass_black_hole
        if (BLACKHOLE)
        {
            sim->bodies[TotalBodyNum].name = "Black Hole";
            sim->bodies[TotalBodyNum].mass = ((solarSystem[0].mass) * 100.0); // [kg]
            sim->bodies[TotalBodyNum].radius = 2E20F; // [m]
            sim->bodies[TotalBodyNum].color = DARKPURPLE;
            sim->bodies[TotalBodyNum].position = { 4.431790029686977E+12F, -8.954348456482631E+10F, 0 };
            sim->bodies[TotalBodyNum].velocity = { -9.431790029686977E+4F, 8.954348456482631E+1F, 6.114486878028781E+1F };
            TotalBodyNum--;
        }

        // Sun's mass
        float centerMass = solarSystem[0].mass;

        // Asteroids setup
        for (int i = 0; i < NUM_ASTEROIDS; i++)
        {
            OrbitalBody* body = &sim->bodies[TotalBodyNum];
            body->name = "Asteroid";
            configureAsteroid(body, centerMass);
            TotalBodyNum--;
        }

            return sim;
        }
 

    /// @brief Simulates a timestep
    /// @param sim The orbital simulation
    void updateOrbitalSim(OrbitalSim *sim)
    {
        // Temporary array to store accelerations
        Vector3 *accelerations = new Vector3[sim->bodyCount]();
        
        // Calculate accelerations due to the gravitational force between significant bodies
        calculateAccelerations(sim, accelerations, 
                                NUM_ASTEROIDS, sim->bodyCount, 
                                NUM_ASTEROIDS, sim->bodyCount);
        
        // Calculate accelerations due to the gravitational force between asteroids and significant bodies
        calculateAccelerations(sim, accelerations, 
                                0, NUM_ASTEROIDS - 1,
                                NUM_ASTEROIDS, sim->bodyCount);
        
        // Update velocities and positions using the corresponding current acceleration
        for (int i = 0; i < sim->bodyCount; i++)
        {
            // Store the previous position before updating
            sim->bodies[i].previousPosition = sim->bodies[i].position;
            
            // v(n+1) = v(n) + a(n) * dt
            Vector3 velocityChange = Vector3Scale(accelerations[i], sim->timeStep);
            sim->bodies[i].velocity = Vector3Add(sim->bodies[i].velocity, velocityChange);
            
            // x(n+1) = x(n) + v(n+1) * dt
            Vector3 positionChange = Vector3Scale(sim->bodies[i].velocity, sim->timeStep);
            sim->bodies[i].position = Vector3Add(sim->bodies[i].position, positionChange);
        }

        // Clean up
        delete[] accelerations;
        
        // Update simulation time
        sim->time += sim->timeStep;
    }


    /// @brief Destroys an orbital simulation
    /// @param sim The orbital simulation
    void destroyOrbitalSim(OrbitalSim *sim)
    {
        delete[] sim->bodies;
        delete sim;
    }
