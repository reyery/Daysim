#ifndef PI
#include <rtmath.h>
#endif

#define radians(deg)	((deg)*(PI/180.0))	/* degrees into radians */
#define degrees(rad)	((rad)*(180.0/PI))	/* radians into degrees */

#define HOURS_PER_YEAR	8760
#define SKY_PATCHES	145	/* assumed maximum # Klems patches */
#define DAYLIGHT_COEFFICIENTS	(SKY_PATCHES + 3)	/* 145 sky and 3 ground patches */
#define WHTEFFICACY				179
#define AU					149597890E3
#define SOLAR_CONSTANT_E	1367	/* solar constant W/m^2 */
#define SOLAR_CONSTANT_L	127.5	/* solar constant klux */
