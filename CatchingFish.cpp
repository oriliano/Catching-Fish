/*********
    
----------

----------
PROBLEMS:
----------
ADDITIONAL FEATURES:
1) When player catches the target he needs to pull it like fishing by holding the up button.
2) Once the player pulls the target he needs to press enter to collect it.
3) There are 4 different targets and every target has its own value which the game sums it in the top right corner.
4) The firing object has a rope that stays attached to the target object.
5) There's a shark.
*********/

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif
#include <GL/glut.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <string.h>
#include <stdarg.h>
#include "vec.h"

#define WINDOW_WIDTH  1280
#define WINDOW_HEIGHT 800

#define TIMER_PERIOD  12 // Period for the timer.
#define TIMER_ON         1 // 0:disable timer, 1:enable timer

#define D2R 0.0174532
#define TARGET_WIDTH  192
#define ENEMY_SPEED 4
#define PLAYER_SPEED 3

float A = 180, //amplitude
fq = 0.5,  //frequency
C = 0,   //horizontal phase shift
B = -150;   //vertical phase shift




/* Global Variables for Template File */
bool up = false, down = false, right = false, left = false, spacebar = false;
int  winWidth, winHeight; // current Window width and height
int order = 0, score = 0;

bool activeTimer = true;
bool hc = false;
bool hc2 = false;
bool hide = false;
bool add = false;
bool change = false;
bool pull = false;
bool pulltext = false;


typedef struct {
    float x, y;
} point_t;

typedef struct {
    vec_t pos;
    float angle;
    bool active;
    vec_t vel;
} fire_t;

typedef struct {
    int r, g, b;
} color_t;

typedef struct {
    vec_t pos;
    color_t color;
    float width;
    vec_t speed;
} target_t;

#define FOV  30    // Field of View Angle
#define MAX_FIRE 120 // 20 fires at a time.
#define FIRE_RATE 8 // after 8 frames you can throw another one.

fire_t fr = { {0 ,255} };
target_t target[4];
int fire_rate = 0;





// mouse position
point_t mouse;

//
// to draw circle, center at (x,y)
// radius r
//
void circle(int x, int y, int r)
{
#define PI 3.1415
    float angle;
    glBegin(GL_POLYGON);
    for (int i = 0; i < 100; i++)
    {
        angle = 2 * PI * i / 100;
        glVertex2f(x + r * cos(angle), y + r * sin(angle));
    }
    glEnd();
}

void circle_wire(int x, int y, int r)
{
#define PI 3.1415
    float angle;

    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < 100; i++)
    {
        angle = 2 * PI * i / 100;
        glVertex2f(x + r * cos(angle), y + r * sin(angle));
    }
    glEnd();
}

void pie_filled(float x, float y, float r, float start, float end) {
    glBegin(GL_POLYGON);
    glVertex2f(x, y);
    for (float angle = start; angle < end; angle += 10) {
        glVertex2f(r * cos(angle * D2R) + x, r * sin(angle * D2R) + y);
    }
    glVertex2f(r * cos(end * D2R) + x, r * sin(end * D2R) + y);
    glEnd();
}

void print(int x, int y, const char* string, void* font)
{
    int len, i;

    glRasterPos2f(x, y);
    len = (int)strlen(string);
    for (i = 0; i < len; i++)
    {
        glutBitmapCharacter(font, string[i]);
    }
}

// display text with variables.
// vprint(-winWidth / 2 + 10, winHeight / 2 - 20, GLUT_BITMAP_8_BY_13, "ERROR: %d", numClicks);
void vprint(int x, int y, void* font, const char* string, ...)
{
    va_list ap;
    va_start(ap, string);
    char str[1024];
    vsprintf_s(str, string, ap);
    va_end(ap);

    int len, i;
    glRasterPos2f(x, y);
    len = (int)strlen(str);
    for (i = 0; i < len; i++)
    {
        glutBitmapCharacter(font, str[i]);
    }
}

// vprint2(-50, 0, 0.35, "00:%02d", timeCounter);
void vprint2(int x, int y, float size, const char* string, ...) {
    va_list ap;
    va_start(ap, string);
    char str[1024];
    vsprintf_s(str, string, ap);
    va_end(ap);
    glPushMatrix();
    glTranslatef(x, y, 0);
    glScalef(size, size, 1);

    int len, i;
    len = (int)strlen(str);
    for (i = 0; i < len; i++)
    {
        glutStrokeCharacter(GLUT_STROKE_ROMAN, str[i]);
    }
    glPopMatrix();
}

float f(float x) {
    return A * sin((fq * x + C) * D2R) + B;
}

void vertex(vec_t P, vec_t Tr, double angle) {
    double xp = (P.x * cos(angle) - P.y * sin(angle)) + Tr.x;
    double yp = (P.x * sin(angle) + P.y * cos(angle)) + Tr.y;
    glVertex2d(xp, yp);
}

void sea() {

    glColor3f(0, 0, 0.3);
    glRectf(-700, 150, 700, 148);

    glColor3f(0, 0, 0.6);
    glRectf(-700, 150, 700, -400);

    glColor3ub(178, 178, 125);
    glRectf(-700, -325, 700, -400);

    for (int i = 0; i <178; i++)
    {
        glColor3ub(178 - i, 178 - i, 125 + i/6 );
        glBegin(GL_LINES);
        glVertex2f(-700, -325 + i);
        glVertex2f(700, -325 + i);
        glEnd();
    }


    glColor3f(0.0, 0.4, 0.2); 
    glBegin(GL_TRIANGLES);
    glVertex2f(-440, -400);
    glVertex2f(-460, -400); 
    glVertex2f(-450, -340); 
    glEnd();

    glColor3f(0.0, 0.4, 0.2);
    glBegin(GL_TRIANGLES);
    glVertex2f(-450, -400);
    glVertex2f(-470, -400);
    glVertex2f(-470, -360);
    glEnd();

    glColor3f(0.0, 0.4, 0.2);
    glBegin(GL_TRIANGLES);
    glVertex2f(-430, -400);
    glVertex2f(-450, -400);
    glVertex2f(-430, -360);
    glEnd();




    glColor3f(0.0, 0.4, 0.2);
    glBegin(GL_TRIANGLES);
    glVertex2f(440, -400);
    glVertex2f(460, -400);
    glVertex2f(450, -340);
    glEnd();

    glColor3f(0.0, 0.4, 0.2);
    glBegin(GL_TRIANGLES);
    glVertex2f(450, -400);
    glVertex2f(470, -400);
    glVertex2f(470, -360);
    glEnd();

    glColor3f(0.0, 0.4, 0.2);
    glBegin(GL_TRIANGLES);
    glVertex2f(430, -400);
    glVertex2f(450, -400);
    glVertex2f(430, -360);
    glEnd();

    

    glColor3f(0.0, 0.4, 0.2);
    glBegin(GL_TRIANGLES);
    glVertex2f(-40, -400);
    glVertex2f(-60, -400);
    glVertex2f(-50, -340);
    glEnd();

    glColor3f(0.0, 0.4, 0.2);
    glBegin(GL_TRIANGLES);
    glVertex2f(-40, -400);
    glVertex2f(-70, -400);
    glVertex2f(-70, -360);
    glEnd();

    glColor3f(0.0, 0.4, 0.2);
    glBegin(GL_TRIANGLES);
    glVertex2f(-30, -400);
    glVertex2f(-50, -400);
    glVertex2f(-30, -360);
    glEnd();




}

void top() {
    glColor3ub(135, 206, 235);
    glRectf(-700, 400, 700, 150);

    glColor3f(1, 1, 1);
    glBegin(GL_POLYGON);
    glVertex2f(-690, 150);
    glVertex2f(-620, 305);
    glVertex2f(-580, 245);
    glVertex2f(-540, 275);
    glVertex2f(-500, 185);
    glVertex2f(-480, 245);
    glVertex2f(-460, 165);
    glVertex2f(-420, 275);
    glVertex2f(-380, 150);
    glEnd();

    glColor3f(0.8, 0.8, 0.8);
    glBegin(GL_POLYGON);
    glVertex2f(-680, 150);
    glVertex2f(-620, 285);
    glVertex2f(-580, 225);
    glVertex2f(-540, 255);
    glVertex2f(-500, 165);
    glVertex2f(-480, 225);
    glVertex2f(-460, 160);
    glVertex2f(-420, 265);
    glVertex2f(-390, 150);
    glEnd();





    glColor3f(1, 1, 1);
    glBegin(GL_POLYGON);
    glVertex2f(690, 150);
    glVertex2f(620, 305);
    glVertex2f(580, 245);
    glVertex2f(540, 275);
    glVertex2f(500, 185);
    glVertex2f(480, 245);
    glVertex2f(460, 165);
    glVertex2f(420, 275);
    glVertex2f(380, 150);
    glEnd();

    glColor3f(0.8, 0.8, 0.8);
    glBegin(GL_POLYGON);
    glVertex2f(680, 150);
    glVertex2f(620, 285);
    glVertex2f(580, 225);
    glVertex2f(540, 255);
    glVertex2f(500, 165);
    glVertex2f(480, 225);
    glVertex2f(460, 160);
    glVertex2f(420, 265);
    glVertex2f(390, 150);
    glEnd();



}

void hasan() {

    glColor3f(0, 0.5, 0);
    glRectf(85, 220, 35, 260);

    glColor3f(0, 0.6, 0);
    glRectf(85, 240, 65, 260);

    glColor3ub(232, 190, 172);
    glRectf(85, 240, 65, 220);

    glColor3ub(232, 190, 172);
    glRectf(70, 260, 50, 280);

    glColor3ub(232, 190, 172);
    glRectf(50, 232, 65, 220);

    glColor3ub(232, 190, 172);
    circle(45, 226, 10);

    glColor3ub(62, 24, 21);
    circle(60, 300, 20);

    glColor3ub(92, 54, 51);
    circle(60, 280, 20);

    glColor3ub(232, 190, 172);
    circle(60, 290, 19);

    glColor3ub(92, 54, 51);
    circle(60, 277, 10);

    glColor3ub(62, 24, 21);
    glRectf(40, 300, 80, 310);

    glColor3ub(232, 190, 172);
    glRectf(54, 278, 66, 280);

    glColor3f(0.9, 0.9, 0.9);
    circle(52, 293, 3);

    glColor3f(0.9, 0.9, 0.9);
    circle(68, 293, 3);

    glColor3f(0, 0, 0.9);
    circle(52, 293, 2);

    glColor3f(0, 0, 0.9);
    circle(68, 293, 2);

    glColor3f(0, 0, 0);
    circle(52, 293, 1);

    glColor3f(0, 0, 0);
    circle(68, 293, 1);

    glColor3f(0, 0, 0);
    glRectf(46, 297, 56, 299);

    glColor3f(0, 0, 0);
    glRectf(74, 297, 64, 299);

    glColor3ub(232, 190, 172);
    circle(40, 290, 3);

    glColor3ub(232, 190, 172);
    circle(80, 290, 3);


    glColor3f(0.5, 0.5, 0.5);
    glBegin(GL_QUADS);
    glVertex2f(42, 225);
    glVertex2f(50, 225);
    glVertex2f(3, 340);
    glVertex2f(-2, 335);
    glEnd();

    glColor3f(0.7, 0.7, 0.7);
    glRectf(35, 257, 10, 263);

    glColor3f(0.6, 0.2, 0.2);
    circle(35, 260, 10);
    

    glColor3f(0.4, 0.2, 0.2);
    circle(35, 265, 3);

    glColor3f(0.4, 0.2, 0.2);
    circle(35, 255, 3);

    glColor3f(0.4, 0.2, 0.2);
    circle(40, 260, 3);

    glColor3f(0.4, 0.2, 0.2);
    circle(30, 260, 3);

    glColor3f(0.4, 0.4, 0.4);
    circle(0, 335, 6);

    

    glColor3f(161. / 255, 102. / 255, 47. / 255);
    glRectf(265, 210, -65, 220);

    glColor3f(131. / 255, 80. / 255, 47. / 255);
    glBegin(GL_QUADS);
    glVertex2f(0, 150);
    glVertex2f(200, 150);
    glVertex2f(260, 210);
    glVertex2f(-60, 210);
    glEnd();

    glColor3f(161. / 255, 102. / 255, 47. / 255);
    glBegin(GL_QUADS);
    glVertex2f(0, 150);
    glVertex2f(200, 150);
    glVertex2f(240, 190);
    glVertex2f(-40, 190);
    glEnd();

    glColor3f(131. / 255, 80. / 255, 47. / 255);
    glBegin(GL_QUADS);
    glVertex2f(0, 150);
    glVertex2f(200, 150);
    glVertex2f(220, 170);
    glVertex2f(-20, 170);
    glEnd();


   





}

void rod() {

    float angle2 = atan2(target[order].pos.x, -f(target[order].pos.x));

    glColor3f(0, 0, 0);
    glBegin(GL_QUADS);
    glVertex2f(-1, 335);
    glVertex2f(1, 335);
    vertex({ +2,  0 }, fr.pos, angle2);
    vertex({ -2,  0 }, fr.pos, angle2);
    glEnd();

    

    glColor3ub(140, 140, 140);
    glBegin(GL_QUADS);
    vertex({ 2,  0 }, fr.pos, angle2);
    vertex({ -2,  0 }, fr.pos, angle2);
    vertex({ -2,  -15 }, fr.pos, angle2);
    vertex({ 2,  -15 }, fr.pos, angle2);
    glEnd();


    glColor3ub(120, 120, 120);
    glBegin(GL_TRIANGLES);
    vertex({ 5,  -15 }, fr.pos, angle2);
    vertex({ -5,  -15 }, fr.pos, angle2);
    vertex({ 0,  -25 }, fr.pos, angle2);
    glEnd();


}

void scoreboard() {

    glColor3ub(255, 255, 255);
    vprint(-600, 360, GLUT_BITMAP_HELVETICA_18, "MONEY:             TL");

    glColor3ub(255, 255, 255);
    vprint2(-520, 360, 0.15, "%00004d", score);
    glColor3ub(255, 255, 255);
    vprint2(-519, 360, 0.15, "%00004d", score);
    glColor3ub(255, 255, 255);
    vprint2(-518, 360, 0.15, "%00004d", score);

    glColor3ub(255, 255, 255);
    vprint2(-520, 359, 0.15, "%00004d", score);
    glColor3ub(255, 255, 255);
    vprint2(-519, 359, 0.15, "%00004d", score);
    glColor3ub(255, 255, 255);
    vprint2(-518, 359, 0.15, "%00004d", score);

}

void money(target_t* t) {


    switch (order)
    {

    case 0:
       

        glColor3ub(170, 120, 0);
        circle(t[0].pos.x, f(t[0].pos.x) + t[0].pos.y, 55);

        glColor3ub(250, 210, 0);
        circle(t[0].pos.x, f(t[0].pos.x) + t[0].pos.y, 45);

        glColor3ub(170, 120, 0);
        glRectf(t[0].pos.x - 8, f(t[0].pos.x) + t[0].pos.y - 27, t[0].pos.x -18, f(t[0].pos.x) + t[0].pos.y + 27);
        
        glColor3ub(170, 120, 0);
        glBegin(GL_QUADS);
        glVertex2f(t[0].pos.x - 18, f(t[0].pos.x) + t[0].pos.y + 27);
        glVertex2f(t[0].pos.x - 18, f(t[0].pos.x) + t[0].pos.y + 16);
        glVertex2f(t[0].pos.x - 27, f(t[0].pos.x) + t[0].pos.y + 11);
        glVertex2f(t[0].pos.x - 27, f(t[0].pos.x) + t[0].pos.y + 18);
        glEnd();

        
        glColor3ub(170, 120, 0);
        circle(t[0].pos.x +17, f(t[0].pos.x) + t[0].pos.y -7, 20);

        glColor3ub(250, 210, 0);
        circle(t[0].pos.x + 17, f(t[0].pos.x) + t[0].pos.y - 7, 12);

        break;

    case 1:
        glColor3ub(0, 60, 0);
        circle(t[1].pos.x, f(t[1].pos.x) + t[1].pos.y, 55);

        glColor3ub(0,180, 0);
        circle(t[1].pos.x, f(t[1].pos.x) + t[1].pos.y, 45);

        glColor3ub(0, 60, 0);
        glRectf(t[1].pos.x - 12, f(t[1].pos.x) + t[1].pos.y - 12, t[1].pos.x + 4, f(t[1].pos.x) + t[1].pos.y - 17);

        glColor3ub(0, 60, 0);
        glRectf(t[1].pos.x - 26, f(t[1].pos.x) + t[1].pos.y - 5, t[1].pos.x - 12, f(t[1].pos.x) + t[1].pos.y - 10);

        glColor3ub(0, 60, 0);
        glRectf(t[1].pos.x - 26, f(t[1].pos.x) + t[1].pos.y - 25, t[1].pos.x + 15, f(t[1].pos.x) + t[1].pos.y - 30);

        glColor3ub(0, 60, 0);
        glRectf(t[1].pos.x - 26, f(t[1].pos.x) + t[1].pos.y - 5, t[1].pos.x - 21, f(t[1].pos.x) + t[1].pos.y - 30);

        glColor3ub(0, 60, 0);
        glRectf(t[1].pos.x - 16, f(t[1].pos.x) + t[1].pos.y - 5, t[1].pos.x - 11, f(t[1].pos.x) + t[1].pos.y - 17);

        glColor3ub(0, 60, 0);
        glRectf(t[1].pos.x +14, f(t[1].pos.x) + t[1].pos.y + 30, t[1].pos.x +19, f(t[1].pos.x) + t[1].pos.y - 30);

        glColor3ub(0, 60, 0);
        glRectf(t[1].pos.x + 4, f(t[1].pos.x) + t[1].pos.y + 30, t[1].pos.x -1, f(t[1].pos.x) + t[1].pos.y - 13);

        glColor3ub(0, 60, 0);
        glRectf(t[1].pos.x + 4, f(t[1].pos.x) + t[1].pos.y + 25, t[1].pos.x + 15, f(t[1].pos.x) + t[1].pos.y + 30);

        break;
    case 2:
        glColor3ub(100, 32, 180);
        circle(t[2].pos.x, f(t[2].pos.x) + t[2].pos.y, 55);

        glColor3ub(160, 32, 240);
        circle(t[2].pos.x, f(t[2].pos.x) + t[2].pos.y, 45);

        glColor3ub(100, 32, 180);
        glBegin(GL_TRIANGLES);
        glVertex2f(t[2].pos.x, f(t[2].pos.x) + t[2].pos.y + 35);
        glVertex2f(t[2].pos.x - 30, f(t[2].pos.x) + t[2].pos.y - 25);
        glVertex2f(t[2].pos.x + 30, f(t[2].pos.x) + t[2].pos.y - 25);
        glEnd();

        glColor3ub(160, 32, 240);
        glBegin(GL_TRIANGLES);
        glVertex2f(t[2].pos.x, f(t[2].pos.x) + t[2].pos.y + 20);
        glVertex2f(t[2].pos.x - 18, f(t[2].pos.x) + t[2].pos.y - 25);
        glVertex2f(t[2].pos.x + 18, f(t[2].pos.x) + t[2].pos.y - 25);
        glEnd();

        glColor3ub(100, 32, 180);
        glRectf(t[2].pos.x - 20, f(t[2].pos.x) + t[2].pos.y - 5, t[2].pos.x + 20, f(t[2].pos.x) + t[2].pos.y - 12);

        break;
    case 3:
        glColor3ub(0, 0, 60);
        circle(t[3].pos.x, f(t[3].pos.x) + t[3].pos.y, 55);

        glColor3ub(0, 0, 120);
        circle(t[3].pos.x, f(t[3].pos.x) + t[3].pos.y, 45);

        glColor3ub(0, 0, 60);
        glRectf(t[3].pos.x - 20, f(t[3].pos.x) + t[3].pos.y - 25, t[3].pos.x + 20, f(t[3].pos.x) + t[3].pos.y + 25);

        glColor3ub(0, 0, 120);
        glRectf(t[3].pos.x - 10, f(t[3].pos.x) + t[3].pos.y - 15, t[3].pos.x + 10, f(t[3].pos.x) + t[3].pos.y + 25);
    default:
        break;
    }

}

void money2(target_t* t) {


    switch (order)
    {

    case 0:

        glColor3ub(170, 120, 0);
        circle(fr.pos.x, fr.pos.y - 15, 55);

        glColor3ub(250, 210, 0);
        circle(fr.pos.x, fr.pos.y - 15, 45);

        glColor3ub(170, 120, 0);
        glRectf(fr.pos.x - 8, fr.pos.y - 15 - 27, fr.pos.x - 18, fr.pos.y - 15 + 27);

        glColor3ub(170, 120, 0);
        glBegin(GL_QUADS);
        glVertex2f(fr.pos.x - 18, fr.pos.y - 15 + 27);
        glVertex2f(fr.pos.x - 18, fr.pos.y - 15 + 16);
        glVertex2f(fr.pos.x - 27, fr.pos.y - 15 + 11);
        glVertex2f(fr.pos.x - 27, fr.pos.y - 15 + 18);
        glEnd();


        glColor3ub(170, 120, 0);
        circle(fr.pos.x + 17, fr.pos.y - 15 - 7, 20);

        glColor3ub(250, 210, 0);
        circle(fr.pos.x + 17, fr.pos.y - 15 - 7, 12);

        break;

    case 1:

        glColor3ub(0, 60, 0);
        circle(fr.pos.x, fr.pos.y - 15, 55);

        glColor3ub(0, 180, 0);
        circle(fr.pos.x, fr.pos.y - 15, 45);

        glColor3ub(0, 60, 0);
        glRectf(fr.pos.x - 12, fr.pos.y - 15 - 12, fr.pos.x + 4, fr.pos.y - 15 - 17);

        glColor3ub(0, 60, 0);
        glRectf(fr.pos.x - 26, fr.pos.y - 15 - 5, fr.pos.x - 12, fr.pos.y - 15 - 10);

        glColor3ub(0, 60, 0);
        glRectf(fr.pos.x - 26, fr.pos.y - 15 - 25, fr.pos.x + 15, fr.pos.y - 15 - 30);

        glColor3ub(0, 60, 0);
        glRectf(fr.pos.x - 26, fr.pos.y - 15 - 5, fr.pos.x - 21, fr.pos.y - 15 - 30);

        glColor3ub(0, 60, 0);
        glRectf(fr.pos.x - 16, fr.pos.y - 15 - 5, fr.pos.x - 11, fr.pos.y - 15 - 17);

        glColor3ub(0, 60, 0);
        glRectf(fr.pos.x + 14, fr.pos.y - 15 + 30, fr.pos.x + 19, fr.pos.y - 15 - 30);

        glColor3ub(0, 60, 0);
        glRectf(fr.pos.x + 4, fr.pos.y - 15 + 30, fr.pos.x - 1, fr.pos.y - 15 - 13);

        glColor3ub(0, 60, 0);
        glRectf(fr.pos.x + 4, fr.pos.y - 15 + 25, fr.pos.x + 15, fr.pos.y - 15 + 30);


        break;
    case 2:

        glColor3ub(100, 32, 180);
        circle(fr.pos.x, fr.pos.y - 15, 55);

        glColor3ub(160, 32, 240);
        circle(fr.pos.x, fr.pos.y - 15, 45);

        glColor3ub(100, 32, 180);
        glBegin(GL_TRIANGLES);
        glVertex2f(fr.pos.x, fr.pos.y - 15 + 35);
        glVertex2f(fr.pos.x - 30, fr.pos.y - 15 - 25);
        glVertex2f(fr.pos.x + 30, fr.pos.y - 15 - 25);
        glEnd();

        glColor3ub(160, 32, 240);
        glBegin(GL_TRIANGLES);
        glVertex2f(fr.pos.x, fr.pos.y - 15 + 20);
        glVertex2f(fr.pos.x - 18, fr.pos.y - 15 - 25);
        glVertex2f(fr.pos.x + 18, fr.pos.y - 15 - 25);
        glEnd();

        glColor3ub(100, 32, 180);
        glRectf(fr.pos.x - 20, fr.pos.y - 15 - 5, fr.pos.x + 20, fr.pos.y - 15 - 12);
        break;
    case 3:
        glColor3ub(0, 0, 60);
        circle(fr.pos.x, fr.pos.y - 15, 55);

        glColor3ub(0, 0, 120);
        circle(fr.pos.x, fr.pos.y - 15, 45);

        glColor3ub(0, 0, 60);
        glRectf(fr.pos.x - 20, fr.pos.y - 15 - 25, fr.pos.x + 20, fr.pos.y - 15 + 25);

        glColor3ub(0, 0, 120);
        glRectf(fr.pos.x - 10, fr.pos.y - 15 - 15, fr.pos.x + 10, fr.pos.y - 15 + 25);

    default:
        break;
    }

}

void Shark() {
    float ss = -500, y, s = -200;

    glColor3ub(115, 116, 117);
    glBegin(GL_POLYGON);
    glVertex2f(s + 270, 280 +ss);
    glVertex2f(s + 272, 282 +ss);
    glVertex2f(s + 274, 284 +ss);
    glVertex2f(s + 276, 286 +ss);
    glVertex2f(s + 278, 288 +ss);
    glVertex2f(s + 280, 290 +ss);
    glVertex2f(s + 390, 313 +ss);
    glVertex2f(450 + s, 317 + ss);
    glVertex2f(470 + s, 313 + ss);
    glVertex2f(500 + s, 306 + ss);
    glVertex2f(550 + s, 300 + ss);
    glVertex2f(600 + s, 285 + ss);
    glEnd();

    glColor3ub(200, 200, 200);
    glBegin(GL_POLYGON);
    glVertex2f(s + 270, ss + 280 );
    glVertex2f(s + 272, ss + 278 );
    glVertex2f(s + 274, ss + 276 );
    glVertex2f(s + 276, ss + 274 );
    glVertex2f(s + 278, ss + 272 );
    glVertex2f(s + 280, ss + 270 );
    glVertex2f(s + 390, ss + 247 );
    glColor3ub(150, 150, 150);
    glVertex2f(450 + s, ss + 243 );
    glVertex2f(470 + s, ss + 247 );
    glVertex2f(500 + s, ss + 254 );
    glVertex2f(550 + s, ss + 257);
    glVertex2f(570 + s, ss + 260);
    glVertex2f(590 + s, ss + 265);
    glVertex2f(600 + s, ss + 270);
    glVertex2f(600 + s, ss + 277);
    glVertex2f(600 + s, ss + 285);
    glVertex2f(650 + s, ss + 285);
    glEnd();

    glColor3ub(138, 3, 3);
    glBegin(GL_POLYGON);
    glVertex2f(302 + s, 263 + ss);
    glVertex2f(330 + s, 276 + ss);
    glVertex2f(297 + s, 268 + ss);
    glEnd();

    glColor3ub(115, 116, 117);
    glBegin(GL_POLYGON);
    glVertex2f(598 + s, 285.6 + ss);
    glVertex2f(640 + s, 330 + ss);
    glVertex2f(628 + s, 275 + ss);
    glVertex2f(640 + s, 220 + ss);
    glVertex2f(598 + s, 265 + ss);
    glEnd();

    glColor3ub(100, 106, 107);
    glBegin(GL_TRIANGLES);
    glVertex2f(410 + s, 313 + ss);
    glColor3ub(150, 150, 150);
    glVertex2f(460 + s, 360 + ss);
    glVertex2f(475 + s, 313 + ss);
    glEnd();

    glColor3ub(115, 116, 117);
    glBegin(GL_TRIANGLES);
    glVertex2f(420 + s, 260 + ss);
    glVertex2f(450 + s, 210 + ss);
    glVertex2f(460 + s, 260 + ss);
    glEnd();

    glColor3ub(115, 116, 117);
    glBegin(GL_TRIANGLES);
    glVertex2f(555 + s, 297 + ss);
    glVertex2f(580 + s, 305 + ss);
    glVertex2f(580 + s, 290 + ss);
    glEnd();

    glColor3ub(115, 116, 117);
    glBegin(GL_TRIANGLES);
    glVertex2f(555 + s, 257 + ss);
    glVertex2f(580 + s, 247 + ss);
    glVertex2f(580 + s, 261 + ss);
    glEnd();
    
    glColor3f(1, 1, 1);
    circle(332 + s, 287 + ss, 3);

    glColor3f(0, 0, 0);
    circle(332 + s, 287 + ss, 1);
    

   

}

void display() {

    glClearColor(0,0,0,0);
    glClear(GL_COLOR_BUFFER_BIT);

    top();
    sea();
    Shark();
    hasan();
    money(target);
    if (change)
        money2(target);
    rod();
    scoreboard();
    if (pulltext) {
        glColor3ub(255, 255, 255);
        vprint(-140, -300, GLUT_BITMAP_HELVETICA_18, "Hold UP BUTTON to pull the money.");

    }

    if (!hide) {
        glColor3ub(255, 255, 255);
        vprint(-120, -300, GLUT_BITMAP_HELVETICA_18, "Press ENTER to collect the money.");

    }








    glutSwapBuffers();
}

//
// key function for ASCII charachters like ESC, a,b,c..,A,B,..Z
//




void onKeyDown(unsigned char key, int x, int y)
{
    // exit when ESC is pressed.
    if (key == 27)
        exit(0);

    if (key == ' ')
        spacebar = true;

    

    if (key == 13)
        add = true;





    // to refresh the window it calls display() function
    glutPostRedisplay();
}

void onKeyUp(unsigned char key, int x, int y)
{
    // exit when ESC is pressed.
    if (key == 27)
        exit(0);
    if (key == ' ') {

    }


    // to refresh the window it calls display() function
    glutPostRedisplay();
}


void onSpecialKeyDown(int key, int x, int y)
{
    // Write your codes here.
    switch (key) {
    case GLUT_KEY_UP:
        up = true;
        break;
    case GLUT_KEY_DOWN:
        down = true;
        break;
    case GLUT_KEY_LEFT:
        left = true;
        break;
    case GLUT_KEY_RIGHT:
        right = true;
        break;
    }

    if (key == GLUT_KEY_UP) {
        pull = true;
    }



    // to refresh the window it calls display() function
    glutPostRedisplay();
}


void onSpecialKeyUp(int key, int x, int y)
{
    // Write your codes here.
    switch (key) {
    case GLUT_KEY_UP:
        up = false;
        break;
    case GLUT_KEY_DOWN:
        down = false;
        break;
    case GLUT_KEY_LEFT:
        left = false;
        break;
    case GLUT_KEY_RIGHT:
        right = false;
        break;
    }

    if (key == GLUT_KEY_UP) {
        pull = false;
    }

    // to refresh the window it calls display() function
    glutPostRedisplay();
}


void onClick(int button, int stat, int x, int y)
{
    // Write your codes here.



    // to refresh the window it calls display() function
    glutPostRedisplay();
}


void onResize(int w, int h)
{
    winWidth = w;
    winHeight = h;
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-w / 2, w / 2, -h / 2, h / 2, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    display(); // refresh window.
}

void onMoveDown(int x, int y) {
    // Write your codes here.



    // to refresh the window it calls display() function   
    glutPostRedisplay();
}


void onMove(int x, int y) {
    // Write your codes here.
    mouse.x = x - winWidth / 2;
    mouse.y = winHeight / 2 - y;


    // to refresh the window it calls display() function
    glutPostRedisplay();
}

bool testCollision(fire_t fr) {
    float dx = target[order].pos.x - fr.pos.x;
    float dy = f(target[order].pos.x) - fr.pos.y;
    float d = sqrt(dx * dx + dy * dy);
    return d <= 50;
}


#if TIMER_ON == 1
void onTimer(int v) {


    glutTimerFunc(TIMER_PERIOD, onTimer, 0);

    if (activeTimer)
    {
        if (!hc) {
            add = false;
            hc2 = false;
            hide = true;

            target[order].pos.x += 2;
            if (target[order].pos.x > 800)
                target[order].pos.x = -900;
        }
        else {
            pulltext = true;
        }

        if (spacebar) {

            if (!hc) {
                fr.pos = addV(fr.pos, fr.vel);
                fr.vel = mulV(ENEMY_SPEED, unitV(subV({ target[order].pos.x, f(target[order].pos.x) }, fr.pos)));
            }
            else {
                if (pull) {
                    
                    fr.vel = mulV(ENEMY_SPEED, unitV(subV({ 0, 230 }, fr.pos)));
                    fr.pos = addV(fr.pos, fr.vel);
                }
                
            }

            if (testCollision(fr)) {
                target[order].pos = { 0, -1000 };
                hc = true;
                hc2 = true;
                change = true;

            }

            if (hc2) {
                if (fr.pos.x >= -3 && fr.pos.x <= 3 && fr.pos.y >= 227 && fr.pos.y <= 233) {
                    hide = false;
                    pulltext = false;
                    if (add) {
                        change = false;
                        target[order].pos = { -1000, 0 };
                        if (order == 3) {
                            order = 0;
                            score += 40;
                        }
                        else if (order == 2) {
                            order++;
                            score += 20;
                        }
                        else if (order == 1) {
                            order++;
                            score += 30;
                        }
                        else {
                            order++;
                            score += 10;
                        }
                        hc = false;
                        spacebar = false;
                    }

                }
            }

        }
    }




    glutPostRedisplay();

}
#endif

void Init() {

    target[0].pos = { 0, 0 };
    target[1].pos = { -1000, 0 };
    target[2].pos = { -1000, 0 };
    target[3].pos = { -1000, 0 };




    // Smoothing shapes
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

}

void main(int argc, char* argv[]) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
    glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    //glutInitWindowPosition(100, 100);
    glutCreateWindow("Big Bass Fishing by Orhan Kemal Koç");

    glutDisplayFunc(display);
    glutReshapeFunc(onResize);

    //
    // keyboard registration
    //
    glutKeyboardFunc(onKeyDown);
    glutSpecialFunc(onSpecialKeyDown);

    glutKeyboardUpFunc(onKeyUp);
    glutSpecialUpFunc(onSpecialKeyUp);

    //
    // mouse registration
    //
    glutMouseFunc(onClick);
    glutMotionFunc(onMoveDown);
    glutPassiveMotionFunc(onMove);

#if  TIMER_ON == 1
    // timer event
    glutTimerFunc(TIMER_PERIOD, onTimer, 0);
#endif

    Init();

    glutMainLoop();
}