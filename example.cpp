// https://youtu.be/YehvctSIJ1w?si=Sr6OtNeD9GSdfuMb
#include "mgm.hpp"
#include <iostream>
#include <fstream>

int main() {
    mgm::Window window(800, 600, "MGM Showcase");

    // Создаyние вкладки для демонстрации всех возможностей
    auto tabs = std::make_unique<mgm::TabWidget>(50, 50, 700, 500);
    tabs->add_tab("Widgets Demo")
        .add_tab("Command Execution")
        .add_tab("Tabs & Themes")
        .add_tab("Hotkeys & Resize")
        .add_tab("Double Click");
    auto* tabs_ptr = tabs.get();

    // 1 Вкладка  виджеты демо
    auto widget_label = std::make_unique<mgm::Label>(60, 100, 300, 30, "Enter text below:");
    tabs->add_widget_to_tab(0, std::move(widget_label));

    auto text_field = std::make_unique<mgm::TextField>(60, 140, 200, 30);
    auto* text_field_ptr = text_field.get();
    text_field->set_focus(true);
    tabs->add_widget_to_tab(0, std::move(text_field));

    auto widget_button = std::make_unique<mgm::Button>(60, 180, 150, 40, "Show Input");
    widget_button->set_on_click([text_field_ptr, tabs_ptr]() {
        std::string text = text_field_ptr->get_text();
        auto label = std::make_unique<mgm::Label>(60, 230, 300, 30, "You entered: " + text);
        tabs_ptr->add_widget_to_tab(tabs_ptr->get_active_tab(), std::move(label));
        std::cout << "Text entered: " << text << "\n";
    });
    tabs->add_widget_to_tab(0, std::move(widget_button));

    // Вкладка 2 Выполнение команд
    auto cmd_label = std::make_unique<mgm::Label>(60, 100, 300, 30, "Run system command:");
    tabs->add_widget_to_tab(1, std::move(cmd_label));

    auto cmd_field = std::make_unique<mgm::TextField>(60, 140, 200, 30);
    auto* cmd_field_ptr = cmd_field.get();
    tabs->add_widget_to_tab(1, std::move(cmd_field));

    auto cmd_button = std::make_unique<mgm::Button>(60, 180, 150, 40, "Execute");
    cmd_button->set_on_click([cmd_field_ptr]() {
        std::string cmd = cmd_field_ptr->get_text();
        if (!cmd.empty()) {
            std::system(cmd.c_str());
            std::cout << "Executed: " << cmd << "\n";
        }
    });
    tabs->add_widget_to_tab(1, std::move(cmd_button));

    auto ls_button = std::make_unique<mgm::Button>(220, 180, 150, 40, "List Files");
    ls_button->set_command("ls -l > files.txt");
    ls_button->set_on_click([]() { std::cout << "Listed files to files.txt\n"; });
    tabs->add_widget_to_tab(1, std::move(ls_button));

    // Вкладка 3 вкладки и тема
    auto theme_label = std::make_unique<mgm::Label>(60, 100, 300, 30, "Switch theme or add tab:");
    tabs->add_widget_to_tab(2, std::move(theme_label));

    bool is_dark = true;
    auto theme_button = std::make_unique<mgm::Button>(60, 140, 150, 40, "Switch to Light");
    auto* theme_button_ptr = theme_button.get();
    theme_button->set_on_click([&window, &is_dark, theme_button_ptr]() {
        is_dark = !is_dark;
        window.set_theme(is_dark ? mgm::Theme::Dark : mgm::Theme::Light);
        theme_button_ptr->set_text(is_dark ? "Switch to Light" : "Switch to Dark");
        std::cout << "Switched to " << (is_dark ? "Dark" : "Light") << " theme\n";
    });
    tabs->add_widget_to_tab(2, std::move(theme_button));

    auto add_tab_button = std::make_unique<mgm::Button>(220, 140, 150, 40, "Add New Tab");
    add_tab_button->set_on_click([tabs_ptr]() {
        static int tab_count = 3;
        tabs_ptr->add_tab("Tab " + std::to_string(++tab_count));
        auto label = std::make_unique<mgm::Label>(60, 100, 300, 30, "New tab content");
        tabs_ptr->add_widget_to_tab(tab_count - 1, std::move(label));
        std::cout << "Added tab: " << tab_count << "\n";
    });
    tabs->add_widget_to_tab(2, std::move(add_tab_button));

    // 4 вкладка хоткей и изменение размера 
    auto resize_label = std::make_unique<mgm::Label>(60, 100, 300, 30, "Resize window or use hotkeys:");
    tabs->add_widget_to_tab(3, std::move(resize_label));

    auto resize_button = std::make_unique<mgm::Button>(60, 140, 150, 40, "Toggle Size");
    bool is_large = false;
    resize_button->set_on_click([&window, &is_large]() {
        is_large = !is_large;
        window.resize(is_large ? 1000 : 800, is_large ? 800 : 600);
        std::cout << "Resized to " << (is_large ? "1000x800" : "800x600") << "\n";
    });
    tabs->add_widget_to_tab(3, std::move(resize_button));

    auto hotkey_label = std::make_unique<mgm::Label>(60, 180, 300, 30, "Press F1 to show message");
    auto* hotkey_label_ptr = hotkey_label.get();
    tabs->add_widget_to_tab(3, std::move(hotkey_label));

    window.add_hotkey(XK_F1, [hotkey_label_ptr]() {
        hotkey_label_ptr->set_text("Hotkey F1 pressed!");
        std::cout << "F1 hotkey triggered\n";
    });

    window.add_hotkey(XK_Return, [cmd_button = cmd_button.get()]() {
        if (cmd_button->is_visible()) {
            cmd_button->trigger_click();
        }
    });

    // Вкладка 5 Дабл клик который крашит программу
    auto double_click_label = std::make_unique<mgm::Label>(60, 100, 300, 30, "Double-click the button:");
    tabs->add_widget_to_tab(4, std::move(double_click_label));

    auto double_click_button = std::make_unique<mgm::Button>(60, 140, 150, 40, "Double Click Me");
    auto* double_click_label_ptr = double_click_label.get();
    double_click_button->set_on_double_click([double_click_label_ptr]() {
        double_click_label_ptr->set_text("Double-click detected!");
        std::cout << "Double-click triggered\n";
    });
    double_click_button->set_on_click([double_click_label_ptr]() {
        double_click_label_ptr->set_text("Single click detected");
        std::cout << "Single click triggered\n";
    });
    tabs->add_widget_to_tab(4, std::move(double_click_button));

    window.add_widget(std::move(tabs));
    window.set_theme(mgm::Theme::Dark);
    window.run();
    return 0;
}
