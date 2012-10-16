{
  'targets': [{
    'target_name': 'node-cubeb',
    'sources': [
      'src/common.cpp',
      'src/constants.cpp',
      'src/context.cpp',
      'src/stream.cpp',
      'src/module.cpp'
    ],
    'conditions': [
      ['OS=="linux"', {
        'link_settings': {
          'libraries': ['-lcubeb']
        }
      }
    ], ['OS=="mac"', {
        'link_settings': {
          'libraries': ['-lcubeb']
        }
      }
    ]]
  }]
}
