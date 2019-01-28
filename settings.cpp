typedef struct
{
	int x;
	int y;
	double lon;
	double lat;
} img_point_t;

bool operator==(const img_point_t& a, const img_point_t& b)
{
	return (a.x == b.x) &&
			(a.y == b.y) &&
			(a.lon == b.lon) &&
			(a.lat == b.lat);
}

typedef struct settings
{
	// Given from user via command line arguments
	img_point_t p1;
	img_point_t p2;
	int output_tile_size;
	std::string file;
	std::string output_folder;
	int zoom_level;

	// Calculated based on the arguments above
	int first_tile_x_px;
	int first_tile_y_px;
	int start_x_coord;
	int start_y_coord;
	float tile_size_px;
} settings_t;

void
print_usage()
{
	LOG("image2tiles - Cutting an image into OGC conformative tiles.");
	LOG("");
	LOG("Options:");
	LOG("  -1, --p1               First point using a point string (1)");
	LOG("  -2, --p2               Second point using a point string (1)");
	LOG("  -z, --zoom-level       Zoom level (0..19)");
	LOG("  -t, --tile-size        Size of a tile in pixel (default: 256)");
	LOG("  -o, --output-folder    Output folder (defult: .out/)");
	LOG("  -f, --file             The image file that should be cutted");
	LOG("");
	LOG("Flags:");
	LOG("  -v, --verbose          More detailed output");
	LOG("  -d, --debug            Even more output including debug logging");
	LOG("      --version          Version of this application");
	LOG("  -h, --help             Prints this message");
	LOG("");
	LOG("");
	LOG("");
	LOG("(1) Point string:");
	LOG("");
	LOG("A string representing the mapping from an image pixel to longitude/\n\
latitude degrees. Specifying two points is used to determine the scale and\n\
starting tiles of the image.");
	LOG("It is formatted as followed:");
	LOG("");
	LOG("    <pixel-X>,<longitude>,<pixel-Y>,<latitude>");
	LOG("");
	LOG("Example:");
	LOG("");
	LOG("    771,43.55,220,-14.96");
	LOG("");
	LOG("The further away these points are on the image, the better is the\n\
accuracy. Choosing the top-left and bottom-right corners of the image will\n\
result in the best accuracy. Be sure the points do not have similar x oder y\n\
coordinated (therefore two diagonal placed points are good)");
	LOG("To fine the longitude/latitude of a point an online service can be used\n\
(e.g. https://openstreetmap.org).");
	LOG("");
	LOG("");
	LOG("");
	LOG("License:     GPL-3.0");
	LOG("Source code: https://github.com/hauke96/image2tiles");
}

void
parse_args(int argc, char** argv, settings_t *settings)
{
	// Default settings
	settings->output_tile_size = 256;
	settings->output_folder = "./out";

	// Regex for parsing the points
	std::string float_regex_str = "[+-]?[\\d]*\\.?[\\d]+";
	std::string int_regex_str = "[-+]?\\d+";
	// For example: --p1=100,20.123,100,64.123
	std::regex point_regex("(" + int_regex_str + "),(" + float_regex_str + "),(" + int_regex_str + "),(" + float_regex_str + ")");

	static struct option long_options[] = {
		{"zoom-level",    required_argument, 0, 'z' },
		{"tile-size",     required_argument, 0, 't' },
		{"p1",            required_argument, 0, '1' },
		{"p2",            required_argument, 0, '2' },
		{"file",          required_argument, 0, 'f' },
		{"output-folder", required_argument, 0, 'o' },
		{"verbose",       no_argument,       0, 'v' },
		{"version",       no_argument,       0,  0  },
		{"debug",         no_argument,       0, 'd' },
		{"help",          no_argument,       0, 'h' },
		{0,               0,                 0,  0  }
	};
	
	while (1)
	{
		int option_index = 0;
		int c = getopt_long(argc, argv, "hvdo:f:z:1:2:t:",
			long_options, &option_index);
		if (c == -1)
		{
			break;
		}

		switch (c)
		{
			case 0:
			{
				std::string opt = long_options[option_index].name;

				if (opt == "version")
				{
					LOG(VERSION);
					exit(0);
				}

				break;
			}
			case '1': // fall through
			case '2':
			{
				std::smatch matches;
				std::string arg_str(optarg);
				if (std::regex_search(arg_str, matches, point_regex))
				{
					img_point_t *p;
					if (c == '1')
					{
						p = &settings->p1;
					}
					else
					{
						p = &settings->p2;
					}
					p->x = atoi(matches.str(1).c_str());
					p->lon = std::stof(matches.str(2).c_str());
					p->y = atoi(matches.str(3).c_str());
					p->lat = std::stof(matches.str(4).c_str());
					DLOG("> %d", p->x);
					DLOG("> %f", p->lon);
					DLOG("> %d", p->y);
					DLOG("> %f", p->lat);
				}
				else
				{
					ELOG("Cannot parse point '%s'", optarg);
					LOG("A correct point option would be: --p1=1,-23,45,+6.78");
					LOG("(The + is optional and make sure there are no spaces)");
					exit(EINVAL);
				}

				break;
			}
			case 't':
				settings->output_tile_size = atoi(optarg);
				break;
			case 'v':
				VERBOSE = 1;
				break;
			case 'd':
				DEBUG = 1;
				VERBOSE = 1;
				break;
			case 'f':
				settings->file = optarg;
				break;
			case 'o':
				settings->output_folder = optarg;
				break;
			case 'z':
				settings->zoom_level = atoi(optarg);
				break;
			case 'h':
				print_usage();
				exit(0);
			case '?':
				ELOG("Unrecognized option '%s'", optarg);
				exit(EINVAL);
		}
	}
}

void
fill_tile_settings(settings_t *settings)
{
	int zoom_level = settings->zoom_level;

	// W/E
	double p1_long = settings->p1.lon;
	int p1_x = settings->p1.x;
	// N/S
	double p1_lat = settings->p1.lat;
	int p1_y = settings->p1.y;

	// W/E
	double p2_long = settings->p2.lon;
	int p2_x = settings->p2.x;
	// N/S
	double p2_lat = settings->p2.lat;
	int p2_y = settings->p2.y;

	double pixel_per_long = abs(p1_x - p2_x) / abs(p1_long - p2_long);
	double pixel_per_lat = abs(p1_y - p2_y) / abs(p1_lat - p2_lat);

	DLOG("pixel per long: %f", pixel_per_long);
	DLOG("pixel per lat: %f", pixel_per_lat);
	
	double long_per_tile = 360 / pow(2, zoom_level);

	settings->tile_size_px = pixel_per_long * long_per_tile;

	DLOG("tile size: %f", settings->tile_size_px);

	// Determine most upper left given point. This is then used to calculate the tile and image offset within the tile of the origin.
	int x_px = std::min(p1_x, p2_x);
	double x_long;
	if (x_px == p1_x)
	{
		x_long = p1_long;
	}
	else
	{
		x_long = p2_long;
	}

	int y_px = std::min(p1_y, p2_y);
	double y_lat;
	if (y_px == p1_y)
	{
		y_lat = p1_lat;
	}
	else
	{
		y_lat = p2_lat;
	}

	double origin_long =  x_long - x_px / pixel_per_long;
	double origin_lat =  y_lat + y_px / pixel_per_lat;

	DLOG("x_long: %f", x_long);
	DLOG("y_lat: %f", y_lat);

	DLOG("origin_long: %f", origin_long);
	DLOG("origin_lat: %f", origin_lat);

	settings->start_x_coord = long_to_tile_x(origin_long, zoom_level);
	settings->start_y_coord = lat_to_tile_y(origin_lat, zoom_level);

	DLOG("tile x: %d", settings->start_x_coord);
	DLOG("tile y: %d", settings->start_y_coord);

	double origin_tile_long = tile_x_to_long(long_to_tile_x(origin_long, zoom_level), zoom_level);
	double origin_tile_lat =  tile_y_to_lat(lat_to_tile_y(origin_lat, zoom_level), zoom_level);

	DLOG("back to long: %f", origin_tile_long);
	DLOG("back to lat: %f", origin_tile_lat);

	settings->first_tile_x_px = (origin_tile_long - origin_long) * pixel_per_long;
	settings->first_tile_y_px = (origin_lat - origin_tile_lat) * pixel_per_lat;

	DLOG("x offset for first tile: %d", settings->first_tile_x_px);
	DLOG("y offset for first tile: %d", settings->first_tile_y_px);
}

int
verify_settings(settings_t *settings)
{
	// Points must not be equal
	if (settings->p1 == settings->p2)
	{
		ELOG("Specified points must be not equal");
		return 1;
	}

	// output tile size valid
	if (settings->output_tile_size > 0)
	{
		ELOG("Output tile size must be greater than zero");
		return 2;
	}

	// imput file exists
	if (std::experimental::filesystem::exists(settings->file))
	{
		ELOG("Input file does not exist");
		return 3;
	}

	// output folder valid (writable)
	
	// zoom level correct
	if (settings->zoom_level >= 0 && settings->zoom_level <= 19)
	{
		ELOG("The zoom level must be from 0 to 19");
		return 4;
	}

	// tile size valid
	if (settings->tile_size_px > 0)
	{
		ELOG("Calculated tile size is not valid, it must be greater than zero");
		return 5;
	}

	return 0;
}
