////////////////////////////////////////
// { PROGRAM NAME } { VERSION }
// Author:
// License:
// Description:
////////////////////////////////////////

/*
* The comments in this file are here to guide you initially. Note that you shouldn't actually
* write comments that are pointless or obvious in your own code, write useful ones instead!
* See this for more details: https://ce-programming.github.io/toolchain/static/coding-guidelines.html
*
* Have fun!
*/

/* You probably want to keep these headers */
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <tice.h>

/* Here are some standard headers. Take a look at the toolchain for more. */
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <graphx.h>
#include <keypadc.h>
#include <usbdrvce.h>

#define num_bodies 2
#define SCREEN_X 320
#define SCREEN_Y 240

void controls();
void calculateAllBodyPhysics();


bool prevDecPoint;
bool prevChs;

uint64_t timeStep = 3600;

// Distance is in KM
// Mass is Metric Tonnes
// Temperatature is in kelvin
// Velocity is m/s


// This is all in km
int64_t camX = 0;
int64_t camY = 0;
int64_t camZoom = 100000;
bool    correctSizedBodies = true;
uint16_t selectedPlanet = -1;

struct body {
    // In metric tonnes, 10 quadrillion  = 1 num
    uint64_t mass;
    uint64_t atmosphereMass;
    int radius;
    int magFieldStrength;
    int surfaceTemperature;
    int coreTemperature;

    // In km/h
    int64_t velocityX;
    int64_t velocityY;
    int64_t x;
    int64_t y;
};

struct body bodies[num_bodies];


void begin(void);
void end(void);
bool step(void);
void draw(void);




// Source for these algorithms
// https://stackoverflow.com/questions/34187171/fast-integer-square-root-approximation


// this function computes a lower bound
uint_fast8_t bit_width(int_fast64_t x) {
    return x == 0 ? 1 : 64 - __builtin_clzll(x);
}

int_fast64_t fastestSqrt64(const int_fast64_t n)
{
    // This will be probably up to 1% off or something
    int_fast64_t log2floor = bit_width(n) - 1;
    int_fast64_t a = (int_fast64_t) (n != 0) << (log2floor >> 1);
    int_fast64_t b;

    b = n / a; a = (a+b) / 2;
    b = n / a; a = (a+b) / 2;
    return a;
}

// Only use this for testing
int_fast64_t fastSqrt64(int_fast64_t n) {
    // This will be probably up to 1% off or something
    int_fast64_t log2floor = bit_width(n) - 1;
    int_fast64_t a = (int_fast64_t) (n != 0) << (log2floor >> 1);
    int_fast64_t b;

    b = n / a; a = (a+b) / 2;
    b = n / a; a = (a+b) / 2;

    // Preform a couple extra iterations of the newton method for extra precision
    b = n / a; a = (a+b) / 2;
    b = n / a; a = (a+b) / 2;
    return a;
}

void setDefaultPlanet(uint16_t i, int64_t x, int64_t y, int64_t vx, int64_t vy, int64_t mass, int radius) {
    // Mars-like planet
    bodies[i].radius = radius;
    bodies[i].mass = mass;
    bodies[i].x = x;
    bodies[i].y = y;
    bodies[i].velocityX = vx;
    bodies[i].velocityY = vy;
}

void cameraOnPlanet(uint16_t planet) {
    camX = bodies[planet].x;
    camY = bodies[planet].y;
}

int main(void)
{
    bool partial_redraw = true;

    /* No rendering allowed! */
    begin();

    //setDefaultPlanet(0, 0, 0,      200, 0, 500, 3000);
    //setDefaultPlanet(1, 0, -20000, 0, 0, 500, 2000);

    setDefaultPlanet(0, 0, 0,      0, 0, 500, 3000);
    setDefaultPlanet(1, 0, -20000, 1000, 0, 100, 2000);

    /* Initialize graphics drawing */
    gfx_Begin();

    /* Draw to the buffer to avoid rendering artifacts */
    gfx_SetDrawBuffer();

    /* No rendering allowed in step! */
    while (step())
    {
        /* Only want to redraw part of the previous frame? */
        if (partial_redraw)
        {
            /* Copy previous frame as a base for this frame */
            gfx_BlitScreen();
        }

        /* As little non-rendering logic as possible */
        calculateAllBodyPhysics();
        draw();
        controls();
        //cameraOnPlanet(selectedPlanet);

        /* Queue the buffered frame to be displayed */
        gfx_SwapDraw();
    }

    /* End graphics drawing */
    gfx_End();
    end();

    return 0;
}

void moveBody(int i) {
    bodies[i].x += bodies[i].velocityX * (timeStep * 1 / 3600) / 1;
    bodies[i].y += bodies[i].velocityY * (timeStep * 1 / 3600) / 1;
}

void gravity(int i) {
    for(int j = 0; j < num_bodies; j++) {
        if(j != i) {
            //TODO: implement inverse square law
            int_fast64_t deltaX = bodies[j].x - bodies[i].x;
            int_fast64_t deltaY = bodies[j].y - bodies[i].y;
            int_fast64_t squaredDist = (deltaX * deltaX) + (deltaY * deltaY);
            int_fast64_t dist   = fastestSqrt64(squaredDist);

            int_fast64_t gravityX = (deltaX * 10) / (dist / 10) * (bodies[j].mass / 100);//(bodies[j].mass / (dist / 10));
            int_fast64_t gravityY = (deltaY * 10) / (dist / 10) * (bodies[j].mass / 100);//(bodies[j].mass) / (dist / 10);

            gravityX*=(timeStep / 3600);
            gravityY*=(timeStep / 3600);
            bodies[i].velocityX += gravityX / (squaredDist / 40000000);//(timeStep / 10);  bodies[j].mass /
            bodies[i].velocityY += gravityY / (squaredDist / 40000000);//(timeStep / 10);

        }
    }
}

void calculateAllBodyPhysics() {
    for(int i = 0; i < num_bodies; i++) {
        // Really bad efficiency since this doesn't just check the parent for gravity
        // O(n^2 - n) efficiency
        gravity(i);
    }

    for(int i = 0; i < num_bodies; i++) {
        moveBody(i);
    }
}

/* Implement me! */
void begin(void)
{

}

/* Implement me! */
void end(void)
{

}

void controls() {
    kb_Scan();

    if (kb_Data[1] & kb_2nd) {
        camZoom -= (camZoom / 10);
    }
    if (kb_Data[2] & kb_Alpha) {
        camZoom += (camZoom / 10);
    }
    if (kb_Data[7] & kb_Right) {
        camX += (camZoom / 40);
    }
    if (kb_Data[7] & kb_Left) {
        camX -= (camZoom / 40);
    }
    if (kb_Data[7] & kb_Up) {
        camY += (camZoom / 40);
    }
    if (kb_Data[7] & kb_Down) {
        camY -= (camZoom / 40);
    }
    if((kb_Data[4] & kb_DecPnt)) {
        if(!prevDecPoint)
            timeStep*=2;
        prevDecPoint = true;
    } else {prevDecPoint = false;}
    if((kb_Data[5] & kb_Chs)) {
        if(!prevChs && timeStep > 3600)
            timeStep/=2;
        prevChs = true;
    } else {prevChs = false;}
}

/* Implement me! */
bool step(void)
{
    return !kb_On;
}

void draw_planet(uint16_t i) {
    gfx_SetColor(193);
    int drawSize = 10;
    if(correctSizedBodies) {
        drawSize = bodies[i].radius * SCREEN_X / camZoom;
        if(drawSize < 2) {
            drawSize =1;
        }
    }
    //gfx_FillCircle(150, 150, drawSize);
    gfx_FillCircle((bodies[i].x - camX) * SCREEN_X / camZoom + 160, (camY - bodies[i].y) * SCREEN_X / camZoom + 120, drawSize);
}

void gui() {
    gfx_SetTextFGColor(254);
    gfx_SetTextXY(0, 0);
    gfx_PrintInt(camZoom, 2);
    gfx_PrintString(" KM    ");

    gfx_PrintInt(timeStep, 2);
    gfx_PrintString("x ");

}

void draw(void)
{
    gfx_FillScreen(0);
    for(uint16_t i = 0; i < num_bodies; i++) {
        draw_planet(i);
    }

    gui();
}
