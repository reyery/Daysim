#include  <stdio.h>
#include <stdlib.h>
#include  <math.h>
#include  <rterror.h>

#include "numerical.h"
#include "nrutil.h"

#define ITMAX 5000
#define EPS 1.0e-4
#define M1 259200
#define IA1 7141
#define IC1 54773
#define RM1 (1.0/M1)
#define M2 134456
#define IA2 8121
#define IC2 28411
#define RM2 (1.0/M2)
#define M3 243000
#define IA3 4561
#define IC3 51349

float gammln(float xx)

{
	double x,tmp,ser;
	static double cof[6]={76.18009173,-86.50532033,24.01409822,
		-1.231739516,0.120858003e-2,-0.536382e-5};
	int j;

	x=xx-1.0;
	tmp=x+5.5;
	tmp -= (x+0.5)*log(tmp);
	ser=1.0;
	for (j=0;j<=5;j++) {
		x += 1.0;
		ser += cof[j]/x;
	}
	return -tmp+log(2.50662827465*ser);
}
		  
float betai(float a,float b,float x)
{
	float bt, help;
	
	if (x < 0.0 || x > 1.0)    error(INTERNAL, "Bad x in routine BETAI");  
	if (x == 0.0 || x == 1.0) bt=0.0;
	else
		bt=exp(gammln(a+b)-gammln(a)-gammln(b)+a*log(x)+b*log(1.0-x));
	if (x < (a+1.0)/(a+b+2.0))
	{
	  help=betacf(a,b,x);
	  if ( help > -98 )  return bt*help/a;
	  else  return -99;
	}  
	else
	{
	  help=betacf(b,a,1.0-x);
	  if ( help > -98 )  return 1.0-bt*help/b;
	  else  return -99;
	}
}


float betacf(float a,float b,float x)
{
	float qap,qam,qab,em,tem,d;
	float bz,bm=1.0,bp,bpp;
	float az=1.0,am=1.0,ap,app,aold;
	int m;
	
	qab=a+b;
	qap=a+1.0;
	qam=a-1.0;
	bz=1.0-qab*x/qap;
	for (m=1;m<=ITMAX;m++) {
		em=(float) m;
		tem=em+em;
		d=em*(b-em)*x/((qam+tem)*(a+tem));
		ap=az+d*am;
		bp=bz+d*bm;
		d = -(a+em)*(qab+em)*x/((qap+tem)*(a+tem));
		app=ap+d*az;
		bpp=bp+d*bz;
		aold=az;
		am=ap/bpp;
		bm=bp/bpp;
		az=app/bpp;
		bz=1.0;
		if (fabs(az-aold) < (EPS*fabs(az))) return az;
	}
	fprintf(stderr,"warning: a or b too big, or ITMAX too small in BETACF\n");
	return -99;
}


float gasdev ( long *idum )
{
	static int iset=0;
	static float gset;
	float fac,r,v1,v2;
	
	if  (iset == 0) {
		do {
			v1=2.0*ran1(idum)-1.0;
			v2=2.0*ran1(idum)-1.0;
			r=v1*v1+v2*v2;
		} while (r >= 1.0);
		fac=sqrt(-2.0*log(r)/r);
		gset=v1*fac;
		iset=1;
		return v2*fac;
	} else {
		iset=0;
		return gset;
	}
}


float ran1 ( long *idum )
{
	static long ix1,ix2,ix3;
	static float r[98];
	float temp;
	static int iff=0;
	int j;
	
	if (*idum < 0 || iff == 0) {
		iff=1;
		ix1=(IC1-(*idum)) % M1;
		ix1=(IA1*ix1+IC1) % M1;
		ix2=ix1 % M2;
		ix1=(IA1*ix1+IC1) % M1;
		ix3=ix1 % M3;
		for (j=1;j<=97;j++) {
			ix1=(IA1*ix1+IC1) % M1;
			ix2=(IA2*ix2+IC2) % M2;
			r[j] = (float)((ix1 + ix2*RM2)*RM1);
		}
		*idum=1;
	}
	ix1=(IA1*ix1+IC1) % M1;
	ix2=(IA2*ix2+IC2) % M2;
	ix3=(IA3*ix3+IC3) % M3;
	j=1 + ((97*ix3)/M3);
	if (j > 97 || j < 1) error(INTERNAL, "RAN1: This cannot happen");
	temp=r[j];
	r[j] = (float)((ix1 + ix2*RM2)*RM1);
	return temp;
}

void indexx ( unsigned long n, float *arrin, int *indx )
{
	int l,j,ir,indxt,i;
	float q;

	for (j=1;j<=n;j++) indx[j]=j;
	l=(n >> 1) + 1;
	ir=n;
	for (;;) {
		if (l > 1)
			q=arrin[(indxt=indx[--l])];
		else {
			q=arrin[(indxt=indx[ir])];
			indx[ir]=indx[1];
			if (--ir == 1) {
				indx[1]=indxt;
				return;
			}
		}
		i=l;
		j=l << 1;
		while (j <= ir) {
			if (j < ir && arrin[indx[j]] < arrin[indx[j+1]]) j++;
			if (q < arrin[indx[j]]) {
				indx[i]=indx[j];
				j += (i=j);
			}
			else j=ir+1;
		}
		indx[i]=indxt;
	}
}

void rank ( unsigned long n,int *indx, int *irank )
{
	int j;

	for (j=1;j<=n;j++) irank[indx[j]]=j;
}

#define NRANSI
#define SWAP(a,b) temp=(a);(a)=(b);(b)=temp;
#define M 7
#define NSTACK 50

void sort(unsigned long n, float arr[])
{
	unsigned long i,ir=n,j,k,l=1,*istack;
	int jstack=0;
	float a,temp;

	istack=lvector(1,NSTACK);
	for (;;) {
		if (ir-l < M) {
			for (j=l+1;j<=ir;j++) {
				a=arr[j];
				for (i=j-1;i>=l;i--) {
					if (arr[i] <= a) break;
					arr[i+1]=arr[i];
				}
				arr[i+1]=a;
			}
			if (jstack == 0) break;
			ir=istack[jstack--];
			l=istack[jstack--];
		} else {
			k=(l+ir) >> 1;
			SWAP(arr[k],arr[l+1])
			if (arr[l] > arr[ir]) {
				SWAP(arr[l],arr[ir])
			}
			if (arr[l+1] > arr[ir]) {
				SWAP(arr[l+1],arr[ir])
			}
			if (arr[l] > arr[l+1]) {
				SWAP(arr[l],arr[l+1])
			}
			i=l+1;
			j=ir;
			a=arr[l+1];
			for (;;) {
				do i++; while (arr[i] < a);
				do j--; while (arr[j] > a);
				if (j < i) break;
				SWAP(arr[i],arr[j]);
			}
			arr[l+1]=arr[j];
			arr[j]=a;
			jstack += 2;
			if (jstack > NSTACK) error(INTERNAL, "NSTACK too small in sort");
			if (ir-i+1 >= j-l) {
				istack[jstack]=ir;
				istack[jstack-1]=i;
				ir=j-1;
			} else {
				istack[jstack]=j-1;
				istack[jstack-1]=l;
				l=i;
			}
		}
	}
	free_lvector(istack,1,NSTACK);
}
#undef M
#undef NSTACK
#undef SWAP
#undef NRANSI

#define SWAP(a,b) tempr=(a);(a)=(b);(b)=tempr

void four1(float data[], unsigned long nn, int isign)
{
	unsigned long n,mmax,m,j,istep,i;
	double wtemp,wr,wpr,wpi,wi,theta;
	float tempr,tempi;

	n=nn << 1;
	j=1;
	for (i=1;i<n;i+=2) {
		if (j > i) {
			SWAP(data[j],data[i]);
			SWAP(data[j+1],data[i+1]);
		}
		m=n >> 1;
		while (m >= 2 && j > m) {
			j -= m;
			m >>= 1;
		}
		j += m;
	}
	mmax=2;
	while (n > mmax) {
		istep=mmax << 1;
		theta=isign*(6.28318530717959/mmax);
		wtemp=sin(0.5*theta);
		wpr = -2.0*wtemp*wtemp;
		wpi=sin(theta);
		wr=1.0;
		wi=0.0;
		for (m=1;m<mmax;m+=2) {
			for (i=m;i<=n;i+=istep) {
				j=i+mmax;
				tempr=wr*data[j]-wi*data[j+1];
				tempi=wr*data[j+1]+wi*data[j];
				data[j]=data[i]-tempr;
				data[j+1]=data[i+1]-tempi;
				data[i] += tempr;
				data[i+1] += tempi;
			}
			wr=(wtemp=wr)*wpr-wi*wpi+wr;
			wi=wi*wpr+wtemp*wpi+wi;
		}
		mmax=istep;
	}
}
#undef SWAP


void realft(float data[], unsigned long n, int isign)
{
	void four1(float data[], unsigned long nn, int isign);
	unsigned long i,i1,i2,i3,i4,np3;
	float c1=0.5,c2,h1r,h1i,h2r,h2i;
	double wr,wi,wpr,wpi,wtemp,theta;

	theta=3.141592653589793/(double) (n>>1);
	if (isign == 1) {
		c2 = -0.5;
		four1(data,n>>1,1);
	} else {
		c2=0.5;
		theta = -theta;
	}
	wtemp=sin(0.5*theta);
	wpr = -2.0*wtemp*wtemp;
	wpi=sin(theta);
	wr=1.0+wpr;
	wi=wpi;
	np3=n+3;
	for (i=2;i<=(n>>2);i++) {
		i4=1+(i3=np3-(i2=1+(i1=i+i-1)));
		h1r=c1*(data[i1]+data[i3]);
		h1i=c1*(data[i2]-data[i4]);
		h2r = -c2*(data[i2]+data[i4]);
		h2i=c2*(data[i1]-data[i3]);
		data[i1]=h1r+wr*h2r-wi*h2i;
		data[i2]=h1i+wr*h2i+wi*h2r;
		data[i3]=h1r-wr*h2r+wi*h2i;
		data[i4] = -h1i+wr*h2i+wi*h2r;
		wr=(wtemp=wr)*wpr-wi*wpi+wr;
		wi=wi*wpr+wtemp*wpi+wi;
	}
	if (isign == 1) {
		data[1] = (h1r=data[1])+data[2];
		data[2] = h1r-data[2];
	} else {
		data[1]=c1*((h1r=data[1])+data[2]);
		data[2]=c1*(h1r-data[2]);
		four1(data,n>>1,-1);
	}
}

double mean(int n, double *array)
{ 
  int i;
  float sum = 0;
  
  if ( n <= 0 ) error(USER, "bad n in function MEAN");

  for ( i=1 ; i<=n ; i++ )  sum += array[i-1];
  return  sum / n;
}  

void mean_var_99 ( float *data, int n, float *mean, float *var)     /*  data in data[0],...,data[n-1], ignore data[j] = -99.0  */
{
   int j,counter=0;
   float sum, dev;
  
   if (n <= 1) error(USER, "n must be at least 2 in mean_sdev");

   sum = 0.0;
   for (j=0;j<n;j++)   
    if ( data[j]>-98 ) 
    {
      sum+=data[j];
      counter++;
    }  
   if ( counter > 0 )  *mean=sum/counter;
   else *mean=0;
   
   sum = 0.0;
   for (j=0;j<n;j++) 
    if ( data[j]>-98 ) 
    {  
      dev = data[j]-(*mean);
      sum+=dev*dev; 
    }
   if ( counter > 1 )  sum/=(counter-1);
   else  sum=0;
   *var=sum;
}

