:mod:`tymber` --- GUI toolkit
===================================

.. module:: tymber
   :synopsis: A GUI toolkit on Windows

.. sectionauthor:: Thomas Führinger
.. moduleauthor:: Thomas Führinger <thomasfuhringer@live.com>

**Binaries:** https://github.com/thomasfuhringer/tymber/bin

**Source code:** https://github.com/thomasfuhringer/tymber

--------------

Tymber

Hello World
-----------

This simple program displays a window with a button that 
has a callback to close it.

    import tymber as ty

    def button__on_click(button):
        button.parent.close()

    app = ty.Application(ty.Window("Tymber", width=360, height=280))
    label = ty.Label(app.window, "label", 20, 60, -20, 22, "Hello World!")
    label.align_h = ty.Align.center
    button = ty.Button(app.window, "button", -70, -40, 50, 20, "Close")
    button.on_click = button__on_click
    app.run()


...

--------------

.. _tymber-module-contents:

Module Functions and Constants
==============================

.. function:: message(message[, title])

    Shows a message box displaying the string *message*,
    using the string *title* as window title.


.. function:: set_setting(key1, key2, subkey, value)

    Stores the byte object *value* as a configuration setting.


.. function:: get_setting(key1, key2, subkey)

    Retrieves the object stored using *set_setting*.
    
.. function:: play_sound([filename])

    Plays the .wav file in *filename* or the default chime.
    
.. function:: joystick_status([stick_no])

    Gets the status of joystick *stick_no* and returns a tuple
    *(xpos, ypos, zpos, rpos, upos, button1, button2, button3, button4)*.
    

.. data:: app

    Singleton instance of Application class.
    
.. data:: default_coordinate

    Value to assign to :attr:`top` or :attr:`left` coordinate of :class:`Window` to show it 
    using MS Windows' default positioning.
    
.. data:: center

    Value to assign to :attr:`top` or :attr:`left` coordinate of :class:`Window` to show it centered.


.. data:: version_info

    The version number as a tuple of integers.


.. data:: copyright

    Copyright notice.


Enumerations
------------

.. data:: Align

    :class:`Enum` for alignment of text in widgets, possible values:
    *left, right, center, top, bottom, block*

.. data:: StockIcon

    :class:`Enum` icons included:
    *file_open, file_new, save, ok, no, window, find*

.. data:: Key

    :class:`Enum` keyboad keys:
    *enter, tab, escape, delete, space, left, right, up, down, f1*



.. _tymber-classes:

Classes
=======

.. _tymber-class-application:

Application
-------

.. class:: Application(window)

    The `app` singleton. Only one instance can be created and it is globally available as `tymber.app`.

    *Attributes and methods*

    .. attribute:: window

        Main :class:`Window`

    .. method:: run()

        Call *run* on :attr:`window`.


.. _tymber-class-window:

Window
------

.. class:: Window([caption, left, top, width, height, visible])

    A GUI window which serves as a canvas to hold :class:`Widget` objects.

    *Attributes and methods*

    .. attribute:: caption

        A str to appear as the Windows's title.

   .. attribute:: left

      Distance from left edge of desktop, if negative from right.
      Default: *tymber.default_coordinate*

   .. attribute:: top

      Distance from top edge of desktop, if negative from bottom

   .. attribute:: width

      Width or, if zero or negative, distance of right edge from right edge
      of desktop

   .. attribute:: height

      Height or, if zero or negative, distance of bottom edge from bottom edge
      of desktop

    .. attribute:: visible

        By default :const:`True`.
        

    .. attribute:: children

        A :class:`Dict` of Widgets contained.

    .. attribute:: position

        A tuple with the *(x, y)* position of the widget on the screen.

    .. attribute:: size

        A tuple with the *(width, height)* of the widget on the screen.

    .. attribute:: icon

        :class:`Icon`
        
    .. attribute:: tool_bar

        :class:`ToolBar`  
        
    .. attribute:: status_bar

        :class:`StatusBar`

    .. attribute:: before_close

        Callback. If it returns *False* the window will stay open.
        
    .. attribute:: on_close

        Callback when closing.
        
    .. attribute:: focus

        Widget holding the focus.
        
    .. attribute:: on_focus_change

        Callback when focus was moved to new widget.
        
   .. attribute:: min_width

      Minimum width
      
   .. attribute:: min_height

      Minimum height

    .. method:: run()

        Show as dialog (modal).

    .. method:: close()

        Hide the window and, if run modal, return from *run*.
        (Does not destroy it).

    .. method:: set_timer(callback[, interval, wait])

        Sets a timer to fire after *wait* seconds every *interval* seconds.

    .. method:: del_timer()

        Deletes the timer.

    .. method:: key_pressed(key)

        Returns True if *key* is pressed.
        

.. _tymber-class-widget:

Widget
------

.. class:: Widget(parent, key[, left, top, width, height, caption, data_type, format, visible])

   Widget is the base class from which all data aware widgets that
   can be displayed on a :class:`Window` are derived.

   The construtor parameters are also available as attributes:

   .. attribute:: parent

      Can be a :class:`Window`, :class:`MdiWindow` or :class:`Widget` that holds the widget.

   .. attribute:: key

      The string that is used to reference the widget in the parent's `children` dict.

   .. attribute:: left

      Distance from left edge of parent, if negative from right.

   .. attribute:: top

      Distance from top edge of parent, if negative from bottom

   .. attribute:: width

      Width or, if zero or negative, distance of right edge from right edge
      of parent

   .. attribute:: height

      Height or, if zero or negative, distance of bottom edge from bottom edge
      of parent

   .. attribute:: caption

      Text used e.g. for :class:`Label` or :class:`Entry`


   .. attribute:: data_type

      The Python data type the widget can hold.


   .. attribute:: format

      The Python format string in the :meth:`str.format()` syntax that is used
      to render the data.

   .. attribute:: visible

      By default :const:`True`

   .. attribute:: enabled

      If :const:`False` widget is grayed.


.. _tymber-class-label:

Label
-----

.. class:: Label

    Shows boilerplate text on the :class:`Window`.

    .. attribute:: align_h

        :class:`Enum` Align of horizontal text alignment.

    .. attribute:: align_v

        :class:`Enum` Align of vertical text alignment.

    .. attribute:: text_color

        A tuple of RGB integers.


.. _tymber-class-entry:

Entry
-----

.. class:: Entry

    Single line data entry. After leaving the widget, the entered text is parsed and converted
    into a value of :attrib:`data_type` and available as :attrib:`data`.

    *Attributes and methods*

    .. attribute:: data

        The value the Entry holds.

    .. attribute:: read_only

        Editing not possible.

    .. attribute:: password

        Characters are not legible.

    .. attribute:: input_data

        The currently entered input converted to the Entry's data type,
        but not yet committed to :attrib:`data`,
        for validation purposes.

    .. attribute:: input_string

        The currently entered input string,
        but not yet committed to :attrib:`data`,
        for validation purposes.

    .. attribute:: align_horiz

        Horizontal alignment. By default tymber.Align.left for data type str and
        tymber.Align.right for numeric data types

    .. attribute:: on_key

        Callback when key is pressed

    .. attribute:: on_leave

        Callback, return True if ready for focus to move on.

    .. attribute:: multiline

        Characters are not legible.


.. _tymber-class-combobox:

ComboBox
--------

.. class:: ComboBox

    ComboBox which allows selection from a drop down list of data values.


    .. method:: append(value[, key])

        Appends o tuple of (*value*, *key*) to the list of available items.
        *key* is displayed in the widget, *value* returned as
        :attr:`data`
        If *key* is not given, *value* will be assumed.


.. _tymber-class-button:

Button
------

.. class:: Button

    Push button to trigger a callback.

    .. attribute:: on_click

        If a callable is assigned here it is called when the button is
        clicked.


.. _tymber-class-tab:

Tab
---

.. class:: Tab

    A tabbed notebook container.
    Holds :class:`TabPage` objects.


.. _tymber-class-tab-page:

TabPage
-------

.. class:: TabPage(parent)

    Page in a :class:`Tab` Widget.
    *parent* in contructor must be the :class:`Tab` object.


.. _tymber-class-box:

Box
---

.. class:: Box

    Container that can hold other Widgets.

    A :class:`Splitter` object holds two Box Widgets.

    .. attribute:: children

        A :class:`Dict` of Widgets contained.


.. _tymber-class-splitter:

Splitter
--------

.. class:: Splitter

    A widget with two adjustable panes. Each one is a
    :class:`Box` object.

    .. attribute:: box1

        :class:`Box` object on the left or top.

    .. attribute:: box2

        :class:`Box` object on the right or bottom.

    .. attribute:: vertical

        If :const:`True` panes are arranged top/bottom.

    .. attribute:: position

        Position of the separator, if negative from left or bottom

    .. attribute:: spacing

        Width of the separator


.. _tymber-class-canvas:

Canvas
--------

.. class:: Canvas

    A widget for basic drawing. It sports two buffers (index 0 and 1). 
    By default buffer 0 is active and live.


    .. method:: set_pen([red, green, blue, alpha, width])

        Color and thickness to be used for drawing and filling.
        Default: (0, 0, 0, 255, 1)


    .. method:: point(x, y)

        Draws a point at the given coordinates.


    .. method:: line(x1, y1, x2, y2)

        Draws a line from *x1*, *y1* to *x2*, *y2*


    .. method:: rectangle(x, y, width, height[, fill])

        Draws a rectangle. 


    .. method:: ellipse(x, y, width, height[, fill])

        Draws an ellipse.


    .. method:: text(x, y, x2, y2, string)

        Writes *string* into the given rectangle.


    .. method:: image(x, y, data[, width, height])

        Puts an image at the given position. If width or height is given, the image will be scaled retaining aspect ratio.
        data is to be either a 'str' holding file name or 'bytes' holding the data in memory.
        

    .. method:: resize_buffer([index, width, height, x, y])

        Changes the size of the buffer. If *index* is not given active_buffer will be used. 
        If *width* and *height* are not given the canvas' are applied.
        If given the current content will be moved to *x* and *y*.
        

    .. method:: copy_buffer(index1, index1)

        Copy the content of buffer 1 to 2.
        

    .. method:: clear_buffer([index])

        Clears the buffer. If index is not given, the active_buffer will be used.
        

    .. method:: renew_buffer([index, width, height])

        Creates a new buffer. If index is not given active_buffer will be used. 
        If width and height are not given the canvas' are applied.
        
        
    .. method:: refresh()

        Triggers the paint process.
        

    .. attribute:: on_resize

        Callback when the widget gets resized.
        Parameters: widget
        

    .. attribute:: active_buffer

        Index of the buffer to be used for drawing operations. Default: 0
        

    .. attribute:: live_buffer

        Index of the buffer to be displayed when the OS refreshes the screen. Default: 0
        

    .. attribute:: on_mouse_move

        Callback when the mouse is moved.
        Parameters: widget, x, y
        

    .. attribute:: on_mouse_wheel

        Callback when the left mouse wheel was turned.
        Parameters: widget, delta, x, y
        

    .. attribute:: on_l_button_down

        Callback when the left mouse button is pressed.
        Parameters: widget, x, y
        

    .. attribute:: on_l_button_up

        Callback when the left mouse button is released.
        Parameters: widget, x, y
        

    .. attribute:: anti_alias

        Drawing operations should be done in a callback which is assigned here.


.. _tymber-class-menu:

Menu
--------

.. class:: Menu(parent, key, caption[, icon])

    A (sub-)menu.

    *caption* must be a str to be displayed in the menu.

    .. attribute:: children

        A :class:`Dict` of Widgets contained.
        
    .. method:: append_separator()


.. _tymber-class-menu-item:

MenuItem
--------

.. class:: MenuItem(parent, key, caption, on_click[, icon])

    An item in a :class:`Menu` object.
    Can also be inserted into :class:`Toolbar`

    *caption* must be a str to be displayed in the menu,

    *on_click* is the callback to be triggered on selection

    .. attribute:: enabled

        If False the MenuItem is grayed.    


.. _tymber-class-tool-bar:

ToolBar
--------

.. class:: ToolBar(window)

    Tool bar attached to *window*.

    .. attribute:: children

        A :class:`Dict` of items contained.

    .. method:: append_item(item)

        *item* must be a :class:`MenuItem`

    .. method:: append_separator()
    
        Adds a separator line

    .. method:: set_enabled(item, enabled)

        *item* must be a :class:`MenuItem`. If *enabled* is False the button is grayed.
      
        
        
.. _tymber-class-icon:

Icon
--------

.. class:: Icon(source)

    An icon to be used in :class:`MenuItem`, :class:`Toolbar` and :class:`Window` objects.

    *source* must be a 'str' of the file name or 'bytes' holding the data.
    
    

.. _tymber-class-status-bar:

StatusBar
--------

.. class:: StatusBar(window[, borders])

    Status bar attached to *window*. *borders* is a :class:`List` of 'int' defining the lengths of parts if more than one are to be used.

    .. method:: set_text(text[, part])

        *part* is an int.

    .. method:: set_text(part)
    
    

.. _tymber-class-image-view:

ImageView
--------

.. class:: ImageView()

    Displays a bitmap.
    :attr:`data` must be a 'str' of the file name or 'bytes' holding the data.
    If :attr:`stretch` is True it will be stretched keeping aspect ratio.
    

.. _tymber-class-list-view:

ListView
--------

.. class:: ListView()

    Displays a list of data in a table.


    .. attribute:: data

        List that holds the data
        
    .. attribute:: columns

        List of Lists that define the columns.
        Format: [caption, data_type, width, data_format] if *width* is *None* the the column is hidden.
        
    .. attribute:: row

        Index of row currently selected, or *None*.

    .. attribute:: on_selection_changed

        Callback when a row was selected.
        Parameters: widget

    .. attribute:: on_double_click

        Callback when a row was double clicked.
        Parameters: widget, index


    .. method:: add_row(data[, index])

        Adds a row with *data* after *index* . If *index* is not given :attr:`row` will be used. 
        In case :attr:`row` is *None* the new row will be appended.
        
    .. method:: update_row(data[, index])

        Replaces the row with *data*. If *index* is not given :attr:`row` will be used.
        
    .. method:: delete_row(index)

        Removes the row at *index*. 
        
        
.. _tymber-class-file-selector:

FileSelector
--------

.. class:: FileSelector( caption[, path])

    File selection dialog. *path* is the initial path.
    
    .. method:: run()

        Show it modal. Returns the file name if selected or None.
        
        
.. _tymber-class-mdi-area:

MdiArea
--------

.. class:: MdiArea()

    Can hold :class:`MdiWindow` objects.
    
    .. method:: tile()

        Arrange child windows in tile format.
        
    .. attribute:: active_child

        Child window that has the focus
        
    .. attribute:: on_activated

        Callback when new child was activated
        
    .. attribute:: maximized

        Child windows are shown maximized        
        
    .. attribute:: menu

        Menu to hold windows
    
    
.. _tymber-class-mdi-window:

MdiWindow
--------

.. class:: MdiWindow()

    A window to be displayed inside an :class:`MdiArea`
        
    .. attribute:: parent

        MdiArea that holds the MdiWindow      

    .. method:: close()

        Remove the window from the parent's *children* collection (and consequently destroy it).