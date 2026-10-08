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
sources = [
  'main.c',
  'src/list.c',
  'src/node.c',
]

if backend == 'gtk'
  dependencies += [dependency('gtk4'), dependency('libadwaita-1')]
  sources += 'src/backends/gtk.c'
elif backend == 'qt'
  dependencies += [
    dependency(
      'qt6',
      modules: ['Core', 'Gui', 'Qml', 'Quick'],
      version: '>=6.5',
      required: true,
    ),
  ]

  add_languages('cpp', native: false)
  sources += [
    'src/backends/qt.cpp',
    import('qt6').compile_moc(
      sources: 'src/backends/qt.cpp',
      dependencies: dependencies,
    ),
  ]
endif

includes = include_directories('include')
executable(
  'ui',
  sources: sources,
  dependencies: dependencies,
  install: true,
  include_directories: includes,
)
