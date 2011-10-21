-- Piece info

------------

-- Width/height of a tile (square for simplicity)
tile_size = 24

-- Colors
colorlist = {'red', 'green', 'blue', 'yellow', 'cyan', 'magenta'}

-- Shapes
shapelist = {
	-- Straight piece: 3x3
	{
		{false, true, false,},
		{false, true, false,},
		{false, true, false,}
	},
	-- "L" piece: 2x2
	{
		{true, false,},
		{true, true,}
	}
}

-- TEMP: This level's color list
level_colorlist = {'red', 'cyan', 'yellow'}

-- The tile colors and their images
tile_images = {}
for i,c in ipairs(colorlist) do
	-- E.g. tile_images['red'] = (red image object)
	tile_images[c] = love.graphics.newImage('media/tile_' .. c .. '.png')
end

------------

-- Create a new piece and return it
function new_piece()
	print('Creating new piece...')
	-- Choose piece shape, using random number
	-- from 1 to size of shape list
	local shape = shapelist[math.random(#shapelist)]
	-- Fill out the piece
	---- piece[n].row and .col are tile n's coords relative to the upper-left
	---- piece[n].color is tile n's color (or false for blank)
	---- piece.x and .y are the piece origin's coords on the level grid
	---- piece.w and .h are the with/height of piece
	local piece = {}


--	local i, w, h = 0, 0, 0
--	for row,val in ipairs(shape) do
--		if row > h then h = row end
--		for col,val in ipairs(shape[row]) do
--			if col > w then w = col end
--			-- If this tile isn't empty, i.e. has a color
--			if val then
--				-- Choose piece shape using random number
--				-- from 1 to size of color list
--				local color = colorlist[math.random(#colorlist)]
--				i = i + 1 -- Number of valid pices
--				piece[i] = { x = col, y = row, color = color }
--				print('col: ' .. col, 'row: ' .. row, 'color: ' .. color)
--			end
--			piece.w = w
--			piece.h = h
--		end
--	end


	for row in ipairs(shape) do
		for col,val in ipairs(shape[row]) do
			local color = false
			if val then
				color = level_colorlist[math.random(#level_colorlist)]
			end
			table.insert(piece, {x=col, y=row, row=row, col=col, color=color})
		end
	end
	piece.x = 1
	piece.y = 1
	piece.w = #shape[1]
	piece.h = #shape

	print('w: ' .. piece.w, 'h: ' .. piece.h)

	return piece
end

------------

-- Freeze the piece to the level grid
-- args:
---- p: the piece to freeze
function freeze(p)
	if not p then return end
	print('Freezing piece...')
	for i,v in ipairs(p) do
		-- Don't freeze "false" values
		if v.color then level_grid[v.y][v.x] = v.color end
	end
end

------------

-- Move the piece to an absolute location
-- args:
---- p: the piece to move
---- x: the x grid coord to move to
---- y: the y grid coord to move to
function move(p, x, y)
	if not p then return end
	p.x = x
	p.y = y
	for i,v in ipairs(p) do
		v.x = x + v.col - 1
		v.y = y + v.row - 1
	end
end

------------

-- Drop the piece down 1 step
-- args:
---- p: the piece to move down
function drop(p)
	if not p then return end
	for i,v in ipairs(p) do
		v.y = v.y + 1
	end
	p.y = p.y + 1
end

------------

-- Slide the piece left or right 1 step
-- args:
---- p: the piece to slide
---- dir: 'left' or 'right'
function slide(p, dir)
	if not p then return end
	if dir == 'left' then
		dir = -1
	elseif dir == 'right' then
		dir = 1
	else
		error('slide() must specify "left" or "right"')
	end
	for i,v in ipairs(p) do
		v.x = v.x + dir
	end
	p.x = p.x + dir
end

------------

-- Rotate the piece
-- args:
---- p: the piece to rotate
---- ccw: if present, rotate counterclockwise instead of clockwise
-- returns: a new table
function rotate(p, ccw)
	if not p then return end
	local copy = {}
	copy.x = p.x
	copy.y = p.y
	copy.w = p.h
	copy.h = p.w
	if ccw then
--		print('Rotating counterclockwise...')
		for i,v in ipairs(p) do
			copy[i] = {}
			copy[i].row = p.w - (v.col - 1)
			copy[i].col = v.row
			copy[i].x = copy[i].col + p.x - 1
			copy[i].y = copy[i].row + p.y - 1
			copy[i].color = v.color
		end
	else
--		print('Rotating clockwise...')
		for i,v in ipairs(p) do
			copy[i] = {}
			copy[i].row = v.col
			copy[i].col = p.h - (v.row - 1)
			copy[i].x = copy[i].col + p.x - 1
			copy[i].y = copy[i].row + p.y - 1
			copy[i].color = v.color
		end
	end
	return copy
end

------------

-- Look at the colors of surrounding tiles
-- Add colors to a chain
-- args:
---- x, y: the coordinates whose neighbors should be checked
---- chain: the chain to add to (otherwise start new chain)
-- returns: ????
function check_neighbors(x, y, chain)
	local chain = chain or {size=1, loc={}}
	chain.loc[tostring(x .. '_' .. y)] = {x=x, y=y}
	local color = level_grid[y][x]
	if not color then return chain end
	-- To the left
	if level_grid[y]
	and level_grid[y][x - 1] == color then
		local loc = tostring(x - 1 .. '_' .. y)
		if not chain.loc[loc] then
			chain.loc[loc] = {x=x-1, y=y}
			chain.size = chain.size + 1
			chain = check_neighbors(x - 1, y, chain)
		end
	end
	-- To the right
	if level_grid[y]
	and level_grid[y][x + 1] == color then
		local loc = tostring(x + 1 .. '_' .. y)
		if not chain.loc[loc] then
			chain.loc[loc] = {x=x+1, y=y}
			chain.size = chain.size + 1
			chain = check_neighbors(x + 1, y, chain)
		end
	end
	-- Above
	if level_grid[y - 1]
	and level_grid[y - 1][x] == color then
		local loc = tostring(x .. '_' .. y - 1)
		if not chain.loc[loc] then
			chain.loc[loc] = {x=x, y=y-1}
			chain.size = chain.size + 1
			chain = check_neighbors(x, y - 1, chain)
		end
	end
	-- Below
	if level_grid[y + 1]
	and level_grid[y + 1][x] == color then
		local loc = tostring(x .. '_' .. y + 1)
		if not chain.loc[loc] then
			chain.loc[loc] = {x=x, y=y+1}
			chain.size = chain.size + 1
			chain = check_neighbors(x, y + 1, chain)
		end
	end
	return chain
end

------------


