#ifndef SHADOW_TESTING_H
#define SHADOW_TESTING_H

extern int *shadow_testing_results;

int shadow_testing(int number_direct_coefficients);
int shadow_test_single(double sun_x, double sun_y, double sun_z);

#endif
