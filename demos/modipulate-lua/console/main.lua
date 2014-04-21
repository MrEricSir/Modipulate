-- Console
-- Barebones Modipulate app. Look at the console for interesting stuff.
-- Check the key functions for keyboard commands.

require 'libmodipulatelua'

sound_effect = love.audio.newSource('rooster.ogg', 'static')
playing_text = 'Playing'

pattern_number = 0
row_number = 0
tempo = 0
rows_in_pattern = 0

function love.load()
    modipulate.init()

    if arg[2] then
        mod_file = arg[2]
    else
        mod_file = '../../media/sponge1.it'
    end

    song = modipulate.loadSong(mod_file)

    tempo = song.defaultTempo

    print('Song loaded')
    print('Title: ', song.title)
    print('Message: ', song.message)
    print('Number of Channels: ', song.numChannels)
    print('Number of Instruments: ', song.numInstruments)
    print('Number of Samples: ', song.numSamples)
    print('Number of Patterns: ', song.numPatterns)
    print('Default tempo: ', song.defaultTempo)
    print('')
    for i = 0, song.numSamples - 1 do
        print('Sample: ', i, song:getSampleName(i))
    end
    print('')
    for i = 0, song.numInstruments - 1 do
       print('Instrument: ', i, song:getInstrumentName(i))
    end
    print('')

    song:onPatternChange(patternChanged)
    song:onRowChange(rowChanged)
    song:onNote(noteChanged)

    song:play(true)
end


function love.quit()
    modipulate.deinit()
end


function love.update(dt)
    modipulate.update()
end


function love.keypressed(k)
    if k == ' ' then
        playing = not playing
        song:play(playing)
        if not playing then
            playing_text = 'Paused'
        else
            playing_text = 'Playing'
        end
    elseif k == 'escape' or k == 'q' then
        love.event.quit()
    elseif k == '0' then
        song:setChannelEnabled(0, false)
    elseif k == '1' then
        song:setChannelEnabled(1, false)
    elseif k == '2' then
        song:setChannelEnabled(2, false)
    elseif k == '3' then
        song:setChannelEnabled(3, false)
    elseif k == '4' then
        song:setChannelEnabled(4, false)
    elseif k == '5' then
        song:setChannelEnabled(5, false)
    elseif k == '6' then
        song:setChannelEnabled(6, false)
    elseif k == '7' then
        song:setChannelEnabled(7, false)
    elseif k == '8' then
        song:setChannelEnabled(8, false)
    elseif k == '9' then
        song:setChannelEnabled(9, false)
    elseif k == 'e' then
        -- Ignore an effect in the song.
        print('Disabling effect');
        song:enableEffect(4, 1, false)
    elseif k == 's' then
        love.audio.play(sound_effect)
    elseif k == 't' then
        print('Increasing tempo')
        -- increase tempo
        tempo = tempo * 2
        if tempo > 0xFF then
            tempo = 0xFF
        end

        song:effectCommand(0, 17, tempo)
	elseif k == 'o' then
		-- Load & play a new song (at the same time)
		song2 = modipulate.loadSong('../../media/8vb1.it')
		song2:play(true)
    end
end

function love.keyreleased(k)
    VOL_STEP = 1 / 10; -- volume step, from 0 to 1
    if k == '0' then
        song:setChannelEnabled(0, true)
    elseif k == '1' then
        song:setChannelEnabled(1, true)
    elseif k == '2' then
        song:setChannelEnabled(2, true)
    elseif k == '3' then
        song:setChannelEnabled(3, true)
    elseif k == '4' then
        song:setChannelEnabled(4, true)
    elseif k == '5' then
        song:setChannelEnabled(5, true)
    elseif k == '6' then
        song:setChannelEnabled(6, true)
    elseif k == '7' then
        song:setChannelEnabled(7, true)
    elseif k == '8' then
        song:setChannelEnabled(8, true)
    elseif k == '9' then
        song:setChannelEnabled(9, true)
    elseif k == '-' then
        modipulate.setVolume(modipulate.getVolume() - VOL_STEP)
        print("Volume is", modipulate.getVolume())
    elseif k == '=' then
        modipulate.setVolume(modipulate.getVolume() + VOL_STEP)
        print("Volume is", modipulate.getVolume())
    elseif k == 'e' then
        -- Re-enable the effect.
        print('Enabling effect');
        song:enableEffect(4, 1, true)
    elseif k == 't' then
        -- decrease tempo
        print('Decreasing tempo')
        tempo = tempo / 2
        if tempo < 0x20 then
            tempo = 0x20
        end

        song:effectCommand(0, 17, tempo)
    elseif k == 'z' then
        for i = 0, song.numChannels, 1 do
            song:setTransposition(i, song:getTransposition(i) - 1)
        end
    elseif k == 'x' then
        for i = 0, song.numChannels, 1 do
            song:setTransposition(i, song:getTransposition(i) + 1)
        end
	elseif k == 'o' then
		-- Kill the song
		song2 = nil
		collectgarbage()
    end
end

function love.draw()
    --love.graphics.setFont(12)
    love.graphics.print('Loaded file: ' .. mod_file, 20, 20)
    love.graphics.print(playing_text, 20, 40)
    love.graphics.print('Play/pause: space', 20, 60)
    love.graphics.print('Pattern:' .. pattern_number, 20, 80)
    love.graphics.print('Row:' .. row_number .. '/' .. rows_in_pattern, 20, 100)
    love.graphics.print('Tempo:' .. tempo, 20, 120)
end

function patternChanged(patternNum)
    print('Pattern:', patternNum)
    pattern_number = patternNum
end

function rowChanged(rowNum)
    print('Row:', rowNum)
    row_number = rowNum
end

function noteChanged(channel, note, instrument, sample, volumeCommand, volumeValue, effectCommand, effectValue)
    print("Note changed (channel, note, instrument, sample)", channel, note, instrument, sample)

    -- TODO: a handy-dandy list of effect and volumeCommand values for Lua devs.
    --       For now, you'll have to consult sndfile.h (in the modplug directory)
    if effectCommand == 17 then
        tempo = effectValue
    end
end

