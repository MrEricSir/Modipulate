-- Piece info

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

-- The tile colors and their images
tile_images = {}
for i,c in ipairs(colorlist) do
	-- E.g. tile_images['red'] = (red image object)
	tile_images[c] = love.graphics.newImage('media/tile_' .. c .. '.png')
end

-- Create a new piece and return it
function new_piece()
	print('Creating new piece...')
	-- Choose piece shape, using random number
	-- from 1 to size of shape list
	local shape = shapelist[math.random(#shapelist)]
	-- Fill out the piece
	---- piece.w and .h = with, height of piece
	---- piece[n].x and .y are tile n's coords
	---- piece[n].color is tile n's color
	local piece = {}
	local i, w, h = 0, 0, 0
	for row,val in ipairs(shape) do
		if row > h then h = row end
		for col,val in ipairs(shape[row]) do
			if col > w then w = col end
			-- If this tile isn't empty, i.e. has a color
			if val then
				-- Choose piece shape using random number
				-- from 1 to size of color list
				local color = colorlist[math.random(#colorlist)]
				i = i + 1 -- Number of valid pices
				piece[i] = { x = col, y = row, color = color }
				print('col: ' .. col, 'row: ' .. row, 'color: ' .. color)
			end
			piece.w = w
			piece.h = h
		end
	end
	print('w: ' .. w, 'h: ' .. h)
	return piece
end

-- Freeze the piece to the level grid
-- args:
---- p: the piece to freeze
function freeze(p)
	if not p then return end
	print('Freezing piece...')
	for i,v in ipairs(p) do
		-- Don't freeze "false" values
		if v then level_grid[v.y][v.x] = v.color end
	end
end

-- Drop the piece down 1 step
-- args:
---- p: the piece to move down
function drop(p)
	if not p then return end
	for i,v in ipairs(p) do
		v.y = v.y + 1
	end
end

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
end

