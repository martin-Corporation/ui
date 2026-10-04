project(
  'ui',
  'c',
  meson_version: '>= 1.3.0',
  version: '0.1',
  default_options: ['warning_level=3'],
)

dependencies = [dependency('gtk4'), dependency('libadwaita-1')]
sources = ['main.c', 'src/list.c', 'src/node.c', 'src/backends/gtk.c']
include_directories = include_directories('src/include')

executable(
  'ui',
  sources: sources,
  dependencies: dependencies,
  install: true,
  include_directories: include_directories,
)
