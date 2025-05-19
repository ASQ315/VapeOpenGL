// Bibliotecas necesarias
#include <GL/glut.h>          // Biblioteca principal de OpenGL
#include <GL/freeglut.h>      // Extensión de GLUT para funciones adicionales
#include <stdlib.h>           // Para funciones estándar como rand()
#include <math.h>             // Funciones matemáticas (sin, cos, etc.)
#include <vector>             // Contenedor para partículas
#include <ctime>              // Para inicializar el generador de números aleatorios
#include <algorithm>          // Para funciones como std::min/max
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"        // Cargador de texturas

// Estados de la simulación
enum VapeState { IDLE, INHALING, HOLDING, EXHALING };
VapeState vapeState = IDLE;  // Estado inicial

// Variables de animación
int frame = 0;               // Contador de frames
float vapeAngle = 0.0;       // Ángulo de rotación del vape
float vapePosX = 0.0;        // Posición X del vape
float vapePosY = 1.0;        // Posición Y del vape
float vapePosZ = -5.0;       // Posición Z del vape

// Estructura para partículas de humo
struct SmokeParticle {
    float x, y, z;           // Posición 3D
    float speed;             // Velocidad de movimiento
    float size;              // Tamaño visual
    float life;              // Tiempo de vida actual
    float maxLife;           // Tiempo de vida máximo
    bool isInhaleParticle;   // Bandera para partículas de inhalación
};

// Contenedores de partículas
std::vector<SmokeParticle> exhaleParticles;  // Partículas de exhalación
std::vector<SmokeParticle> inhaleParticles;  // Partículas de inhalación
const float INHALE_PARTICLE_RATE = 0.05f;    // Frecuencia de creación de partículas (segundos)
float timeSinceLastInhaleParticle = 0.0f;    // Temporizador para creación de partículas

// Inicialización de OpenGL
void initialize() {
    glClearColor(1.0, 1.0, 1.0, 1.0);  // Color de fondo 
    glEnable(GL_DEPTH_TEST);            // Habilitar prueba de profundidad
    glEnable(GL_BLEND);                 // Mezcla de colores para transparencia
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);  // Configuración de mezcla
    srand(time(0));                     // Semilla para números aleatorios

    // Configuración de iluminación
    GLfloat ambient[] = {0.2, 0.2, 0.2, 1.0};
    GLfloat diffuse[] = {0.9, 0.9, 0.9, 1.0};
    GLfloat specular[] = {1.0, 1.0, 1.0, 1.0};
    GLfloat position[] = {2.0, 3.0, 2.0, 1.0};

    glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, specular);
    glLightfv(GL_LIGHT0, GL_POSITION, position);

    glEnable(GL_LIGHTING);   // Habilitar iluminación
    glEnable(GL_LIGHT0);     // Habilitar luz 0
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
}

// Creación de partículas de exhalación
void addExhaleParticle() {
    SmokeParticle p;
    p.x = 0.0f + ((rand()%100)-50)/200.0f;  // Posición X con variación aleatoria
    p.y = 1.8f + ((rand()%100)-50)/200.0f;  // Posición Y (altura de la boca)
    p.z = 0.1f;                             // Posición Z (ligeramente frente al vape)
    p.speed = 0.1f + (rand()%100)/1000.0f;  // Velocidad aleatoria
    p.size = 0.45f + (rand()%100)/500.0f;   // Tamaño aleatorio
    p.life = 0.1f;                          // Tiempo de vida inicial
    p.maxLife = 5.0f + (rand()%100)/100.0f; // Vida máxima aleatoria
    p.isInhaleParticle = false;             // Marcar como partícula de exhalación
    exhaleParticles.push_back(p);
}

// Iniciar inhalación
void startInhale() {
    if (vapeState == IDLE) {
        vapeState = INHALING;
        exhaleParticles.clear();          // Limpiar partículas anteriores
        inhaleParticles.clear();
        frame = 0;                        // Reiniciar contador
        timeSinceLastInhaleParticle = 0.0f;
    }
}

// Iniciar exhalación
void startExhale() {
    if (vapeState == INHALING || vapeState == HOLDING) {
        vapeState = EXHALING;
        // Crear 30 partículas de exhalación
        for (int i = 0; i < 30; i++) {
            addExhaleParticle();
        }
        frame = 0;
    }
}

// Manejador de teclado
void handleKey(unsigned char key, int x, int y) {
    switch(key) {
        case 'i': case 'I': startInhale(); break;  // Tecla I para inhalar
        case 'e': case 'E': startExhale(); break;  // Tecla E para exhalar
    }
}

// Actualización de la animación (lógica del juego)
void updateAnimation() {
    float deltaTime = 0.016f;  // Tiempo aproximado por frame (60 FPS)
    
    // Lógica durante la inhalación
    if (vapeState == INHALING) {
        if (frame < 30) {  // Animación de 30 frames
            // Interpolación suave hacia la posición de inhalación
            vapePosX += (0.0 - vapePosX) * 0.1;
            vapePosY += (0.5 - vapePosY) * 0.1;
            vapePosZ += (0.0 - vapePosZ) * 0.1;
            vapeAngle += (45.0 - vapeAngle) * 0.2;
            frame++;
        } else {
            vapeState = HOLDING;  // Cambiar a estado de retención
        }
    } 
    
    // Generar partículas durante inhalación/retención
    if (vapeState == INHALING || vapeState == HOLDING) {
        timeSinceLastInhaleParticle += deltaTime;
        
        if (timeSinceLastInhaleParticle >= INHALE_PARTICLE_RATE) {
            SmokeParticle p;
            p.x = vapePosX + ((rand()%100)-50)/300.0f;  // Posición con variación
            p.y = vapePosY + 2.3f;  // Altura de la boquilla
            p.z = vapePosZ + ((rand()%100)-50)/300.0f;
            p.speed = 0.02f + (frame/30.0f)*0.01f;  // Velocidad aumenta con el tiempo
            p.size = 0.1f;
            p.life = 0.0f;
            p.maxLife = 1.5f;
            p.isInhaleParticle = true;
            inhaleParticles.push_back(p);
            timeSinceLastInhaleParticle = 0.0f;
        }
    }
    
    // Actualizar partículas de inhalación
    for (size_t i = 0; i < inhaleParticles.size(); ) {
        inhaleParticles[i].y += inhaleParticles[i].speed;  // Mover hacia arriba
        // Movimiento oscilante para efecto más natural
        inhaleParticles[i].x += sin(inhaleParticles[i].life*2.0f)*0.01f;
        inhaleParticles[i].size += 0.005f;  // Crecer con el tiempo
        inhaleParticles[i].life += deltaTime;
        
        // Eliminar partículas viejas
        if (inhaleParticles[i].life >= inhaleParticles[i].maxLife) {
            inhaleParticles.erase(inhaleParticles.begin() + i);
        } else {
            i++;
        }
    }
    
    // Lógica durante exhalación
    if (vapeState == EXHALING) {
        for (size_t i = 0; i < exhaleParticles.size(); ) {
            exhaleParticles[i].z -= exhaleParticles[i].speed;  // Mover hacia adelante
            exhaleParticles[i].y += 0.008f;  // Flotar hacia arriba
            exhaleParticles[i].x += (rand()%100 - 50)/1000.0f;  // Movimiento lateral aleatorio
            exhaleParticles[i].size += 0.008f;  // Expandirse
            exhaleParticles[i].life += deltaTime;
            
            // Eliminar partículas viejas
            if (exhaleParticles[i].life >= exhaleParticles[i].maxLife) {
                exhaleParticles.erase(exhaleParticles.begin() + i);
            } else {
                i++;
            }
        }
        
        // Volver a la posición inicial
        vapePosX += (0.0 - vapePosX) * 0.05;
        vapePosY += (1.0 - vapePosY) * 0.05;
        vapePosZ += (-5.0 - vapePosZ) * 0.05;
        vapeAngle += (0.0 - vapeAngle) * 0.05;
        
        // Finalizar exhalación cuando no hay partículas
        if (exhaleParticles.empty()) {
            vapeState = IDLE;
        }
    }
    
    glutPostRedisplay();  // Solicitar redibujado
}

// Dibujar partículas de humo
void drawSmoke() {
    glDisable(GL_LIGHTING);  // Desactivar iluminación para partículas
    glDepthMask(GL_FALSE);   // Desactivar escritura en buffer de profundidad
    
    // Partículas de inhalación (color azulado)
    for (const auto& p : inhaleParticles) {
        float alpha = 0.6f * (1.0f - p.life/p.maxLife);  // Transparencia por edad
        glPushMatrix();
            glTranslatef(p.x, p.y, p.z);
            glColor4f(0.7, 0.7, 0.8, alpha);  // Color con transparencia
            glutSolidSphere(p.size, 10, 10);   // Dibujar esfera
        glPopMatrix();
    }
    
    // Partículas de exhalación (color blanco)
    for (const auto& p : exhaleParticles) {
        float alpha = 0.7f * (1.0f - p.life/p.maxLife);
        glPushMatrix();
            glTranslatef(p.x, p.y, p.z);
            glColor4f(0.9, 0.9, 0.9, alpha);
            glutSolidSphere(p.size, 12, 12);
        glPopMatrix();
    }
    
    glDepthMask(GL_TRUE);  // Restaurar escritura de profundidad
    glEnable(GL_LIGHTING); // Reactivar iluminación
}

// Dibujar el modelo del vape
void drawVape() {
    
    glPushMatrix();
        // Boquilla
        glPushMatrix();
            glTranslatef(0.0, 2.8, 0.0);
            glColor3f(0.1, 0.1, 0.1);  // Color oscuro
            glRotatef(90.0, 1.0, 0.0, 0.0);
            glutSolidCylinder(0.15, 0.3, 32, 10);  // Cilindro delgado
        glPopMatrix();

        // tapa superior
        glPushMatrix();
            glTranslatef(0.0, 2.5, 0.0);
            glColor3f(0.1, 0.1, 0.1); // Color negro
            glRotatef(90.0, 1.0, 0.0, 0.0);
            glutSolidCylinder(0.4, 0.2, 32, 10);
        glPopMatrix();
        


        // Líquido dentro del tanque
        glPushMatrix();
            glTranslatef(0.0, 1.8, 0.0); // ligeramente centrado dentro del tanque
            glColor4f(1.0, 0.8, 0.6, 0.5); // Rosa translúcido
            glRotatef(90.0, 1.0, 0.0, 0.0);
            glutSolidCylinder(0.28, 0.4, 32, 10);  // Más delgado y más corto
        glPopMatrix();

        // Base inferior
        glPushMatrix();
            glTranslatef(0.0, 1.5, 0.0);
            glColor3f(0.1, 0.1, 0.1); // Color negro
            glRotatef(90.0, 1.0, 0.0, 0.0);
            glutSolidCylinder(0.7, 0.2, 32, 10);
        glPopMatrix();
        // Sección media (tubo central)
        glPushMatrix();
            glTranslatef(0.0, 2.3, 0.0);
            glColor4f(0.6, 0.6, 0.9, 0.5);
            glRotatef(90.0, 1.0, 0.0, 0.0);
            glutSolidCylinder(0.1, 0.8, 32, 10);  // Cilindro delgado y largo
        glPopMatrix();

        // Tanque de vidrio
        glPushMatrix();
            glTranslatef(0.0, 2.3, 0.0);
            glRotatef(90.0, 1.0, 0.0, 0.0);

            glColor4f(1.0, 1.0, 1.0, 0.25);  // Color muy transparente
            glutSolidCylinder(0.35, 0.8, 32, 10);  // Cilindro exterior

            glColor4f(0.6, 0.7, 0.9, 0.15);  // Más transparente aún
            glutSolidCylinder(0.2, 0.8, 32, 10);  // Cilindro interior
        glPopMatrix();



        // Cuerpo principal con gradiente de color
        glEnable(GL_COLOR_MATERIAL);
        glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

        const int slices = 64;  // Resolución del cilindro
        const float height = 2.5;
        const float radius = 0.6f;

        glPushMatrix();
            glTranslatef(0.0, 1.3, 0.0);
            glRotatef(90.0, 1.0, 0.0, 0.0);
            glBegin(GL_QUAD_STRIP);
            for (int i = 0; i <= slices; ++i) {
                float theta = 2.0f * 3.1415926f * i / slices;
                float x = cos(theta) * radius;
                float y = sin(theta) * radius;
                float t = (float)i / slices;
                // Gradiente de color arcoíris
                float r = fabs(sin(t * 3.1415));
                float g = fabs(sin(t * 3.1415 + 2.0));
                float b = fabs(sin(t * 3.1415 + 4.0));
                glColor3f(r, g, b);
                glVertex3f(x, y, 0.0f);  // Vértice inferior
                glVertex3f(x, y, height); // Vértice superior
            }
            glEnd();
        glPopMatrix();

        // Base rectangular
        glPushMatrix();
            glTranslatef(0.0, 0.2, 0.0);
            glScalef(1.3, 2.5, 0.8);  // Escalar el cubo
            glColor3f(0.1, 0.1, 0.1); // Color negro
            glutSolidCube(1.0);       // Cubo unitario escalado
        glPopMatrix();
        

        // Pantalla LCD
        glPushMatrix();
            glTranslatef(0.0, 0.8, 0.61);  // Posición frontal
            glScalef(0.2, 0.3, 0.01);      // Aplanado
            glColor3f(0.0, 0.5, 1.0);      // Color azul
            glutSolidCube(1.0);
        glPopMatrix();

        // Botones (3 en vertical)
        for (int i = 0; i < 3; ++i) {
            glPushMatrix();
                glTranslatef(0.0, 0.4 - i * 0.15, 0.61);  // Espaciado vertical
                glScalef(0.1, 0.05, 0.01);                // Forma aplanada
                glColor3f(0.1, 0.1, 0.1);                 // Color oscuro
                glutSolidCube(1.0);
            glPopMatrix();
        }

         glPushMatrix();
            glTranslatef(0.0, 0.2, 0.61);  // Posición frontal
            glScalef(0.5, 2.0, 0.01);      // Aplanado
            glColor3f(0.0, 0.0, 0.0);    
            glutSolidCube(1.0);
        glPopMatrix();
        


    glPopMatrix();
   
    
}

// Función principal de renderizado
void drawImage() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);  // Limpiar buffers
    glLoadIdentity();  // Reiniciar matriz de vista

    // Configurar cámara
    gluLookAt(0.0, 1.0, 6.0,  // Posición del ojo
              0.0, 1.0, 0.0,  // Punto de mira
              0.0, 1.0, 0.0); // Vector arriba

    // Dibujar vape con transformaciones
    glPushMatrix();
        glTranslatef(vapePosX, vapePosY, vapePosZ);  // Posición actual
        glRotatef(vapeAngle, 1, 0, 0);               // Rotación por inhalación
        drawVape();                                  // Dibujar modelo 3D
    glPopMatrix();
    
    drawSmoke();  // Dibujar todos los efectos de humo

    glutSwapBuffers();  // Intercambiar buffers (doble buffer)
}

// Manejo de redimensionado de ventana
void reshapeWindow(int width, int height) {
    glViewport(0, 0, (GLsizei)width, (GLsizei)height);  // Área de dibujo
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    // Configurar perspectiva (campo de visión 45°, relación aspecto, planos near/far)
    gluPerspective(45, (float)width/height, 0.1, 100.0);
    glMatrixMode(GL_MODELVIEW);
}

// Función principal
int main(int argc, char **argv) {
    glutInit(&argc, argv);
    // Configuración inicial de GLUT
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);  // Doble buffer, RGB, profundidad
    glutInitWindowSize(800, 600);  // Tamaño inicial
    glutCreateWindow("Simulador de Vapeo (I=Inhalar, E=Exhalar)");  // Título

    initialize();      // Configuración OpenGL
    // Registrar callbacks
    glutDisplayFunc(drawImage);      // Función de renderizado
    glutReshapeFunc(reshapeWindow);  // Redimensionado
    glutKeyboardFunc(handleKey);     // Teclado
    glutIdleFunc(updateAnimation);   // Animación en tiempo de inactividad

    glutMainLoop();  // Bucle principal
    return 0;
}