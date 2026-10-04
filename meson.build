project(
  'ui',
  'c',
  meson_version: '>= 1.3.0',
  version: '0.1',
  default_options: [
    'warning_level=3',
    'cpp_std=c++20',
  ],
)

dependencies = []
backend = get_option('ui_backend')

if backend == 'gtk'
  dependencies += [dependency('gtk4'), dependency('libadwaita-1')]
  backend_file_extension = 'c'
elif backend == 'qt'
  dependencies += [
    dependency(
      'qt6',
      modules: ['Core', 'Gui', 'Qml', 'Quick'],
      version: '>=6.5',
      required: true,
    ),
  ]

  add_languages('cpp')
  backend_file_extension = 'cpp'
endif

sources = [
  'main.c',
  'src/list.c',
  'src/node.c',
  'src/backends/' + backend + '.' + backend_file_extension,
]

include_directories = include_directories('src/include')

executable(
  'ui',
  sources: sources,
  dependencies: dependencies,
  install: true,
  include_directories: include_directories,
)
