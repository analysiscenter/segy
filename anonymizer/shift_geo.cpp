#include <iostream>
#include <stdio.h>
#include <math.h>
#include <vector>
#include <cassert>
#include "shift_geo.h"

double dot(const std::vector<double> &x, const std::vector<double> &y) {
/**
    Scalar product of two vectors.

    @param x
    @param y
    @return sum
*/
    assert (x.size() == y.size());
    double sum = 0;
    for (std::size_t i = 0; i < x.size(); ++i) {
        sum += x[i] * y[i];
    }
    return sum;
}

std::vector<double> normalize_vector(std::vector<double> &v) {
/**
    Normalizing.

    @param v
    @return norm
*/
    double norm = sqrt(dot(v, v));
    if (norm == 0) {
        return v;
    }
    for (double &i : v) {
        i /= norm;
    }
    return v;
}

std::vector<double> compute_cross_product(const std::vector<double> &x, const std::vector<double> &y) {
/**
    Scalar cross-product of two vectors.

    @param x
    @param y
    @return res
*/	
    assert (x.size() == y.size());
	std::vector<double> res{x[1] * y[2] - x[2] * y[1],
							x[2] * y[0] - x[0] * y[2],
							x[0] * y[1] - x[1] * y[0]};
	return res;
}

std::vector<std::vector<double> > compute_rotation_matrix(std::vector<double> &axis, double theta) {
/**
    Compute the rotation matrix associated with counterclockwise rotation about
    the given axis by theta radians (using Euler-Rodrigues formula).

    @param axis    Vector
    @param theta   Angle
    @return res
*/
	axis = normalize_vector(axis);
	double a = cos(theta / 2.0);
	double b = - axis[0] * sin(theta / 2.0), c = - axis[1] * sin(theta / 2.0);
	double d = - axis[2] * sin(theta / 2.0);

    double aa = a * a, bb = b * b, cc = c * c, dd = d * d;
    double bc = b * c, ad = a * d, ac = a * c, ab = a * b, bd = b * d, cd = c * d;
	std::vector<std::vector<double> > res{{aa + bb - cc - dd, 2 * (bc + ad), 2 * (bd - ac)},
										  {2 * (bc - ad), aa + cc - bb - dd, 2 * (cd + ab)},
										  {2 * (bd + ac), 2 * (cd - ab), aa + dd - bb - cc}};
	return res;
}

std::vector<double> geo2cartesian(std::vector<double> &coordinates) {
/**
    Convert cartesian coordinates to geographical.

    @param coordinates{latitude, longitude}
    @return res
*/
	double latitude = coordinates[0] * PI / 180.0, longitude = coordinates[1] * PI / 180.0;
    double x = EARTH_RADIUS * cos(latitude) * cos(longitude);
    double y = EARTH_RADIUS * cos(latitude) * sin(longitude);
    double z = EARTH_RADIUS * sin(latitude);
    std::vector<double> res{x, y, z};
    return res;
}

std::vector<double> cartesian2geo(std::vector<double> &coordinates) {
/**
    Convert geographical coordinates to cartesian.

    @param coordinates
    @return res{latitude, longitude}
*/
    double latitude = asin(coordinates[2] / EARTH_RADIUS) * 180.0 / PI;
	double longitude = atan2(coordinates[1], coordinates[0]) * 180.0 / PI;
	std::vector<double> res{latitude, longitude};
	return res;
}

std::vector<double> rotate(std::vector<double> v, std::vector<double> &axis, double theta) {
/**
    Rotate vector.

    @param v
    @param axis    Vector
    @param theta   Angle
    @return res
*/
	std::vector<std::vector<double> > rotation_matrix = compute_rotation_matrix(axis, theta);
    std::vector<double> res(3, 0.0);
    for (std::size_t i = 0; i < v.size(); ++i) {
    	res[i] = dot(rotation_matrix[i], v);
	}
	return res;
}

std::vector<double> shift_geo_coordinates(double latitude, double longitude, double distance, double azimut) {
/**
	Shift by distance towards azimut angle
	
    @param latitude     Given in degrees.
    @param longitude    Given in degrees.
	@param distance     Shift distance given in kilometers.
	@param azimut 		Azimut angle given in degrees. Default value is 0.
		                An angle between vector pointing to the north and
                        vector to the destination point.

	@example
	100 meters shift from the Saint Basil Chuch to the North:
	shift_geo_coordinates(55.752618, 37.622960, 0.1, 0)
*/
	azimut = azimut * PI / 180.0;
	std::vector<double> coordinates{latitude, longitude};
	std::vector<double> start_point = geo2cartesian(coordinates);
	std::vector<double> north_pole{0, 0, EARTH_RADIUS};
    std::vector<double> axis_1 = compute_cross_product(start_point, north_pole);
    double theta_1 = distance / EARTH_RADIUS;
    std::vector<double> shifted2north = rotate(start_point, axis_1, theta_1);
    std::vector<double> res = rotate(shifted2north, start_point, azimut);
    return res;
}
