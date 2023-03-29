#include "uf3_pair_bspline.h"

#include "uf3_bspline_basis2.h"
#include "uf3_bspline_basis3.h"

#include "utils.h"
#include <vector>
#include <cmath>

using namespace LAMMPS_NS;

// Dummy constructor
uf3_pair_bspline::uf3_pair_bspline() {}

// Constructor
// Passing vectors by reference
uf3_pair_bspline::uf3_pair_bspline(LAMMPS *ulmp, const std::vector<double> &uknot_vect,
                                   const std::vector<double> &ucoeff_vect, double uknot_spacing)
{
  lmp = ulmp;
  knot_vect = uknot_vect;
  coeff_vect = ucoeff_vect;
  knot_spacing = uknot_spacing;

  knot_vect_size = uknot_vect.size();
  coeff_vect_size = ucoeff_vect.size();

  // Initialize B-Spline Basis Functions
  for (int i = 0; i < knot_vect.size() - 4; i++)
    bspline_bases.push_back(uf3_bspline_basis3(lmp, &knot_vect[i], coeff_vect[i]));

  // Initialize Coefficients and Knots for Derivatives
  for (int i = 0; i < coeff_vect_size - 1; i++) {
    double dntemp4 = 3 / (knot_vect[i + 4] - knot_vect[i + 1]);
    dncoeff_vect.push_back((coeff_vect[i + 1] - coeff_vect[i]) * dntemp4);
  }
  for (int i = 1; i < knot_vect_size - 1; i++) dnknot_vect.push_back(knot_vect[i]);

  // Initialize B-Spline Derivative Basis Functions
  for (int i = 0; i < dnknot_vect.size() - 3; i++)
    dnbspline_bases.push_back(uf3_bspline_basis2(lmp, &dnknot_vect[i], dncoeff_vect[i]));
}

uf3_pair_bspline::~uf3_pair_bspline() {}

double *uf3_pair_bspline::eval(double r)
{

  // Find knot starting position

  int start_index = 3+ (int) floor((r-knot_vect[0])/knot_spacing);
  //if (knot_vect.front() <= r && r < knot_vect.back()) {
  //  //Determine the interval for value_rij
  //  for (int i = 3; i < knot_vect_size - 1; ++i) {
  //    if (knot_vect[i] <= r && r < knot_vect[i + 1]) {
  //      start_index = i;
  //      break;
  //    }
  //  }
  //}
  //

  int knot_affect_start = start_index - 3;

  double rsq = r * r;
  double rth = rsq * r;

  // Calculate energy

  ret_val[0] = bspline_bases[knot_affect_start + 3].eval0(rth, rsq, r);
  ret_val[0] += bspline_bases[knot_affect_start + 2].eval1(rth, rsq, r);
  ret_val[0] += bspline_bases[knot_affect_start + 1].eval2(rth, rsq, r);
  ret_val[0] += bspline_bases[knot_affect_start].eval3(rth, rsq, r);

  // Calculate force

  ret_val[1] = dnbspline_bases[knot_affect_start + 2].eval0(rsq, r);
  ret_val[1] += dnbspline_bases[knot_affect_start + 1].eval1(rsq, r);
  ret_val[1] += dnbspline_bases[knot_affect_start].eval2(rsq, r);

  return ret_val;
}
