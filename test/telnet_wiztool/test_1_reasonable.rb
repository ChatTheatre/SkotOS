#!/usr/bin/env ruby

require_relative "test_helper.rb"

class TestBoot < Minitest::Test
  def setup
    @client = SkotOSClient.new
    # When running Wafer, any account name and password should work here.
    # By default, Wafer is set up so that any unknown account name works
    # and is an admin account.
    @client.login_as("skott", "any_password_should_work")
    @client.command("cd /")
  end

  def teardown
    @client.close
  end

  def test_basic_connection
    # Check that the output contains a usr directory
    assert @client.command("ls").include?("usr"), "No /usr directory found in SkotOS root!"
  end

  # For now we're just testing that the metrics object exists.
  # Later we'll chase down errors and looping conditions, and then
  # assert that none happen on boot.
  def test_metrics
    metrics = @client.mapping_from_dgd_code '"/usr/System/sys/syslogd"->query_syslogd_metrics()'
    assert metrics["uncaught_exceptions"], "Syslogd metrics contains no uncaught_exceptions key!"
  end
end
