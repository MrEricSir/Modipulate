-- The grid/playfield

require('piece')

grid = {}
grid.rows = 14
grid.cols = 8
-- Fill out the grid
local grid = grid
for row=1,grid.rows do
	grid[row] = {} -- 14 empty rows
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
