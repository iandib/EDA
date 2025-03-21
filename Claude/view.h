/**
 * @brief Orbital simulation view header
 *
 * @copyright Copyright (c) 2022-2023
 */

 #ifndef VIEW_H
 #define VIEW_H
 
 #include "OrbitalSim.h"
 
 // Funciones de visualización
 void renderView(const OrbitalSim* sim);
 bool isViewOpen();
 Camera setupCamera();
 void updateCamera(Camera* camera);
 
 #endif // VIEW_H
