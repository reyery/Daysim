#include <string.h>
#include <stdlib.h>

#include <fvect.h>
#include "rterror.h"

#include "parse.h"
#include "fropen.h"
#include "read_in_header.h"


/*
 *
 */
int parse_format( char* data, char* format, void* var ) {
	return sscanf( data, format, var ) != 1;
}

int parse_string( char* key, char* data, void* var ) {
	return parse_format( data, "%s", var );
}

int parse_line( char* key, char* data, void* var ) {
	return parse_format( data, "%[^\n]\n", var );
}

int parse_int( char* key, char* data, void* var ) {
	return parse_format( data, "%d", var );
}

int parse_long( char* key, char* data, void* var ) {
	return parse_format( data, "%ld", var );
}

int parse_float( char* key, char* data, void* var ) {
	return parse_format( data, "%f", var );
}

int parse_double( char* key, char* data, void* var ) {
	return parse_format( data, "%lf", var );
}

int parse_fvect( char* key, char* data, void* var ) {
	RREAL* p= (RREAL*)var;
	return sscanf( data, "%lf %lf %lf", &p[0], &p[1], &p[2] ) != 3;
}

int copy_key( char* key, char* data, void* var ) {
	strcpy( var, key );
	return 0;
}

int parse_cmode( char* key, char* data, void* var ) {
	int ret= 0;
	char mode_str[20];
	enum CalculationMode* cm= (enum CalculationMode*)var;

	*cm= RtraceClassic;

	if( (ret= parse_string( key, data, mode_str )) == 0 ) {
		if( strcmp( mode_str, "photonmap" ) == 0 )
			*cm= RtracePhotonMap;
		else if (strcmp(mode_str, "classic")) {
			sprintf(errmsg, "wrong calculation mode: '%s'. valid options: 'classic', 'photonmap'. using 'classic'-mode.", mode_str);
			error(WARNING, errmsg);
		}
	}

	return ret;
}



struct parse_s parse_params[]= {
	/* project details */
	{ "project_directory", parse_string, project_directory },
	{ "project_name", parse_string, project_name },
	{ "tmp_directory", parse_string, tmp_directory },
	{ "material_directory", parse_string, MaterialDatabaseDirectory },
	{ "bin_directory", parse_string, bin_dir },

	/* building info - partly */
	{ "geometry_file", parse_line, geometry_file },
	{ "material_file", parse_line, material_file },
	{ "viewpoint_file", parse_string, viewpoint_file },
	{ "dgp_out_file", parse_string, dgp_out_file },
	{ "dgp_check_file", parse_string, dgp_check_file },
	{ "dgp_image_x_size", parse_int, &dgp_image_x_size },
	{ "dgp_image_y_size", parse_int, &dgp_image_y_size },
	{ "dgp_profile_cut", parse_int, &dgp_profile_cut },

	{ NULL, NULL, 0 }
};


/* temporary variable storing the result for direct and diffuse options */
RtraceOptions rtrace_options;

struct parse_s rtrace_parse_params[]= {
	{ "aa", parse_double, &rtrace_options.rad.aa },
	{ "ab", parse_int, &rtrace_options.rad.ab },
	{ "ad", parse_int, &rtrace_options.rad.ad },
	{ "as", parse_int, &rtrace_options.rad.as },
	{ "ar", parse_int, &rtrace_options.rad.ar },
	{ "dj", parse_double, &rtrace_options.rad.dj },
	{ "ds", parse_double, &rtrace_options.rad.ds },
	{ "dr", parse_int, &rtrace_options.rad.dr },
	{ "dp", parse_double, &rtrace_options.rad.dp },
	{ "lr", parse_int, &rtrace_options.rad.lr },
	{ "lw", parse_double, &rtrace_options.rad.lw },
	{ "st", parse_double, &rtrace_options.rad.st },
	/*	{ "sj", parse_double, &rtrace_options.rad.sj },*/
	{ "ss", parse_double, &rtrace_options.rad.ss },
	{ "additional_rtrace_parameter", parse_line, &rtrace_options.rad.additional },
	
	{ "pmap_t", parse_int, &rtrace_options.pmap.report },
	{ "pmap_apD", parse_double, &rtrace_options.pmap.predistribution },
	{ "pmap_apo", parse_string, &rtrace_options.pmap.photonPort },
	
	{ "pmap_apg_file", parse_string, &rtrace_options.pmap.globalPmap },
	{ "pmap_apg_nphotons", parse_long, &rtrace_options.pmap.globalPhotons },
	{ "pmap_apg_bwidth", parse_long, &rtrace_options.pmap.globalCollect },
	{ "pmap_apc_file", parse_string, &rtrace_options.pmap.causticPmap },
	{ "pmap_apc_nphotons", parse_long, &rtrace_options.pmap.causticPhotons },
	{ "pmap_apc_bwidth", parse_long, &rtrace_options.pmap.causticCollect },
	
	{ "calculation_mode", parse_cmode, &rtrace_options.calculationMode },

	{ "irradiance_switch", parse_string, &rtrace_options.rad.irradianceSwitch },

	{ NULL, NULL, 0 }
};

/*
 * @returns	0 if key matches an option, 1 otherwise
 */
int parse( char* key, FILE* fp, struct parse_s* pd ) {
	int		ret= 1;
	int		i;
	char	buf[1024];

	for( i= 0; pd[i].key != NULL; i++ ) {
		if( strlen( key ) != strlen( pd[i].key ) )
			continue;

		if( strcmp( key, pd[i].key ) == 0 ) {
			if( pd[i].func != NULL ) {
				getWord( buf, fp, 1024, '\n' );
				if( pd[i].func( key, buf, pd[i].data ) ) {
					sprintf(errmsg, "invalid parameter for keyword '%s': %s", key, buf);
					error(WARNING, errmsg);
					break;
				}
			}
			ret= 0;
			break;
		}
	}
	return ret;
}

