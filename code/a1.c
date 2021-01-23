
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

   float vxp = 0;
   float vyp = 0;
   float vzp = 0;
   int worldValue = 0;
   int worldValue2 = 0;
   int outOfSpace = 0;
   //jui
   getViewPosition(&vxp,&vyp,&vzp);
   worldValue = world[-1*(int)vxp][-1*(int)vyp][-1*(int)vzp];
   worldValue2= world[-1*(int)vxp][1+(-1)*(int)vyp][-1*(int)vzp];
   outOfSpace = (((-1*(int)vxp)> 99) || ((-1*(int)vyp)> 49) || ((-1*(int)vzp)> 99));
   outOfSpace = outOfSpace || (((-1*(int)vxp) < 0) || ((-1*(int)vyp) < 21) || ((-1*(int)vzp) <0));
   if ((outOfSpace == 1)||((worldValue > 0) && (worldValue2 > 0)))
   {
      
      getOldViewPosition(&vxp,&vyp,&vzp);
      setViewPosition(vxp,vyp,vzp); 
   }
   else if ((worldValue > 0) && (worldValue2 == 0))
   {
      getOldViewPosition(&vxp,&vyp,&vzp);
      setViewPosition(vxp,vyp-1,vzp); 
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
      int valOfWorldAtBelowVP = 0;
      static float vpx = 0;
      static float vpy = 0;
      static float vpz = 0;
      float newY = 0;
      float floorLv = 20;
      static int fallState = 0;
      // yn = y(n-1) - 0.5*g*(tn^2-t(n-1)^2)
      static clock_t timeCount,clkRef;
      static float timePast,timeCurrent;
      getViewPosition(&vpx,&vpy,&vpz);
      valOfWorldAtBelowVP = world[-1*(int)vpx][(-1*(int)vpy)-1][-1*(int)vpz];
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
         newY = (-1.0)*vpy - 4.0*(timeCurrent*timeCurrent-timePast*timePast);
         
         if (valOfWorldAtBelowVP != 0) 
         {
            floorLv = (-1*(int)vpy)-1;
         }

         if((valOfWorldAtBelowVP != 0) && ((((float)((int)vpy)) - vpy) <= 0.0))
         {
            fallState = 0;
            newY =floorLv+1.0;
         }
         if ((newY < (floorLv+1))||(newY > 49.0)) newY =floorLv+1.0;
         setViewPosition(vpx,newY*(-1.0),vpz);
         //printf("newY:%f , dffY:%f time diff :%f, tC:%f, tP:%f, floorLv:%f\n",newY,(((float)((int)vpy)) - vpy) ,(timeCurrent*timeCurrent-timePast*timePast),timeCurrent,timePast,floorLv);
         }

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
            world[i][20][j] = 2;
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

// // Second row

//       for(i=50; i<100; i++)
//          for(j=0; j<33; j++)
//          world[i][25][j] =5;   


//       for(i=50; i<100; i++)
//          for(j=33; j<66; j++)
//          world[i][25][j]= 2;  


//       for(i=50; i<100; i++)
//          for(j=66; j<100; j++)
//          world[i][25][j]= 3; 
      int l,m,n,o,p;
      int xLenght = 0;
      int zLenght = 0;
      int yLenght = 12;
      int xStartP = 0;
      int zStartP = 0;
      int xViewP = 0;
      int zViewP = 0;
      int yStartP = 21;
      int colorID1 = 3;
      const int numRoom = 9;
      const int doorWidth = 2;
      int ViewPointID = getRandomNumber(0,8); //which the view point will be
      int doorHeight = 3;
      int oppositeRoomID = -1;
      int sparForCorridors = 2;
      int sparForRoomSize = 5;  // sparForRoomSize >= 3+doorWidth
      //{x,z,xLenght,zLenght,CanBuildDoorW,CanBuildDoorE,CanBuildDoorS,CanBuildDoorN}
      // CanBuildDoor status = -1 , Corridor cannot be built
      //                       0 - 8 room number which Corridor can be built
      //                       m = 10 - 18 the corridor of room number m-10 is   built
      int grid3x3[9][8]= {{0,0,33,33,-1,1,-1,3},
                        {33,0,34,33,0,2,-1,4},
                        {67,0,33,33,1,-1,-1,5},
                        {0,33,33,34,-1,4,0,6},
                        {33,33,34,34,3,5,1,7},
                        {67,33,33,34,4,-1,2,8},
                        {0,67,33,33,-1,7,3,-1},
                        {33,67,34,33,6,8,4,-1},
                        {67,67,33,33,7,-1,5,-1}};
      int roomProperty[9][8];
      for(k = 0;k<numRoom;k++)
      {

         // random location of a room
         xStartP = grid3x3[k][0] + getRandomNumber(0,grid3x3[k][2]-sparForCorridors-sparForRoomSize-1);
         zStartP = grid3x3[k][1] + getRandomNumber(0,grid3x3[k][3]-sparForCorridors-sparForRoomSize-1);

         // random size of a room
         xLenght = sparForRoomSize + getRandomNumber(0,grid3x3[k][2]+grid3x3[k][0]-1-sparForCorridors-sparForRoomSize - xStartP);
         zLenght = sparForRoomSize + getRandomNumber(0,grid3x3[k][3]+grid3x3[k][1]-1-sparForCorridors-sparForRoomSize - zStartP);

         // find view point
         printf("k: %d, xStart : %d, zStartP:%d, XL:%d, ZL:%d\n",k,xStartP,zStartP,xLenght,zLenght);
         if (k == ViewPointID)
         {
            xViewP = 2 + xStartP + getRandomNumber(0,xLenght-4);
            zViewP = 2 + zStartP + getRandomNumber(0,zLenght-4);
            world[xViewP+1][yStartP][zViewP] =7;
            world[xViewP+2][yStartP+1][zViewP] =7;
            world[xViewP+3][yStartP+2][zViewP] =7;
            world[xViewP+4][yStartP+3][zViewP] =4;
            world[xViewP+5][yStartP+4][zViewP] =1;
            printf("ViewPId:%d, xV:%d, zV:%d\n",ViewPointID,xViewP,zViewP);
         }

         roomProperty[k][0] = xStartP;
         roomProperty[k][1] = zStartP;
         roomProperty[k][2] = xLenght;
         roomProperty[k][3] = zLenght;
         roomProperty[k][4] = grid3x3[k][4];
         roomProperty[k][5] = grid3x3[k][5];
         roomProperty[k][6] = grid3x3[k][6];
         roomProperty[k][7] = grid3x3[k][7];
         
         // build walls of room
         yLenght = getRandomNumber(7,13);
         for(j = 0;j<yLenght;j++)
         {
            for(i = 0;i<xLenght;i++)
            {
               world[xStartP+i][yStartP+j][zStartP]= getRandomNumber(5,6); 
               world[xStartP+i][yStartP+j][zStartP+zLenght-1]=getRandomNumber(5,6); 
            }
            // -2 and move offset +1 at Z-axis cuz 2 cells of X-wall sides have already built 
            for(i = 0;i<zLenght-2;i++)
            {
               world[xStartP][yStartP+j][zStartP+i+1]= getRandomNumber(5,6); 
               world[xStartP+xLenght-1][yStartP+j][zStartP+i+1]= getRandomNumber(5,6); 
            }
         }

         // build door and corridor for 4 direction
         // west
         if((roomProperty[k][5] > -1) && (roomProperty[k][5] < 9)) 
         {
            xStartP = roomProperty[k][0] + roomProperty[k][2] -1;
            //find door z point of room k th
            zStartP = roomProperty[k][1] + getRandomNumber(1,roomProperty[k][3]-doorWidth-1);
            roomProperty[k][5] = roomProperty[k][5]*10;

            for(j = 0; j< doorHeight;j++)
             for (i = 0; i < doorWidth; i++)
               world[xStartP][yStartP+j][zStartP+i] = 0;

         }

         // east
         if((roomProperty[k][4] > -1) && (roomProperty[k][4] < 9)) 
         {
            xStartP = roomProperty[k][0];
            zStartP = roomProperty[k][1] + getRandomNumber(1,roomProperty[k][3]-doorWidth-1);
            roomProperty[k][4] = roomProperty[k][4]*10;

            for(j = 0; j< doorHeight;j++)
               for (i = 0; i < doorWidth; i++)
                  world[xStartP][yStartP+j][zStartP+i] = 0;
         }
         // south
         if((roomProperty[k][6] > -1) && (roomProperty[k][6] < 9)) 
         {

               zStartP = roomProperty[k][1];
               //find door z point of room k th
               xStartP = roomProperty[k][0] + getRandomNumber(1,roomProperty[k][2]-doorWidth-1);


               roomProperty[k][6] = roomProperty[k][6]*10;

               for(j = 0; j< doorHeight;j++)
                for (i = 0; i < doorWidth; i++)
                  world[xStartP+i][yStartP+j][zStartP] = 0;
         }
         // north
         if((roomProperty[k][7] > -1) && (roomProperty[k][7] < 9)) 
         {

               zStartP = roomProperty[k][1] + roomProperty[k][3] -1;
               xStartP = roomProperty[k][0] + getRandomNumber(1,roomProperty[k][2]-doorWidth-1);

               roomProperty[k][7] = roomProperty[k][7]*10;

               for(j = 0; j< doorHeight;j++)
                for (i = 0; i < doorWidth; i++)
                  world[xStartP+i][yStartP+j][zStartP] = 0;
         }
         // build  hallway and corridor
         // w to e
         if((roomProperty[k][4] > 8) && (roomProperty[k][5] > 8)) 
         {

         }
         // n to s
         if((roomProperty[k][6] > 8) && (roomProperty[k][7] > 8))
         {

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

   setViewPosition(-1*xViewP, -1*yStartP, -1*zViewP);
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




