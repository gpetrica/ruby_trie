# Copyright 2008 David Vollbracht & Philippe Hanrigou

require 'rake'
require 'rake/testtask'
require 'rake/rdoctask'
require 'rake/gempackagetask'
require 'rake/clean'
require 'rubygems'

CLEAN.include '**/*.o'
CLEAN.include '**/*.so'
CLEAN.include '**/*.bundle'
CLOBBER.include '**/*.log'
CLOBBER.include '**/Makefile'
CLOBBER.include '**/extconf.h'

RUBY_TRIE_VERSION = "1.0"
RUBY_TRIE_GEM_NAME = "RubyTrie"

desc 'Default: run unit tests.'
task :default => :test

desc 'Install the gem into the local gem repository' 
task :install => 'package' do
  sh "gem install ./pkg/#{RUBY_TRIE_GEM_NAME}-#{RUBY_TRIE_VERSION}.gem"
end

desc 'Test SystemTimer'
Rake::TestTask.new(:test) do |t|
  t.libs << 'lib'
  t.pattern = 'test/**/*_test.rb'
  t.verbose = true
end
task :test => 'ext/trie/libtrie.so'

desc 'Generate documentation for SystemTimer.'
Rake::RDocTask.new(:rdoc) do |rdoc|
  rdoc.rdoc_dir = 'rdoc'
  rdoc.title    = 'SystemTimer'
  rdoc.options << '--line-numbers' << '--inline-source'
  rdoc.rdoc_files.include('README')
  rdoc.rdoc_files.include('lib/**/*.rb')
end

file 'ext/trie/Makefile' => 'ext/trie/extconf.rb' do
  Dir.chdir('ext/trie') do
    ruby 'extconf.rb'
  end
end

file 'ext/trie/libtrie.so' => 'ext/trie/Makefile' do
  Dir.chdir('ext/trie') do
    pid = fork { exec "make" }
    Process.waitpid pid
  end
  fail "Make failed (status #{m})" unless $?.exitstatus == 0
end

specification = Gem::Specification.new do |s|
  s.name = RUBY_TRIE_GEM_NAME
  s.summary = "Set a Timeout based on signals, which are more reliable than Timeout. Timeout is based on green threads."
  s.version = RUBY_TRIE_VERSION
  s.authors = ["Matt Freels", "Petrica Ghiurca"]
  s.platform = Gem::Platform::RUBY

  s.files = [ "README", "ChangeLog"] + 
              FileList['ext/**/*.c'] + 
              FileList['ext/**/*.rb'] + 
              FileList['lib/**/*.rb'] + 
              FileList['test/**/*.rb']
  s.autorequire = "trie"
  s.extensions = ["ext/trie/extconf.rb"]

  s.require_path = "lib"
  s.rdoc_options << '--title' << 'RubyTrie' << '--main' << 'README' << '--line-numbers'
  s.has_rdoc = false
  s.extra_rdoc_files = ['README']
	s.test_file = "test/trie_test.rb"
	# s.rubyforge_project = "rubytrie"
end
  
Rake::GemPackageTask.new(specification) do |package|
	 package.need_zip = false
	 package.need_tar = false
end

desc "Publish RDoc on Rubyforge website"
task :publish_rdoc => :rdoc do
  sh "scp -r rdoc/* #{ENV['USER']}@rubyforge.org:/var/www/gforge-projects/rubytrie"
end
