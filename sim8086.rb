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

SimilarOps = {0 => :add, 5 => :sub, 7 => :cmp}

DisplacementOps = {
  0x70 => :jo,
  0x71 => :jno,
  0x72 => :jb,
  0x73 => :jnb,
  0x74 => :jz,
  0x75 => :jnz,
  0x76 => :jbe,
  0x77 => :jnbe,
  0x78 => :js,
  0x79 => :jns,
  0x7A => :jp,
  0x7B => :jnp,
  0x7C => :jl,
  0x7D => :jnl,
  0x7E => :jle,
  0x7F => :jnle,
  0xE0 => :loopnz,
  0xE1 => :loopz,
  0xE2 => :loop,
  0xE3 => :jcxz,
}

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

def read_data_signed(w, input)
  if w == 1
    input.read(2).unpack('<S')[0]
  else
    input.read(1).unpack('c')[0]
  end
end

def read_data_ws(w, s, input)
  if w == 1 && s == 0
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
  when (byte0 & 0b11000100) == 0   # ADD/SUB/CMP: Register/memory with register
    byte1 = input.read(1).ord
    op = SimilarOps.fetch(byte0 >> 3 & 7)
    d = byte0[1]
    w = byte0[0]
    mod = byte1 >> 6 & 3
    reg = byte1 >> 3 & 7
    rm = byte1 & 7
    op2 = RegNames.fetch(w).fetch(reg)
    op1 = decode_rm(mod, w, rm, input)
    dest, src = d == 1 ? [op2, op1] : [op1, op2]
    "#{op} #{dest}, #{src}"
  when (byte0 & 0b11000100) == 0x80  # ADD/SUB/CMP: Immediate to register/memory
    byte1 = input.read(1).ord
    w = byte0[0]
    s = byte0[1]
    op = SimilarOps.fetch(byte1 >> 3 & 7)
    mod = byte1 >> 6 & 3
    rm = byte1 & 7
    op2 = decode_rm(mod, w, rm, input)
    imm = read_data_ws(w, s, input)
    size = ["byte ", "word "].fetch(w)
    "#{op} #{size}#{op2}, #{imm}"
  when (byte0 & 0b11000100) == 0x04  # ADD/SUB/CMP: Immediate to accumulator
    w = byte0[0]
    imm = read_data_signed(w, input)
    op = SimilarOps.fetch(byte0 >> 3 & 7)
    dest = %w{al ax}.fetch(w)
    "#{op} #{dest}, #{imm}"
  when DisplacementOps.key?(byte0)   # Conditional jumps and loops
    op = DisplacementOps.fetch(byte0)
    disp = input.read(1).unpack('c')[0]
    "#{op} ($+2)+#{disp}"
  else
    raise NotImplementedError, "byte0 = 0x%X" % byte0
  end
end

input = File.open(input_filename, 'rb')
puts "bits 16\n\n"
while true
  str = decode(input) or break
  puts str
end
