
/* Derived from scene.c in the The OpenGL Programming Guide */
/* Keyboard and mouse rotation taken from Swiftless Tutorials #23 Part 2 */
/* http://www.swiftless.com/tutorials/opengl/camera2.html */

/* Frames per second code taken from : */
/* http://www.lighthouse3d.com/opengl/glut/index.php?fps */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include "graphics.h"

extern GLubyte world[WORLDX][WORLDY][WORLDZ];

/* mouse function called by GLUT when a button is pressed or released */
void mouse(int, int, int, int);

/* initialize graphics library */
extern void graphicsInit(int *, char **);

/* lighting control */
extern void setLightPosition(GLfloat, GLfloat, GLfloat);
extern GLfloat *getLightPosition();

/* viewpoint control */
extern void setViewPosition(float, float, float);
extern void getViewPosition(float *, float *, float *);
extern void getOldViewPosition(float *, float *, float *);
extern void setOldViewPosition(float, float, float);
extern void setViewOrientation(float, float, float);
extern void getViewOrientation(float *, float *, float *);

/* add cube to display list so it will be drawn */
extern void addDisplayList(int, int, int);

/* mob controls */
extern void createMob(int, float, float, float, float);
extern void setMobPosition(int, float, float, float, float);
extern void hideMob(int);
extern void showMob(int);

/* player controls */
extern void createPlayer(int, float, float, float, float);
extern void setPlayerPosition(int, float, float, float, float);
extern void hidePlayer(int);
extern void showPlayer(int);

/* tube controls */
extern void createTube(int, float, float, float, float, float, float, int);
extern void hideTube(int);
extern void showTube(int);

/* 2D drawing functions */
extern void draw2Dline(int, int, int, int, int);
extern void draw2Dbox(int, int, int, int);
extern void draw2Dtriangle(int, int, int, int, int, int);
extern void set2Dcolour(float[]);

/* flag which is set to 1 when flying behaviour is desired */
extern int flycontrol;
/* flag used to indicate that the test world should be used */
extern int testWorld;
/* flag to print out frames per second */
extern int fps;
/* flag to indicate the space bar has been pressed */
extern int space;
/* flag indicates the program is a client when set = 1 */
extern int netClient;
/* flag indicates the program is a server when set = 1 */
extern int netServer;
/* size of the window in pixels */
extern int screenWidth, screenHeight;
/* flag indicates if map is to be printed */
extern int displayMap;
/* flag indicates use of a fixed viewpoint */
extern int fixedVP;

/* frustum corner coordinates, used for visibility determination  */
extern float corners[4][3];

/* determine which cubes are visible e.g. in view frustum */
extern void ExtractFrustum();
extern void tree(float, float, float, float, float, float, int);

/* allows users to define colours */
extern int setUserColour(int, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat,
                         GLfloat, GLfloat, GLfloat);
void unsetUserColour(int);
extern void getUserColour(int, GLfloat *, GLfloat *, GLfloat *, GLfloat *,
                          GLfloat *, GLfloat *, GLfloat *, GLfloat *);

/********* end of extern variable declarations **************/


struct Point
{
   int x;
   int y;
   int z;
};
struct Wall
{
   struct Point StartPoint;
   int height;
   int width;
   int XorZSide; //0 = X side, 1 = Y side
   int color;
   int (*generateColorStyle)(int,int,int,int);
};
struct Room
{
   struct Point StartPoint;      //count from ground point
   struct Wall Walls[4];         // ID=0(WEST).1(EAST),2(SOUTH),3(NORTH)
   struct Point DoorPosition[4]; // if point = {-1,-1,-1} = not defined, ID=0(WEST).1(EAST),2(SOUTH),3(NORTH)
   int doorHeight;
   int doorWidth;
   int haveRoof;   // 0 = no, 1 = yes
   int haveGround; //0 = no, 1 = yes
   int Roofcolor;
   int Groundcolor;
};

#define WEST 0
#define EAST 1
#define SOUTH 2
#define NORTH 3
#define X_SIDE_WALL 0
#define Z_SIDE_WALL 1
#define NOT_HAVE_ROOF 0
#define NOT_HAVE_GROUND 0
#define HAVE_ROOF 1
#define HAVE_GROUND 1
void SetMAXandMINPoint(const int **MaxPoint, const int **MinPoint, const int *P1, const int *P2);
void BuildABox(const struct Point *StartPoint, const struct Point *Endpoint, int color,int (*generateColorStyle)(int,int,int,int));
void BuildAWall(const struct Wall *AWall);
struct Room BuildEasyRoom(const struct Point *StartPoint, int xLenght, int zLenght, int height, int haveRoof, int haveGround, int color);
void BuildARoom(const struct Room *ARoom);
void BuildDoorsWestVsEast(int roomID, struct Room *Rooms,int CorridorColor);
void BuildDoorsSouthVsNorth(int roomID, struct Room *Rooms,int CorridorColor);
#define READY 1
#define NOT_READY 0
int checkRoomAndOppositeRoomCanBuildCorridorAndHallway(int XorZSide,int roomID, struct Room *Rooms);


int normalColorStyle(int originalColor,int i,int j,int z);
int randomPinkWhiteStyle(int originalColor,int i,int j,int z);

#define AREA_XP 0
#define AREA_ZP 1
#define AREA_X_LENGHT 2
#define AREA_Z_LENGHT 3

// define the dimension of Grid 3X3
int dimensionOfGrid3x3[9][4] = {{0, 0, 33, 33},
                                {33, 0, 34, 33},
                                {67, 0, 33, 33},
                                {0, 33, 33, 34},
                                {33, 33, 34, 34},
                                {67, 33, 33, 34},
                                {0, 67, 33, 33},
                                {33, 67, 34, 33},
                                {67, 67, 33, 33}};

int DoorDirections[9][4] = {{-1, 1, -1, 3},
                            {0, 2, -1, 4},
                            {1, -1, -1, 5},
                            {-1, 4, 0, 6},
                            {3, 5, 1, 7},
                            {4, -1, 2, 8},
                            {-1, 7, 3, -1},
                            {6, 8, 4, -1},
                            {7, -1, 5, -1}};

/*** collisionResponse() ***/
/* -performs collision detection and response */
/*  sets new xyz  to position of the viewpoint after collision */
/* -can also be used to implement gravity by updating y position of vp*/
/* note that the world coordinates returned from getViewPosition()
      will be the negative value of the array indices */
void collisionResponse()
{

   /* your code for collisions goes here */
   float tempX=0.0;
   float tempY=0.0;
   float tempZ=0.0;
   float margin = 0.2;
   float vxp = 0;
   float vyp = 0;
   float vzp = 0;
   float Xdirection = 0; // - , + or 0
   float Zdirection = 0; // - , + or 0
   int floorLv = 20;
   int isHit = 0;
   int isAble2Jump = 0;
   int outOfSpace = 0;
   int SpaceOverHeadPos = 0;
   getViewPosition(&vxp, &vyp, &vzp);
   getOldViewPosition(&tempX, &tempY, &tempZ);
   Xdirection = -1 * (vxp > tempX) + (vxp < tempX);
   Zdirection = -1 * (vzp > tempZ) + (vzp < tempZ);
   int ValueAtCubePointDetect     = 0;
   int ValueAtMarginXOfCubeDetect = 0;
   int ValueAtMarginZOfCubeDetect = 0;
   int ValueAtMarginXZOfCubeDetect= 0;
   SpaceOverHeadPos = 1 + (-1) * (int)vyp;

   ValueAtCubePointDetect     = world[-1 * (int)vxp][-1 * (int)vyp][-1 * (int)vzp];
   ValueAtMarginXOfCubeDetect = world[-1 * (int)(vxp - margin * Xdirection)][-1 * (int)vyp][-1 * (int)vzp];
   ValueAtMarginZOfCubeDetect = world[-1 * (int)vxp][-1 * (int)vyp][-1 * (int)(vzp - margin * Zdirection)];
   ValueAtMarginXZOfCubeDetect= world[-1 * (int)(vxp - margin * Xdirection)][-1 * (int)vyp][-1 * (int)(vzp - margin * Zdirection)];
   isHit = ((ValueAtCubePointDetect + ValueAtMarginXOfCubeDetect + ValueAtMarginZOfCubeDetect + ValueAtMarginXZOfCubeDetect) > 0);
   isAble2Jump = world[-1 * (int)vxp][SpaceOverHeadPos][-1 * (int)vzp] +
                 world[-1 * (int)(vxp - margin * Xdirection)][SpaceOverHeadPos][-1 * (int)vzp] +
                 world[-1 * (int)vxp][SpaceOverHeadPos][-1 * (int)(vzp - margin * Zdirection)] +
                 world[-1 * (int)(vxp - margin * Xdirection)][SpaceOverHeadPos][-1 * (int)(vzp - margin * Zdirection)];
   outOfSpace = (((-1 * (int)vxp) > 99) || ((-1 * (int)vyp) > 49) || ((-1 * (int)vzp) > 99));
   outOfSpace = outOfSpace || (((-1 * (int)vxp) < 0) || ((-1 * (int)vyp) < (floorLv + 1)) || ((-1 * (int)vzp) < 0));
   // protect to pass through
   if ((outOfSpace == 1) || ((isHit != 0) && (isAble2Jump != 0)))
   {
      setViewPosition(tempX, tempY, tempZ);
   }
   // jump on the box
   else if ((isHit != 0) && (isAble2Jump == 0))
   {
      if (ValueAtCubePointDetect != 0)
      {
         tempX = (float)(int)vxp;
         tempZ = (float)(int)vzp;
      }
      else if (ValueAtMarginXOfCubeDetect != 0)
      {
         tempX = (float)(int)(vxp - margin * Xdirection);
         tempZ = (float)(int)vzp;
      }
      else if (ValueAtMarginZOfCubeDetect != 0)
      {
         tempX = (float)(int)vxp;
         tempZ = (float)(int)(vzp - margin * Zdirection);
      }
      else if (ValueAtMarginXZOfCubeDetect != 0)
      {
         tempX = (float)(int)(vxp - margin * Xdirection);
         tempZ = (float)(int)(vzp - margin * Zdirection);
      }
      setViewPosition(tempX, tempY - 1, tempZ);
   }
}

/******* draw2D() *******/
/* draws 2D shapes on screen */
/* use the following functions:        */
/* draw2Dline(int, int, int, int, int);      */
/* draw2Dbox(int, int, int, int);         */
/* draw2Dtriangle(int, int, int, int, int, int);   */
/* set2Dcolour(float []);           */
/* colour must be set before other functions are called  */
void draw2D()
{

   if (testWorld)
   {
      /* draw some sample 2d shapes */
      if (displayMap == 1)
      {
         GLfloat green[] = {0.0, 0.5, 0.0, 0.5};
         set2Dcolour(green);
         draw2Dline(0, 0, 500, 500, 15);
         draw2Dtriangle(0, 0, 200, 200, 0, 200);

         GLfloat black[] = {0.0, 0.0, 0.0, 0.5};
         set2Dcolour(black);
         draw2Dbox(500, 380, 524, 388);
      }
   }
   else
   {

      /* your code goes here */
   }
}


/*** update() ***/
/* background process, it is called when there are no other events */
/* -used to control animations and perform calculations while the  */
/*  system is running */
/* -gravity must also implemented here, duplicate collisionResponse */
void update()
{
   int i, j, k;
   float *la;
   float x, y, z;

   /* sample animation for the testworld, don't remove this code */
   /* demo of animating mobs */
   if (testWorld)
   {

      /* update old position so it contains the correct value */
      /* -otherwise view position is only correct after a key is */
      /*  pressed and keyboard() executes. */
#if 0
// Fire a ray in the direction of forward motion
float xx, yy, zz;
getViewPosition(&x, &y, &z);
getOldViewPosition(&xx, &yy, &zz);
printf("%f %f %f %f %f %f\n", xx, yy, zz, x, y, z);
printf("%f %f %f\n",  -xx+((x-xx)*25.0), -yy+((y-yy)*25.0), -zz+((z-zz)*25.0));
createTube(2, -xx, -yy, -zz, -xx-((x-xx)*25.0), -yy-((y-yy)*25.0), -zz-((z-zz)*25.0), 5);
#endif

      getViewPosition(&x, &y, &z);
      setOldViewPosition(x, y, z);

      /* sample of rotation and positioning of mob */
      /* coordinates for mob 0 */
      static float mob0x = 50.0, mob0y = 25.0, mob0z = 52.0;
      static float mob0ry = 0.0;
      static int increasingmob0 = 1;
      /* coordinates for mob 1 */
      static float mob1x = 50.0, mob1y = 25.0, mob1z = 52.0;
      static float mob1ry = 0.0;
      static int increasingmob1 = 1;
      /* counter for user defined colour changes */
      static int colourCount = 0;
      static GLfloat offset = 0.0;

      /* move mob 0 and rotate */
      /* set mob 0 position */
      setMobPosition(0, mob0x, mob0y, mob0z, mob0ry);

      /* move mob 0 in the x axis */
      if (increasingmob0 == 1)
         mob0x += 0.2;
      else
         mob0x -= 0.2;
      if (mob0x > 50)
         increasingmob0 = 0;
      if (mob0x < 30)
         increasingmob0 = 1;

      /* rotate mob 0 around the y axis */
      mob0ry += 1.0;
      if (mob0ry > 360.0)
         mob0ry -= 360.0;

      /* move mob 1 and rotate */
      setMobPosition(1, mob1x, mob1y, mob1z, mob1ry);

      /* move mob 1 in the z axis */
      /* when mob is moving away it is visible, when moving back it */
      /* is hidden */
      if (increasingmob1 == 1)
      {
         mob1z += 0.2;
         showMob(1);
      }
      else
      {
         mob1z -= 0.2;
         hideMob(1);
      }
      if (mob1z > 72)
         increasingmob1 = 0;
      if (mob1z < 52)
         increasingmob1 = 1;

      /* rotate mob 1 around the y axis */
      mob1ry += 1.0;
      if (mob1ry > 360.0)
         mob1ry -= 360.0;

      /* change user defined colour over time */
      if (colourCount == 1)
         offset += 0.05;
      else
         offset -= 0.01;
      if (offset >= 0.5)
         colourCount = 0;
      if (offset <= 0.0)
         colourCount = 1;
      setUserColour(9, 0.7, 0.3 + offset, 0.7, 1.0, 0.3, 0.15 + offset, 0.3, 1.0);

   /* sample tube creation  */
      /* draws a purple tube above the other sample objects */
      createTube(1, 45.0, 30.0, 45.0, 50.0, 30.0, 50.0, 6);

      /* end testworld animation */
   }
   else
   {

      /* your code goes here */

      int valOfWorldAtBelowVP = 0;
      float vpx = 0;
      float vpy = 0;
      float vpz = 0;
      float gravityVal = 21;
      float newY = 0;
      float floorLv = 20;
      static int fallState = 0;
      // yn = y(n-1) - 0.5*g*(tn^2-t(n-1)^2)
      static clock_t timeCount, clkRef;
      static float timePast, timeCurrent;
      getViewPosition(&vpx, &vpy, &vpz);
      valOfWorldAtBelowVP = world[-1 * (int)vpx][(-1 * (int)vpy) - 1][-1 * (int)vpz];
      timeCount = clock();

      if (fallState == 0)
      {
         fallState = (valOfWorldAtBelowVP == 0);
      }
      else if (fallState == 1)
      {
         clkRef = clock();
         timePast = 0;
         timeCurrent = 0;
         fallState =2 ;
      }
      else if (fallState == 2)
      {

         timePast = timeCurrent;
         timeCurrent = (float)(clock() - clkRef)/CLOCKS_PER_SEC;
         newY = (-1.0)*vpy - gravityVal*(timeCurrent*timeCurrent-timePast*timePast);
         
         if (valOfWorldAtBelowVP != 0) 
         {
            floorLv = (-1*(int)vpy)-1;
         }

         if ((valOfWorldAtBelowVP != 0) && ((((float)((int)vpy)) - vpy) <= 0.0))
         {
            fallState = 0;
            newY = floorLv + 1.0;
         }
         if ((newY < (floorLv + 1)) || (newY > 49.0))
            newY = floorLv + 1.0;
         setViewPosition(vpx, newY * (-1.0), vpz);
      }
   }
}

/* called by GLUT when a mouse button is pressed or released */
/* -button indicates which button was pressed or released */
/* -state indicates a button down or button up event */
/* -x,y are the screen coordinates when the mouse is pressed or */
/*  released */
void mouse(int button, int state, int x, int y)
{

   if (button == GLUT_LEFT_BUTTON)
      printf("left button - ");
   else if (button == GLUT_MIDDLE_BUTTON)
      printf("middle button - ");
   else
      printf("right button - ");

   if (state == GLUT_UP)
      printf("up - ");
   else
      printf("down - ");

   printf("%d %d\n", x, y);
}

void makeWorld()
{
   int i, j, k;
   for (i = 0; i < WORLDX; i++)
      for (j = 0; j < WORLDY; j++)
         for (k = 0; k < WORLDZ; k++)
            world[i][j][k] = 0;
}


int getRandomNumber(int minimum, int maximum)
{
   return minimum + rand() % (maximum + 1 - minimum);
}


int main(int argc, char **argv)
{
   int i, j, k;
   /* initialize the graphics system */
   graphicsInit(&argc, argv);
   srand(time(NULL));

   /* the first part of this if statement builds a sample */
   /* world which will be used for testing */
   /* DO NOT remove this code. */
   /* Put your code in the else statment below */
   /* The testworld is only guaranteed to work with a world of
      with dimensions of 100,50,100. */
   if (testWorld == 1)
   {
      /* initialize world to empty */
      for (i = 0; i < WORLDX; i++)
         for (j = 0; j < WORLDY; j++)
            for (k = 0; k < WORLDZ; k++)
               world[i][j][k] = 0;

      /* some sample objects */
      /* build a red platform */
      for (i = 0; i < WORLDX; i++)
      {
         for (j = 0; j < WORLDZ; j++)
         {
            world[i][24][j] = 3;
         }
      }
      /* create some green and blue cubes */
      world[50][25][50] = 1;
      world[49][25][50] = 1;
      world[49][26][50] = 1;
      world[52][25][52] = 2;
      world[52][26][52] = 2;

   /* create user defined colour and draw cube */
      setUserColour(9, 0.7, 0.3, 0.7, 1.0, 0.3, 0.15, 0.3, 1.0);
      world[54][25][50] = 9;

      /* blue box shows xy bounds of the world */
      for (i = 0; i < WORLDX - 1; i++)
      {
         world[i][25][0] = 2;
         world[i][25][WORLDZ - 1] = 2;
      }
      for (i = 0; i < WORLDZ - 1; i++)
      {
         world[0][25][i] = 2;
         world[WORLDX - 1][25][i] = 2;
      }

      /* create two sample mobs */
      /* these are animated in the update() function */
      createMob(0, 50.0, 25.0, 52.0, 0.0);
      createMob(1, 50.0, 25.0, 52.0, 0.0);

      /* create sample player */
      createPlayer(0, 52.0, 27.0, 52.0, 0.0);
   }
   else
   {

      /* your code to build the world goes here */

      int GroundLv = 20;
      flycontrol = 0;
      makeWorld();

      //ground custom colors (brown and light brown)
      //light brown color
      setUserColour(20, 0.724, 0.404, 0.116, 1.0, 0.2, 0.2, 0.2, 1.0);
      //dark brown
      setUserColour(21, 0.404, 0.268, 0.132, 1.0, 0.2, 0.2, 0.2, 1.0);


      for (i = 0; i < WORLDX; i++)
      {
         for (j = 0; j < WORLDZ; j++)
         {
            //world[i][GroundLv][j] = 4*(i%(1+(i+j)%4) <1) + 7*(i%(1+(i+j)%4) >=1) ; // dark and light blown , mathematic pattern
            world[i][GroundLv][j] = (((j%2==0)&&(i%2==1))||((j%2==1)&&(i%2==0)))?20:21;  // chessboard pattern
         }
      }

      int l,m,n,o,p;
      int xLenght = 0;
      int zLenght = 0;
      int roomX = 0;
      int roomZ = 0;
      int wallHeight = 3;
      int xViewP = 0;
      int zViewP = 0;
      int yStartP = GroundLv+1;
      int numberofCubes = 2;
      int colorID1 = 3;
      int corridorColor = 5; // Pink
      const struct Point DoorInitialPoint = {-1, -1, -1};
      const int numRoom = 9;
      const int doorWidth = 2;
      const int wallColor = 1;                 // Green
    //  int ViewPointID = getRandomNumber(0, 8); //which room the view point will be
       int ViewPointID = 2;
      int doorHeight = 2;
      int oppositeRoomID = -1;
      int sparForCorridorsX = 4;
      int sparForCorridorsZ = 4;
      int sparForRoomSizeMax = 24;  // sparForRoomSize >= 3+doorWidth
      int sparForRoomSizeMin = 8;
      struct Room Rooms[9];
      
      // Initial value or set default value to Door position Point (X,Y,Z)
      for(k=0;k<numRoom;k++)
         for (i = WEST; i <=NORTH;i++)
            Rooms[k].DoorPosition[i] = DoorInitialPoint;
      // Build 9 Rooms
      for(k = 0;k<numRoom;k++)
      {
         // Find spar area between 2 rooms to build corridors and hallways
         sparForCorridorsX = (dimensionOfGrid3x3[k][AREA_X_LENGHT] - sparForRoomSizeMax)/2;
         sparForCorridorsZ = (dimensionOfGrid3x3[k][AREA_Z_LENGHT] - sparForRoomSizeMax)/2;
         // define door direction to Room attribute

         // random size of a room
         xLenght = getRandomNumber(sparForRoomSizeMin,sparForRoomSizeMax);
         zLenght = getRandomNumber(sparForRoomSizeMin,sparForRoomSizeMax);
         // Random location of a room
         roomX = sparForCorridorsX + dimensionOfGrid3x3[k][AREA_XP] + getRandomNumber(0,dimensionOfGrid3x3[k][AREA_X_LENGHT]-xLenght-sparForCorridorsX-sparForCorridorsX);
         roomZ=  sparForCorridorsZ + dimensionOfGrid3x3[k][AREA_ZP] + getRandomNumber(0,dimensionOfGrid3x3[k][AREA_Z_LENGHT]-zLenght-sparForCorridorsZ-sparForCorridorsZ);

         // Reference point to build a room
         const struct Point RoomStartPoint = {roomX,GroundLv,roomZ};
         // find view point
         if (k == ViewPointID)
         {
            xViewP = 2 + RoomStartPoint.x + getRandomNumber(0,xLenght-4);
            zViewP = 2 + RoomStartPoint.z + getRandomNumber(0,zLenght-4);
         }
         // create a few Cubes 1 high
         numberofCubes = getRandomNumber(1,2);
         for(i = 0; i < numberofCubes;i++)
            world[2 + roomX + getRandomNumber(0,xLenght-4)][yStartP][2 + roomZ  + getRandomNumber(0,zLenght -4)]= 8;
         
         // build A room
         Rooms[k]= BuildEasyRoom(&RoomStartPoint,xLenght,zLenght,wallHeight,HAVE_ROOF,NOT_HAVE_GROUND,wallColor);

         // Set and Build Doors
         Rooms[k].doorWidth = doorWidth;
         Rooms[k].doorHeight = doorHeight;

         // build door and corridor for 4 directions
         // east to west
         BuildDoorsWestVsEast(k,Rooms,corridorColor);
         BuildDoorsSouthVsNorth(k,Rooms,corridorColor+3);
      }

    setViewPosition(-1 * xViewP, -1 * yStartP, -1 * zViewP);

      //setViewPosition(-1*xViewP, -1*48, -1*zViewP);
   }

   /* starts the graphics processing loop */
   /* code after this will not run until the program exits */
   glutMainLoop();
   return 0;
}


void SetMAXandMINPoint(const int** MaxPoint,const int** MinPoint,const int* P1,const int* P2)
{
   if(*P1 > *P2)
   {
      *MaxPoint = P1;
      *MinPoint = P2;
   }
   else
   {
      *MaxPoint = P2;
      *MinPoint = P1;
   }
}

void BuildABox(const struct Point *StartPoint,const struct Point *Endpoint,int color,int (*generateColorStyle)(int,int,int,int))
{
   const int *MaxX;
   const int *MaxY;
   const int *MaxZ;
   const int *MinX;
   const int *MinY;
   const int *MinZ;
   int i,j,k;
   SetMAXandMINPoint(&MaxX,&MinX,&(StartPoint->x),&(Endpoint->x));
   SetMAXandMINPoint(&MaxY,&MinY,&(StartPoint->y),&(Endpoint->y));
   SetMAXandMINPoint(&MaxZ,&MinZ,&(StartPoint->z),&(Endpoint->z));
   for(k=*MinZ;k<=*MaxZ;k++)
      for(j=*MinY;j<=*MaxY;j++)
         for(i=*MinX;i<=*MaxX;i++)
            world[i][j][k] = generateColorStyle(color,i,j,k);
}
/*
struct Wall
{
   struct Point StartPoint;
   int height;
   int width;
   int XorZSide; //0 = X side, 1 = Y side
   int color;
};*/
void BuildAWall(const struct Wall *AWall)
{

   struct Point EndPoint = {0,0,0};
   if(AWall->XorZSide == 0) //0 = X side, 1 = Y side  // Z is not changed
   {
      EndPoint.x = AWall->StartPoint.x + AWall->width - 1; // for example start X=1 ,length =3 --> There are 3 points (X = 1, X=2, X=3) --> End X at value = 1 + 3 -1 =3
      EndPoint.z = AWall->StartPoint.z;
   }
   else
   {
      EndPoint.x = AWall->StartPoint.x;
      EndPoint.z = AWall->StartPoint.z + AWall->width - 1; //  same idea above (Xpoint )
   }
   EndPoint.y = AWall->StartPoint.y+AWall->height -1; //  same idea above (Xpoint )
   BuildABox(&(AWall->StartPoint),&EndPoint,AWall->color,AWall->generateColorStyle);
   
}

void BuildARoom(const struct Room *ARoom)
{
   int i = WEST;
   struct Point StartPoint = {0,0,0};
   struct Point EndPoint= {0,0,0};
   for(i=WEST;i<=NORTH;i++)
   BuildAWall(&(ARoom->Walls[i]));

   // Build Roof
   if (ARoom->haveRoof == HAVE_ROOF)
   {
      StartPoint.x = ARoom->StartPoint.x;
      StartPoint.y = ARoom->StartPoint.y + ARoom->Walls[WEST].height+1; // Assume that all rooms have the same height, not count ground LV.
      StartPoint.z = ARoom->StartPoint.z;

      EndPoint.x = ARoom->StartPoint.x + ARoom->Walls[SOUTH].width+1;
      EndPoint.y = ARoom->StartPoint.y + ARoom->Walls[WEST].height+1;// Assume that all rooms have the same height, not count ground LV.
      EndPoint.z = ARoom->StartPoint.z + ARoom->Walls[WEST].width-1;
      BuildABox(&StartPoint,&EndPoint,ARoom->Roofcolor,&normalColorStyle);
   }
   // Build Ground
   if (ARoom->haveGround == HAVE_GROUND)
   {
      EndPoint.x = ARoom->StartPoint.x + ARoom->Walls[SOUTH].width +1;
      EndPoint.y = ARoom->StartPoint.y;
      EndPoint.z = ARoom->StartPoint.z + ARoom->Walls[WEST].width -1;
      BuildABox(&(ARoom->StartPoint),&EndPoint,ARoom->Groundcolor,&normalColorStyle);
   }
}

struct Room BuildEasyRoom(const struct Point *StartPoint,int xLenght,int zLenght,int height,int haveRoof,int haveGround,int color)
{


/*   Top View of Room

                     North Wall side


                          |--- X- Width        --|
                   ____ _______________________ ____
               |  |    |_______________________|    | 
                  |    |                       |    | 
               |  |    |                       |    | 
               _  |    |                       |    | 
               Z  |    |                       |    | 
                  |    |                       |    | 
               W  |    |                       |    | 
West Wall Side i  |    |                       |    |                EAST Wall Side
               d  |    |                       |    | 
               t  |    |                       |    | 
               h_ |    |                       |    | 
               |  |    |                       |    | 
                  |    |                       |    | 
               |  |    |                       |    | 
                  |    |                       |    | 
               |  |    |                       |    |          
                   ____                         ____
               |  |    |_______________________|    | 
                  _____ _______________________ _____


                           SOUTH  wall Side
*/

   struct Room ARoom;
   int x = StartPoint->x;
   int y = StartPoint->y;
   int z = StartPoint->z;


   // Set Attribute to Wall 
   ARoom.StartPoint            = *StartPoint;
   ARoom.Groundcolor           = color;
   ARoom.Roofcolor             = color;
   ARoom.haveRoof              = haveRoof;
   ARoom.haveGround            = haveGround;
   // the wall at West side
   ARoom.Walls[WEST].StartPoint.x = x;                            
   ARoom.Walls[WEST].StartPoint.y = y+1;
   ARoom.Walls[WEST].StartPoint.z = z;
   ARoom.Walls[WEST].height       = height;
   ARoom.Walls[WEST].width        = zLenght;
   ARoom.Walls[WEST].XorZSide     = Z_SIDE_WALL;
   ARoom.Walls[WEST].color        = color;
   ARoom.Walls[WEST].generateColorStyle = &normalColorStyle;

   // the wall at East side
   ARoom.Walls[EAST].StartPoint.x = x + xLenght - 1;
   ARoom.Walls[EAST].StartPoint.y = y+1;
   ARoom.Walls[EAST].StartPoint.z = z;
   ARoom.Walls[EAST].height       = height;
   ARoom.Walls[EAST].width        = zLenght;
   ARoom.Walls[EAST].XorZSide     = Z_SIDE_WALL;
   ARoom.Walls[EAST].color        = color;
   ARoom.Walls[EAST].generateColorStyle = &normalColorStyle;

   // the wall at South side
   ARoom.Walls[SOUTH].StartPoint.x = x + 1;
   ARoom.Walls[SOUTH].StartPoint.y = y+1;
   ARoom.Walls[SOUTH].StartPoint.z = z;
   ARoom.Walls[SOUTH].height       = height;
   ARoom.Walls[SOUTH].width        = xLenght - 2;
   ARoom.Walls[SOUTH].XorZSide     = X_SIDE_WALL;
   ARoom.Walls[SOUTH].color        = color;
   ARoom.Walls[SOUTH].generateColorStyle = &normalColorStyle;


   // the wall at North side
   ARoom.Walls[NORTH].StartPoint.x = x + 1;
   ARoom.Walls[NORTH].StartPoint.y = y+1;
   ARoom.Walls[NORTH].StartPoint.z = z + zLenght -1;
   ARoom.Walls[NORTH].height       = height;
   ARoom.Walls[NORTH].width        = xLenght - 2;
   ARoom.Walls[NORTH].XorZSide     = X_SIDE_WALL;
   ARoom.Walls[NORTH].color        = color;
   ARoom.Walls[NORTH].generateColorStyle = &normalColorStyle;
   BuildARoom(&ARoom);
   return ARoom;
}
/*

void BuildDoorsSouthVsNorth(int roomID, struct Room *Rooms);
#define READY 1
#define NOT_READY 0
int checkRoomAndOppositeRoomCanBuildCorridorAndHallway(int XorZSide,int roomID, struct Room *Rooms);

*/

void BuildDoorsWestVsEast(int roomID, struct Room *Rooms,int CorridorColor)
{
      int i=0,j=0;
      struct Point DoorOfAroomPoint = {-1,-1,-1};
      struct Point DoorOfOppositeRoomPoint = {-1,-1,-1};
      int doorWidth = 0;
      int doorHeight = 0;
      int color = CorridorColor;
      struct Room *Aroom = NULL;
      struct Room *OpposisteRoom = NULL;
      int oppositeRoomID = -1;
      int AroomDirection = WEST;
      int OppositeRoomDirection = EAST;
      for(AroomDirection = WEST ; AroomDirection <= EAST;AroomDirection++)
      {
         if(DoorDirections[roomID][AroomDirection] >= 0)
         { 
            oppositeRoomID = DoorDirections[roomID][AroomDirection];
            OppositeRoomDirection = (AroomDirection+1)%2; // Becasue there are only 2 direnctions: WEST(0) and EAST(1)
            Aroom = &Rooms[roomID];
            OpposisteRoom = &Rooms[oppositeRoomID];
            doorWidth = Aroom->doorWidth;
            doorHeight = Aroom->doorHeight;
            oppositeRoomID = DoorDirections[roomID][AroomDirection];
            DoorOfOppositeRoomPoint = OpposisteRoom->DoorPosition[OppositeRoomDirection];
            Aroom->DoorPosition[AroomDirection]= Aroom->Walls[AroomDirection].StartPoint;
            i = 30; // Protect infinity loop
            do
            {
               Aroom->DoorPosition[AroomDirection].z = Aroom->Walls[AroomDirection].StartPoint.z + getRandomNumber(1, Aroom->Walls[AroomDirection].width - Aroom->doorWidth - 1);
            } while ((Aroom->DoorPosition[AroomDirection].z == DoorOfOppositeRoomPoint.z) && (i-- >= 0));

            DoorOfAroomPoint = Aroom->DoorPosition[AroomDirection];
            const struct Wall ADoor = {DoorOfAroomPoint,Aroom->doorHeight,Aroom->doorWidth,Z_SIDE_WALL,0,&normalColorStyle};
            BuildAWall(&ADoor);
         }
         if((DoorOfAroomPoint.x > -1)&&(DoorOfOppositeRoomPoint.x> -1) &&(AroomDirection < OppositeRoomDirection))
         {
            int turningPointX = (dimensionOfGrid3x3[oppositeRoomID][AREA_XP] + dimensionOfGrid3x3[oppositeRoomID][AREA_X_LENGHT]) - (doorWidth / 2);

            struct Point AnOppositeCorridorRoofStartPoint = {turningPointX + doorWidth,DoorOfAroomPoint.y + doorHeight,DoorOfAroomPoint.z - 1};
            struct Point AnOppositeCorridorRoofEndPoint =   {DoorOfAroomPoint.x-1 ,DoorOfAroomPoint.y + doorHeight,DoorOfAroomPoint.z +doorWidth};
            struct Point ACorridorRoofStartPoint= {1+DoorOfOppositeRoomPoint.x,DoorOfAroomPoint.y + doorHeight,DoorOfOppositeRoomPoint.z-1};
            struct Point ACorridorRoofEndPoint  = {turningPointX-2,DoorOfAroomPoint.y + doorHeight,DoorOfOppositeRoomPoint.z+doorWidth};
            struct Point AMiddleCorridorRoofStartPoint = {turningPointX-1,DoorOfAroomPoint.y + doorHeight,DoorOfOppositeRoomPoint.z - 1};
            struct Point AMiddleCorridorRoofEndPoint = {turningPointX+doorWidth,DoorOfAroomPoint.y + doorHeight,DoorOfAroomPoint.z+doorWidth};



            struct Wall ALongCorridorWall ={{turningPointX,
                                             DoorOfAroomPoint.y,
                                             DoorOfAroomPoint.z+ doorWidth},
                                             doorHeight,
                                             DoorOfAroomPoint.x - turningPointX,
                                             X_SIDE_WALL,
                                             CorridorColor,
                                             &randomPinkWhiteStyle};
            struct Wall AShortOppositeCorridorWall ={{DoorOfOppositeRoomPoint.x+ 1,
                                                      DoorOfOppositeRoomPoint.y,
                                                      DoorOfOppositeRoomPoint.z + doorWidth},
                                                      doorHeight,
                                                      turningPointX - 1 - DoorOfOppositeRoomPoint.x,
                                                      X_SIDE_WALL,
                                                      CorridorColor,
                                                      &randomPinkWhiteStyle};
            struct Wall ALongOppositeCorridorWall = {{DoorOfOppositeRoomPoint.x+ 1,
                                                      DoorOfOppositeRoomPoint.y,
                                                      DoorOfOppositeRoomPoint.z-1},
                                                      doorHeight,
                                                      doorWidth + AShortOppositeCorridorWall.width,
                                                      X_SIDE_WALL,
                                                      CorridorColor,
                                                      &randomPinkWhiteStyle};

            struct Wall AShortCorridorWall = {{turningPointX + doorWidth,
                                               DoorOfAroomPoint.y,
                                               DoorOfAroomPoint.z-1},
                                               doorHeight,
                                               DoorOfAroomPoint.x - turningPointX - doorWidth,
                                               X_SIDE_WALL,
                                               CorridorColor,
                                               &randomPinkWhiteStyle};


            struct Wall AnOppositeZSIDECorridorWall= {{turningPointX - 1,
                                                       DoorOfOppositeRoomPoint.y,
                                                       DoorOfOppositeRoomPoint.z + doorWidth+1},
                                                       doorHeight,
                                                       DoorOfAroomPoint.z - DoorOfOppositeRoomPoint.z,
                                                       Z_SIDE_WALL,
                                                       CorridorColor,
                                                       &randomPinkWhiteStyle};


            struct Wall AZSIDECorridorWall = {{turningPointX + doorWidth,
                                               DoorOfAroomPoint.y,
                                               DoorOfOppositeRoomPoint.z-1},
                                               doorHeight,
                                               DoorOfAroomPoint.z - DoorOfOppositeRoomPoint.z,
                                               Z_SIDE_WALL,
                                               CorridorColor,
                                               &randomPinkWhiteStyle};

            if (DoorOfAroomPoint.z < DoorOfOppositeRoomPoint.z)
            {
               ALongCorridorWall.StartPoint.z = DoorOfAroomPoint.z - 1;
               AShortOppositeCorridorWall.StartPoint.z = DoorOfOppositeRoomPoint.z- 1;
               ALongOppositeCorridorWall.StartPoint.z = DoorOfOppositeRoomPoint.z + doorWidth; 
               AShortCorridorWall.StartPoint.z = DoorOfAroomPoint.z + doorWidth;
               AnOppositeZSIDECorridorWall.StartPoint.x = turningPointX  + doorWidth; 
               AnOppositeZSIDECorridorWall.StartPoint.z = DoorOfAroomPoint.z + doorWidth;
               AnOppositeZSIDECorridorWall.width =  1 + DoorOfOppositeRoomPoint.z - DoorOfAroomPoint.z;
               AZSIDECorridorWall.StartPoint.x = turningPointX -1; 
               AZSIDECorridorWall.StartPoint.z = DoorOfAroomPoint.z - 1;
               AZSIDECorridorWall.width = AnOppositeZSIDECorridorWall.width;
            }

            // Build Roof for Hallway
            if (DoorOfOppositeRoomPoint.z > DoorOfAroomPoint.z)
            {
               AMiddleCorridorRoofEndPoint.z  =  DoorOfOppositeRoomPoint.z +doorWidth;
               AMiddleCorridorRoofStartPoint.z = DoorOfAroomPoint.z -1;
            }

            BuildAWall(&AnOppositeZSIDECorridorWall);
            BuildAWall(&AZSIDECorridorWall);
            BuildAWall(&AShortCorridorWall);
            BuildAWall(&ALongCorridorWall);
            BuildAWall(&AShortOppositeCorridorWall);
            BuildAWall(&ALongOppositeCorridorWall);
            
            // Build Roofs of Corridors
            BuildABox(&AMiddleCorridorRoofStartPoint,&AMiddleCorridorRoofEndPoint,color,&randomPinkWhiteStyle);
            BuildABox(&ACorridorRoofStartPoint,&ACorridorRoofEndPoint,color,&randomPinkWhiteStyle);
            BuildABox(&AnOppositeCorridorRoofStartPoint,&AnOppositeCorridorRoofEndPoint,color,&randomPinkWhiteStyle);
      } 
   }
}
void BuildDoorsSouthVsNorth(int roomID, struct Room *Rooms,int CorridorColor)
{

      int i=0,j=0;
      struct Point DoorOfAroomPoint = {-1,-1,-1};
      struct Point DoorOfOppositeRoomPoint = {-1,-1,-1};
      int doorWidth = 0;
      int doorHeight = 0;
      int color = CorridorColor;
      struct Room *Aroom = NULL;
      struct Room *OpposisteRoom = NULL;
      int oppositeRoomID = -1;
      int AroomDirection = SOUTH;
      int OppositeRoomDirection = NORTH;
      for(AroomDirection = SOUTH ; AroomDirection <= NORTH;AroomDirection++)
   {
      if(DoorDirections[roomID][AroomDirection] >= 0)
      { 
         oppositeRoomID = DoorDirections[roomID][AroomDirection];
         OppositeRoomDirection = 2+(AroomDirection-1)%2; // Becasue there are only 2 direnctions: SOUTH(2) and NORTH(3)
         Aroom = &Rooms[roomID];
         OpposisteRoom = &Rooms[oppositeRoomID];
         doorWidth = Aroom->doorWidth;
         doorHeight = Aroom->doorHeight;
         oppositeRoomID = DoorDirections[roomID][AroomDirection];
         DoorOfOppositeRoomPoint = OpposisteRoom->DoorPosition[OppositeRoomDirection];
         Aroom->DoorPosition[AroomDirection] = Aroom->Walls[AroomDirection].StartPoint;

         i = 30; // Protect infinity loop
         do
         {
            Aroom->DoorPosition[AroomDirection].x = Aroom->Walls[AroomDirection].StartPoint.x + getRandomNumber(0, Aroom->Walls[AroomDirection].width - Aroom->doorWidth - 1);
         } while ((Aroom->DoorPosition[AroomDirection].x == DoorOfOppositeRoomPoint.x) && (i-- >= 0));

         DoorOfAroomPoint = Aroom->DoorPosition[AroomDirection];
         const struct Wall ADoor = {DoorOfAroomPoint,Aroom->doorHeight,Aroom->doorWidth,X_SIDE_WALL,0,&normalColorStyle};
         BuildAWall(&ADoor);
      }
      // south and north side

      if((DoorOfAroomPoint.z > -1)&&(DoorOfOppositeRoomPoint.z > -1) &&(AroomDirection < OppositeRoomDirection))
      {

            int turningPointZ = (dimensionOfGrid3x3[oppositeRoomID][AREA_ZP] + dimensionOfGrid3x3[oppositeRoomID][AREA_Z_LENGHT]) - (doorWidth / 2);

            struct Point AnOppositeCorridorRoofStartPoint = {DoorOfAroomPoint.x - 1,DoorOfAroomPoint.y + doorHeight,turningPointZ + doorWidth};
            struct Point AnOppositeCorridorRoofEndPoint =   {DoorOfAroomPoint.x +doorWidth ,DoorOfAroomPoint.y + doorHeight,DoorOfAroomPoint.z-1};
            struct Point ACorridorRoofStartPoint= {DoorOfOppositeRoomPoint.x-1,DoorOfAroomPoint.y + doorHeight,1+DoorOfOppositeRoomPoint.z};
            struct Point ACorridorRoofEndPoint  = {DoorOfOppositeRoomPoint.x+doorWidth,DoorOfAroomPoint.y + doorHeight,turningPointZ-2};
            struct Point AMiddleCorridorRoofStartPoint = {DoorOfOppositeRoomPoint.x - 1,DoorOfAroomPoint.y + doorHeight,turningPointZ-1};
            struct Point AMiddleCorridorRoofEndPoint = {DoorOfAroomPoint.x+doorWidth,DoorOfAroomPoint.y + doorHeight,turningPointZ+doorWidth};



            struct Wall ALongCorridorWall ={{DoorOfAroomPoint.x+ doorWidth,
                                             DoorOfAroomPoint.y,
                                             turningPointZ},
                                             doorHeight,
                                             DoorOfAroomPoint.z - turningPointZ,
                                             Z_SIDE_WALL,
                                             CorridorColor,
                                             &randomPinkWhiteStyle};
            struct Wall AShortOppositeCorridorWall ={{DoorOfOppositeRoomPoint.x + doorWidth,
                                                      DoorOfOppositeRoomPoint.y,
                                                      DoorOfOppositeRoomPoint.z+ 1},
                                                      doorHeight,
                                                      turningPointZ - 1 - DoorOfOppositeRoomPoint.z,
                                                      Z_SIDE_WALL,
                                                      CorridorColor,
                                                      &randomPinkWhiteStyle};
            struct Wall ALongOppositeCorridorWall = {{DoorOfOppositeRoomPoint.x-1,
                                                      DoorOfOppositeRoomPoint.y,
                                                      DoorOfOppositeRoomPoint.z+ 1},
                                                      doorHeight,
                                                      doorWidth + AShortOppositeCorridorWall.width,
                                                      Z_SIDE_WALL,
                                                      CorridorColor,
                                                      &randomPinkWhiteStyle};

            struct Wall AShortCorridorWall = {{DoorOfAroomPoint.x-1,
                                               DoorOfAroomPoint.y,
                                               turningPointZ + doorWidth},
                                               doorHeight,
                                               DoorOfAroomPoint.z - turningPointZ - doorWidth,
                                               Z_SIDE_WALL,
                                               CorridorColor,
                                               &randomPinkWhiteStyle};


            struct Wall AnOppositeZSIDECorridorWall= {{DoorOfOppositeRoomPoint.x + doorWidth+1,
                                                       DoorOfOppositeRoomPoint.y,
                                                       turningPointZ - 1},
                                                       doorHeight,
                                                       DoorOfAroomPoint.x - DoorOfOppositeRoomPoint.x,
                                                       X_SIDE_WALL,
                                                       CorridorColor,
                                                       &randomPinkWhiteStyle};


            struct Wall AZSIDECorridorWall = {{DoorOfOppositeRoomPoint.x-1,
                                               DoorOfAroomPoint.y,
                                               turningPointZ + doorWidth},
                                               doorHeight,
                                               DoorOfAroomPoint.x - DoorOfOppositeRoomPoint.x,
                                               X_SIDE_WALL,
                                               CorridorColor,
                                               &randomPinkWhiteStyle};

            if (DoorOfAroomPoint.x < DoorOfOppositeRoomPoint.x)
            {
               ALongCorridorWall.StartPoint.x = DoorOfAroomPoint.x - 1;
               AShortOppositeCorridorWall.StartPoint.x = DoorOfOppositeRoomPoint.x- 1;
               ALongOppositeCorridorWall.StartPoint.x = DoorOfOppositeRoomPoint.x + doorWidth; 
               AShortCorridorWall.StartPoint.x = DoorOfAroomPoint.x + doorWidth;
               AnOppositeZSIDECorridorWall.StartPoint.z = turningPointZ  + doorWidth; 
               AnOppositeZSIDECorridorWall.StartPoint.x = DoorOfAroomPoint.x + doorWidth;
               AnOppositeZSIDECorridorWall.width =  1 + DoorOfOppositeRoomPoint.x - DoorOfAroomPoint.x;
               AZSIDECorridorWall.StartPoint.z = turningPointZ -1; 
               AZSIDECorridorWall.StartPoint.x = DoorOfAroomPoint.x - 1;
               AZSIDECorridorWall.width = AnOppositeZSIDECorridorWall.width;
            }

            // Build Roof for Hallway
            if (DoorOfOppositeRoomPoint.x > DoorOfAroomPoint.x)
            {
               AMiddleCorridorRoofEndPoint.x  =  DoorOfOppositeRoomPoint.x +doorWidth;
               AMiddleCorridorRoofStartPoint.x = DoorOfAroomPoint.x -1;
            }

            BuildAWall(&AnOppositeZSIDECorridorWall);
            BuildAWall(&AZSIDECorridorWall);
            BuildAWall(&AShortCorridorWall);
            BuildAWall(&ALongCorridorWall);
            BuildAWall(&AShortOppositeCorridorWall);
            BuildAWall(&ALongOppositeCorridorWall);
            
            // Build Roofs of Corridors
            BuildABox(&AMiddleCorridorRoofStartPoint,&AMiddleCorridorRoofEndPoint,color,&randomPinkWhiteStyle);
            BuildABox(&ACorridorRoofStartPoint,&ACorridorRoofEndPoint,color,&randomPinkWhiteStyle);
            BuildABox(&AnOppositeCorridorRoofStartPoint,&AnOppositeCorridorRoofEndPoint,color,&randomPinkWhiteStyle);
            


      }
   }
}

int normalColorStyle(int originalColor,int i,int j,int z)
{
   return originalColor;
}

int randomPinkWhiteStyle(int originalColor,int i,int j,int z)
{
   return getRandomNumber(5,6);
}
