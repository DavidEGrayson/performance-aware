#!/usr/bin/env ruby

require 'json'

EarthRadius = 6372.8

def square(x)
  x * x
end

def degrees_to_radians(d)
  0.01745329251994329577 * d
end

def haversine_distance(pair)
  lat1 = pair.fetch(:y0)
  lat2 = pair.fetch(:y1)
  lon1 = pair.fetch(:x0)
  lon2 = pair.fetch(:x1)
  d_lat = degrees_to_radians(lat2 - lat1)
  d_lon = degrees_to_radians(lon2 - lon1)
  lat1 = degrees_to_radians(lat1)
  lat2 = degrees_to_radians(lat2)

  a = square(Math.sin(d_lat / 2.0)) + \
    Math.cos(lat1) * Math.cos(lat2) * square(Math.sin(d_lon / 2))
  c = 2.0 * Math.asin(Math.sqrt(a))

  EarthRadius * c
end

count = ARGV.fetch(0, 10).to_i

point_file = File.open('points.json', 'w')
answer_file = File.open('haversine.f64', 'w')

total = 0
point_file.puts '{ "pairs": ['
count.times do |n|
  pair = {
    x0: rand(-180.0...180.0), y0: rand(-90.0...90.0),
    x1: rand(-180.0...180.0), y1: rand(-90.0...90.0),
  }
  point_file.puts '  ' + JSON.dump(pair) + ','
  distance = haversine_distance(pair)
  answer_file.write([distance].pack('d'))
  total += distance
end
point_file.puts ']}'

average = total / count
puts "Average: #{average}"
