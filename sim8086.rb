# 8086 simulator homework
# https://www.computerenhance.com/p/simulating-non-memory-movs
# https://www.computerenhance.com/p/simulating-conditional-jumps
# https://www.computerenhance.com/p/simulating-memory

require 'C:/Users/david/Documents/computer_enhance/perfaware/sim86/shared/contrib_ruby/sim86'

class Sim
  attr_accessor :memory
  attr_accessor :program_end
  attr_accessor :ip
  attr_accessor :registers

  def initialize
    @ip = 0
    @registers = [0] * 12
    @zero_flag = false
    @sign_flag = false
  end

  def load_program(program)
    @program_end = program.size
    @memory = program.ljust(64 * 1024, "\x00")
  end

  def reg_read_byte(index, offset)
    @registers.fetch(index) >> (offset * 8) & 0xFF
  end

  def reg_write_byte(index, offset, value)
    shift = offset * 8
    @registers[index] = @registers.fetch(index) &
      ~(0xFF << shift) | (value & 0xFF) << shift
  end

  def calculate_address_term(term)
    return 0 if term.nil? || term.fetch(:register) == 0
    raise NotImplementedError, term.inspect if term.fetch(:offset) != 0
    @registers.fetch(term[:register] - 1)
  end

  def calculate_address(operand)
    calculate_address_term(operand[:t0]) +
      calculate_address_term(operand[:t1]) +
      operand.fetch(:displacement, 0)
  end

  def operand_read(operand)
    if operand.is_a?(Integer)
      operand
    elsif operand.key?(:register)
      value = 0
      operand.fetch(:count).times do |i|
        offset = operand.fetch(:offset)
        value |= reg_read_byte(operand[:register] - 1, offset + i) << (i * 8)
      end
      value
    elsif operand.key?(:t0) || operand.key?(:t1) || operand.key?(:displacement)
      address = calculate_address(operand)
      # Assumption: 16-bit memory read
      @memory[address + 0].ord | @memory[address + 1].ord << 8
    else
      raise NotImplementedError, "operand read: #{operand.inspect}"
    end
  end

  def operand_write(operand, value)
    if operand.is_a?(Integer)
      raise NotSupportedError
    elsif operand.key?(:register)
      operand.fetch(:count).times do |i|
        reg_write_byte(operand[:register] - 1, operand.fetch(:offset) + i, value & 0xFF)
        value >>= 8
      end
    elsif operand.key?(:t0) || operand.key?(:t1) || operand.key?(:displacement)
      address = calculate_address(operand)
      # Assumption: 16-bit memory write
      @memory[address + 0] = (value & 0xFF).chr
      @memory[address + 1] = (value >> 8 & 0xFF).chr
    else
      raise NotImplementedError, "operand write: #{operand.inspect}"
    end
  end

  def run_instruction
    inst = Sim86.decode_8086_instruction(@memory, @ip)
    #puts "Run inst at ip=#{@ip}: #{inst.inspect}"

    orig_ip = @ip
    op = inst.fetch(:op)
    @ip += inst.fetch(:size)
    case op
    when :mov
      value = operand_read(inst.fetch(:o2))
      operand_write(inst.fetch(:o1), value)
    when :add
      v2 = operand_read(inst.fetch(:o2))
      v1 = operand_read(inst.fetch(:o1))
      result = (v1 + v2) & 0xFFFF
      @zero_flag = result == 0
      @sign_flag = result & 0x8000 != 0
      operand_write(inst.fetch(:o1), result)
    when :sub, :cmp
      v2 = operand_read(inst.fetch(:o2))
      v1 = operand_read(inst.fetch(:o1))
      result = (v1 - v2) & 0xFFFF
      @zero_flag = result == 0
      @sign_flag = result & 0x8000 != 0
      operand_write(inst.fetch(:o1), result) if op == :sub
    when :jne
      @ip += inst.fetch(:o1) if !@zero_flag
    else
      raise NotImplementedError, "Unimplemented op #{op} at ip=#{orig_ip}"
    end
  end

  def valid_ip?
    (0...program_end).include?(@ip)
  end

  def print_state
    puts "ax: 0x%04X" % @registers[0]
    puts "bx: 0x%04X" % @registers[1]
    puts "cx: 0x%04X" % @registers[2]
    puts "dx: 0x%04X" % @registers[3]
    puts "sp: 0x%04X" % @registers[4]
    puts "bp: 0x%04X" % @registers[5]
    puts "si: 0x%04X" % @registers[6]
    puts "di: 0x%04X" % @registers[7]
    puts "cs: 0x%04X" % @registers[8]
    puts "ds: 0x%04X" % @registers[9]
    puts "ss: 0x%04X" % @registers[10]
    puts "es: 0x%04X" % @registers[11]
    puts "Flags: %c%c" % [@zero_flag ? 'Z' : ' ', @sign_flag ? 'S' : ' ']
  end
end

$stdout.sync = true

input_filename = ARGV.fetch(0)

sim = Sim.new
sim.load_program File.open(input_filename, 'rb') { |f| f.read }

while sim.valid_ip?
  sim.run_instruction
end

sim.print_state
