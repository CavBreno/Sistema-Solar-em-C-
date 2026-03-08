#include <GL/glut.h> //biblioteca problemática
#include <math.h>
#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

const float PI = 3.1415926535f;

// Controle da Câmera
float angleAlpha = 0.0f;
float angleBeta  = 0.8f;
float radius     = 55.0f;

float camTargetX = 0.0f;
float camTargetY = 0.0f;
float camTargetZ = 0.0f;

// Controle do Mouse
bool isDragging = false;
int lastMouseX, lastMouseY;

// Estrutura dos Planetas
struct Planet {
    float radius;
    float distance;
    float r, g, b;       // Cor de fallback
    float tilt;
    const char* texFile;
};

Planet planets[] = {
    {0.4f,  6.0f,  0.6f, 0.6f, 0.6f,  7.0f, "textures/mercury.jpg"},
    {0.7f,  9.0f,  0.9f, 0.6f, 0.2f,  3.0f, "textures/venus.jpg"},
    {0.8f, 12.0f,  0.2f, 0.5f, 1.0f,  0.0f, "textures/earth.jpg"},
    {0.5f, 15.0f,  1.0f, 0.3f, 0.2f,  2.0f, "textures/mars.jpg"},
    {1.8f, 22.0f,  0.8f, 0.6f, 0.4f,  1.0f, "textures/jupiter.jpg"},
    {1.5f, 28.0f,  0.9f, 0.8f, 0.5f,  1.5f, "textures/saturn.jpg"},
    {1.1f, 34.0f,  0.5f, 0.8f, 1.0f,  0.5f, "textures/uranus.jpg"},
    {1.0f, 40.0f,  0.2f, 0.4f, 1.0f,  0.3f, "textures/neptune.jpg"}
};

int numPlanets = sizeof(planets) / sizeof(Planet);

// Índices de Textura
// 0-7: planetas | 8: sol | 9: lua | 10: estrelas | 11: anel de saturno
GLuint textures[12];

// Carrega Textura
GLuint loadTexture(const char* filename, bool hasAlpha = false) {
    int w, h, channels;
    int forceChannels = hasAlpha ? 4 : 3;
    unsigned char* data = stbi_load(filename, &w, &h, &channels, forceChannels);

    GLuint texID = 0;
    glGenTextures(1, &texID);
    glBindTexture(GL_TEXTURE_2D, texID);

    if (data) {
        GLenum format = hasAlpha ? GL_RGBA : GL_RGB;
        glTexImage2D(GL_TEXTURE_2D, 0, format, w, h, 0, format, GL_UNSIGNED_BYTE, data);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        stbi_image_free(data);
        std::cout << "Textura carregada: " << filename << " (" << w << "x" << h << ")\n";
    } else {
        std::cout << "AVISO: Nao foi possivel carregar " << filename << ". Usando cor padrao.\n";
        unsigned char white[4] = {255, 255, 255, 255};
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, white);
    }

    glBindTexture(GL_TEXTURE_2D, 0);
    return texID;
}

// Configuração Inicial
void init() {
    glClearColor(0.02f, 0.02f, 0.05f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_TEXTURE_2D);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    GLfloat lightPos[]     = {0.0f, 0.0f, 0.0f, 1.0f};
    GLfloat ambientLight[] = {0.15f, 0.15f, 0.15f, 1.0f};
    GLfloat diffuseLight[] = {1.0f, 1.0f, 0.95f, 1.0f};
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
    glLightfv(GL_LIGHT0, GL_AMBIENT,  ambientLight);
    glLightfv(GL_LIGHT0, GL_DIFFUSE,  diffuseLight);

    // Planetas
    for (int i = 0; i < numPlanets; i++)
        textures[i] = loadTexture(planets[i].texFile);

    // Extras
    textures[8]  = loadTexture("textures/sun.jpg");
    textures[9]  = loadTexture("textures/moon.jpg");
    textures[10] = loadTexture("textures/stars.jpg");
    textures[11] = loadTexture("textures/saturn_ring.png", true); // PNG com canal alpha
}

// Câmera
void updateCamera() {
    if (angleBeta >  1.5f) angleBeta =  1.5f;
    if (angleBeta < -1.5f) angleBeta = -1.5f;

    float offsetX = radius * cos(angleBeta) * sin(angleAlpha);
    float offsetY = radius * sin(angleBeta);
    float offsetZ = radius * cos(angleBeta) * cos(angleAlpha);

    gluLookAt(camTargetX + offsetX, camTargetY + offsetY, camTargetZ + offsetZ,
              camTargetX, camTargetY, camTargetZ,
              0.0f, 1.0f, 0.0f);
}

// Teclado (setas)
void specialKeys(int key, int x, int y) {
    float speed  = radius * 0.02f;
    float rightX = cos(angleAlpha);
    float rightZ = -sin(angleAlpha);

    switch (key) {
        case GLUT_KEY_UP:    camTargetY += speed; break;
        case GLUT_KEY_DOWN:  camTargetY -= speed; break;
        case GLUT_KEY_RIGHT: camTargetX += rightX * speed; camTargetZ += rightZ * speed; break;
        case GLUT_KEY_LEFT:  camTargetX -= rightX * speed; camTargetZ -= rightZ * speed; break;
    }
    glutPostRedisplay();
}

// Teclado (WASD, ESC, R) 
void keyboard(unsigned char key, int x, int y) {
    float speed  = radius * 0.02f;
    float rightX = cos(angleAlpha);
    float rightZ = -sin(angleAlpha);

    switch (key) {
        case 27:  exit(0); break;
        case 'w': case 'W': camTargetY += speed; break;
        case 's': case 'S': camTargetY -= speed; break;
        case 'd': case 'D': camTargetX += rightX * speed; camTargetZ += rightZ * speed; break;
        case 'a': case 'A': camTargetX -= rightX * speed; camTargetZ -= rightZ * speed; break;
        case 'r': case 'R':
            angleAlpha = 0.0f; angleBeta = 0.8f; radius = 55.0f;
            camTargetX = camTargetY = camTargetZ = 0.0f;
            break;
    }
    glutPostRedisplay();
}

// Mouse
void mouseMotion(int x, int y) {
    if (isDragging) {

        float dx = (x - lastMouseX);
        float dy = (y - lastMouseY);

        float panSpeed = radius * 0.002f;

        float rightX = cos(angleAlpha);
        float rightZ = -sin(angleAlpha);

        float forwardX = sin(angleAlpha);
        float forwardZ = cos(angleAlpha);

        camTargetX -= rightX * dx * panSpeed;
        camTargetZ -= rightZ * dx * panSpeed;

        camTargetY += dy * panSpeed;

        lastMouseX = x;
        lastMouseY = y;

        glutPostRedisplay();
    }
}

void mouseClick(int button, int state, int x, int y) {

    if (button == GLUT_LEFT_BUTTON) {

        if (state == GLUT_DOWN) {
            isDragging = true;
            lastMouseX = x;
            lastMouseY = y;
        }
        else if (state == GLUT_UP) {
            isDragging = false;
        }
    }

    // Zoom com scroll
    if (button == 3) { // scroll up
        radius *= 0.9f;
    }
    if (button == 4) { // scroll down
        radius *= 1.1f;
    }

    glutPostRedisplay();
}



// Animação
float orbitAngle[8] = {0};      // translação ao redor do sol
float rotationAngle[8] = {0};   // rotação do planeta

// Velocidades aproximadas
float orbitSpeed[8] = {
    4.7f, 3.5f, 3.0f, 2.4f,
    1.3f, 1.0f, 0.7f, 0.5f
};

float rotationSpeed[8] = {
    6.0f, 4.0f, 7.0f, 5.0f,
    12.0f, 10.0f, 8.0f, 7.0f
};

// Lua
float moonOrbit = 0.0f;
float moonSpeed = 8.0f;

// Atualiza Animação
void updateAnimation(int value) {

    for (int i = 0; i < numPlanets; i++) {
        orbitAngle[i] += orbitSpeed[i] * 0.02f;
        rotationAngle[i] += rotationSpeed[i] * 0.5f;

        if (orbitAngle[i] > 360) orbitAngle[i] -= 360;
        if (rotationAngle[i] > 360) rotationAngle[i] -= 360;
    }

    moonOrbit += moonSpeed * 0.05f;
    if (moonOrbit > 360) moonOrbit -= 360;

    glutPostRedisplay();

    // chama novamente após 16ms (~60 FPS)
    glutTimerFunc(16, updateAnimation, 0);
}

// Esfera Texturizada
void drawTexturedSphere(float r, int stacks, int slices) {
    for (int i = 0; i < stacks; i++) {
        float phi0 = PI * (-0.5f + (float)i       / stacks);
        float phi1 = PI * (-0.5f + (float)(i + 1) / stacks);

        float cosPhi0 = cos(phi0), sinPhi0 = sin(phi0);
        float cosPhi1 = cos(phi1), sinPhi1 = sin(phi1);

        glBegin(GL_TRIANGLE_STRIP);
        for (int j = 0; j <= slices; j++) {
            float theta    = 2.0f * PI * (float)j / slices;
            float cosTheta = cos(theta), sinTheta = sin(theta);
            float u = 1.0f - ((float)j / slices);

            glTexCoord2f(u, 1.0f - (float)(i + 1) / stacks);
            glNormal3f(cosTheta * cosPhi1, sinPhi1, sinTheta * cosPhi1);
            glVertex3f(r * cosTheta * cosPhi1, r * sinPhi1, r * sinTheta * cosPhi1);

            glTexCoord2f(u, 1.0f - (float)i / stacks);
            glNormal3f(cosTheta * cosPhi0, sinPhi0, sinTheta * cosPhi0);
            glVertex3f(r * cosTheta * cosPhi0, r * sinPhi0, r * sinTheta * cosPhi0);
        }
        glEnd();
    }
}

// Fundo Estrelado
void drawStarBackground() {
    float skyRadius = 2000.0f;

    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);

    glBindTexture(GL_TEXTURE_2D, textures[10]);
    glColor3f(1.0f, 1.0f, 1.0f);

    // Esfera invertida (normais para dentro)
    for (int i = 0; i < 40; i++) {
        float phi0 = PI * (-0.5f + (float)i       / 40);
        float phi1 = PI * (-0.5f + (float)(i + 1) / 40);

        float cosPhi0 = cos(phi0), sinPhi0 = sin(phi0);
        float cosPhi1 = cos(phi1), sinPhi1 = sin(phi1);

        glBegin(GL_TRIANGLE_STRIP);
        for (int j = 0; j <= 40; j++) {
            float theta    = 2.0f * PI * (float)j / 40;
            float cosTheta = cos(theta), sinTheta = sin(theta);
            float u = (float)j / 40;

            glTexCoord2f(u, (float)(i + 1) / 40);
            glNormal3f(-cosTheta * cosPhi1, -sinPhi1, -sinTheta * cosPhi1);
            glVertex3f(skyRadius * cosTheta * cosPhi1, skyRadius * sinPhi1, skyRadius * sinTheta * cosPhi1);

            glTexCoord2f(u, (float)i / 40);
            glNormal3f(-cosTheta * cosPhi0, -sinPhi0, -sinTheta * cosPhi0);
            glVertex3f(skyRadius * cosTheta * cosPhi0, skyRadius * sinPhi0, skyRadius * sinTheta * cosPhi0);
        }
        glEnd();
    }

    glBindTexture(GL_TEXTURE_2D, 0);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
}

// Lua
void drawMoon(float earthRadius) {
    float moonDist   = earthRadius * 1.8f;
    float moonRadius = 0.22f;

    glPushMatrix();

        glRotatef(moonOrbit, 0.0f, 1.0f, 0.0f); // órbita
        glTranslatef(moonDist, 0.0f, 0.0f);

        glBindTexture(GL_TEXTURE_2D, textures[9]);
        glColor3f(1.0f, 1.0f, 1.0f);
        drawTexturedSphere(moonRadius, 20, 20);
        glBindTexture(GL_TEXTURE_2D, 0);

    glPopMatrix();
}

// Anel de Saturno
void drawSaturnRing(float innerRadius, float outerRadius) {
    int segments = 180;

    glEnable(GL_BLEND);
    glDisable(GL_LIGHTING);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, textures[11]);
    glColor4f(1.0f, 1.0f, 1.0f, 0.9f);

    glRotatef(20.0f, 1.0f, 0.0f, 0.0f); // inclinação do anel

    glBegin(GL_TRIANGLE_STRIP);
    for (int i = 0; i <= segments; i++) {
        float angle = 2.0f * PI * i / segments;
        float cosA  = cos(angle);
        float sinA  = sin(angle);
        float u     = (float)i / segments;

        glTexCoord2f(u, 1.0f);
        glVertex3f(outerRadius * cosA, 0.0f, outerRadius * sinA);

        glTexCoord2f(u, 0.0f);
        glVertex3f(innerRadius * cosA, 0.0f, innerRadius * sinA);
    }
    glEnd();

    glBindTexture(GL_TEXTURE_2D, 0);
    glEnable(GL_LIGHTING);
    glDisable(GL_BLEND);
}

// Órbita
void drawOrbit(float distance) {
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    glColor3f(0.3f, 0.3f, 0.3f);
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < 360; i++) {
        float rad = i * PI / 180.0f;
        glVertex3f(cos(rad) * distance, 0.0f, sin(rad) * distance);
    }
    glEnd();
    glEnable(GL_LIGHTING);
    glEnable(GL_TEXTURE_2D);
}

// Sistema Solar
void drawSolarSystem() {
    // ----- FUNDO ESTRELADO -----
    drawStarBackground();

    // ----- SOL -----
    glPushMatrix();
        GLfloat emission[]   = {1.0f, 0.7f, 0.0f, 1.0f};
        GLfloat noEmission[] = {0.0f, 0.0f, 0.0f, 1.0f};
        glMaterialfv(GL_FRONT, GL_EMISSION, emission);

        glBindTexture(GL_TEXTURE_2D, textures[8]);
        glColor3f(1.0f, 1.0f, 1.0f);
        drawTexturedSphere(3.0f, 60, 60);
        glBindTexture(GL_TEXTURE_2D, 0);

        glMaterialfv(GL_FRONT, GL_EMISSION, noEmission);
    glPopMatrix();

    // ----- PLANETAS -----
    for (int i = 0; i < numPlanets; i++) {
        float dist = planets[i].distance;

        // Órbita
        glPushMatrix();
            glRotatef(planets[i].tilt, 0.0f, 0.0f, 1.0f);
            drawOrbit(dist);
        glPopMatrix();

        // Planeta
        glPushMatrix();
            glRotatef(planets[i].tilt, 0.0f, 0.0f, 1.0f);
            glRotatef(orbitAngle[i], 0.0f, 1.0f, 0.0f);
            glTranslatef(dist, 0.0f, 0.0f);

            glRotatef(rotationAngle[i], 0.0f, 1.0f, 0.0f); // rotação planeta

            glBindTexture(GL_TEXTURE_2D, textures[i]);
            glColor3f(1.0f, 1.0f, 1.0f);
            drawTexturedSphere(planets[i].radius, 40, 40);
            glBindTexture(GL_TEXTURE_2D, 0);

            // Lua orbita a Terra (índice 2)
            if (i == 2) {
                drawMoon(planets[i].radius);
            }

            // Anel de Saturno (índice 5)
            if (i == 5) {
                drawSaturnRing(planets[i].radius * 1.3f, planets[i].radius * 2.4f);
            }
        glPopMatrix();
    }
}

// Display
void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    updateCamera();
    drawSolarSystem();

    glutSwapBuffers();
}

// Reshape
void reshape(int w, int h) {
    if (h == 0) h = 1;
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0f, (float)w / h, 0.1f, 5000.0f);
    glMatrixMode(GL_MODELVIEW);
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
    glutCreateWindow("Sistema Solar 3D - Modelagem");

    init();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutMouseFunc(mouseClick);
    glutMotionFunc(mouseMotion);
    glutSpecialFunc(specialKeys);
    glutKeyboardFunc(keyboard);
    glutTimerFunc(16, updateAnimation, 0);

    glutMainLoop();
    return 0;
}
