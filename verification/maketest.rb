#
#  		Create assembler code from a test file
#
class TestGenerator
	def initialize(fileName)
		@lines = []
		@src = open(fileName).collect { |a| a.strip }
		prefix
		@src.select { |a| a[0..3] == "WRT:" }.each { |a| writeMem a[4..] }

		add "setupmemory:"
		@src.select { |a| a[0..3] == "INP:" }.each { |a| loadRegisters a[4..] }	

		@src.select { |a| a[0..3] == "OUT:" }.each { |a| saveCheckRegisters a[4..] }	
		add "checkmemory:"
		@src.select { |a| a[0..3] == "CHK:" }.each { |a| checkMem a[4..] }
		add "jp $FFFF"
		postfix
	end 

	def add(l)
		@lines.append l[-1] == ':' ? l : "\t"+l 
	end

	def to_s 
		@lines.join "\n"
	end 

	def prefix
		add 	".org 0"
		add 	"jp start"
		add 	".org $100"
		add 	"regstore:"
		add 	"ds 32"
		add 	"start:"
		add 	"ld sp,$F000"
		add 	"call setupexit"
	end 

	def postfix
	end 

	def loadRegisters a 
		loadpairs a[20..38]
		add "ex af,af'"
		add "exx"
		loadpairs a[0..18]
		add "ld ix,$#{a[40..43]}"
		add "ld iy,$#{a[45..48]}"
		add "ld sp,$#{a[50..53]}"
		@pc = a[55..59]
		add "jp $#{@pc}"
	end 

	def saveCheckRegisters a
		@exitpc = a[55..59]
		setupExit @exitpc.to_i(16),"$400"
		add ".org $400"
		add "ld (regstore+8*2),ix"
		add "ld (regstore+9*2),iy"
		add "ld (regstore+10*2),sp"
		add "ld sp,$F000"
		savepairs 0
		add "ex af,af'"
		add "exx"
		savepairs 4

		add "ld a,(correct+0)"
		add "and a,$D7"
		add "ld (correct+0),a"
		add "ld a,(correct+8)"
		add "and a,$D7"
		add "ld (correct+8),a"

		add "ld hl,regstore"
		add "ld de,correct"
		add "ld b,11*2"
		add "checkloop:"
		add "ld a,(de)"
		add "cp (hl)"
		add "jr z,$+3"
		add "halt"
		add "inc de"
		add "inc hl"
		add "djnz checkloop"
		add "jp checkmemory"
		add ".org $600"
		add "correct:"
		a.split(" ").each { |w| add ".dw $#{w}" 	}
	end 

	def setupExit(a,continue)
		add "setupexit:"
		add "ld a,$C3"
		add "ld ($%04x),a" % [a]
		add "ld hl,#{continue}"
		add "ld ($%04x),hl" % [a+1]
		add "ret"
	end 

	def loadpairs(s)
		s = s.split " "
		add "ld hl,$#{s[0]}"
		add "push hl"
		add "pop af"
		add "ld bc,$#{s[1]}"
		add "ld de,$#{s[2]}"
		add "ld hl,$#{s[3]}"
	end 

	def savepairs r 
		add "ld (regstore+#{r+1}*2),bc"
		add "ld (regstore+#{r+2}*2),de"
		add "ld (regstore+#{r+3}*2),hl"
		add "push af"
		add "pop hl"
		add "ld (regstore+#{r+0}*2),hl"
	end

	def writeMem s 
		addr = s[0..3].to_i(16)
		s[4..].strip.split(" ").each do |b|
			add "ld a,$#{b}"
			add "ld ($%04x),a" % [addr]
			addr += 1
		end 
	end 

	def checkMem s 
		addr = s[0..3].to_i(16)
		s[4..].strip.split(" ").each do |b|
			add "ld a,($%04x)" % [addr]
			add "cp $#{b}"
			add "jr z,$+3"
			add "halt"
			addr += 1
		end 
	end 

end

tg = TestGenerator.new("tests/#{ARGV[0]}.tst")
open("test.asm","w").write(tg)