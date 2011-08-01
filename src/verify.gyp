{
	'targets': [
		{
			'target_name': 'verify',
			'type': 'executable',
			'sources': [
				'verify.cpp',
				'md5.cpp',
				'md5.h',
			],
			'dependencies': [
				'third_party/yaml-cpp/yaml-cpp.gyp:yaml-cpp'
			],
			'cflags': [
				'<!@(pkg-config --cflags audiofile)'
			],
			'ldflags': [
				'<!@(pkg-config --libs-only-L audiofile)'
			],
			'libraries': [
				'<!@(pkg-config --libs-only-l audiofile)'
			]
		}
	]
}
