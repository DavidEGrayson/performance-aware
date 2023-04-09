# 8086 simulator homework
# https://www.computerenhance.com/p/simulating-non-memory-movs

require 'C:/Users/david/Documents/computer_enhance/perfaware/sim86/shared/contrib_ruby/sim86'

class Sim
  attr_accessor :memory
  attr_accessor :pc
  attr_accessor :registers

  def initialize
    @pc = 0
    @registers = [0] * 12
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
    inst = Sim86.decode_8086_instruction(@memory, @pc)
    p inst

    if inst.fetch(:op) == :mov
      value = operand_read(inst.fetch(:o2))
      p value
      operand_write(inst.fetch(:o1), value)
      p @registers
    end

    @pc += inst.fetch(:size)
  end

  def valid_pc?
    (0...@memory.size).include?(@pc)
  end
end

$stdout.sync = true

input_filename = ARGV.fetch(0)

sim = Sim.new
sim.memory = File.open(input_filename, 'rb') { |f| f.read }

while sim.valid_pc?
  sim.run_instruction
end

sim.registers.each_with_index do |value, i|
  puts "Reg %d: 0x%x" % [i, value]
end
