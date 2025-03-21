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
        // TODO Complete the structure definition


    };


    /// @brief Orbital simulation definition
    struct OrbitalSim
    {
        // TODO Complete the structure definition


    };


    //* PUBLIC FUNCTIONS PROTOTYPES

    OrbitalSim *constructOrbitalSim(float timeStep);
    void destroyOrbitalSim(OrbitalSim *sim);
    void updateOrbitalSim(OrbitalSim *sim);


    #endif // ORBITALSIM_H





