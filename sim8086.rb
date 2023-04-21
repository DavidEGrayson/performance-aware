# 8086 simulator homework
# https://www.computerenhance.com/p/simulating-non-memory-movs
# https://www.computerenhance.com/p/simulating-conditional-jumps

require 'C:/Users/david/Documents/computer_enhance/perfaware/sim86/shared/contrib_ruby/sim86'

class Sim
  attr_accessor :memory
  attr_accessor :ip
  attr_accessor :registers

  def initialize
    @ip = 0
    @registers = [0] * 12
    @zero_flag = false
    @sign_flag = false
  end

  def reg_read_byte(index, offset)
    @registers.fetch(index) >> (offset * 8) & 0xFF
  end

  def reg_write_byte(index, offset, value)
    shift = offset * 8
    @registers[index] = @registers.fetch(index) &
      ~(0xFF << shift) | (value & 0xFF) << shift
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
    else
      raise NotImplementedError, "operand write11: #{operand.inspect}"
    end
  end

  def run_instruction
    puts "Run inst at ip=#{@ip}"
    inst = Sim86.decode_8086_instruction(@memory, @ip)

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
      raise NotImplementedError, inst[:op].to_s
    end
  end

  def valid_ip?
    (0...@memory.size).include?(@ip)
  end

  def print_state
    puts "ax: 0x%x" % @registers[0]
    puts "bx: 0x%x" % @registers[1]
    puts "cx: 0x%x" % @registers[2]
    puts "dx: 0x%x" % @registers[3]
    puts "sp: 0x%x" % @registers[4]
    puts "bp: 0x%x" % @registers[5]
    puts "di: 0x%x" % @registers[6]
    puts "si: 0x%x" % @registers[7]
    puts "cs: 0x%x" % @registers[8]
    puts "ds: 0x%x" % @registers[9]
    puts "ss: 0x%x" % @registers[10]
    puts "es: 0x%x" % @registers[11]
    puts "Flags: %c%c" % [@zero_flag ? 'Z' : ' ', @sign_flag ? 'S' : ' ']
  end
end

$stdout.sync = true

input_filename = ARGV.fetch(0)

sim = Sim.new
sim.memory = File.open(input_filename, 'rb') { |f| f.read }

while sim.valid_ip?
  sim.run_instruction
end

sim.print_state
