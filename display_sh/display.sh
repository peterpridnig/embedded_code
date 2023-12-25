#!/bin/bash
# ----------------
# --- script
# ----------------

if [[ $# -eq 0 ]] ; then
    echo Usage
    echo 'display clear|text <text>|demo'
    echo ' <text> grammar:'
    echo '  <clr>DISPLAYTEXT            |'
    echo '  <pos line,col>DISPLAYTEXT   |'
    echo '  <scr left|right>IGNOREDTEXT |'
    echo '  DISPLAYTEXT'
    echo
    exit 1
fi

case "$1" in
    clear) echo '<clr>' > /dev/hd44780
	 ;;
    text) echo $2 > /dev/hd44780
	   ;;
    demo) while true
	  do
	  echo '<clr>' > /dev/hd44780
	  echo '<pos 1,3>Peter Pridnig' > /dev/hd44780
	  sleep 0.5
	  echo '<pos 2,4>16.12.1973' > /dev/hd44780
	  sleep 0.5
	  for i in `seq 1 3`
	  do
	      echo '<scr left>' > /dev/hd44780
	      sleep 0.5
	  done
	  for i in `seq 1 6`
	  do
	      echo '<scr right>' > /dev/hd44780
	      sleep 0.5
	  done
	  for i in `seq 1 3`
	  do
	      echo '<scr left>' > /dev/hd44780
	      sleep 0.5
	  done
	  for i in `seq 1 18`
	  do
	      echo "<pos 0,$i>-" > /dev/hd44780
	      echo "<pos 3,$i>-" > /dev/hd44780
	      sleep 0.1
	  done
	  for i in `seq 1 2`
	  do
	      echo "<pos $i,0>|" > /dev/hd44780
	      echo "<pos $i,19>|" > /dev/hd44780 
	      sleep 0.1
	  done
          echo "<pos 0,0>*"  > /dev/hd44780
          echo "<pos 0,19>*" > /dev/hd44780
          echo "<pos 3,0>*"  > /dev/hd44780
          echo "<pos 3,19>*" > /dev/hd44780
	  sleep 5
	  done
	;;
esac



