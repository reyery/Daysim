#include <cassert>
#include <memory.h>
#include <float.h>

#include "cSkyVault.h"

//////////////////////////
// Sky Vault
//////////////////////////

cSkyVault::cSkyVault(double latitude, double longitude):
						m_latitude(latitude), m_longitude(longitude), m_SkyCalculated(false)
{
	double defaultvalues[7]={12,12,15,15,20,30,60};
//	double defaultvalues[14]={6,6,6,6,7.5,7.5,7.5,7.5,10,10,15,15,30,30};
	double Az, Alt, DeltaAz, DeltaAlt;

	int i,j,pointer;
	pointer=0;
	double SkyAziIncrement[7];
	for (i=0; i<7; i++)
	{
		for (j=0; j<1;j++)
		{
			SkyAziIncrement[pointer]=defaultvalues[i];
			pointer++;
		}
	}

	int CurrentPatch;

	// initialise array pointer to 0
	m_ptRadiance=0;

	// create sky patches
	m_NumPatches = SKY_PATCHES;

	m_ptPatchAlt=new double[m_NumPatches];
	m_ptPatchAz=new double[m_NumPatches];
	m_ptPatchDeltaAlt=new double[m_NumPatches];
	m_ptPatchDeltaAz=new double[m_NumPatches];
	m_ptPatchLuminance=new double[m_NumPatches];
	m_ptPatchSolidAngle=new double[m_NumPatches];

	m_ptRadiance = new float[HOURS_PER_YEAR][SKY_PATCHES];
	// CMR: I think that the program relies on this memory being zero.
	memset(m_ptRadiance, 0, sizeof(float[HOURS_PER_YEAR][SKY_PATCHES]));

	CurrentPatch=0;

	DeltaAlt=12;
	pointer=0;

	for (Alt=DeltaAlt/2; Alt<=84; Alt+=DeltaAlt)
	{
		DeltaAz=SkyAziIncrement[pointer];
		for (Az=0; Az <= 360-DeltaAz; Az+=DeltaAz)
		{
			// set each patch's position and size
			m_ptPatchAlt[CurrentPatch] = radians(Alt);
			m_ptPatchAz[CurrentPatch] = radians(Az);
			m_ptPatchDeltaAlt[CurrentPatch] = radians(DeltaAlt);
			m_ptPatchDeltaAz[CurrentPatch] = radians(DeltaAz);
			m_ptPatchSolidAngle[CurrentPatch]=2*M_PI*(sin(m_ptPatchAlt[CurrentPatch]+m_ptPatchDeltaAlt[CurrentPatch]/2)-sin(m_ptPatchAlt[CurrentPatch]-m_ptPatchDeltaAlt[CurrentPatch]/2))/(2*M_PI/m_ptPatchDeltaAz[CurrentPatch]);
			CurrentPatch++;
		}
		pointer++;
	}

	// zenith patch (last patch is always top patch)
	m_ptPatchAlt[CurrentPatch]=M_PI_2;
	m_ptPatchAz[CurrentPatch]=0;
	m_ptPatchDeltaAlt[CurrentPatch] = radians(6);
	m_ptPatchDeltaAz[CurrentPatch]=2*M_PI;
	m_ptPatchSolidAngle[CurrentPatch]=2*M_PI*(sin(m_ptPatchAlt[CurrentPatch])-sin(m_ptPatchAlt[CurrentPatch]-m_ptPatchDeltaAlt[CurrentPatch]))/(2*M_PI/m_ptPatchDeltaAz[CurrentPatch]);

	return;
}

cSkyVault::~cSkyVault(void)
{
	delete[] m_ptPatchAlt;
	delete[] m_ptPatchAz;
	delete[] m_ptPatchDeltaAlt;
	delete[] m_ptPatchDeltaAz;
	delete[] m_ptPatchLuminance;
	delete[] m_ptPatchSolidAngle;
	delete[] m_ptRadiance;
}

void cSkyVault::GetPatchDetails(double (*ptPatchDat)[5])
{
	int i;

	for (i=0; i<m_NumPatches; i++)
	{
		ptPatchDat[i][0] = m_ptPatchAlt[i];
		ptPatchDat[i][1] = m_ptPatchAz[i];
		ptPatchDat[i][2] = m_ptPatchDeltaAlt[i];
		ptPatchDat[i][3] = m_ptPatchDeltaAz[i];
		ptPatchDat[i][4] = m_ptPatchSolidAngle[i];
	}

	return;
}

bool cSkyVault::LoadClimateFile(char *filename, cClimateFile::eClimateFileFormat ClimateFileFormat,double StartTime, double EndTime, int StartDay, int EndDay, int StartMonth, int EndMonth)
{
	return m_ClimateFile.ReadClimateFile(filename,0, ClimateFileFormat, StartTime, EndTime, StartDay, EndDay, StartMonth, EndMonth);
}

bool cSkyVault::SetLatitude(double latitude)
{
	if (latitude <= M_PI_2 && latitude >= -M_PI_2)
	{
		m_latitude=latitude;
		m_Sun.SetLatitude(m_latitude);
		return true;
	}

	return false;
}

bool cSkyVault::SetLongitude(double longitude)
{
	if (longitude <= M_PI && longitude >= -M_PI)
	{
		m_longitude=longitude;
		return true;
	}

	return false;
}

bool cSkyVault::SetMeridian(double meridian)
{

		m_meridian=meridian;
		m_Sun.SetMeridian(m_meridian);
		return true;
}

void cSkyVault::CalculateSky(cSkyVault::eSunType Suns, bool DoDiffuse, bool DoIlluminance, double hourshift)
{
	// calculates sky patch radiance distribution for whole year (0:30 1st Jan -> 23:30 31st Dec)
	int i, j,SunPatch;

	double hour;
	int day, index;

	double CosMinSunDist, CosSunDist;
	double SunAlt, SunAz;

	double *ptLv, EIllum;
	double PatchAltitude, PatchAzimuth, Idh, Ibh, Ibn;

	double sunset, sunrise, hourangle;
	int AltIndex, AzIndex;

	const double BINSIZE=5;

	double x,y,z,SolarRadiance;
	double halfsolarangle=0.02665; //degrees
//	double halfsolarangle=0.2665; //degrees
//	double halfsolarangle=1.2665; //degrees
double temp=0;

	// 5 degree bins of sun position
	double SunRadiance[18][72];
	// various arrays to keep track of the actual positions of the suns that were binned into a particular sun
	double AltxIbh[18][72];
	double AzxIbh[18][72];
	double NxIbh[18][72];
	int SunUpHourCount=0;
	double NormFac;

	for (i=0; i<18; i++)
	{
		for (j=0; j<72; j++)
		{
			SunRadiance[i][j]=0;
			AltxIbh[i][j]=0;
			AzxIbh[i][j]=0;
			NxIbh[i][j]=0;
		}
	}
	char *SunFileName="SunFile.rad";
	FILE *SunFile = NULL;
	if (Suns==MANY_SUNS)
	{
		// open file for sun data (checking that you can...)
		if ((SunFile=fopen(SunFileName,"w"))==NULL)
		{
			fprintf(stderr,"Error opening: %s\n",SunFileName);
			return;
		}
	}

	
	for (day=1; day<=365; day++)
	{

		// setup sun position
		m_Sun.SetDay(day);

		sunrise=m_Sun.GetSunrise();
		sunset=2*M_PI - sunrise;

		
		
		for (hour=.5; hour<24; hour++)
		{
			EIllum=0;
			CosMinSunDist = -DBL_MAX;

			index=(day-1)*24+(int)hour;
			ptLv=new double[m_NumPatches];

			// !!!!!!!!!!!!!!!!!!!!!!
			hourangle=(hour+hourshift+m_Sun.TimeDiff(m_longitude,m_meridian))*M_PI/12;
			// TODO: UNCOMMENT THESE LINES
			// if this is the first/last sun-up hour of the day, use the average position for while it is up
			if (fabs(hourangle-sunrise)<M_PI/24)
				hourangle=(hourangle+M_PI/24+sunrise)/2;
			else if (fabs(hourangle-sunset)<M_PI/24)
				hourangle=(hourangle-M_PI/24+sunset)/2;

			m_Sun.SetHourAngle(hourangle);
			m_Sun.GetPosition(SunAlt,SunAz);

			// get diffuse horizontal irradiation from climate file
			Idh=m_ClimateFile.GetDiffuseRad(hour,day);
			assert(!isnan(Idh));
			Ibh=m_ClimateFile.GetDirectRad(hour,day);
			
			//Tito
			//fprintf(stderr,"Report: %d %.1f dir %.0f dif %.0f, long %.0f mer %.0f",day,hour,Ibh,Idh,degrees(m_longitude),degrees(m_meridian));
			//fprintf(stderr, "sun position %.2f %.2f hour ang %.2f time Diff  %f\n",degrees(SunAlt),degrees(SunAz),12/M_PI*hourangle,12/M_PI*m_Sun.TimeDiff(m_longitude,m_meridian));

			if (Idh < 0) Idh=0;
			if (Ibh < 0) Ibh=0;

			// setup sky model
			if (!m_SkyModel.SetSkyConditions(Idh,Ibh,&m_Sun))
				goto NextHour;

            CosSunDist=-1;
			for (i=0; i<m_NumPatches; i++)
			{
				// first calculate relative luminance and scaling factor
				PatchAltitude=m_ptPatchAlt[i];
				PatchAzimuth=m_ptPatchAz[i];

				ptLv[i]=m_SkyModel.GetRelativeLuminance(PatchAltitude,PatchAzimuth);
				EIllum+= ptLv[i]*m_ptPatchSolidAngle[i]*sin(PatchAltitude);

				// work out distance of sun from this patch
				CosSunDist = cos(SunAlt)*cos(fabs(SunAz-PatchAzimuth))*cos(PatchAltitude) + sin(SunAlt)*sin(PatchAltitude);

				if (CosSunDist > CosMinSunDist)
				{
					CosMinSunDist=CosSunDist;
					SunPatch=i;
				}

			}
			if (SunAlt> 0)
	            Ibn=Ibh/sin(SunAlt);
			else
				Ibn=0;

			if (Ibn > SOLAR_CONSTANT_E)
			{
				// Very large value for direct radiation - probably low solar altitude
				Idh=Idh+Ibh;
				Ibn=0;
			}

			if (DoIlluminance)
				NormFac=Idh*m_SkyModel.GetDiffuseLumEffy(SunAlt,0.0);
			else
				NormFac=Idh;

			if (EIllum>0)
				SunUpHourCount++;

			for (i=0; i<m_NumPatches; i++)
			{
				if ((EIllum > 0) && DoDiffuse)
				{
					m_ptRadiance[index][i] = (float)(ptLv[i] * NormFac / EIllum);
				}
				else
					m_ptRadiance[index][i]=0;

			}

			if (DoIlluminance)
				NormFac=Ibn*m_SkyModel.GetBeamLumEffy(SunAlt,0.0);
			else
				NormFac=Ibn;

			// now do sun
            if (Suns==CUMULATIVE_SUN)
			{
				// add on direct radiation to patch with sun in
				if (SunAlt > 0 && Ibn > 0)
					m_ptRadiance[index][SunPatch] += (float)(NormFac / (m_ptPatchSolidAngle[SunPatch]));
			}
			else if (Suns==MANY_SUNS && Ibn>0 && m_SkyModel.GetSkyClearness() > 1 )
			{
				// Work out solar radiance
				SolarRadiance = NormFac / (2.0*M_PI*(1 - cos(radians(halfsolarangle))));

				AltIndex = (int)(SunAlt / radians(BINSIZE));
				AzIndex = (int)(SunAz / radians(BINSIZE));

				if (DoIlluminance)
					NormFac=Ibh*m_SkyModel.GetBeamLumEffy(SunAlt,0.0);
				else
					NormFac=Ibh;

				AltxIbh[AltIndex][AzIndex]+=SunAlt*NormFac;
				AzxIbh[AltIndex][AzIndex]+=SunAz*NormFac;
				NxIbh[AltIndex][AzIndex]+=NormFac;
			}
			delete ptLv;
NextHour:
			continue;
		}
	}
	fprintf(stderr,"There were %d sun up hours in this climate file\n",SunUpHourCount);

	// work out cumulative sky
	m_CumSky = new double[m_NumPatches];

	// if we are doing the annual irradiance output the total, for illuminance output the mean
	if (!DoIlluminance)
		SunUpHourCount=1;

	for (i=0; i<m_NumPatches; i++)
	{
		m_CumSky[i]=0;
		for (j = 0; j<HOURS_PER_YEAR; j++)
		{
			m_CumSky[i]+=m_ptRadiance[j][i]/SunUpHourCount;
		}
	}
	// output the suns
	if (Suns == MANY_SUNS)
	{
		//////////////////////
		temp = 0;
		for (day = 1; day <= 365; day++)
			//	for (day=91; day<=273; day++)
		{
			sunrise = m_Sun.GetSunrise();
			sunset = 2 * M_PI - sunrise;

			//Tito for (hour=0; hour<24; hour+=0.25)
			for (hour = 0.5; hour < 24; hour++)

				//		for (hour=12.5; hour<=16.5; hour++)
			{
				CosMinSunDist = -DBL_MAX;

				// setup sun position
				m_Sun.SetDay(day);
				hourangle = (hour + m_Sun.TimeDiff(m_longitude, m_meridian))*M_PI / 12;



				m_Sun.SetHourAngle(hourangle);
				m_Sun.GetPosition(SunAlt, SunAz);

				// we impose a minimum on the solar altitude to avoid extremely bright low altitude suns
				if (SunAlt < radians(3) && SunAlt>0) // 6 degrees is minimum alt for circumsolar region in perez mod.
				{
					SunAlt = radians(3);
				}

				// Tito: commented statement out
				//if (SunAlt < radians(49))
				//	continue;

				// get diffuse horizontal irradiation from climate file
				Idh = m_ClimateFile.GetDiffuseRad(hour, day);
				Ibh = m_ClimateFile.GetDirectRad(hour, day);

				if (Idh < 0) Idh = 0;
				if (Ibh < 0) Ibh = 0;

				//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
				//Ibh=0.25;

				if (SunAlt> 0)
					Ibn = Ibh / sin(SunAlt);
				else
					Ibn = 0;

				if (Ibn > SOLAR_CONSTANT_E)
				{
					// Very large value for direct radiation - probably low solar altitude
					Ibn = 0;
				}

				CosSunDist = -1;
				for (i = 0; i < m_NumPatches; i++)
				{
					// first calculate relative luminance and scaling factor
					PatchAltitude = m_ptPatchAlt[i];
					PatchAzimuth = m_ptPatchAz[i];

					// work out distance of sun from this patch
					CosSunDist = cos(SunAlt)*cos(fabs(SunAz - PatchAzimuth))*cos(PatchAltitude) + sin(SunAlt)*sin(PatchAltitude);

					if (CosSunDist > CosMinSunDist)
					{
						CosMinSunDist = CosSunDist;
						SunPatch = i;
					}

				}
				if (SunAlt >= 0)
				{
					SolarRadiance = Ibn / (2.0*M_PI*(1 - cos(radians(halfsolarangle)))); // !!!!!!!!!!!!!!!!+m_CumSky[SunPatch];

					temp += SolarRadiance*(2.0*M_PI*(1 - cos(radians(halfsolarangle))));

					// work out sun direction in x,y,z form
					x = sin(SunAz)*cos(SunAlt);
					y = cos(SunAz)*cos(SunAlt);
					z = sin(SunAlt);

					fprintf(SunFile, "\nvoid light solar\n");
					fprintf(SunFile, "0\n0\n");
					fprintf(SunFile, "3 %.3e %.3e %.3e", SolarRadiance, SolarRadiance, SolarRadiance);

					fprintf(SunFile, "\nsolar source sun\n");
					fprintf(SunFile, "0\n0\n");
					fprintf(SunFile, "4 %f %f %f %f\n", x, y, z, 2 * halfsolarangle);

				}
			}
		}
	}


	fprintf(stderr,"Total Ibh/Lbh: %f\n",temp);
	m_SkyCalculated=true;
	return;
}

double* cSkyVault::GetCumulativeSky()
{
	return m_CumSky;
}

