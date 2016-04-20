#ifndef CLEAR_SKY_MODELS_H
#define CLEAR_SKY_MODELS_H




void esra_clearsky_irradiance_instant(double solar_elevation, double solar_azimuth, double eccentricity_correction, double site_elevation,
	double linke_turbidity_factor_am2, double horizon, double *irrad_glo_clear, double *irrad_beam_nor_clear, double *irrad_dif_clear);

void glo_and_beam_indices_hour(double latitude, double longitude, double time_zone, int jday, double centrum_time, int solar_time,
	double irrad_glo, double irrad_beam_nor, double *index_glo, double *index_beam);

void irrads_clear_st(double latitude, double longitude, double time_zone, int jday, double centrum_time, int timecode, int sph, double *irrads_glo_clear_st);

void estimate_linke_factor_from_hourly_direct_irradiances();

void beam_nor_clearsky_irradiance_during_hour(int month, int day, double centrum_time, double T_lam2, double *irrad_beam_nor_clear);



#endif
