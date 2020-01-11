#!/usr/bin/env python3

import os
import os.path
import sys

prefix = os.environ.get('MESON_INSTALL_DESTDIR_PREFIX')
bindir = sys.argv[1]
os.chdir(os.path.join(prefix, bindir))
os.symlink("garmintool", "garmin_save_runs")
os.symlink("garmintool", "garmin_tcx")
os.symlink("garmintool", "garmin_gpx")
os.symlink("garmintool", "garmin_dump")
os.symlink("garmintool", "garmin_get_info")
os.symlink("garmintool", "garmin_gchar")
os.symlink("garmintool", "garmin_gmap")
