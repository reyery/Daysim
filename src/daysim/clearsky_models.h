#ifndef CLEAR_SKY_MODELS_H
#define CLEAR_SKY_MODELS_H




void esra_clearsky_irradiance_instant ( float solar_elevation, float solar_azimuth,
										float eccentricity_correction, float site_elevation,
                                        float linke_turbidity_factor_am2, int horizon,
										float *irrad_glo_clear, float *irrad_beam_nor_clear,
										float *irrad_dif_clear );				   
					  
void glo_and_beam_indices_hour(double latitude, double longitude, double time_zone, int jday, double centrum_time, int solar_time,
	double irrad_glo, double irrad_beam_nor, double *index_glo, double *index_beam);

void irrads_clear_st(double latitude, double longitude, double time_zone, int jday, double centrum_time, int timecode, int sph, double *irrads_glo_clear_st);

void estimate_linke_factor_from_hourly_direct_irradiances();

void beam_nor_clearsky_irradiance_during_hour ( int month, int day, float centrum_time,
												float T_lam2, float *irrad_beam_nor_clear );




#endif
