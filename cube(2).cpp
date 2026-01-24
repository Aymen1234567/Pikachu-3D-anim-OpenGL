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
float distance_cam = 8.0f;  // Distance initiale de la caméra

float angleX = 0.0f;   // Rotation horizontale (gauche/droite)
float angleY = 0.0f;   // Rotation verticale (haut/bas)

// === ANIMATION NEZ (AUTOMATIQUE) ===
float nez_angle = 90.0f;         // Angle initial (base en haut)
float nez_cible = 90.0f;         // Angle cible
float nez_vitesse = 3.0f;        // Vitesse de rotation
bool nez_en_mouvement = false;
float angle_light = 0.0f;  // <-- Red light rotation (now global + updated in idle)

// === Animation clavier : bras levés ===
bool bras_leves = false;
float bras_angle = 0.0f;  // Angle actuel des bras
float bras_cible = 0.0f;  // Angle cible
const float vitesse_bras = 5.0f;  // Vitesse d'animation


char presse;
int anglex = 20, angley = -30, xold, yold;

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

void specialKeys(int key, int x, int y) {
    const float step = 3.0f;  // Vitesse de rotation

    switch (key) {
        case GLUT_KEY_LEFT:   // Left Arrow
            angleX += step;
            break;
        case GLUT_KEY_RIGHT:  // Right Arrow
            angleX -= step;
            break;
        case GLUT_KEY_DOWN:   // Down Arrow
            angleY -= step;
            if (angleY < -80.0f) angleY = -80.0f;  // Limite bas
            break;
        case GLUT_KEY_UP:     // Up Arrow
            angleY += step;
            if (angleY > 80.0f) angleY = 80.0f;    // Limite haut
            break;
    }
    glutPostRedisplay();
}


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
    glTranslatef(0.0f, 0.0f, 0.0f); glScalef(1.8f, 2.7f, 1.3f); solidSphere(1.0f, 30, 30, YELLOW);
    glPopMatrix();

    // 2. TÊTE
    glPushMatrix();
    glTranslatef(0.0f, 2.0f, 0.0f);
    drawTexturedSphere(1.3f, 40, 40);  // Texture sur la tête !
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

        // === NEZ ANIMÉ (AUTOMATIQUE) ===
    glPushMatrix();
    glTranslatef(0.0f, 2.4f, 1.3f);  // Position nez

    // === ROTATION DU NEZ (base en haut → base en bas) ===
    glRotatef(nez_angle, 1.0f, 0.0f, 0.0f);  // Tourne sur X

    glColor3f(BLACK);
    glutSolidCone(0.12f, 0.2f, 15, 1);  // Cône (pointe vers le haut ou bas)

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
    glRotatef(-40.0f + bras_angle, 0.0f, 1.0f, 0.0f);  // Animation ici
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
    glRotatef(40.0f - bras_angle, 0.0f, 1.0f, 0.0f);  // Miroir
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



        // === QUEUE SIMPLE (2 morceaux seulement) ===
    glPushMatrix();
    glTranslatef(0.0f, -0.5f, -1.0f);   // attachée au bas du dos


    glColor3f(BLACK);
    //glScalef(1.2f, 0.8f, 1.6f);
    glutSolidCube(1.0f);

    glTranslatef(0.5f, 0.0f, -0.5f);
    glRotatef(90.0f, 0.0f, 0.0f, 1.0f);
    glColor3f(YELLOW);
    glScalef(0.8f, 2.0f, 0.2f);
    glutSolidCube(1.0f);


    glPopMatrix();

     glPushMatrix();
     glTranslatef(1.5f, 0.3f, -1.5f);      // partie jaune qui dépasse
    glRotatef(90.0f, 0.0f, 0.0f, 1.0f); // penche un peu en arrière
    glColor3f(YELLOW);
    glScalef(0.8f, 2.0f, 0.2f);
    glutSolidCube(1.0f);
    glPopMatrix();

    glPushMatrix();
     glTranslatef(2.5f, 0.8f, -1.5f);      // partie jaune qui dépasse
    glRotatef(90.0f, 0.0f, 0.0f, 1.0f); // penche un peu en arrière
    glColor3f(YELLOW);
    glScalef(0.8f, 2.0f, 0.2f);
    glutSolidCube(1.0f);
    glPopMatrix();

    glPushMatrix();
     glTranslatef(3.5f, 1.3f, -1.5f);      // partie jaune qui dépasse
    glRotatef(90.0f, 0.0f, 0.0f, 1.0f); // penche un peu en arrière
    glColor3f(YELLOW);
    glScalef(0.8f, 2.0f, 0.2f);
    glutSolidCube(1.0f);
    glPopMatrix();
    // =============================================

    glPopMatrix(); // Fin du Pikachu
}

// === AFFICHAGE ===
void affichage() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);

    // ==============================================================
    // 1. Lumière directionnelle = le solIEL
    // ==============================================================
    glPushMatrix();

    // --- LUMIÈRE DIRECTIONNELLE FIXE (GL_LIGHT1) ---
    GLfloat sun_dir[] = {0.0f, -1.0f, 0.0f, 0.0f};  // w=0 → direction
    glLightfv(GL_LIGHT1, GL_POSITION, sun_dir);

    // --- DESSINER LA SPHÈRE DU SOLEIL (avec éclairage) ---
    glTranslatef(0.0f, 6.0f, 0.0f);  // Position dans le ciel

    // Option : Sphère "brillante" sans éclairage (glow)
    glDisable(GL_LIGHTING);
    glColor3f(1.0f, 0.8f, 0.0f);
    glutSolidSphere(1.0f, 16, 16);
    glEnable(GL_LIGHTING);
    glPopMatrix();

    // ==============================================================
    // 2. CAMÉRA : ORBITE + ZOOM
    // ==============================================================
     glLoadIdentity();
    // Calculer position de la caméra en orbite
    float camX = distance_cam * sinf(angleX * M_PI / 180.0f) * cosf(angleY * M_PI / 180.0f);
    float camY = distance_cam * sinf(angleY * M_PI / 180.0f);
    float camZ = distance_cam * cosf(angleX * M_PI / 180.0f) * cosf(angleY * M_PI / 180.0f);

    gluLookAt(camX, camY, camZ,   // Position caméra
              0, 0, 0,            // Regarde le centre (Pikachu)
              0, 1, 0);           // Haut

    // ==============================================================
    // 3. LUMIÈRE jaune TOURNANTE (Lumière positionnelle = la boule jaune qui tourne)
    // ==============================================================

    GLfloat red_light_pos[] = {6*cosf(angle_light), 3.0f, 6*sinf(angle_light), 1.0f};
    glLightfv(GL_LIGHT0, GL_POSITION, red_light_pos);

    glPushMatrix();
    glTranslatef(red_light_pos[0], red_light_pos[1], red_light_pos[2]);
    glDisable(GL_LIGHTING);
    glColor3f(1.0f, 1.0f, 0.0f);
    glutSolidSphere(0.35f, 16, 16);
    glEnable(GL_LIGHTING);
    glPopMatrix();

    // ==============================================================
    // 4. DESSIN
    // ==============================================================
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
        // === ZOOM ===
        case 'z':
            distance_cam -= 0.5f;           // rapproche
            if (distance_cam < 2.0f) distance_cam = 2.0f;  // limite
            break;
        case 'Z':
            distance_cam += 0.5f;           // éloigne
            if (distance_cam > 20.0f) distance_cam = 20.0f; // limite
            break;
        // === ANIMATION : BRAS LEVÉS ===
        case 'a':
        case 'A':
            bras_leves = !bras_leves;
            bras_cible = bras_leves ? 90.0f : 0.0f;  // 90° = bras en l'air
            break;

        case 'r':  // Reset
            angleX = 0.0f; angleY = 0.0f; distance_cam = 8.0f;
            break;

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
        angleX += (x - xold);
        angleY += (y - yold);
        glutPostRedisplay();
    }
    xold = x; yold = y;
}

void idle() {

    static float minut = 0.0f;
    minut += 0.016f;  // ~60 FPS

    // Toutes les 2 secondes : déclencher rotation
    if (minut > 2.0f) {
        nez_en_mouvement = true;
        nez_cible = (nez_cible == 90.0f) ? 270.0f : 90.0f;  // 90° → 270° (inverse)
        minut = 0.0f;
    }

    // Animation fluide
    if (nez_en_mouvement) {
        if (nez_cible > nez_angle) {
            nez_angle += nez_vitesse;
            if (nez_angle >= nez_cible) {
                nez_angle = nez_cible;
                nez_en_mouvement = false;
            }
        } else {
            nez_angle -= nez_vitesse;
            if (nez_angle <= nez_cible) {
                nez_angle = nez_cible;
                nez_en_mouvement = false;
            }
        }
    }

    // Bras fluides
    if (bras_angle < bras_cible - 0.1f) {
        bras_angle += vitesse_bras;
    } else if (bras_angle > bras_cible + 0.1f) {
        bras_angle -= vitesse_bras;
    } else {
        bras_angle = bras_cible;
    }

    //Lumiére rouge tournant
    angle_light += 0.025f;
    if (angle_light > 2*M_PI) angle_light -= 2*M_PI;

    glutPostRedisplay();
}

int main(int argc, char** argv)
{
    // ================================================================
    // 1. Chargement des textures (avant glutInit)
    // ================================================================
    loadJpegImage2("./pelouse.jpg");
    loadJpegImage("./texture.jpg");

    // ================================================================
    // 2. Initialisation GLUT
    // ================================================================
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(1500, 1500);
    glutCreateWindow("Pikachu 3D + Axes X Y Z");

    // ================================================================
    // 3. Paramètres OpenGL de base
    // ================================================================
    glClearColor(0.94f, 0.94f, 0.94f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);

    // ================================================================
    // 4. Lumière ambiante globale
    // ================================================================
    //const GLfloat global_ambient[] = { 0.5f, 0.5f, 0.5f, 1.0f };
    //glLightModelfv(GL_LIGHT_MODEL_AMBIENT, global_ambient);

    // ================================================================
    // 5. LUMIÈRE 0 : Lampe jaune tournante
    // ================================================================
    glEnable(GL_LIGHT0);

    // Définition de la couleur jaune pour la lumière diffuse
    const GLfloat light0_diffuse[]  = { 1.0f, 1.0f, 0.0f, 1.0f };  // Jaune (R=1.0, G=1.0, B=0.0)
    const GLfloat light0_ambient[]  = { 0.15f, 0.15f, 0.0f, 1.0f }; // Ambiante légèrement jaune
    const GLfloat light0_specular[] = { 1.0f, 1.0f, 0.0f, 1.0f };  // Jaune pour la lumière spéculaire

    glLightfv(GL_LIGHT0, GL_DIFFUSE,  light0_diffuse);
    glLightfv(GL_LIGHT0, GL_AMBIENT,  light0_ambient);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light0_specular);


    // ================================================================
    // 6. LUMIÈRE 1 : Soleil directionnel fixe
    // ================================================================
    glEnable(GL_LIGHT1);
    const GLfloat light1_diffuse[] = { 0.4f, 0.35f, 0.3f, 1.0f };
    const GLfloat light1_ambient[] = { 0.1f, 0.1f, 0.12f, 1.0f };
    glLightfv(GL_LIGHT1, GL_DIFFUSE, light1_diffuse);
    glLightfv(GL_LIGHT1, GL_AMBIENT, light1_ambient);


    glutDisplayFunc(affichage);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(clavier);
    glutSpecialFunc(specialKeys);
    glutMouseFunc(mouse);
    glutMotionFunc(mousemotion);
    glutIdleFunc(idle);


    glutMainLoop();
    return 0;
}























