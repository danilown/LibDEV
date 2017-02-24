#ifndef _DBM_H
#define _DBM_H

#include "dev.h"

/* Deep Boltzmann Machines */

double Bernoulli_BernoulliDBM4Reconstruction(Agent *a, va_list arg); /* It executes a Bernoulli-Bernoulli DBM and returns the reconstruction error */
double Bernoulli_BernoulliDBM4ReconstructionWithDropout(Agent *a, va_list arg); /* It executes a Bernoulli-Bernoulli DBM with Dropout and returns the reconstruction error */
double Bernoulli_BernoulliDBM4ReconstructionWithDropconnect(Agent *a, va_list arg); /* It executes a Bernoulli-Bernoulli DBM with Dropconnect and returns the reconstruction error */
/***********************************************/

#endif
