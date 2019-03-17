width = 80

utf_bytes = 0
function compute_color(byte)
	-- handle utf-8 codepoints
	if byte >= 128 then
		local mask
		if (byte & ~0xf) == 0xf0 then
			mask = 0xf
			utf_bytes = 3
		elseif (byte & ~0x1f) == 0xe0 then
			mask = 0x1f
			utf_bytes = 2
		elseif (byte & ~0x3f) == 0xc0 then
			mask = 0x3f
			utf_bytes = 1
		elseif (byte & ~0x7f) == 0x80 then
			utf_bytes = utf_bytes - 1
			mask = 0x7f
		end

		if utf_bytes == 0 then
			return 255, 255, 0
		else
			return nil, SKIP, nil
		end
	end

	if byte == 10 then
		return nil, ENDL, nil
	end

	if byte >= 97 and byte <= 122 then
		return 0, 0, 255
	elseif byte >= 65 and byte <= 90 then
		return 255, 0, 0
	elseif byte >= 48 and byte <= 57 then
		return 0, 255, 0
	else
		return 255, 255, 255
	end
end
