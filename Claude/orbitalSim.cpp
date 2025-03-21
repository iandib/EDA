/**
 * @brief Orbital simulation
 * @author Marc S. Ressl (modificado)
 *
 * @copyright Copyright (c) 2022-2023
 */

// Enables M_PI #define in Windows
#define _USE_MATH_DEFINES

#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "OrbitalSim.h"
#include "ephemerides.h"

#define GRAVITATIONAL_CONSTANT 6.6743E-11F
#define ASTEROIDS_MEAN_RADIUS 4E11F
#define NUM_ASTEROIDS 500  // Número de asteroides a simular

/**
 * @brief Gets a uniform random value in a range
 *
 * @param min Minimum value
 * @param max Maximum value
 * @return The random value
 */
float getRandomFloat(float min, float max)
{
    return min + (max - min) * rand() / (float)RAND_MAX;
}

/**
 * @brief Configures an asteroid
 *
 * @param body An orbital body
 * @param centerMass The mass of the most massive object in the star system
 */
void configureAsteroid(OrbitalBody *body, float centerMass)
{
    // Logit distribution
    float x = getRandomFloat(0, 1);
    float l = logf(x) - logf(1 - x) + 1;

    // https://mathworld.wolfram.com/DiskPointPicking.html
    float r = ASTEROIDS_MEAN_RADIUS * sqrtf(fabsf(l));
    float phi = getRandomFloat(0, 2.0F * (float)M_PI);

    // Surprise!
    // phi = 0;

    // https://en.wikipedia.org/wiki/Circular_orbit#Velocity
    float v = sqrtf(GRAVITATIONAL_CONSTANT * centerMass / r) * getRandomFloat(0.6F, 1.2F);
    float vy = getRandomFloat(-1E2F, 1E2F);

    // Completamos los campos con nuestros propios valores
    body->mass = 1E12F;  // Masa típica de asteroide: 1 billón de toneladas
    body->radius = 2E3F; // Radio típico de asteroide: 2km
    body->color = GRAY;
    body->position = (Vector3){r * cosf(phi), 0, r * sinf(phi)};
    body->velocity = (Vector3){-v * sinf(phi), vy, v * cosf(phi)};
}

// ----- Funciones auxiliares para el algoritmo de Barnes-Hut -----

/**
 * @brief Crea un nuevo nodo del octree
 * 
 * @param position Posición central del nodo
 * @param size Tamaño del nodo
 * @return Puntero al nuevo nodo
 */
OctreeNode* createNode(Vector3 position, float size)
{
    OctreeNode* node = (OctreeNode*)malloc(sizeof(OctreeNode));
    
    node->position = position;
    node->size = size;
    node->totalMass = 0.0f;
    node->centerOfMass = (Vector3){0.0f, 0.0f, 0.0f};
    node->isLeaf = true;
    node->body = NULL;
    
    for (int i = 0; i < 8; i++) {
        node->children[i] = NULL;
    }
    
    return node;
}

/**
 * @brief Libera la memoria de un nodo y sus hijos recursivamente
 * 
 * @param node Nodo a liberar
 */
void destroyNode(OctreeNode* node)
{
    if (node == NULL)
        return;
        
    for (int i = 0; i < 8; i++) {
        if (node->children[i] != NULL) {
            destroyNode(node->children[i]);
        }
    }
    
    free(node);
}

/**
 * @brief Determina el octante al que pertenece un punto respecto a un nodo
 * 
 * @param nodePos Posición del nodo
 * @param bodyPos Posición del cuerpo
 * @return Índice del octante (0-7)
 */
int getOctant(Vector3 nodePos, Vector3 bodyPos)
{
    int octant = 0;
    if (bodyPos.x >= nodePos.x) octant |= 1;  // Bit 0: x >= centro
    if (bodyPos.y >= nodePos.y) octant |= 2;  // Bit 1: y >= centro
    if (bodyPos.z >= nodePos.z) octant |= 4;  // Bit 2: z >= centro
    return octant;
}

/**
 * @brief Obtiene la posición central de un octante específico
 * 
 * @param nodePos Posición del nodo padre
 * @param octant Índice del octante (0-7)
 * @param halfSize Mitad del tamaño del nodo padre
 * @return Posición central del octante
 */
Vector3 getOctantPosition(Vector3 nodePos, int octant, float halfSize)
{
    Vector3 pos = nodePos;
    
    if (octant & 1) pos.x += halfSize; else pos.x -= halfSize;  // Bit 0: x
    if (octant & 2) pos.y += halfSize; else pos.y -= halfSize;  // Bit 1: y
    if (octant & 4) pos.z += halfSize; else pos.z -= halfSize;  // Bit 2: z
    
    return pos;
}

/**
 * @brief Inserta un cuerpo en el octree
 * 
 * @param node Nodo actual del octree
 * @param body Cuerpo a insertar
 */
void insertBody(OctreeNode* node, OrbitalBody* body)
{
    // Si el nodo está vacío, asignamos directamente el cuerpo
    if (node->totalMass == 0) {
        node->body = body;
        node->totalMass = body->mass;
        node->centerOfMass = body->position;
        return;
    }
    
    // Actualizamos el centro de masa y la masa total del nodo
    float newTotalMass = node->totalMass + body->mass;
    node->centerOfMass.x = (node->centerOfMass.x * node->totalMass + body->position.x * body->mass) / newTotalMass;
    node->centerOfMass.y = (node->centerOfMass.y * node->totalMass + body->position.y * body->mass) / newTotalMass;
    node->centerOfMass.z = (node->centerOfMass.z * node->totalMass + body->position.z * body->mass) / newTotalMass;
    node->totalMass = newTotalMass;
    
    // Si es un nodo hoja con un cuerpo, subdividimos
    if (node->isLeaf && node->body != NULL) {
        // Guardamos el cuerpo actual
        OrbitalBody* oldBody = node->body;
        node->body = NULL;
        node->isLeaf = false;
        
        // Determinar en qué octante insertar el cuerpo antiguo
        int octant = getOctant(node->position, oldBody->position);
        float halfSize = node->size / 2.0f;
        
        // Crear el octante si no existe
        if (node->children[octant] == NULL) {
            Vector3 childPos = getOctantPosition(node->position, octant, halfSize / 2.0f);
            node->children[octant] = createNode(childPos, halfSize);
        }
        
        // Insertar el cuerpo antiguo en su octante
        insertBody(node->children[octant], oldBody);
    }
    
    // Insertar el nuevo cuerpo en su octante correspondiente
    if (!node->isLeaf) {
        int octant = getOctant(node->position, body->position);
        float halfSize = node->size / 2.0f;
        
        // Crear el octante si no existe
        if (node->children[octant] == NULL) {
            Vector3 childPos = getOctantPosition(node->position, octant, halfSize / 2.0f);
            node->children[octant] = createNode(childPos, halfSize);
        }
        
        // Insertar el cuerpo en su octante
        insertBody(node->children[octant], body);
    }
}

/**
 * @brief Construye el octree para la simulación
 * 
 * @param sim Simulación orbital
 * @return Raíz del octree
 */
OctreeNode* buildOctree(OrbitalSim* sim)
{
    // Encontrar los límites del espacio
    Vector3 min = sim->bodies[0].position;
    Vector3 max = sim->bodies[0].position;
    
    for (int i = 1; i < sim->bodyCount; i++) {
        Vector3 pos = sim->bodies[i].position;
        
        // Actualizar mínimos
        if (pos.x < min.x) min.x = pos.x;
        if (pos.y < min.y) min.y = pos.y;
        if (pos.z < min.z) min.z = pos.z;
        
        // Actualizar máximos
        if (pos.x > max.x) max.x = pos.x;
        if (pos.y > max.y) max.y = pos.y;
        if (pos.z > max.z) max.z = pos.z;
    }
    
    // Calcular el centro y tamaño del octree
    Vector3 center = {
        (min.x + max.x) / 2.0f,
        (min.y + max.y) / 2.0f,
        (min.z + max.z) / 2.0f
    };
    
    // Buscar el tamaño máximo en cualquier dimensión
    float size = max.x - min.x;
    if (max.y - min.y > size) size = max.y - min.y;
    if (max.z - min.z > size) size = max.z - min.z;
    
    // Añadir margen para asegurar que todos los cuerpos estén dentro
    size *= 1.1f;
    
    // Crear el nodo raíz
    OctreeNode* root = createNode(center, size);
    
    // Insertar todos los cuerpos
    for (int i = 0; i < sim->bodyCount; i++) {
        insertBody(root, &sim->bodies[i]);
    }
    
    return root;
}

/**
 * @brief Calcula la aceleración de un cuerpo usando el algoritmo Barnes-Hut
 * 
 * @param node Nodo actual del octree
 * @param body Cuerpo para el cual calcular la aceleración
 * @param acceleration Vector de aceleración a actualizar
 * @param theta Parámetro de precisión
 */
void calculateAcceleration(OctreeNode* node, OrbitalBody* body, Vector3* acceleration, float theta)
{
    // Si el nodo está vacío, no hay fuerza
    if (node->totalMass == 0) {
        return;
    }
    
    // Calcular vector de distancia al centro de masa del nodo
    Vector3 distanceVec = Vector3Subtract(node->centerOfMass, body->position);
    float distance = Vector3Length(distanceVec);
    
    // Evitar auto-interacción y división por cero
    if (distance < 1e-6f) {
        return;
    }
    
    // Criterio de apertura: s/d < theta
    // donde s: tamaño del nodo, d: distancia al centro de masa
    if (node->isLeaf || (node->size / distance < theta)) {
        // Tratar el nodo como una única partícula
        
        // Calcular magnitud de la fuerza gravitacional (G*m1*m2/d^2)
        float forceMagnitude = GRAVITATIONAL_CONSTANT * node->totalMass / (distance * distance);
        
        // Convertir a aceleración (F/m = G*m2/d^2)
        Vector3 unitDir = Vector3Normalize(distanceVec);
        Vector3 accelContribution = Vector3Scale(unitDir, forceMagnitude);
        
        // Acumular la aceleración
        *acceleration = Vector3Add(*acceleration, accelContribution);
    } else {
        // El nodo no cumple el criterio, recorrer sus hijos
        for (int i = 0; i < 8; i++) {
            if (node->children[i] != NULL) {
                calculateAcceleration(node->children[i], body, acceleration, theta);
            }
        }
    }
}

/**
 * @brief Constructs an orbital simulation
 *
 * @param timeStep The time step
 * @return The orbital simulation
 */
OrbitalSim* constructOrbitalSim(float timeStep)
{
    OrbitalSim* sim = (OrbitalSim*)malloc(sizeof(OrbitalSim));
    if (!sim)
        return NULL;
    
    // Inicializar parámetros de la simulación
    sim->timeStep = timeStep;
    sim->time = 0.0f;
    sim->theta = 0.5f;  // Valor típico para precisión/velocidad en Barnes-Hut
    
    // Contar cuerpos en el sistema solar
    int solarSystemCount = 0;
    while (solarSystem[solarSystemCount].name != NULL)
        solarSystemCount++;
    
    // Reservar memoria para todos los cuerpos (sistema solar + asteroides)
    sim->bodyCount = solarSystemCount + NUM_ASTEROIDS;
    sim->bodies = (OrbitalBody*)malloc(sim->bodyCount * sizeof(OrbitalBody));
    
    if (!sim->bodies) {
        free(sim);
        return NULL;
    }
    
    // Copiar los cuerpos del sistema solar
    for (int i = 0; i < solarSystemCount; i++) {
        sim->bodies[i].name = strdup(solarSystem[i].name);
        sim->bodies[i].mass = solarSystem[i].mass;
        sim->bodies[i].radius = solarSystem[i].radius;
        sim->bodies[i].color = solarSystem[i].color;
        sim->bodies[i].position = solarSystem[i].position;
        sim->bodies[i].velocity = solarSystem[i].velocity;
    }
    
    // Encontrar el cuerpo más masivo (generalmente el Sol)
    float centerMass = 0.0f;
    for (int i = 0; i < solarSystemCount; i++) {
        if (sim->bodies[i].mass > centerMass) {
            centerMass = sim->bodies[i].mass;
        }
    }
    
    // Configurar los asteroides
    for (int i = 0; i < NUM_ASTEROIDS; i++) {
        sim->bodies[solarSystemCount + i].name = NULL;  // Los asteroides no tienen nombre
        configureAsteroid(&sim->bodies[solarSystemCount + i], centerMass);
    }
    
    return sim;
}

/**
 * @brief Destroys an orbital simulation
 */
void destroyOrbitalSim(OrbitalSim* sim)
{
    if (sim == NULL)
        return;
    
    // Liberar memoria de los nombres de los cuerpos
    for (int i = 0; i < sim->bodyCount; i++) {
        if (sim->bodies[i].name != NULL) {
            free((void*)sim->bodies[i].name);
        }
    }
    
    // Liberar el arreglo de cuerpos
    free(sim->bodies);
    
    // Liberar la estructura de simulación
    free(sim);
}

/**
 * @brief Simulates a timestep
 *
 * @param sim The orbital simulation
 */
void updateOrbitalSim(OrbitalSim* sim)
{
    if (sim == NULL || sim->bodyCount == 0)
        return;
    
    // Construir el octree para la simulación (algoritmo Barnes-Hut)
    OctreeNode* root = buildOctree(sim);
    
    // Arreglo temporal para almacenar las aceleraciones
    Vector3* accelerations = (Vector3*)malloc(sim->bodyCount * sizeof(Vector3));
    
    // PASO 1: Calcular todas las aceleraciones
    for (int i = 0; i < sim->bodyCount; i++) {
        // Inicializar aceleración a cero
        accelerations[i] = (Vector3){0.0f, 0.0f, 0.0f};
        
        // Calcular aceleración usando el algoritmo Barnes-Hut
        calculateAcceleration(root, &sim->bodies[i], &accelerations[i], sim->theta);
    }
    
    // PASO 2: Actualizar velocidades con las aceleraciones calculadas
    for (int i = 0; i < sim->bodyCount; i++) {
        // v_i(n+1) = v_i(n) + a_i(n) * Δt
        sim->bodies[i].velocity = Vector3Add(
            sim->bodies[i].velocity, 
            Vector3Scale(accelerations[i], sim->timeStep)
        );
    }
    
    // PASO 3: Actualizar posiciones con las nuevas velocidades
    for (int i = 0; i < sim->bodyCount; i++) {
        // x_i(n+1) = x_i(n) + v_i(n+1) * Δt
        sim->bodies[i].position = Vector3Add(
            sim->bodies[i].position, 
            Vector3Scale(sim->bodies[i].velocity, sim->timeStep)
        );
    }
    
    // Actualizar el tiempo de simulación
    sim->time += sim->timeStep;
    
    // Liberar memoria temporal
    free(accelerations);
    destroyNode(root);
}