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

#define num_bodies 2
#define SCREEN_X 320
#define SCREEN_Y 240

void controls();
void calculateAllBodyPhysics();

uint64_t timeStep = 360000;

// Distance is in KM
// Mass is Metric Tonnes
// Temperatature is in kelvin
// Velocity is m/s


// This is all in km
int64_t camX = 0;
int64_t camY = 0;
int64_t camZoom = 1000;
bool    correctSizedBodies = true;


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

void newDefaultPlanet(uint16_t i, int64_t x, int64_t y, int64_t vx, int64_t vy) {
    // Mars-like planet
    bodies[i].mass = 63900;
    bodies[i].x = x;
    bodies[i].y = y;
    bodies[i].velocityX = vx;
    bodies[i].velocityY = vy;
}

int main(void)
{
    bool partial_redraw = true;

    /* No rendering allowed! */
    begin();

    newDefaultPlanet(0, 0, 0, 0, 0);
    newDefaultPlanet(1, 0, -20000, 1, 0);

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

        /* Queue the buffered frame to be displayed */
        gfx_SwapDraw();
    }

    /* End graphics drawing */
    gfx_End();
    end();

    return 0;
}

void moveBody(int i) {
    bodies[i].x += bodies[i].velocityX * timeStep / 3600;
    bodies[i].y += bodies[i].velocityY * timeStep / 3600;
}

void gravity(int i) {

}

void calculateAllBodyPhysics() {
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
    if (kb_Data[1] & kb_2nd) {
        camZoom -= (camZoom / 10);
    }
    if (kb_Data[1] & kb_Alpha) {
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
}

/* Implement me! */
bool step(void)
{
    return !kb_On;
}

void draw_planet(uint16_t i) {
    gfx_SetColor(193);
    uint16_t drawSize = 10;
    if(correctSizedBodies) {
        drawSize = 1000 * SCREEN_X / camZoom;
        if(drawSize < 2) {
            drawSize =1;
        }
    }
    gfx_FillCircle((bodies[i].x - camX) * SCREEN_X / camZoom + 160, (camY - bodies[i].y) * SCREEN_X / camZoom + 120, drawSize);
}

void gui() {
    gfx_SetTextFGColor(254);
    gfx_SetTextXY(0, 0);
    gfx_PrintInt(camZoom, 2);
    gfx_PrintString(" KM    ");

    gfx_PrintInt(timeStep, 2);
    gfx_PrintString("x");
}

void draw(void)
{
    gfx_FillScreen(0);
    for(uint16_t i = 0; i < num_bodies; i++) {
        draw_planet(i);
    }

    gui();
}
