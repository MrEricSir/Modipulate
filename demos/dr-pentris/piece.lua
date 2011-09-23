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
	print('random shape', shape)
	-- Apply the chosen color to the shape array's "true" values
	local piece = {}
	for row,val in ipairs(shape) do
		piece[row] = {}
		for col,val in ipairs(shape[row]) do
			if val then
				-- Choose piece shape using random number
				-- from 1 to size of color list
				local color = colorlist[math.random(#colorlist)]
				print('random color', color)
				piece[row][col] = color
			else
				piece[row][col] = false -- Not to be nil so #piece always ok
			end
		end
	end
	return piece
end

