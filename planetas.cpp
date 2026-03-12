#include <GL/glut.h> 
#include <math.h>
#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

const float PI = 3.1415926535f;

// Controle da Câmera
float anguloHorizontal = 0.0f;
float anguloVertical  = 0.8f;
float distanciaCamera = 55.0f;

float alvoCameraX = 0.0f;
float alvoCameraY = 0.0f;
float alvoCameraZ = 0.0f;

// Controle do Mouse
bool estaArrastando = false;
int ultimoMouseX, ultimoMouseY;

// Estrutura dos Planetas
struct Planeta {
    float raio;
    float distanciaOrbital;
    float r, g, b;       // Cor de fallback
    float inclinacao;
    const char* arquivoTextura;
};

Planeta planetas[] = {
    {0.4f,  6.0f,  0.6f, 0.6f, 0.6f,  7.0f, "textures/mercury.jpg"},
    {0.7f,  9.0f,  0.9f, 0.6f, 0.2f,  3.0f, "textures/venus.jpg"},
    {0.8f, 12.0f,  0.2f, 0.5f, 1.0f,  0.0f, "textures/earth.jpg"},
    {0.5f, 15.0f,  1.0f, 0.3f, 0.2f,  2.0f, "textures/mars.jpg"},
    {1.8f, 22.0f,  0.8f, 0.6f, 0.4f,  1.0f, "textures/jupiter.jpg"},
    {1.5f, 28.0f,  0.9f, 0.8f, 0.5f,  1.5f, "textures/saturn.jpg"},
    {1.1f, 34.0f,  0.5f, 0.8f, 1.0f,  0.5f, "textures/uranus.jpg"},
    {1.0f, 40.0f,  0.2f, 0.4f, 1.0f,  0.3f, "textures/neptune.jpg"}
};

int numPlanetas = sizeof(planetas) / sizeof(Planeta);

// Índices de Textura
// 0-7: planetas | 8: sol | 9: lua | 10: estrelas | 11: anel de saturno
GLuint texturas[12];

// Carrega Textura
GLuint carregarTextura(const char* caminhoArquivo, bool temAlpha = false) {
    int largura, altura, canais;
    int canaisForcados = temAlpha ? 4 : 3;
    unsigned char* dados = stbi_load(caminhoArquivo, &largura, &altura, &canais, canaisForcados);

    GLuint idTextura = 0;
    glGenTextures(1, &idTextura);
    glBindTexture(GL_TEXTURE_2D, idTextura);

    if (dados) {
        GLenum format = temAlpha ? GL_RGBA : GL_RGB;
        glTexImage2D(GL_TEXTURE_2D, 0, format, largura, altura, 0, format, GL_UNSIGNED_BYTE, dados);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        stbi_image_free(dados);
        std::cout << "Textura carregada: " << caminhoArquivo << " (" << largura << "x" << altura << ")\n";
    } else {
        std::cout << "AVISO: Nao foi possivel carregar " << caminhoArquivo << ". Usando cor padrao.\n";
        unsigned char white[4] = {255, 255, 255, 255};
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, white);
    }

    glBindTexture(GL_TEXTURE_2D, 0);
    return idTextura;
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

    GLfloat posicaoLuz[] = {0.0f, 0.0f, 0.0f, 1.0f};
    GLfloat luzAmbiente[] = {0.15f, 0.15f, 0.15f, 1.0f};
    GLfloat luzDifusa[] = {1.0f, 1.0f, 0.95f, 1.0f};
    glLightfv(GL_LIGHT0, GL_POSITION, posicaoLuz);
    glLightfv(GL_LIGHT0, GL_AMBIENT,  luzAmbiente);
    glLightfv(GL_LIGHT0, GL_DIFFUSE,  luzDifusa);

    // Planetas
    for (int i = 0; i < numPlanetas; i++)
        texturas[i] = carregarTextura(planetas[i].arquivoTextura);

    // Extras
    texturas[8]  = carregarTextura("textures/sun.jpg");
    texturas[9]  = carregarTextura("textures/moon.jpg");
    texturas[10] = carregarTextura("textures/stars.jpg");
    texturas[11] = carregarTextura("textures/saturn_ring.png", true); // PNG com canal alpha
}

// Câmera
void updateCamera() {
    if (anguloVertical >  1.5f)  {
        anguloVertical =  1.5f;
    } 

    if (anguloVertical < -1.5f) {
        anguloVertical = -1.5f;
    } 

    float deslocamentoX = distanciaCamera * cos(anguloVertical) * sin(anguloHorizontal);
    float deslocamentoY = distanciaCamera * sin(anguloVertical);
    float deslocamentoZ = distanciaCamera * cos(anguloVertical) * cos(anguloHorizontal);

    gluLookAt(alvoCameraX + deslocamentoX, alvoCameraY + deslocamentoY, alvoCameraZ + deslocamentoZ,
              alvoCameraX, alvoCameraY, alvoCameraZ,
              0.0f, 1.0f, 0.0f);
}

// Teclado (setas)
void setas(int tecla, int x, int y) {
    float velocidade  = distanciaCamera * 0.02f;
    float direitaX = cos(anguloHorizontal);
    float direitaZ = -sin(anguloHorizontal);

    switch (tecla) {
        case GLUT_KEY_UP:    alvoCameraY += velocidade; break;
        case GLUT_KEY_DOWN:  alvoCameraY -= velocidade; break;
        case GLUT_KEY_RIGHT: alvoCameraX += direitaX * velocidade; alvoCameraZ += direitaZ * velocidade; break;
        case GLUT_KEY_LEFT:  alvoCameraX -= direitaX * velocidade; alvoCameraZ -= direitaZ * velocidade; break;
    }
    glutPostRedisplay();
}

// Teclado (WASD, ESC, R) 
void teclado(unsigned char tecla, int x, int y) {
    float velocidade  = distanciaCamera * 0.02f;
    float direitaX = cos(anguloHorizontal);
    float direitaZ = -sin(anguloHorizontal);

    switch (tecla) {
        case 27:  exit(0); break;
        case 'w': case 'W': alvoCameraY += velocidade; break;
        case 's': case 'S': alvoCameraY -= velocidade; break;
        case 'd': case 'D': alvoCameraX += direitaX * velocidade; alvoCameraZ += direitaZ * velocidade; break;
        case 'a': case 'A': alvoCameraX -= direitaX * velocidade; alvoCameraZ -= direitaZ * velocidade; break;
        case 'r': case 'R':
            anguloHorizontal = 0.0f; anguloVertical = 0.8f; distanciaCamera = 55.0f;
            alvoCameraX = alvoCameraY = alvoCameraZ = 0.0f;
            break;
    }
    glutPostRedisplay();
}

// Mouse
void movimentoMouse(int x, int y) {
    if (estaArrastando) {
        float dx = (x - ultimoMouseX);
        float dy = (y - ultimoMouseY);

        float velocidadeRotacao = 0.005f;

        anguloHorizontal -= dx * velocidadeRotacao;
        anguloVertical  += dy * velocidadeRotacao;

        ultimoMouseX = x;
        ultimoMouseY = y;

        glutPostRedisplay();
    }
}

void mouseClick(int botao, int estado, int x, int y) {

    if (botao == GLUT_LEFT_BUTTON) {

        if (estado == GLUT_DOWN) {
            estaArrastando = true;
            ultimoMouseX = x;
            ultimoMouseY = y;
        }
        else if (estado == GLUT_UP) {
            estaArrastando = false;
        }
    }

    // Zoom com scroll
    if (botao == 3) { // scroll up
        distanciaCamera *= 0.9f;
    }
    if (botao == 4) { // scroll down
        distanciaCamera *= 1.1f;
    }

    glutPostRedisplay();
}

// Animação
float anguloOrbita[8] = {0};      // translação ao redor do sol
float anguloRotacao[8] = {0};   // rotação do planeta

// Velocidades aproximadas
float velocidadeOrbita[8] = {
    4.7f, 3.5f, 3.0f, 2.4f,
    1.3f, 1.0f, 0.7f, 0.5f
};

float velocidadeRotacao[8] = {
    6.0f, 4.0f, 7.0f, 5.0f,
    12.0f, 10.0f, 8.0f, 7.0f
};

// Lua
float anguloOrbitaLua = 0.0f;
float velocidadeOrbitaLua = 8.0f;

// Atualiza Animação
void atualizarAnimacao(int valor) {

    for (int i = 0; i < numPlanetas; i++) {
        anguloOrbita[i] += velocidadeOrbita[i] * 0.02f;
        anguloRotacao[i] += velocidadeRotacao[i] * 0.5f;

        if (anguloOrbita[i] > 360) {
            anguloOrbita[i] -= 360;
        } 

        if (anguloRotacao[i] > 360) {
            anguloRotacao[i] -= 360;
        } 
    }

    anguloOrbitaLua += velocidadeOrbitaLua * 0.05f;
    if (anguloOrbitaLua > 360) {
        anguloOrbitaLua -= 360;
    } 

    glutPostRedisplay();

    glutTimerFunc(16, atualizarAnimacao, 0);
}

// Esfera Texturizada
void desenharEsferaTexturizada(float raio, int pilhas, int fatias) {
    for (int i = 0; i < pilhas; i++) {
        float phi0 = PI * (-0.5f + (float)i       / pilhas);
        float phi1 = PI * (-0.5f + (float)(i + 1) / pilhas);

        float cosPhi0 = cos(phi0), sinPhi0 = sin(phi0);
        float cosPhi1 = cos(phi1), sinPhi1 = sin(phi1);

        glBegin(GL_TRIANGLE_STRIP);
        for (int j = 0; j <= fatias; j++) {
            float theta    = 2.0f * PI * (float)j / fatias;
            float cosTheta = cos(theta), sinTheta = sin(theta);
            float u = 1.0f - ((float)j / fatias);

            glTexCoord2f(u, 1.0f - (float)(i + 1) / pilhas);
            glNormal3f(cosTheta * cosPhi1, sinPhi1, sinTheta * cosPhi1);
            glVertex3f(raio * cosTheta * cosPhi1, raio * sinPhi1, raio * sinTheta * cosPhi1);

            glTexCoord2f(u, 1.0f - (float)i / pilhas);
            glNormal3f(cosTheta * cosPhi0, sinPhi0, sinTheta * cosPhi0);
            glVertex3f(raio * cosTheta * cosPhi0, raio * sinPhi0, raio * sinTheta * cosPhi0);
        }
        glEnd();
    }
}

// Fundo Estrelado
void desenharFundoEstrelado() {
    float raioCeu = 2000.0f;

    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);

    glBindTexture(GL_TEXTURE_2D, texturas[10]);
    glColor3f(1.0f, 1.0f, 1.0f);

    // Esfera invertida (normais para dentro)
    for (int i = 0; i < 40; i++) {
        float phi0 = PI * (-0.5f + (float)i / 40);
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
            glVertex3f(raioCeu * cosTheta * cosPhi1, raioCeu * sinPhi1, raioCeu * sinTheta * cosPhi1);

            glTexCoord2f(u, (float)i / 40);
            glNormal3f(-cosTheta * cosPhi0, -sinPhi0, -sinTheta * cosPhi0);
            glVertex3f(raioCeu * cosTheta * cosPhi0, raioCeu * sinPhi0, raioCeu * sinTheta * cosPhi0);
        }
        glEnd();
    }

    glBindTexture(GL_TEXTURE_2D, 0);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
}

// Lua
void desenharLua(float raioTerra) {
    float distanciaLua = raioTerra * 1.8f;
    float raioLua = 0.22f;

    glPushMatrix();

        glRotatef(anguloOrbitaLua, 0.0f, 1.0f, 0.0f); // órbita
        glTranslatef(distanciaLua, 0.0f, 0.0f);

        glBindTexture(GL_TEXTURE_2D, texturas[9]);
        glColor3f(1.0f, 1.0f, 1.0f);
        desenharEsferaTexturizada(raioLua, 20, 20);
        glBindTexture(GL_TEXTURE_2D, 0);

    glPopMatrix();
}

// Anel de Saturno
void desenharAnelSaturno(float raioInterno, float raioExterno) {
    int segmentos = 180;

    glEnable(GL_BLEND);
    glDisable(GL_LIGHTING);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texturas[11]);
    glColor4f(1.0f, 1.0f, 1.0f, 0.9f);

    glRotatef(20.0f, 1.0f, 0.0f, 0.0f); // inclinação do anel

    glBegin(GL_TRIANGLE_STRIP);
    for (int i = 0; i <= segmentos; i++) {
        float angle = 2.0f * PI * i / segmentos;
        float cosA  = cos(angle);
        float sinA  = sin(angle);
        float u     = (float)i / segmentos;

        glTexCoord2f(u, 1.0f);
        glVertex3f(raioExterno * cosA, 0.0f, raioExterno * sinA);

        glTexCoord2f(u, 0.0f);
        glVertex3f(raioInterno * cosA, 0.0f, raioInterno * sinA);
    }
    glEnd();

    glBindTexture(GL_TEXTURE_2D, 0);
    glEnable(GL_LIGHTING);
    glDisable(GL_BLEND);
}

// Órbita
void desenharOrbita(float distanciaOrbital) {
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    glColor3f(0.3f, 0.3f, 0.3f);
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < 360; i++) {
        float rad = i * PI / 180.0f;
        glVertex3f(cos(rad) * distanciaOrbital, 0.0f, sin(rad) * distanciaOrbital);
    }
    glEnd();
    glEnable(GL_LIGHTING);
    glEnable(GL_TEXTURE_2D);
}

// Sistema Solar
void desenharSistemaSolar() {
    // Fundo
    desenharFundoEstrelado();

    // Sol
    glPushMatrix();
        GLfloat emissao[]   = {1.0f, 0.7f, 0.0f, 1.0f};
        GLfloat semEmissao[] = {0.0f, 0.0f, 0.0f, 1.0f};
        glMaterialfv(GL_FRONT, GL_EMISSION, emissao);

        glBindTexture(GL_TEXTURE_2D, texturas[8]);
        glColor3f(1.0f, 1.0f, 1.0f);
        desenharEsferaTexturizada(3.0f, 60, 60);
        glBindTexture(GL_TEXTURE_2D, 0);

        glMaterialfv(GL_FRONT, GL_EMISSION, semEmissao);
    glPopMatrix();

    // planetas
    for (int i = 0; i < numPlanetas; i++) {
        float distanciaOrbital = planetas[i].distanciaOrbital;

        // Órbita
        glPushMatrix();
            glRotatef(planetas[i].inclinacao, 0.0f, 0.0f, 1.0f);
            desenharOrbita(distanciaOrbital);
        glPopMatrix();

        // Planeta
        glPushMatrix();
            glRotatef(planetas[i].inclinacao, 0.0f, 0.0f, 1.0f);
            glRotatef(anguloOrbita[i], 0.0f, 1.0f, 0.0f);
            glTranslatef(distanciaOrbital, 0.0f, 0.0f);

            glRotatef(anguloRotacao[i], 0.0f, 1.0f, 0.0f); // rotação planeta

            glBindTexture(GL_TEXTURE_2D, texturas[i]);
            glColor3f(1.0f, 1.0f, 1.0f);
            desenharEsferaTexturizada(planetas[i].raio, 40, 40);
            glBindTexture(GL_TEXTURE_2D, 0);

            // Lua orbita a Terra 
            if (i == 2) {
                desenharLua(planetas[i].raio);
            }

            // Anel de Saturno 
            if (i == 5) {
                desenharAnelSaturno(planetas[i].raio * 1.3f, planetas[i].raio * 2.4f);
            }
        glPopMatrix();
    }
}

// Display
void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    updateCamera();
    desenharSistemaSolar();

    glutSwapBuffers();
}

// Reshape
void reshape(int largura, int altura) {
    if (altura == 0) {
        altura = 1;
    }
    glViewport(0, 0, largura, altura);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0f, (float)largura / altura, 0.1f, 5000.0f);
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
    glutMotionFunc(movimentoMouse);
    glutSpecialFunc(setas);
    glutKeyboardFunc(teclado);
    glutTimerFunc(16, atualizarAnimacao, 0);

    glutMainLoop();
    return 0;
}
