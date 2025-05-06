#include "mgm.hpp"
#include <chrono>
#include <cmath>
#include <unistd.h>
#include <cstdlib>

namespace mgm {

Button::Button(int x, int y, int w, int h, std::string lbl)
    : Widget(x, y, w, h), label(std::move(lbl)) {}

void Button::draw(cairo_t* cr) {
    if (!visible) return;

    cairo_save(cr);
    cairo_translate(cr, x + width / 2, y + height / 2);
    cairo_scale(cr, hover_scale * press_scale, hover_scale * press_scale);
    cairo_translate(cr, -width / 2, -height / 2);

    cairo_set_source_rgb(cr, 0.2, 0.6, 1.0);
    cairo_rectangle(cr, 0, 0, width, height);
    cairo_fill(cr);

    cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
    cairo_move_to(cr, 10, height / 2);
    cairo_show_text(cr, label.c_str());

    cairo_restore(cr);
}

bool Button::handle_event(XEvent& event) {
    if (!visible) return false;

    int mx = event.xbutton.x, my = event.xbutton.y;
    bool is_inside = (mx >= x && mx <= x + width && my >= y && my <= y + height);

    if (event.type == ButtonPress && event.xbutton.button == Button1 && is_inside) {
        is_pressed = true;
        return true;
    }

    if (event.type == ButtonRelease && event.xbutton.button == Button1 && is_inside) {
        is_pressed = false;
        Time current_time = event.xbutton.time;
        if (current_time - last_click_time < 250) { // это не работает не трогайте пожалуйста
            if (on_double_click) on_double_click();
        } else {
            if (on_click) on_click();
            if (!command.empty()) system(command.c_str());
        }
        last_click_time = current_time;
        return true;
    }

    return false;
}

void Button::animate(float delta) {
    hover_scale = 1.0f + 0.1f * std::sin(animation_time * 2.0f);
    animation_time += delta;

    if (is_pressed) {
        press_scale = std::max(0.9f, press_scale - 5.0f * delta);
    } else {
        press_scale = std::min(1.0f, press_scale + 5.0f * delta);
    }
}

TextField::TextField(int x, int y, int w, int h) : Widget(x, y, w, h) {}

void TextField::draw(cairo_t* cr) {
    if (!visible) return;

    cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
    cairo_rectangle(cr, x, y, width, height);
    cairo_fill(cr);

    cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
    cairo_rectangle(cr, x, y, width, height);
    cairo_stroke(cr);

    cairo_move_to(cr, x + 5, y + height / 2);
    cairo_show_text(cr, text.c_str());

    if (focused) {
        cairo_move_to(cr, x + 5 + text.length() * 8, y + height / 2);
        cairo_line_to(cr, x + 5 + text.length() * 8, y + height / 2 - 10);
        cairo_stroke(cr);
    }
}

bool TextField::handle_event(XEvent& event) {
    if (!visible) return false;

    if (event.type == ButtonPress && event.xbutton.button == Button1) {
        int mx = event.xbutton.x, my = event.xbutton.y;
        focused = (mx >= x && mx <= x + width && my >= y && my <= y + height);
        return focused;
    }
    if (event.type == KeyPress && focused) {
        char buf[32];
        KeySym keysym;
        XLookupString(&event.xkey, buf, sizeof(buf), &keysym, nullptr);
        if (keysym >= XK_space && keysym <= XK_asciitilde) {
            text += buf[0];
        } else if (keysym == XK_BackSpace && !text.empty()) {
            text.pop_back();
        }
        return true;
    }
    return false;
}

Label::Label(int x, int y, int w, int h, std::string txt)
    : Widget(x, y, w, h), text(std::move(txt)) {}

void Label::draw(cairo_t* cr) {
    if (!visible) return;

    cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
    cairo_move_to(cr, x + 5, y + height / 2);
    cairo_show_text(cr, text.c_str());
}

TabWidget::TabWidget(int x, int y, int w, int h) : Widget(x, y, w, h) {}

TabWidget& TabWidget::add_tab(const std::string& name) {
    tab_names.push_back(name);
    tab_widgets.emplace_back();
    return *this;
}

TabWidget& TabWidget::add_widget_to_tab(int tab_index, std::unique_ptr<Widget> widget) {
    if (tab_index >= 0 && tab_index < tab_widgets.size()) {
        tab_widgets[tab_index].push_back(std::move(widget));
    }
    return *this;
}

void TabWidget::draw(cairo_t* cr) {
    if (!visible) return;

    int tab_width = width / tab_names.size();
    for (size_t i = 0; i < tab_names.size(); ++i) {
        cairo_set_source_rgb(cr, i == active_tab ? 0.3 : 0.5, 0.5, 0.5);
        cairo_rectangle(cr, x + i * tab_width, y, tab_width, 30);
        cairo_fill(cr);

        cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
        cairo_move_to(cr, x + i * tab_width + 10, y + 20);
        cairo_show_text(cr, tab_names[i].c_str());
    }

    for (auto& widget : tab_widgets[active_tab]) {
        widget->draw(cr);
    }
}

bool TabWidget::handle_event(XEvent& event) {
    if (!visible) return false;

    bool tab_clicked = false;
    if (event.type == ButtonPress && event.xbutton.button == Button1) {
        int mx = event.xbutton.x, my = event.xbutton.y;
        int tab_width = width / tab_names.size();
        if (my >= y && my <= y + 30) {
            for (size_t i = 0; i < tab_names.size(); ++i) {
                if (mx >= x + i * tab_width && mx <= x + (i + 1) * tab_width) {
                    active_tab = i;
                    tab_clicked = true;
                    break;
                }
            }
        }
    }

    for (auto& widget : tab_widgets[active_tab]) {
        if (widget->handle_event(event)) return true;
    }

    return tab_clicked;
}

Window::Window(int width, int height, std::string title) {
    display = XOpenDisplay(nullptr);
    screen = DefaultScreen(display);
    win = XCreateSimpleWindow(display, RootWindow(display, screen), 10, 10, width, height, 1,
                              BlackPixel(display, screen), WhitePixel(display, screen));

    XSelectInput(display, win, ExposureMask | ButtonPressMask | ButtonReleaseMask | KeyPressMask);
    XMapWindow(display, win);
    XStoreName(display, win, title.c_str());

    surface = cairo_xlib_surface_create(display, win, DefaultVisual(display, screen), width, height);
    cr = cairo_create(surface);
}

Window::~Window() {
    cairo_destroy(cr);
    cairo_surface_destroy(surface);
    XDestroyWindow(display, win);
    XCloseDisplay(display);
}

Window& Window::add_widget(std::unique_ptr<Widget> widget) {
    widgets.push_back(std::move(widget));
    return *this;
}

Window& Window::set_theme(Theme t) {
    theme = t;
    return *this;
}

Window& Window::resize(int width, int height) {
    XResizeWindow(display, win, width, height);
    cairo_surface_destroy(surface);
    surface = cairo_xlib_surface_create(display, win, DefaultVisual(display, screen), width, height);
    cairo_destroy(cr);
    cr = cairo_create(surface);
    return *this;
}

Window& Window::add_hotkey(KeySym key, std::function<void()> callback) {
    hotkeys[key] = callback;
    return *this;
}

void Window::run() {
    auto last_time = std::chrono::steady_clock::now();
    while (running) {
        auto now = std::chrono::steady_clock::now();
        float delta = std::chrono::duration<float>(now - last_time).count();
        last_time = now;

        animate();
        handle_events();
        redraw();

        usleep(16666); // https://youtu.be/YehvctSIJ1w?si=Sr6OtNeD9GSdfuMb
    }
}

void Window::redraw() {
    cairo_set_source_rgb(cr, theme == Theme::Light ? 1.0 : 0.2, theme == Theme::Light ? 1.0 : 0.2, theme == Theme::Light ? 1.0 : 0.2);
    cairo_paint(cr);

    for (auto& widget : widgets) {
        widget->draw(cr);
    }
    cairo_surface_flush(surface);
}

void Window::handle_events() {
    while (XPending(display)) {
        XEvent event;
        XNextEvent(display, &event);
        if (event.type == Expose) {
            redraw();
        }
        for (auto& widget : widgets) {
            if (widget->handle_event(event)) break;
        }
        if (event.type == KeyPress) {
            KeySym keysym;
            XLookupString(&event.xkey, nullptr, 0, &keysym, nullptr);
            if (keysym == XK_Escape) running = false;
            auto it = hotkeys.find(keysym);
            if (it != hotkeys.end()) it->second();
        }
    }
}

void Window::animate() {
    for (auto& widget : widgets) {
        widget->animate(0.016f);
    }
}

} 
