Building (linux.student.cs.uwaterloo.ca)
========================================


```sh
make
```

This should produce iotest.elf and iotest.img.
You can inspect the .elf file (e.g., using readelf) to understand the structure of your compiled code.
The .img file is a memory image of your program (generated from the .elf file), which can be deployed
to the RPi and run.
See the course web page for deployment instructions.

## TODO

- [x] convert commands into a queue as well (currnet bug is that only or sensors and train control works (not both))
- [ ] DR register is used for both reads and writes, right now they happen too close to each other so we are reading the data we just wrote
