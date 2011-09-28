-- "Dr. Pentris"
-- Tetris clone using new audio engine

----------------

function love.load()

	-- SEEEED
	math.randomseed(os.time() / 3)

	-- Libraries and source files
	require('audio')
	require('callbacks')
	require('piece')
	require('grid')

	-- Local copies
	local grid = level_grid

	-- Timing
	tempo = 140 -- Beats per minute
	period = 60 / tempo -- Beat interval in seconds
	-- Beat counters
	t_beat = 0 -- Timer for 1/4 notes; always start at 0
	t_eighth = 0 -- Timer for 1/8 notes
	t_sixteenth = 0 -- Timer for 1/16 notes
	-- For display
	beat_text = ' '
	eighth_text = ' '
	sixteenth_text = ' '

	-- Functions -- move later

	-- Get X/Y of the origin of a tile based on a tile coordinate
	-- E.g., 1,5 is column 1, row 5 (numbers start from 1)
	function tile2coord(col, row)
		local x = col * tile_size + grid.x
		local y = row * tile_size + grid.y
		return x, y
	end

	-- Shift the values in a 2D table down 1 step
	-- args:
	---- t: table to move
	function shift(t)
		-- For each column between the second to last row
		-- and the first, in reverse...
		for row=(#t-1),1,-1 do
			for col=1,#t[row] do
				-- ...give the next highest row the current value
				t[row + 1][col] = t[row][col]
				t[row][col] = false -- So nothing is left behind
			end
		end
		-- The last row is skipped to avoid increasing the table's length.
	end

	-- Rotate a 2D table
	-- args:
	---- t: table to rotate
	---- x: first column of section to rotate
	---- y: first row of section to rotate
	---- w: number of columns in section to rotate
	---- h: number of rows in section to rotate
	---- ccw: any value means counterclockwise; otherwise clockwise
	function rotate(t, x, y, w, h, ccw)
		-- Total rows & columns of original table
		local trows = #t
		local tcols = #t[1] -- WONT WORK WITH NIL --- fix
		-- Initialize an array with inverted dimensions
		local new_t = {}
		for col in ipairs(t[1]) do new_t[col] = {} end
		-- Rotate the values
		for rowi,rowk in ipairs(t) do
			for coli,colk in ipairs(rowk) do
				if ccw then new_row = tcols + 1 - coli else new_row = coli end
				if ccw then new_col = rowi else new_col = trows + 1 - rowi end
				new_t[new_row][new_col] = colk
			end
		end
		return new_t
	end

	-- TEST: Start in play mode
	level_in_progress = true
	init_cycle = true -- TODO: make this automatic from some init_play()

end

----------------

function love.update(dt)

	-- The beat
	t_beat = t_beat + dt
	t_eighth = t_eighth + dt
	t_sixteenth = t_sixteenth + dt
	-- Trigger the 1/4 note callback
	if t_beat >= period then
		exe_beat()
		-- Beat display
		beat_text = 'x'
		-- Reset the timers
		t_beat = 0
		t_eighth = 0
		t_sixteenth = 0
	else
		beat_text = ' '
	end
	-- Trigger the 1/8 note callback
	if t_eighth == 0 or t_eighth >= period / 2 then
		exe_eighth()
		-- Eighth note display
		eighth_text = 'x'
		-- Reset the timers
		t_eighth = 0
		t_sixteenth = 0
	else
		eighth_text = ' '
	end
	-- Trigger the 1/16th note callback
	-- Just for demonstration purposes
	-- This implementation inside LOVE obviously won't work too well
	if t_sixteenth == 0 or t_sixteenth >= period / 4 then
		exe_sixteenth()
		-- Sixteenth note display
		sixteenth_text = 'x'
		-- Reset the timer
		t_sixteenth = 0
	else
		sixteenth_text = ' '
	end

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
			love.graphics.draw(tile_images[v.color],
					grid_x + (v.x - 1) * tile_size,
					grid_y + (v.y - 1) * tile_size)
		end
	end

	-- TEST: print the beat
	love.graphics.print('Beat:', 20, 20)
	love.graphics.print(sixteenth_text, 60, 20)
	love.graphics.print(eighth_text, 70, 20)
	love.graphics.print(beat_text, 80, 20)

end

----------------

function love.keypressed(k)

	-- Move the piece around
	if k == 'left' or k == 'right' then
		slide_queue = k
	elseif k == 'down' then
		-- TODO: accelerate piece descent? May not work well in this system
		local dummy = nil
	-- Rotate piece
	elseif k == 'up'
	or k == 'x'
	or k == ' ' then
		rotate_queue = 'clockwise'
	elseif k == 'z' then
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

end

----------------

