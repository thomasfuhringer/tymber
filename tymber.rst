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

        :class:`Image` of type *icon*.

    .. attribute:: before_close

        Callback. If it returns *False* the window will stay open.

    .. method:: run()

        Show as dialog (modal).

    .. method:: close()

        Hide the window and, if run modal, return from *run*.
        

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

    A widget for very basic drawing.

    .. attribute:: on_paint

        Drawing operations should be done in a callback which is assigned here.

    .. attribute:: pen_color

        A tuple of (R, G. B) values to be used for drawing.

    .. attribute:: fill_color

        A tuple of (R, G. B) values to be used for filling rectangles.

    .. attribute:: text_color

        A tuple of (R, G. B) values to be used for drawing text.


    .. method:: point(x, y)

        Draws a point at the given coordinates.


    .. method:: move_to(x, y)

        Sets x, y as the origin of the next line operation.


    .. method:: line_to(x, y)

        Draws a line to x, y.


    .. method:: rectangle(x, y, width, height[, radius])

        Draws a rectangle. If *radius* is provided, corners will be rounded.


    .. method:: text(x, y, string)

        Writes *string* at given position.


    .. method:: repaint()

        Triggers the paint process.



.. _tymber-class-menu:

Menu
--------

.. class:: Menu(parent, key, caption[, icon])

    A (sub-)menu.

    *caption* must be a str to be displayed in the menu.

    .. attribute:: children

        A :class:`Dict` of Widgets contained.


.. _tymber-class-menu-item:

MenuItem
--------

.. class:: MenuItem(parent, key, caption, on_click[, icon])

    An item in a :class:`Menu` object.
    Can also be inserted into :class:`Toolbar`

    *caption* must be a str to be displayed in the menu,

    *on_click* is the callback to be triggered on selection
