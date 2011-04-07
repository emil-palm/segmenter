PKG_FILES = [
	"Rakefile", "COPYING", "NEWS", "README.rdoc",
	"ext/segmenter.c", "Manifest", "ext/extconf.rb", "lib/segmenter.rb"
]

Gem::Specification.new do |s|
	s.name = "segmenter"
	s.version = "0.3"
	s.email = "emil.palm@x86.nu"
	s.homepage = "http://segmenter.x86.nu"
	s.summary = "Ruby wrapper for http://svn.assembla.com/svn/legend/segmenter/segmenter.c"
	s.files = PKG_FILES
	s.required_ruby_version = '>= 1.8.1'
	s.extensions = "ext/extconf.rb"
	s.author = "Emil Palm"
  	s.require_paths << "ext"
end
