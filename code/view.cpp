/* *****************************************************************
    * FILE INFORMATION *
   ***************************************************************** */
   
/// @brief Implements an orbital simulation view
/// @author Marc S. Ressl, Ian A. Dib, Luciano S. Cordero
/// @copyright Copyright (c) 2022-2023


/* *****************************************************************
    * FILE CONFIGURATION *
   ***************************************************************** */

    //* NECESSARY LIBRARIES

    #include <time.h>

    #include "raylib.h"
    #include "raymath.h"

    #include <stdio.h>


    //* NECESSARY HEADERS

    #include "View.h"
    #include "OrbitalSim.h"


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

    //* DATE CONTROL

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


    //* RENDERING OPTIMIZER

    /// @brief Renders significant bodies as spheres and asteroids as either spheres or lines,
            // depending on their distance to the camera
    /// @param sim The orbital simulation
    /// @param startIndex Starting index of the group of bodies to render
    /// @param endIndex Ending index of the group of bodies to render
    /// @param renderDistance Render distance threshold
    /// @param cameraDistance Camera distance from the origin
    void renderOptimizer(OrbitalSim *sim, int startIndex, int endIndex, 
                    float renderDistance, float cameraDistance) 
    {
        for(int i = startIndex; i < endIndex; i++)
        {
            // Scale position according to the recommended scale factor
            Vector3 scaledPosition = Vector3Scale(sim->bodies[i].position, SCALE_FACTOR);
            Vector3 scaledPreviousPosition = Vector3Scale(sim->bodies[i].previousPosition, 
                                                         SCALE_FACTOR);
            
            // Determine if the body is an asteroid
            int isAsteroid = (i < NUM_ASTEROIDS);
            
            // Calculate visual size using the recommended empirical formula
            float visualRadius = 0.005F * logf(sim->bodies[i].radius);
            
            // Significant bodies always rendered as spheres
            if (!isAsteroid)
            {
                DrawSphere(scaledPosition, visualRadius, sim->bodies[i].color);
            }

            // Asteroids have dynamic rendering based on camera distance
            else 
            {
                // Close view: draw asteroids as spheres
                if (cameraDistance < renderDistance)
                {
                    DrawSphere(scaledPosition, visualRadius, sim->bodies[i].color);
                }

                // Far view: asteroids rendered as lines
                else 
                {
                    // Calculate and normalize the direction vector of the asteroid's movement
                    Vector3 direction = Vector3Subtract(scaledPosition, scaledPreviousPosition);
                    direction = Vector3Normalize(direction);

                    // The line is drawn as a small segment in the direction of the movement
                    Vector3 lineTop = Vector3Add(scaledPosition, Vector3Scale(direction, 0.1f));
                    Vector3 lineBottom = Vector3Subtract(scaledPosition, Vector3Scale(direction, 0.1f));
                    DrawLine3D(lineTop, lineBottom, sim->bodies[i].color);
                }
            }
        }
    }

    
    //* VIEW MANAGEMENT

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


    /// @brief Renders an orbital simulation
    /// @param view
    /// @param sim The orbital sim
    void renderView(View *view, OrbitalSim *sim)
    {
        UpdateCamera(&view->camera, CAMERA_FREE);

        // Calculate camera distance threshold for switching rendering modes
        float cameraDistance = Vector3Length(view->camera.position);
        float renderDistance = 10.0f;

        BeginDrawing();

        ClearBackground(BLACK);
        BeginMode3D(view->camera);

        //* 3D DRAWING CODE

        // Render asteroids and significant bodies if enabled in orbitalSim.cpp
        renderOptimizer(sim, 0, sim->bodyCount, renderDistance, cameraDistance);

        // Draw reference grid
        DrawGrid(50, 1.0f);
        
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


    /// @brief Should the view still render?
    /// @param view
    /// @return Should rendering continue?
    bool isViewRendering(View *view)
    {
        return !WindowShouldClose();
    }


    /// @brief Destroys an orbital simulation view
    /// @param view The view
    void destroyView(View *view)
    {
        CloseWindow();
        delete view;
    }