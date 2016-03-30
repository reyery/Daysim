#ifndef DS_ILLUM_H
#define DS_ILLUM_H

#include <stdio.h>
#include <math.h>
#include <string.h>


extern int 		CalculateLuminance;
extern int 		CalculateSolarRadiation;
extern int      start_month, start_day;
extern int      all_warnings; /* defines the number or error messages */
extern float    start_hour, centrum_hour;
extern char	    direct_file[1024];
extern char	    ill_file[1024];
extern char	    temp_octree[1024];
extern char	    temp_rad[1024];
extern int      number_dc_points;
extern float**  dc;
extern int      direct_view[10];


#ifndef PROCESS_ROW
extern float*** dc_shading;
extern int***   dc_shading_next;
#endif

extern float*   SkyPatchLuminance;
extern float*   SkyPatchSolarRadiation;
extern int*     NextNonEmptySkyPatch;
extern float    Dx_dif_patch[148],Dy_dif_patch[148],Dz_dif_patch[148]; /*direction of diffuse sky patches */
extern FILE     *DIRECT_SUNLIGHT_FILE;
extern FILE     *INPUT_DATAFILE;
extern FILE     *SHADING_ILLUMINANCE_FILE[100]; // file pointer for the different blind settings defined in "shading"

/* introduce shading*/
extern int solar_pen; /*switch: if =1,
				   then the input direct and diffuse irradiances are changed into
				   dir=0 and dif -> reduction_dif*dif */
extern float lower_lat,upper_lat,lower_azi,upper_azi, reduction_diffuse;

/* i/o format */
extern int	OutputUnits;	/*define the unit of the output (sky luminance or radiance): visible watt=0, solar watt=1, lumen=2*/
extern int	input_unities;	/*define the input for the calulation*/
/* Perez parameters: epsilon, delta =0*/
/* direct normal Irradiance [W/m^2] diffuse horizontal Irrad. [W/m^2] =1 */
/* direct normal Illuminance [Lux] diffuse horizontal Ill. [Lux] =2*/
/* direct horizontal Irradiance [W/m^2] diffuse horizontal Irrad. [W/m^2] =3 */
extern int	input_type; /* input data format */
/* month, day, hour, direct, diffuse =2*/
extern double lowest_altitude;		/* lowest solar altitude in degree */
extern char  *progname;


/*file names*/
extern char input_file[200], input_datafile[200], Perez_outputfile[200],direct_points_file[200];
extern char diffuse_points_file[200], base_file[200], points_input_file[200];
extern FILE *PEREZ_OUTPUTFILE;
extern FILE *DC_FILE;
extern FILE *ILL_FILE;
/* diverse arrays */
extern float diffuse_pts[145][2]; /* position of sky patches of diffuse daylight coefficients */
extern float direct_pts[288][4];    /* position of sky patches of direct daylight coefficients if n.n.*/
extern float direct_calendar[13][25][4]; /* position of sky patches of direct daylight coefficients otherwise*/
extern float horizon[36];
extern float horizon_factor[145];
extern double  sundir[3];
extern float point_coefficients[6];

/* diverse functions */
void assign_values();void get_dc();double  normsc(double altitude, int S_INTER);
int  shadow_testing();
void get_horizon_factors();
int  shadow_testing_new;
void process_dc_shading(int number_direct_coefficients);



/*
 * structure storing coefficients for a specific shading variant and sensor.
 */
struct dc_shading_coeff_data_s {
	int		coefficients;
	float*	dc;
	int*	next;
};

/*
 * structure storing file pointer and daylight coefficients
 */
struct dc_shading_coeff_s {
	FILE*	fp;
	struct dc_shading_coeff_data_s data;
};

/**
 *
 */
int dc_shading_coeff_rewind( int blind );

/**
 *
 */
struct dc_shading_coeff_data_s* dc_shading_coeff_read( int blind );

#endif
