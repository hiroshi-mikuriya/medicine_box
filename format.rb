# frozen_string_literal: true

require 'fileutils'

DIRS = %w[
  src
].freeze
EXTS = %w[c h cpp hpp].freeze

FileUtils.chdir(File.dirname($PROGRAM_NAME))

Dir.glob("{#{DIRS.join(',')}}/**/*.{#{EXTS.join(',')}}").each do |f|
  next if f.include? 'CMakeFiles'

  `clang-format -i #{f}`
end
