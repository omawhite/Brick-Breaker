/* rect.c ---
 *
 * Filename: rect.c
 * Description:
 * Author: Bryce Himebaugh
 * Maintainer:
 * Created: Wed Aug 13 10:50:11 2014
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

/* Copyright (c) 2004-2007 The Trustees of Indiana University and
 * Indiana University Research and Technology Corporation.
 *
 * All rights reserved.
 *
 * Additional copyrights may follow
 */

/* Code: */
#include <rect.h>
#include <f3d_lcd_sd.h>
#include <brick.h>


/*initializes a rectangle*/
void initRect(rect_t *rect, uint8_t x, uint8_t y, uint8_t width, uint8_t depth, uint16_t color) {
  rect->pos_x = x;
  rect->pos_y = y;
  rect->width = width;
  rect->depth = depth;
  rect->color = color;
  //int x2 = x + width;
  //int y2 = y + depth;
  f3d_lcd_drawRectangle(rect->color, rect->pos_x, rect->pos_y, rect->pos_x + rect->width, rect->pos_y + rect->depth);
}
/*erases a rectangle*/
void eraseRect(rect_t *rect, uint16_t background_color) {
  f3d_lcd_drawRectangle(background_color, rect->pos_x, rect->pos_y, rect->pos_x + rect->width, rect->pos_y + rect->depth);
}
/*redraws a rectangle*/
void redrawRect(rect_t *rect) {
  f3d_lcd_drawRectangle(rect->color, rect->pos_x, rect->pos_y, rect->pos_x + rect->width, rect->pos_y + rect->depth);
}
/*moves a rectangle given a rectange and the velocity in both the x and y direction (as well as a background color)*/
int moveRect(rect_t *rect, int8_t delta_x, int8_t delta_y, uint16_t background_color) {
  int xtemp;
  int ytemp;
  int collision = 0;

  // erase current rectangle
  //f3d_lcd_drawRectangle(rect->color, rect->pos_x, rect->pos_y, rect->width, rect->depth);
  //drawRect(rect->pos_x, rect->pos_y, rect->width, rect->depth, background_color);
  eraseRect(rect,background_color);


  // update x,y postion based on deltas,
  xtemp = (int) (rect->pos_x + delta_x);   // cast as int to gain benefit of sign and larger size
  ytemp = (int) (rect->pos_y + delta_y);

//these are if the object goes out of bounds
  if (xtemp < 1) {
    xtemp = 1;
    collision = COLLISION_LEFT;
  }
  else if (xtemp > (ST7735_width - 1 - rect->width)) {
    xtemp = ST7735_width - rect->width;
    collision = COLLISION_RIGHT;
  }
  if (ytemp < 0) {
    ytemp = 0;
    collision = COLLISION_TOP;
  }
  else if (ytemp > (ST7735_height - rect->depth)) {
    ytemp = ST7735_height - rect->depth;
    collision = COLLISION_BOTTOM;
  }
  rect->pos_x = (uint8_t) xtemp;
  rect->pos_y = (uint8_t) ytemp;

  // draw the new rectangle
 redrawRect(rect);


  return (collision);
}

/* rect.c ends here */
