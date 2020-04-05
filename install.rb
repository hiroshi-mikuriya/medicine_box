# frozen_string_literal: true

require 'fileutils'

ROOT = File.expand_path(File.dirname($PROGRAM_NAME))
BUILD = './build'
SERVICE = 'medicine-box'
FileUtils.chdir(ROOT)
FileUtils.mkdir_p(BUILD) unless File.exist? BUILD
FileUtils.chdir(BUILD)
`cmake #{ROOT}/src && make`
from = "#{ROOT}/#{SERVICE}.service"
to = "/etc/systemd/system/#{SERVICE}.service"
`sudo systemctl disable #{SERVICE}` if File.exist? to
`sudo ln -s #{from} /etc/systemd/system`
`sudo systemctl enable #{SERVICE}`
`sudo systemctl restart #{SERVICE}`
