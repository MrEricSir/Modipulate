-- 8vb
-- A shooter game to demo Modipulate

require('modipulate')
require('AnAL')

-- Constants
Direction = {
	NONE = 0,
	LEFT = 1,
	RIGHT = 2
}
SHIP_SPEED = 4
ENEMY_SPEED = 4
LASER_SPEED = 6
LOW_NOTE = 85
HIGH_NOTE = 92
EVIL_INSTRUMENT = 2

-- Direction we're moving in.
dir = Direction.NONE

---- Modipulate callbacks

function love.load()

	-- Modipulate
	modipulate.load(true)
	modipulate.open_file('../media/sponge1.it')
	modipulate.set_on_note_changed(note_changed)
	modipulate.set_on_pattern_changed(pattern_changed)
	modipulate.set_on_row_changed(row_changed)
	modipulate.set_on_tempo_changed(tempo_changed)

	-- Graphics
	imgs = {}
	imgs.bat = love.graphics.newImage('gfx/bat.png')
	imgs.mouse = love.graphics.newImage('gfx/mouse.png')
	imgs.laser = love.graphics.newImage('gfx/laser.png')

	-- List of enemies
	enemies = {}

	-- List of lasers
	lasers = {}

	-- The ship
	ship = {}
	ship.anim = newAnimation(imgs.bat, 32, 20, 0.125, 0)
	ship.w = ship.anim:getWidth()
	ship.h = ship.anim:getHeight()
	ship.x = love.graphics.getWidth() / 2
	ship.y = love.graphics.getHeight() - ship.h * 2

	-- Font
	love.graphics.setFont('Courier_New.ttf', 12)

end

----

function love.quit()

	modipulate.quit()

end

----


function love.update(dt)

	modipulate.update(dt)

	-- Update animations
	ship.anim:update(dt)
	for i,enemy in ipairs(enemies) do
		enemy.anim:update(dt)
	end

	-- Move ship
	if dir == Direction.LEFT then
		if ship.x > ship.w then
			ship.x = ship.x - SHIP_SPEED * dt * 50
		end
	elseif dir == Direction.RIGHT then
		if ship.x < love.graphics.getWidth() - ship.w then
			ship.x = ship.x + SHIP_SPEED * dt * 50
		end
	end

	-- Move enemies
	for i,enemy in ipairs(enemies) do
		enemy.y = enemy.y + ENEMY_SPEED * dt * 50
	end

	-- Move lasers
	for i, laser in ipairs(lasers) do
		laser.y = laser.y + LASER_SPEED * dt * 50
	end

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

	-- Background
	love.graphics.setBackgroundColor(0xa0, 0xa0, 0xa0)

	-- Reset foreground
	love.graphics.setColor(0xff, 0xff, 0xff, 0xff)

	-- Draw lasers
	for i,laser in ipairs(lasers) do
		laser.anim:draw(laser.x, laser.y, 0, 1, 1, laser.w / 2, laser.h / 2)
	end

	-- Draw enemies
	for i,enemy in ipairs(enemies) do
		enemy.anim:draw(enemy.x, enemy.y, 0, 1, 1, enemy.w / 2, enemy.h / 2)
	end

	-- Draw ship
	ship.anim:draw(ship.x, ship.y, 0, 1, 1, ship.w / 2, ship.h / 2)

	-- Debug
	love.graphics.setColor(0x40, 0x40, 0x40)
	love.graphics.print('Active animations\n'
			.. '  Enemies:  ' #enemies
			.. '  Lasers:   ' #lasers, 10, 10)
end

---- Modipulate callbacks

function note_changed(channel, note, instrument, sample, volume)

	if sample == EVIL_INSTRUMENT then
		local a = newAnimation(imgs.mouse, 24, 44, 0.1, 0)
		
		-- Adjust note value
		if note > HIGH_NOTE then
		    note = HIGH_NOTE
		elseif note < LOW_NOTE then
		    note = LOW_NOTE
		end;
		 
		local p = ((note - LOW_NOTE) / (HIGH_NOTE - LOW_NOTE)) -- percentage
		local x = p * love.graphics.getWidth() + 15 -- add an adjustment to keep 'em on screen.
		
		if x < 0 then
			x = 0
		elseif x > love.graphics.getWidth() then
			x = love.graphics.getWidth()
		end
		local w = a:getWidth()
		local h = a:getHeight()
		table.insert(enemies, {anim = a, x = x, y = 0, w = w, h = h})
	end

end

----

function pattern_changed(pattern)
	-- Take a sec to clean up dead anims
	if #enemies == 0 and #lasers == 0 then
	    return
	end
	
	local dirty = true
	while dirty do
	    dirty = false
		--print('checking enemy ' .. i)
		--print('y = ' .. enemies[i].y)
		for i,enemy in ipairs(enemies) do
			if enemies[i].y > love.graphics.getHeight() + 50 then
				--print('deleting...')
				table.remove(enemies, i)
				dirty = true
				break
			end
		end
	end

end

----

function row_changed(row)

	if row % 4 == 0 then
		-- Make a new laser instance
		local a = newAnimation(imgs.laser, 2, 12, 0.08, 0)
		table.insert(lasers, {anim = a, x = ship.x, y = ship.y, w = 2, h = 12})
	end

end

----

function tempo_changed(tempo)

end


