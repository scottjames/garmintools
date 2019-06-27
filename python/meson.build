pymod = import('python')
python = pymod.find_installation('python2', required: false)
pydep = dependency('python', required: false)

if python.found() and get_option('python-module')
    python.extension_module('pygarmin', 'pygarmin.c',
                            dependencies: [pydep, libgarmintools],
                            install: true, subdir: 'garmintools')

    python.install_sources('garmintools/__init__.py', subdir: 'garmintools')
endif
