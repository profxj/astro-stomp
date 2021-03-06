// Copyright 2010  All Rights Reserved.
// Author: ryan.scranton@gmail.com (Ryan Scranton)

// STOMP is a set of libraries for doing astrostatistical analysis on the
// celestial sphere.  The goal is to enable descriptions of arbitrary regions
// on the sky which may or may not encode futher spatial information (galaxy
// density, CMB temperature, observational depth, etc.) and to do so in such
// a way as to make the analysis of that data as algorithmically efficient as
// possible.
//
// This header file loads the header files for the various pieces of the STOMP
// library.  This is intended to be a convenience so that users can have access
// to the library objects without having to specify which pieces of the library
// they want to access explicity.

#ifndef S2OMP_H
#define S2OMP_H

#include <s2omp/core.h>
#include <s2omp/pixel.h>
#include <s2omp/scalar_pixel.h>
#include <s2omp/tree_pixel.h>

#include <s2omp/point.h>
#include <s2omp/cosmo_point-inl.h>
#include <s2omp/indexed_point-inl.h>

#include <s2omp/bound_interface.h>
#include <s2omp/circle_bound.h>
#include <s2omp/annulus_bound.h>

#include <s2omp/pixel_union.h>
#include <s2omp/scalar_union.h>
#include <s2omp/tree_union.h>

#include <s2omp/angular_bin-inl.h>
#include <s2omp/coverer.h>
#include <s2omp/region_map.h>
#include <s2omp/util.h>

#endif
