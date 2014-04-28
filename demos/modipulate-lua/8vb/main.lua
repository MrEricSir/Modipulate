-- 8vb
-- A shooter game to demo Modipulate

package.cpath = package.cpath .. ';./?.dylib'

require('libmodipulatelua')
require('AnAL')

-- Constants
Direction = {
	NONE = 0,
	LEFT = 1,
	RIGHT = 2
}
SHIP_SPEED = 6
ENEMY_SPEED = 3
POWERUP_SPEED = ENEMY_SPEED
LASER_SPEED = 8
BG_SPEED = 0.5
NORMAL_RATE_OF_FIRE = 4 -- Fire every n rows
LOW_NOTE = 58
HIGH_NOTE = 94
EVIL_INSTRUMENT = 2
SCREEN_WIDTH = love.graphics.getWidth()
SCREEN_HEIGHT = love.graphics.getHeight()
-- Playfield bounds not quite symmetrical
-- Compensating for images' origin being upper-left instead of center
PLAYFIELD_LEFT = 20
PLAYFIELD_RIGHT = SCREEN_WIDTH - 50
PLAYFIELD_WIDTH = PLAYFIELD_RIGHT - PLAYFIELD_LEFT
SHOW_BOUNDING_BOXES = false

-- These change
dir = Direction.NONE
rate_of_fire = NORMAL_RATE_OF_FIRE
tempo = 0

---- Modipulate callbacks

function love.load()
    modipulate.init()
    
	-- Modipulate
	song = modipulate.loadSong('../../media/8vb1.it')
    tempo = song.defaultTempo
    song:onPatternChange(pattern_changed)
    song:onRowChange(row_changed)
    song:onNote(note_changed)
    
	-- Graphics
	imgs = {}
	imgs.bat = love.graphics.newImage('gfx/bat.png')
	imgs.mouse = love.graphics.newImage('gfx/mouse.png')
	imgs.laser = love.graphics.newImage('gfx/laser2.png')
	imgs.explosion = love.graphics.newImage('gfx/explosion2.png')
	imgs.cherry = love.graphics.newImage('gfx/cherry.png')
	imgs.clock = love.graphics.newImage('gfx/clock.png')
	imgs.candy = love.graphics.newImage('gfx/candy.png')
	-- Background pattern
	local bg_sprite = love.image.newImageData('gfx/bg1.png')
	local sprite_w = bg_sprite:getWidth()
	local sprite_h = bg_sprite:getHeight()
	local new_h = sprite_h
	while new_h < SCREEN_HEIGHT * 2 do
		new_h = new_h + sprite_h
	end
	local bg_repeat = love.image.newImageData(SCREEN_WIDTH, new_h)
	for x=0,(SCREEN_WIDTH / sprite_w - 1) do
		for y=0,((SCREEN_HEIGHT / sprite_h) * 2) do
			local dx = x * sprite_w
			local dy = y * sprite_h
			--print('dx',dx,'dy',dy)
			--local sx = 0
			--local sy = 0
			--local sw = sprite_w
			--local sh = sprite_h
			bg_repeat:paste(bg_sprite, dx, dy)
		end
	end
	imgs.bg = love.graphics.newImage(bg_repeat)
	bg = {}
	bg.w = imgs.bg:getWidth()
	bg.h = imgs.bg:getHeight()
	bg.y = -(bg.h / 2)

	-- Lists of things
	enemies = {}
	lasers = {}
	explosions = {}
	powerups = {}

	-- The ship
	ship = {}
	ship.anim = newAnimation(imgs.bat, 32, 20, 0.125, 0)
	ship.w = ship.anim:getWidth()
	ship.h = ship.anim:getHeight()
	ship.x = SCREEN_WIDTH / 2
	ship.y = love.graphics.getHeight() - ship.h * 2

	-- Font
	font = love.graphics.newFont('Courier_New.ttf', 12)
	love.graphics.setFont(font)

	--love.timer.sleep(500)

    song:play(true)
end

----

function love.quit()

	modipulate.deinit()

end

----


function love.update(dt)

	modipulate.update()

	-- Update animations
	ship.anim:update(dt)
	for i,enemy in ipairs(enemies) do
		enemy.anim:update(dt)
	end
	for i,laser in ipairs(lasers) do
		laser.anim:update(dt)
	end
	for i,explosion in ipairs(explosions) do
		explosion.anim:update(dt)
		if not explosion.anim.playing then
			--remove
			local dummy = nil
		end
	end

	-- Move ship
	if dir == Direction.LEFT then
		if ship.x > PLAYFIELD_LEFT then
			ship.x = ship.x - SHIP_SPEED * dt * 50
		end
	elseif dir == Direction.RIGHT then
		if ship.x < PLAYFIELD_RIGHT then
			ship.x = ship.x + SHIP_SPEED * dt * 50
		end
	end

	-- Move enemies
	for i,enemy in ipairs(enemies) do
		enemy.y = enemy.y + ENEMY_SPEED * dt * 50
	end

	-- Move lasers
	for i,laser in ipairs(lasers) do
		laser.y = laser.y - LASER_SPEED * dt * 50
	end

	-- Move powerups
	for i,candy in ipairs(powerups) do
		candy.y = candy.y + POWERUP_SPEED * dt * 50
	end

	-- Detect collision between enemies and lasers
	for li,laser in ipairs(lasers) do
		for ei,enemy in ipairs(enemies) do
			-- Help with collision taken from:
			-- https://github.com/STRd6/gamelib/blob/pixie/src/collision.coffee#L82
			if laser.x < enemy.x + enemy.w
			and laser.x + laser.w > enemy.x
			and laser.y < enemy.y + enemy.h
			and laser.y + laser.h > enemy.y then
				-- Boom
				-- Add an extra frame so it ends on a blank
				local a = newAnimation(imgs.explosion, 32, 32, 0.1, 5)
				a:setMode('once')
				local w = a:getWidth()
				local h = a:getHeight()
				table.insert(explosions, {anim = a,
						x = enemy.x + enemy.w / 2 - w / 2,
						y = enemy.y + enemy.h / 2 - h / 2,
						w = w, h = h})
				-- Destroy the mouse and laser
				table.remove(enemies, ei)
				table.remove(lasers, li)
			end
		end
	end

	-- Detect collision between ship and powerups
	for pi,powerup in ipairs(powerups) do
		if powerup.x < ship.x + ship.w
		and powerup.x + powerup.w > ship.x
		and powerup.y < ship.y + ship.h
		and powerup.y + powerup.h > ship.y then
			if powerup.type == 'clock' then
    			-- Ignore future tempo changes.
				song:enableEffect(0, 17, false)
				
				-- Slow music + enemies.
				tempo = tempo * 0.75
				song:effectCommand(0, 17, tempo)
				-- Compensate for slowed tempo
				--rate_of_fire = math.floor(rate_of_fire / 2)
			elseif powerup.type == 'candy' then
				-- Speed up fire
				rate_of_fire = math.floor(NORMAL_RATE_OF_FIRE / 2)
			end
			-- Remove the powerup
			table.remove(powerups, pi)
		end
	end

	-- Slide background image
	bg.y = bg.y + BG_SPEED
	if bg.y > 0 then bg.y = -(bg.h / 2) end

end

----

function love.keypressed(k)

	if k == 'escape' or k == 'q' then
		love.event.push('q')
	elseif k == 'left' then
		dir = Direction.LEFT
	elseif k == 'right' then
		dir = Direction.RIGHT
	end

end

----

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

----

function love.draw()

	-- Reset pen
	love.graphics.setColor(0xff, 0xff, 0xff, 0xff)

	-- Background
	love.graphics.setBackgroundColor(0xa0, 0xa0, 0xa0)
	love.graphics.draw(imgs.bg, 0, bg.y)

	-- Draw powerups
	for i,candy in ipairs(powerups) do
		love.graphics.draw(candy.anim, candy.x, candy.y)
		if SHOW_BOUNDING_BOXES then
			love.graphics.setColor(0, 0xff, 0)
			love.graphics.rectangle('line', candy.x, candy.y, candy.w, candy.h)
			love.graphics.setColor(0xff, 0xff, 0xff)
		end
	end

	-- Draw lasers
	for i,laser in ipairs(lasers) do
		laser.anim:draw(laser.x, laser.y)
		if SHOW_BOUNDING_BOXES then
			love.graphics.setColor(0, 0xff, 0)
			love.graphics.rectangle('line', laser.x, laser.y, laser.w, laser.h)
			love.graphics.setColor(0xff, 0xff, 0xff)
		end
	end

	-- Draw enemies
	for i,enemy in ipairs(enemies) do
		enemy.anim:draw(enemy.x, enemy.y)
		if SHOW_BOUNDING_BOXES then
			love.graphics.setColor(0, 0xff, 0)
			love.graphics.rectangle('line', enemy.x, enemy.y, enemy.w, enemy.h)
			love.graphics.setColor(0xff, 0xff, 0xff)
		end
	end

	-- Draw ship
	--ship.anim:draw(ship.x, ship.y, 0, 1, 1, ship.w / 2, ship.h / 2)
	ship.anim:draw(ship.x, ship.y)
	if SHOW_BOUNDING_BOXES then
		love.graphics.setColor(0, 0xff, 0)
		love.graphics.rectangle('line', ship.x, ship.y, ship.w, ship.h)
		love.graphics.setColor(0xff, 0xff, 0xff)
	end

	-- Draw explosions
	for i,explosion in ipairs(explosions) do
		explosion.anim:draw(explosion.x, explosion.y)
		if SHOW_BOUNDING_BOXES then
			love.graphics.setColor(0, 0xff, 0)
			love.graphics.rectangle('line', explosion.x, explosion.y,
					explosion.w, explosion.h)
			love.graphics.setColor(0xff, 0xff, 0xff)
		end
	end

	-- Debug
	love.graphics.setColor(0x40, 0x40, 0x40)
	love.graphics.print('Active animations\n'
			.. '  Enemies:    ' .. #enemies .. '\n'
			.. '  Powerups:   ' .. #powerups .. '\n'
			.. '  Lasers:     ' .. #lasers .. '\n'
			.. '  Explosions: ' .. #explosions, 10, 10)
end

---- Modipulate callbacks
function note_changed(channel, note, instrument, sample, volumeCommand, 
    volumeValue, effectCommand, effectValue)
    
	if sample == EVIL_INSTRUMENT then
		local a = newAnimation(imgs.mouse, 24, 44, 0.1, 0)

		-- Adjust note value
		if note > HIGH_NOTE then
		    note = HIGH_NOTE
		elseif note < LOW_NOTE then
		    note = LOW_NOTE
		end

		local p = ((note - LOW_NOTE) / (HIGH_NOTE - LOW_NOTE)) -- percentage
		local x = p * PLAYFIELD_WIDTH + PLAYFIELD_LEFT -- add an adjustment to keep 'em on screen.
		if x < 0 then
			x = 0
		elseif x > SCREEN_WIDTH then
			x = SCREEN_WIDTH
		end
		local w = a:getWidth()
		local h = a:getHeight()
		table.insert(enemies, {anim = a, x = x, y = 0, w = w, h = h})
	end

end

----

function pattern_changed(pattern)

	-- Every 4th pattern + 1, generate a candy
	if pattern % 4 == 1 then
		local x = math.random(PLAYFIELD_LEFT, PLAYFIELD_RIGHT)
		local powerup = {type = 'candy', anim = imgs.candy, x = x, y = 0,
				w = imgs.candy:getWidth(), h = imgs.candy:getHeight()}
		table.insert(powerups, powerup)
	end

	-- Every 4th pattern + 3, generate a clock
	if pattern % 4 == 3 then
		local x = math.random(PLAYFIELD_LEFT, PLAYFIELD_RIGHT)
		local powerup = {type = 'clock', anim = imgs.clock, x = x, y = 0,
				w = imgs.clock:getWidth(), h = imgs.clock:getHeight()}
		table.insert(powerups, powerup)
	end

	-- Take a sec to clean up dead anims
	if #enemies == 0 and #lasers == 0 then
		return
	end
	local dirty = true
	while dirty do
	    dirty = false
	    -- Look for inactive enemy anims
		for i,enemy in ipairs(enemies) do
			if enemies[i].y > love.graphics.getHeight() + 50 then
				table.remove(enemies, i)
				dirty = true
				break
			end
		end
	    -- Look for inactive laser anims
	    for i,laser in ipairs(lasers) do
	    	if lasers[i].y < -20 then
	    		table.remove(lasers, i)
	    		dirty = true
	    		break
	    	end
	    end
	    -- Look for inactive explosion anims
	    for i,explosion in ipairs(explosions) do
	    	if not explosions[i].anim.playing then
	    		table.remove(explosions, i)
	    		dirty = true
	    		break
	    	end
	    end
	    -- Look for inactive powerups (not animated
	    for i,powerup in ipairs(powerups) do
	    	if powerups[i].y > love.graphics.getHeight() + 50 then
	    		table.remove(powerups, i)
	    		dirty = true
	    		break
	    	end
	    end
	end

end

----

function row_changed(row)

	if row % rate_of_fire == 0 then
		-- Make a new laser instance
		local a = newAnimation(imgs.laser, 4, 12, 0.08, 0)
		table.insert(lasers, {anim = a, x = ship.x + ship.w / 2, y = ship.y,
				w = a:getWidth(), h = a:getHeight()})
	end

end


