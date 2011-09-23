-- Example 1
-- Barebones Modipulate app
-- Now makes use of keypressed() callback


require('modipulate')


function love.load()
	modipulate.load()

	if arg[2] then
		mod_file = arg[2]
	else
		mod_file = 'v-cf.it'
	end
	modipulate.open_file(mod_file)

	playing = true
	playing_text = 'Playing'
	modipulate.set_playing(playing)
	
	c = modipulate.get_num_channels()
	print("Number of channels", c)
	
	modipulate.set_on_note_changed(note_changed)
	modipulate.set_on_pattern_changed(pattern_changed)
end


function love.quit()
	modipulate.quit()
end


function love.update(dt)
	modipulate.update()
end


function love.keypressed(k)
	if k == ' ' then
		playing = not playing
		modipulate.set_playing(playing)
		if not playing then
			playing_text = 'Paused'
		else
			playing_text = 'Playing'
		end
	elseif k == 'escape' or k == 'q' then
		love.event.push('q')
	elseif k == '0' then
	    modipulate.set_channel_enabled(0, false);
	elseif k == '1' then
	    modipulate.set_channel_enabled(1, false);
	elseif k == '2' then
        modipulate.set_channel_enabled(2, false);
    elseif k == '3' then
        modipulate.set_channel_enabled(3, false);
    elseif k == '4' then
        modipulate.set_channel_enabled(4, false);
    elseif k == '5' then
        modipulate.set_channel_enabled(5, false);
    elseif k == '6' then
        modipulate.set_channel_enabled(6, false);
    elseif k == '7' then
        modipulate.set_channel_enabled(7, false);
    elseif k == '8' then
        modipulate.set_channel_enabled(8, false);
    elseif k == '9' then
        modipulate.set_channel_enabled(9, false);
	end
end

function love.keyreleased(k)
    if k == '0' then
        modipulate.set_channel_enabled(0, true);
    elseif k == '1' then
        modipulate.set_channel_enabled(1, true);
    elseif k == '2' then
        modipulate.set_channel_enabled(2, true);
    elseif k == '3' then
        modipulate.set_channel_enabled(3, true);
    elseif k == '4' then
        modipulate.set_channel_enabled(4, true);
    elseif k == '5' then
        modipulate.set_channel_enabled(5, true);
    elseif k == '6' then
        modipulate.set_channel_enabled(6, true);
    elseif k == '7' then
        modipulate.set_channel_enabled(7, true);
    elseif k == '8' then
        modipulate.set_channel_enabled(8, true);
    elseif k == '9' then
        modipulate.set_channel_enabled(9, true);
    end
end

function love.draw()
	love.graphics.setFont(12)
	love.graphics.print('Loaded file: ' .. mod_file, 20, 20)
	love.graphics.print(playing_text, 20, 40)
	love.graphics.print('Play/pause: space', 20, 60)
end

function note_changed(channel, note) 
    print("Note changed (channel, note)", channel, note )
end

function pattern_changed(pattern) 
    print("Pattern changed ", pattern)
end

