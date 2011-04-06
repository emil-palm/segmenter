$:.unshift "#{File.dirname(__FILE__)}";require "segmenter_ext";a = Segmenter::segment("bajs/bajs.ts","test",180);b = a[0]; puts "#{b.index} - #{b.duration} - #{b.filename}\n"
