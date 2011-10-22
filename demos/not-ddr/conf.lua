-- LOVE settings

function love.conf(t)
	t.title = 'Modipulate demo: Not DDR'
	t.author = 'hryx'
	t.identity = 'not-ddr'
	t.version = 0
	t.screen.width = 800
	t.screen.height = 600
	t.modules.joystick = false
	t.modules.audio = false
	t.modules.keyboard = true
	t.modules.event = true
	t.modules.image = true
	t.modules.graphics = true
	t.modules.timer = true
	t.modules.mouse = false
	t.modules.sound = false
	t.modules.physics = false
end

