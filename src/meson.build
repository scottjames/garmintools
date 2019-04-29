pkg = import('pkgconfig')

lib = library('garmintools',
        ['usb_comm.c',
         'byte_util.c',
         'unpack.c',
         'pack.c',
         'protocol.c',
         'command.c',
         'packet_id.c',
         'print.c',
         'datatype.c',
         'symbol_name.c',
         'run.c'],
         dependencies : [config, usb],
         version: '6.2.0',
         install : true)
install_headers('garmin.h', subdir: 'garmintools')
pkg.generate(lib,
             description: 'Library implementing the Garmin USB protocol',
             subdirs: 'garmintools')

libgarmintools = declare_dependency(link_with: lib,
                                    include_directories: include_directories('.'),
                                    dependencies: usb)


executable('garmin_save_runs', 'garmin_save_runs.c', dependencies: libgarmintools)
executable('garmin_dump', 'garmin_dump.c', dependencies: libgarmintools)
executable('garmin_get_info', 'garmin_get_info.c', dependencies: libgarmintools)
executable('garmin_gmap', 'garmin_get_info.c', dependencies: [libgarmintools, math])
executable('garmin_gchart', 'garmin_gchart.c', dependencies: [libgarmintools, math])
executable('garmin_gpx', 'garmin_gpx.c', dependencies: [libgarmintools, math])
executable('garmin_tcx', 'garmin_tcx.c', dependencies: [libgarmintools, math])
