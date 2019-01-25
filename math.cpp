#include <math.h>

/**
 * Determines the signum of a number.
 * 
 * @return -1 for negative number, 1 for positive number and 0 for 0
 */
int
sgn(double n)
{
	return (0 < n) - (n < 0);
}

/**
 * Calculates the cosine.
 * 
 * @param number The number in degree
 */
double
dcos(double number)
{
	return cos(number * M_PI / 180.0);
}

/**
 * Calculates the tangent.
 * 
 * @param number The number in degree
 */
double
dtan(double number)
{
	return tan(number * M_PI / 180.0);
}

/**
 * Determines the X coordinate for the given longitude and zoom level.
 * 
 * @param lon Longitude degrees
 * @param z Zoom level
 */
int
long_to_tile_x(double lon, int z)
{
	return (int)(floor((lon + 180.0) / 360.0 * pow(2.0, z)));
}

/**
 * Determines the Y coordinate for the given latitdue and zoom level.
 * 
 * @param lat Latitude degrees
 * @param z Zoom level
 */
int
lat_to_tile_y(double lat, int z)
{
	return (int)(floor((1.0 - log(dtan(lat) + 1.0 / dcos(lat)) / M_PI) / 2.0 * pow(2.0, z)));
}

/**
 * Determines the longitude degrees for the given X coordinate and zoom level.
 * The longitude degrees are at the top edge of the tile image.
 * 
 * @param x X coordinate of the tile
 * @param z Zoom level
 */
double
tile_x_to_long(int x, int z)
{
	return x / pow(2, z) * 360.0 - 180;
}

/**
 * Determines the latitude degrees for the given Y coordinate and zoom level.
 * The latitude degrees are at the left edge of the tile image.
 * 
 * @param y Y coordinate of the tile
 * @param z Zoom level
 */
double
tile_y_to_lat(int y, int z)
{
	double n = M_PI - 2.0 * M_PI * y / pow(2.0, z);
	return 180.0 / M_PI * atan(0.5  * (exp(n) - exp(-n)));
}
