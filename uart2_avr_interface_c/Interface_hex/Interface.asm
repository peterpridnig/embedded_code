;Interface.asm, Input/Output mit 9600 Baud

	.include "tn13def.inc"

	.def   A      = r16
	.def   Delay  = r17
	.def   Count  = r18
	.def   Kom    = r19
	.def   B      = r20
	.def   C      = r21
	.def   D      = r22
	.def   Count2 = r23
	.def   EEadr  = r24
	.def   EEmode = r25

	 ;Port B
	.equ   TXD    = 1
	.equ   RXD    = 2
	.equ   LED    = 4


	rjmp   Anfang
Anfang:
	sbi    ddrb,TXD		;Datenrichtung TXD
	sbi    portb,TXD
	
	rcall	AdcInit
	rcall	ADCrd
	rjmp	OscKorr		;skip OsccalSet
	
OsccalSet:
	ldi	A,74		;default osccal, necessary for unfused device! smaller=> faster
	out	osccal,A

OscKorr:
	rcall  OscKorrektur	;compare osccal with EEPROM and ajdust if necessary
	rjmp   Schleife

	sbi    ddrb,LED	
myLoop1:
	rcall RdCOM		; MIRROR TEST
	rcall WrCOM
myLoop1End:	
	rjmp  myLoop1
	
myLoop:
	ldi    A,100		; write 100 TEST
	rcall  WrCOM 
myLoopEnd:	
	rjmp  myLoop

	
Schleife:
	rcall  RdCOM		;Interface 
	mov    Kom,A
k1:   	cpi    Kom,1
	brne   K16
	ldi    A,100
	rcall  WrCOM 
k16:    cpi    Kom,16
	brne   K17
	rcall  RdCOM
	  andi   A,0b00011001    
	out    portb,A
k17:      cpi    Kom,17
	brne   K18
	rcall  RdCOM
	andi   A,0b00011011  
	ori    A,0b00000010
	out    ddrb,A
k18:      cpi    Kom,18
	brne   K32
	ldi    A,0        ;PWM initialisieren
	out    OCR0A, A 
	ldi    A,0x83     
	out    TCCR0A, A
	ldi    A,0x02
	out    TCCR0B, A
k32:      cpi    Kom,32
	brne   K48
	in     A,pinb
	rcall  WrCOM
k48:      cpi    Kom,48
	brne   K49
	ldi    A,2
	rcall  AD8Bit
	rcall  WrCOM
k49:      cpi    Kom,49
	brne   K56
	ldi    A,3
	rcall  AD8Bit
	rcall  WrCOM

k56:    cpi    Kom,56
	brne   K57
	ldi    A,2
	rcall  ADCrd
	rcall  WrCOM
	mov    A,B
	rcall  WrCOM

k57:      cpi    Kom,57
	brne   K64
	ldi    A,3
	rcall  ADCrd
      rcall  WrCOM
	mov    A,B
	rcall  WrCOM
k64:  cpi    Kom,64
	brne   K100
	rcall  RdCOM
	out    OCR0A,A 

k100: cpi    Kom,100 ;Oszi 1 Kanal
	brne   K101
Oszi: ldi    XL,96
	ldi    XH,0
	ldi    Count2,61
O100: rcall  ADCrd8BitB4
	st     X+,A
	dec    Count2
	brne   O100      ;61 mal

	ldi   XL,96
      ldi   XH,0
	ldi   Count2,61
O101:
	Ld    A,X+
	rcall WrCOM
	dec   Count2
	brne  O101      ;61 mal

k101: cpi   Kom,101   ;Oszi 2 Kanal
	brne  K250
Oszi2:ldi   XL,96
	Ldi   XH,0
	Ldi   Count2,31
O102: rcall ADCrd8BitB4
	St    X+,A
	rcall	ADCrd8BitB3
      st	X+,A
	dec   Count2
      brne  O102     ;31 mal
	ldi   XL,96
	ldi   XH,0
	ldi   Count2,62
O103:	
	Ld    A,X+
	rcall WrCOM
	dec   Count2
	brne  O103	 ;31 mal

k250: cpi   Kom,250
	brne  K251
	rcall RdRXD
	rcall WrCOM

k251: 	cpi   Kom,251
	brne  K252
	in A, osccal
	rcall WrCOM

k252: 	cpi   Kom,252
	brne  K253
	ldi   EEadr,63
	rcall RdEE
	rcall WrCOM

k253: 	cpi   Kom,253
	brne  K254
	rcall RdCOM
	ldi   EEadr,63
	rcall WrEE

k254: cpi   Kom,254
	brne  K255
	rcall RdCOM
	out   osccal,A


k255: cpi   Kom,255  ;RC-Osc. kalibireren
	brne  SchleifenEnde
	rcall Cal	;20 x Byte 0

SchleifenEnde:
	rjmp  Schleife

OscKorrektur:	 ;OSCCAL in EEPROM(63) 
	ldi	EEadr,63
	rcall	RdEE
	in	B, osccal
	sub	B,A
	cpi	B,5	;Abweichung <5?
	brlo	OscCopy
	cpi	B,252	;Abweichung >-5?
	brsh	OscCopy
	ret
OscCopy:
	rcall  WrCOM
	ldi	EEadr,63
	rcall  RdEE
	out	osccal,A
	ret

Cal: 
	in	 B,osccal
	ldi	D, 10	;maximale Abweichung
	subi   B,10
	ldi	Count2,20
C1:
	out	osccal, B
	rcall  RdRXD	 ;Zeit messen
	subi   A,225	 ;Byte 0: 225 
	cpi	A,128
	brlo   C1b
	com	A
C1b:
	cp	 A,D
	brsh   C3
	rcall  Hit
C2:
	cpi	A,3
	brsh   C3
	rcall  Hit
C3: 
	rcall  WrCOM
	inc	B
	dec	Count2
	brne   C1
	subi   B,10	;alter Wert in osccal
	mov	A,C	 ;neuer Wert
	out	osccal,A
	ldi	EEadr,63
	rcall  WrEE
	rcall  WrCOM
	mov	A,B
	rcall  WrCOM
	ret

Hit: 
	mov   C,B	;besserer Osccal-Wert in C
	inc   A
	mov   D,A	;kleinerer Grenzwert in D
	dec   A
	ret

RdRXD:
	sbic  PINB,RXD  ;9,6 kBaud, 25/Bit; was sbis
	rjmp  RdRXD
	ldi   A,0  
RXD0: 	inc   A		 ;1
	nop			;1
	sbis  PINB,RXD  ;2; was sbic
	rjmp  RXD0	;1, 5 Takte
	ret

AD8Bit:
	out   ADMUX,A
	sbi   ADMUX,ADLAR   ;Left adjust
	sbi   ADCSRA,ADSC   ;Wandlung starten
AD8rdy:
	sbic  ADCSRA,ADSC   
	rjmp  AD8rdy
	sbi   ADCSRA,ADSC
AD8rdyb:
	sbic  ADCSRA,ADSC   
	rjmp  AD8rdyb
	in	A,ADCH
	ret
	
ADCrd:	
	out   ADMUX,A
	sbi   ADCSRA,ADSC   ;Wandlung starten
AD10rdy:
	sbic  ADCSRA,ADSC   
	rjmp  AD10rdy
	sbi   ADCSRA,ADSC
AD10rdyb:
	sbic  ADCSRA,ADSC   
	rjmp  AD10rdyb
	in	B,ADCL
	in	A,ADCH
	ret

AdcInit: 
	ldi   A,3		 ;Clock / 4 
	out   ADCSRA,A	
	sbi   ADCSRA,ADEN ;AD einschalten 
	ret

ADCrd8BitB4:			;8 Bit ADC2
	ldi   A, 34 
	out   ADMUX,A
	sbi   ADCSRA,ADSC   
AD4rdy:
	sbic  ADCSRA,ADSC   
	rjmp  AD4rdy
	sbi   ADCSRA,ADSC
AD4rdyb:
	sbic  ADCSRA,ADSC   
	rjmp  AD4rdyb
	in	A,ADCH
	ret

ADCrd8BitB3:		 ;8 Bit ADC3
	ldi   A, 35 
	out   ADMUX,A
	sbi   ADCSRA,ADSC   
AD3rdy:
	sbic  ADCSRA,ADSC   
	rjmp  AD3rdy
	sbi   ADCSRA,ADSC
AD3rdyb:
	sbic  ADCSRA,ADSC   
	rjmp  AD3rdyb
	in	A,ADCH
	ret


RdCOM:	sbic  pinb,RXD		; was sbis; sbic = skip if cleared => wait for falling edge
	rjmp  RdCOM
	ldi   Delay,58  	; 1.5bit
D1:	 dec   Delay
	brne  D1
	ldi   A,0
	ldi   Count,8
L1:	 lsr   A
	sbic  pinb,RXD		; was sbic pinb,RXD
	ori   A,128		; was ori A,128
	ldi   Delay, 38  
D2:	 dec   Delay
	brne  D2
	dec   Count
	brne  L1
	ldi   Delay, 38
D3:	 dec   Delay
	brne  D3		;	com   A 
	ret

RdCOM2:	sbic  pinb,RXD		; was sbis; sbic = skip if cleared => wait for falling edge
	rjmp  RdCOM2
	ldi   Delay,58  	; 1.5bit
D12:	dec   Delay
	brne  D12
	ldi   A,0
	ldi   Count,8
L12:	lsr   A
	sbic  pinb,RXD		; was sbic pinb,RXD
	ori   A,128		; was ori A,128
	ldi   Delay, 38  	; was 38
	sbi   portb,LED		; set LED	
D22:	dec   Delay
	brne  D22
	cbi   portb,LED		; clear LED	
	dec   Count
	brne  L12
	ldi   Delay, 38
D32:	dec   Delay
		brne  D32		; com   A
		ret
	
WrCOM:  cbi   portb,TXD  ;Senden INVERTED w.r.t. original code!
	ldi   Delay,38 
D4:	dec   Delay
	brne  D4
	ldi   Count,8
L2:	sbrc  A,0
	rjmp  OFF
	rjmp  ONt
ONt:	cbi   portb,TXD		; inv
	rjmp  BitD
OFF:	sbi   portb,TXD		; inv
	rjmp  BitD
BitD:   ldi   Delay,38   	; was 38
D5:	dec   Delay
	brne  D5
	lsr   A
	dec   Count
	brne  L2
	cbi   PORTB,TXD		; was sbi
	ldi   Delay,38  
D6:	dec   Delay
	brne  D6
	sbi   PORTB,TXD		;
	ret
	
RdEE:   sbic  EECR,EEWE	
	rjmp  RdEE
	out   EEAR,EEadr
	sbi   EECR,EERE
	in	A,EEDR
	ret

WrEE:   sbic  EECR,EEWE
	rjmp  WrEE
	ldi   EEmode,0
	out   EECR,EEmode	
	out   EEARL,EEadr
	out   EEDR,A
	sbi   EECR,EEMPE	
	sbi   EECR,EEPE	
	ret

