/* Copyright (c) 1992 Regents of the University of California */

#ifndef lint
static char SCCSid[] = "$SunId$ LBL";
#endif

/*
 *  gensky.c - program to generate sky functions.
 *		Our zenith is along the Z-axis, the X-axis
 *		points east, and the Y-axis points north.
 *		Radiance is in watts/steradian/sq. meter.
 *
 *     3/26/86
 */

#include  <stdio.h>

#include  <math.h>

#include  "color.h"

extern char  *strcpy(), *strcat(), *malloc();
extern double  stadj(), sdec(), sazi(), salt();

#define  PI		3.141592654

#define  DOT(v1,v2)	(v1[0]*v2[0]+v1[1]*v2[1]+v1[2]*v2[2])

double  normsc();
					/* sun calculation constants */
extern double  s_latitude;
extern double  s_longitude;
extern double  s_meridian;
					/* required values */
int  month, day;				/* date */
double  hour;					/* time */
int  tsolar;					/* 0=standard, 1=solar */
double  altitude, azimuth;			/* or solar angles */
					/* default values */
int  cloudy = 0;				/* 1=standard, 2=uniform */
int  dosun = 1;
double  zenithbr = 0.0;
int	u_zenith = 0;				/* -1=irradiance, 1=radiance */
double  turbidity = 2.75;
double  gprefl = 0.2;
					/* computed values */
double  sundir[3];
double  groundbr;
double  F2;
double  solarbr = 0.0;
int	u_solar = 0;				/* -1=irradiance, 1=radiance */

char  *progname;
char  errmsg[128];


main(argc, argv)
int  argc;
char  *argv[];
{
	int  i;

	progname = argv[0];
	if (argc == 2 && !strcmp(argv[1], "-defaults")) {
		printdefaults();
		exit(0);
	}
	if (argc < 4)
		userror("arg count");
	if (!strcmp(argv[1], "-ang")) {
		altitude = atof(argv[2]) * (PI/180);
		azimuth = atof(argv[3]) * (PI/180);
		month = 0;
	} else {
		month = atoi(argv[1]);
		if (month < 1 || month > 12)
			userror("bad month");
		day = atoi(argv[2]);
		if (day < 1 || day > 31)
			userror("bad day");
		hour = atof(argv[3]);
		if (hour < 0 || hour >= 24)
			userror("bad hour");
		tsolar = argv[3][0] == '+';
	}
	for (i = 4; i < argc; i++)
		if (argv[i][0] == '-' || argv[i][0] == '+')
			switch (argv[i][1]) {
			case 's':
				cloudy = 0;
				dosun = argv[i][0] == '+';
				break;
			case 'r':
			case 'R':
				u_solar = argv[i][1]=='R' ? -1 : 1;
				solarbr = atof(argv[++i]);
				break;
			case 'c':
				cloudy = argv[i][0] == '+' ? 2 : 1;
				dosun = 0;
				break;
			case 't':
				turbidity = atof(argv[++i]);
				break;
			case 'b':
			case 'B':
				u_zenith = argv[i][1]=='B' ? -1 : 1;
				zenithbr = atof(argv[++i]);
				break;
			case 'g':
				gprefl = atof(argv[++i]);
				break;
			case 'a':
				s_latitude = atof(argv[++i]) * (PI/180);
				break;
			case 'o':
				s_longitude = atof(argv[++i]) * (PI/180);
				break;
			case 'm':
				s_meridian = atof(argv[++i]) * (PI/180);
				break;
			default:
				sprintf(errmsg, "unknown option: %s", argv[i]);
				userror(errmsg);
			}
		else
			userror("bad option");

	if (fabs(s_meridian-s_longitude) > 30*PI/180)
		fprintf(stderr,
	"%s: warning: %.1f hours btwn. standard meridian and longitude\n",
			progname, (s_longitude-s_meridian)*12/PI);

	printhead(argc, argv);

	computesky();
	printsky();
}


computesky()			/* compute sky parameters */
{
	double	normfactor;
					/* compute solar direction */
	if (month) {			/* from date and time */
		int  jd;
		double  sd, st;

		jd = jdate(month, day);		/* Julian date */
		sd = sdec(jd);			/* solar declination */
		if (tsolar)			/* solar time */
			st = hour;
		else
			st = hour + stadj(jd);
		altitude = salt(sd, st);
		azimuth = sazi(sd, st);
		printf("# Solar altitude and azimuth: %f %f\n",
				180./PI*altitude, 180./PI*azimuth);
	}
	if (!cloudy && altitude > 87.*PI/180.) {
		fprintf(stderr,
"%s: warning - sun too close to zenith, reducing altitude to 87 degrees\n",
				progname);
		printf(
"# warning - sun too close to zenith, reducing altitude to 87 degrees\n");
		altitude = 87.*PI/180.;
	}
	sundir[0] = -sin(azimuth)*cos(altitude);
	sundir[1] = -cos(azimuth)*cos(altitude);
	sundir[2] = sin(altitude);

					/* Compute normalization factor */
	if (cloudy == 2)
		normfactor = 1.0;
	else if (cloudy == 1)
		normfactor = 0.777778;
	else {
		F2 = 0.274*(0.91 + 10.0*exp(-3.0*(PI/2.0-altitude)) +
				0.45*sundir[2]*sundir[2]);
		normfactor = normsc(altitude)/F2/PI;
	}
					/* Compute zenith brightness */
	if (u_zenith == -1)
		zenithbr /= normfactor*PI;
	else if (u_zenith == 0) {
		if (cloudy)
			zenithbr = 8.6*sundir[2] + .123;
		else
			zenithbr = (1.376*turbidity-1.81)*tan(altitude)+0.38;
		if (zenithbr < 0.0)
			zenithbr = 0.0;
		else
			zenithbr *= 1000.0/SKYEFFICACY;
	}
					/* Compute horizontal radiance */
	groundbr = zenithbr*normfactor;
	printf("# Ground ambient level: %f\n", groundbr);
	if (sundir[2] > 0.0 && (!u_solar || solarbr > 0.0)) {
		if (u_solar == -1)
			solarbr /= 6e-5*sundir[2];
		else if (u_solar == 0)
			solarbr = 1.5e9/SUNEFFICACY *
			(1.147 - .147/(sundir[2]>.16?sundir[2]:.16));
		groundbr += 6e-5/PI*solarbr*sundir[2];
	} else
		dosun = 0;
	groundbr *= gprefl;
}


printsky()			/* print out sky */
{
	if (dosun) {
		printf("\nvoid light solar\n");
		printf("0\n0\n");
		printf("3 %.2e %.2e %.2e\n", solarbr, solarbr, solarbr);
		printf("\nsolar source sun\n");
		printf("0\n0\n");
		printf("4 %f %f %f 0.5\n", sundir[0], sundir[1], sundir[2]);
	}
	
	printf("\nvoid brightfunc skyfunc\n");
	printf("2 skybright skybright.cal\n");
	printf("0\n");
	if (cloudy)
		printf("3 %d %.2e %.2e\n", cloudy, zenithbr, groundbr);
	else
		printf("7 -1 %.2e %.2e %.2e %f %f %f\n", zenithbr, groundbr,
				F2, sundir[0], sundir[1], sundir[2]);
}


printdefaults()			/* print default values */
{
	if (cloudy == 1)
		printf("-c\t\t\t\t# Cloudy sky\n");
	else if (cloudy == 2)
		printf("+c\t\t\t\t# Uniform cloudy sky\n");
	else if (dosun)
		printf("+s\t\t\t\t# Sunny sky with sun\n");
	else
		printf("-s\t\t\t\t# Sunny sky without sun\n");
	printf("-g %f\t\t\t# Ground plane reflectance\n", gprefl);
	if (zenithbr > 0.0)
		printf("-b %f\t\t\t# Zenith radiance (watts/ster/m2\n", zenithbr);
	else
		printf("-t %f\t\t\t# Atmospheric turbidity\n", turbidity);
	printf("-a %f\t\t\t# Site latitude (degrees)\n", s_latitude*(180/PI));
	printf("-o %f\t\t\t# Site longitude (degrees)\n", s_longitude*(180/PI));
	printf("-m %f\t\t\t# Standard meridian (degrees)\n", s_meridian*(180/PI));
}


userror(msg)			/* print usage error and quit */
char  *msg;
{
	if (msg != NULL)
		fprintf(stderr, "%s: Use error - %s\n", progname, msg);
	fprintf(stderr, "Usage: %s month day hour [options]\n", progname);
	fprintf(stderr, "   Or: %s -ang altitude azimuth [options]\n", progname);
	fprintf(stderr, "   Or: %s -defaults\n", progname);
	exit(1);
}


double
normsc(theta)			/* compute normalization factor (E0*F2/L0) */
double  theta;
{
	static double  nf[5] = {2.766521, 0.547665,
				-0.369832, 0.009237, 0.059229};
	double  x, nsc;
	register int  i;
					/* polynomial approximation */
	x = (theta - PI/4.0)/(PI/4.0);
	nsc = nf[4];
	for (i = 3; i >= 0; i--)
		nsc = nsc*x + nf[i];

	return(nsc);
}


printhead(ac, av)		/* print command header */
register int  ac;
register char  **av;
{
	putchar('#');
	while (ac--) {
		putchar(' ');
		fputs(*av++, stdout);
	}
	putchar('\n');
}
