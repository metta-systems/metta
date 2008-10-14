require 'find'

license = IO.readlines('license_header').join
exts = {'.cpp'=>license, '.c'=>license, '.h'=>license,'.s'=>license.dup.gsub("//",";"),'.rb'=>license.dup.gsub("//","#")}

Find.find('./') do |f|
	if File.file?(f) && exts.include?(File.extname(f))
		ext = File.extname(f)
		lic = exts[ext]
		content = IO.readlines(f).join
		if content.index(lic).nil?
			content = lic + content
			File.open(f+".new", "w") do |out|
				out.write content
			end
			begin
				File.rename(f+".new", f)
			rescue SystemCallError
				puts "Couldn't rename file #{f+".new"} to #{f}:", $!
			end
			puts "#{f} updated"
		else
			puts "#{f} is ok"
		end
	end
end
