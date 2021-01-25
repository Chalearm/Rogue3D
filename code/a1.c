
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

extern GLubyte  world[WORLDX][WORLDY][WORLDZ];

   /* mouse function called by GLUT when a button is pressed or released */
void mouse(int, int, int, int);

   /* initialize graphics library */
extern void graphicsInit(int *, char **);

   /* lighting control */
extern void setLightPosition(GLfloat, GLfloat, GLfloat);
extern GLfloat* getLightPosition();

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
extern void  draw2Dline(int, int, int, int, int);
extern void  draw2Dbox(int, int, int, int);
extern void  draw2Dtriangle(int, int, int, int, int, int);
extern void  set2Dcolour(float []);


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


   /*** collisionResponse() ***/
   /* -performs collision detection and response */
   /*  sets new xyz  to position of the viewpoint after collision */
   /* -can also be used to implement gravity by updating y position of vp*/
   /* note that the world coordinates returned from getViewPosition()
      will be the negative value of the array indices */
void collisionResponse() {

   /* your code for collisions goes here */


}


   /******* draw2D() *******/
   /* draws 2D shapes on screen */
   /* use the following functions:        */
   /* draw2Dline(int, int, int, int, int);      */
   /* draw2Dbox(int, int, int, int);         */
   /* draw2Dtriangle(int, int, int, int, int, int);   */
   /* set2Dcolour(float []);           */
   /* colour must be set before other functions are called  */
void draw2D() {

   if (testWorld) {
      /* draw some sample 2d shapes */
      if (displayMap == 1) {
         GLfloat green[] = {0.0, 0.5, 0.0, 0.5};
         set2Dcolour(green);
         draw2Dline(0, 0, 500, 500, 15);
         draw2Dtriangle(0, 0, 200, 200, 0, 200);

         GLfloat black[] = {0.0, 0.0, 0.0, 0.5};
         set2Dcolour(black);
         draw2Dbox(500, 380, 524, 388);
      }
   } else {

   /* your code goes here */

   }

}
void drawWallStacks(int x, int y, int z, int color) {
   int i;
   int ground = 21;
   int wallHeight = ground + y;


   for(i=ground; i<wallHeight; i++)
   {
      world[x][i][z] = color;
   }
}

   /*** update() ***/
   /* background process, it is called when there are no other events */
   /* -used to control animations and perform calculations while the  */
   /*  system is running */
   /* -gravity must also implemented here, duplicate collisionResponse */
void update() {
int i, j, k;
float *la;
float x, y, z;

   /* sample animation for the testworld, don't remove this code */
   /* demo of animating mobs */
   if (testWorld) {

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
      setOldViewPosition(x,y,z);

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
      if (mob0x > 50) increasingmob0 = 0;
      if (mob0x < 30) increasingmob0 = 1;

   /* rotate mob 0 around the y axis */
      mob0ry += 1.0;
      if (mob0ry > 360.0) mob0ry -= 360.0;

   /* move mob 1 and rotate */
      setMobPosition(1, mob1x, mob1y, mob1z, mob1ry);

   /* move mob 1 in the z axis */
   /* when mob is moving away it is visible, when moving back it */
   /* is hidden */
      if (increasingmob1 == 1) {
         mob1z += 0.2;
         showMob(1);
      } else {
         mob1z -= 0.2;
         hideMob(1);
      }
      if (mob1z > 72) increasingmob1 = 0;
      if (mob1z < 52) increasingmob1 = 1;

   /* rotate mob 1 around the y axis */
      mob1ry += 1.0;
      if (mob1ry > 360.0) mob1ry -= 360.0;

   /* change user defined colour over time */
      if (colourCount == 1) offset += 0.05;
      else offset -= 0.01;
      if (offset >= 0.5) colourCount = 0;
      if (offset <= 0.0) colourCount = 1;
      setUserColour(9, 0.7, 0.3 + offset, 0.7, 1.0, 0.3, 0.15 + offset, 0.3, 1.0);

   /* sample tube creation  */
   /* draws a purple tube above the other sample objects */
       createTube(1, 45.0, 30.0, 45.0, 50.0, 30.0, 50.0, 6);

    /* end testworld animation */


   } else {

   /* your code goes here */

   }
}


   /* called by GLUT when a mouse button is pressed or released */
   /* -button indicates which button was pressed or released */
   /* -state indicates a button down or button up event */
   /* -x,y are the screen coordinates when the mouse is pressed or */
   /*  released */ 
void mouse(int button, int state, int x, int y) {

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


void makeBorderRow1(int sidesLength, int endsLength, int xPosition, int zPosition, int color) {
   int i;

   sidesLength = xPosition + sidesLength;
   endsLength = zPosition + endsLength;

   printf("sidesLength = %i\n", sidesLength);

   for(i=xPosition;i<sidesLength;i++)
   {
      //left side
      drawWallStacks(i,3,0, color);
      //right side
      drawWallStacks(i,3,endsLength, color);
   }

   for(i=zPosition;i<endsLength;i++)
    {
      //back side
      drawWallStacks(0,3,i, color);
      //front side
      drawWallStacks(sidesLength-1,3,i, color);
   }
}



void makeWorld() {
   int i, j, k;
for(i=0; i<WORLDX; i++)
         for(j=0; j<WORLDY; j++)
            for(k=0; k<WORLDZ; k++)
               world[i][j][k] = 0;
}
void drawFloor()
{
   int i, j;

   for(i=0; i<WORLDX; i++) {
         for(j=0; j<WORLDZ; j++) {
            world[i][10][j] = 4;
         }
      }
}

void makeBorderRow2(int sidesLength, int endsLength, int xPosition, int zPosition, int color) {
   int i;

   sidesLength = xPosition + sidesLength;
   endsLength = zPosition + endsLength;

   for(i=xPosition;i<sidesLength;i++)
   {
      //left side
      drawWallStacks(i,3,33, color);
      //right side
      drawWallStacks(i,3,66, color);
   }
   for(i=zPosition;i<endsLength;i++)
    {
      //back side
      drawWallStacks(xPosition,3,i, color);
      //front side
      drawWallStacks(sidesLength-1,3,i, color);
   }
}

void drawBorderTest3(int sidesLength, int endsLength, int xPosition, int zPosition, int color) {
   int i;


   for(i=33;i<66;i++)
   {
      //left side
      drawWallStacks(i,3,33, color);
      //right side
      drawWallStacks(i,3,66, color);
   }

   for(i=33;i<66;i++)
    {
      //back side
      drawWallStacks(33,3,i, color);
      //front side
      drawWallStacks(66,3,i, color);
   }
}

void makeBorderRow3(int sidesLength, int endsLength, int xPosition, int zPosition, int color) {
   int i;

   sidesLength = xPosition + sidesLength;
   endsLength = zPosition + endsLength;

   for(i=xPosition;i<sidesLength;i++)
   {
      //left side
      drawWallStacks(i,3,zPosition, color);
      //right side
      drawWallStacks(i,3,endsLength, color);
   }
   for(i=zPosition;i<endsLength;i++)
    {
      //back side
      drawWallStacks(xPosition,3,i, color);
      //front side
      drawWallStacks(sidesLength-1,3,i, color);
   }
}

int getRandomNumber(int minimum, int maximum) {

   return minimum + rand() % (maximum+1 - minimum);
}
int main(int argc, char** argv)
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
   if (testWorld == 1) {
   /* initialize world to empty */
      for(i=0; i<WORLDX; i++)
         for(j=0; j<WORLDY; j++)
            for(k=0; k<WORLDZ; k++)
               world[i][j][k] = 0;

   /* some sample objects */
   /* build a red platform */
      for(i=0; i<WORLDX; i++) {
         for(j=0; j<WORLDZ; j++) {
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
      for(i=0; i<WORLDX-1; i++) {
         world[i][25][0] = 2;
         world[i][25][WORLDZ-1] = 2;
      }
      for(i=0; i<WORLDZ-1; i++) {
         world[0][25][i] = 2;
         world[WORLDX-1][25][i] = 2;
      }

   /* create two sample mobs */
   /* these are animated in the update() function */
      createMob(0, 50.0, 25.0, 52.0, 0.0);
      createMob(1, 50.0, 25.0, 52.0, 0.0);

   /* create sample player */
      createPlayer(0, 52.0, 27.0, 52.0, 0.0);
   } else {

   /* your code to build the world goes here */
      makeWorld();
      drawFloor();
      // drawBorder(0, 33);
      // drawBorder(33, 66);
// drawBorder(66, 100);
      
//    for(i=0; i<50; i++)
//          for(j=0; j<33; j++)
//          world[i][25][j] =2;   


//       for(i=0; i<50; i++)
//          for(j=33; j<66; j++)
//          world[i][25][j]= 3;  


//       for(i=0; i<50; i++)
//          for(j=66; j<100; j++)
//          world[i][25][j]= 5; 
//  ROOM attributes
#define ROOM_XP 0
#define ROOM_ZP 1
#define ROOM_X_LENGTH 2
#define ROOM_Z_LENGTH 3
#define DOOR_WEST_XP 4
#define DOOR_WEST_ZP 5
#define DOOR_EAST_XP 6
#define DOOR_EAST_ZP 7
#define DOOR_SOUTH_XP 8
#define DOOR_SOUTH_ZP 9
#define DOOR_NORTH_XP 10
#define DOOR_NORTH_ZP 11
#define WEST_ID 12
#define EAST_ID 13
#define SOUTH_ID 14
#define NORTH_ID 15

      int RMAT[9][16]; // RooM ATtribute

// 3x3 areas and Door position directions
      //{x,z,xLenght,zLenght,CanBuildDoorW,CanBuildDoorE,CanBuildDoorS,CanBuildDoorN}
      // CanBuildDoor status = -1 , Corridor cannot be built
      //                       0 - 8 room number which Corridor can be built
      //                       m = 10 - 18 the door of room number m-10 is   built
      //                       m = 20 - 28 the hallway and corridors are built
#define AREA_XP 0
#define AREA_ZP 1
#define AREA_X_LENGHT 2
#define AREA_Z_LENGHT 3
#define WEST_DEFINE_ID 4
#define EAST_DEFINE_ID 5
#define SOUTH_DEFINE_ID 6
#define NORTH_DEFINE_ID 7

      int areaAndDoorPosition[9][8]= {{0,0,33,33,-1,1,-1,3},
                        {33,0,34,33,0,2,-1,4},
                        {67,0,33,33,1,-1,-1,5},
                        {0,33,33,34,-1,4,0,6},
                        {33,33,34,34,3,5,1,7},
                        {67,33,33,34,4,-1,2,8},
                        {0,67,33,33,-1,7,3,-1},
                        {33,67,34,33,6,8,4,-1},
                        {67,67,33,33,7,-1,5,-1}};

//       for(i=50; i<100; i++)
//          for(j=66; j<100; j++)
//          world[i][25][j]= 3; 
      int l,m,n,o,p;
      int xLenght = 0;
      int zLenght = 0;
      int wallHeight = 3;
      int xViewP = 0;
      int zViewP = 0;
      int yStartP = 11;
      int colorID1 = 3;
      const int numRoom = 9;
      const int doorWidth = 2;
      const int wallColor = 1; // Green
      int ViewPointID = getRandomNumber(0,8); //which room the view point will be
      int doorHeight = 2;
      int oppositeRoomID = -1;
      int sparForCorridorsX = 4;
      int sparForCorridorsZ = 4;
      int sparForRoomSizeMax = 24;  // sparForRoomSize >= 3+doorWidth
      int sparForRoomSizeMin = 6;
      for(k = 0;k<numRoom;k++)
      {
         sparForCorridorsX = (areaAndDoorPosition[k][AREA_X_LENGHT] - sparForRoomSizeMax)/2;
         sparForCorridorsZ = (areaAndDoorPosition[k][AREA_Z_LENGHT] - sparForRoomSizeMax)/2;
         //initial DOOR points
         RMAT[k][DOOR_WEST_XP] = -1;
         RMAT[k][DOOR_WEST_ZP] = -1;
         RMAT[k][DOOR_EAST_XP] = -1;
         RMAT[k][DOOR_EAST_ZP] = -1;
         RMAT[k][DOOR_SOUTH_XP] = -1;
         RMAT[k][DOOR_SOUTH_ZP] = -1;
         RMAT[k][DOOR_NORTH_XP] = -1;
         RMAT[k][DOOR_NORTH_ZP] = -1;
         // define door direction to Room attribute
         RMAT[k][WEST_ID] = areaAndDoorPosition[k][WEST_DEFINE_ID];
         RMAT[k][EAST_ID] = areaAndDoorPosition[k][EAST_DEFINE_ID];
         RMAT[k][SOUTH_ID] = areaAndDoorPosition[k][SOUTH_DEFINE_ID];
         RMAT[k][NORTH_ID] = areaAndDoorPosition[k][NORTH_DEFINE_ID];

                  // random size of a room
         RMAT[k][ROOM_X_LENGTH] = getRandomNumber(sparForRoomSizeMin,sparForRoomSizeMax);
         RMAT[k][ROOM_Z_LENGTH] = getRandomNumber(sparForRoomSizeMin,sparForRoomSizeMax);
//printf("k:%d, max X:%d, Z:%d, CX:%d, CZ:%d\n",k,areaAndDoorPosition[k][AREA_X_LENGHT]-RMAT[k][ROOM_X_LENGTH]-sparForCorridorsX-1,areaAndDoorPosition[k][AREA_Z_LENGHT]-RMAT[k][ROOM_Z_LENGTH]-sparForCorridorsZ-1,sparForCorridorsX,sparForCorridorsZ);
         // random location of a room
         //printf("OK[%d] :%d, sparForCorridorsX:%d, RMAT[k][ROOM_X_LENGTH]:%d\n",k,areaAndDoorPosition[k][AREA_X_LENGHT]-RMAT[k][ROOM_X_LENGTH]-sparForCorridorsX-sparForCorridorsX,sparForCorridorsX,RMAT[k][ROOM_X_LENGTH]);
         RMAT[k][ROOM_XP] = sparForCorridorsX + areaAndDoorPosition[k][AREA_XP] + getRandomNumber(0,areaAndDoorPosition[k][AREA_X_LENGHT]-RMAT[k][ROOM_X_LENGTH]-sparForCorridorsX-sparForCorridorsX);
         RMAT[k][ROOM_ZP] = sparForCorridorsZ + areaAndDoorPosition[k][AREA_ZP] + getRandomNumber(0,areaAndDoorPosition[k][AREA_Z_LENGHT]-RMAT[k][ROOM_Z_LENGTH]-sparForCorridorsZ-sparForCorridorsZ);

//printf("k:%d, Grid3x3:(%d,%d)(X,Z) : (%d,%d), LX:%d, LZ:%d\n",k,areaAndDoorPosition[k][AREA_XP],areaAndDoorPosition[k][AREA_ZP],RMAT[k][ROOM_XP],RMAT[k][ROOM_ZP],RMAT[k][ROOM_X_LENGTH],RMAT[k][ROOM_Z_LENGTH]);

         // find view point
         if (k == ViewPointID)
         {
            xViewP = 2 + RMAT[k][ROOM_XP] + getRandomNumber(0,RMAT[k][ROOM_X_LENGTH]-4);
            zViewP = 2 + RMAT[k][ROOM_ZP] + getRandomNumber(0,RMAT[k][ROOM_Z_LENGTH]-4);
         }
         world[2 + RMAT[k][ROOM_XP] + getRandomNumber(0,RMAT[k][ROOM_X_LENGTH] -4)][yStartP][2 + RMAT[k][ROOM_ZP]  + getRandomNumber(0,RMAT[k][ROOM_Z_LENGTH] -4)]= 8;

         
         // build walls of room
         //wallHeight = getRandomNumber(7,13);
         for(j = 0;j<wallHeight;j++)
         {
            for(i = 0;i<RMAT[k][ROOM_X_LENGTH];i++)
            {
               world[RMAT[k][ROOM_XP]+i][yStartP+j][RMAT[k][ROOM_ZP]]= wallColor;
               world[RMAT[k][ROOM_XP]+i][yStartP+j][RMAT[k][ROOM_ZP]+RMAT[k][ROOM_Z_LENGTH]-1]=wallColor; 
            }
            // -2 and move offset +1 at Z-axis cuz 2 cells of X-wall sides have already built 
            for(i = 0;i<RMAT[k][ROOM_Z_LENGTH]-2;i++)
            {
               world[RMAT[k][ROOM_XP]][yStartP+j][RMAT[k][ROOM_ZP]+i+1]= wallColor; 
               world[RMAT[k][ROOM_XP]+RMAT[k][ROOM_X_LENGTH]-1][yStartP+j][RMAT[k][ROOM_ZP]+i+1]= wallColor; 
            }
         }

         // build door and corridor for 4 direction
         // east
         if(RMAT[k][EAST_ID] > -1) 
         {
            RMAT[k][DOOR_EAST_XP] = RMAT[k][ROOM_XP] + RMAT[k][ROOM_X_LENGTH] -1;
            do {RMAT[k][DOOR_EAST_ZP] = RMAT[k][ROOM_ZP] + getRandomNumber(1,RMAT[k][ROOM_Z_LENGTH]-doorWidth-1);}
            while(RMAT[k][DOOR_EAST_ZP] == RMAT[RMAT[k][EAST_ID]][DOOR_WEST_ZP]);
            for(j = 0; j< doorHeight;j++)
             for (i = 0; i < doorWidth; i++)
               world[RMAT[k][DOOR_EAST_XP]][yStartP+j][RMAT[k][DOOR_EAST_ZP]+i] = 0;

         }

         // west
         if(RMAT[k][WEST_ID] > -1) 
         {
            RMAT[k][DOOR_WEST_XP] = RMAT[k][ROOM_XP];
            do {RMAT[k][DOOR_WEST_ZP] = RMAT[k][ROOM_ZP] + getRandomNumber(1,RMAT[k][ROOM_Z_LENGTH]-doorWidth-1);}
            while(RMAT[k][DOOR_WEST_ZP] == RMAT[RMAT[k][WEST_ID]][DOOR_EAST_ZP]);
            for(j = 0; j< doorHeight;j++)
               for (i = 0; i < doorWidth; i++)
                  world[RMAT[k][DOOR_WEST_XP]][yStartP+j][RMAT[k][DOOR_WEST_ZP]+i] = 0;
         }
         // south
         if(RMAT[k][SOUTH_ID] > -1) 
         {
            RMAT[k][DOOR_SOUTH_ZP] = RMAT[k][ROOM_ZP];
            do {RMAT[k][DOOR_SOUTH_XP] = RMAT[k][ROOM_XP] + getRandomNumber(1,RMAT[k][ROOM_X_LENGTH]-doorWidth-1);}
            while(RMAT[k][DOOR_SOUTH_XP] == RMAT[RMAT[k][SOUTH_ID]][DOOR_NORTH_XP]);


               for(j = 0; j< doorHeight;j++)
                for (i = 0; i < doorWidth; i++)
                  world[RMAT[k][DOOR_SOUTH_XP] +i][yStartP+j][RMAT[k][DOOR_SOUTH_ZP]] = 0;
         }
         // north
         if(RMAT[k][NORTH_ID] > -1) 
         {
            RMAT[k][DOOR_NORTH_ZP] = RMAT[k][ROOM_ZP] + RMAT[k][ROOM_Z_LENGTH] -1;
            do {RMAT[k][DOOR_NORTH_XP] = RMAT[k][ROOM_XP] + getRandomNumber(1,RMAT[k][ROOM_X_LENGTH]-doorWidth-1);}
            while(RMAT[k][DOOR_NORTH_XP] == RMAT[RMAT[k][NORTH_ID]][DOOR_SOUTH_XP]);
               for(j = 0; j< doorHeight;j++)
                for (i = 0; i < doorWidth; i++)
                  world[RMAT[k][DOOR_NORTH_XP]+i][yStartP+j][RMAT[k][DOOR_NORTH_ZP]] = 0;
         }
         // build  hallway and corridor
         // west and east side
         //RMAT[k][EAST_ID]
         if((RMAT[k][DOOR_WEST_XP] > -1)&&(RMAT[RMAT[k][WEST_ID]][DOOR_EAST_XP] > -1))
         {//AREA_X_LENGHT

            oppositeRoomID = RMAT[k][WEST_ID];
            int LL = 0;
            int dXP = (areaAndDoorPosition[oppositeRoomID][AREA_XP] + areaAndDoorPosition[oppositeRoomID][AREA_X_LENGHT]) - (doorWidth/2) -1;
            int dXL = dXP - RMAT[oppositeRoomID][DOOR_EAST_XP];
            int aXP = dXP + doorWidth;
            int aXL = dXL + doorWidth;
            int aZP = RMAT[oppositeRoomID][DOOR_EAST_ZP]-1;
            int dZP = RMAT[oppositeRoomID][DOOR_EAST_ZP]+doorWidth;
            int bXP = aXP+1;
            int bXL = RMAT[k][DOOR_WEST_XP] - bXP;
            int bZP = RMAT[k][DOOR_WEST_ZP]-1;
            int fXP = dXP+1;
            int fXL = RMAT[k][DOOR_WEST_XP] - fXP;
            int fZP = RMAT[k][DOOR_WEST_ZP]+ doorWidth;
            if (RMAT[k][DOOR_WEST_ZP] < RMAT[oppositeRoomID][DOOR_EAST_ZP])
            {
               dZP = RMAT[oppositeRoomID][DOOR_EAST_ZP]-1; 
               aZP = RMAT[oppositeRoomID][DOOR_EAST_ZP]+doorWidth;
               bZP = RMAT[k][DOOR_WEST_ZP]+ doorWidth;
               fZP = RMAT[k][DOOR_WEST_ZP]-1;


            }

            for(j = 0;j <doorHeight;j++)
            {
               LL = bXL;
               if (fXL > bXL ) LL = fXL;
               for (i = 0; i < LL;i++)
               {
                  if(i <bXL)world[bXP+i][yStartP+j][bZP] = 3;
                  if(i <fXL)world[fXP+i][yStartP+j][fZP] = 3;
               }

               for (i = 0; i < (aXL-1);i++)
               {
                   world[RMAT[oppositeRoomID][DOOR_EAST_XP]+i+1][yStartP+j][aZP] = 5;
                   if(i < (dXL-1))world[RMAT[oppositeRoomID][DOOR_EAST_XP]+i+1][yStartP+j][dZP] = 2;
               }

            }

//printf("k:%d, BX,BZ:%d,%d, BXL:%d - FX,FZ:%d, %d, FXL:%d\n",k,bXP,bZP,bXL,fXL,fZP,fXL);

         }
         // south and north side 
         if((RMAT[k][DOOR_SOUTH_ZP] > -1)&&(RMAT[RMAT[k][SOUTH_ID]][DOOR_NORTH_ZP] > -1))
         {

            oppositeRoomID = RMAT[k][SOUTH_ID];
            int LL = 0;
            int dZP = (areaAndDoorPosition[oppositeRoomID][AREA_ZP] + areaAndDoorPosition[oppositeRoomID][AREA_Z_LENGHT]) - (doorWidth/2)-1;
            int dZL = dZP - RMAT[oppositeRoomID][DOOR_NORTH_ZP];
            int aZP = dZP + doorWidth;
            int aZL = dZL + doorWidth;
            int aXP = RMAT[oppositeRoomID][DOOR_NORTH_XP]-1;
            int dXP = RMAT[oppositeRoomID][DOOR_NORTH_XP]+doorWidth;
            int bZP = aZP+1;
            int bZL = RMAT[k][DOOR_SOUTH_ZP] - bZP;
            int bXP = RMAT[k][DOOR_SOUTH_XP]-1;

            int fZP = dZP+1;
            int fZL = RMAT[k][DOOR_SOUTH_ZP] - fZP;
            int fXP = RMAT[k][DOOR_SOUTH_XP]+ doorWidth;
            if (RMAT[k][DOOR_SOUTH_XP] < RMAT[oppositeRoomID][DOOR_NORTH_XP])
            {
               dXP = RMAT[oppositeRoomID][DOOR_NORTH_XP]-1; 
               aXP = RMAT[oppositeRoomID][DOOR_NORTH_XP]+doorWidth;
               bXP = RMAT[k][DOOR_SOUTH_XP]+ doorWidth;
               fXP = RMAT[k][DOOR_SOUTH_XP]-1;


            }
            for(j = 0;j <doorHeight;j++)
            {
               LL = bZL;
               if (fZL > bZL ) LL = fZL;
               for (i = 0; i < LL;i++)
               {
                  if(i <bZL)world[bXP][yStartP+j][bZP+i] = 3;
                  if(i <fZL)world[fXP][yStartP+j][fZP+i] = 3;
               }

               for (i = 0; i < (aZL-1);i++)
               {
                   world[aXP][yStartP+j][RMAT[oppositeRoomID][DOOR_NORTH_ZP]+i+1] = 6;
                   if(i < (dZL-1))world[dXP][yStartP+j][RMAT[oppositeRoomID][DOOR_NORTH_ZP]+i+1] = 5;
               }

            }
         }

      }

      //drawBorderTest3(33, 33, 33, 0, 6);

     // drawBorderTest4(33, 33, 33, 0, 7);

     // int randomNum = getRandomNumber(50, 100);
    //  printf("Random Number = %i\n", randomNum);

    //drawFloor();
      //drawBorder();
     // drawPillars();
      // drawTestBlocks();

   setViewPosition(-1*xViewP, -48, -1*zViewP);
   }



   /* starts the graphics processing loop */
   /* code after this will not run until the program exits */
   glutMainLoop();
   return 0; 
}



void makeWall(int numOfBlocks, int color) {
   int i = 0;
   for (i = 0; i<numOfBlocks; i++) {

   }
}



void drawBorder(int borderMinRange, int borderMaxRange)
{
   int i;

   for(i=borderMinRange; i<borderMaxRange; i++) {
      drawWallStacks(i, 3, borderMinRange,6);
      drawWallStacks(i, 3, borderMaxRange,6);    
   }
   // for(i=borderMinRange; i<borderMaxRange; i++) {
   //    drawWallStacks(borderMinRange, 3, i,6);
   //    drawWallStacks(borderMaxRange, 3, i,6);
   // }
}





void drawBorderTest4(int sidesLength, int endsLength, int xPosition, int zPosition, int color) {
   int i;


   for(i=66;i<99;i++)
   {
      //left side
      drawWallStacks(i,3,33, color);
      //right side
      drawWallStacks(i,3,66, color);
   }

   for(i=33;i<66;i++)
    {
      //back side
      drawWallStacks(66,3,i, color);
      //front side
      drawWallStacks(99,3,i, color);
   }
}






void draw_wall_3high(int x, int z, int cubeStyle)
{
   world[x][21][z] = cubeStyle;
   world[x][22][z] = cubeStyle;
   world[x][23][z] = cubeStyle;
}

void drawPillars()
{
   int i, j;

   for(i=0; i<28; i=i+4) {  
      for(j=0; j<28; j=j+4) {
         draw_wall_3high(i,j,2);
         draw_wall_3high(j,i,2);
      }
   }
}




void drawTestBlocks()
{
      /* 1 high */
      world[1][25][1] = 1;

      /* 2 high */
      world[1][25][2] = 8;
      world[1][26][2] = 8;

      /* 3 high */
      draw_wall_3high(1,3,3);
}




