/* *****************************************************************
    * FILE INFORMATION *
   ***************************************************************** */
   
/// @brief Implements an orbital simulation view
/// @author Marc S. Ressl, Ian A. Dib, Luciano S. Cordero
/// @copyright Copyright (c) 2022-2023


/* *****************************************************************
    * FILE CONFIGURATION *
   ***************************************************************** */

    //* NECESSARY LIBRARIES AND HEADERS

    #include <time.h>
    #include "View.h"


    //* CONSTANTS
    
    #define WINDOW_WIDTH 1280
    #define WINDOW_HEIGHT 720

    // Scale factor for converting astronomical distances to screen coordinates
    #define SCALE_FACTOR 1E-11F

    // Constants for UI placement
    #define UI_TEXT_SIZE 20
    #define UI_MARGIN 10
    #define UI_LINE_SPACING 25
    
    // Color definitions for UI text
    #define UI_TEXT_COLOR RAYWHITE
    #define UI_HIGHLIGHT_COLOR YELLOW

    // Point size for distant objects
    #define POINT_SIZE 1.0f


/* *****************************************************************
    * LOGIC MODULES *
   ***************************************************************** */

    //* GENERAL FUNCTIONALITY MODULES

    /// @brief Converts a timestamp (number of seconds since 1/1/2022)
            // to an ISO date ("YYYY-MM-DD")
    /// @param timestamp
    /// @return The ISO date (a raylib string)
    const char *getISODate(float timestamp)
    {
        // Timestamp epoch: 1/1/2022
        struct tm unichEpochTM = {0, 0, 0, 1, 0, 122};

        // Convert timestamp to UNIX timestamp (number of seconds since 1/1/1970)
        time_t unixEpoch = mktime(&unichEpochTM);
        time_t unixTimestamp = unixEpoch + (time_t)timestamp;

        // Returns ISO date
        struct tm *localTM = localtime(&unixTimestamp);
        return TextFormat("%04d-%02d-%02d",
                        1900 + localTM->tm_year, localTM->tm_mon + 1, localTM->tm_mday);
    }

    
    //* VIEW MANAGEMENT MODULES

    /// @brief Constructs an orbital simulation view
    /// @param fps Frames per second for the view
    /// @return The view
    View *constructView(int fps)
    {
        View *view = new View();

        InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "EDA Orbital Simulation");
        SetTargetFPS(fps);
        DisableCursor();

        // Position the camera at a better viewing angle for the solar system
        view->camera.position = {0.0f, 10.0f, 10.0f};
        view->camera.target = {0.0f, 0.0f, 0.0f};
        view->camera.up = {0.0f, 1.0f, 0.0f};
        view->camera.fovy = 45.0f;
        view->camera.projection = CAMERA_PERSPECTIVE;

        return view;
    }


    /// @brief Destroys an orbital simulation view
    /// @param view The view
    void destroyView(View *view)
    {
        CloseWindow();
        delete view;
    }


    /// @brief Should the view still render?
    /// @param view
    /// @return Should rendering continue?
    bool isViewRendering(View *view)
    {
        return !WindowShouldClose();
    }


    /// @brief Renders an orbital simulation
    /// @param view
    /// @param sim The orbital sim
    void renderView(View *view, OrbitalSim *sim)
    {
        UpdateCamera(&view->camera, CAMERA_FREE);

        BeginDrawing();

        ClearBackground(BLACK);
        BeginMode3D(view->camera);

        //* 3D DRAWING CODE

        for(int i = 0; i < sim->bodyCount; i++)
        {
            // Scale position according to the recommended scale factor
            Vector3 scaledPosition = Vector3Scale(sim->bodies[i].position, SCALE_FACTOR);
            
            // Calculate visual size using the recommended empirical formula
            float visualRadius = 0.005F * logf(sim->bodies[i].radius);
            
            // Draw the sphere for the celestial body
            DrawSphere(scaledPosition, visualRadius, sim->bodies[i].color);
            
            // Draw a point for distant objects to ensure visibility
            DrawPoint3D(scaledPosition, sim->bodies[i].color);
        }


        // Draw reference grid
        DrawGrid(30, 1.0f);
        
        EndMode3D();

        //* 2D DRAWING CODE
        
        DrawFPS(UI_MARGIN, UI_MARGIN);
        
        // Show simulation date using the provided getISODate function
        const char* dateStr = getISODate(sim->time);
        DrawText(dateStr, UI_MARGIN, UI_MARGIN + UI_LINE_SPACING, UI_TEXT_SIZE, UI_TEXT_COLOR);
        
        // Show simulation time in days
        DrawText(TextFormat("Simulation Time: %.2f days", sim->time / 86400), 
                 UI_MARGIN, UI_MARGIN + 2 * UI_LINE_SPACING, UI_TEXT_SIZE, UI_TEXT_COLOR);
        
        // Show navigation help
        DrawText("Camera Controls: WASD to move, SPACE/CTRL to up/down, Q/E to rotate", 
                 UI_MARGIN, WINDOW_HEIGHT - UI_LINE_SPACING, UI_TEXT_SIZE, UI_HIGHLIGHT_COLOR);
        
        EndDrawing();
    }