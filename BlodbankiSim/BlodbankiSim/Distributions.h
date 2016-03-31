//Distributions from GNU

extern int poissonRandom(double);
extern double  normalDistribution(double, double);

// Distributions from Tómas

float discrete_empirical(float *F, int n, int stream);
int poissonrnd(float lambda, int stream);
float gammarnd(float alpha, float beta, int stream);
int negativebinomrnd(float size, float mu, int stream);