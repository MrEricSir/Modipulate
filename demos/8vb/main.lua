-- 8vb
-- A shooter game to demo Modipulate

require('modipulate')


Direction = {
    NONE = 0,
    LEFT = 1,
    RIGHT = 2
}

-- Direction we're moving in.
dir = Direction.NONE


function love.load()
    modipulate.load()
    
    modipulate.open_file('../media/sponge1.it')
    
    modipulate.set_on_note_changed(note_changed)
    modipulate.set_on_pattern_changed(pattern_changed)
    modipulate.set_on_row_changed(row_changed)
    modipulate.set_on_tempo_changed(tempo_changed)
end


function love.quit()
    modipulate.quit()
end


function love.update(dt)
    modipulate.update()
    
    -- move ship
    if dir == Direction.LEFT then
        -- go left
    elseif dir == Direction.RIGHT then
        -- go right
    end
end


function love.keypressed(k)
    if k == 'escape' or k == 'q' then
        love.event.push('q')
    elseif k == 'left' then
        dir = Direction.LEFT
    elseif k == 'right' then
        dir = Direction.RIGHT
    end
end


function love.keyreleased(k)
    if k == 'left' then
        if love.keyboard.isDown('right') then
            dir = Direction.RIGHT
        else
            dir = Direction.NONE
        end
    elseif k == 'right' then
        if love.keyboard.isDown('left') then
            dir = Direction.LEFT
        else
            dir = Direction.NONE
        end
    end
end


function love.draw()
    
end


function note_changed(channel, note, instrument, sample, volume)
    
end


function pattern_changed(pattern)
    
end


function row_changed(row)
   
end


function tempo_changed(tempo)
    
end

