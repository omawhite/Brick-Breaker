/* main.c ---
 *
 * Filename: main.c
 * Description:
 * Author:
 * Maintainer:
 * Created: Thu Jan 10 11:23:43 2013
 * Last-Updated:
 *           By:
 *     Update #: 0
 * Keywords:
 * Compatibility:
 *
 */

/* Commentary:
 *
 *
 *
 */

/* Change log:
 *
 *
 */
/* Code: */

#include <stm32f30x.h>  // Pull in include files for F30x standard drivers
#include <f3d_led.h>     // Pull in include file for the local drivers
#include <f3d_uart.h>
#include <f3d_user_btn.h>
#include <f3d_gyro.h>
#include <f3d_lcd_sd.h>
#include <f3d_i2c.h>
#include <f3d_accel.h>
#include <f3d_timer2.h>
#include <f3d_mag.h>
#include <f3d_nunchuk.h>
#include <f3d_rtc.h>
#include <f3d_systick.h>
#include <f3d_dac.h>
#include <ff.h>
#include <diskio.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <queue.h>
#include <rect.h>
#include "bmp.h"
#include "brick.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////
//structs for handeling bmp files
struct bmpfile_magic magic;
struct bmpfile_header header;
BITMAPINFOHEADER info;
struct bmppixel pix[128];
///////////////////////////////////////////////////////////////////////////////////////////////////
#define TIMER 20000
#define AUDIOBUFSIZE 128
#define PLAYER_MOVE_INCREMENT 3
#define PADDLE_HEIGHT 3
#define PADDLE_THICKNESS 35
#define BALL_DIM 4
#define BRICK_THICKNESS 9
#define BRICK_HEIGHT 5
#define EVENT_LOOP_TIME 20
#define MAX_X 128
#define MAX_Y 160
//////////////////////////////////////////////////////////////////////////////////////////////////////
extern uint8_t Audiobuf[AUDIOBUFSIZE];
extern int audioplayerHalf;
extern int audioplayerWhole;

FATFS Fatfs;		/* File system object */
FIL fid;		/* File object */
BYTE Buff[512];		/* File read buffer */
int ret;

void play(void); /*function that play the wav file */
void drawpicture(void); /*function that draws john cena */

struct ckhd {
  uint32_t ckID;
  uint32_t cksize;
};

struct fmtck {
  uint16_t wFormatTag;
  uint16_t nChannels;
  uint32_t nSamplesPerSec;
  uint32_t nAvgBytesPerSec;
  uint16_t nBlockAlign;
  uint16_t wBitsPerSample;
};

void readckhd(FIL *fid, struct ckhd *hd, uint32_t ckID) {
  f_read(fid, hd, sizeof(struct ckhd), &ret);
  if (ret != sizeof(struct ckhd))
    exit(-1);
  if (ckID && (ckID != hd->ckID))
    exit(-1);
}

void die (FRESULT rc) {
  printf("Failed with rc=%u.\n", rc);
  while (1);
}

//call out rectangle objects
rect_t player;
rect_t ball;

//create structure to hold our layers and bricks info
struct layer brickLayers[3];

//variables to hold our files names
char audiofileName[30];
char bmpfileName[30];

//variable for reading nunchuk input
nunchuk_t n;

//for main menu function
int step = 0;
int stepMod;
//game score and lives
int score;
int lives;
char scoreString[30];
char levelString[30];
char livesString[30];

//game level
int level;
int developerMode;

//keeps track of bricks destroyed
int bricksDestroyed;

int main(void) {

  setvbuf(stdin, NULL, _IONBF, 0);
  setvbuf(stdout, NULL, _IONBF, 0);
  setvbuf(stderr, NULL, _IONBF, 0);

  f3d_i2c1_init();
  delay(10);
  f3d_nunchuk_init();
  delay(10);
  f3d_rtc_init();
  delay(10);

  f3d_uart_init();
  f3d_timer2_init();
  f3d_dac_init();
  f3d_delay_init();
  f3d_led_init();
  f3d_lcd_init();
  f3d_systick_init();
  f3d_user_btn_init();

  printf("initalizations complete, loading main menu\n");


  //call up the main menu
  mainMenu();
  //all the rest of the game stuff is handled by other function
}

//draws a single brick
drawBrick(int lx, int ly, uint16_t color){
  f3d_lcd_drawRectangle(color,lx,ly,lx + (BRICK_THICKNESS -1),ly + (BRICK_HEIGHT -1));
}

//erases the specified brick
eraseBrick(int lx, int ly){
  f3d_lcd_drawRectangle(BLACK,lx,ly,lx + (BRICK_THICKNESS -1),ly + (BRICK_HEIGHT -1));
}

// draw bricks at begining of game////////////////////////////////////////////////////////////////////
createbricks(int currentlevel){
  int i;
  int j; 
  int current_x = 1; 
  int layer_y = 1;

  for(i=0; i<3; i++){ //3 rows
    //update layer structure
    brickLayers[i].pos_y = layer_y;
    for(j=0; j<10; j++){ //10 bricks per row
      //update brick structure
      brickLayers[i].elements[j].pos_x = current_x;
      brickLayers[i].elements[j].life = currentlevel;
      if(j % 2 == 0){
	brickLayers[i].elements[j].color = BLUE;
      }else{
	brickLayers[i].elements[j].color = YELLOW;
      }
      //draw brick at coordinate
      drawBrick(current_x, layer_y, brickLayers[i].elements[j].color);
      //set offset for next coordinate
      //this number will likely Change depends on how wide the bricks end up being
      current_x += (BRICK_THICKNESS +  4);
    }
    //set lx back to original value for next row
    current_x = 1;
    //set the offset for ly, this number will likely change depending on how tall the bricks are
    layer_y += (BRICK_HEIGHT * 2);
  }
}

//displays a main menu///////////////////////////////////////////////////////////////////////////////
mainMenu(){
  //use this once the picture is the backgroud
  //  f3d_lcd_fillScreen(WHITE);
  f3d_lcd_fillScreen(WHITE);

  printf("screen cleared\n");

  //set file name for menu screen
  strcpy(bmpfileName, "menu.bmp");
  drawpicture();

  delay(1000);

  //set file name for audio
  strcpy(audiofileName, "radar.wav");
  play();


  //draw image
  while(1){
    //get nunchuck input
    f3d_nunchuk_read(&n);

    //Joystick controls
    //down
    if(n.jy > 150){
      step--;
    }
    //up
    if(n.jy < 15){
      step++;
    }

    stepMod = abs(step)%3;

    if(user_btn_read() == 1){
      developerMenu();
    }

    //select different menu options
    switch(stepMod) {
    case 0:
      f3d_lcd_drawString(20,10,"Breakout", WHITE,BLACK);
      f3d_lcd_drawString(20,30,"Play",WHITE,RED);
      f3d_lcd_drawString(20,40,"Instructions",WHITE,BLACK);
      f3d_lcd_drawString(20,50,"Exit",WHITE,BLACK);
      if(n.c){
        initgameSetup();
      }
      break;
    case 1:
      f3d_lcd_drawString(20,10,"Breakout", WHITE,BLACK);
      f3d_lcd_drawString(20,30,"Play",WHITE,BLACK);
      f3d_lcd_drawString(20,40,"Instructions",WHITE,RED);
      f3d_lcd_drawString(20,50,"Exit",WHITE,BLACK);
      if(n.c){
        //show the instructions
        f3d_lcd_fillScreen(BLACK);
	int i = 1;
	while(i == 1){
	  //print the instructions
	  
	  f3d_lcd_drawString(5,10, "move with  nunchuck",WHITE,BLACK);
	  f3d_lcd_drawString(5,40, "breaking bricks gets you points",WHITE,BLACK);
	  f3d_lcd_drawString(5,80, "if you lose the ball you will lose a life",WHITE,BLACK);
	  f3d_lcd_drawString(5,110, "press z to go back", WHITE, BLACK);
	  
	  f3d_nunchuk_read(&n);
	  if(n.z){
	    i = 0;
	  }
	}
	//set file name for menu screen
	strcpy(bmpfileName, "menu.bmp");
	drawpicture();
      }
      break;
    case 2:
      f3d_lcd_drawString(20,10,"Breakout", WHITE,BLACK);
      f3d_lcd_drawString(20,30,"Play",WHITE,BLACK);
      f3d_lcd_drawString(20,40,"Instructions",WHITE,BLACK);
      f3d_lcd_drawString(20,50,"Exit",WHITE,RED);
      if(n.c){
        quitGame();
      }
      break;
    }
  }
}

developerMenu(){
  f3d_lcd_fillScreen(BLACK);
   while(1){
    //get nunchuck input
    f3d_nunchuk_read(&n);

    //Joystick controls
    //down
    if(n.jy > 150){
      step--;
    }
    //up
    if(n.jy < 15){
      step++;
    }
    stepMod = abs(step)%3;

    switch(stepMod) {
    case 0:
      f3d_lcd_drawString(20,10,"Select Level", WHITE,BLACK);
      f3d_lcd_drawString(20,30,"One",RED,BLACK);
      f3d_lcd_drawString(20,40,"Two",WHITE,BLACK);
      f3d_lcd_drawString(20,50,"Three",WHITE,BLACK);
      if(n.c){
	lives = 3;
	score = 0;
	level = 1;
        gameLoop();
      }
      break;
    case 1:
      f3d_lcd_drawString(20,10,"Select Level", WHITE,BLACK);
      f3d_lcd_drawString(20,30,"One",WHITE,BLACK);
      f3d_lcd_drawString(20,40,"Two",RED,BLACK);
      f3d_lcd_drawString(20,50,"Three",WHITE,BLACK);
      if(n.c){
	lives = 3;
	score = 0;
	level = 2;
        gameLoop();
      }
      break;
    case 2:
      f3d_lcd_drawString(20,10,"Select Level", WHITE,BLACK);
      f3d_lcd_drawString(20,30,"One",WHITE,BLACK);
      f3d_lcd_drawString(20,40,"Two",WHITE,BLACK);
      f3d_lcd_drawString(20,50,"Three",RED,BLACK);
      if(n.c){
	lives = 3;
	score = 0;
	level = 3;
        gameLoop();
      }
      break;
    }
   }
}

/*handles user input from nunchuck*/
inputLoop(){

  //read from nunchuck
  f3d_nunchuk_read(&n);

  //move paddle based on nunchuck input

  if(n.jx > 220){//right movement
    //move the user paddle over some
    moveRect(&player, 3, 0, BLACK);

  }else if(n.jx < 20){//left movement
    //move the user paddle over some
    moveRect(&player, -3, 0, BLACK);
  }

 }
////////////////////////////////////////////////////////////////////////////////////////////////////

initgameSetup(){//to be called when a game starts over
  //set lives
  lives = 3;
  score = 0;
  level = 1;
  gameLoop();
}

gameLoop(){
  //initalize and draw objects here initalliy

  //draw our game screen
  f3d_lcd_fillScreen(BLACK);
  createbricks(level);
  int collision;

  //boolean to keep the ball from releasing
  int bool = 1;

  int ball_vx = 0;
  int ball_vy = 0;

  //offset for brick collision
  int bh = BRICK_HEIGHT;
  int bw = (BRICK_THICKNESS -1);
  int bd = BALL_DIM -1;

  //set scoreString, livesString, and levelString
  sprintf(scoreString, "Score = %d", score);
  sprintf(livesString, "Lives = %d", lives);
  sprintf(levelString, "Level = %d", level);

  //draw our paddle and ball
  initRect(&player,60,140,PADDLE_THICKNESS,PADDLE_HEIGHT,WHITE);
  initRect(&ball,40,135,BALL_DIM,BALL_DIM,WHITE);

  //draw lives and level
  f3d_lcd_drawString(5,150,livesString, RED,BLACK);
  f3d_lcd_drawString(70,150,levelString, RED,BLACK); 


//loop our game code
  while(1){
    //must continue to redraw play and ball
    redrawRect(&player);
    redrawRect(&ball);

    //if the bool is true we keep the ball tethered to the paddle
    if(bool){
      while(1){
	inputLoop();
	ball_vx = 0;
	ball_vy = 0;
	if(n.jx > 220){//right movement
	  //move the user ball over some
	  moveRect(&ball, 3, 0, BLACK);
	}else if(n.jx < 20){//left movement
	  //move the user ball over some
	  moveRect(&ball, -3, 0, BLACK);
	}
	//if the button is pressed release the ball
	if(n.z){
	  ball_vx = 1;
	  ball_vy = -1;
	  bool = 0;
	  break;
	}
      }
    }

    //check for user input
    inputLoop();

    //moves and redraws the ball, also reports if there was a colision with boundries
    collision = moveRect(&ball, ball_vx, ball_vy, BLACK);


    //handles ball collision with bricks

    //check if ball in is the first layer
    if(((ball.pos_y <= (brickLayers[2].pos_y + bh)) && (ball.pos_y >= brickLayers[2].pos_y)) 
       ||
       ((ball.pos_y + bd) <= (brickLayers[2].pos_y + bh)) && ((ball.pos_y + bd) >= brickLayers[2].pos_y)){
      //check if the ball pos_x lines up with a bricks x position
      // printf("ball in first layer\n");
      int i;
      for(i=0; i<10; i++){
        //step throught the bricks until you find one the ball has hit
        if(((ball.pos_x <= (brickLayers[2].elements[i].pos_x + bw)) && (ball.pos_x >= brickLayers[2].elements[i].pos_x))
	   ||
	   (((ball.pos_x + bd) <= (brickLayers[2].elements[i].pos_x + bw)) && ((ball.pos_x + bd) >= brickLayers[2].elements[i].pos_x))){
          //if it has been hit check if the brick is alive
          if(brickLayers[2].elements[i].life > 0){
	    printf("ball has hit a brick\n");
            //reduce the bricks life
            brickLayers[2].elements[i].life -= 1;
            //if the bricks life is now 0 destroy it
            if(brickLayers[2].elements[i].life == 0){
              //erase the Brick
              eraseBrick(brickLayers[2].elements[i].pos_x, brickLayers[2].pos_y);
	      bricksDestroyed++;
            }
	    //add to score
            score += 100;
            //set scoreString
            sprintf(scoreString, "Score = %d", score);
	    //score
            f3d_lcd_drawString(20,90,scoreString, RED,BLACK);
            //flip the y velocity
            ball_vy = -ball_vy;
	    ball_vx = -ball_vx;
	    break;
          }
        }
      }
    }

    //check if ball in is the second layer
    if(((ball.pos_y <= (brickLayers[1].pos_y + bh)) && (ball.pos_y >= brickLayers[1].pos_y))
       ||
       (((ball.pos_y + bd) <= (brickLayers[1].pos_y + bh)) && ((ball.pos_y + bd) >= brickLayers[1].pos_y))){
      //check if the ball pos_x lines up with a bricks x position
      // printf("ball in second layer\n");
      int i;
      for(i=0; i<10; i++){
        //step throught the bricks until you find one the ball has hit
        if((ball.pos_x <= (brickLayers[1].elements[i].pos_x + bw) && ball.pos_x >= brickLayers[1].elements[i].pos_x) ||
	   ((ball.pos_x + bd) <= (brickLayers[1].elements[i].pos_x + bw) && (ball.pos_x + bd) >= brickLayers[1].elements[i].pos_x)){
          //if it has been hit check if the brick is alive
          if(brickLayers[1].elements[i].life > 0){
	    //printf("ball has hit a brick\n");
            //reduce the bricks life
            brickLayers[1].elements[i].life -= 1;
            //if the bricks life is now 0 destroy it
            if(brickLayers[1].elements[i].life == 0){
              //erase the Brick
              eraseBrick(brickLayers[1].elements[i].pos_x, brickLayers[1].pos_y);
	      bricksDestroyed++;
            }
            //add to score
            score += 100;
            //set scoreString
            sprintf(scoreString, "Score = %d", score);
                //score
            f3d_lcd_drawString(20,90,scoreString, RED,BLACK);
            //flip the y velocity
            ball_vy = -ball_vy;
	    ball_vx = -ball_vx;
	    break;
          }
        }
      }
    }

    //check if ball in is the third layer
    if(((ball.pos_y <= (brickLayers[0].pos_y + bh)) && (ball.pos_y >= brickLayers[0].pos_y)) 
       ||
       (((ball.pos_y + bd) <= (brickLayers[0].pos_y + bh)) && ((ball.pos_y + bd) >= brickLayers[0].pos_y))){
      //check if the ball pos_x lines up with a bricks x position
      //printf("ball in third layer\n");
      int i;
      for(i=0; i<10; i++){
        //step throught the bricks until you find one the ball has hit
        if(((ball.pos_x <= (brickLayers[0].elements[i].pos_x + bw)) && ball.pos_x >= brickLayers[0].elements[i].pos_x) ||
	   (((ball.pos_x + bd) <= (brickLayers[0].elements[i].pos_x + bw)) && ((ball.pos_x + bd) >= brickLayers[0].elements[i].pos_x))){
          //if it has been hit check if the brick is alive
          if(brickLayers[0].elements[i].life > 0){
	    //printf("ball has hit a brick\n");
            //reduce the bricks life
            brickLayers[0].elements[i].life -= 1;
            //if the bricks life is now 0 destroy it
            if(brickLayers[0].elements[i].life == 0){
              //erase the Brick
              eraseBrick(brickLayers[0].elements[i].pos_x, brickLayers[0].pos_y);
	      bricksDestroyed++;
            }
            //add to score
            score += 100;
            //set scoreString
      	    sprintf(scoreString, "Score = %d", score);
      	        //score
      	    f3d_lcd_drawString(20,90,scoreString, RED,BLACK);
            //flip the y velocity
            ball_vy = -ball_vy;
	    ball_vx = -ball_vx;
	    break;
          }
        }
      }
    }
    //handle PADDLE collision
    if(ball.pos_x + 1 >= player.pos_x && ball.pos_x + 1 <= (player.pos_x + PADDLE_THICKNESS) )
      {
        if(ball.pos_y + 1 >= player.pos_y && ball.pos_y + 1 <= (player.pos_y + PADDLE_HEIGHT)  ){
          ball_vy = -ball_vy;
	  	    //set scoreString
          sprintf(scoreString, "Score = %d", score);
	        //score
          f3d_lcd_drawString(20,90,scoreString, RED,BLACK);
        }
      }

//checks the collision variable that is returned by moveRect and changes ball velocity, or updates score etc.
    switch (collision) {
    case COLLISION_TOP:
      ball_vy = -ball_vy;
      break;
    case COLLISION_BOTTOM:
      if(lives > 0){
	lives--;
	bool = 1;

	//erase ball
	eraseRect(&ball, BLACK);
	
	//update livesString
	sprintf(livesString, "Lives = %d", lives);
	
	//redraw level and lives
	f3d_lcd_drawString(5,150,livesString, RED,BLACK);
	f3d_lcd_drawString(70,150,levelString, RED,BLACK);
	
	//reset ball position
	ball.pos_x = player.pos_x + 25;
	ball.pos_y = player.pos_y - 5;
      }else{
	ball_vx = 0;
	ball_vy = 0;
	f3d_lcd_drawString(44,80,"GAME OVER", RED,BLACK);
	
	//set audio file name for game over
	strcpy(audiofileName, "horn.wav");
	//play audio
	play();
	
	//score
	f3d_lcd_drawString(20,90,scoreString, RED,BLACK);
	//wait for user to restart
	while(1){
	  f3d_nunchuk_read(&n);
	  f3d_lcd_drawString(20,100,"z to restart",RED,BLACK);
	  if(n.z){
	    initgameSetup();
	  }
	}
      }
      break;
    case COLLISION_LEFT:
      ball_vx = -ball_vx;
      break;
    case COLLISION_RIGHT:
      ball_vx = -ball_vx;
      break;
    }
    //check if all bricks are dead
    if(bricksDestroyed == 30){
      level++;
      //update level string and redraw
      sprintf(levelString, "Level = %d", level);
      f3d_lcd_drawString(70,150,levelString, RED,BLACK);
      //set bricksDestroyed back to 0
      bricksDestroyed = 0;
      
      //rerun the game loop with level increased
      gameLoop();
    }
  }
}

quitGame(){
  //fill the screen black the close the game
  f3d_lcd_fillScreen(BLACK);
  f3d_lcd_drawString(10,80,"press reset to start again", RED,BLACK);
  
  exit(0);
}


void play(){

  FRESULT rc;
  DIR dir;
  FILINFO fno;
  UINT bw, br;
  unsigned int retval;
  int bytesread;

  f_mount(0, &Fatfs);/* Register volume work area */

  printf("attempting to open %s\n", audiofileName);
  rc = f_open(&fid, audiofileName, FA_READ);

  if (!rc) {
    struct ckhd hd;
    uint32_t  waveid;
    struct fmtck fck;

    readckhd(&fid, &hd, 'FFIR');

    f_read(&fid, &waveid, sizeof(waveid), &ret);
    if ((ret != sizeof(waveid)) || (waveid != 'EVAW'))
      return ;//-1

    readckhd(&fid, &hd, ' tmf');

    f_read(&fid, &fck, sizeof(fck), &ret);

    // skip over extra info

    if (hd.cksize != 16) {
      printf("extra header info %d\n", hd.cksize - 16);
      f_lseek(&fid, hd.cksize - 16);
    }

    printf("audio format 0x%x\n", fck.wFormatTag);
    printf("channels %d\n", fck.nChannels);
    printf("sample rate %d\n", fck.nSamplesPerSec);
    printf("data rate %d\n", fck.nAvgBytesPerSec);
    printf("block alignment %d\n", fck.nBlockAlign);
    printf("bits per sample %d\n", fck.wBitsPerSample);

    // now skip all non-data chunks !

    while(1){
      readckhd(&fid, &hd, 0);
      if (hd.ckID == 'atad')
	break;
      f_lseek(&fid, hd.cksize);
    }

    printf("Samples %d\n", hd.cksize);

    // Play it !

    //      audioplayerInit(fck.nSamplesPerSec);

    f_read(&fid, Audiobuf, AUDIOBUFSIZE, &ret);
    hd.cksize -= ret;
    printf("playing sound %s\n", audiofileName);
    audioplayerStart();
    while (hd.cksize) {
      int next = hd.cksize > AUDIOBUFSIZE/2 ? AUDIOBUFSIZE/2 : hd.cksize;
      if (audioplayerHalf) {
	if (next < AUDIOBUFSIZE/2)
	  bzero(Audiobuf, AUDIOBUFSIZE/2);
	f_read(&fid, Audiobuf, next, &ret);
	hd.cksize -= ret;
	audioplayerHalf = 0;
      }
      if (audioplayerWhole) {
	if (next < AUDIOBUFSIZE/2)
	  bzero(&Audiobuf[AUDIOBUFSIZE/2], AUDIOBUFSIZE/2);
	f_read(&fid, &Audiobuf[AUDIOBUFSIZE/2], next, &ret);
	hd.cksize -= ret;
	audioplayerWhole = 0;
      }
    }
    audioplayerStop();
    printf("File played\n");
  }

  rc = f_close(&fid);
  printf("\nClose the file.\n");

  if (rc) die(rc);
  // while (1);
}

void drawpicture(){
  //debug
  //f3d_lcd_fillScreen(RED);
  char footer[20];
  int count=0;
  int i;

  FRESULT rc;			/* Result code */
  DIR dir;			/* Directory object */
  FILINFO fno;			/* File information object */
  UINT bw, br;
  unsigned int retval;
  f_mount(0, &Fatfs);

  //debug
  //f3d_lcd_fillScreen(BLUE);

  printf("attempting to open file %s\n", bmpfileName);
  rc = f_open(&fid, bmpfileName, FA_READ);
  if (rc) die(rc);


  rc = f_read(&fid, &magic, sizeof(magic ), &br);
  //  printf("Magic %c%c\n", magic.magic[0], magic.magic[1]);
  rc = f_read(&fid, &header, sizeof(header), &br);
  //printf("file size %d offset %d\n", header.filesz, header.bmp_offset);
  rc = f_read(&fid, &info, sizeof(info), &br);
  //printf("Width %d Height %d, bitspp %d\n", info.width, info.height, info.bitspp);

  //debug
  //f3d_lcd_fillScreen(BLUE);
  uint16_t color;
  uint16_t colors[ST7735_width];
  int row,col;

  for(row = 0; row < 160; row++){

    rc = f_read(&fid, &pix, sizeof pix, &br);
    if(rc) die(rc);

    for (i =0; i < 128; i++) {
      color = ((pix[i].r >> 3) << 11) | ((pix[i].g >> 2) << 5) | (pix[i].b >>3);
      colors[i] = color;
    }
    f3d_lcd_setAddrWindow(0,159-row,ST7735_width-1,159-row,MADCTLGRAPHICS);
    f3d_lcd_pushColor(colors, 128);
  }

  if (rc) die(rc);
  printf("\nClose the file.\n");
  rc = f_close(&fid);
  if (rc) die(rc);
}

#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t* file, uint32_t line) {
  /* Infinite loop */
  /* Use GDB to find out why we're here */
  while (1);
}
#endif


/* main.c ends here */
