/*  Copyright (c) 2002
 *  National Research Council Canada
 *  Fraunhofer Institute for Solar Energy Systems
 *  written by Christoph Reinhart
 */

int julian_day(int day,int month)
{	if( (day >= 32 ) || (day <= 0 )){ printf("WARNING: day in function julian_day out of range %d",day);exit(0);}
	if( (month >= 13 ) || (day <= 0 )){ printf("WARNING:  monthin function julian_day out of range %d",month);exit(0);}
	
	if ( month  == 1 ){ return(day);}
	if ( month  == 2 ){ return(day+31);}
	if ( month  == 3 ){ return(day+59);}
	if ( month  == 4 ){ return(day+90);}
	if ( month  == 5 ){ return(day+120);} 
	if ( month  == 6 ){ return(day+151);}
	if ( month  == 7 ){ return(day+181);}
	if ( month  == 8 ){ return(day+212);}
	if ( month  == 9 ){ return(day+243);}
	if ( month  == 10 ){ return(day+273);} 
	if ( month  == 11 ){ return(day+304);}
	if ( month  == 12 ){ return(day+334);} 
}
