# SPITEST
pridnig, 6.1.2023

ls /sys/bus/spi

spidev[bus].[chip select]
spidev0.0

## compiled implementation
./spidev_test -D /dev/spidev1.0

##  distributed implementation
spidev_test -D /dev/spidev1.0 -v


## spi-config -h
usage: spi-config options...
  options:
    -d --device=<dev>  use the given spi-dev character device.
    -q --query         print the current configuration.
    -m --mode=[0-3]    use the selected spi mode:
             0: low idle level, sample on leading edge,
             1: low idle level, sample on trailing edge,
             2: high idle level, sample on leading edge,
             3: high idle level, sample on trailing edge.
    -l --lsb={0,1}     LSB first (1) or MSB first (0).
    -b --bits=[7...]   bits per word.
    -s --speed=<int>   set the speed in Hz.
    -r --spirdy={0,1}  consider SPI_RDY signal (1) or ignore it (0).
    -w --wait          block keeping the file descriptor open to avoid speed reset.
    -h --help          this screen.
    -v --version       display the version number.



