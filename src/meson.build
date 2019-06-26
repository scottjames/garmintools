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


executable('garmintool',
    files(
        'garmintool.c',
        'garmin_get_info.c',
        'garmin_dump.c',
        'garmin_save_runs.c',
        'garmin_convert.c',
        'garmin_tcx.c',
        'garmin_gchart.c',
        'garmin_gpx.c',
        'garmin_gmap.c',
    ),
    dependencies: [config, libgarmintools, math],
    install: true
)
