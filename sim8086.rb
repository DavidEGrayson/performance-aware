# 8086 MOV instruction decoder
# https://www.computerenhance.com/p/instruction-decoding-on-the-8086
# https://www.computerenhance.com/p/decoding-multiple-instructions-and

$stdout.sync = true

input_filename = ARGV.fetch(0)

RegNames = [
  %w{ al cl dl bl ah ch dh bh },
  %w{ ax cx dx bx sp bp si di },
]

AddressExprs = [
  'bx + si', 'bx + di', 'bp + si', 'bp + di', 'si', 'di', 'bp', 'bx'
]

def decode_rm(mod, w, rm, input)
  if mod == 0b11
    # r/m is treated as a reg field
    RegNames.fetch(w).fetch(rm)
  else
    # r/m is a memory address expression
    expr = AddressExprs.fetch(rm)
    if mod == 0b00
      if rm == 0b110
        # 16-bit direct address
        return '[' + input.read(2).unpack('<S')[0].to_s + ']'
      end
      # No displacement
      disp = 0
    elsif mod == 0b01
      # 8-bit signed memory displacement
      disp = input.read(1).unpack('c')[0]
    elsif mod == 0b10
      # 16-bit signed memory displacement
      disp = input.read(2).unpack('<s')[0]
    end
    expr += " + #{disp}" if disp > 0
    expr += " - #{-disp}" if disp < 0
    '[' + expr + ']'
  end
end

def read_data(w, input)
  if w == 1
    input.read(2).unpack('<S')[0]
  else
    input.read(1).ord
  end
end

def decode(input)
  s0 = input.read(1) or return
  byte0 = s0.ord
  case
  when byte0 >> 2 == 0b100010      # MOV: Register/memory to/from register
    byte1 = input.read(1).ord
    d = byte0[1]
    w = byte0[0]
    mod = byte1 >> 6 & 3
    reg = byte1 >> 3 & 7
    rm = byte1 & 7
    op2 = RegNames.fetch(w).fetch(reg)
    op1 = decode_rm(mod, w, rm, input)
    dest, src = d == 1 ? [op2, op1] : [op1, op2]
    "mov #{dest}, #{src}"
  when byte0 >> 1 == 0b1100011     # MOV: Immediate to register/memory
    w = byte0[0]
    byte1 = input.read(1).ord
    mod = byte1 >> 6 & 3
    rm = byte1 & 7
    dest = decode_rm(mod, w, rm, input)
    imm = read_data(w, input).to_s
    imm = "word " + imm if w == 1 && mod != 0b11
    imm = "byte " + imm if w == 0 && mod != 0b11
    "mov #{dest}, #{imm}"
  when byte0 >> 4 == 0b1011        # MOV: Immediate to register
    w = byte0[3]
    reg = byte0 & 7
    dest = RegNames.fetch(w).fetch(reg)
    imm = read_data(w, input)
    "mov #{dest}, #{imm}"
  when byte0 >> 2 == 0b101000      # MOV: Memory <-> accumulator
    w = byte0[0]
    d = byte0[1]
    addr = read_data(w, input)
    if d == 1
      "mov [#{addr}], ax"
    else
      "mov ax, [#{addr}]"
    end
  else
    raise NotImplementedError, "byte0 = 0x%x" % byte0
  end
end

input = File.open(input_filename, 'rb')
puts "bits 16\n\n"
while true
  str = decode(input) or break
  puts str
end
