require "minitest"
require "minitest/autorun"

# Ruby has built-in telnet support. Weird but convenient.
# Eventually I assume it will stop being built in and you'll need to
# use this gem: https://github.com/ruby/net-telnet
require "net/telnet"
# Conveniently, the gem also lets you read the code for
# Ruby's telnet implementation in case you need to use it in
# a more interesting way: https://github.com/ruby/net-telnet/blob/master/lib/net/telnet.rb

class SkotOSClient
  def initialize
    @telnet = Net::Telnet::new("Host" => "localhost", "Port" => 10098, "Timeout" => 10, "FailEOF" => true, "Prompt" => /> /)
  end

  def login_as(user, password)
    @telnet.waitfor(/What is your name: /)
    @telnet.write(user + "\n")
    @telnet.waitfor(/Password:/)
    @telnet.write(password + "\n")
    @telnet.waitfor(/> /)
  end

  def close
    @telnet.close
  end

  def command(text)
    return @telnet.cmd(text).chomp("\n> ")
  end

  def mapping_from_dgd_code(expr)
    output = command("code #{expr}")
    mapping = output.chomp("])").sub(/^\$\d+ = \(\[/, "")
    items = mapping.split(",").map(&:strip).map { |i| k, v = i.split(":", 2); [ k.gsub('"', ""), v.to_i ]}
    Hash[items]
  end
end
