/* gauss.h
 */

#ifndef gauss_h
#define gauss_h

#ifdef __cplusplus
extern "C" {
#endif

double gsl_cdf_ugaussian_P (const double x);
double gsl_cdf_ugaussian_Q (const double x);

double gsl_cdf_gaussian_P (const double x, const double sigma);
double gsl_cdf_gaussian_Q (const double x, const double sigma);

#ifdef __cplusplus
}
#endif

#endif
