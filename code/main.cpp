/* *****************************************************************
    * FILE INFORMATION *
   ***************************************************************** */
   
/// @brief Orbital simulation main module
/// @author Marc S. Ressl
/// @copyright Copyright (c) 2022-2023


/* *****************************************************************
    * FILE CONFIGURATION *
   ***************************************************************** */

    //* NECESSARY HEADERS

    #include "OrbitalSim.h"
    #include "View.h"


    //* CONSTANTS

    #define SECONDS_PER_DAY 86400

    
/* *****************************************************************
    * MAIN LOGIC *
   ***************************************************************** */

    int main()
    {  
        // Frames per second
        int fps = 140;

        // Simulation speed: 50 days per simulation second
        float timeMultiplier = 50 * SECONDS_PER_DAY;

        // Time interval used to analyze data
        float timeStep = timeMultiplier / fps;


        //* SIMULATION SETUP, UPDATE AND RENDERING

        OrbitalSim *sim = constructOrbitalSim(timeStep);
        View *view = constructView(fps);

        while (isViewRendering(view))
        {
            updateOrbitalSim(sim);
            renderView(view, sim);
        }


        //* SIMULATION STOP AND CLEANUP

        destroyView(view);
        destroyOrbitalSim(sim);

        return 0;
    }

