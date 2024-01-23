#!/bin/sh

xclock &
xeyes &
sleep 2 &
# sleep 2 && xterm &

exec ./main
