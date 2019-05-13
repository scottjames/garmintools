project('garmintools', ['c'], version : '0.12.0')

cc = meson.get_compiler('c')

config = configuration_data()
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
