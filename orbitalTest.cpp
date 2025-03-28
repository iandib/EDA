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
      #define M_PI 3.14159265358979323846
      #define YES 1
      #define NO 0
      
      
      
      
      //* CONFIGURATION (YES/NO)
      #define SOLAR_SYSTEM YES
      #define ALPHA_CENTAURI NO
      #define BLACKHOLE NO
      #define MASIVE_JUPITER YES
      #define NUM_ASTEROIDS 0
      
      
      
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
      void configureAsteroid(OrbitalBody* body, float centerMass)
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
      
          // Fill in with the fields:
          body->mass = 1E12F;  // Typical asteroid weight: 1 billion tons
          body->radius = 2E3F; // Typical asteroid radius: 2km
          body->color = GRAY;
          body->position = { r * cosf(phi), 0, r * sinf(phi) };
          body->velocity = { -v * sinf(phi), vy, v * cosf(phi) };
      }
      
      
      //* GRAVITATIONAL FORCE CALCULATION MODULES
      
      /// @brief Calculates the gravitational force between two bodies
      /// @param pos1 Position of first body
      /// @param mass1 Mass of first body
      /// @param pos2 Position of second body
      /// @param mass2 Mass of second body
      /// @return Gravitational force vector
      //! EL ERROR EN EL CÁLCULO DE LA FUERZA DE GRAVEDAD DEBE ESTAR ACÁ
      Vector3 calculateGravitationalForce(Vector3 pos1, float mass1, Vector3 pos2, float mass2)
      {
          // Calculate direction vector from pos1 to pos2
          Vector3 direction = Vector3Subtract(pos2, pos1);
      
          // Calculate distance
          float distance = Vector3Length(direction);
      
          // Avoid division by zero
          if (distance < 1.0f)
          {
              return { 0, 0, 0 };
          }
      
          Vector3 unitDirection = Vector3Normalize(direction);
      
          // Calculate force magnitude using Newton's law of gravitation
          // F = G * m1 * m2 / r^2
          float forceMagnitude = GRAVITATIONAL_CONSTANT * mass1 * mass2 / (distance * distance);
      
          // Return force vector with negative sign to make it attractive
          auto ret = Vector3Scale(unitDirection, forceMagnitude);
      
          return ret;
      }
      
      
      //* ORBITAL SIMULATION MANAGEMENT MODULES
      
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
      
          // Define the number of bodies
          sim->bodyCount = SOLARSYSTEM_BODYNUM * SOLAR_SYSTEM + ALPHACENTAURISYSTEM_BODYNUM * ALPHA_CENTAURI + NUM_ASTEROIDS + BLACKHOLE;
      
          int TotalBodyNum = sim->bodyCount - 1;
      
          // Allocate memory for the bodies
          sim->bodies = new OrbitalBody[sim->bodyCount];
      
          if (SOLAR_SYSTEM) {
              // Copy solar system bodies from ephemerides
              for (int i = 0; i < SOLARSYSTEM_BODYNUM; i++, TotalBodyNum--)
              {
                  sim->bodies[TotalBodyNum].velocity = solarSystem[i].velocity;
                  sim->bodies[TotalBodyNum].position = solarSystem[i].position;
                  sim->bodies[TotalBodyNum].color = solarSystem[i].color;
                  if (MASIVE_JUPITER && (i == 5)) {
                      sim->bodies[TotalBodyNum].mass = ((solarSystem[i].mass) * 1000.0); // Massive Jupiter
                  }
                  else {
                      sim->bodies[TotalBodyNum].mass = solarSystem[i].mass;
                  }
                  sim->bodies[TotalBodyNum].radius = solarSystem[i].radius;
                  sim->bodies[TotalBodyNum].name = solarSystem[i].name;
              }
          }
          // Copy Alpha Centauri system bodies from ephemerides
          if (ALPHA_CENTAURI) {
              for (int j = 0; j < ALPHACENTAURISYSTEM_BODYNUM; j++, TotalBodyNum--) {
      
                  sim->bodies[TotalBodyNum].velocity = alphaCentauriSystem[j].velocity;
      
                  sim->bodies[TotalBodyNum].position = alphaCentauriSystem[j].position;
      
                  sim->bodies[TotalBodyNum].color = alphaCentauriSystem[j].color;
      
                  sim->bodies[TotalBodyNum].mass = alphaCentauriSystem[j].mass;
      
                  sim->bodies[TotalBodyNum].radius = alphaCentauriSystem[j].radius;
      
                  sim->bodies[TotalBodyNum].name = alphaCentauriSystem[j].name;
      
              }
          }
      
          // Configure asteroids
          float centerMass = solarSystem[0].mass; // Sun's mass
      
          if (NUM_ASTEROIDS) {
              for (int i = 0; i < NUM_ASTEROIDS; i++, TotalBodyNum--)
              {
                  OrbitalBody* body = &sim->bodies[TotalBodyNum];
                  body->name = "Asteroid";
                  configureAsteroid(body, centerMass);
              }
          }
          // Black Hole configuration
          if (BLACKHOLE) {
      
              sim->bodies[TotalBodyNum].name = "Black Hole";
      
              sim->bodies[TotalBodyNum].mass = ((solarSystem[0].mass) * 100.0); // stellar black hole (*100 mass)
      
              sim->bodies[TotalBodyNum].radius = 2E20F;
      
              sim->bodies[TotalBodyNum].color = DARKPURPLE;
      
              sim->bodies[TotalBodyNum].position = { 4.431790029686977E+12F, -8.954348456482631E+10F, 0 };
      
              sim->bodies[TotalBodyNum].velocity = { -9.431790029686977E+4F, 8.954348456482631E+1F, 6.114486878028781E+1F };
          }
      
          return sim;
      }
      
      
      /// @brief Destroys an orbital simulation
      /// @param sim The orbital simulation
      void destroyOrbitalSim(OrbitalSim* sim)
      {
          delete[] sim->bodies;
          delete sim;
      }
      
      
      /// @brief Simulates a timestep
      /// @param sim The orbital simulation
      void updateOrbitalSim(OrbitalSim* sim)
      {
          //* VELOCITY AND ACCELERATION CALCULATION
      
          // Temporary array to store accelerations
          Vector3* accelerations = new Vector3[sim->bodyCount];
      
          //! ACÁ HAY UN ERROR, CON i=0 NO SE SIMULA NADA PERO CON i=7 LA GRAVEDAD NO SE CALCULA BIEN
          // Calculate accelerations for all bodies
      
      
          for (int i = 0; i < sim->bodyCount; i++)
          {
              Vector3 netForce = { 0, 0, 0 };
      
              for (int j = 0; j < sim->bodyCount; j++)
              {
                  if (i != j)
                  {
                      // Calculate gravitational force between bodies i and j
                      Vector3 force = calculateGravitationalForce(sim->bodies[i].position, sim->bodies[i].mass,
                          sim->bodies[j].position, sim->bodies[j].mass);
      
                      netForce = Vector3Add(netForce, force);
                  }
              }
      
              // Calculate acceleration (F = ma, so a = F/m)
              accelerations[i] = Vector3Scale(netForce, 1.0f / sim->bodies[i].mass);
      
              // Update velocity using current acceleration (v(n+1) = v(n) + a(n) * dt)
              sim->bodies[i].velocity = Vector3Add(sim->bodies[i].velocity,
                  Vector3Scale(accelerations[i], sim->timeStep));
      
      
          }
      
      
          //* POSITION UPDATE
      
          // Update positions using the new velocities
          for (int i = 0; i < sim->bodyCount; i++)
          {
              // Update position (x(n+1) = x(n) + v(n+1) * dt)
              sim->bodies[i].position = Vector3Add(sim->bodies[i].position,
                  Vector3Scale(sim->bodies[i].velocity, sim->timeStep));
          }
      
          // Clean up
          delete[] accelerations;
      
          // Update simulation time
          sim->time += sim->timeStep;
      }
      