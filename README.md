<div align="center">

# marklinctl

command interface for marklin train controller

</div>

## Building

If you need to override some variables like `XDIR` in the make file, you can
copy `config.mk.example` and make any modifications:
```sh
cp config.mk.example config.mk
```

```sh
make
```

This should produce marklinctl.elf and marklinctl.img. You can inspect the .elf file
(e.g., using readelf) to understand the structure of your compiled code. The
.img file is a memory image of your program (generated from the .elf file),
which can be deployed to the RPi and run. See the course web page for
deployment instructions.

Once the image has been uploaded to the CS452 TFTP server, you can fetch the
image and run it with the following on the RPi.

```sh
dhcp
tftpboot 0x200000 129.97.167.60:images/<user>.img
go 0x200000
```

## Commands


`tr <train number> <speed>`
- Controls the speed of a given train, where `<train number>` is between 1 and
  80, and speed is between 0 and 14

`rv <train number>`
- Reverses the direction a given train is going

`sw <switch number> <switch direction>`
- Sets the state of a switch. `<switch direction>` must be "S" (for straight)
  or "C" (for curved)

`light <train number> <on|off>`
- Toggle the lights on the train on and off

`go`
- Turns on the marklin

`stop`
- Turns off the marklin

`q`
- Quits the program


## Timing

Through extensive measurement of the main loop, it has been found that the
maximum runtime of a single interation is between 7ms and 10ms. This is still
an order of magnitude away from interfering with our timer, which runs every
100 ms.

The program takes between 150ms and 300ms to request and sensor dump and
retrieve all bytes.

## Program Structure

### Main Loop

The main loop consists of the following operations:
- The current timer value is fetched
- If more than 100 milliseconds has passed since last clock update, update the
  time in the UI
- If sensor query timer value reached, send request to Marklin to dump state of
  all switches
- If read timer value reached, check if Marklin has sent us a sensor byte
- Poll for a character from the terminal
- Handle the character accordingly. If the character is alphanumeric, append it
  to the command buffer. If the character is enter, pass the entire command
  buffer to the parser and execute command accordingly
- Poll to see if train reverse and start timers have expired, if so issue
  commands accordingly. This part is simply used to insert a delay between when
  a train is told to stop, to reverse and to start again
- Render the command prompt if required
- If write timer value reached, write a byte to marklin

## Data Structures / Algorithms

### String

A custom utility string struct was created to facilitate dynamic string
operations such as pushing and popping characters. The struct consists of a
constant size char buffer and a counter for number of characters. This is not
the most efficient of implementations, especially with the lack of the heap. In
the future this should definitely be replaced.

### CBuf

A circular buffer was also implemented for use as a output FIFO. Since at most
one byte is written to the Marklin per iteration, we use this as a queue. CBuf
was implemented as a constant size byte array with a front and a back pointer.

### Train State Table

When reversing trains, we must accelerate the train back to the original speed.
Since there is no way to query for the train speed, we use a table to keep
track of each trains speed. The table is updated only by the `tr` command.

## Bugs / Caveats

- Sensor names are incorrect on track B
- Commands issued with invalid parameters may result in unexpected behavior
- If the speed of a train has not been set using `tr` prior to a `rv` call, the
  train will halt instead of reversing at the same speed
- In some rare cases the sensor activations don't show up in the sensor pane.
  If so, upon quitting the program and starting it back up it should all show
  up

## TODO

- [x] Handle switches in middle of track
- [ ] Initalize all trains to speed zero
- [ ] add support for functions like lights and horn
- [ ] validation for commands
- [ ] test build on waterloo linux server

