typedef struct
{
	int x;
	int y;
	double lon;
	double lat;
} img_point_t;

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
		{"zoom-level", required_argument, 0, 'z' },
		{"tile-size",  required_argument, 0, 't' },
		{"p1",         required_argument, 0, '1' },
		{"p2",         required_argument, 0, '2' },
		{"file",       required_argument, 0, 'f' },
		{"output",     required_argument, 0, 'o' },
		{"verbose",    no_argument,       0, 'v' },
		{"version",    no_argument,       0,  0  },
		{"debug",      no_argument,       0, 'd' },
		{0,            0,                 0,  0  }
	};
	
	while (1)
	{
		int option_index = 0;
		int c = getopt_long(argc, argv, "vdo:f:z:1:2:t:",
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
			case '?':
				ELOG("Unrecognized option '%s'", optarg);
				exit(EINVAL);
		}
	}
}

