# *****************************************************************************
# *****************************************************************************
#
#		Name:		scancodes.rb
#		Author:		Paul Robson (paul@robsons.org.uk)
#		Date:		22nd October 2021
#		Reviewed: 	No
#		Purpose:	Emu format => Scan code mapper.
#
# *****************************************************************************
# *****************************************************************************

scan_to_our = [ 0 ] * 256

"""
0	45	0
1	16	1
2	1E	2
3	26	3
4	25	4
5	2E	5
6	36	6
7	3D	7
8	3E	8
9	46	9
-	4E	-
,	41	,
;	4c	;
.	49	.
[	54	]
]	5b	[
/	4a	/
\\	61	\\
#	5D	#
\`	52	@
=	55	=
A	1C	A
B	32	B
BKSP	66	BACKSPACE
C	21	C
D	23	D
DELETE E071 DELETE
D_ARROW	E072	DOWN
E	24	E
ENTER	5A	RETURN
ESC	76	ESCAPE
F	2B	F
G	34	G
H	33	H
I	43	I
INSERT	E070	INSERT
J	3B	J
K	42	K
L	4B	L
L_ARROW	E06B	LEFT
L_CTRL	14	LCTRL
L_SHFT	12	LSHIFT
M	3A	M
N	31	N
O	44	O
P	4D	P
Q	15	Q
R	2D	R
R_ARROW	E074	RIGHT
R_CTRL	E014	RCTRL
R SHFT	59	RSHIFT
S	1B	S
SPACE	29	 SPACE
T	2C	T
TAB	0D	TAB
U	3C	U
U_ARROW	E075	UP
V	2A	V
W	1D	W
X	22	X
Y	35	Y
Z	1A	Z
""".strip.upcase.split("\n").collect { |a| a.strip }.select { |a| a != "" }.each do |l|
	m = l.match(/^(.*?)\s+([0-9A-F]+)\s+(.*?)\s*$/)
	raise "Bad line #{l}" unless m
	if m[3].length == 1 or m[3] == "SPACE"
		key = "'"+m[3]+"'"
		key = "'\\\\'" if m[3] == '\\'
		key = "' '" if m[3] == "SPACE"
	else
		key = "GFXKEY_"+m[3]
	end
	code = m[2].to_i(16)
	code = (code & 0x7F) + 0x80 if code >= 0x8000
	raise "Duplicate #{code.to_s(16)}" unless scan_to_our[code] == 0
	scan_to_our[code] = key
end
data = scan_to_our.collect { |a| a.to_s }.join(",")
open("_scan_to_gfx.h","w").write(data+"\n")