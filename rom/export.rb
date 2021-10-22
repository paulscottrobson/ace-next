# *****************************************************************************
# *****************************************************************************
#
#		Name:		export.rb
#		Author:		Paul Robson (paul@robsons.org.uk)
#		Date:		30th September 2021
#		Reviewed: 	No
#		Purpose:	Export ROM image
#
# *****************************************************************************
# *****************************************************************************

# *****************************************************************************
#
#  					Class encapsulating standard ROM
#
# *****************************************************************************

class StandardROM
	def initialize(binary_file)
		@data = open(binary_file,"rb").each_byte.collect { |a| a }
	end 

	def export_include(include_file)
		bytes = @data.collect { |b| b.to_s }.join(",")
		open(include_file,"w").write(bytes+"\n")
		self
	end

	def export_binary(binary_file) 
		h = open(binary_file,"wb")
		@data.each { |b| h.write(b.chr) }
		self 
	end
end

# *****************************************************************************
#
# 		Character ROM. Uploaded on Reset, it frees up ROM for tinkering
#
# *****************************************************************************

class CharacterROM < StandardROM
	def initialize(binary_file)
		super
		@data = @data[0x2C00..0x2FFF]
	end
end

# *****************************************************************************
#
# 						Monitor ROM with patches
#
# *****************************************************************************

class MonitorROM < StandardROM
	def initialize(binary_file)
		super
		#
		# 	Disable the Character RAM initialisation. We put this in a ROM and load
		# 	at reset. This allows it to be changed, perhaps, and also frees up about 1k of
		#  	ROM for expansions.
		#
		disable(0x52,0x8D)
		#
		# 	This instruction loads the font into RAM from internal ROM. Thus I can reuse the
		#  	font ROM if I want to.
		#
		patch(0x52,0xED)
		patch(0x53,0xF0)
		#
		# 	This displays the new prompt.
		#
		patch(0x54,0xED)
		patch(0x55,0xF1)
		#
		# 	Patch in tape load at $18A7 to make it load in from TAP
		#
		patch(0x18A7,0xED)
		patch(0x18A8,0xF2)
		patch(0x18A9,0xC9)
		#
		# 	Patch to stop the tape file name comparison test failing - makes a CP(HL) XOR A
		#  	We aren't using serial storage devices, so this is meaningless. load <no name> is dir now.
		#
		patch(0x1AAA,0xAF)
		#
		#  	Patch to initially acquire the TAP name. This calls EDF3 pseudio operation
		# 	Also loads DE with 0x0019 which is what it replaces
		#
		patch(0x1A76,0xED)
		patch(0x1A77,0xF3)
		patch(0x1A78,0x00)
		#
		# 	Patch the routine that writes out a TAP blokc
		#
		patch(0x1820,0xED)
		patch(0x1821,0xF4)
		patch(0x1822,0xC9)
		#
		# 	Patch the keyboard routine for spooling.
		#
		patch(0x014E,0xED)
		patch(0x014F,0xF5)
		patch(0x0150,0x00)
		#
		# 	Replaces the clock incrementer with a pseudo op setting it directly.
		#
		patch(0x0147,0xED)
		patch(0x0148,0xF6)
		patch(0x0149,0x00)
		patch(0x014A,0x00)
		#
		# 	EDE0 Patch
		#
		patch_ede0
	end
	#
	# 		Patch ROM
	#
	def patch(addr,data)
		@data[addr] = data
	end
	#
	# 		Patch all CALL $084E with ED E0 00 
	#
	# 		These are manually checked just in case by chance the sequence appears elsewhere
	#
	def patch_ede0
		(0..0x1FFC).each do |a|
			if @data[a] == 0xCD and @data[a+1] == 0x4E and @data[a+2] == 0x08
				@data[a] = 0xED
				@data[a+1] = 0xE0
				@data[a+2] = 0x00
			end
		end
	end

	#
	# 		Disable ROM area by filling with NOP
	#
	def disable(from,to) 
		(from..to).each { |a| @data[a] = 0 }
	end
end 

if __FILE__ == $0 
	StandardROM.new("rom.bin").export_include("default_rom.h")
	MonitorROM.new("rom.bin").export_include("fixed_rom.h")
 	CharacterROM.new("allmem.bin").export_include("char_rom.h")

 	StandardROM.new("test.tap").export_include("test_tap.h")	
end

#
# 		Other usage of specific addresses.
#
# 	 	$0008 is used to print via A
# 		$00AD is used after directory listing to restart, executing ABORT.
# 		$1AB6 is the tape fail routine, which does RST 20h ; DB $0A
# 		
# 		Optimisation usage
#
#