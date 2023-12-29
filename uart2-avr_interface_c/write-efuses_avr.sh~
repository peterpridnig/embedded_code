#!/bin/bash
# ----------------
# --- script
# ----------------

echo Connect usbasp interface
echo Reading EFuses...
avrdude -c usbasp -p t13 -U hfuse:r:-:h -U lfuse:r:-:h -F 
echo Done.

