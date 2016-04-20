#pragma once

double gammln(double xx);
double betai(double a, double b, double x);
double betacf(double a, double b, double x);
double gasdev(long *idum);
float ran1 ( long *idum );
void indexx ( unsigned long n, float *arrin, int *indx );
void rank ( unsigned long n, int *indx, int *irank );			     					    
void sort(unsigned long n, float arr[]);  
void four1(float data[], unsigned long nn, int isign);
void realft(float data[], unsigned long n, int isign);
double mean(int n, double *array);
void mean_var_99 ( float *data, int n, float *mean, float *var);  
