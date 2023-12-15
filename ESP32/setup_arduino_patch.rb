#!/bin/env ruby

#small helper script to link to souce file in a way expected
#by the arduino IDE


if ARGV.size != 1
	STDERR.puts "A single folder name is expected as argument"
	exit(1)
	return
end

require 'fileutils'
folder = ARGV[0]

unless File.exist? folder
	FileUtils.mkdir_p folder
end

source_files = %w(
pector_config.c
pector_reader.c
pector_stream_processor.c
pector_util.c
pffft.c
)

header_files = %w(
pector_config.h
pector_reader.h
pector_stream_processor.h
pector_util.h
pffft.h
)

ino_file = "#{folder}/#{folder}.ino"
unless File.exist? ino_file
	content = header_files.map{|e| "#include \"#{e}\""}.join("\n")
	File.write(ino_file, content)
end

(header_files + source_files).each do |file|
	if ! File.exist? "../src/#{file}"
		STDERR.puts "../src/#{file} does not exist"
		exit -10
	end
end

source_files.each do |source_file|
	system("ln -s ../../src/#{source_file} #{File.join(folder,source_file)}pp")
end

header_files.each do |header_file|
	system("ln -s ../../src/#{header_file} #{folder}")
end

source_files.each do |source_file|
	system("ln -s ../../src/#{source_file} #{File.join(folder,source_file)}pp")
end
