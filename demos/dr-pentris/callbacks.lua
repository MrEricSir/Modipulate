-- Custom callbacks

require('piece')

----------------

-- exe_pattern
-- Called on every pattern

function exe_pattern(pattern)

	print('Pattern: ' .. pattern)

end

----------------

-- exe_row
-- Called on every beat

function exe_row(row)

	-- Metronome
--	play(sfx_metronome)

--	print('Row: ' .. row)

	-- Time to play the game
	if level_in_progress then

		-- Number of rows between drops
		if love.keyboard.isDown('down') then
			descent_interval = 2
		else
			descent_interval = 4
		end

		---------------
		-- Every 4 rows
		---------------
		if row % descent_interval == 0 then

			-- No active piece: start a cycle
			if not active_piece then
				-- Create a new piece
				active_piece = new_piece()
				local start_x = 4
				local start_y = 1
				move(active_piece, start_x, start_y)
			-- Active piece is on the board: continue the cycle
			else
				local freeze_flag = false
				-- Check if the tiles below are clear; else freeze it
				for i,v in ipairs(active_piece) do
					if v.color then
						-- Reached the bottom
						if v.y >= level_grid.rows then
							freeze_flag = true
						-- There is another tile below
						elseif level_grid[v.y + 1][v.x] then
							freeze_flag = true
						end
					end
				end

				-- Time to freeze the piece on the board
				if freeze_flag then
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

					-- Whether to process color chains
					local process_chains_flag = true

					-- Process color sequences, dropping pieces if necessary
					while process_chains_flag do
						-- Find color chains
						for row in ipairs(level_grid) do
							for col,v in ipairs(level_grid[row]) do
								local chain = check_neighbors(col, row)
								-- Check the chain
								if chain.size >= 5 then
									print('Chain of ' .. chain.size)
									-- Destroy this chain before looping
									-- This will prevent re-processing the same colors
									for k,tile in pairs(chain.loc) do
										print('Block to destroy', 'x: ' .. tile.x, 'y: ' .. tile.y)
										level_grid[tile.y][tile.x] = false
										drop_pieces_flag = true
									end
								end
							end
						end
						-- Drop pieces that are hanging in the air
--						while drop_pieces_flag do
--							if 1 == 1 then
--								drop_pieces_flag = false
--							end
--						end
						process_chains_flag = false
					end
										
					


					-- This round is OVER
					active_piece = nil
				-- Not time to freeze; continue cycle
				else
					-- If we're still going, drop the piece down by 1 step
					drop(active_piece)
				end

			end -- // if not active_piece

		end -- // Every 4 rows

		---------------
		-- Every 2 rows
		---------------
		if row % 2 == 0 then

			if not active_piece then return end

			-- Attempt to slide the piece left or right
			if slide_queue == 'right' then
				for i,v in ipairs(active_piece) do
					if v.color then
						-- The level wall or a tile is to the right
						if v.x >= level_grid.cols
						or level_grid[v.y][v.x + 1] then
							slide_queue = nil
						end
					end
				end
			elseif slide_queue == 'left' then
				for i,v in ipairs(active_piece) do
					if v.color then
						-- The level wall or a tile is to the right
						if v.x <= 1
						or level_grid[v.y][v.x - 1] then
							slide_queue = nil
						end
					end
				end
			else
				slide_queue = nil
			end
			if slide_queue then
--				play(sfx_slide)
				slide(active_piece, slide_queue)
				slide_queue = nil
			end

			-- Attempt to rotate piece array clockwise or counterclockwise
			if rotate_queue == 'clockwise'
			or rotate_queue == 'counterclockwise' then
				if rotate_queue == 'counterclockwise' then
					local ccw = true
				end
				local copy = rotate(active_piece, ccw)
				-- We use a rotated test copy
				for i,v in ipairs(copy) do
					if v.color then
						-- A tile or the wall occupies this space
						if v.x < 1
						or v.x > level_grid.cols
						or v.y > level_grid.rows then
							rotate_queue = nil
						elseif level_grid[v.y]
						and level_grid[v.y][v.x] then
							rotate_queue = nil
						end
					end
				end
				-- If all went well
				if rotate_queue then
					active_piece = copy
				end
--				play(sfx_rotate)
				rotate_queue = nil
			end

		end -- // Every 2 rows

	end -- // if level_in_progress

end -- // function exe_row()


