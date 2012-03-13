-- LOVE settings

function love.conf(t)
	t.title = 'Dr. Pentris'
	t.author = 'hryx & MrEricSir'
	t.identity = 'dr_pentris'
	t.version = 0
	t.screen.width = 360
	t.screen.height = 500
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

