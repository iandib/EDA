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
   
   
    //* CONSTANTS & STRUCTURES
   
    /// @brief Orbital body definition
    struct OrbitalBody
    {
        const char *name;
        float mass;         // [kg]
        float radius;       // [m]
        Color color;        // Raylib color
        Vector3 position;   // [m]
        Vector3 velocity;   // [m/s]
    };


    /// @brief Orbital simulation definition
    struct OrbitalSim
    {
        float timeStep;     // [s]
        float time;         // Total elapsed time [s]
        int bodyCount;
        OrbitalBody* bodies;
        // Barnes-Hut precision parameter (typically 0.5)
        float theta;
    };


    // Octree node structure for Barnes-Hut
    struct OctreeNode 
    {
        Vector3 centerOfMass;
        float totalMass;
        Vector3 position;
        float size;
        struct OctreeNode* children[8]; // 8 octants
        // Reference to the body (only for leaf nodes)
        OrbitalBody* body;
        bool isLeaf;
    };


    //* PUBLIC FUNCTIONS PROTOTYPES

    OrbitalSim *constructOrbitalSim(float timeStep);
    void destroyOrbitalSim(OrbitalSim *sim);
    void updateOrbitalSim(OrbitalSim *sim);


    #endif // ORBITALSIM_H





