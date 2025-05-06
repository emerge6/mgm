#ifndef MGM_HPP
#define MGM_HPP

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <cairo/cairo-xlib.h>
#include <string>
#include <vector>
#include <functional>
#include <memory>
#include <map>

namespace mgm {
// https://youtu.be/YehvctSIJ1w?si=Sr6OtNeD9GSdfuMb
enum class Theme { Light, Dark };

class Widget {
protected:
    int x, y, width, height;
    bool visible = true;
    std::function<void()> on_click;
    std::function<void()> on_double_click;

public:
    Widget(int x, int y, int w, int h) : x(x), y(y), width(w), height(h) {}
    virtual ~Widget() = default;

    virtual void draw(cairo_t* cr) = 0;
    virtual bool handle_event(XEvent& event) { return false; }

    Widget& set_visible(bool vis) { visible = vis; return *this; }
    Widget& set_on_click(std::function<void()> callback) { on_click = callback; return *this; }
    Widget& set_on_double_click(std::function<void()> callback) { on_double_click = callback; return *this; }
    bool is_visible() const { return visible; }
    void trigger_click() { if (on_click) on_click(); }
    virtual void animate(float delta) {}
};

class Button : public Widget {
    std::string label;
    float hover_scale = 1.0f;
    float press_scale = 1.0f;
    float animation_time = 0.0f;
    std::string command;
    bool is_pressed = false;
    Time last_click_time = 0;

public:
    Button(int x, int y, int w, int h, std::string lbl);
    void draw(cairo_t* cr) override;
    bool handle_event(XEvent& event) override;
    void animate(float delta) override;
    Button& set_text(const std::string& txt) { label = txt; return *this; }
    Button& set_command(const std::string& cmd) { command = cmd; return *this; }
};

class TextField : public Widget {
    std::string text;
    bool focused = false;

public:
    TextField(int x, int y, int w, int h);
    void draw(cairo_t* cr) override;
    bool handle_event(XEvent& event) override;
    std::string get_text() const { return text; }
    TextField& set_text(const std::string& txt) { text = txt; return *this; }
    TextField& set_focus(bool focus) { focused = focus; return *this; }
};

class Label : public Widget {
    std::string text;

public:
    Label(int x, int y, int w, int h, std::string txt);
    void draw(cairo_t* cr) override;
    Label& set_text(const std::string& txt) { text = txt; return *this; }
};

class TabWidget : public Widget {
    std::vector<std::string> tab_names;
    std::vector<std::vector<std::unique_ptr<Widget>>> tab_widgets;
    int active_tab = 0;

public:
    TabWidget(int x, int y, int w, int h);
    void draw(cairo_t* cr) override;
    bool handle_event(XEvent& event) override;
    TabWidget& add_tab(const std::string& name);
    TabWidget& add_widget_to_tab(int tab_index, std::unique_ptr<Widget> widget);
    int get_active_tab() const { return active_tab; }
};

class Window {
    Display* display;
    int screen;
    ::Window win;
    cairo_surface_t* surface;
    cairo_t* cr;
    Theme theme = Theme::Light;
    std::vector<std::unique_ptr<Widget>> widgets;
    bool running = true;
    float animation_time = 0.0f;
    std::map<KeySym, std::function<void()>> hotkeys;
// event pizda
public:
    Window(int width, int height, std::string title);
    ~Window();

    Window& add_widget(std::unique_ptr<Widget> widget);
    Window& set_theme(Theme t);
    Window& resize(int width, int height);
    Window& add_hotkey(KeySym key, std::function<void()> callback);
    void run();

private:
    void redraw();
    void handle_events();
    void animate();
};

} 

#endif
