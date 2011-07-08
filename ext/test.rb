$:.unshift "#{File.dirname(__FILE__)}";require "segmenter_ext";a = Segmenter::segment("audio_only.ts","najs",3); a.each do |b| puts "#{b.index} - #{b.duration} - #{b.filename}\n";end
#$:.unshift "#{File.dirname(__FILE__)}";require "segmenter_ext";a = Segmenter::segment("movie.ts","najs",3); a.each do |b| puts "#{b.index} - #{b.duration} - #{b.filename}\n";end
#$:.unshift "#{File.dirname(__FILE__)}";require "segmenter_ext";a = Segmenter::segment("bajs/bajs.ts","09a7ca73-9ee6-4664-b73a-cc342c556fba",40);b = a[0]; puts "#{b.index} - #{b.duration} - #{b.filename}\n"
