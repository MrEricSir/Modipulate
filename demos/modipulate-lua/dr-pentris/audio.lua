-- Audio

-- Redefine "play" to always play from start
function play(source)
	love.audio.rewind(source)
	love.audio.play(source)
end

-- SFX
sfx_metronome = love.audio.newSource('media/sfx_metro.ogg', static)
sfx_rotate = love.audio.newSource('media/sfx_rotate.ogg', static)
sfx_slide = love.audio.newSource('media/sfx_slide.ogg', static)

