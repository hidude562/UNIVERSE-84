////////////////////////////////////////
// { Slightly Accurate Planetary Simulator } { 0.1 }
// Author: hidude562
// License: Literally Dont care
// Description: Pretty much 2D universe sandbox
////////////////////////////////////////

#include "header.h"
//#include "input.c"

void controls();
void calculateAllBodyPhysics();
void inputDisplay(int64_t val);

bool prevDecPoint;
bool prevChs;
bool prevEnter;
bool paused;
uint8_t selectedIndex;

bool upIndexerIsDown;
bool downIndexerIsDown;
bool isInputting = true;

uint64_t timeStep = 3600;

// Distance is in KM
// Mass is described in the struct body
// Temperatature is in kelvin


// This is all in km
int64_t camX = 0;
int64_t camY = 0;

int_fast8_t num_bodies;
int_fast8_t num_debris;
int64_t camX2 = 0;
int64_t camY2 = 0;

int64_t camZoom = 100000;
bool    correctSizedBodies = true;

//Don't use this unless you know you will be in the input dialauge, it wont be updated otherwise!
bool    pressingNumNow;

uint16_t selectedPlanet = 0;
clock_t beginFrame;

struct body {
    // Mass isn't really a specific unit but the weight of the earth is about 130,000
    // To convert from kg to this unit, divide by 46,000,000,000,000,000,000
    char* name;
    char* desc;

    uint64_t mass;

    // If the body is actually in use or is free to be replaced
    bool isBeingUsed;
    bool isSimple;

    // 1000 = 1kg per cubic meter
    uint32_t atmosphereDensity;
    int64_t radius;
    int magFieldStrength;
    int64_t area;

    // Kelvin
    int64_t surfaceTemperature;
    uint8_t color;

    // Surface temp and area combined
    int64_t brightness;
    int64_t surfaceStabilizeTemperature;

    int64_t coreTemperature;

    // In km/h
    int64_t velocityX;
    int64_t velocityY;

    // A value equal to its own mass is similar water levels to Earth
    uint32_t waterAmount;

    // Planet display vars
    bool moonLike;

    int64_t x;
    int64_t y;
};

// Debris starts from back of this array
struct body bodies[50];

void begin(void);
void end(void);
bool step(void);
void draw(void);
void drawSelectedIndex();
int64_t estimatedCoreTemp(int64_t mass);
void removeBody(uint8_t index);


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

int8_t getNumOnKeyboard(bool prevNum) {
    // Too lazy to access index through the kb array
    // 0
    pressingNumNow = false;

    if(kb_Data[3] & kb_0) {
        pressingNumNow = true;
        if(!prevNum)
            return 0;
    }


    // 1
    if(kb_Data[3] & kb_1) {
        pressingNumNow = true;
        if(!prevNum)
            return 1;
    }

    // 2
    if(kb_Data[4] & kb_2) {
        pressingNumNow = true;
        if(!prevNum)
            return 2;
    }

    // 3
    if(kb_Data[5] & kb_3) {
        pressingNumNow = true;
        if(!prevNum)
            return 3;
    }


    // 4
    if(kb_Data[3] & kb_4) {
        pressingNumNow = true;
        if(!prevNum)
            return 4;
    }

    // 5
    if(kb_Data[4] & kb_5) {
        pressingNumNow = true;
        if(!prevNum)
            return 5;
    }

    // 6
    if(kb_Data[5] & kb_6) {
        pressingNumNow = true;
        if(!prevNum)
            return 6;
    }


    // 7
    if(kb_Data[3] & kb_7) {
        pressingNumNow = true;
        if(!prevNum)
            return 7;
    }

    // 8
    if(kb_Data[4] & kb_8) {
        pressingNumNow = true;
        if(!prevNum)
            return 8;
    }

    // 9
    if(kb_Data[5] & kb_9) {
        pressingNumNow = true;
        if(!prevNum)
            return 9;
    }

    // Negative sign
    if(kb_Data[5] & kb_Chs) {
        pressingNumNow = true;
        if(!prevNum)
            return -1;
    }

    // multiply sign
    if(kb_Data[6] & kb_Mul) {
        pressingNumNow = true;
        if(!prevNum)
            return -3;
    }

    // divide sign
    if(kb_Data[6] & kb_Div) {
        pressingNumNow = true;
        if(!prevNum)
            return -4;
    }

    return -2;
}

int64_t getInput(int64_t formerValue) {
    isInputting = true;
    int64_t value = formerValue;
    bool prevDel = false;
    bool prevNum = false;

    while(!(kb_Data[6] & kb_Enter)) {
        if(kb_On) {
            while(kb_On) {}
            isInputting = false;
            return formerValue;
        }
        inputDisplay(value);
        gfx_SwapDraw();

        kb_Scan();

        // Inputs
        if(kb_Data[1] & kb_Del) {
            if(!prevDel) {
                value/=10;
            }
            prevDel = true;
        } else {
            prevDel = false;
        }

        int8_t numOnKeyBoard = getNumOnKeyboard(prevNum);

        if(numOnKeyBoard != -2) {
            if(numOnKeyBoard == -1) {
                value *= -1;
            } else if(numOnKeyBoard == -3) {
                value *= 3;
                value /= 2;
            } else if(numOnKeyBoard == -4) {
                value *= 2;
                value /= 3;
            } else {
                if(value < 0) {
                    value *= 10;
                    value -= numOnKeyBoard;
                } else {
                    value *= 10;
                    value += numOnKeyBoard;
                }
            }
        }
        prevNum = pressingNumNow;
    }

    while(kb_Data[6] & kb_Enter) {kb_Scan();}
    isInputting = false;
    return value;
}

// Not actually apart of the GFX lib but quite useful
void gfx_PrintInt64_t(int64_t num, int16_t _hereForConsistancyDoesntDoAnything) {
    // This can't harvest all the bits of int64_t but its good enough

    if(num / 10000000 != 0) {
        gfx_PrintInt((num / 10000000), 1);
    }
    if(llabs(num) < 10000000) {
        gfx_PrintInt(num % 10000000, 1);
    } else {
        gfx_PrintInt(llabs(num % 10000000), 7);
    }
}

void inputDisplay(int64_t val) {
    gfx_SetTextFGColor(254);

    gfx_SetColor(2);
    gfx_FillRectangle(20, 126, SCREEN_X - 40, 33);

    gfx_SetColor(255);
    gfx_Rectangle(20, 126, SCREEN_X - 40, 33);

    gfx_Rectangle(25, 140, SCREEN_X - 50, 15);
    gfx_SetTextXY(27, 144);
    gfx_PrintInt64_t(val, 2);

    gfx_SetTextXY(27, 130);
    gfx_PrintString("Enter a new value:");

}

void setAdvancedPlanet(uint16_t i, int64_t x, int64_t y, int64_t vx, int64_t vy, int64_t mass, int radius, char* name, int32_t coreTemperature, uint32_t atmosphereDensity, uint32_t waterAmount, bool moonLike) {
    num_bodies++;
    // Mars-like planet
    bodies[i].isBeingUsed = true;
    bodies[i].name = name;
    bodies[i].radius = radius;
    bodies[i].mass = mass;
    bodies[i].x = x;
    bodies[i].y = y;
    bodies[i].velocityX = vx;
    bodies[i].velocityY = vy;
    bodies[i].moonLike = moonLike;

    bodies[i].surfaceTemperature = 288;
    bodies[i].coreTemperature = coreTemperature;
    bodies[i].atmosphereDensity = atmosphereDensity;
    bodies[i].waterAmount = waterAmount;
}

void cameraOnPlanet(uint16_t planet) {
    camX2 = bodies[planet].x;
    camY2 = bodies[planet].y;
}

// Creates new debris that has similar compisition to I but less of it
void newDebris(uint8_t i, uint8_t otherObj, uint8_t percentOfPrevBody) {
    uint8_t newIndex = maxBodies - num_debris;
    num_debris++;
    bodies[newIndex].isBeingUsed = true;
    bodies[newIndex].isSimple = true;

    bodies[newIndex].mass = (bodies[i].mass * percentOfPrevBody) / 100;
    bodies[newIndex].waterAmount = (bodies[i].waterAmount * percentOfPrevBody) / 100;
    bodies[newIndex].atmosphereDensity = (bodies[i].atmosphereDensity * percentOfPrevBody) / 100;
    bodies[newIndex].radius = (bodies[i].radius * percentOfPrevBody) / 100;
    bodies[newIndex].surfaceTemperature = 300;
    bodies[newIndex].coreTemperature = 1000;


    int8_t randomDir1 = rand() % 256;
    int8_t randomDir2 = rand() % 256;

    bodies[newIndex].velocityX = 0 - bodies[i].velocityX / 2 + randomDir1 * 50;//(bodies[i].velocityX / 3 * randomDir / 128);
    bodies[newIndex].velocityY = 0 - bodies[i].velocityY / 2 + randomDir2 * 50;//(bodies[i].velocityY / 3 * 128 / randomDir);

    int64_t deltaX = (bodies[i].x - bodies[otherObj].x);
    int64_t deltaY = (bodies[i].y - bodies[otherObj].y);

    int64_t distanceToOther = fastestSqrt64((deltaX * deltaX) + (deltaY * deltaY));
    bodies[newIndex].x += ((deltaX * 100) / distanceToOther) * (bodies[otherObj].radius + bodies[i].radius) / 80;
    bodies[newIndex].y += ((deltaY * 100) / distanceToOther) * (bodies[otherObj].radius + bodies[i].radius) / 80;
}

// Precondition: obj1 is larger (in mass) than obj2
// Also that obj1 is not a simple object, which should be the case anyway
void createDebris(uint8_t obj1, uint8_t obj2) {
    bodies[obj1].surfaceTemperature += bodies[obj2].mass * 100 / (bodies[obj1].mass / 100) * 4;
    if(bodies[obj1].mass / bodies[obj2].mass > 200 || bodies[obj2].isSimple) {
        bodies[obj1].mass += bodies[obj2].mass;
        bodies[obj1].waterAmount += bodies[obj2].waterAmount;
        bodies[obj1].atmosphereDensity += bodies[obj2].atmosphereDensity / (bodies[obj1].mass / bodies[obj2].mass);
        // technically not correct
        bodies[obj1].radius += bodies[obj2].radius;

        removeBody(obj2);
        return;
    } else {

        newDebris(obj2, obj1, 20);
        newDebris(obj2, obj1, 20);
        newDebris(obj2, obj1, 20);
        newDebris(obj2, obj1, 20);
        newDebris(obj2, obj1, 20);

        removeBody(obj2);
    }
}

int main(void)
{
    srand(time(NULL));
    bool partial_redraw = true;

    /* No rendering allowed! */
    begin();


    /*
    // Test for collision
    setAdvancedPlanet(0, 0, 0, 0, 0, 130000, 63780, "Earth", 15900, 1293, 130000, false);
    setAdvancedPlanet(1, 0, -384400, 0, 0, 1560, 10079, "Luna", 1300, 0, 0, true);
    */

    setAdvancedPlanet(0, 0, 0, 107826, 0, 130000, 6378, "Earth", 1590, 1293, 130000, false);
    setAdvancedPlanet(1, 0, -384400, 107826 + 3683, 0, 1600, 1079, "Luna", 1300, 0, 0, true);
    setAdvancedPlanet(2, 0, -150000000, 0, 0, 43000000000, 696000, "Sol", 15000255, 0, 0, false);
    setAdvancedPlanet(3, 0, -150000000 + 66784000, 146000, 0, 106000, 6051, "Venus", 5160, 65000, 0, false);
    setAdvancedPlanet(4, 0, -150000000 + 250000000, 86677, 0, 13891, 2106, "Mars", 1090, 12, 2000, false);

    //4324000000

    /* Initialize graphics drawing */
    gfx_Begin();

    /* Draw to the buffer to avoid rendering artifacts */
    gfx_SetDrawBuffer();
    gfx_SetPalette(global_palette, 256, 0);

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

        if(timeStep > 0) {
            calculateAllBodyPhysics();
        }
        cameraOnPlanet(selectedPlanet);
        draw();
        controls();
        beginFrame = clock();

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

    // TODO: Do off of timeStep! This is a temp thing
    bodies[i].surfaceTemperature += ((bodies[i].surfaceStabilizeTemperature - bodies[i].surfaceTemperature)) / 2;
    bodies[i].brightness = (((bodies[i].radius / 100) * (bodies[i].radius / 100) / 1000) * bodies[i].surfaceTemperature) / 48400;
    if(bodies[i].surfaceTemperature > 385) {
        int64_t waterRemovePercent = 100 - (38500 / bodies[i].waterAmount);
        waterRemovePercent = 10 - ((100 - waterRemovePercent) / (timeStep / 3600));
        bodies[i].atmosphereDensity += (bodies[i].waterAmount * waterRemovePercent) / bodies[i].radius;
        bodies[i].waterAmount -=  (bodies[i].waterAmount * waterRemovePercent) / 100;
    }

    //bodies[i].surfaceTemperature += (bodies[i].surfaceStabilizeTemperature - bodies[i].surfaceTemperature) / 10;
}

void physics(int i) {
    bodies[i].surfaceStabilizeTemperature = bodies[i].coreTemperature / 2599;

    for(int j = 0; j < num_bodies; j++) {
        if(j != i) {
            if(bodies[j].mass * 15 / bodies[i].mass != 0) {
                int_fast64_t deltaX = bodies[j].x - bodies[i].x;
                int_fast64_t deltaY = bodies[j].y - bodies[i].y;
                int_fast64_t squaredDist = (deltaX * deltaX) + (deltaY * deltaY);
                int_fast64_t dist   = fastestSqrt64(squaredDist);
                bodies[i].surfaceStabilizeTemperature += (6931208 * bodies[j].brightness) / (dist + 1213 * bodies[j].brightness) + 10;

                // gravity calculations
                if((bodies[j].mass * 100000) / (squaredDist / 10000 + 1) != 0) {
                    int_fast64_t gravityX = (deltaX * 10) / (dist / 10) * (bodies[j].mass / 100);//(bodies[j].mass / (dist / 10));
                    int_fast64_t gravityY = (deltaY * 10) / (dist / 10) * (bodies[j].mass / 100);//(bodies[j].mass) / (dist / 10);

                    gravityX*=(timeStep / 3600);
                    gravityY*=(timeStep / 3600);
                    bodies[i].velocityX += gravityX / (squaredDist / 40000000); //(timeStep / 10);  bodies[j].mass /
                    bodies[i].velocityY += gravityY / (squaredDist / 40000000); //(timeStep / 10);
                }

                // Collision calculations
                if(dist - bodies[i].radius < bodies[j].radius) {
                    if(bodies[i].mass > bodies[j].mass) {
                        createDebris(i, j);
                    } else {
                        createDebris(j, i);
                    }
                }
           }
        }
    }

    bodies[i].coreTemperature = estimatedCoreTemp(bodies[i].mass);
    bodies[i].surfaceStabilizeTemperature = (bodies[i].surfaceStabilizeTemperature * (1000 + bodies[i].atmosphereDensity / 300)) / 1000;
}

void simplePhysics(int i) {
    for(int j = 0; j < num_bodies; j++) {
        if(bodies[j].mass * 15 / bodies[i].mass != 0) {
            int_fast64_t deltaX = bodies[j].x - bodies[i].x;
            int_fast64_t deltaY = bodies[j].y - bodies[i].y;
            int_fast64_t squaredDist = (deltaX * deltaX) + (deltaY * deltaY);
            int_fast64_t dist   = fastestSqrt64(squaredDist);

            if((bodies[j].mass * 100000) / (squaredDist / 10000 + 1) != 0) {
                int_fast64_t gravityX = (deltaX * 10) / (dist / 10) * (bodies[j].mass / 100);//(bodies[j].mass / (dist / 10));
                int_fast64_t gravityY = (deltaY * 10) / (dist / 10) * (bodies[j].mass / 100);//(bodies[j].mass) / (dist / 10);

                gravityX*=(timeStep / 3600);
                gravityY*=(timeStep / 3600);
                bodies[i].velocityX += gravityX / (squaredDist / 40000000); //(timeStep / 10);  bodies[j].mass /
                bodies[i].velocityY += gravityY / (squaredDist / 40000000); //(timeStep / 10);
            }

            if(dist - bodies[i].radius < bodies[j].radius) {
                createDebris(j, i);
            }

        }
    }
}

void removeBody(uint8_t index) {
    //Automatically shifts elements too
    bool wasSimple = bodies[index].isSimple;
    bodies[index].isBeingUsed = false;
    if(!wasSimple) {
        for(uint8_t i = index; i < num_bodies + 1; i++ ) {
            bodies[i] = bodies[i + 1];
        }
    } else {
        for(uint8_t i = index; i > maxBodies - num_debris; i--) {
            bodies[i] = bodies[i - 1];
        }
    }
    if(!wasSimple) {
        num_bodies--;
        if(selectedIndex >= index) {
            selectedIndex--;
        }
    } else {
        num_debris--;
    }
}

void calculateAllBodyPhysics() {
    for(int i = 0; i < num_bodies; i++) {
        // Really bad efficiency since this doesn't just check the parent for gravity
        // O(n^2 - n) efficiency worst case
        // Best case is O(n) i think
        physics(i);
    }

    for(int i = maxBodies; i > maxBodies - num_debris; i--) {
        simplePhysics(i);
    }

    for(int i = 0; i < num_bodies; i++) {
        applyBody(i);
    }

    for(int i = maxBodies; i > maxBodies - num_debris; i--) {
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

void setSelectedIndexValueByUserInput() {
    int64_t prevVal = 0;
    if(selectedIndex == 0) {//name

    }
    else if(selectedIndex == 1) {//radius
        prevVal = bodies[selectedPlanet].radius;
        bodies[selectedPlanet].radius = getInput(prevVal);
    }
    else if(selectedIndex == 2) {//surfaceTemp
        prevVal = bodies[selectedPlanet].surfaceTemperature;
        bodies[selectedPlanet].surfaceTemperature = getInput(prevVal);
    }
    else if(selectedIndex == 3) {//surfaceTempF
        prevVal = (bodies[selectedPlanet].surfaceTemperature - 273) * 9 / 5 + 32;
        bodies[selectedPlanet].surfaceTemperature = (getInput(prevVal) - 32) * 5 / 9 + 273;
    }
    else if(selectedIndex == 4) {//Vx
        prevVal = bodies[selectedPlanet].velocityX;
        bodies[selectedPlanet].velocityX = getInput(prevVal);
    }
    else if(selectedIndex == 5) {//Vy
        prevVal = bodies[selectedPlanet].velocityY;
        bodies[selectedPlanet].velocityY = getInput(prevVal);
    }
    else if(selectedIndex == 6) {//mass
        prevVal = bodies[selectedPlanet].mass;
        bodies[selectedPlanet].mass = getInput(prevVal);
    }
    else if(selectedIndex == 7) {//atmosphree
        prevVal = bodies[selectedPlanet].atmosphereDensity;
        bodies[selectedPlanet].atmosphereDensity = getInput(prevVal);
    }
    else if(selectedIndex == 8) {//water
        prevVal = bodies[selectedPlanet].waterAmount;
        bodies[selectedPlanet].waterAmount = getInput(prevVal);
    }
    else if(selectedIndex == 10) {// New Moon
        setAdvancedPlanet(num_bodies, bodies[selectedPlanet].x, bodies[selectedPlanet].y + bodies[selectedPlanet].radius * 10,bodies[selectedPlanet].velocityX + bodies[selectedPlanet].mass / 14, bodies[selectedPlanet].velocityY, 13891, 2106, getRandomName(), 1090, 12, 2000, false);
    }
    else if(selectedIndex == 11) {// Remove body
        removeBody(selectedPlanet);
    }
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

    if(kb_Data[2] & kb_Sto) {
        timeStep = 0;
        draw();
        setSelectedIndexValueByUserInput();
    }


    if((kb_Data[5] & kb_6)) {
        if(!upIndexerIsDown) {
            selectedIndex--;
        }
        upIndexerIsDown = true;
    } else {upIndexerIsDown = false;}

    if((kb_Data[5] & kb_3)) {
        if(!downIndexerIsDown) {
            selectedIndex++;
        }
        downIndexerIsDown = true;
    } else {downIndexerIsDown = false;}
}

/* Implement me! */
bool step(void)
{
    return !kb_On;
}

void draw_planet(uint16_t i) {
    gfx_SetColor(bodies[i].color);
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
    gfx_PrintUInt(bodies[selectedPlanet].surfaceTemperature, 1);
    gfx_PrintString("K");

    gfx_SetTextXY(SCREEN_X - 110, 40);

    gfx_PrintString("T:");
    gfx_PrintInt((bodies[selectedPlanet].surfaceTemperature - 273) * 9 / 5 + 32, 1);
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


    gfx_SetTextXY(SCREEN_X - 110, 90);
    gfx_PrintString("W:");
    gfx_PrintInt(bodies[selectedPlanet].waterAmount, 1);

    gfx_SetTextXY(SCREEN_X - 110, 100);
    gfx_PrintString("CT:");
    gfx_PrintInt64_t(bodies[selectedPlanet].coreTemperature, 1);
    gfx_PrintString("K");

    gfx_SetTextXY(SCREEN_X - 110, 110);
    gfx_PrintString("Create moon");

    gfx_SetTextXY(SCREEN_X - 110, 120);
    gfx_PrintString("Remove Body");
}

// Note that this also updates the planet color in the main view
// TODO: update all planet appearences, not just when selected
void drawPreviewPlanet() {
    if(bodies[selectedPlanet].moonLike) {
        gfx_TransparentSprite(lunarBase, 4, SCREEN_Y - 45);
    } else {
        gfx_TransparentSprite(dustyPlanet, 4, SCREEN_Y - 45);
    }
    bodies[selectedPlanet].color = 11;

    // Draw water
    if(bodies[selectedPlanet].atmosphereDensity > 50) {
        if(bodies[selectedPlanet].waterAmount > bodies[selectedPlanet].mass * 2) {
            gfx_TransparentSprite(waterAll, 4, SCREEN_Y - 45);
            bodies[selectedPlanet].color = 4;
        } else if(bodies[selectedPlanet].waterAmount > bodies[selectedPlanet].mass) {
            gfx_TransparentSprite(waterLots, 4, SCREEN_Y - 45);
            bodies[selectedPlanet].color = 4;
        } else if(bodies[selectedPlanet].waterAmount > bodies[selectedPlanet].mass / 2) {
            gfx_TransparentSprite(waterSome, 4, SCREEN_Y - 45);
            bodies[selectedPlanet].color = 4;
        } else if(bodies[selectedPlanet].waterAmount > bodies[selectedPlanet].mass / 4) {
            gfx_TransparentSprite(verySmallwater, 4, SCREEN_Y - 45);
            bodies[selectedPlanet].color = 4;
        } else if(bodies[selectedPlanet].waterAmount > bodies[selectedPlanet].mass / 12) {
            gfx_TransparentSprite(evenSmallerWater, 4, SCREEN_Y - 45);
            bodies[selectedPlanet].color = 4;
        }
    }


    // Ice-caps
    if(bodies[selectedPlanet].waterAmount > bodies[selectedPlanet].mass / 2) {
        if(bodies[selectedPlanet].surfaceTemperature < 225) {
            gfx_TransparentSprite(allIce, 4, SCREEN_Y - 45);
            bodies[selectedPlanet].color = 1;
        } else if(bodies[selectedPlanet].surfaceTemperature < 260) {
            gfx_TransparentSprite(mostIce, 4, SCREEN_Y - 45);
        } else if(bodies[selectedPlanet].surfaceTemperature < 280) {
            gfx_TransparentSprite(someIce, 4, SCREEN_Y - 45);
        } else if(bodies[selectedPlanet].surfaceTemperature < 310) {
            gfx_TransparentSprite(littleIce, 4, SCREEN_Y - 45);
        }
    }


    // Atmoshpere
    if(bodies[selectedPlanet].surfaceTemperature > 98) {
        if(bodies[selectedPlanet].atmosphereDensity > 50000) {
            gfx_TransparentSprite(gasGiantWarm, 4, SCREEN_Y - 45);
            bodies[selectedPlanet].color = 18;
        }
        else if(bodies[selectedPlanet].atmosphereDensity > 5000) {
            gfx_TransparentSprite(atmosphereDenseWarm, 4, SCREEN_Y - 45);
        }
    } else {
        if(bodies[selectedPlanet].atmosphereDensity > 50000) {
            gfx_TransparentSprite(gasGiantCold, 4, SCREEN_Y - 45);
            bodies[selectedPlanet].color = 4;
        } else if(bodies[selectedPlanet].atmosphereDensity > 5000) {
            gfx_TransparentSprite(atmosphereDenseCold, 4, SCREEN_Y - 45);
        }
    }


    // Draw bright object if the object is warm
    if(bodies[selectedPlanet].surfaceTemperature > 10000) {
        gfx_TransparentSprite(blueStar, 4, SCREEN_Y - 45);
        bodies[selectedPlanet].color = 3;
    } else if(bodies[selectedPlanet].surfaceTemperature > 3000) {
        gfx_TransparentSprite(midStar, 4, SCREEN_Y - 45);
        bodies[selectedPlanet].color = 18;
    } else if(bodies[selectedPlanet].surfaceTemperature > 2000) {
        bodies[selectedPlanet].color = 19;
        gfx_TransparentSprite(dimStar, 4, SCREEN_Y - 45);
    } else if(bodies[selectedPlanet].surfaceTemperature > 1000) {
        bodies[selectedPlanet].color = 19;
        gfx_TransparentSprite(planetWarm, 4, SCREEN_Y - 45);
    }

    drawSelectedIndex();
}

int64_t estimatedCoreTemp(int64_t mass) {
    if(mass < 3848) {
        return mass;
    } else if(mass < 6063)  {
        return mass / 80 + 3800;
    }

    return mass / 2868 + 6000;
}

void drawSelectedIndex() {
    gfx_SetColor(0);
    if(selectedIndex > 0) {
        gfx_Rectangle(SCREEN_X - 110, selectedIndex * 10 + 10, 110, 8);
    } else {
        gfx_Rectangle(SCREEN_X - 110, selectedIndex * 10, 110, 8);
    }
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

    drawPreviewPlanet();

    planetInfo();
}

void draw(void)
{
    gfx_FillScreen(2);
    //gfx_SetDefaultPalette(global_palette);
    for(uint16_t i = 0; i < num_bodies; i++) {
        draw_planet(i);
    }

    for(uint16_t i = maxBodies; i > maxBodies - num_debris; i--) {
        draw_planet(i);
    }

    gui();
}