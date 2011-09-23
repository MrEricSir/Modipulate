-- Custom callbacks

require('piece')

----------------

-- exe_beat
-- Called on every beat

function exe_beat()

	-- Metronome
	play(sfx_metronome)

	-- Begin or continue a cycle (dropping a piece)
	if not cycle_in_progress then
		-- Create a new piece
		piece = new_piece()
		-- Put the piece on the grid
		local xoffset = 3
		local yoffset = 0
		for row=1,#piece do
			for col=1,#piece[row] do
				local tile = piece[row][col]
				if tile then
					grid[row + yoffset][col + xoffset] = tile
				end
			end
		end
		-- Flag on! Start the cycle at the next beat
		cycle_in_progress = true
	else

	-- Attempt to move the piece down one step; else freeze it
	-- TODO: check ok
			-- Checking should be pretty easy
			-- If there is a tile below any of this piece's tiles, it will fail
	-- TODO: if ok, move down
		--drop_table()???
	-- TODO: if no, freeze pice into grid and start next drop

	-- TODO: Process color sequences
	--
	-- For every colored tile, perform a check against adjacent tiles (UDLR).
	-- If a tile matches, perform a check against it too after the first one
	-- is finished (add to the tile processing queue). Keep a list of all
	-- matching tiles in this sequence, and if there are 5 or more, destroy
	-- them (with appropriate special effects, score adjustment, etc.).
	--
	-- Continue by checking the next colored tile, until the end of the grid.
	--
	-- Once all sequences are destroyed, apply gravity to what's left by
	-- dropping everything above blank rows. (Consider other gravity methods,
	-- like dropping everything above single blank spaces.) Since this will
	-- affect color sequences, process them again if things are dropped.
	--
	-- This whole series of events doesn't need to happen on *every* beat,
	-- just on ones where a piece has touched down and frozen. In such an
	-- event, activate the "color sequence process queue" (process_queue).
	-- That queue can be checked on the 1/4 note callback, 1/16 note, whenever.

	end

end

----------------

-- exe_eighth
-- Called twice every beat

function exe_eighth()

	-- Process the slide and rotate queues
	-- Attempt to slide the piece left or right
	if slide_queue == 'left' then
		-- TODO: check ok
				-- Checking should be pretty easy for both sliding and rotating
				-- If there is another tile inside this piece's matrix,
				-- sliding or rotating will fail
		-- if ok...
			-- TODO: slide piece
			-- Play slide sfx
			play(sfx_slide)
		-- end
		-- Empty the queue
		slide_queue = nil
	elseif slide_queue == 'right' then
		-- TODO: check ok
		-- if ok...
			-- TODO: slide piece
			-- Play slide sfx
			play(sfx_slide)
		-- end
		-- Empty the queue
		slide_queue = nil
	end
	-- Attempt to rotate piece array clockwise or counterclockwise
	if rotate_queue == 'clockwise' then
		-- TODO: check ok
		-- if ok...
			-- TODO: rotate piece
			-- Play rotate sfx
			play(sfx_rotate)
		-- end
		-- Empty the queue
		rotate_queue = nil
	elseif rotate_queue == 'counterclockwise' then
		-- TODO: check ok
		-- if ok...
			-- TODO: rotate piece
			-- Play rotate sfx
			play(sfx_rotate)
		-- end
		-- Empty the queue
		rotate_queue = nil
	end

end

----------------

-- exe_sixteenth
-- Called four times every beat

function exe_sixteenth()

end

