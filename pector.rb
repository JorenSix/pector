#!/usr/bin/env ruby

require 'fileutils'
require 'tempfile'
require 'open3'

TARGET_SAMPLE_RATE=16000
ALLOWED_AUDIO_FILE_EXTENSIONS = "**/*.{m4a,wav,mp4,wv,ape,ogg,mp3,raw,flac,wma,M4A,WAV,MP4,WV,APE,OGG,MP3,FLAC,WMA}"
AUDIO_DURATION_COMMAND = "ffprobe -i \"__input__\" -show_entries format=duration -v quiet -of csv=\"p=0\""
AUDIO_CONVERT_COMMAND = "ffmpeg -hide_banner -y -loglevel panic  -i \"__input__\" -ac 1 -ar #{TARGET_SAMPLE_RATE} -f f32le -acodec pcm_f32le \"__output__\""
AUDIO_CONVERT_FROM_RAW_COMMAND = "ffmpeg -hide_banner -y -loglevel panic  -ac 1 -ar #{TARGET_SAMPLE_RATE} -f f32le -acodec pcm_f32le -i \"__input__\"  \"__output__\""
AUDIO_CONVERT_COMMAND_WITH_START_DURATION = "ffmpeg -hide_banner -y -loglevel panic -ss __start__ -i \"__input__\" -t __duration__ -ac 1 -ar #{TARGET_SAMPLE_RATE} -f f32le -acodec pcm_f32le \"__output__\""
MIC_INPUT = "ffmpeg -hide_banner -loglevel panic  -f avfoundation -i 'none:default' -ac 1 -ar #{TARGET_SAMPLE_RATE} -f f32le -acodec pcm_f32le pipe:1"
EXECUTABLE_LOCATION = "bin/pector_c"


# for a folder each audio file within that folder (and subfolders) is added to the list
def audio_file_list(arg,files_to_process)
  arg = File.expand_path(arg)
  if File.directory?(arg)
    audio_files_in_dir = Dir.glob(File.join(arg,ALLOWED_AUDIO_FILE_EXTENSIONS))
    audio_files_in_dir.each do |audio_filename|
      files_to_process << audio_filename
    end
  elsif File.extname(arg).eql? ".txt"
    audio_files_in_txt = File.read(arg).split("\n")
    audio_files_in_txt.each do |audio_filename|
      audio_filename = File.expand_path(audio_filename)
      if File.exist?(audio_filename)
        files_to_process << audio_filename
      else
        STDERR.puts "Could not find: #{audio_filename}"
      end
    end
  elsif File.exist? arg
    files_to_process << arg
  else
    STDERR.puts "Could not find: #{arg}"
  end
  files_to_process
end

def with_converted_audio(audio_filename_escaped)
  tempfile = Tempfile.new(["audio_#{rand(20000)}", '.raw'])
  convert_command = AUDIO_CONVERT_COMMAND
  convert_command = convert_command.gsub("__input__",audio_filename_escaped)
  convert_command = convert_command.gsub("__output__",tempfile.path)
  system convert_command

  yield tempfile

  #remove the temp file afer use
  tempfile.close
  tempfile.unlink
end

def microphone
  argument = ""
  puts "#{MIC_INPUT} | #{EXECUTABLE_LOCATION}"
  #Open3.pipeline(MIC_INPUT, [EXECUTABLE_LOCATION])
end

def wait_for_clap
  system "#{MIC_INPUT} | #{EXECUTABLE_LOCATION} on clap"
end

if ARGV.size == 0
  microphone
elsif (ARGV.size >= 1 and ARGV[0] == "onclap" )
  command = ARGV[1]
  while true
    wait_for_clap
    puts "Executing: " + command
    system(command)
  end
else
  audio_files = Array.new
  ARGV.each do |audio_argument|
    audio_files = audio_file_list(audio_argument,audio_files)
  end

  audio_files.each do |audio_file|
    with_converted_audio(audio_file) do |tempfile|
        system " #{tempfile.path}"
    end
end
end


