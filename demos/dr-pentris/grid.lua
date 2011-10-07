-- The grid/playfield

require('piece')

-- One permanent one for the level
level_grid = {}
level_grid.rows = 14
level_grid.cols = 8
-- One temporary one for each drop cycle
cycle_grid = {}
-- Fill out the grid (plus a little extra)
local grid = level_grid
for row=1,level_grid.rows + 2 do
	-- Initialize
	grid[row] = {}
	cycle_grid[row] = {}
	-- Fill with false -- avoid nil so #grid still works
	for col=1,level_grid.cols do
		grid[row][col] = false
		cycle_grid[row][col] = false
	end
end
-- Redundant information about the grid
grid.w = grid.cols * tile_size
grid.h = grid.rows * tile_size
-- Coordinates of the grid's top-left corner (origin)
-- Calculate to put the grid in the center of the screen
grid.x = love.graphics.getWidth() / 2 - grid.w / 2
grid.y = love.graphics.getHeight() / 2 - grid.h / 2
-- All points of the grid's lines
-- (TODO: Set origin to 0,0 and offset at draw time)
grid.lines = {}
local lines = grid.lines
for row=0,grid.rows do -- Starting at 0 for extra line before row 1
	table.insert(lines,
			{grid.x, row * tile_size + grid.y, grid.w + grid.x,
			row * tile_size + grid.y})
end
for col=0,grid.cols do -- Starting at 0 for extra line before col 1
	table.insert(lines,
			{col * tile_size + grid.x, grid.y,
			col * tile_size + grid.x, grid.h + grid.y})
end

