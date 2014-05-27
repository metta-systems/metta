#!/usr/bin/env ruby
#
# Apply license and modeline changes to text source files.
# Run as tools/apply_boilerplate.rb
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
no_license = ['./runtime/MersenneTwister.h', './kernel/generic/range_list.h', './kernel/arch/x86/cpu_flags.h', './kernel/arch/x86/mmu.h']

class Array
    def do_not_has?(path)
        count {|x| path.start_with?(x)} === 0
    end
end

license = IO.readlines('tools/license_header').join
modelines = ''
exts = {
    '.cpp'=>[license, modelines],
    '.c'=>[license, modelines],
    '.h'=>[license, modelines],
    '.s'=>[license.gsub(/^\/\//,";"), modelines.gsub(/^\/\//,";")],
    '.rb'=>[license.gsub(/^\/\//,"#"), modelines.gsub(/^\/\//,"#")],
    '.lua'=>[license.gsub(/^\/\//,"--"), modelines.gsub(/^\/\//,"--")],
    '.if'=>[license.gsub(/^\/\//,"#"), modelines.gsub(/^\/\//,"#")]
}

ok_count = 0
modified_count = 0
modified_files = []

Find.find('./') do |f|
    ext = File.extname(f)
    dir = File.dirname(f)
    if File.file?(f) && exts.include?(ext) && exclude_dirs.do_not_has?(dir)
        lic = exts[ext][0]
        mod = exts[ext][1]
        modified = false
        content = IO.readlines(f).join
        if content.index(lic).nil? && no_license.do_not_has?(dir) && no_license.do_not_has?(f)
            content = lic + content
            modified = true
        end
        if modified
            File.open(f+".new", "w") do |out|
                out.write content
            end
            begin
                File.rename(f+".new", f)
            rescue SystemCallError
                puts "Couldn't rename file #{f+".new"} to #{f}:", $!
            end
            puts "#{f} is UPDATED"
            modified_count += 1
            modified_files << f
        else
            puts "#{f} is ok"
            ok_count += 1
        end
    end
end

puts "#{modified_count} files changed, #{ok_count} files ok."
unless modified_files.empty?
    puts "Modified files:"
    modified_files.each { |f| puts f }
end
