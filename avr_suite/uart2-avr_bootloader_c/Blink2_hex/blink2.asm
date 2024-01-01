;Blink2.asm Blinker mit Unterprogramm

      .include "tn13def.inc"

rjmp Anfang
Anfang:
      ldi   r16,0x18    ;PB4 und PB4
      out   ddrb,r16    ;Datenrichtung
Schleife:
      ldi   r16,8       ;8 = 0x08
      out   portb,r16   ;PB3 = 1, PB4 = 0
      rcall Warten      ;Unterprogrammaufruf
      ldi   r16,16      ;16 = 0x10
      out   portb,r16   ;PB3 = 0, PB4 = 1
      rcall Warten      ;Unterprogrammaufruf
rjmp  Schleife

Warten:	
      Ldi   r16,125      
Warten1:                ;äußere Schleife
      Ldi   r17,250
Warten2:                ;innere Schleife
      dec   r17
      brne  Warten2
      dec   r16
      brne  Warten1 
      ret               ;Rücksprung

