#
# 		Split the source files tests.in/tests.expected up and recombine them as single test files.
#

class TestSource
	def initialize(fileName)
		@elements = {}
		src = open(fileName).collect { |a| a.strip+"|" }
		src.join("").split("||").each do |e|
			key = e.split("|")[0]
			@elements[key.downcase.strip] = e.split("|")[1..]
		end
	end

	def keys
		@elements.keys 
	end

	def get(key)
		@elements[key.downcase.strip]
	end
end

class TestElement
	def initialize(id,tin,tex)
		@id = id
		@tin = tin.collect { |a| a.strip }.select { |a| a != "" }
		@tex = tex.collect { |a| a.strip }.select { |a| a != "" }
		while @tex[0].length != 64 
			@tex = @tex[1..] 
		end
	end

	def create
		s = "IDT:"+@id+"\n"
		s += "INP:" + @tin[0][..-5]+"\n" 									# Input Registers,dump MEMPTR
		s += (2..@tin.length-2).collect { |a| "WRT:"+@tin[a].gsub("-1","") }.join("\n")
		s += "\n"
		s += "OUT:" + @tex[0][..-5]+"\n" 									# Output Registers,dump MEMPTR
		s += (2..@tex.length-1).collect { |a| "CHK:"+@tex[a].gsub("-1","") }.join("\n")
		s += "\n"
		s
	end
end

s_in = TestSource.new("source/tests.in")
s_ex = TestSource.new("source/tests.expected")		

s_in.keys.each do |k|
	te = TestElement.new(k,s_in.get(k),s_ex.get(k)).create
	open("tests/#{k}.tst","w").write(te)
end
puts "Generated #{s_in.keys.length} tests."

@reject = [0x76,0xED40,0xDB,0xED48,0xED50,0xED58,0xED60,0xED68,0xED78,0xEDA2,0xEDAA,0xEDB2,0xEDBA,
			0xED57,0xED5F,0xEDA2,0xEDA3,0xEDAA,0xEDAB,0xEDB2,0xEDB3,0xEDBA,0xEDBB,0xFDCB35]
h = open("doall.sh","w")
s_in.keys.select { |a| not(@reject.include?(a.split("_")[0].to_i(16))) }.each { |k| h.write("sh dotest.sh #{k.downcase}\n") }
h.close()
