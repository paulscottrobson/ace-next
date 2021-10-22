# *****************************************************************************
# *****************************************************************************
#
#		Name:		keyboard.rb
#		Author:		Paul Robson (paul@robsons.org.uk)
#		Date:		22nd October 2021
#		Reviewed: 	No
#		Purpose:	Keyboard mapper include generator.
#
# *****************************************************************************
# *****************************************************************************


# *****************************************************************************
#
# 						Class describing keyboard changes
#
# *****************************************************************************

class KeyboardMapping_UK
	#
	# 		UK keyboard mapping. First is PC keyboard key, 
	# 		There are two characters, one of which is the unshifted character
	# 		the other the "symbolled" character on the VZ you want
	#
	# 		Oddly, as it's British, Shift-2 is @ as on US keyboards. This is fixed here.
	# 		But it's unfixable if you want.
	#
	def get_keyboard_mapping
		{
			"2" => "2\"", 			# American users may want to swap these two round. 
			"@" => "'@", 			# @ is internal code for the single quote key

			"6" => "6^", 			# Fix the top row. Lost ' and @
			"7" => "7&",
			"8" => "8*",
			"9" => "9(",
			"0" => "0)",
			"-" => "-_",
			"=" => "=+",

			"[" => "[{",
			"]" => "]}",
			";" => ";:",
			"#" => "##",

			"," => ",<",
			"." => ".>",
			"/" => "/?",

			"\\" => "\\\\",

			"UP" => "6",			# Named keys use Ctrl+character not shift : SHIFT symbol, CTRL shift on Ace
			"DOWN" => "7",
			"LEFT" => "5",
			"RIGHT" => "8",
			"BACKSPACE" => "0",
 		}
	end
end 

# *****************************************************************************
#
# 						Generate checking code.
#
# *****************************************************************************

class MapperFactory
	def initialize(mapping)
		@mapping = mapping.get_keyboard_mapping

		@unshift_keys = [
			"  ZXC",
			"ASDFG",
			"QWERT",
			"12345",
			"09876",
			"POIUY",
			" LKJH",
			" MNBV"
		]

		@symbol_keys = [
			"  : ?",
			"~|\\{}",
			"QWE<>",
			"!@#$%",
			"_)(\'&",
			"\";I[]",
			" =+-^",
			" .,*/"
		]

	end
	#
	# 		Get key for a connection.
	#
	def get_from_matrix(key,matrix,group)
		(0..7).each do |row|
			p = matrix[row].index(key.upcase)			
			return [ 0xFFFF ^ (1 << row), 4-p, group ] if p 
		end
		nil
	end
	#
	# 		Export exports the code to fix up non ACE keys
	#
	def export_include(include_file)
		h = open(include_file,"w")
		h.write("/* Automatically generated */\n")
		@mapping.each do |key,output| 
			is_alpha = key.match(/^[A-Z]+$/)
			key_name = is_alpha ? "GFXKEY_#{key}" : "'"+key+"'"
			key_name = "\'\\\\\'" if key == "\\"
			exec_code = is_alpha ? ctrl_key_code(output) : get_key_code(key,output)
			exec_code = exec_code.collect { |a| "0x"+a.to_s(16) }.join(",")
			h.write("if (HWXIsKeyPressed(#{key_name})) v = HWForceKey(addr,v,#{exec_code});\n")
		end
		h.close
	end
	#
	# 		Get standard result
	#
	def get_key_code(key,output)
		kc = []
		output.each_char do |c| 
			mat_pos = get_from_matrix(c,@unshift_keys,' ')
			is_unshift = mat_pos 
			mat_pos = get_from_matrix(c,@symbol_keys,'S') unless mat_pos
			raise "Can't find #{c}" unless mat_pos		
			kc.append(convert_key_data(mat_pos))
		end
		kc+[0]
	end 
	#
	# 		Get Ctrl key only result
	#
	def ctrl_key_code(output)
		p = get_from_matrix(output,@unshift_keys,'C')
		raise "Unknown character #{p}" unless p
		[convert_key_data(p),0,0]
	end
	#
	#  		Convert a matrix position to an low address mask and bit mask
	#
	def convert_key_data(mat_pos)
		n = (((mat_pos[0] & 0xFF) << 8) ^ 0xFF00)+((0x10 >> (mat_pos[1] & 0x0F)) ^ 0x00)
		n += 0x10000 if mat_pos[2] == 'S'
		n += 0x20000 if mat_pos[2] == 'C'
		n
	end
end 

if __FILE__ == $0 
	MapperFactory.new(KeyboardMapping_UK.new).export_include("_keyboard_fix.h")
end