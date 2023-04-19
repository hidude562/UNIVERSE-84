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
#include <time.h>

/* Here are some standard headers. Take a look at the toolchain for more. */
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <graphx.h>
#include <keypadc.h>
#include <usbdrvce.h>

#define num_bodies 4
#define SCREEN_X 320
#define SCREEN_Y 240

void controls();
void calculateAllBodyPhysics();


bool prevDecPoint;
bool prevChs;
bool prevEnter;
bool paused;

uint64_t timeStep = 3600;

// Distance is in KM
// Mass is Metric Tonnes
// Temperatature is in kelvin


// This is all in km
int64_t camX = 0;
int64_t camY = 0;


int64_t camX2 = 0;
int64_t camY2 = 0;

int64_t camZoom = 100000;
bool    correctSizedBodies = true;
uint16_t selectedPlanet = 0;
clock_t beginFrame;

struct body {
    // Mass isn't really a specific unit but the weight of the earth is about 130,000
    // To convert from kg to this unit, divide by 46,000,000,000,000,000,000
    char* name;
    char* desc;

    uint64_t mass;


    // 1000 = 1kg per cubic meter
    uint32_t atmosphereDensity;
    int64_t radius;
    int magFieldStrength;
    int64_t area;

    // Kelvin
    int64_t surfaceTemperature;

    // Surface temp and area combined
    int64_t brightness;
    int64_t surfaceStabilizeTemperature;

    int64_t coreTemperature;

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

char* getRandomName() {

    const char* vowels[8] = {"a", "ar", "oo", "o", "u", "e", "ea", "i"};
    const char* consonants[8] = {"b", "c", "n", "p", "k", "d", "t", "v"};
    char* newName = "";

    for(int i = 0; i < rand() % 5 + 2; i++) {
        if(i % 2 == 0) {
            strcat(newName, vowels[rand() % 8]);
        } else {
            strcat(newName, consonants[rand() % 8]);
        }
    }

    return newName;
}


void setAdvancedPlanet(uint16_t i, int64_t x, int64_t y, int64_t vx, int64_t vy, int64_t mass, int radius, char* name, int32_t coreTemperature, uint32_t atmosphereDensity) {
    // Mars-like planet
    bodies[i].name = name;
    bodies[i].desc = "A description.....";
    bodies[i].radius = radius;
    bodies[i].mass = mass;
    bodies[i].x = x;
    bodies[i].y = y;
    bodies[i].velocityX = vx;
    bodies[i].velocityY = vy;

    bodies[i].surfaceTemperature = 288;
    bodies[i].coreTemperature = coreTemperature;
    bodies[i].atmosphereDensity = atmosphereDensity;
}

void setDefaultPlanet(uint16_t i, int64_t x, int64_t y, int64_t vx, int64_t vy, int64_t mass, int radius) {
    // Mars-like planet
    bodies[i].name = getRandomName();
    bodies[i].desc = "A description.....";
    bodies[i].radius = radius;
    bodies[i].mass = mass;
    bodies[i].x = x;
    bodies[i].y = y;
    bodies[i].velocityX = vx;
    bodies[i].velocityY = vy;
    bodies[i].surfaceTemperature = 288;
    bodies[i].coreTemperature = 1590;
    bodies[i].atmosphereDensity = 1200;
}

void cameraOnPlanet(uint16_t planet) {
    camX2 = bodies[planet].x;
    camY2 = bodies[planet].y;
}

int main(void)
{
    srand(time(NULL));
    bool partial_redraw = true;

    /* No rendering allowed! */
    begin();


    /*
    // Earth Moon orbit

    setDefaultPlanet(0, 0, 0,      0, 0, 130000, 6378);
    setDefaultPlanet(1, 0, -384400, 3683, 0, 100, 1079);
    */

    setAdvancedPlanet(0, 0, 0, 107826, 0, 130000, 6378, "Earth", 1590, 1200);
    setAdvancedPlanet(1, 0, -384400, 107826 + 3683, 0, 1600, 1079, "Luna", 1300, 0);
    //setDefaultPlanet(1, 0,  -384400, 107826 + 3683, 0, 1600, 1079);
    setAdvancedPlanet(2, 0, -150000000, 0, 0, 43000000000, 696000, "Sol", 15000255, 0);
    setAdvancedPlanet(3, 0, -150000000 + 66784000, 146000, 0, 106000, 6051, "Venus", 5160, 65000);

    //4324000000

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
        beginFrame = clock();
        if(timeStep > 0) {
            calculateAllBodyPhysics();
        }
        cameraOnPlanet(selectedPlanet);
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

void applyBody(int i) {
    bodies[i].x += bodies[i].velocityX * (timeStep * 1 / 3600) / 1;
    bodies[i].y += bodies[i].velocityY * (timeStep * 1 / 3600) / 1;
    //bodies[i].xOver100 = bodies[i].x / 100;
    //bodies[i].yOver100 = bodies[i].y / 100;

    bodies[i].surfaceTemperature = bodies[i].surfaceStabilizeTemperature;
    bodies[i].brightness = (((bodies[i].radius / 100) * (bodies[i].radius / 100) / 1000) * bodies[i].surfaceTemperature) / 48400;

    //bodies[i].surfaceTemperature += (bodies[i].surfaceStabilizeTemperature - bodies[i].surfaceTemperature) / 10;
}

void physics(int i) {
    bodies[i].surfaceStabilizeTemperature = bodies[i].coreTemperature / 2599;

    for(int j = 0; j < num_bodies; j++) {
        if(j != i) {
            //TODO: implement inverse square law
            int_fast64_t deltaX = bodies[j].x - bodies[i].x;
            int_fast64_t deltaY = bodies[j].y - bodies[i].y;
            int_fast64_t squaredDist = (deltaX * deltaX) + (deltaY * deltaY);
            int_fast64_t dist   = fastestSqrt64(squaredDist);

            int_fast64_t gravityX = (deltaX * 10) / (dist / 10) * (bodies[j].mass / 100);//(bodies[j].mass / (dist / 10));
            int_fast64_t gravityY = (deltaY * 10) / (dist / 10) * (bodies[j].mass / 100);//(bodies[j].mass) / (dist / 10);

            bodies[i].surfaceStabilizeTemperature += (6931208 * bodies[j].brightness) / (dist + 1213 * bodies[j].brightness) + 10;// * 100000 / ((bodies[j].radius * bodies[j].radius / 10000000 + 1))) ;

            gravityX*=(timeStep / 3600);
            gravityY*=(timeStep / 3600);
            bodies[i].velocityX += gravityX / (squaredDist / 40000000);//(timeStep / 10);  bodies[j].mass /
            bodies[i].velocityY += gravityY / (squaredDist / 40000000);//(timeStep / 10);
        }
    }
    bodies[i].surfaceStabilizeTemperature = (bodies[i].surfaceStabilizeTemperature * (1000 + bodies[i].atmosphereDensity / 300)) / 1000;
}

void calculateAllBodyPhysics() {
    for(int i = 0; i < num_bodies; i++) {
        // Really bad efficiency since this doesn't just check the parent for gravity
        // O(n^2 - n) efficiency
        //calculateStabilizingTemperature(i);
        physics(i);
    }

    for(int i = 0; i < num_bodies; i++) {
        applyBody(i);

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
            if(timeStep == 0) {
                timeStep+=3600;
            } else {
                timeStep*=2;
            }
        prevDecPoint = true;
    } else {prevDecPoint = false;}
    if((kb_Data[5] & kb_Chs)) {
        if(!prevChs && timeStep > 3600)
            timeStep/=2;
        else {
            paused = true;
            timeStep = 0;
        }
        prevChs = true;
    } else {prevChs = false;}

    if((kb_Data[6] & kb_Enter)) {
        if(!prevEnter) {
            selectedPlanet=(selectedPlanet + 1) % num_bodies;
            camX = 0;
            camY = 0;
        }
        prevEnter = true;
    } else {prevEnter = false;}
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
    int64_t dispX = (bodies[i].x - camX - camX2) * SCREEN_X / camZoom + 160;
    if(dispX + drawSize > 0 && dispX - drawSize < SCREEN_X) {
        int64_t dispY = (camY + camY2 - bodies[i].y) * SCREEN_X / camZoom + 120;
        if(dispY + drawSize > 0 && dispY - drawSize < SCREEN_Y) {
            gfx_FillCircle(dispX, dispY, drawSize);
        }
    }
}

void planetInfo() {
    gfx_SetColor(0);
    //gfx_FillRectangle(SCREEN_X - 80, 0, 80, 240);
    gfx_SetTextXY(SCREEN_X - 110, 0);

    gfx_PrintString(bodies[selectedPlanet].name);

    gfx_SetTextXY(SCREEN_X - 110, 20);

    gfx_PrintString("R:");
    gfx_PrintUInt(bodies[selectedPlanet].radius, 1);
    gfx_PrintString("km");

    gfx_SetTextXY(SCREEN_X - 110, 30);

    gfx_PrintString("T:");
    gfx_PrintUInt(bodies[selectedPlanet].surfaceStabilizeTemperature, 1);
    gfx_PrintString("K");

    gfx_SetTextXY(SCREEN_X - 110, 40);

    gfx_PrintString("T:");
    gfx_PrintInt((bodies[selectedPlanet].surfaceStabilizeTemperature - 273) * 9 / 5 + 32, 1);
    gfx_PrintString("F");

    gfx_SetTextXY(SCREEN_X - 110, 50);

    gfx_PrintString("Vx:");
    gfx_PrintInt(bodies[selectedPlanet].velocityX, 1);
    gfx_PrintString("kmh");

    gfx_SetTextXY(SCREEN_X - 110, 60);

    gfx_PrintString("Vy:");
    gfx_PrintInt(bodies[selectedPlanet].velocityY, 1);
    gfx_PrintString("kmh");

    gfx_SetTextXY(SCREEN_X - 110, 70);

    gfx_PrintString("M:");
    gfx_PrintInt(bodies[selectedPlanet].mass, 1);

    gfx_SetTextXY(SCREEN_X - 110, 80);

    gfx_PrintString("A:");
    gfx_PrintInt(bodies[selectedPlanet].atmosphereDensity / 1000, 1);
    gfx_PrintString(".");
    gfx_PrintInt(bodies[selectedPlanet].atmosphereDensity % 1000, 1);
    gfx_PrintString("kg/m");
}

void gui() {
    gfx_SetTextFGColor(254);
    gfx_SetTextXY(0, 0);
    gfx_PrintInt(camZoom, 2);
    gfx_PrintString(" KM    ");

    if(timeStep > 0) {
        gfx_PrintInt(timeStep, 1);
        gfx_PrintString("x ");
    } else {
        gfx_PrintString("Paused ");
    }

    gfx_SetTextXY(0, SCREEN_Y - 9);
    gfx_PrintInt(10000 / (clock() - beginFrame), 2);
    gfx_PrintString("fps ");

    planetInfo();
}

void draw(void)
{
    gfx_FillScreen(0);
    for(uint16_t i = 0; i < num_bodies; i++) {
        draw_planet(i);
    }

    gui();
}
