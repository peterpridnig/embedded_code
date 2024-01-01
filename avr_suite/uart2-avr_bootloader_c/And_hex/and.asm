;And.asm,  PB1 = PB0 AND PB2

      .include "tn13def.inc"

rjmp Anfang
Anfang:
      sbi   ddrb,1    ;Datenrichtungsbit
Schleife:
      clr   r16
      clr   r17
      sbic  pinb,0    ;b.0 in r16 lesen
      ldi   r16,1
      sbic  pinb,2    ;b.2 in r17 lesen
      ldi   r17,1
      and   r16,r17   ;b.0 AND b.2
      breq  Null      ;springe wenn Ergebnis = 0
      sbi   portb,1   ;1 ausgeben
      rjmp  Schleife
Null:
      cbi   portb,1   ;0 ausgeben
      rjmp  Schleife
