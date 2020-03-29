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
if File.exist? f
  `sudo systemctl disable #{SERVICE}`
  `sudo rm #{f}`
end
`sudo ln -s #{f} /etc/systemd/system`
`sudo systemctl enable #{SERVICE}`
`sudo systemctl restart #{SERVICE}`
