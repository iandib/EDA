/**
 * @brief Implements an orbital simulation view
 * @author Marc S. Ressl
 *
 * @copyright Copyright (c) 2022-2023
 */

#include <iostream>
using namespace std;

 #include <time.h>

 #include "View.h"

 #define INTERNAL_SCALE 1.0E-10F  // Convertir metros a unidades internas
 #define WINDOW_WIDTH 1280
 #define WINDOW_HEIGHT 720
 
 /**
  * @brief Converts a timestamp (number of seconds since 1/1/2022)
  *        to an ISO date ("YYYY-MM-DD")
  *
  * @param timestamp the timestamp
  * @return The ISO date (a raylib string)
  */
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
 
 /**
  * @brief Constructs an orbital simulation view
  *
  * @param fps Frames per second for the view
  * @return The view
  */
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
 
    // ? BUSCO BUG
    cout << "View initialized" << endl;

    return view;
 }
 
 /**
  * @brief Destroys an orbital simulation view
  *
  * @param view The view
  */
 void destroyView(View *view)
 {
     CloseWindow();
 
     delete view;
 }
 
 /**
  * @brief Should the view still render?
  *
  * @return Should rendering continue?
  */
 bool isViewRendering(View *view)
 {
     return !WindowShouldClose();
 }
 
 /**
  * Renders an orbital simulation
  *
  * @param view The view
  * @param sim The orbital sim
  */
 void renderView(View *view, OrbitalSim *sim)
 {
     UpdateCamera(&view->camera, CAMERA_FREE);
     
     BeginDrawing();
     
     ClearBackground(BLACK);
     BeginMode3D(view->camera);
     
     // Dibujar una cuadrícula de referencia
     DrawGrid(10, 10.0f);
     
     // Verificar si la simulación está inicializada correctamente
     if (sim && sim->bodies)
     {
         // ! No necesitamos un factor de escala adicional pq ya reescalamos internamente
         const float VISUAL_SCALE = 1.0F;
         
         // Dibujar cada cuerpo celeste
         for (int i = 0; i < sim->bodyCount; i++)
         {
             // Obtener el cuerpo actual
             OrbitalBody *body = &sim->bodies[i];
             
             // Las posiciones ya están escaladas internamente, solo ajustar visualmente si es necesario
             Vector3 scaledPosition = {
                 body->position.x * VISUAL_SCALE,
                 body->position.y * VISUAL_SCALE,
                 body->position.z * VISUAL_SCALE
             };
             
             // ? Depuración: imprimir posiciones
             // cout << "Body " << i << ": (" << scaledPosition.x << ", " 
                 // << scaledPosition.y << ", " << scaledPosition.z << ")" << endl;
             
             // Calcular el radio escalado
             float scaledRadius = 0.02f * logf(body->radius * INTERNAL_SCALE + 1.0f);
             
             // Asegurarnos de que el radio no sea menor que un valor mínimo
             if (scaledRadius < 0.05f)
                 scaledRadius = 0.05f;
             
             // Dibujamos una esfera para el cuerpo celeste
             DrawSphere(scaledPosition, scaledRadius, body->color);
             
             // Dibujamos un punto para asegurar visibilidad
             DrawPoint3D(scaledPosition, body->color);
         }
         
         // Dibujamos una línea desde el origen hasta el Sol
         if (sim->bodyCount > 0)
         {
             Vector3 sunPosition = {
                 sim->bodies[0].position.x * VISUAL_SCALE,
                 sim->bodies[0].position.y * VISUAL_SCALE,
                 sim->bodies[0].position.z * VISUAL_SCALE
             };
             DrawLine3D({0, 0, 0}, sunPosition, YELLOW);
         }
     }
     
     EndMode3D();
 
     // Información 2D
     DrawFPS(10, 10);
     
     // Información de debug para ayudar a entender el problema
     if (sim)
     {
         DrawText(TextFormat("Número de cuerpos: %d", sim->bodyCount), 10, 40, 20, WHITE);
         DrawText(TextFormat("Tiempo de simulación: %.2f", sim->elapsedTime), 10, 70, 20, WHITE);
         DrawText(TextFormat("Fecha: %s", getISODate(sim->elapsedTime)), 10, 100, 20, WHITE);
         
         // Información sobre la cámara
         DrawText(TextFormat("Posición de cámara: (%.2f, %.2f, %.2f)", 
                           view->camera.position.x, 
                           view->camera.position.y, 
                           view->camera.position.z), 
                10, 130, 20, WHITE);
     }
     else
     {
         DrawText("Error: Simulación no inicializada", 10, 40, 20, RED);
     }
     
     // Instrucciones
     DrawText("Controles: W,A,S,D para mover, MOUSE para rotar", 
              10, WINDOW_HEIGHT - 30, 20, GRAY);
 
     EndDrawing();
 }