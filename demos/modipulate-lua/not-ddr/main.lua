-- not-ddr
-- A music-visualization demo for modipulate

---- LOVE callbacks

require 'libmodipulatelua'

tempo = 0

function love.load()

	-- Convert HSL to RGB
	-- Input and output range: 0 - 255
	-- http://love2d.org/wiki/HSL
	function HSL(h, s, l)
		if s <= 0 then
			return l, l, l
		end
		h, s, l = (h / 256) * 6, s / 255, l / 255
		local c = (1 - math.abs(2 * l - 1)) * s
		local x = (1 - math.abs(h % 2 - 1)) * c
		local m, r, g, b = (l - 0.5 * c), 0, 0, 0
		if h < 1 then
			r, g, b = c, x, 0
		elseif h < 2 then
			r, g, b = x, c, 0
		elseif h < 3 then
			r, g, b = 0, c, x
		elseif h < 4 then
			r, g, b = 0, x, c
		elseif h < 5 then
			r, g, b = x, 0, c
		else
			r, g, b = c, 0, x
		end
		return (r + m) * 255, (g + m) * 255, (b + m) * 255
	end


	-- Setup Modipulate
	modipulate.init()
	song = modipulate.loadSong('../../media/vhiiula-inventio_in_4k.it')  --v-cf.it')
	
	modipulate.setVolume(1)
    tempo = song.defaultTempo
    song:onRowChange(exe_row)
    song:onNote(exe_note)
	

	-- Frame for columns
	frame_margin = 20
	frame_width = love.graphics.getWidth() - 2 * frame_margin - 240
	frame_height = love.graphics.getHeight() - 2 * frame_margin
	frame_x1 = frame_margin
	frame_x2 = frame_x1 + frame_width
	frame_y1 = frame_margin
	frame_y2 = frame_y1 + frame_height

	-- Channel info
	channels = {}
	no_channels = song.numChannels
	channel_width = frame_width / no_channels
	for i=1,no_channels do
		channels[i] = {enabled=true}
	end

	-- Initialize bubbles
	bubbles = {}

	-- Other stuff
	bubble_spacing = 20
	bubble_decay_rate = 0.85
	transposition = 0
	control_info = '-- Controls --\
- / +    Change tempo\
[ / ]    Change bubble spacing\
, / .    Change decay rate\
z / x    Transpose (semitone)\
a / s    Transpose (octave)\
(click)  Toggle channel'
	font = love.graphics.newFont('Courier_New.ttf', 12)
	love.graphics.setFont(font)

    song:play(true)
end

----

function love.update(dt)

	modipulate.update()

end

----

function love.draw(dt)

	-- Draw bubbles
	local bubble_x_center = (frame_width / no_channels) / 2
	for i,bubble in ipairs(bubbles) do
		love.graphics.setColor(0x40, 0x40, 0x40)
		love.graphics.circle('fill', bubble.x, bubble.y, bubble.radius * 1.15, 20)
		love.graphics.setColor(bubble.color)
		love.graphics.circle('fill', bubble.x, bubble.y, bubble.radius, 20)
	end

	-- Draw a black border to cover stuff outside of bounds?

	-- Draw the frame
	love.graphics.setColor(0x40, 0x40, 0x40)
	love.graphics.line(frame_x1, frame_y1, frame_x1, frame_y2,
			frame_x2, frame_y2, frame_x2, frame_y1, frame_x1, frame_y1)
	-- Column bars
	local no_channels = no_channels
	for i=1,no_channels-1 do
		local x = i * (frame_width / no_channels) + frame_x1
		love.graphics.line(x, frame_y1, x, frame_y2)
	end
	-- X-out disabled channels
	for i,ch in ipairs(channels) do
		if not ch.enabled then
			local x1 = (i - 1) * channel_width + frame_x1
			local x2 = x1 + channel_width
			love.graphics.setColor(0x80, 0x10, 0x10)
			love.graphics.line(x1, frame_y1, x2, frame_y2)
			love.graphics.line(x2, frame_y1, x1, frame_y2)
		end
	end

	-- Print info
	local print_x = frame_x2 + 8
	love.graphics.setColor(0x40, 0x40, 0x40)
	love.graphics.print('Total bubbles: ' .. #bubbles, print_x, 20)
	love.graphics.print('Bubble spacing: ' .. bubble_spacing, print_x, 35)
	love.graphics.print('Decay rate: ' .. bubble_decay_rate, print_x, 50)
	love.graphics.print('Transpose: ' .. transposition, print_x, 65)
	love.graphics.print('Tempo: ' .. tempo, print_x, 80)
	-- Controls
	love.graphics.print(control_info, print_x, 120)

end

----

function love.quit()

	modipulate.deinit()

end

----

function love.keypressed(k)

	if k == 'escape'
	or k == 'q' then
		love.event.push('q')
	elseif k == '+'
	or k == '='
	or k == 'kp+' then
		if tempo < 300 then
			tempo = tempo + 5
			song:effectCommand(0, 17, tempo)
		end
	elseif k == '-'
	or k == 'kp-' then
		if tempo > 40 then
			tempo = tempo - 5
			song:effectCommand(0, 17, tempo)
		end
	elseif k == '[' then
		if bubble_spacing > 2 then
			bubble_spacing = bubble_spacing - 2
		end
	elseif k == ']' then
		if bubble_spacing < 50 then
			bubble_spacing = bubble_spacing + 2
		end
	elseif k == ',' then
		if bubble_decay_rate > 0.5 then
			bubble_decay_rate = bubble_decay_rate - 0.05
		end
	elseif k == '.' then
		if bubble_decay_rate < 1 then
			bubble_decay_rate = bubble_decay_rate + 0.05
		end
	elseif k == 'z' then
		transposition = transposition - 1
		for i=0,no_channels-1 do
			song:setTransposition(i, transposition)
		end
	elseif k == 'x' then
		transposition = transposition + 1
		for i=0,no_channels-1 do
			song:setTransposition(i, transposition)
		end
	elseif k == 'a' then
		transposition = transposition - 12
		for i=0,no_channels-1 do
			song:setTransposition(i, transposition)
		end
	elseif k == 's' then
		transposition = transposition + 12
		for i=0,no_channels-1 do
			song:setTransposition(i, transposition)
		end
	end

end

----

function love.mousepressed(x, y, button)

	-- See if mouse L press was within within frame
	if button == 'l'
	and x > frame_x1
	and x < frame_x2
	and y > frame_y1
	and y < frame_y2 then
		-- Figure out which track was pressed
		local ch = math.floor(((x - frame_x1) / frame_width)
				* no_channels)
		-- Toggle mute status and set muting
		channels[ch + 1].enabled = not channels[ch + 1].enabled
		song:setChannelEnabled(ch, channels[ch + 1].enabled)
	end

end

---- Modipulate callbacks

-- New note
function exe_note(channel, note, instrument, sample, volumeCommand, volumeValue, effectCommand, effectValue)
	local new = {
		x = (frame_width / no_channels) * (channel + 1)
				- ((frame_width / no_channels) / 2) + frame_x1,
		y = frame_y1 + (channel_width / 1.9),
		radius = (channel_width / 2) * 0.8,
		color = {HSL(((note + transposition) * 4 - 128), 0x80, 0xa0)},
		first_row = true
	}
	table.insert(bubbles, new)
end

-- Row change
function exe_row(row)
	for i,bubble in ipairs(bubbles) do
		bubble.y = bubble.y + (bubble_spacing)
		if bubble.y > (frame_y2 - bubble.radius) then
			table.remove(bubbles, i)
		end
		if bubble.first_row then
			-- Shrink the bubble from its initial size
			bubble.radius = bubble.radius * 0.7
			bubble.first_row = false
		else
			-- Shrink the bubble a litle each time
			bubble.radius = bubble.radius * bubble_decay_rate
		end
	end
end

