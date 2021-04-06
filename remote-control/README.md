# source codes for remote control
These programs runs on the a remote computer which connects to the same network as the Raspberry pi. The Raspberry Pi sits on the RC car and drives the servo and controls the ESC.

## Software on the PI
The Pi runs only a [pigpiod](http://abyz.me.uk/rpi/pigpio/pigpiod.html) daemon which receives commands through TCP socket. The command packets send by the remote computer. See the source codes in this directory.

## Source Codes
- `send-pigpiod-with-asio.cpp`: Sends packets through TCP socket with asio
- `send-pigpiod-with-pigpiif.cpp`: Sends the commands with the given interface library

