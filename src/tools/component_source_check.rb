#!/usr/bin/env ruby
#
# Check sources for adhering to some large-scale software design principles as outlined in [Lakos96]
# Run as tools/component_source_check.rb
#
# Part of Metta OS. Check http://atta-metta.net for latest version.
#
# Copyright 2007 - 2012, Stanislav Karchebnyy <berkus@exquance.com>
#
# Distributed under the Boost Software License, Version 1.0.
# (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
#
require 'find'

exclude_dirs = ['./_build_', './runtime/stl', './tools/meddler/llvm']

class Array
    def do_not_has?(path)
        count {|x| path.start_with?(x)} === 0
    end
end

exts = [ '.cpp' ]

ok_count = 0
incorrect_count = 0
incorrect_files = []

def right_include(file, incl)
    File.basename(incl, '.h') == File.basename(file, '.cpp')
end

Find.find('./') do |f|
    ext = File.extname(f)
    dir = File.dirname(f)
    if File.file?(f) && exts.include?(ext) && exclude_dirs.do_not_has?(dir)
        # Get all includes in the file
        file_includes = []
        IO.readlines(f).each { |l|
            if l.start_with?("#include")
                l =~ /^#include ("|<)(.*?)("|>)/
                file_includes << $2
            end
        }
        # Now check if include file with same basename is mentioned somewhere in the includes list.
        # If it is mentioned first, or not mentioned at all - this is fine, if it's not first - need to fix.
        if file_includes.size == 0 or right_include(f, file_includes[0])
            ok_count += 1
        else
            file_includes.each { |incl|
                if right_include(f, incl)
                    incorrect_files << f
                    incorrect_count += 1
                end
            }
        end
    end
end

puts "#{incorrect_count} files are incorrect, #{ok_count} files are ok."
unless incorrect_files.empty?
    puts "Files need a fix:"
    incorrect_files.each { |f| puts f }
end
