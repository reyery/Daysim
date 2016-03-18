#pragma once

float gammln ( float xx );
float betai (float a, float b, float x );
float betacf (float a, float b, float x );
float gasdev ( long *idum );
float ran1 ( long *idum );
void indexx ( unsigned long n, float *arrin, int *indx );
void rank ( unsigned long n, int *indx, int *irank );			     					    
void sort(unsigned long n, float arr[]);  
void four1(float data[], unsigned long nn, int isign);
void realft(float data[], unsigned long n, int isign);
float mean ( int n, float *array );
void mean_var_99 ( float *data, int n, float *mean, float *var);  
