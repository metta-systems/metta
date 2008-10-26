#!/usr/bin/env ruby
#
# Copyright 2007 - 2008, Stanislav Karchebnyy <berkus+metta@madfire.net>
#
# Distributed under the Boost Software License, Version 1.0.
# (See file LICENSE_1_0.txt or a copy at http:#www.boost.org/LICENSE_1_0.txt)
#
#
# Apply license and modeline changes to text source files.
#
require 'find'

license = IO.readlines('license_header').join
modelines = IO.readlines('modelines.txt').join
exts = {
    '.cpp'=>[license, modelines],
    '.c'=>[license, modelines],
    '.h'=>[license, modelines],
    '.s'=>[license.gsub("//",";"), modelines.gsub("//",";")],
    '.rb'=>[license.gsub("//","#"), modelines.gsub("//","#")]
}

ok_count = 0
modified_count = 0

Find.find('./') do |f|
    if File.file?(f) && exts.include?(File.extname(f))
        lic = exts[File.extname(f)][0]
        mod = exts[File.extname(f)][1]
        modified = false
        content = IO.readlines(f).join
        if content.index(lic).nil?
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
            puts "#{f} updated"
            modified_count += 1
        else
            puts "#{f} is ok"
            ok_count += 1
        end
    end
end

puts "#{modified_count} files changed, #{ok_count} files ok."

# kate: indent-width 4; replace-tabs on;
# vi:set ts=4:set expandtab=on:
