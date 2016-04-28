#include "rterror.h"
#include "ds_shortterm.h"
#include "numerical.h"
#include "ds_constants.h"


/*
 * forward declarations
 */
void beam_nor_clearsky_irradiance_during_hour(int month, int day, double centrum_time, double T_lam2, double *irrad_beam_nor_clear);






void esra_clearsky_irradiance_instant(double solar_elevation, double solar_azimuth, double eccentricity_correction, double site_elevation, \
	double linke_turbidity_factor_am2, double horizon, double *irrad_glo_clear, double *irrad_beam_nor_clear, double *irrad_dif_clear)
{
	/*  ESRA clearsky model; angles in degrees, formulas from [Rig00] = Rigollier et al., Solar Energy 68,33-48,2000     */
	/*  modified: if solar_elevation < 0, then all irradiances = 0 ( as this doesn't affect daylight autonomies anyway ) */

	int azimuth_class;
	double relative_optical_air_mass;
	double solar_elevation_true, delta_solar_elevation;
	double rayleigh_atmosphere_scale_height = 8434.5;     /* in metres */
	double rayleigh_optical_thickness = 0.0;
	double beam_transmittance;
	double a0, a1, a2;
	double diffuse_transmission_function;
	double diffuse_angular_function;

	/*  beam irradiance  */

	delta_solar_elevation = 0.061359*degrees(0.1594 + 1.123 * radians(solar_elevation)
		+ 0.065656 * pow(radians(solar_elevation), 2))
		/ (1 + 28.9344 * radians(solar_elevation) + 277.3971 * pow(radians(solar_elevation), 2));

	solar_elevation_true = solar_elevation + delta_solar_elevation;

	relative_optical_air_mass = exp(-site_elevation/rayleigh_atmosphere_scale_height) \
		/ (sin(radians(solar_elevation_true)) + 0.50572 * pow(solar_elevation_true + 6.07995, -1.6364));

	if ( relative_optical_air_mass <= 20 )
		rayleigh_optical_thickness = 6.62960 + 1.7513 * relative_optical_air_mass - 0.1202 * pow(relative_optical_air_mass,2) \
			+ 0.0065 * pow(relative_optical_air_mass,3) - 0.00013 * pow(relative_optical_air_mass,4);
	if ( relative_optical_air_mass > 20 )
		rayleigh_optical_thickness = 10.4 + 0.718 * relative_optical_air_mass;

	rayleigh_optical_thickness = 1.0 / rayleigh_optical_thickness;

	beam_transmittance = exp ( -0.8662 * linke_turbidity_factor_am2 * relative_optical_air_mass * rayleigh_optical_thickness );

	if ( solar_elevation > 0 )
		{
			*irrad_beam_nor_clear = SOLAR_CONSTANT_E * eccentricity_correction * beam_transmittance;

			if ( horizon == 1 )
				{
					if ( solar_azimuth < 0 )  azimuth_class = ((int)solar_azimuth)/10 + 17;
					else   azimuth_class = ((int)solar_azimuth)/10 + 18;

					if (  solar_elevation <= horizon_azimuth_in[azimuth_class] )       /*   if the sun is under the horizon: global=diffuse  */
						{
							*irrad_beam_nor_clear = 0;
						}
				}

			if ( horizon == 3 )
				{
					if ( solar_azimuth < 0 )  azimuth_class = ((int)solar_azimuth)/10 + 17;
					else   azimuth_class = ((int)solar_azimuth)/10 + 18;

					if (  solar_elevation <= horizon_azimuth_out[azimuth_class] )       /*   if the sun is under the horizon: global=diffuse  */
						{
							*irrad_beam_nor_clear = 0;
						}
				}
		}
	else
		{
			*irrad_beam_nor_clear = 0;
			*irrad_dif_clear = 0;
			*irrad_glo_clear = 0;
			return;
		}

	/*  diffuse irradiance  */

	diffuse_transmission_function = -0.015843 + 0.030543 * linke_turbidity_factor_am2 + 0.0003797 * pow(linke_turbidity_factor_am2,2);

	a0 = 0.26463 - 0.061581 * linke_turbidity_factor_am2 + 0.0031408 * pow(linke_turbidity_factor_am2,2);
	a1 = 2.0402 + 0.018945 * linke_turbidity_factor_am2 - 0.011161 * pow(linke_turbidity_factor_am2,2);
	a2 = -1.3025 + 0.039231 * linke_turbidity_factor_am2 + 0.0085079 * pow(linke_turbidity_factor_am2,2);

	if ( a0 * diffuse_transmission_function < 0.002 )    a0 = 0.002 / diffuse_transmission_function;

	diffuse_angular_function = a0 + a1 * sin(radians(solar_elevation)) + a2 * pow(sin(radians(solar_elevation)), 2);

	*irrad_dif_clear = SOLAR_CONSTANT_E * eccentricity_correction * diffuse_transmission_function * diffuse_angular_function;

	if ( *irrad_dif_clear < 0 ) *irrad_dif_clear = 0;

	/*  global irradiance  */

	*irrad_glo_clear = *irrad_beam_nor_clear * sin(radians(solar_elevation)) + *irrad_dif_clear;
}

void glo_and_beam_indices_hour(double latitude, double longitude, double time_zone,
	int jday, double centrum_time, int solar_time,
	double irrad_glo, double irrad_beam_nor,
	double *index_glo, double *index_beam)
{
	int i, month;
	int sph_k = 60;

	double dt_k, time_i;
	double irrad_glo_clear, irrad_beam_nor_clear;
	double solar_elevation_i, solar_azimuth_i, eccentricity_correction_i;
	double irrad_glo_clear_i, irrad_beam_nor_clear_i, irrad_dif_clear_i;
	double sum_irrad_glo_clear, sum_irrad_beam_nor_clear;
	double sunrise_localtime, sunset_localtime;

	sunrise_sunset_localtime ( latitude, longitude, time_zone, jday, &sunrise_localtime, &sunset_localtime );

	if ( (centrum_time + 0.5) < sunrise_localtime  || (centrum_time - 0.5) > sunset_localtime )
		{     *index_glo = 1;  *index_beam = 1;   }
	else  if (  (centrum_time + 0.5 - sunrise_localtime) < 0.0  ||  (sunset_localtime - (centrum_time - 0.5)) < 0.0  )
		{  *index_glo = 1;  *index_beam = 1;   }

	else
		{
			month = day_to_month(jday);
			dt_k = 3600.0 / sph_k;

			sum_irrad_glo_clear = 0;
			sum_irrad_beam_nor_clear = 0;

			for ( i=1 ; i <= sph_k ; i++ )          /*  equidistant and symmetrically around "centrum_time"  */
				{
					time_i = centrum_time - 0.5 + (i - 0.5) / (float) sph_k;

					solar_elev_azi_ecc (latitude, longitude, time_zone, jday, \
										time_i, solar_time, &solar_elevation_i, &solar_azimuth_i, &eccentricity_correction_i);

					esra_clearsky_irradiance_instant ( solar_elevation_i, solar_azimuth_i, eccentricity_correction_i, site_elevation,\
													   linke_turbidity_factor_am2[month-1], horizon_in, \
													   &irrad_glo_clear_i, &irrad_beam_nor_clear_i, &irrad_dif_clear_i );

					if ( irrad_glo_clear_i > 0 )  sum_irrad_glo_clear += irrad_glo_clear_i;

					if ( irrad_beam_nor_clear_i > 0 )  sum_irrad_beam_nor_clear += irrad_beam_nor_clear_i;

				}

			irrad_glo_clear = sum_irrad_glo_clear / (float) sph_k;
			irrad_beam_nor_clear = sum_irrad_beam_nor_clear / (float) sph_k;

			if ( irrad_glo_clear > 0 )        *index_glo = irrad_glo / irrad_glo_clear;
			else    *index_glo = 1;
			if ( irrad_beam_nor_clear > 0 )    *index_beam = irrad_beam_nor / irrad_beam_nor_clear;
			else   *index_beam = 1;

			if ( *index_glo > 1.5 )  *index_glo = 1.5;
			if ( *index_beam > 1.0 )  *index_beam = 1.0;
		}
}

void irrads_clear_st(double latitude, double longitude, double time_zone, int jday, double centrum_time, int timecode, int sph, double *irrads_glo_clear_st)
{
	int i, month;
	double time_i;
	double solar_elevation_i, solar_azimuth_i, eccentricity_correction_i;
	double irrad_glo_clear_i, irrad_beam_nor_clear_i, irrad_dif_clear_i;

	month = day_to_month(jday);

	for ( i=1 ; i <= sph ; i++ )
		{
			time_i = centrum_time - 0.5 + (i - 0.5) / (float) sph;

			solar_elev_azi_ecc ( latitude, longitude, time_zone, jday, \
								 time_i, solar_time, &solar_elevation_i, &solar_azimuth_i, &eccentricity_correction_i );

			esra_clearsky_irradiance_instant ( solar_elevation_i, solar_azimuth_i, eccentricity_correction_i,\
											   site_elevation, linke_turbidity_factor_am2[month-1], horizon_out+2,\
											   &irrad_glo_clear_i, &irrad_beam_nor_clear_i, &irrad_dif_clear_i );

			irrads_glo_clear_st[i - 1] = (float)irrad_glo_clear_i;
		}
}

void estimate_linke_factor_from_hourly_direct_irradiances()
{
	int i, j, status=6, npm=3, counts;
	int jday, month, day;

	double time, centrum_time, sunrise_localtime, sunset_localtime;
	double solar_elevation, solar_azimuth, eccentricity_correction;
	double delta_st_ti, difference_beam, linke_estimate;
	double irrad_beam_nor, irrad_beam_nor_clear, irrad_beam_hor, irrad_dif;
	float daily_linke_estimate[366];

	for (i=0;i<366;i++)   daily_linke_estimate[i]=100;

	while ( status > 0 )                       /*  as long as EOF is not reached  */
		{
			if ( input_units_genshortterm == 1 )
				status = fscanf(HOURLY_DATA,"%d %d %f %f %f", &month, &day, &time, &irrad_beam_nor, &irrad_dif);
			if ( input_units_genshortterm == 2 )
				status = fscanf(HOURLY_DATA,"%d %d %f %f %f", &month, &day, &time, &irrad_beam_hor, &irrad_dif);
			if ( status <= 0 )  goto end;

			jday = jdate(month, day);
			if ( input_units_genshortterm == 2 )                             /*  calculate irrad_beam_nor  */
				{
					if ( irrad_beam_hor > 0 )
						{
							sunrise_sunset_localtime ( latitude, longitude, time_zone, jday, &sunrise_localtime, &sunset_localtime );
							centrum_time=time;
							if ( fabs(time-sunrise_localtime) <= 0.5 )  centrum_time=sunrise_localtime+(time+0.5-sunrise_localtime)/2.0;
							if ( fabs(time-sunset_localtime) <= 0.5 )  centrum_time=time-0.5+(sunset_localtime-(time-0.5))/2.0;
							solar_elev_azi_ecc ( latitude, longitude, time_zone, jday, \
												 centrum_time, solar_time, &solar_elevation, &solar_azimuth, &eccentricity_correction);
							irrad_beam_nor = irrad_beam_hor / sin(radians(solar_elevation));
							if ( irrad_beam_nor < 0 )  irrad_beam_nor=0;
						}
					else irrad_beam_nor=0;
				}

			solar_elev_azi_ecc(latitude,longitude,time_zone,jday,time,solar_time,&solar_elevation,&solar_azimuth,&eccentricity_correction);

			if ( solar_elevation > 10 )
				{
					if ( solar_time == 0 )   delta_st_ti = 0.170 * sin( (4*PI/373) * (jday - 80) )\
												 - 0.129 * sin( (2*PI/355) * (jday - 8) ) + 12/180.0 * (time_zone - longitude);
					else  delta_st_ti = 0;

					if ( time > 10-delta_st_ti && time < 14-delta_st_ti )    /*  regard only the 4 hours symmetrically around solar noon  */
						{
							difference_beam=1000;
							linke_estimate=100;

							for (i=0;i<=55;i++)     /*  for every hour the best fitting linke factor in [1.5,7.0] is determined  */
								{
									beam_nor_clearsky_irradiance_during_hour ( month, day, time, 1.5+i/10.0, &irrad_beam_nor_clear );

									if ( fabs(irrad_beam_nor-irrad_beam_nor_clear) < difference_beam )
										{
											difference_beam=fabs(irrad_beam_nor-irrad_beam_nor_clear);
											linke_estimate=1.5+i/10.0;
										}
								}

							if (linke_estimate < daily_linke_estimate[jday])   daily_linke_estimate[jday] = (float)linke_estimate;
						}
				}
		}

 end:
	{ /*for (i=1;i<366;i++)  printf("linke[%d]=%f\n",i,daily_linke_estimate[i]);*/
		sort(31,&daily_linke_estimate[0]);          /*  sort the daily estimates in ascending order for every month  */
		sort(28,&daily_linke_estimate[31]);
		sort(31,&daily_linke_estimate[59]);
		sort(30,&daily_linke_estimate[90]);
		sort(31,&daily_linke_estimate[120]);
		sort(30,&daily_linke_estimate[151]);
		sort(31,&daily_linke_estimate[181]);
		sort(31,&daily_linke_estimate[212]);
		sort(30,&daily_linke_estimate[243]);
		sort(31,&daily_linke_estimate[273]);
		sort(30,&daily_linke_estimate[304]);
		sort(31,&daily_linke_estimate[334]);

		for (i=0;i<12;i++)     /*  estimate linke factor for every month as the mean of the npm=3 smallest values of the month    */
			{                      /*  if there are less than npm values < 6.99 per month: set linke factor to the default value 3.0  */
				linke_turbidity_factor_am2[i] = 0;
				counts=0;
				for (j = 0; j < npm; j++) {
					jday = jdate(i + 1, j + 1);
					if (daily_linke_estimate[jday] > 1 && daily_linke_estimate[jday] < 6.99)
					{
						linke_turbidity_factor_am2[i] += daily_linke_estimate[jday];
						counts++;
					}
				}

				if (counts==npm)  linke_turbidity_factor_am2[i] = linke_turbidity_factor_am2[i]/npm;
				else
					{
						sprintf(errmsg, "Linke Turbidity could not be estimated for month=%d due too cloudy weather conditions or insufficient data", i + 1);
						error(WARNING, errmsg);
						linke_turbidity_factor_am2[i] = 3.0;
					}
			}
	}
}

void beam_nor_clearsky_irradiance_during_hour(int month, int day, double centrum_time, double T_lam2, double *irrad_beam_nor_clear)
{
	int i;
	int steps_k=60;
	int jday;

	double dt_k, time_i;
	double solar_elevation_i, solar_azimuth_i, eccentricity_correction_i;
	double irrad_glo_clear_i, irrad_beam_nor_clear_i, irrad_dif_clear_i;
	double sum_irrad_beam_nor_clear = 0;

	jday = jdate(month, day);

	dt_k = 1.0/steps_k;

	for ( i=1 ; i <= steps_k ; i++ )          /*  equidistant and symmetrically around "centrum_time"  */
		{
			time_i = centrum_time - 0.5 + (i - 0.5) * dt_k;

			solar_elev_azi_ecc ( latitude, longitude, time_zone, jday, \
								 time_i, solar_time, &solar_elevation_i, &solar_azimuth_i, &eccentricity_correction_i);

			esra_clearsky_irradiance_instant ( solar_elevation_i, solar_azimuth_i, eccentricity_correction_i, site_elevation, T_lam2, horizon_in,\
											   &irrad_glo_clear_i, &irrad_beam_nor_clear_i, &irrad_dif_clear_i );

			sum_irrad_beam_nor_clear += irrad_beam_nor_clear_i;
		}

	*irrad_beam_nor_clear = sum_irrad_beam_nor_clear / (float) steps_k;
}

