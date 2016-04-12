
#include <math.h>
#include <stdlib.h>
#include <limits.h>

#include "simlib.h"


/* Dreifingar sem Tómas setti inn */
/* discrete empirical distribution, F is the discrete CDF of length n and discrete values returned are 0,1,2,3,...(n-1) */
float discrete_empirical(float *F, int n, int stream)
{
	float u = lcgrand(stream);
	int i;

	for (i = 0; i < n; i++)
		if (u <= F[i])
			break;
	return ((float)i);
}

int poissonrnd(float lambda, int stream) {
	float p = 0;
	int X = 0;
	while (1) {
		p -= log(lcgrand(stream));
		if (p >= lambda)
			break;
		X++;
	}
	return(X);
}

float gammarnd(float alpha, float beta, int stream) {
	float a = 1.0 / sqrt(2.0*alpha - 1.0);
	float b = alpha - log(4.0);
	float q = alpha + 1.0 / a;
	float theta = 4.5;
	float d = 1 + log(theta);
	int i = 0;
	float U1, U2, V, Y, Z, W, GAM = 0;
	while (i < 1) {
		i = i + 1;
		U1 = lcgrand(stream);
		U2 = lcgrand(stream);
		V = a * log(U1 / (1.0 - U1));
		Y = alpha * exp(V);
		Z = U1*U1 * U2;
		W = b + q*V - Y;
		if ((W + d - theta*Z) >= 0) {
			GAM = Y;
		}
		else if (W >= log(Z)) {
			GAM = Y;
		}
		else {
			i -= 1;
		}
	}
	return(GAM * beta);
}

int negativebinomrnd(float size, float mu, int stream) {
	float p = size / (size + mu);
	float X = gammarnd(size, 1.0, stream);
	int Z = poissonrnd(X*(1 - p) / p, stream);
	return(Z);
}

int geoRandom( double probability, int stream){
	double u = lcgrand(stream);
	float geo = ceil(abs(log(u)/log(1-probability)-1));
	return geo;
}
