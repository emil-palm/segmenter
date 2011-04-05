require 'mkmf'


if RUBY_PLATFORM == "universal-darwin10.0"
  # TODO Set the archflags to -arch x86_64 ONLY if it's a 64-bit snow leopard machine.
   #ENV['ARCHFLAGS'] = "-arch i386 -arch x86_64"
   $CFLAGS.sub!("-arch x86_64", "")
end

if find_executable('pkg-config')
$CFLAGS << ' ' + `pkg-config libavfilter --cflags`.strip
$LDFLAGS << ' ' + `pkg-config libavfilter --libs`.strip
end

$CFLAGS << " -W -Wall"

if have_library("avformat") and find_header('libavformat/avformat.h') then
    $objs = %w(av_segmenter.o)
    create_makefile("av_segmenter/av_segmenter")
else
    STDERR.puts "missing library"
    exit 1
end
