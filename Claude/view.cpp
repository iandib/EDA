/**
 * @brief Orbital simulation view
 *
 * @copyright Copyright (c) 2022-2023
 */

#include <stdio.h>
#include <string.h>
#include <time.h>

#include "raylib.h"
#include "raymath.h"

#include "OrbitalSim.h"

// Parámetros de visualización
#define POSITION_SCALE 1E-11F  // Factor de escala para posiciones
#define RADIUS_SCALE_FACTOR 0.005F  // Factor de escala para radios

// Distancias para nivel de detalle (LOD)
#define FULL_DETAIL_DISTANCE 10.0f
#define MEDIUM_DETAIL_DISTANCE 30.0f
#define LOW_DETAIL_DISTANCE 60.0f

// Número máximo de cuerpos principales que siempre se deben mostrar con detalle
#define MAX_MAIN_BODIES 10

/**
 * @brief Obtiene la fecha en formato ISO a partir del tiempo de simulación
 * 
 * @param time Tiempo de simulación en segundos
 * @param buffer Buffer para almacenar la fecha
 * @param bufferSize Tamaño del buffer
 */
void getISODate(float time, char* buffer, int bufferSize)
{
    // Fecha base: 1 de enero de 2023
    struct tm date = {0};
    date.tm_year = 2023 - 1900;
    date.tm_mday = 1;
    
    // Convertir tiempo de simulación a tiempo UNIX
    time_t unixTime = mktime(&date) + (time_t)time;
    
    // Convertir a estructura tm
    struct tm* newDate = gmtime(&unixTime);
    
    // Formatear como ISO8601
    strftime(buffer, bufferSize, "%Y-%m-%d %H:%M:%S UTC", newDate);
}

/**
 * @brief Configura la cámara para la simulación
 * 
 * @return La cámara configurada
 */
Camera setupCamera()
{
    Camera camera = {0};
    camera.position = (Vector3){0.0f, 10.0f, -20.0f};
    camera.target = (Vector3){0.0f, 0.0f, 0.0f};
    camera.up = (Vector3){0.0f, 1.0f, 0.0f};
    camera.fovy = 60.0f;
    camera.projection = CAMERA_PERSPECTIVE;
    
    return camera;
}

/**
 * @brief Actualiza la cámara según la entrada del usuario
 * 
 * @param camera Puntero a la cámara a actualizar
 */
void updateCamera(Camera* camera)
{
    // Mover adelante/atrás/izquierda/derecha con WASD
    if (IsKeyDown(KEY_W)) camera->position.z += 0.5f;
    if (IsKeyDown(KEY_S)) camera->position.z -= 0.5f;
    if (IsKeyDown(KEY_A)) camera->position.x -= 0.5f;
    if (IsKeyDown(KEY_D)) camera->position.x += 0.5f;
    
    // Mover arriba/abajo con SPACE/CTRL
    if (IsKeyDown(KEY_SPACE)) camera->position.y += 0.5f;
    if (IsKeyDown(KEY_LEFT_CONTROL)) camera->position.y -= 0.5f;
    
    // Rotar izquierda/derecha con Q/E
    if (IsKeyDown(KEY_Q)) {
        // Rotación alrededor del eje Y
        Vector3 forward = Vector3Subtract(camera->target, camera->position);
        float distance = Vector3Length(forward);
        forward = Vector3Normalize(forward);
        
        // Matriz de rotación para Y
        Vector3 newForward = {
            forward.x * cosf(0.02f) - forward.z * sinf(0.02f),
            forward.y,
            forward.x * sinf(0.02f) + forward.z * cosf(0.02f)
        };
        
        camera->target = Vector3Add(camera->position, Vector3Scale(newForward, distance));
    }
    
    if (IsKeyDown(KEY_E)) {
        // Rotación alrededor del eje Y (opuesta a Q)
        Vector3 forward = Vector3Subtract(camera->target, camera->position);
        float distance = Vector3Length(forward);
        forward = Vector3Normalize(forward);
        
        // Matriz de rotación para Y
        Vector3 newForward = {
            forward.x * cosf(-0.02f) - forward.z * sinf(-0.02f),
            forward.y,
            forward.x * sinf(-0.02f) + forward.z * cosf(-0.02f)
        };
        
        camera->target = Vector3Add(camera->position, Vector3Scale(newForward, distance));
    }
}

/**
 * @brief Calcula un tamaño de radio escalado para visualización
 * 
 * @param radius Radio real del cuerpo
 * @return Radio escalado para visualización
 */
float getScaledRadius(float radius)
{
    return RADIUS_SCALE_FACTOR * logf(radius);
}

/**
 * @brief Renderiza la vista de la simulación
 * 
 * @param sim Simulación orbital
 */
void renderView(const OrbitalSim* sim)
{
    static Camera camera = {0};
    static bool cameraInitialized = false;
    
    // Inicializar la cámara la primera vez
    if (!cameraInitialized) {
        camera = setupCamera();
        cameraInitialized = true;
    }
    
    // Actualizar la cámara según la entrada del usuario
    updateCamera(&camera);
    
    // Iniciar modo 3D
    BeginMode3D(camera);
    
    // --------- Fill in your 3D drawing code here: ---------
    
    // Dibujar los cuerpos celestes
    for (int i = 0; i < sim->bodyCount; i++) {
        // Escalar posición para visualización
        Vector3 scaledPos = Vector3Scale(sim->bodies[i].position, POSITION_SCALE);
        
        // Calcular distancia desde la cámara
        float distanceFromCamera = Vector3Length(Vector3Subtract(scaledPos, camera.position));
        
        // Determinar nivel de detalle según distancia y tipo de cuerpo
        if (i < MAX_MAIN_BODIES || distanceFromCamera < FULL_DETAIL_DISTANCE) {
            // Nivel máximo de detalle: esfera completa
            float scaledRadius = getScaledRadius(sim->bodies[i].radius);
            DrawSphere(scaledPos, scaledRadius, sim->bodies[i].color);
            
            /* Para cuerpos principales, dibujar también su nombre
            if (i < MAX_MAIN_BODIES && sim->bodies[i].name != NULL) {
                // Posición ajustada para el texto, ligeramente sobre el cuerpo
                Vector3 textPos = Vector3Add(scaledPos, (Vector3){0.0f, getScaledRadius(sim->bodies[i].radius) * 1.5f, 0.0f});
                DrawText3D(sim->bodies[i].name, textPos, 0.5f, 0.5f, 0.5f, WHITE);
            } */
        } 
        else if (distanceFromCamera < MEDIUM_DETAIL_DISTANCE) {
            // Nivel medio de detalle: punto grande
            DrawPoint3D(scaledPos, sim->bodies[i].color);
        }
        else if (distanceFromCamera < LOW_DETAIL_DISTANCE) {
            // Nivel bajo de detalle: punto pequeño, y solo mostrar una fracción
            if (i % 5 == 0) {
                DrawPoint3D(scaledPos, sim->bodies[i].color);
            }
        }
        else {
            // Muy lejos: solo mostrar una pequeña fracción
            if (i % 20 == 0) {
                DrawPoint3D(scaledPos, ColorAlpha(sim->bodies[i].color, 0.5f));
            }
        }
    }
    
    // Finalizar modo 3D
    EndMode3D();
    
    // --------- Fill in your 2D drawing code here: ---------
    
    // Mostrar FPS
    DrawFPS(10, 10);
    
    // Mostrar tiempo de simulación
    char timeBuffer[64];
    sprintf(timeBuffer, "Simulation Time: %.2f days", sim->time / (24.0f * 3600.0f));
    DrawText(timeBuffer, 10, 30, 20, WHITE);
    
    // Mostrar fecha
    char dateBuffer[64];
    getISODate(sim->time, dateBuffer, sizeof(dateBuffer));
    DrawText(dateBuffer, 10, 50, 20, WHITE);
    
    // Mostrar número de cuerpos
    char bodyCountBuffer[64];
    sprintf(bodyCountBuffer, "Bodies: %d", sim->bodyCount);
    DrawText(bodyCountBuffer, 10, 70, 20, WHITE);
    
    // Mostrar controles
    DrawText("Controls: WASD = Move, SPACE/CTRL = Up/Down, Q/E = Rotate", 10, GetScreenHeight() - 30, 20, GRAY);
}