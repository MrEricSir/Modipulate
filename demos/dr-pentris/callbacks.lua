-- Custom callbacks

require('piece')

----------------

-- exe_beat
-- Called on every beat

function exe_beat()

	-- Metronome
	play(sfx_metronome)

	-- Time to play the game
	if level_in_progress then

		-- No active piece: start a cycle
		if not active_piece then
			-- Create a new piece
			active_piece = new_piece()
			-- Put the piece on the cycle grid
			local xoffset = 3
			local yoffset = 0
			for i,v in ipairs(active_piece) do
				v.x = v.x + xoffset
				v.y = v.y + yoffset
			end
		-- Active piece is on the board: continue the cycle
		else
			local freeze_flag = false
			-- Check if the tiles below are clear; else freeze it
			for i,v in ipairs(active_piece) do
				-- Reached the bottom
				if v.y >= level_grid.rows then
					freeze_flag = true
				-- There is another tile below
				elseif level_grid[v.y + 1][v.x] then
					freeze_flag = true
				end
			end
			if freeze_flag then
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

				-- Do the freezing
				freeze(active_piece)
				active_piece = nil
			else
				-- If we're still going, drop the piece down by 1 step
				drop(active_piece)
			end -- if freeze_flag

		end -- if not active_piece

	end -- if level_in_progress

end

----------------

-- exe_eighth
-- Called twice every beat

function exe_eighth()

	if active_piece then
		-- Process the slide and rotate queues
		-- Attempt to slide the piece left or right
		if slide_queue == 'right' then
			for i,v in ipairs(active_piece) do
				-- The level wall or a tile is to the right
				if v.x >= level_grid.cols
				or level_grid[v.y][v.x + 1] then
					slide_queue = nil
				end
			end
		elseif slide_queue == 'left' then
			for i,v in ipairs(active_piece) do
				-- The level wall or a tile is to the right
				if v.x <= 1
				or level_grid[v.y][v.x - 1] then
					slide_queue = nil
				end
			end
		else
			slide_queue = nil
		end
		if slide_queue then
			play(sfx_slide)
			slide(active_piece, slide_queue)
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

	end -- if active_piece

end

----------------

-- exe_sixteenth
-- Called four times every beat

function exe_sixteenth()

end

