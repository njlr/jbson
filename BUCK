include_defs('//BUCKAROO_DEPS')

prebuilt_cxx_library(
  name = 'jbson',
  header_only = True,
  header_namespace = 'jbson'
  exported_headers = subdir_glob([
    ('include/jbson', '**/*.hpp'),
  ]),
  visibility = [
    'PUBLIC',
  ],
  deps = BUCKAROO_DEPS,
)
