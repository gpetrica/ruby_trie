$:.unshift File.dirname(__FILE__) + '/../lib'
$:.unshift File.dirname(__FILE__) + '/../ext/trie'

require 'trie'

max = 1000000

def time
  starttime = Time.now
  res = yield
  endtime = Time.now
  puts "Time elapsed: #{endtime - starttime} seconds"
	res
end

puts "With a native Trie..."
t = Trie.new
time do
  1.upto(max) do |i|
    t["item #{i}"] = "sweet"
  end

  1.upto(max) do |i|
    t["item #{i}"].class
  end

  # not implemented yet  
  # 1.upto(max) do |i|
  #   t.delete("item #{i}")
  # end
end


puts "With a Hash..."
t = Hash.new
time do
  1.upto(max) do |i|
    t["item #{i}"] = "sweet"
  end

  1.upto(max) do |i|
    t["item #{i}"].class
  end

  # 1.upto(max) do |i|
  #   t.delete("item #{i}")
  # end
end

# puts "With a Ruby Trie..."
# t = RubyTrie.new
# time do
#   1.upto(max) do |i|
#     t["item #{i}"] = "sweet"
#   end
# 
#   1.upto(max) do |i|
#     t["item #{i}"].class
#   end
# end

