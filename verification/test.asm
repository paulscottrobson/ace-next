	.org 0
	jp start
	.org $100
regstore:
	ds 32
start:
	ld sp,$F000
	call setupexit
	ld a,$44
	ld ($0000),a
	ld a,$50
	ld ($a169),a
setupmemory:
	ld hl,$0000
	push hl
	pop af
	ld bc,$0000
	ld de,$0000
	ld hl,$0000
	ex af,af'
	exx
	ld hl,$0200
	push hl
	pop af
	ld bc,$cf98
	ld de,$90d8
	ld hl,$a169
	ld ix,$0000
	ld iy,$0000
	ld sp,$0000
	jp $0000
setupexit:
	ld a,$C3
	ld ($0001),a
	ld hl,$400
	ld ($0002),hl
	ret
	.org $400
	ld (regstore+8*2),ix
	ld (regstore+9*2),iy
	ld (regstore+10*2),sp
	ld sp,$F000
	ld (regstore+1*2),bc
	ld (regstore+2*2),de
	ld (regstore+3*2),hl
	push af
	pop hl
	ld (regstore+0*2),hl
	ex af,af'
	exx
	ld (regstore+5*2),bc
	ld (regstore+6*2),de
	ld (regstore+7*2),hl
	push af
	pop hl
	ld (regstore+4*2),hl
	ld a,(correct+0)
	and a,$D7
	ld (correct+0),a
	ld a,(correct+8)
	and a,$D7
	ld (correct+8),a
	ld hl,regstore
	ld de,correct
	ld b,11*2
checkloop:
	ld a,(de)
	cp (hl)
	jr z,$+3
	halt
	inc de
	inc hl
	djnz checkloop
	jp checkmemory
	.org $600
correct:
	.dw $0200
	.dw $a198
	.dw $90d8
	.dw $a169
	.dw $0000
	.dw $0000
	.dw $0000
	.dw $0000
	.dw $0000
	.dw $0000
	.dw $0000
	.dw $0001
checkmemory:
	jp $FFFF