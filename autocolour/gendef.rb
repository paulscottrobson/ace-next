# *****************************************************************************
# *****************************************************************************
#
#		Name:		gendef.rb
#		Author:		Paul Robson (paul@robsons.org.uk)
#		Date:		23rd October 2021
#		Reviewed: 	No
#		Purpose:	Generate colouring file from memory.dump and default.dump
#
# *****************************************************************************
# *****************************************************************************

# *****************************************************************************
#
#  					Class encapsulating standard ROM
#
# *****************************************************************************

class Font
	def initialize(binary_file)
		@data = open(binary_file,"rb").each_byte.collect { |a| a }[0x2C00..0x2FFF]
	end 

	def get(char,row)
		@data[char*8+row]
	end

	def get_def(char)
		@data[char*8 .. char*8+7]
	end

end

#
#  		Examines memory dumps of a "normal" run and a "game" run to identify which
# 		characters are redefined, and lists them, with defaults for space, alpha,
#  		digits and everything.
#
# 		The output of this file (autocolour.def) should be renamed appropriately
# 		(aticraid.def) run through the converter and copied in storage.
#
default = Font.new("default.dump")
game = Font.new("memory.dump")

matches = {}
#
# 		Defaults for Space, Digits, Alpha and Everything.
#
matches[0x20] = true  						# Space for background colour
matches[0x30] = true 						# colour for digits 0-9
matches[0x41] = true 						# colour for A-Z and a-z
matches[0x21] = true 						# colour for all others 0x20-0x7F 
#
# 		See what has changed from the default.
#
(0x00..0x7F).each do |c|
	matches[c] = true if default.get_def(c) != game.get_def(c)
end
#
# 		Get the changed and sort them, then generate for each.
#  		Colours are RGB here and converted to GRB
#
h = open("autocolour.def","w")
h.write("; *************************************************************\n")
h.write(";\n;\tAutocolour file for\n;\n")
h.write(";\tColours are 0:Black 1:Blue 2:Red 3:Purple 4:Green 5:Cyan\n")
h.write(";\t6:Yellow 7:White ; Background in upper nibble, foreground in lower\n;\n")
h.write("; *************************************************************\n")
matches.keys.sort.each do |c|
	h.write("\n$#{c.to_s(16)}  00 ; '#{c.chr}'\n")
	game.get_def(c).each do |r|
		s = ("00000000"+r.to_s(2))[-8..].gsub("0",".").gsub("1","X")
		h.write(";\t#{s}\n")
	end
end
h.close
