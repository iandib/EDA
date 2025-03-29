/* *****************************************************************
    * FILE INFORMATION *
   ***************************************************************** */
   
/// @brief Implements an orbital simulation view
/// @author Marc S. Ressl
/// @copyright Copyright (c) 2022-2023


/* *****************************************************************
    * HEADER CONFIGURATION *
   ***************************************************************** */

    #ifndef ORBITALSIMVIEW_H
    #define ORBITALSIMVIEW_H
   
    
    //* NECESSARY LIBRARIES AND HEADERS

    #include <raylib.h>
    #include "OrbitalSim.h"
   
   
    //* STRUCTURES

    /// @brief View data
    struct View
    {
        Camera3D camera;
    };
   
   
    //* PUBLIC FUNCTIONS PROTOTYPES

    View* constructView(int fps);
    void destroyView(View *view);
    bool isViewRendering(View *view);
    void renderView(View *view, OrbitalSim *sim);


    #endif // ORBITALSIMVIEW_H


