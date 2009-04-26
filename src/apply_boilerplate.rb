#!/usr/bin/env ruby
#
# Copyright 2007 - 2009, Stanislav Karchebnyy <berkus+metta@madfire.net>
#
# Distributed under the Boost Software License, Version 1.0.
# (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
#
#
# Apply license and modeline changes to text source files.
#
require 'find'

exclude_dirs = ['./_build_', './vesper/_build_']
no_license_dirs = ['./vesper/lib/bstrlib', './vesper/lib/oskit', './vesper/lib/atomic']

class Array
    def do_not_has?(path)
        count {|x| path.start_with?(x)} === 0
    end
end

license = IO.readlines('license_header').join
modelines = IO.readlines('modelines.txt').join
exts = {
    '.cpp'=>[license, modelines],
    '.c'=>[license, modelines],
    '.h'=>[license, modelines],
    '.s'=>[license.gsub(/^\/\//,";"), modelines.gsub(/^\/\//,";")],
    '.rb'=>[license.gsub(/^\/\//,"#"), modelines.gsub(/^\/\//,"#")],
    '.lua'=>[license.gsub(/^\/\//,"--"), modelines.gsub(/^\/\//,"--")]
}

ok_count = 0
modified_count = 0
modified_files = []

Find.find('./') do |f|
    ext = File.extname(f)
    dir = File.dirname(f)
    if File.file?(f)
       && exts.include?(ext)
       && exclude_dirs.do_not_has?(dir)
        lic = exts[ext][0]
        mod = exts[ext][1]
        modified = false
        content = IO.readlines(f).join
        if content.index(lic).nil? && no_license_dirs.do_not_has?(dir)
            content = lic + content
            modified = true
        end
        if content.index(mod).nil?
            content = content + mod
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

# kate: indent-width 4; replace-tabs on;
# vim: set et sw=4 ts=4 sts=4 cino=(4 :
