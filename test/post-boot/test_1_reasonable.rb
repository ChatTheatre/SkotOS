#!/usr/bin/env ruby -w

# Ruby has built-in telnet support. Weird but convenient.
# Eventually I assume it will stop being built in and you'll need to
# use this gem: https://github.com/ruby/net-telnet
require "net/telnet"
# Conveniently, the gem also lets you read the (simple) code for
# Ruby's telnet implementation in case you need to use it in
# a more interesting way: https://github.com/ruby/net-telnet/blob/master/lib/net/telnet.rb

skotos = Net::Telnet::new("Host" => "localhost", "Port" => 10098, "Timeout" => 10, "FailEOF" => true, "Prompt" => /> /)

skotos.waitfor(/What is your name: /)
skotos.write("skott\n")
skotos.waitfor(/Password:/) { |c| puts c.inspect }
skotos.write("any_password_should_work\n")
skotos.waitfor(/> /)

skotos.cmd("cd /")

skotos.close
