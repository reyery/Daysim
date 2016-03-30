#ifndef LIGHTSWITCH_H
#define LIGHTSWITCH_H

extern double	a_hunt;
extern double	b_hunt;
extern double	c_hunt;
extern double	m_hunt;

double switch_prob(float ill, float a,float b, float c, float m);
double switch_on_prob_intermediate_prob(float ill);
double switch_off(float time_of_absence, int EffLightSystem);
int Pigg_switch_off(int current_time, int LightSystem, int UserLight,int dep);
void lightswitch_function( int UserLight;
#endif
