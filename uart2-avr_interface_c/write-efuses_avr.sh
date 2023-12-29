#!/bin/bash
# ----------------
# --- script
# ----------------

echo Connect usbasp interface
echo Writing Default EFuses...
avrdude -c usbasp -p t13 -U hfuse:w:0XEB:m -U lfuse:w:0X6A:m -F
echo Done.

