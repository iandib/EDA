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

    // Number of asteroids to generate
    #define NUM_ASTEROIDS 500


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

        // Fill in with the fields:
        body->mass = 1E12F;  // Typical asteroid weight: 1 billion tons
        body->radius = 2E3F; // Typical asteroid radius: 2km
        body->color = GRAY;
        body->position = {r * cosf(phi), 0, r * sinf(phi)};
        body->velocity = {-v * sinf(phi), vy, v * cosf(phi)};
    }


    /* *****************************************************************
       * BARNES-HUT OCTREE FUNCTIONS *
       ***************************************************************** */

    /// @brief Creates a new octree node
    /// @param position Position of the node
    /// @param size Size of the node
    /// @return Pointer to the new node
    OctreeNode* createNode(Vector3 position, float size) 
    {
        OctreeNode* node = (OctreeNode*)malloc(sizeof(OctreeNode));
        
        node->position = position;
        node->size = size;
        node->totalMass = 0;
        node->centerOfMass = {0, 0, 0};
        node->isLeaf = true;
        node->body = NULL;
        
        for (int i = 0; i < 8; i++) {
            node->children[i] = NULL;
        }
        
        return node;
    }

    /// @brief Determines which octant a position belongs to within a node
    /// @param nodePos Position of the node
    /// @param bodyPos Position of the body
    /// @return Octant index (0-7)
    int getOctant(Vector3 nodePos, Vector3 bodyPos) 
    {
        int octant = 0;
        
        // Determine which octant the body belongs to (0-7)
        if (bodyPos.x >= nodePos.x) octant |= 1;
        if (bodyPos.y >= nodePos.y) octant |= 2;
        if (bodyPos.z >= nodePos.z) octant |= 4;
        
        return octant;
    }

    /// @brief Computes the position of a child node based on parent node and octant
    /// @param parentPos Position of the parent node
    /// @param halfSize Half size of the parent node
    /// @param octant Octant index (0-7)
    /// @return Position of the child node
    Vector3 getChildPosition(Vector3 parentPos, float halfSize, int octant) 
    {
        Vector3 childPos = parentPos;
        
        if (octant & 1) childPos.x += halfSize;
        else childPos.x -= halfSize;
        
        if (octant & 2) childPos.y += halfSize;
        else childPos.y -= halfSize;
        
        if (octant & 4) childPos.z += halfSize;
        else childPos.z -= halfSize;
        
        return childPos;
    }

    /// @brief Inserts a body into the octree
    /// @param node Current node
    /// @param body Body to insert
    void insertBody(OctreeNode* node, OrbitalBody* body) 
    {
        // Update node's mass and center of mass
        Vector3 weightedPos = {
            body->position.x * body->mass,
            body->position.y * body->mass,
            body->position.z * body->mass
        };
        
        if (node->totalMass == 0) {
            node->centerOfMass = weightedPos;
        } else {
            node->centerOfMass = Vector3Add(node->centerOfMass, weightedPos);
        }
        
        node->totalMass += body->mass;
        
        // If node is empty, just store body
        if (node->totalMass == body->mass) {
            node->body = body;
            return;
        }
        
        // If this is an internal node, insert the body in the appropriate child
        if (!node->isLeaf) {
            int octant = getOctant(node->position, body->position);
            
            // Create child if it doesn't exist
            if (node->children[octant] == NULL) {
                float halfSize = node->size / 4.0f;  // Quarter of the current node size
                Vector3 childPos = getChildPosition(node->position, halfSize, octant);
                node->children[octant] = createNode(childPos, halfSize * 2.0f);
            }
            
            insertBody(node->children[octant], body);
            return;
        }
        
        // If this is a leaf node with a body already in it
        if (node->body != NULL) {
            // Store the existing body
            OrbitalBody* existingBody = node->body;
            node->body = NULL;
            node->isLeaf = false;
            
            // Insert both the existing and new bodies
            int octant1 = getOctant(node->position, existingBody->position);
            int octant2 = getOctant(node->position, body->position);
            
            // Create children if they don't exist
            if (node->children[octant1] == NULL) {
                float halfSize = node->size / 4.0f;
                Vector3 childPos = getChildPosition(node->position, halfSize, octant1);
                node->children[octant1] = createNode(childPos, halfSize * 2.0f);
            }
            
            if (octant1 != octant2 && node->children[octant2] == NULL) {
                float halfSize = node->size / 4.0f;
                Vector3 childPos = getChildPosition(node->position, halfSize, octant2);
                node->children[octant2] = createNode(childPos, halfSize * 2.0f);
            }
            
            insertBody(node->children[octant1], existingBody);
            
            // If both bodies go to the same octant, the new body will be inserted later
            if (octant1 != octant2) {
                insertBody(node->children[octant2], body);
            } else {
                insertBody(node->children[octant1], body);
            }
            
            return;
        }
        
        // If this is an empty leaf, store the body
        node->body = body;
    }

    /// @brief Builds the octree for a set of bodies
    /// @param bodies Array of bodies
    /// @param numBodies Number of bodies
    /// @return Root node of the octree
    OctreeNode* buildOctree(OrbitalBody* bodies, int numBodies) 
    {
        // Find the bounding box for all bodies
        Vector3 min = {1E20F, 1E20F, 1E20F};
        Vector3 max = {-1E20F, -1E20F, -1E20F};
        
        for (int i = 0; i < numBodies; i++) {
            if (bodies[i].position.x < min.x) min.x = bodies[i].position.x;
            if (bodies[i].position.y < min.y) min.y = bodies[i].position.y;
            if (bodies[i].position.z < min.z) min.z = bodies[i].position.z;
            
            if (bodies[i].position.x > max.x) max.x = bodies[i].position.x;
            if (bodies[i].position.y > max.y) max.y = bodies[i].position.y;
            if (bodies[i].position.z > max.z) max.z = bodies[i].position.z;
        }
        
        // Calculate the center of the bounding box
        Vector3 center = {
            (min.x + max.x) / 2.0f,
            (min.y + max.y) / 2.0f,
            (min.z + max.z) / 2.0f
        };
        
        // Calculate the size of the bounding box (use the largest dimension)
        float size = max.x - min.x;
        if (max.y - min.y > size) size = max.y - min.y;
        if (max.z - min.z > size) size = max.z - min.z;
        
        // Add a margin to ensure all bodies fit
        size *= 1.1f;
        
        // Create the root node
        OctreeNode* root = createNode(center, size);
        
        // Insert all bodies into the octree
        for (int i = 0; i < numBodies; i++) {
            insertBody(root, &bodies[i]);
        }
        
        return root;
    }

    /// @brief Recursively frees the memory allocated for an octree
    /// @param node Root node of the octree
    void destroyOctree(OctreeNode* node) 
    {
        if (node == NULL) {
            return;
        }
        
        // Recursively destroy all children
        for (int i = 0; i < 8; i++) {
            if (node->children[i] != NULL) {
                destroyOctree(node->children[i]);
            }
        }
        
        // Free the node itself
        free(node);
    }


    /* *****************************************************************
       * GRAVITATIONAL FORCE CALCULATION MODULES *
       ***************************************************************** */

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
        if (distance < 1.0f) {
            return {0, 0, 0};
        }
        
        // Normalize direction (unit vector)
        Vector3 unitDirection = Vector3Scale(direction, 1.0f / distance);
        
        // Calculate force magnitude using Newton's law of gravitation
        // F = G * m1 * m2 / r^2
        float forceMagnitude = GRAVITATIONAL_CONSTANT * mass1 * mass2 / (distance * distance);
        
        // Return force vector
        return Vector3Scale(unitDirection, forceMagnitude);
    }

    /// @brief Calculates the force on a body from a node in the octree
    /// @param body Body to calculate force for
    /// @param node Node in the octree
    /// @param theta Barnes-Hut approximation parameter
    /// @return Force vector
    Vector3 calculateForceFromNode(OrbitalBody* body, OctreeNode* node, float theta) 
    {
        // If node is empty, no force
        if (node->totalMass == 0) {
            return {0, 0, 0};
        }
        
        // Calculate the direction and distance to the node's center of mass
        Vector3 direction = Vector3Subtract(node->centerOfMass, body->position);
        float distance = Vector3Length(direction);
        
        // If distance is zero, no force (avoid division by zero)
        if (distance < 1.0f) {
            return {0, 0, 0};
        }
        
        // If this is a leaf node with a body, calculate direct force
        if (node->isLeaf && node->body != NULL) {
            // Don't calculate force from body to itself
            if (node->body == body) {
                return {0, 0, 0};
            }
            
            return calculateGravitationalForce(
                body->position, body->mass,
                node->body->position, node->body->mass
            );
        }
        
        // Check if we can use approximation
        // If s/d < theta, where s is the size of the region and d is the distance
        // to the center of mass, we can treat the node as a single body
        if (node->size / distance < theta) {
            return calculateGravitationalForce(
                body->position, body->mass,
                node->centerOfMass, node->totalMass
            );
        }
        
        // Otherwise, recursively calculate forces from children
        Vector3 totalForce = {0, 0, 0};
        
        for (int i = 0; i < 8; i++) {
            if (node->children[i] != NULL) {
                Vector3 force = calculateForceFromNode(body, node->children[i], theta);
                totalForce = Vector3Add(totalForce, force);
            }
        }
        
        return totalForce;
    }


    /* *****************************************************************
       * ORBITAL SIMULATION MANAGEMENT MODULES *
       ***************************************************************** */

    /// @brief Constructs an orbital simulation
    /// @param timeStep Time step for integration
    /// @return The orbital simulation
    OrbitalSim *constructOrbitalSim(float timeStep)
    {
        // Allocate memory for the simulation structure
        OrbitalSim *sim = (OrbitalSim*)malloc(sizeof(OrbitalSim));
        
        if (sim == NULL) {
            return NULL;
        }
        
        // Initialize fields
        sim->timeStep = timeStep;
        sim->time = 0.0f;
        sim->theta = 0.5f; // Barnes-Hut precision parameter
        
        // Define the number of bodies (solar system + asteroids)
        sim->bodyCount = SOLARSYSTEM_BODYNUM + NUM_ASTEROIDS;
        
        // Allocate memory for the bodies
        sim->bodies = (OrbitalBody*)malloc(sim->bodyCount * sizeof(OrbitalBody));
        
        if (sim->bodies == NULL) {
            free(sim);
            return NULL;
        }
        
        // Copy solar system bodies from ephemerides
        for (int i = 0; i < SOLARSYSTEM_BODYNUM; i++) {
            sim->bodies[i] = solarSystem[i];
        }
        
        // Configure asteroids
        float centerMass = solarSystem[0].mass; // Sun's mass
        for (int i = 0; i < NUM_ASTEROIDS; i++) {
            OrbitalBody *body = &sim->bodies[SOLARSYSTEM_BODYNUM + i];
            body->name = "Asteroid";
            configureAsteroid(body, centerMass);
        }
        
        return sim;
    }


    /// @brief Destroys an orbital simulation
    /// @param sim The orbital simulation
    void destroyOrbitalSim(OrbitalSim *sim)
    {
        if (sim == NULL) {
            return;
        }
        
        // Free the bodies array
        if (sim->bodies != NULL) {
            free(sim->bodies);
        }
        
        // Free the simulation structure
        free(sim);
    }


    /// @brief Simulates a timestep
    /// @param sim The orbital simulation
    void updateOrbitalSim(OrbitalSim *sim)
    {
        if (sim == NULL || sim->bodies == NULL) {
            return;
        }
        
        /* *****************************************************************
           * STEP 1: BUILD OCTREE *
           ***************************************************************** */
        OctreeNode* root = buildOctree(sim->bodies, sim->bodyCount);
        
        /* *****************************************************************
           * STEP 2: CALCULATE ACCELERATIONS AND VELOCITIES FOR ALL BODIES *
           ***************************************************************** */
        // Temporary array to store accelerations
        Vector3 *accelerations = (Vector3*)malloc(sim->bodyCount * sizeof(Vector3));
        
        // Calculate accelerations for all bodies
        for (int i = 0; i < sim->bodyCount; i++) {
            // Calculate force using Barnes-Hut algorithm
            Vector3 force = calculateForceFromNode(&sim->bodies[i], root, sim->theta);
            
            // Calculate acceleration (F = ma, so a = F/m)
            accelerations[i] = Vector3Scale(force, 1.0f / sim->bodies[i].mass);
            
            // Update velocity using current acceleration (v(n+1) = v(n) + a(n) * dt)
            sim->bodies[i].velocity = Vector3Add(
                sim->bodies[i].velocity,
                Vector3Scale(accelerations[i], sim->timeStep)
            );
        }
        
        /* *****************************************************************
           * STEP 3: UPDATE POSITIONS FOR ALL BODIES *
           ***************************************************************** */
        // Update positions using the new velocities
        for (int i = 0; i < sim->bodyCount; i++) {
            // Update position (x(n+1) = x(n) + v(n+1) * dt)
            sim->bodies[i].position = Vector3Add(
                sim->bodies[i].position,
                Vector3Scale(sim->bodies[i].velocity, sim->timeStep)
            );
        }
        
        // Clean up
        free(accelerations);
        destroyOctree(root);
        
        // Update simulation time
        sim->time += sim->timeStep;
    }