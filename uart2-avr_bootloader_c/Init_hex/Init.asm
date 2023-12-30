; Tiny13

;       Reset=PB5  	1	8	VCC 3,3 V
;	PB3=ADC3	2	7	PB2 = RXD über 100k
;	PB4=ADC2	3	6	PB1 = TXD
;	GND			4	5	PB0, PWM	= Pol ADC2 Umschaltung +6V / +-3,3V


    .include "tn13def.inc"

;RS232, Empfangen und Senden mit 9600 Baud bei 1,2 MHz

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
    .equ   PWM    = 0

    .equ   OSC = 26

.org    $0000
	rjmp $0180

.org 	$0010
     rjmp Anfang

Anfang:
      ldi   r16,0x08    ;PB3=out
      out   ddrb,r16    ;Datenrichtung
Schleife:
      ldi   r16,0x08    
      out   portb,r16   ;PB3 = 1
      rcall WartenA     ;Unterprogrammaufruf
      ldi   r16,0x00     
      out   portb,r16   ;PB3 = 0
      rcall WartenA      ;Unterprogrammaufruf
      rcall WartenA      ;Unterprogrammaufruf	
      ldi   r16,0x08    
      out   portb,r16   ;PB3 = 1
      rcall WartenB     ;Unterprogrammaufruf
      ldi   r16,0x00     
      out   portb,r16   ;PB3 = 0
      rcall WartenB      ;Unterprogrammaufruf
      rcall WartenA      ;Unterprogrammaufruf
      rcall WartenA      ;Unterprogrammaufruf	
      ldi   r16,0x08    
      out   portb,r16   ;PB3 = 1
      rcall WartenC     ;Unterprogrammaufruf
      ldi   r16,0x00     
      out   portb,r16   ;PB3 = 0
      rcall WartenC      ;Unterprogrammaufruf
      rcall WartenA      ;Unterprogrammaufruf
      rcall WartenA      ;Unterprogrammaufruf		
	
rjmp  Schleife

	
WartenA:	
      Ldi   r16,250      
WartenA1:                ;äußere Schleife
      Ldi   r17,250
WartenA2:                ;innere Schleife
      dec   r17
      brne  WartenA2
      dec   r16
      brne  WartenA1 
      ret               ;Rücksprung

	
WartenB:	
      Ldi   r16,125      
WartenB1:                ;äußere Schleife
      Ldi   r17,255
WartenB2:                ;innere Schleife
      dec   r17
      brne  WartenB2
      dec   r16
      brne  WartenB1 
      ret               ;Rücksprung

WartenC:	
      Ldi   r16,63      
WartenC1:                ;äußere Schleife
      Ldi   r17,255
WartenC2:                ;innere Schleife
      dec   r17
      brne  WartenC2
      dec   r16
      brne  WartenC1
      ret               ;Rücksprung

	
	
.org $017E
      rjmp $0010

	
.org 	$0180

      .def	spmcsrval	=r28

RESETboot:  
	cli			;rjmp	OscKorr		;COMMENT OUT = speed adjustment=skip OsccalSet

OsccalSet:
      ldi    A,OSC		;adjust osccal: smaller=>slower
      out    osccal,A

Breakstart:  
      sbis  PINB,RXD		;RXD=1 => start main prog; RXD=0 => start bootloader
      Rjmp  Brkend
      Rjmp  $017E
Brkend:
      Sbis  PINB,RXD		;wait until RXD=1
      Rjmp  Brkend
      sbi   ddrb, TXD
      ldi   A,105
      rcall WrCOMb

BootKom:
      Rcall RdCOMb

K201: mov   Kom,A
      Cpi   Kom,201     ;201, Block schreiben
      Brne  K202
      Rcall write_page

K202: cpi   Kom,202     ;202, Block lesen 
      Brne  K203
      Rcall read_page

K203: cpi   Kom,203	;203, ProgStart
      Brne  K204
      In    A,PORTB
      Rjmp  $017E
K204: rjmp  BootKom



write_page:
      ldi   Count2,16
      rcall RdCOMb      ;Adr Hi,Lo vom Host
      mov   ZH,A
      rcall RdCOMb
      mov   ZL,A
page_erase:
      ldi   spmcsrval,3
      out   SPMCSR, spmcsrval
      spm

wrloop:
      rcall RdCOMb      ;32 Bytes vom Host
      mov   r0,A	;Highbyte zuerst
      rcall RdCOMb
      mov   r1,A
      ldi   spmcsrval,1
      out   SPMCSR, spmcsrval
      spm
      inc   ZL
      inc   ZL
      mov   A,r0
      rcall WrCOMb
      rcall myDelay
      mov   A,r1
      rcall WrCOMb
      dec   Count2     ;16 mal
      brne  wrloop
      subi  ZL,32
      ldi   spmcsrval,5
      out   SPMCSR,spmcsrval
      spm
      ret
	
read_page:
      rcall RdCOMb     ;Adr Hi,Lo vom Host
      mov   ZH,A
      rcall RdCOMb
      mov   ZL,A
      ldi   Count2, 32
rdloop:
      lpm   A, Z+
      rcall WrCOMb
      rcall myDelay
      dec   Count2	
      brne  rdloop
      ret

RdCOMb:	sbic  pinb,RXD		; was sbis; sbic = skip if cleared => wait for falling edge
	rjmp  RdCOMb
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

WrCOMb:  cbi   portb,TXD  ;Senden INVERTED w.r.t. original code!
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
	
myDelay:
      ldi Delay,255
myDelay0:
     dec Delay
     brne myDelay0
     ret	
