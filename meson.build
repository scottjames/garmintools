project('garmintools', ['c'], version : '0.13.0', default_options : ['c_std=gnu11'])

cc = meson.get_compiler('c')

config = configuration_data()
config.set_quoted('GARMINTOOLS_VERSION_STRING', meson.project_version())
if host_machine.endian() == 'big'
    config.set('WORDS_BIGENDIAN', '1')
endif

usb = dependency('libusb-1.0')
math = cc.find_library('m', required: false)

# generate config.h include header
configure_file(output: 'config.h', configuration: config)
config = declare_dependency(include_directories : include_directories('.'))

subdir('src')
subdir('python')
subdir('doc')

meson.add_install_script('build-aux/meson/postinstall.py', get_option('bindir'))
