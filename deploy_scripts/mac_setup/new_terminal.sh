#!/bin/bash

# I got the iTerm2 code from a Gist on GitHub. I'm not finding the one
# I originally used, but it was inspired by this: https://gist.github.com/bobthecow/757788

# OSX only
[ `uname -s` != "Darwin" ] && echo "Must be MacOS" && exit -1

# I got this from a GitHub gist but haven't been able to get it to work in iTerm2 yet...
function iterm () {
    local cdto="$1"
    local cmd="$2"

    osascript &>/dev/null <<EOF
        tell application "iTerm"
            activate
            create window with default profile
            tell current session of current window
                write text "cd \"$cdto\""
                write text "$cmd"
            end tell
        end tell
EOF
}

# This doesn't work.
function apple_terminal () {
    local cdto="$1"
    local cmd="$2"

    echo "In Apple Terminal... $cdto $cmd"
    osascript <<EOF
        tell application "Terminal"
            activate
            tell application \"System Events\" to keystroke \"n\" using command down
            repeat while contents of selected tab of window 1 starts with linefeed
                delay 0.01
            end repeat
            write text "cd \"$cdto\"" in window 1
            write text "$cmd" in window 1
        end tell
EOF
}

pwd=`pwd`
command="$@"

# TODO: add AquaTerm?
case $TERM_PROGRAM in

    Apple_Terminal | Terminal.app | Terminal)
        open -a "Terminal.app" "$@"
        #apple_terminal "$pwd" "$command"
        ;;
    iTerm | iTerm.app | iTerm2)
        iterm "$pwd" "$command"
        ;;
    *)
        echo "Terminal program $TERM_PROGRAM is unknown... Not running $@"
esac
