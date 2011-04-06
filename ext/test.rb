$:.unshift "#{File.dirname(__FILE__)}"
require 'segmenter_ext'
a = Segmenter::segment("bajs.ts","test",10);
b = a[3]
puts "#{b.index} - #{b.duration} - #{b.filename}\n"