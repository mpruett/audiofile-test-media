{
	'variables': {
		'yaml_cpp': 'yaml-cpp-0.2.6'
	},
	'targets': [
		{
			'target_name': 'yaml-cpp',
			'type': 'static_library',
			'include_dirs': [
				'<(yaml_cpp)/include'
			],
			'direct_dependent_settings': {
				'include_dirs': [
					'<(yaml_cpp)/include'
				]
			},
			'sources': [
				'<(yaml_cpp)/src/aliasmanager.cpp',
				'<(yaml_cpp)/src/collectionstack.h',
				'<(yaml_cpp)/src/conversion.cpp',
				'<(yaml_cpp)/src/directives.cpp',
				'<(yaml_cpp)/src/directives.h',
				'<(yaml_cpp)/src/emitfromevents.cpp',
				'<(yaml_cpp)/src/emitter.cpp',
				'<(yaml_cpp)/src/emitterstate.cpp',
				'<(yaml_cpp)/src/emitterstate.h',
				'<(yaml_cpp)/src/emitterutils.cpp',
				'<(yaml_cpp)/src/emitterutils.h',
				'<(yaml_cpp)/src/exp.cpp',
				'<(yaml_cpp)/src/exp.h',
				'<(yaml_cpp)/src/indentation.h',
				'<(yaml_cpp)/src/iterator.cpp',
				'<(yaml_cpp)/src/iterpriv.h',
				'<(yaml_cpp)/src/node.cpp',
				'<(yaml_cpp)/src/nodebuilder.cpp',
				'<(yaml_cpp)/src/nodebuilder.h',
				'<(yaml_cpp)/src/nodeownership.cpp',
				'<(yaml_cpp)/src/nodeownership.h',
				'<(yaml_cpp)/src/null.cpp',
				'<(yaml_cpp)/src/ostream.cpp',
				'<(yaml_cpp)/src/parser.cpp',
				'<(yaml_cpp)/src/ptr_stack.h',
				'<(yaml_cpp)/src/ptr_vector.h',
				'<(yaml_cpp)/src/regex.cpp',
				'<(yaml_cpp)/src/regex.h',
				'<(yaml_cpp)/src/regeximpl.h',
				'<(yaml_cpp)/src/scanner.cpp',
				'<(yaml_cpp)/src/scanner.h',
				'<(yaml_cpp)/src/scanscalar.cpp',
				'<(yaml_cpp)/src/scanscalar.h',
				'<(yaml_cpp)/src/scantag.cpp',
				'<(yaml_cpp)/src/scantag.h',
				'<(yaml_cpp)/src/scantoken.cpp',
				'<(yaml_cpp)/src/setting.h',
				'<(yaml_cpp)/src/simplekey.cpp',
				'<(yaml_cpp)/src/singledocparser.cpp',
				'<(yaml_cpp)/src/singledocparser.h',
				'<(yaml_cpp)/src/stream.cpp',
				'<(yaml_cpp)/src/stream.h',
				'<(yaml_cpp)/src/streamcharsource.h',
				'<(yaml_cpp)/src/stringsource.h',
				'<(yaml_cpp)/src/tag.cpp',
				'<(yaml_cpp)/src/tag.h',
				'<(yaml_cpp)/src/token.h'
			]
		}
	]
}
