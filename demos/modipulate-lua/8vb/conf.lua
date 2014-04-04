-- LOVE settings

function love.conf(t)
	t.title = '8vb'
	t.author = 'hryx & MrEricSir'
	t.identity = '8vb'
	t.version = '0.9.0'
	t.window.width = 360
	t.window.height = 500
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

