# 8086 MOV instruction decoder
# https://www.computerenhance.com/p/instruction-decoding-on-the-8086

input_filename = ARGV.fetch(0)

RmFieldNames = [
  %w{ al cl dl bl ah ch dh bh },
  %w{ ax cx dx bx sp bp si di },
]

def decode(opcode_str)
  byte0, byte1 = opcode_str.unpack('CC')
  case
  when byte0 >> 2 == 0b100010
    d = byte0[1]
    w = byte0[0]
    mod = byte1 >> 6 & 3
    reg = byte1 >> 3 & 7
    rm = byte1 & 7
    raise NotImplementedError if d == 1
    raise NotImplementedError if mod != 0b11
    op1 = RmFieldNames.fetch(w).fetch(rm)
    op2 = RmFieldNames.fetch(w).fetch(reg)
    "mov #{op1}, #{op2}"
  else
    raise NotImplementedError
  end
end

input = File.open(input_filename, 'rb')
puts "bits 16\n\n"
while true
  opcode_str = input.read(2) or break
  puts decode(opcode_str)
end
