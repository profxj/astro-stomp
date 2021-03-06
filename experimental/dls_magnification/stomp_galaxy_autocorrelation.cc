#include <stdint.h>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <iostream>
#include <sstream>
#include <stomp.h>
#include <gflags/gflags.h>

// Define our command-line flags.
DEFINE_string(map_file, "",
              "Name of the ASCII file containing the StompMap geometry");
DEFINE_string(galaxy_file, "",
              "Name of the ASCII file containing the input galaxy catalog");
DEFINE_bool(galaxy_radec, true, "Galaxy coordinates are in RA-DEC");
DEFINE_bool(use_only_pairs, false, "Use Only Pairs in correlation");
DEFINE_string(output_tag, "test",
              "Tag for output file: Wtheta_OUTPUT_TAG");
DEFINE_double(theta_min, 0.001, "Minimum angular scale (in degrees)");
DEFINE_double(theta_max, 1.0, "Maximum angular scale (in degrees)");
DEFINE_int32(n_bins_per_decade, 5, "Number of angular bins per decade.");
DEFINE_int32(n_random, 1,
	     "Integer number of random points per galaxy to use.");
DEFINE_bool(single_index, false, "Use older single-index file format.");
DEFINE_bool(no_weight, false, "Input file is missing weight column.");
DEFINE_int32(maximum_resolution, -1,
	     "Maximum resolution to use for pixel-based estimator");

int main(int argc, char **argv) {
  std::string usage = "Usage: ";
  usage += argv[0];
  usage += " --map_file=<StompMap ASCII>";
  usage += " --galaxy_file=<Galaxy catalog ASCII>";
  google::SetUsageMessage(usage);
  google::ParseCommandLineFlags(&argc, &argv, true);

  if (FLAGS_map_file.empty()) {
    std::cout << usage << "\n";
    std::cout << "Type '" << argv[0] << " --help' for a list of options.\n";
    exit(1);
  }

  if (FLAGS_galaxy_file.empty()) {
    std::cout << usage << "\n";
    std::cout << "Type '" << argv[0] << " --help' for a list of options.\n";
    exit(1);
  }


  // First, we read our STOMP map into a map object.  There are a couple
  // permutations based on the various map formats that are out there: with
  // or without a weight column or in the single index or double index format.
  Stomp::Map* stomp_map;
  if (FLAGS_single_index) {
    if (FLAGS_no_weight) {
      stomp_map = new Stomp::Map(FLAGS_map_file, false, false);
    } else {
      stomp_map = new Stomp::Map(FLAGS_map_file, false);
    }
  } else {
    if (FLAGS_no_weight) {
      stomp_map = new Stomp::Map(FLAGS_map_file, true, false);
    } else {
      stomp_map = new Stomp::Map(FLAGS_map_file);
    }
  }
  std::cout << "Read map from " << FLAGS_map_file << "; total area: " <<
    stomp_map->Area() << " sq. deg.\n";

  // Now we read in our galaxy data file.  The expected format is
  //  RA  DEC  WEIGHT  MAGNITUDE
  // where the WEIGHT column is the likelihood that the object is a galaxy
  // and MAGNITUDE is the apparent magnitude in a given filter.  We filter all
  // of the objects against the map, tossing out any objects that aren't in the
  // map.
  Stomp::WAngularVector galaxy;
  std::ifstream galaxy_file(FLAGS_galaxy_file.c_str());
  double ra, dec, prob, mag, weight;
  uint32_t n_galaxy = 0;
  weight = 1.0;
  
  Stomp::AngularCoordinate::Sphere galaxy_sphere =
    Stomp::AngularCoordinate::Survey;
  if (FLAGS_galaxy_radec) galaxy_sphere = Stomp::AngularCoordinate::Equatorial;

  std::cout << "Reading in File\n";
  while (!galaxy_file.eof()) {
    galaxy_file >> ra >> dec >> prob >> mag;
    Stomp::WeightedAngularCoordinate tmp_ang(ra, dec, prob,
					     galaxy_sphere);
    if (stomp_map->FindLocation(tmp_ang, weight) &&
	(tmp_ang.Weight() > 0.2)) galaxy.push_back(tmp_ang);
    n_galaxy++;
  }
  galaxy_file.close();

  std::cout << "Read " << n_galaxy << " galaxies from " << FLAGS_galaxy_file <<
    "; kept " << galaxy.size() <<"\n";
  n_galaxy = galaxy.size();


  // Now, we set up the object that will contain the measurement results.  The
  // correlation object is a essentially a container for angular bin objects
  // which have a given angular range (all object or pixel pairs separated by
  // 0.01 < theta < 0.1 degrees, for instance).  In addition, the constructor
  // for these objects will work out, based on the angular bin size, which
  // Stomp::Map resolution would be appropriate for calculating the angular
  // correlation on that scale.
  Stomp::AngularCorrelation wtheta(FLAGS_theta_min, FLAGS_theta_max,
				   FLAGS_n_bins_per_decade);

  // That pixel-based estimator works well on large scales, but on small scales
  // we want to use a pair-based estimator (which will be faster and require
  // less memory, provided we choose the break sensibly).  This call will
  // modify all of the high-resolution bins so that they use the pair-based
  // estimator.
  if (FLAGS_use_only_pairs) {
    wtheta.UseOnlyPairs();
  }
  else {
    if (FLAGS_maximum_resolution == -1) {
      wtheta.AutoMaxResolution(static_cast<uint32_t>(n_galaxy), 
			       stomp_map->Area());
    }
    else {
      std::cout << "Setting maximum resolution to " <<
	static_cast<uint16_t>(FLAGS_maximum_resolution) << "...\n";
      wtheta.SetMaxResolution(static_cast<uint16_t>(FLAGS_maximum_resolution));
    }
  }

  // Now, we're ready to start calculating the autocorrelation.  It is
  // possible to do this all in a single call with
  //
  // wtheta.FindAutoCorrelation(*stomp_map, galaxy, 1);
  //
  // where the last argument controls the number of random catalogs generated
  // for the pair-based estimator.  Instead, we'll walk through the process
  // explicitly, starting with the creation of a ScalarMap for the
  // pixel-based estimator.
  std::cout << "Min Resolution is " << wtheta.MinResolution();
  wtheta.FindAutoCorrelation(*stomp_map, galaxy, FLAGS_n_random);
  

  // Finally write out the results...
  std::string wtheta_file_name = "Wtheta_" + FLAGS_output_tag;
  std::cout << "Writing galaxy auto-correlation to " <<
    wtheta_file_name << "\n";

  wtheta.Write(wtheta_file_name);

  return 0;
}
