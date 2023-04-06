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
struct body tempBodies[num_bodies];


void begin(void);
void end(void);
bool step(void);
void draw(void);

int64_t fastSqrt64(int64_t n) {
    // Preform binary search to find sqrt
    int64_t val = n / 2;
    int64_t iter = n / 2;

    while(iter > 1) {
        iter/=2;
        int64_t squared = val * val;
        if(squared > n) {
            val-=iter;
        } else if(squared < n) {
            val+=iter;
        } else {
            return val;
        }
    }
    return val;
}

void newDefaultPlanet(uint16_t i, int64_t x, int64_t y, int64_t vx, int64_t vy, int64_t mass) {
    // Mars-like planet
    bodies[i].mass = mass;
    bodies[i].x = x;
    bodies[i].y = y;
    bodies[i].velocityX = vx;
    bodies[i].velocityY = vy;
}

int main(void)
{
    bool partial_redraw = false;

    /* No rendering allowed! */
    begin();

    newDefaultPlanet(0, 0, 0, 0, 0, 1000);
    newDefaultPlanet(1, 0, -20000, 2000, 0, 100);

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
    bodies[i].x += bodies[i].velocityX * (timeStep / 3600);
    bodies[i].y += bodies[i].velocityY * (timeStep / 3600);
}

void gravity(int i) {
    for(int j = 0; j < num_bodies; j++) {
        if(j != i) {
            //TODO: implement inverse square law
            int64_t deltaX = bodies[j].x - bodies[i].x;
            int64_t deltaY = bodies[j].y - bodies[i].y;
            int64_t dist   = fastSqrt64((deltaX * deltaX) + (deltaY * deltaY));

            int64_t gravityX = (deltaX * 10) / (dist / 10) * (bodies[j].mass / 100);//(bodies[j].mass / (dist / 10));
            int64_t gravityY = (deltaY * 10) / (dist / 10) * (bodies[j].mass / 100);//(bodies[j].mass) / (dist / 10);

            gfx_PrintInt(dist, 2);
            gfx_PrintString(" ");
            bodies[i].velocityX += gravityX / 10;
            bodies[i].velocityY += gravityY / 10;

        }
    }
}

void calculateAllBodyPhysics() {
    for(int i = 0; i < num_bodies; i++) {
        moveBody(i);

        // Really bad efficiency since this doesn't just check the parent for gravity
        // O(n^2 - n) efficiency
        gravity(i);
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
    int drawSize = 10;
    if(correctSizedBodies) {
        drawSize = 1000 * SCREEN_X / camZoom;
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
    gfx_PrintString("x");

    // i is the current object
    int i = 0;

    // j is the object the current object is reacting to
    int j = 1;

    //gfx_PrintInt(gravityX, 2);
}

void draw(void)
{
    gfx_FillScreen(0);
    for(uint16_t i = 0; i < num_bodies; i++) {
        draw_planet(i);
    }

    gui();
}
