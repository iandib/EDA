/**
 * @brief Orbital simulation main module
 * @author Marc S. Ressl (modificado)
 *
 * @copyright Copyright (c) 2022-2023
 */

 #include "raylib.h"
 #include "OrbitalSim.h"
 #include "View.h"
 
 #define SECONDS_PER_DAY 86400
 #define FPS 60
 
 int main()
 {
     // Configuración de la simulación
     float timeMultiplier = 100 * SECONDS_PER_DAY; // 100 días por segundo de simulación
     float timeStep = timeMultiplier / FPS;
 
     // Inicialización de la ventana
     InitWindow(1280, 720, "Sistema Solar con Barnes-Hut");
     SetTargetFPS(FPS);
 
     // Construcción de la simulación
     OrbitalSim *sim = constructOrbitalSim(timeStep);
 
     // Bucle principal
     while (!WindowShouldClose())    // Detecta la tecla ESC o cierre de ventana
     {
         // Actualizar simulación
         updateOrbitalSim(sim);
 
         // Renderizado
         BeginDrawing();
             ClearBackground(BLACK);
             renderView(sim);
         EndDrawing();
     }
 
     // Limpieza
     destroyOrbitalSim(sim);
     CloseWindow();
 
     return 0;
 }