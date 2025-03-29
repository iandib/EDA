# EDA #level1: Orbital simulation

## Integrantes del grupo y contribución al trabajo de cada integrante

* [Ian Dib]: [Implementó las funciones calculateGravitationalForce y calculateAccelerations para modelar las interacciones gravitacionales entre los cuerpos simulados, optimizando posteriormente el cálculo para permitir la simulación eficiente de 500+ asteroides sin comprometer el rendimiento. Además, desarrolló viewOptimizer en view.cpp, optimizando el renderizado de los asteroides al ajustar su visibilidad según la distancia de la cámara. Como mejora adicional, ajustó la función configureAsteroid en orbitalSim.cpp para proporcionar una distribución más realista de los asteroides dentro del sistema solar, a pesar de no estar propuesto en la guía del proyecto.]

* [Luciano Cordero]: [Implementó la simulación del agujero negro y del sistema Alfa Centauri, ampliando la funcionalidad de constructOrbitalSim en orbitalSim.cpp y gestionando el control de estos elementos dentro de orbitalSim.h. Adicionalmente, desarrolló la funcionalidad de renderView en view.cpp, encargada de definir y ajustar el renderizado de los cuerpos. También llevó a cabo la validación del timestep para garantizar la estabilidad numérica de la simulación y evaluó la precisión del tipo de dato float en los cálculos físicos.]

## Verificación del timestep

[La simulación permite ajustar la velocidad del tiempo simulado mediante la variable timeMultiplier. Este parámetro afecta la escala temporal, determinando cuántos días simulados transcurren por cada segundo real.

Como se nos explicó en las clases prácticas, un delta-t mayor reduce la carga computacional, pero a costa de precisión, mientras que un delta-t menor mejora la estabilidad de la simulación a expensas de mayor procesamiento. En consecuencia, optamos por realizar pruebas para evaluar el comportamiento de la simulación con diferentes valores de timeMultiplier:

- Con timeMultiplier = 1200 * SECONDS_PER_DAY Mercurio resultó visible por apenas un breve período antes de perder su órbita, lo que indica una inestabilidad severa en la simulación.

- Con timeMultiplier = 1000 * SECONDS_PER_DAY se observó que Venus, la Tierra y Mercurio dejan de ser visibles, lo que sugiere que han sido expulsados de sus órbitas debido a errores acumulativos en la integración numérica.

- Con timeMultiplier = 800 * SECONDS_PER_DAY aunque la simulación está acelerada, no observamos comportamientos erráticos significativos en una prueba de más de 200 años, lo que sugiere que el error acumulado sigue dentro de un margen aceptable.

El timeMultiplier óptimo es aquel cuyo valor es el más alto que permite una aceleración considerable de la simulación sin comprometer la estabilidad orbital por errores acumulativos. Sin embargo, siendo que el timeStep depende directamente de timeMultiplier / fps, un mayor fps puede compensar en parte el aumento de timeMultiplier, permitiendo simulaciones más rápidas sin degradar la precisión numérica. 

En consecuencia, no hay una única respuesta para lo que es un timeStep adecuado. En la simulación dejamos un valor por defecto de 50 * SECONDS_PER_DAY para que las órbitas fueran más apreciables.]


## Verificación del tipo de datos float

[La precisión del tipo de dato elegido para la simulación, float, es adecuada. Para verificarlo, realizamos una prueba modificando la implementación de los vectores de raymath para operar con double en lugar de float, sin observar cambios en la estabilidad o precisión de la simulación. Aunque el cambio pudo haber incrementado ligeramente el tiempo de cómputo, no detectamos ningún impacto en el rendimiento durante nuestras pruebas.

Esto se debe a que, con float, los valores utilizados en la simulación se mantienen dentro de un rango en el que la precisión es suficiente. Considerando que float tiene una precisión de 23 bits en la mantisa (aproximadamente 7 cifras decimales), el error relativo en magnitudes como la distancia Tierra-Sol (~1.5 × 10⁸ km) resulta insignificante en el contexto de la simulación orbital. Además, las velocidades planetarias y los intervalos de tiempo utilizados tampoco generan acumulación de errores perceptibles en la trayectoria de los cuerpos.

Si en lugar de float se utilizara int, la simulación perdería precisión en los cálculos de aceleración y posición, ya que la representación discreta de los números enteros no permite modelar correctamente valores con alta variabilidad dentro de escalas pequeñas. Así, float es la opción adecuada para este tipo de simulación.

Como verificación adicional, vimos que los valores numéricos manejados en la simulación permanecen dentro de los límites representables por el tipo float. Un float en IEEE 754 puede almacenar valores en un rango que va desde aproximadamente 1.2 × 10⁻³⁸ hasta 3.4 × 10³⁸ (https://en.wikipedia.org/wiki/Single-precision_floating-point_format), lo que abarca valores varios órdenes de magnitud menor y mayor que cualquier distancia o fuerza gravitacional en la simulación. La ausencia de warnings de compilación y el correcto funcionamiento de la simulación indican que, como se esperaba, no se producen casos de overflow ni de underflow.]
## Complejidad computacional con asteroides

[Inicialmente, se calculaba la fuerza gravitatoria entre todos los pares de cuerpos en la simulación, incluyendo a los asteroides. Esto ocasionaba que el algoritmo en la función calculateGravitationalForce tuviera una complejidad 𝑂(𝑛^2), afectando su rendimiento. Este crecimiento cuadrático se volvió un cuello de botella en la simulación cuando se intentó agregar más de 500 asteroides, reduciendo drásticamente la velocidad de ejecución.

Además, cada asteroide se representaba gráficamente como una esfera, lo que añadía una sobrecarga adicional de renderizado. Esta carga era menor que la del cálculo gravitacional, pero también contribuía a la ralentización de la simulación.]

## Mejora de la complejidad computacional

[Para resolver el problema de la complejidad cuadrática en el cálculo de fuerzas gravitatorias entre asteroides, se modificó la función calculateGravitationalForce y se agregó calculateAccelerations en orbitalSim.cpp. Con esta nueva implementación, solo se calcularon:

- Interacciones entre cuerpos masivos (planetas, soles, agujero negro).

- Interacciones entre cuerpos masivos y asteroides.

Se ignoraron las interacciones entre asteroides, reduciendo drásticamente la cantidad de cálculos de 𝑂(𝑛^2) a 𝑂(𝑛) para los asteroides.

Asimismo, para mejorar el rendimiento gráfico, se implementó la función viewOptimizer en view.cpp. Esta optimización consistió en representar los asteroides como líneas cuando la cámara está alejada. La dirección de la línea se calculó como la diferencia entre sus dos últimas posiciones, permitiendo visualizar su trayectoria sin necesidad de renderizarlos como esferas.

También probamos dejar los planetas lejanos puntos, pero el impacto en el rendimiento fue insignificante, por lo que descartamos esta optimización.]

## Bonus points

[Además de mejorar el realismo en la distribución inicial de los asteroides, aunque no fuese un requisito de la guía, se implementaron las siguientes funcionalidades adicionales:

Configuraciones avanzadas en OrbitalSim.h:

- Se agregó la opción de multiplicar por 1000 la masa de Júpiter o añadir un agujero negro en el sistema solar.

Al incrementar la masa de Júpiter, las órbitas de todos los planetas y del Sol se ven alteradas. El agujero negro, por su parte, cruza el sistema solar atrayendo gravitatoriamente los cuerpos a su paso. Si un planeta o asteroide se acerca demasiado, realiza un giro cerrado y, debido a la ausencia de efectos relativistas en la simulación, es acelerado violentamente en lugar de formar una órbita estable o caer en el horizonte de eventos.

- Añadido el sistema Alpha Centauri

- Easter Egg: En el código base, el parámetro phi (ángulo de fase inicial en la órbita de los planetas) estaba definido como cero. Como resultado, todos los planetas aparecían alineados sobre el mismo eje.
No llegamos a completar la implementación de la nave con físicas realistas, así que descartamos ese código.]
