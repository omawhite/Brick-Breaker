/* brick.h ---
 *
 * Filename: brick.h
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
#include <stdio.h>
#ifndef _BRICKH_
#define _BRICKH_
/*
#define BRICK_THICKNESS 9
#define BRICK_HEIGHT 5
*/
struct brick{
	int pos_x;
	int life;
	uint16_t color;
};

struct layer{
	int pos_y;
	struct brick elements[10];
};

#endif
