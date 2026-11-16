desktop_file = i18n.merge_file(
        input: 'id.blankonlinux.warsi.desktop.in',
       output: 'id.blankonlinux.warsi.desktop',
         type: 'desktop',
       po_dir: '../po',
      install: true,
  install_dir: get_option('datadir') / 'applications'
)

desktop_utils = find_program('desktop-file-validate', required: false)
if desktop_utils.found()
  test('Validate desktop file', desktop_utils, args: [desktop_file])
endif

appstream_file = i18n.merge_file(
        input: 'id.blankonlinux.warsi.metainfo.xml.in',
       output: 'id.blankonlinux.warsi.metainfo.xml',
       po_dir: '../po',
      install: true,
  install_dir: get_option('datadir') / 'metainfo'
)

appstreamcli = find_program('appstreamcli', required: false, disabler: true)
test('Validate appstream file', appstreamcli,
     args: ['validate', '--no-net', '--explain', appstream_file])

install_data('id.blankonlinux.warsi.gschema.xml',
  install_dir: get_option('datadir') / 'glib-2.0' / 'schemas'
)

compile_schemas = find_program('glib-compile-schemas', required: false, disabler: true)
test('Validate schema file',
     compile_schemas,
     args: ['--strict', '--dry-run', meson.current_source_dir()])


service_conf = configuration_data()
service_conf.set('bindir', get_option('prefix') / get_option('bindir'))
configure_file(
  input: 'id.blankonlinux.warsi.service.in',
  output: 'id.blankonlinux.warsi.service',
  configuration: service_conf,
  install_dir: get_option('prefix') / 'lib' / 'systemd' / 'user'
)

service_conf = configuration_data()
configure_file(
  input: 'id.blankonlinux.warsi.search-provider.ini.in',
  output: 'id.blankonlinux.warsi.search-provider.ini',
  configuration: service_conf,
  install_dir: get_option('datadir') / 'gnome-shell' / 'search-providers'
)

# DOWNSTREAM PATCH (APT system updates): start warsi-daemon at login while
# update notifications are enabled. Named warsi.desktop like the per-user
# autostart file upstream writes for automatic updates, which overrides it.
install_data('bazaar-update-notifications.desktop',
  rename: 'warsi.desktop',
  install_dir: get_option('sysconfdir') / 'xdg' / 'autostart'
)

# DOWNSTREAM PATCH (APT system updates): open .deb files with the package
# installer by default, see ai-patches/DIFF.md H15
install_data('bazaar-deb-mimeapps.list',
  rename: 'gnome-mimeapps.list',
  install_dir: get_option('sysconfdir') / 'xdg'
)

subdir('icons')
subdir('dbus')
