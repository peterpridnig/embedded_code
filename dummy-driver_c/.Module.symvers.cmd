cmd_/home/peter/mastering_beaglebone/userland/code/dummy-driver_c/Module.symvers := sed 's/\.ko$$/\.o/' /home/peter/mastering_beaglebone/userland/code/dummy-driver_c/modules.order | scripts/mod/modpost -m -a  -o /home/peter/mastering_beaglebone/userland/code/dummy-driver_c/Module.symvers -e -i Module.symvers   -T -