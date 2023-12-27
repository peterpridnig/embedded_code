# Copyright (C) 2023, Peter Pridnig
#
# make ASM=avra ASMLIB=/usr/share/avra

PROGRAM=Init

$(PROGRAM): $(PROGRAM).asm
	$(ASM) -I $(ASMLIB) $^

install : $(PROGRAM).hex
	scp $(PROGRAM).hex root@10.0.0.117:/root
	cp $(PROGRAM).hex ../../bin

clean:
	rm -f $(PROGRAM)
	rm *.cof *.hex *.obj
	rm *~

