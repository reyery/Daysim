#ifndef CHECK_DIRECT_H
#define CHECK_DIRECT_H

extern double alt, azi;
extern float dir, dif;

void  calculate_shading_status_ab0(char *octree, char *direct_sunlight_file_tmp, int LightSourceCounter);

#endif
