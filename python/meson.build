pymod = import('python')
if get_option('python2')
    python2 = pymod.find_installation('python2', required: false)
    if python2.found()
        py2dep = python2.dependency('python', required: false)
        python2.extension_module('pygarmin', 'pygarmin.c',
                                dependencies: [py2dep, libgarmintools],
                                install: true, subdir: 'garmintools')

        python2.install_sources('garmintools/__init__.py', subdir: 'garmintools')
    endif
endif

if get_option('python')
    python3 = pymod.find_installation('python3', required: false)
    if python3.found()
        py3dep = python3.dependency('python', required: false)
        python3.extension_module('pygarmin', 'py3garmin.c',
                                dependencies: [py3dep, libgarmintools],
                                install: true, subdir: 'garmintools')

        python3.install_sources('garmintools/__init__.py', subdir: 'garmintools')
    endif
endif
