# frozen_string_literal: true

require 'fileutils'

ROOT = File.expand_path(File.dirname($PROGRAM_NAME))
BUILD = 'src/build'
SERVICE = 'medicine-box'
FileUtils.chdir(ROOT)
FileUtils.mkdir_p(BUILD)
FileUtils.chdir(BUILD)
`cmake .. && make`
f = "#{ROOT}/#{SERVICE}.service"
`sudo rm #{f}` if File.exist? f
`sudo ln -s #{f} /etc/systemd/system`
`sudo systemctl enable #{SERVICE}`
`sudo systemctl restart #{SERVICE}`
