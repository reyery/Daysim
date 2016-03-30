#ifndef DAYSIM_PARSE_H
#define DAYSIM_PARSE_H


#include <stdio.h>

#ifndef PATH_SIZE
#define PATH_SIZE	1024
#endif

struct parse_s {
	char*	key;
	int		(*func)( char*, char*, void* );
	void*	data;
};


extern struct genvf_s genvf;
extern struct parse_s parse_params[];
extern struct parse_s rtrace_parse_params[];
/**
 * Radiance(partially also photon map) options
 */
typedef struct
{
	char   irradianceSwitch[3]; /* i, I-, I, I+ */

	double aa;
	int    ab;
	int    ad;
	int    ar;
	int    as;

	double dj;
	double dp;
	int    dr;
	double ds;
	double dt;

	double sj; /* deprecated */
	double ss;
	double st;

	int    lr;
	double lw;

	char   additional[1024]; /* for additional options */
} RadianceOptions;


/**
 * Photon map options.
 */
typedef struct
{
	/* mkpmap-option -t */
	int    report;

	/* mkpmap-option -apD */
	double predistribution;
	/* mkpmap-option -apo */
	char   photonPort[256];

	/* mkpmap-option -apg */
	char   globalPmap[256];
	/* mkpmap-option -apg */
	long   globalPhotons;
	/* rtrace-option -apg */
	int    globalCollect;

	/* mkpmap-option -apc */
	char   causticPmap[256];
	/* mkpmap-option -apc */
	long   causticPhotons;
	/* rtrace-option -apc */
	int    causticCollect;
} PhotonMapOptions;

/* potential illumination modes for daylight coefficient calculations */
enum Illumination { DirectIllumination= 1, DiffuseIllumination= 2, DirectIllumination_old= 3};

/* method to use for daylight coefficient computation */
enum CalculationMode { RtraceClassic= 1, RtracePhotonMap= 2 };

/**
 * Options used for direct or diffuse calculation.
 */
typedef struct
{
	enum Illumination illumination;       /* direct or diffuse illumination */
	enum CalculationMode calculationMode; /* classic or photon map */
	RadianceOptions  rad;
	PhotonMapOptions pmap;
} RtraceOptions;

extern RtraceOptions rtrace_options;
/* rtrace simulation options for direct and diffuse illumination calculation */
extern RtraceOptions directOptions;
extern RtraceOptions diffuseOptions;


extern int parse_format( char* data, char* format, void* var );
extern int parse_string( char* key, char* data, void* var );
extern int parse_line( char* key, char* data, void* var );
extern int parse_int( char* key, char* data, void* var );
extern int parse_long( char* key, char* data, void* var );
extern int parse_float( char* key, char* data, void* var );
extern int parse_double( char* key, char* data, void* var );
extern int parse_fvect( char* key, char* data, void* var );
extern int copy_key( char* key, char* data, void* var );
extern int parse_cmode( char* key, char* data, void* var );

/**
 * @returns	0 if key matches an option, 1 otherwise
 */
extern int parse( char* key, FILE* fp, struct parse_s* pd );


#endif
