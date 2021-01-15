# Animation of bouncing ball
# © 2021 by Thomas Führinger <thomasfuhringer@live.com>
# The Tymber Python library is available here: https://github.com/thomasfuhringer/tymber

import tymber as ty, pickle
version = "0.9"

def button_go__on_click(button):
    ball.position = app.window.entry_init.data
    ball.velocity = 0
    global running
    running = False
    animation_start_menu_item__on_click()

def animation_start_menu_item__on_click():
    global running
    if running:
        app.window.del_timer()
        running = False
    else:
        app.window.set_timer(iterate, 1 / app.window.entry_frame.data)
        running = True

def canvas__on_resize(canvas):
    global height, width, center, pad_x
    canvas.renew_buffer()
    width, height = canvas.size
    center = int(width / 2)
    pad_x = center - 30

def entry__on_key(key, entry):
    if key == ty.Key.enter and entry.commit():
        button_go__on_click(None)

def load_state():
    pos_bytes = ty.get_setting("Tymber", "Animation", "WindowPos")
    if pos_bytes is not None:
        state = pickle.loads(pos_bytes)
        ty.app.window.position = state[0]
        ty.app.window.maximized = state[1]

def save_state():
    position = ty.app.window.position
    main_maximized = ty.app.window.maximized
    ty.set_setting("Tymber", "Animation", "WindowPos", pickle.dumps((position, main_maximized)))

def window__before_close(self):
    save_state()
    return True

def iterate():
    global pad_x
    if app.window.key_pressed(ty.Key.left) and pad_x > 1:
        pad_x -= 2
    if app.window.key_pressed(ty.Key.right) and pad_x < width - 60:
        pad_x += 2

    canvas.clear_buffer()
    canvas.set_pen(0, 120, 240)
    canvas.ellipse(center - 15, height - int((height - 60) * ball.position / 100) - 40, 30, 30, True)
    canvas.set_pen(100, 50, 0, width=10)
    canvas.line(pad_x, height - 5, pad_x + 60, height - 5)
    canvas.refresh()
    ball.iterate()


class Ball():
    def __init__(self, position):
        self.position = position
        self.velocity = 0

    def iterate(self):
        direction = (1, -1)[self.velocity < 0]
        self.velocity -= int((self.velocity**2) * app.window.entry_drag.data * direction / 600)
        self.velocity -= app.window.entry_gravity.data / 50
        self.position += self.velocity
        if self.position < 0 and abs(center - pad_x - 30) < 36:
            self.position = -self.position
            self.velocity *= -1


app = ty.Application(ty.Window("Bouncer", width=750, height=550))
app.window.before_close = window__before_close

menu = ty.Menu(app, "main", "Main")
animation_menu = ty.Menu(menu, "animation", "Animation")
ty.MenuItem(animation_menu, "start", "Start / Stop", animation_start_menu_item__on_click, ty.Icon(ty.StockIcon.refresh))
tool_bar = ty.ToolBar(app.window)
tool_bar.append_item(animation_menu.start)

x = -150
x2 = -80
y = 10
ty.Label(app.window, "label_init", x, y, 70, 20, "Intital Height")
ty.Entry(app.window, "entry_init", x2, y, 60, 20, "100", data_type=float).on_key = entry__on_key
y += 30
ty.Label(app.window, "label_gravity", x, y, 70, 20, "Gravity")
ty.Entry(app.window, "entry_gravity", x2, y, 60, 20, "9.807", data_type=float).on_key = entry__on_key
y += 24
ty.Label(app.window, "label_drag", x, y, 70, 20, "Drag")
ty.Entry(app.window, "entry_drag", x2, y, 60, 20, "10", data_type=float).on_key = entry__on_key
y += 30
ty.Label(app.window, "label_frame", x, y, 70, 20, "Frame Rate")
ty.Entry(app.window, "entry_frame", x2, y, 60, 20, "25", data_type=int).on_key = entry__on_key

y += 30
ty.Button(app.window, "button_go", x2, y, 60, 20, "Restart").on_click = button_go__on_click

canvas = ty.Canvas(app.window, "canvas", 0, 0, -170, 0)
canvas.anti_alias = True
canvas.on_resize = canvas__on_resize

load_state()

running = False
ball = Ball(app.window.entry_init.data)

width, height, center, pad_x = 0, 0, 0, 0
canvas__on_resize(canvas)
animation_start_menu_item__on_click()
app.run()