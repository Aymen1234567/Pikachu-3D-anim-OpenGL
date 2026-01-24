/****************************************************************************************/
/* pikachu3D_axes.cpp - VERSION CORRIGÉE AVEC BRAS + MAINS + DOIGTS ATTACHÉS */
/****************************************************************************************/

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/freeglut.h>
#endif
#include <algorithm>  // pour std::swap
#include <cmath>
#include <cstdio>       // pour FILE*, fopen, etc.
#include <cstdlib>      // pour exit()
#include <jpeglib.h>    // pour struct jpeg_decompress_struct et jpeg_stdio_src()
#include <jerror.h>     // parfois utile avec certaines versions



#define MAX_LARG 256
#define MAX_HAUT 256

unsigned char image[MAX_LARG * MAX_HAUT * 3];
unsigned char texture[MAX_LARG][MAX_HAUT][3];
//pelouse
unsigned char image2[4500 * 4500 * 3];
unsigned char pelouse[4500][4500][3];
int largimg, hautimg;
GLuint texCorps;


int anglex = 20, angley = -30, xold, yold;
char presse = 0;


// Couleurs
#define YELLOW 246/255.0f, 208/255.0f, 47/255.0f
#define BLACK 0.0f, 0.0f, 0.0f
#define RED 221/255.0f, 77/255.0f, 40/255.0f
#define DARKRED 136/255.0f, 20/255.0f, 29/255.0f
#define PINK 221/255.0f, 113/255.0f, 111/255.0f
#define BROWN 146/255.0f, 62/255.0f, 36/255.0f
#define WHITE 1.0f, 1.0f, 1.0f




float Lbras = 2.0f;
float Rbras = 0.3f;




void solidSphere(float radius, int slices, int stacks, float r, float g, float b) {
    glColor3f(r, g, b);
    glutSolidSphere(radius, slices, stacks);
}

// === AXES X, Y, Z BIEN VISIBLES ===
void drawAxes() {
    glLineWidth(4.0f);
    // X rouge
    glColor3f(1.0f, 0.0f, 0.0f);
    glBegin(GL_LINES);
        glVertex3f(-40.0f, 0.0f, 0.0f);
        glVertex3f( 40.0f, 0.0f, 0.0f);
    glEnd();
    glRasterPos3f(10.3f, 0.1f, 0.0f);
    glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, 'X');
    // Y vert
    glColor3f(0.0f, 1.0f, 0.0f);
    glBegin(GL_LINES);
        glVertex3f(0.0f, -40.0f, 0.0f);
        glVertex3f(0.0f, 40.0f, 0.0f);
    glEnd();
    glRasterPos3f(0.1f, 10.3f, 0.0f);
    glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, 'Y');
    // Z bleu
    glColor3f(0.0f, 0.0f, 1.0f);
    glBegin(GL_LINES);
        glVertex3f(0.0f, 0.0f, -40.0f);
        glVertex3f(0.0f, 0.0f, 40.0f);
    glEnd();
    glRasterPos3f(0.0f, 0.1f, 10.3f);
    glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, 'Z');
    glLineWidth(1.0f);
}

void drawSphere(float r, int slices, int stacks, float R, float G, float B) {
    glColor3f(R, G, B);
    for (int i = 0; i < stacks; ++i) {
        float phi1 = M_PI * i / stacks;
        float phi2 = M_PI * (i + 1) / stacks;
        glBegin(GL_TRIANGLE_STRIP);
        for (int j = 0; j <= slices; ++j) {
            float theta = 2 * M_PI * j / slices;
            float x1 = r * sin(phi1) * cos(theta);
            float y1 = r * cos(phi1);
            float z1 = r * sin(phi1) * sin(theta);
            float x2 = r * sin(phi2) * cos(theta);
            float y2 = r * cos(phi2);
            float z2 = r * sin(phi2) * sin(theta);
            glNormal3f(x1 / r, y1 / r, z1 / r);
            glVertex3f(x1, y1, z1);
            glNormal3f(x2 / r, y2 / r, z2 / r);
            glVertex3f(x2, y2, z2);
        }
        glEnd();
    }
}


// -------------------- Lecture JPEG --------------------
void loadJpegImage(const char *fichier)
{
    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr jerr;
    FILE *file;
    unsigned char *ligne;

    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_decompress(&cinfo);

#ifdef __WIN32
    if (fopen_s(&file, fichier, "rb") != 0)
#else
    if ((file = fopen(fichier, "rb")) == 0)
#endif
    {
        fprintf(stderr, "Erreur : impossible d'ouvrir %s\n", fichier);
        exit(1);
    }

    jpeg_stdio_src(&cinfo, file);
    jpeg_read_header(&cinfo, TRUE);

    largimg = cinfo.image_width;
    hautimg = cinfo.image_height;

    if (cinfo.jpeg_color_space == JCS_GRAYSCALE)
    {
        fprintf(stdout, "Erreur : l'image doit être RGB\n");
        exit(1);
    }

    if (largimg > MAX_LARG || hautimg > MAX_HAUT)
    {
        fprintf(stderr, "Erreur : image trop grande (max %dx%d)\n", MAX_LARG, MAX_HAUT);
        exit(1);
    }

    jpeg_start_decompress(&cinfo);
    while (cinfo.output_scanline < cinfo.output_height)
    {
        ligne = image + 3 * largimg * cinfo.output_scanline;
        jpeg_read_scanlines(&cinfo, &ligne, 1);
    }

    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);
    fclose(file);

    // Copie vers tableau [256][256][3]
     // Copie vers tableau [MAX_HAUT][MAX_LARG][3]
    for (int i = 0; i < hautimg; i++) {
        for (int j = 0; j < largimg; j++) {
            int idx = (i * largimg + j) * 3;
            texture[i][j][0] = image[idx];
            texture[i][j][1] = image[idx + 1];
            texture[i][j][2] = image[idx + 2];
        }
    }

    // Inversion verticale (flip) pour OpenGL
    for (int i = 0; i < hautimg / 2; i++) {
        for (int j = 0; j < largimg; j++) {
            std::swap(texture[i][j][0], texture[hautimg - 1 - i][j][0]);
            std::swap(texture[i][j][1], texture[hautimg - 1 - i][j][1]);
            std::swap(texture[i][j][2], texture[hautimg - 1 - i][j][2]);
        }
    }


}




// -------------------- Lecture JPEG --------------------
void loadJpegImage2(const char *fichier)
{
    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr jerr;
    FILE *file2;
    unsigned char *ligne;

    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_decompress(&cinfo);

#ifdef __WIN32
    if (fopen_s(&file, fichier, "rb") != 0)
#else
    if ((file2 = fopen(fichier, "rb")) == 0)
#endif
    {
        fprintf(stderr, "Erreur : impossible d'ouvrir %s\n", fichier);
        exit(1);
    }

    jpeg_stdio_src(&cinfo, file2);
    jpeg_read_header(&cinfo, TRUE);

    largimg = cinfo.image_width;
    hautimg = cinfo.image_height;

    if (cinfo.jpeg_color_space == JCS_GRAYSCALE)
    {
        fprintf(stdout, "Erreur : l'image doit être RGB\n");
        exit(1);
    }

    if (largimg > 4500 || hautimg > 4500)
    {
        fprintf(stderr, "Erreur : image trop grande (max %dx%d)\n", MAX_LARG, MAX_HAUT);
        exit(1);
    }

    jpeg_start_decompress(&cinfo);
    while (cinfo.output_scanline < cinfo.output_height)
    {
        ligne = image2 + 3 * largimg * cinfo.output_scanline;
        jpeg_read_scanlines(&cinfo, &ligne, 1);
    }

    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);
    fclose(file2);

    // Copie vers tableau [256][256][3]
     // Copie vers tableau [MAX_HAUT][MAX_LARG][3]
    for (int i = 0; i < 4500; i++) {
        for (int j = 0; j < 4500; j++) {
            int idx = (i * 4500 + j) * 3;
            pelouse[i][j][0] = image2[idx];
            pelouse[i][j][1] = image2[idx + 1];
            pelouse[i][j][2] = image2[idx + 2];
        }
    }




}





// -------------------- Création de la texture OpenGL --------------------
void initTextureCorps()
{

      glGenTextures(1, &texCorps);
    glBindTexture(GL_TEXTURE_2D, texCorps);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, largimg, hautimg, 0, GL_RGB, GL_UNSIGNED_BYTE, texture);

}

// -------------------- Dessin du corps avec texture --------------------
    void drawCorpsTexture()
    {
         glEnable(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, largimg, hautimg, 0, GL_RGB, GL_UNSIGNED_BYTE, texture);
        glColor3f(1.0f, 1.0f, 1.0f);

        GLUquadric *quad = gluNewQuadric();
        gluQuadricTexture(quad, GL_TRUE);
        gluQuadricNormals(quad, GLU_SMOOTH);
        glPushMatrix();
          glScalef(1.8f, 2.7f, 1.3f);
          gluSphere(quad, 1.0f, 40, 40);
        glPopMatrix();
        gluDeleteQuadric(quad);
        glDisable(GL_TEXTURE_2D);
        glEnable(GL_LIGHTING);
    }

// -------------------- Dessin du sphere tete avec texture --------------------
void drawTexturedSphere(float r, int slices, int stacks) {
    glEnable(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, largimg, hautimg, 0, GL_RGB, GL_UNSIGNED_BYTE, texture);
    glColor3f(1.0f, 1.0f, 1.0f);       // Couleur blanche = texture pure

    for (int i = 0; i < stacks; ++i) {
        float phi1 = M_PI * i / stacks;
        float phi2 = M_PI * (i + 1) / stacks;

        float v1 = 1.0f - (float)i / stacks;     // v = 1 en haut, 0 en bas
        float v2 = 1.0f - (float)(i + 1) / stacks;

        glBegin(GL_TRIANGLE_STRIP);
        for (int j = 0; j <= slices; ++j) {
            float theta = 2 * M_PI * j / slices;
            float u = (float)j / slices;         // u = 0 à 1 autour de la sphère

            // Point 1 (phi1)
            float x1 = r * sin(phi1) * cos(theta);
            float y1 = r * cos(phi1);
            float z1 = r * sin(phi1) * sin(theta);

            // Point 2 (phi2)
            float x2 = r * sin(phi2) * cos(theta);
            float y2 = r * cos(phi2);
            float z2 = r * sin(phi2) * sin(theta);

            // === Vertex 1 ===
            glNormal3f(x1 / r, y1 / r, z1 / r);
            glTexCoord2f(u, v1);
            glVertex3f(x1, y1, z1);

            // === Vertex 2 ===
            glNormal3f(x2 / r, y2 / r, z2 / r);
            glTexCoord2f(u, v2);
            glVertex3f(x2, y2, z2);
        }
        glEnd();
    }

    glDisable(GL_TEXTURE_2D);
    glEnable(GL_LIGHTING);  // Réactiver pour les autres parties
}





void dessinePikachu3D() {
    glPushMatrix(); // Début du Pikachu

    //le sol
/*--------------------------------------------------------------------------------------------*/
    glPushMatrix();

    glEnable(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 4500, 4500, 0, GL_RGB, GL_UNSIGNED_BYTE, pelouse);
    glScalef(50, 1, 50);
    glTranslatef(0, -4, 0);
    glColor3f(1, 1, 1);
    glBegin(GL_POLYGON);
    glTexCoord2f(0, 0);
    glVertex3f(-1.0, 0, -1.0);
    glTexCoord2f(0, 1);
    glVertex3f(1.0, 0, -1.0);
    glTexCoord2f(1, 1);
    glVertex3f(1.0, 0, 1.0);
    glTexCoord2f(1, 0);
    glVertex3f(-1.0, 0, 1.0);

    glEnd();
    glDisable(GL_TEXTURE_2D);
    glPopMatrix();









    // 1. CORPS
    glPushMatrix();
    drawCorpsTexture();
    glPopMatrix();

    // 2. TÊTE → REMPLACE drawSphere(...) par drawTexturedSphere
    glPushMatrix();
    glTranslatef(0.0f, 2.0f, 0.0f);
    //drawTexturedSphere(1.3f, 40, 40);  // Texture sur la tête !
    solidSphere(1.3f, 40, 40, YELLOW);
    glPopMatrix();

    // 3. OREILLE GAUCHE
    glPushMatrix();
    glTranslatef(-0.7f, 2.3f, 0.0f);
    glScalef(0.6f, 1.8f, 0.6f);
    solidSphere(1.0f, 20, 20, YELLOW);
    glTranslatef(0.0f, 0.5f, 0.0f);
    solidSphere(0.7f, 10, 10, BLACK);
    glPopMatrix();

    // 4. OREILLE DROITE
    glPushMatrix();
    glTranslatef(0.7f, 2.3f, 0.0f);
    glScalef(0.6f, 1.8f, 0.6f);
    solidSphere(1.0f, 20, 20, YELLOW);
    glTranslatef(0.0f, 0.5f, 0.0f);
    solidSphere(0.7f, 20, 20, BLACK);
    glPopMatrix();

    // 5. JOUES ROUGES
    glPushMatrix();
    glTranslatef(0.4f, 2.0f, 0.8f);
    solidSphere(0.45f, 25, 25, RED);
    glPopMatrix();
    glPushMatrix();
    glTranslatef(-0.4f, 2.0f, 0.8f);
    solidSphere(0.45f, 25, 25, RED);
    glPopMatrix();

    // 6. OEIL GAUCHE
    glPushMatrix();
    glTranslatef(-0.5f, 2.5f, 0.8f);
    solidSphere(0.35f, 25, 25, BLACK);
    glTranslatef(0.05f, 0.1f, 0.1f);
    solidSphere(0.18f, 20, 20, WHITE);
    glTranslatef(0.05f, 0.05f, 0.05f);
    solidSphere(0.12f, 15, 15, BLACK);
    glTranslatef(-0.1f, 0.05f, 0.05f);
    solidSphere(0.08f, 10, 10, WHITE);
    glPopMatrix();

    // 7. OEIL DROIT
    glPushMatrix();
    glTranslatef(0.5f, 2.5f, 0.8f);
    solidSphere(0.35f, 25, 25, BLACK);
    glTranslatef(-0.05f, 0.1f, 0.1f);
    solidSphere(0.18f, 20, 20, WHITE);
    glTranslatef(-0.05f, 0.05f, 0.05f);
    solidSphere(0.12f, 15, 15, BLACK);
    glTranslatef(0.1f, 0.05f, 0.05f);
    solidSphere(0.08f, 10, 10, WHITE);
    glPopMatrix();

    // 8. NEZ
    glPushMatrix();
    glTranslatef(0.0f, 2.4f, 1.3f);
    glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
    glColor3f(BLACK);
    glutSolidCone(0.12f, 0.2f, 15, 1);
    glPopMatrix();

    // 9. BOUCHE + LANGUE
    glPushMatrix();
    glTranslatef(0.0f, 1.8f, 1.1f);
    glScalef(1.0f, 0.4f, 0.4f);
    solidSphere(0.5f, 20, 20, DARKRED);
    glTranslatef(0.0f, -0.3f, -0.1f);
    solidSphere(0.35f, 20, 20, PINK);
    glPopMatrix();

    // 11. BRAS GAUCHE + MAIN + DOIGTS
    glPushMatrix();
    glColor3f(YELLOW);
    glTranslatef(-1.6f, 1.0f, 0.0f);
    glRotatef(-40.0f, 0.0f, 1.0f, 0.0f);
    glutSolidCylinder(Rbras, Lbras, 20, 5);
    glTranslatef(0.0f, 0.0f, Lbras);
    // Main
    glPushMatrix();
    glScalef(1.2f, 0.8f, 1.0f);
    solidSphere(0.4f, 20, 20, YELLOW);
    glPopMatrix();
    // Doigts
    glPushMatrix(); glTranslatef(-0.35f, 0.1f, 0.0f); glScalef(0.6f, 0.7f, 0.6f); solidSphere(0.25f, 12, 12, RED); glPopMatrix();
    glPushMatrix(); glTranslatef(-0.15f, 0.1f, 0.3f); glScalef(0.6f, 0.7f, 0.6f); solidSphere(0.25f, 12, 12, RED); glPopMatrix();
    glPushMatrix(); glTranslatef( 0.10f, 0.1f, 0.3f); glScalef(0.6f, 0.7f, 0.6f); solidSphere(0.25f, 12, 12, RED); glPopMatrix();
    glPushMatrix(); glTranslatef( 0.35f, 0.1f, 0.2f); glScalef(0.6f, 0.7f, 0.6f); solidSphere(0.25f, 12, 12, RED); glPopMatrix();
    glPopMatrix();

    // 11. BRAS DROIT + MAIN + DOIGTS
    glPushMatrix();
    glColor3f(YELLOW);
    glTranslatef(1.6f, 1.0f, 0.0f);
    glRotatef(40.0f, 0.0f, 1.0f, 0.0f);
    glutSolidCylinder(Rbras, Lbras, 20, 5);
    glTranslatef(0.0f, 0.0f, Lbras);
    // Main
    glPushMatrix();
    glScalef(1.2f, 0.8f, 1.0f);
    solidSphere(0.4f, 20, 20, YELLOW);
    glPopMatrix();
    // Doigts
    glPushMatrix(); glTranslatef(0.35f, 0.1f, 0.0f); glScalef(0.6f, 0.7f, 0.6f); solidSphere(0.25f, 12, 12, RED); glPopMatrix();
    glPushMatrix(); glTranslatef(0.15f, 0.1f, 0.3f); glScalef(0.6f, 0.7f, 0.6f); solidSphere(0.25f, 12, 12, RED); glPopMatrix();
    glPushMatrix(); glTranslatef(-0.10f, 0.1f, 0.3f); glScalef(0.6f, 0.7f, 0.6f); solidSphere(0.25f, 12, 12, RED); glPopMatrix();
    glPushMatrix(); glTranslatef(-0.35f, 0.1f, 0.2f); glScalef(0.6f, 0.7f, 0.6f); solidSphere(0.25f, 12, 12, RED); glPopMatrix();
    glPopMatrix();

    // 13. JAMBE GAUCHE
    glPushMatrix();
    glTranslatef(-0.7f, -2.2f, 0.0f);
    glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
    glColor3f(YELLOW);
    glutSolidCylinder(Rbras, Lbras, 20, 5);
    glPopMatrix();

    // 13. JAMBE DROITE
    glPushMatrix();
    glTranslatef(0.7f, -2.2f, 0.0f);
    glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
    glColor3f(YELLOW);
    glutSolidCylinder(Rbras, Lbras, 20, 5);
    glPopMatrix();

    // 14. PIED DROIT ROUGE
    glPushMatrix();
    glTranslatef(0.7f, -4.2f, 0.7f);
    glScalef(1.0f, 0.7f, 2.3f);
    solidSphere(0.45f, 20, 20, RED);
    glPopMatrix();

    // 15. PIED GAUCHE ROUGE
    glPushMatrix();
    glTranslatef(-0.7f, -4.2f, 0.7f);
    glScalef(1.0f, 0.7f, 2.3f);
    solidSphere(0.45f, 20, 20, RED);
    glPopMatrix();

    glPopMatrix(); // Fin du Pikachu
}

// === AFFICHAGE ===
void affichage() {


    // ==================================================================
    // 4. DESSIN
    // ==================================================================
    drawAxes();
    dessinePikachu3D();

    glutSwapBuffers();
}

// === CLAVIER / SOURIS / MAIN ===
void clavier(unsigned char touche, int x, int y) {
    switch (touche) {
        case 'p': glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); break;
        case 'f': glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); break;
        case 's': glPolygonMode(GL_FRONT_AND_BACK, GL_POINT); break;
        case 'q': exit(0); break;
    }
    glutPostRedisplay();
}

void reshape(int w, int h) {
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60, (float)w/h, 0.1, 100);
    glMatrixMode(GL_MODELVIEW);
}

void mouse(int bouton, int etat, int x, int y) {
    if (bouton == GLUT_LEFT_BUTTON && etat == GLUT_DOWN) { presse = 1; xold = x; yold = y; }
    if (bouton == GLUT_LEFT_BUTTON && etat == GLUT_UP) presse = 0;
}

void mousemotion(int x, int y) {
    if (presse) {
        anglex += (x - xold);
        angley += (y - yold);
        glutPostRedisplay();
    }
    xold = x; yold = y;
}

int main(int argc, char** argv) {
    /* Chargement de la texture */

    // → Position définie DANS affichage() avec matrice propre

    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);


    loadJpegImage2("./pelouse.jpg");
    loadJpegImage("./texture.jpg");

    initTextureCorps();

    FILE* f = fopen("texture.jpg", "rb");
if (!f) {
    printf("texture.jpg manquant !\n");
    exit(1);
}
fclose(f);




    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(1500, 1500);
    glutCreateWindow("Pikachu 3D + Axes X Y Z - BRAS CORRIGES");
    glClearColor(0.94f, 0.94f, 0.94f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_COLOR_MATERIAL);

    // Lumière basique
    GLfloat light_pos[] = { 5.0, 5.0, 5.0, 1.0 };
    glLightfv(GL_LIGHT0, GL_POSITION, light_pos);

    glutDisplayFunc(affichage);
    glutKeyboardFunc(clavier);
    glutReshapeFunc(reshape);
    glutMouseFunc(mouse);
    glutMotionFunc(mousemotion);

    glutMainLoop();
    return 0;
}





















/*
void initCylinder() {
    float angleStep = 2.0f * M_PI / NBFACE;
    for (int i = 0; i < NBFACE; i++) {
        float angle = i * angleStep;
        float x = RADIUS * cosf(angle);
        float z = RADIUS * sinf(angle);

        // Haut (y = +0.5)
        pCyl[i][0] = x;           pCyl[i][1] =  0.5f; pCyl[i][2] = z;
        // Bas (y = -0.5)
        pCyl[i + NBFACE][0] = x;  pCyl[i + NBFACE][1] = -0.5f; pCyl[i + NBFACE][2] = z;

        nCyl[i][0] = x; nCyl[i][1] = dessinJambeDroitePikachu();0.0f; nCyl[i][2] = z;
    }
    for (int i = 0; i < NBFACE; i++) {
        int n = (i + 1) % NBFACE;
        fCyl[i][0] = i;           // haut i
        fCyl[i][1] = n;           // haut i+1
        fCyl[i][2] = n + NBFACE;  // bas i+1
        fCyl[i][3] = i + NBFACE;  // bas i
    }
}


void dessinBrasDroitPikachu() {
    glPushMatrix();

    // === POSITION + ROTATION : MIROIR DU BRAS GAUCHE ===
    glTranslatef(1.6f, 1.0f, 0.0f);     // X inversé
    glRotatef(-80.0f, 0.0f, 1.0f, 1.0f);  // Angle inversé

    // === DESSIN DU CYLINDRE JAUNE ===
    glPushMatrix();
    glScalef(Rbras, Lbras, Rbras);  // Rayon X/Z, Hauteur Y

    // Couvercle bas (près du corps)
    glNormal3f(0.0f, -1.0f, 0.0f);
    glBegin(GL_POLYGON);
    glColor3f(YELLOW);
    for (int i = 0; i < NBFACE; i++) {
        glVertex3f(pCyl[i + NBFACE][0], pCyl[i + NBFACE][1], pCyl[i + NBFACE][2]);
    }
    glEnd();

    // Couvercle haut (main)
    glNormal3f(0.0f, 1.0f, 0.0f);
    glBegin(GL_POLYGON);
    glColor3f(YELLOW);
    for (int i = 0; i < NBFACE; i++) {
        glVertex3f(pCyl[i][0], pCyl[i][1], pCyl[i][2]);
    }
    glEnd();

    // Faces latérales jaunes
    glColor3f(YELLOW);
    for (int i = 0; i < NBFACE; i++) {
        glBegin(GL_POLYGON);
        glNormal3f(nCyl[i][0], nCyl[i][1], nCyl[i][2]);
        glVertex3f(pCyl[fCyl[i][0]][0], pCyl[fCyl[i][0]][1], pCyl[fCyl[i][0]][2]);
        glVertex3f(pCyl[fCyl[i][1]][0], pCyl[fCyl[i][1]][1], pCyl[fCyl[i][1]][2]);
        glVertex3f(pCyl[fCyl[i][2]][0], pCyl[fCyl[i][2]][1], pCyl[fCyl[i][2]][2]);
        glVertex3f(pCyl[fCyl[i][3]][0], pCyl[fCyl[i][3]][1], pCyl[fCyl[i][3]][2]);
        glEnd();
    }

    glPopMatrix(); // Fin du scale

    // CORRIGÉ : ALLER AU BOUT DU BRAS LE LONG DE Z
    glTranslatef(0.0f, 0.0f, Lbras);
}

void dessinJambeDroitePikachu() {
    glPushMatrix();

    // === POSITION + ROTATION : IDENTIQUE À glutSolidCylinder ===
    glTranslatef(0.7f, -3.2f, 0.0f);
    glRotatef(0.0f, 1.0f, 0.0f, 0.0f);  // Cylindre le long de Y négatif

    // === DESSIN DU CYLINDRE JAUNE (le long de Y) ===
    glPushMatrix();
    glScalef(Rbras, Lbras, Rbras);  // X=rayon, Y=hauteur, Z=rayon

    // Couvercle bas (pied) → y = -0.5 → après scale = -Lbras/2
    glNormal3f(0.0f, -1.0f, 0.0f);
    glBegin(GL_POLYGON);
    glColor3f(YELLOW);
    for (int i = 0; i < NBFACE; i++) {
        glVertex3f(pCyl[i + NBFACE][0], pCyl[i + NBFACE][1], pCyl[i + NBFACE][2]); // BAS
    }
    glEnd();

    // Couvercle haut (genou) → y = +0.5 → après scale = +Lbras/2
    glNormal3f(0.0f, 1.0f, 0.0f);
    glBegin(GL_POLYGON);
    glColor3f(YELLOW);
    for (int i = 0; i < NBFACE; i++) {
        glVertex3f(pCyl[i][0], pCyl[i][1], pCyl[i][2]); // HAUT
    }
    glEnd();

    // Faces latérales jaunes
    glColor3f(YELLOW);
    for (int i = 0; i < NBFACE; i++) {
        glBegin(GL_POLYGON);
        glNormal3f(nCyl[i][0], nCyl[i][1], nCyl[i][2]);

        glVertex3f(pCyl[fCyl[i][0]][0], pCyl[fCyl[i][0]][1], pCyl[fCyl[i][0]][2]);
        glVertex3f(pCyl[fCyl[i][1]][0], pCyl[fCyl[i][1]][1], pCyl[fCyl[i][1]][2]);
        glVertex3f(pCyl[fCyl[i][2]][0], pCyl[fCyl[i][2]][1], pCyl[fCyl[i][2]][2]);
        glVertex3f(pCyl[fCyl[i][3]][0], pCyl[fCyl[i][3]][1], pCyl[fCyl[i][3]][2]);
        glEnd();
    }

    glPopMatrix(); // Fin du scale

    // === ALLER AU PIED (optionnel, si tu veux ajouter un pied) ===
    // glTranslatef(0.0f, -Lbras, 0.0f);  // Y négatif
}

void initCylinder() {
    float angleStep = 2.0f * M_PI / NBFACE;
    for (int i = 0; i < NBFACE; i++) {
        float angle = i * angleStep;
        float x = RADIUS * cosf(angle);
        float z = RADIUS * sinf(angle);

        // Haut (y = +0.5)
        pCyl[i][0] = x;           pCyl[i][1] =  0.5f; pCyl[i][2] = z;
        // Bas (y = -0.5)
        pCyl[i + NBFACE][0] = x;  pCyl[i + NBFACE][1] = -0.5f; pCyl[i + NBFACE][2] = z;

        nCyl[i][0] = x; nCyl[i][1] = dessinJambeDroitePikachu();0.0f; nCyl[i][2] = z;
    }
    for (int i = 0; i < NBFACE; i++) {
        int n = (i + 1) % NBFACE;
        fCyl[i][0] = i;           // haut i
        fCyl[i][1] = n;           // haut i+1
        fCyl[i][2] = n + NBFACE;  // bas i+1
        fCyl[i][3] = i + NBFACE;  // bas i
    }
}*/


