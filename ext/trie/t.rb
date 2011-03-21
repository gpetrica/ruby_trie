require 'rubygems'
require 'trie'
require 'benchmark'

t = Trie.new
c = 0
s1 = (Benchmark.measure do 
  open('/usr/share/dict/web2').each_line do |w|
    t[w.chop] = w.chop
    c += 1
  end
end)

# %w(gol golas golaster lux xal).each {|w| t[w] = w}
s2 = (Benchmark.measure do
    t.levenshtein_search('food', 1) {|p| puts p if p}
end)

puts "#{t.memory/(1024*1024)}Mb, #{c} words"
puts s1
puts s2