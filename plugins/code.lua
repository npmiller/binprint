width = 80
window_size = 1
endl = 10

function compute_color(byte)
	if byte == 10 then
		return nil, nil, nil
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
