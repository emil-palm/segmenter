require 'rake/gempackagetask'

PKG_FILES = FileList[
	"Rakefile", "COPYING", "NEWS", "README.rdoc",
	"ext/*.[ch]", "Manifest", "ext/extconf.rb"
]

SPEC = Gem::Specification.new do |s|
	s.name = "Segmenter"
	s.version = "0.1"
	s.email = "emil.palm@x86.nu"
	s.homepage = "http://segmenter.x86.nu"
	s.summary = "Ruby wrapper for http://svn.assembla.com/svn/legend/segmenter/segmenter.c"
	s.files = PKG_FILES
	s.required_ruby_version = '>= 1.8.1'
	s.extensions = "ext/extconf.rb"
	s.author = "Emil Palm"
end

Rake::GemPackageTask.new(SPEC) do |pkg|
pkg.need_tar = true
pkg.need_zip = true
end
