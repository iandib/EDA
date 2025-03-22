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

    #define SCALE_FACTOR 1E-11


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

        view->camera.position = {10.0f, 10.0f, 10.0f};
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

        // TODO Complete this function with the 3D drawing code

        for(int i = 0; i < sim->bodyCount; i++)
        {
            Vector3 scaledPosition = Vector3Scale(sim->bodies[i].position, SCALE_FACTOR);
            DrawSphere(scaledPosition, 0.005F * logf(sim->bodies[i].radius), sim->bodies[i].color);
        }

        DrawGrid(10, 10.0f);
        EndMode3D();

        DrawFPS(10, 10);
        // TODO Complete this function with the 2D drawing code

        EndDrawing();
    }
