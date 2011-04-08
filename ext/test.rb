$:.unshift "#{File.dirname(__FILE__)}";require "segmenter_ext";a = Segmenter::segment("oi.ts","najs",10);b = a[0]; puts "#{b.index} - #{b.duration} - #{b.filename}\n"; puts a
#$:.unshift "#{File.dirname(__FILE__)}";require "segmenter_ext";a = Segmenter::segment("bajs/bajs.ts","09a7ca73-9ee6-4664-b73a-cc342c556fba",40);b = a[0]; puts "#{b.index} - #{b.duration} - #{b.filename}\n"
