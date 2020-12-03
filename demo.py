# Demostration of Tymber, Thomas Führinger, 2020
import tymber as ty, pickle, datetime

def file_new_menu_item__on_click():
    global child_windows
    child_windows += 1
    mdi_window = ty.MdiWindow(mdi, "window " + str(child_windows), 10, 10, 540, 300, "Window " + str(child_windows))
    mdi.active_child = mdi_window
    if child_position:
        mdi_window.position = (child_position[0] + 14 * child_windows, child_position[1] + 14 * child_windows, child_position[2], child_position[3])
    tab = ty.Tab(mdi_window, "tab", 10, 10, -10, -10)
    tab_page_1 = ty.TabPage(tab, "page1", 15, 15, -15, -11, "Page 1")
    tab_page_2 = ty.TabPage(tab, "page2", caption="Page 2")
    splitter = ty.Splitter(tab_page_1, "splitter", 10, 10, -10, -10)
    splitter.position = -220
    #splitter.box1.frame = True
    #splitter.box2.frame = True
    
    ty.Label(splitter.box1, "label_dividend", 20, 20, 50, 20, "Dividend")
    ty.Entry(splitter.box1, "entry_dividend", 80, 20, -20, 20, data_type=float, format="{:,.2f}")
    ty.Label(splitter.box1, "label_divisor", 20, 44, 50, 20, "Divisor")
    ty.Entry(splitter.box1, "entry_divisor", 80, 44, -20, 20, "1", data_type=float, format="{:,.2f}")
    ty.Label(splitter.box1, "label_quotient", 20, 90, 50, 20, "Quotient")
    ty.Entry(splitter.box1, "entry_quotient", 80, 90, -20, 20, data_type=float, format="{:,.2f}").read_only = True
    splitter.box1.entry_dividend.tab_next = splitter.box1.entry_divisor
    splitter.box1.entry_divisor.tab_next = splitter.box1.entry_dividend
    combo_box_operation = ty.ComboBox(splitter.box1, "combo_box_operation", -170, -40, 70, 22, caption="Divide", data_type = int)
    combo_box_operation.append("Divide", 1)
    combo_box_operation.append("Multiply", 2)
    combo_box_operation.data = 1
    ty.Button(splitter.box1, "button_calculate", -90, -40, 70, 22, "Caclulate").on_click = button_calculate__on_click
    combo_box_operation.tab_next = splitter.box1.button_calculate    
    
    list_view = ty.ListView(splitter.box2, "list_view", 20, 20, -20, -60)
    list_view.columns = [["A", float, 50, "{:,.2f}"], ["◌", str, 20], ["B", float, 50, "{:,.2f}"], ["=", float, 50, "{:,.2f}"], ["Hidden", datetime.datetime, None]]
    #list_view.data = [[6.0, "/", 3.0, 2.0, datetime.datetime.now()], ]
    list_view.on_selection_changed = list_view__on_selection_changed
    
    ty.Label(splitter.box2, "label_timestamp", 20, -40, 70, 20, "Time Stamp")
    ty.Entry(splitter.box2, "entry_timestamp", 90, -40, -20, 20, data_type=datetime.datetime, format="{:%Y-%m-%d %H:%M:%S}").read_only = True

def button_calculate__on_click(button):
    try:
        op = "/"
        if button.parent.combo_box_operation.data == 1:
            button.parent.entry_quotient.data = button.parent.entry_dividend.data / button.parent.entry_divisor.data
        else:
            button.parent.entry_quotient.data = button.parent.entry_dividend.data * button.parent.entry_divisor.data
            op = "*"
        button.parent.parent.box2.list_view.add_row([button.parent.entry_dividend.data, op, button.parent.entry_divisor.data, button.parent.entry_quotient.data, datetime.datetime.now()], -1)
        app.window.status_bar.set_text(None)
    except Exception as exception:
            app.window.status_bar.set_text("Error: " + str(exception))
            button.parent.entry_quotient.data = None                
            
def list_view__on_selection_changed(list_view):
    list_view.parent.entry_timestamp.data = list_view.data[list_view.row][4]
    
def about_menu_item__on_click():
    window = ty.Window("About", width=340, height=240)
    ty.Label(window, "line1", 50, 50, -10, 22, "Demonstration of ")
    ty.Label(window, "line2", 50, 70, -10, 22, "Tymber GUI Library for Python, Version %d.%d.%d" % ty.version_info[:3])
    ty.Label(window, "line3", 50, 90, -10, 22, "Thomas Führinger, 2020 ")
    ty.Label(window, "line4", 50, 110, -10, 22, "Source: github.com/thomasfuhringer/tymber")
    window.run()

def load_state():
    pos_bytes = ty.get_setting("Tymber", "Demo", "WindowPos")
    if pos_bytes is not None:
        state = pickle.loads(pos_bytes)
        ty.app.window.position = state[0]
        ty.app.window.maximized = state[1]
        ty.app.window.mdi.maximized = state[2]
        global child_position
        child_position = state[3]
        if ty.app.window.mdi.active_child:
            ty.app.window.mdi.active_child.position = child_position

def save_state():
    position = ty.app.window.position
    main_maximized = ty.app.window.maximized
    mdi_maximized = ty.app.window.mdi.maximized
    if ty.app.window.mdi.active_child:
        global child_position
        child_position = ty.app.window.mdi.active_child.position
    ty.set_setting("Tymber", "Demo", "WindowPos", pickle.dumps((position, main_maximized, mdi_maximized, child_position)))

def window__before_close(self):
    save_state()
    return True

app = ty.Application(ty.Window("Tymber Demo", width=640, height=480, visible=False))
app.window.before_close = window__before_close
menu = ty.Menu(app, "main", "Main")
file_menu = ty.Menu(menu, "file", "File")
file_new_menu_item = ty.MenuItem(file_menu, "new", "New", file_new_menu_item__on_click, ty.Image(ty.StockIcon.file_new))
window_menu = ty.Menu(menu, "window", "Window")
help_menu = ty.Menu(menu, "help", "Help")
about_menu_item = ty.MenuItem(help_menu, "about", "About...", about_menu_item__on_click, ty.Image(ty.StockIcon.information))

tool_bar = ty.ToolBar(app.window)
tool_bar.append_item(file_new_menu_item)
ty.StatusBar(app.window)

mdi = ty.MdiArea(app.window, "mdi", 0, 0, 0, 0)
mdi.menu = window_menu
child_windows = 0
child_position = None
file_new_menu_item__on_click()
app.window.visible = True

load_state()
app.run()