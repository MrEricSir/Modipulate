-- "Dr. Pentris"
-- Tetris clone using new audio engine

----------------

function love.load()

	-- SEEEED
	math.randomseed(os.time() / 3)

	-- Libraries and source files
	require('modipulate')
--	require('audio')
	require('callbacks')
	require('piece')
	require('grid')

	-- Local copies
	local grid = level_grid

	-- Timing
	tempo = 140 -- Beats per minute
	period = 60 / tempo -- Beat interval in seconds
	-- Count rows; 1 beat = 4 rows
	row_counter = 0

	-- Modipulate
	modipulate.load()
	modipulate.open_file('../media/v-cf.it')
	modipulate.set_playing(true)
	modipulate.set_volume(1)
	modipulate.set_on_row_changed(exe_row)
	modipulate.set_on_pattern_changed(exe_pattern)

	-- TEST: Start in play mode
	level_in_progress = true
	init_cycle = true -- TODO: make this automatic from some init_play()


	-- Functions -- move later

	-- Get X/Y of the origin of a tile based on a tile coordinate
	-- E.g., 1,5 is column 1, row 5 (numbers start from 1)
	function tile2coord(col, row)
		local x = col * tile_size + grid.x
		local y = row * tile_size + grid.y
		return x, y
	end

--	-- Rotate a 2D table
--	-- args:
--	---- t: table to rotate
--	---- x: first column of section to rotate
--	---- y: first row of section to rotate
--	---- w: number of columns in section to rotate
--	---- h: number of rows in section to rotate
--	---- ccw: any value means counterclockwise; otherwise clockwise
--	function rotate(t, x, y, w, h, ccw)
--		-- Total rows & columns of original table
--		local trows = #t
--		local tcols = #t[1] -- WONT WORK WITH NIL --- fix
--		-- Initialize an array with inverted dimensions
--		local new_t = {}
--		for col in ipairs(t[1]) do new_t[col] = {} end
--		-- Rotate the values
--		for rowi,rowk in ipairs(t) do
--			for coli,colk in ipairs(rowk) do
--				if ccw then new_row = tcols + 1 - coli else new_row = coli end
--				if ccw then new_col = rowi else new_col = trows + 1 - rowi end
--				new_t[new_row][new_col] = colk
--			end
--		end
--		return new_t
--	end

end

----------------

function love.update(dt)

	modipulate.update()

end

----------------

function love.draw()

	-- Locals for great speed
	local grid = level_grid
	local grid_x = grid.x
	local grid_y = grid.y
	local grid_rows = grid.rows
	local grid_cols = grid.cols
	local grid_lines = grid.lines

	-- Draw the grid itself
	love.graphics.setColor(0x80, 0x80, 0x80)
	for i,line in ipairs(grid_lines) do
		love.graphics.line(line)
	end

	-- Draw the pieces
	love.graphics.setColor(0xFF, 0xFF, 0xFF)
	for row=1,grid_rows do
		-- For each tile in the level
		for col=1,grid_cols do
			-- Draw the frozen level piece, if there is one
			local color = grid[row][col]
			if color then
				love.graphics.draw(tile_images[color],
						grid_x + (col - 1) * tile_size,
						grid_y + (row - 1) * tile_size)
			end
			-- Draw the currently falling piece, if there is one
			color = cycle_grid[row][col]
			if color then
				love.graphics.draw(tile_images[color],
						grid_x + (col - 1) * tile_size,
						grid_y + (row - 1) * tile_size)
			end
		end
	end
	-- Draw the active piece
	if active_piece then
		for i,v in ipairs(active_piece) do
--			love.graphics.draw(tile_images[v.color],
--					grid_x + (v.x - 1) * tile_size,
--					grid_y + (v.y - 1) * tile_size)
			if v.color then
				love.graphics.draw(tile_images[v.color],
						grid_x + (v.x - 1) * tile_size,
						grid_y + (v.y - 1) * tile_size)
			end
		end
	end

end

----------------

function love.keypressed(k)

	-- Move the piece around
	if k == 'left' or k == 'right' then
		slide_queue = k
	elseif k == 'down' then
		-- TODO: accelerate piece descent? May not work well in this system
		-- Perhaps it can move the drop callback to the 8th note callback
		-- and switch it back to quarter note at the end of the round
		local dummy = nil
	-- Rotate piece
	elseif k == 'up'
	or k == 'x' then
		rotate_queue = 'clockwise'
	elseif k == 'z'
	or k == ' ' then
		rotate_queue = 'counterclockwise'
	-- Quit the game
	elseif k == 'escape' then
		love.event.push('q')
	end

end

----------------

function love.keyreleased(k)

end

----------------

function love.quit()

	modipulate.quit()

end

----------------

