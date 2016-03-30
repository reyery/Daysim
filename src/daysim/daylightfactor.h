#ifndef DAYLIGHTFACTOR_H
#define DAYLIGHTFACTOR_H


extern float diffuse_dc[149], overcast[149],DZ[149];
extern float *daylight_factor;

/* function declarations */
void getDaylightFactor( );
double CIE_SKY(double Dz);



#endif
