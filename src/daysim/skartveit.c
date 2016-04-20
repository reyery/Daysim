#include <rtmath.h>
#include <stdio.h>
#include <stdlib.h>
#include <rterror.h>

#include "numerical.h"
#include "skartveit.h"
#include "ds_constants.h"


/* Defined in gen_reindl.c */
extern int new;
extern long random_seed;

void skartveit(float *indices_glo, float index_beam, int sph, float previous_ligoh, float *indices_glo_st, float *actual_ligoh)
{
  int i, est_glo=1;
  int *glo_ranking;
  float sigma_glo, sigma_beam, act_ligoh;

  if ( (glo_ranking = malloc ((sph+1)*sizeof(int))) == NULL  )   { error(SYSTEM, "out of memory in skartveit"); }

  estimate_sigmas ( indices_glo, index_beam, sph, &sigma_glo, &sigma_beam );

  if ( sigma_glo < 0.0001 || indices_glo[1] <= 0 || indices_glo[1] >= 1.5 )
  {
    for ( i=1 ; i<=sph ; i++ )   indices_glo_st[i-1] = indices_glo[1];
    est_glo = 0;
  }
  if ( est_glo )  estimate_indices_glo_st ( indices_glo[1], index_beam, sph, sigma_glo, previous_ligoh,\
                                            glo_ranking, indices_glo_st, &act_ligoh );

  *actual_ligoh = act_ligoh;

  free(glo_ranking);
}

void estimate_sigmas ( float *indices_glo, float index_beam, int sph, double *sigma_glo, float *sigma_beam )
{
  double sig3, sigstar, fKb=1.0;
  double sigg5, delg;
  double sigbx, sigb5, delb, sigbdif;

  double gam, alpha, ran, s, time_step;

  sigbx = index_beam * ( 1 - index_beam );           /*  steht nicht im paper  */
  if ( index_beam < 1 )   sigbx = sqrt ( sigbx );

  sig3 = pow ( 0.5 * ( pow ( indices_glo[1] - indices_glo[0] , 2 ) + pow ( indices_glo[1] - indices_glo[2] , 2 ) ) , 0.5 );
  /*sigstar = 0.87 * pow ( indices_glo[1] , 2 ) * ( 1 - indices_glo[1] ) + 0.39 * pow ( indices_glo[1] , 0.5 ) * sig3;*/

  if ( new )
  {
    if ( indices_glo[1] <= 0.3 )  fKb = 1.3;
    else if ( indices_glo[1] <= 0.6 )  fKb = 1.3;
	else if (indices_glo[1] <= 0.9)  fKb = 1.0;
	else  fKb = 0.7;
  }

  sigstar = ( 0.87 * pow ( indices_glo[1] , 2 ) * ( 1 - indices_glo[1] ) + 0.39 * pow ( indices_glo[1] , 0.5 ) * sig3 ) * fKb;
  if ( sigstar < 0 )   sigstar = 0;
  gam = 0.88 + 42 * pow ( sigstar , 2 );
  alpha = exp ( gammln(1+1.0/gam) );

  ran = ran1 ( &random_seed );
  s = 1/alpha * exp ( 1/gam * log ( log ( 1/(1-ran) ) ) );
  sigg5 = s * sigstar;
  sigb5 = sigg5 * ( 15 * exp ( -0.15 * pow ( indices_glo[1] - 0.65 , 2 ) ) - 14.08 + 1.85 * ( sigg5 - 0.42 ) * sin ( 1.5 * PI * indices_glo[1] ) );
  if ( sigb5 < 0 )   sigb5 = 0;

  time_step = 60.0 / sph;
  if ( time_step > 5 )   time_step = 5;
  delg = ( 0.6 - 1.3 * sigg5 ) * ( indices_glo[1] - 0.2 );
  *sigma_glo = sigg5 * ( 1 + pow ( (5-time_step)/5 , 1.5 ) * delg );
  delb = 0.18 + ( 0.2 - sigb5 ) * ( 0.6 + 2.5 * pow ( index_beam - 0.5 , 2 ) );
  *sigma_beam = sigb5 * ( 1 + pow ( (5-time_step)/5 , 1.5 ) * delb );

  sigbdif = sigbx - *sigma_beam;
  if ( *sigma_glo < sigg5 )   *sigma_glo = sigg5;
  if ( *sigma_beam < sigb5 )   *sigma_beam = sigb5;
  if ( *sigma_beam > sigbx )   *sigma_beam = sigbx - 0.01;
  if ( sigbdif < 0.01 )   *sigma_beam = sigbx - 0.01;
  if ( *sigma_beam < 0 )   *sigma_beam = 0.0001;
}

void estimate_indices_glo_st ( float index_glo, float index_beam, int sph, float sigma_glo, float previous_ligoh,\
                               int *glo_ranking, float *indices_glo_st, float *actual_ligoh)
{
  int i, j;
  int steps_cdf = 40;          /*  the number of discretization steps was increased from 20 to 40  */
  int *indx, *irank;
  int ar_ok=0, rank_of_previous_ligoh, odd=1, count=0;
  float rgmax, rgmin, tobs, sigt, attenuation_1=1.0, attenuation_2=1.0;
  float t1, t2, w, tt11, tt22, ttt11, ttt22, sig11, sig22;
  float a1, a2, b1, b2, help1, help2;
  float cdf1, cdf2, p;
  float time_step, auto_corr, tau, sdev, ran, ar1, *ar1_series1, *ar1_series2, *indices_glo_st_order;
  float *t, *cdf;
  float *t_st;

  if ((indices_glo_st_order = malloc(sph*sizeof(float))) == NULL) goto memerr;
  if ((ar1_series1 = malloc((sph + 1)*sizeof(float))) == NULL) goto memerr;
  if ((ar1_series2 = malloc((sph + 1)*sizeof(float))) == NULL) goto memerr;
  if ((indx = malloc((sph + 1)*sizeof(int))) == NULL) goto memerr;
  if ((irank = malloc((sph + 1)*sizeof(int))) == NULL) goto memerr;
  if ((t_st = malloc((sph + 1)*sizeof(float))) == NULL) goto memerr;
  if ((t = malloc((steps_cdf + 1)*sizeof(float))) == NULL) goto memerr;
  if ((cdf = malloc((steps_cdf + 1)*sizeof(float))) == NULL) goto memerr;

  if ( new )
  {
    if ( index_glo > -0.1 && index_glo <= 0.3 )  {  attenuation_1=-0.03;  attenuation_2=1.66; }
    if ( index_glo > 0.3 && index_glo <= 0.6 )   {  attenuation_1=0.20;  attenuation_2=4.26; }
    if ( index_glo > 0.6 && index_glo <= 0.9 )   {  attenuation_1=0.47;  attenuation_2=0.89; }
    if ( index_glo > 0.9 && index_glo <= 85 )    {  attenuation_1=2.00;  attenuation_2=0.38; }
  }

  rgmin = ( index_glo - 0.03 ) * exp ( -11 * attenuation_1 * pow ( sigma_glo , 1.4 ) ) - 0.09;
  rgmax = ( index_glo - 1.5 ) * exp ( -9 * attenuation_2 * pow ( sigma_glo , 1.3 ) ) + 1.5;
  if ( rgmin < 0 )   rgmin = 0;
  tobs = ( index_glo - rgmin ) / ( rgmax - rgmin );
  sigt = sigma_glo / ( rgmax - rgmin );
  t1 = tobs * ( 0.01 + 0.98 * exp ( -60 * pow ( sigt , 3.3 ) ) );
  t2 = ( tobs - 1 ) * ( 0.01 + 0.98 * exp ( -11 * pow ( sigt , 2 ) ) ) + 1;
  w = ( t2 - tobs ) / ( t2 - t1 );
  tt11 = pow ( t1 , 2 ) * ( 1 - t1 ) / ( 1 + t1 );
  tt22 = pow ( t2 , 2 ) * ( 1 - t2 ) / ( 1 + t2 );
  ttt11 = t1 * pow ( 1 - t1 , 2 ) / ( 2 - t1 );
  ttt22 = t2 * pow ( 1 - t2 , 2 ) / ( 2 - t2 );
  sig11 = 0.014;
  sig22 = 0.006;
  if ( sig11 >= tt11 )   sig11 = tt11;
  if ( sig22 >= tt22 )   sig22 = tt22;
  if ( sig11 >= ttt11 )   sig11 = ttt11;
  if ( sig22 >= ttt22 )   sig22 = ttt22;

  a1 = pow ( t1 , 2 ) * ( 1 - t1 ) / sig11 - t1;
  if (a1<1)  a1=1;
  a2 = pow ( t2 , 2 ) * ( 1 - t2 ) / sig22 - t2;
  if (a2<1)  a2=1;
  b1 = ( t1 * ( 1 - t1 ) - sig11 ) * ( 1 - t1 ) / sig11;
  if (b1<1)  b1=1;
  b2 = ( t2 * ( 1 - t2 ) - sig22 ) * ( 1 - t2 ) / sig22;
  if (b2<1)  b2=1;

  t[0] = 0;                     /*  discrete form of the cumulated distribution (sum of two incomplete beta functions)  */
  t[steps_cdf] = 1;
  cdf[0] = 0;
  cdf[steps_cdf] = 1;
  for ( i=1 ; i <= steps_cdf-1 ; i++ )
  {
    t[i] = i/(float)steps_cdf;
    help1 = betai ( a1, b1, t[i] );
    cdf1 = help1;
    help2 = betai ( a2, b2, t[i] );
    cdf2 = help2;
    if ( help1 > -98 && help2 > -98 )  cdf[i] = w * cdf1 + ( 1 - w ) * cdf2;
    else  cdf[i] = cdf[i-1];
  }

  if ( new )
  {
    for ( i=1 ; i <= sph ; i++ )     /*  draw 60 random numbers from p(t) via its cumulated distribution  */
    {
      ran = ran1(&random_seed);
      for ( j=1 ; j <= steps_cdf ; j++ )
       if ( ran >= cdf[j-1] && ran <= cdf[j] )
       {
	 t_st[i] = t[j-1] + ( ran - cdf[j-1] ) * ( t[j] - t[j-1] ) / ( cdf[j] - cdf[j-1] );
	 break;
       }
    }


    sort(sph,&t_st[0]); /*for ( i=1 ; i <= sph ; i++ ) printf("%.3f\n",t_st[i]);*/
    for ( i=1 ; i <= sph ; i++ )  indices_glo_st_order[i-1] = rgmin + ( rgmax - rgmin ) * t_st[i];
  }

  else
  {
    for ( i=1 ; i <= sph ; i++ )     /*  draw 60 random numbers from p(t) equidistantly via its cumulated distribution  */
    {
      p = (i-0.5)/(float)sph;
      for ( j=1 ; j <= steps_cdf ; j++ )
       if ( p >= cdf[j-1] && p <= cdf[j] )
       {
	 t_st[i] = t[j-1] + ( p - cdf[j-1] ) * ( t[j] - t[j-1] ) / ( cdf[j] - cdf[j-1] );
	 break;
       }
      indices_glo_st_order[i-1] = rgmin + ( rgmax - rgmin ) * t_st[i];
    }
  }


  /*  temporal rearrangement of the 60 drawn shortterm indices  */

  if ( previous_ligoh <= indices_glo_st_order[0] )  rank_of_previous_ligoh=1;
  for ( i=1 ; i<=sph-1 ; i++)
   if ( previous_ligoh > indices_glo_st_order[i-1] && previous_ligoh <= indices_glo_st_order[i] )  rank_of_previous_ligoh=i;
  if ( previous_ligoh > indices_glo_st_order[sph-1] )  rank_of_previous_ligoh=sph;

  time_step = 60.0/sph;
  auto_corr = 0.896 + 0.084 * exp ( -0.11 * time_step );
  if ( auto_corr > 0.99 )   auto_corr = 0.99;
  sdev = sqrt ( 1 - pow ( auto_corr , 2 ) );
  tau = -1/log(auto_corr);

  ar1 = 0;
  for ( i=1 ; i <= 2*ceil(tau) ; i++ )       /*  wait till "stationarity" is reached */
  {
    ran = sdev * gasdev (&random_seed);
    ar1 = auto_corr * ar1 + ran;
  }

  ar1_series1[0] = ar1;              /*   ATTENTION: ar1_series[0] is ignored, only ar1_series[1,..,sph] is ranked  */
  for ( i=1 ; i <= sph ; i++ )
  {
    ran = sdev * gasdev (&random_seed);
    ar1_series1[i] = auto_corr * ar1_series1[i-1] + ran;
  }

  indexx ( sph, &ar1_series1[0], &indx[0] );   /*for ( i=1 ; i <= sph ; i++ )  printf("%.3f\n",ar1_series1[i]);*/

  if ( new )
  {
    if ( indx[rank_of_previous_ligoh] == 1 )  ar_ok=1;

    while ( ar_ok == 0 )        /*  ar_ok=1 when the discontinuity between two subsequent hours is minimized  */
    {
      if (odd==1)
      {
	for ( i=1 ; i <= sph-1 ; i++ )  ar1_series2[i]=ar1_series1[i+1];
	ar1_series2[sph] = auto_corr * ar1_series2[sph-1] + sdev * gasdev (&random_seed);
	indexx ( sph, &ar1_series2[0], &indx[0] );
	if ( indx[rank_of_previous_ligoh] == 1 )  ar_ok=1;
	odd=0;
      }

      else
      {
	for ( i=1 ; i <= sph-1 ; i++ )  ar1_series1[i]=ar1_series2[i+1];
	ar1_series1[sph] = auto_corr * ar1_series1[sph-1] + sdev * gasdev (&random_seed);
	indexx ( sph, &ar1_series1[0], &indx[0] );
	if ( indx[rank_of_previous_ligoh] == 1 )  ar_ok=1;
	odd=1;
      }
      count++;
    }
  }

  rank ( sph, &indx[0], &irank[0] );

  for ( i=1 ; i <= sph ; i++ )   indices_glo_st[i-1] = indices_glo_st_order[irank[i]-1];    /*  rearrange short-term indices  */

  glo_ranking[0] = 0;
  for ( i=1 ; i <= sph ; i++ ) 	 glo_ranking[i] = irank[i];

  *actual_ligoh = indices_glo_st[sph-1];

  free(indices_glo_st_order);
  free(indx);
  free(irank);
  free(t_st);
  free(t);
  free(cdf);
  free(ar1_series1);
  free(ar1_series2);
  return;

memerr:
  error(SYSTEM, "out of memory in estimate_indices_glo_st");
}

float diffuse_fraction(float irrad_glo, float solar_elevation, float eccentricity_correction)
{
	/*  estimation of the diffuse fraction according to Reindl et al., Solar Energy, Vol.45, pp.1-7, 1990  = [Rei90]  */
	/*                        (reduced form without temperatures and humidities)                                                                          */

	float irrad_ex;
	float index_glo_ex;
	float dif_frac = 0.0;

	if (solar_elevation > 0)  irrad_ex = SOLAR_CONSTANT_E * eccentricity_correction * sin(DTR*solar_elevation);
	else irrad_ex = 0;

	if (irrad_ex > 0)   index_glo_ex = irrad_glo / irrad_ex;
	else  return 0;

	if (index_glo_ex < 0)  { fprintf(stderr, "negative irrad_glo in diffuse_fraction_th\n"); exit(1); }
	if (index_glo_ex > 1)  { index_glo_ex = 1; }

	if (index_glo_ex <= 0.3)
	{
		dif_frac = 1.02 - 0.254*index_glo_ex + 0.0123*sin(DTR*solar_elevation);
		if (dif_frac > 1)  { dif_frac = 1; }
	}

	if (index_glo_ex > 0.3 && index_glo_ex < 0.78)
	{
		dif_frac = 1.4 - 1.749*index_glo_ex + 0.177*sin(DTR*solar_elevation);
		if (dif_frac > 0.97)  { dif_frac = 0.97; }
		if (dif_frac < 0.1)   { dif_frac = 0.1; }
	}

	if (index_glo_ex >= 0.78)
	{
		dif_frac = 0.486*index_glo_ex - 0.182*sin(DTR*solar_elevation);
		if (dif_frac < 0.1)  { dif_frac = 0.1; }
	}

	return dif_frac;
}
