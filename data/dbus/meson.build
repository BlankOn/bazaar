service_conf = configuration_data()
service_conf.set('bindir', get_option('prefix') / get_option('bindir'))

configure_file(
  input: 'id.blankonlinux.warsi.SearchProvider.service.in',
  output: 'id.blankonlinux.warsi.SearchProvider.service',
  configuration: service_conf,
  install_dir: get_option('datadir') / 'dbus-1' / 'services'
)

configure_file(
  input: 'id.blankonlinux.warsi.service.in',
  output: 'id.blankonlinux.warsi.service',
  configuration: service_conf,
  install_dir: get_option('datadir') / 'dbus-1' / 'services'
)