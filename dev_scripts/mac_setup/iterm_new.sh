#!/bin/bash

# I got this from a Gist on GitHub.

#
# Open new iTerm window from the command line
#
# Usage:
#     iterm_new                   Opens the current directory in a new iTerm window
#     iterm_new [PATH]            Open PATH in a new iTerm window
#     iterm_new [CMD]             Open a new iTerm window and execute CMD
#     iterm_new [PATH] [CMD] ...  You can prob'ly guess
#
# Example:
#     iterm_new ~/Code/HelloWorld ./setup.sh
#
# References:
#     iTerm AppleScript Examples:
#     https://gitlab.com/gnachman/iterm2/wikis/Applescript
# 
# Credit:
#     Inspired by tab.bash by @bobthecow
#     link: https://gist.github.com/bobthecow/757788
#

# OSX only
[ `uname -s` != "Darwin" ] && return

function iterm () {
    local cmd=""
    local wd="$PWD"
    local args="$@"

    if [ -d "$1" ]; then
        wd="$1"
        args="${@:2}"
    fi

    if [ -n "$args" ]; then
        # echo $args
        cmd="; $args"
    fi

    osascript &>/dev/null <<EOF
        tell application "iTerm"
            activate
            set term to (make new terminal)
            tell term
                launch session "Default Session"
                tell the last session
                    delay 1
                    write text "cd $wd$cmd"
                end
            end
        end tell
EOF
}
iterm $@
