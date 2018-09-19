#define EARTH_RADIUS 6371
#define PI 3.14159265

std::vector<double> cartesian2geo(std::vector<double> coordinates);
std::vector<double> shift_geo_coordinates(double latitude, double longitude, double distance, double azimut);
