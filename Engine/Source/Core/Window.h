#ifndef WINDOW_H
#define WINDOW_H

#include <SDL3/SDL.h>
#include <string>

namespace SquareCore {

// Window properties
struct WindowProps {
    unsigned int Width;
    unsigned int Height;
    std::string Title;

    // Constructor with default settings
    WindowProps(unsigned int width = 1920, unsigned int height = 1080, std::string title = "Square")
        : Width(width), Height(height), Title(title)
    {

    }
};

class Window {
public:
    Window(WindowProps props = WindowProps());
    ~Window();

    // Sets the pointer to the underlying SDL window
    void SetNativeWindow(SDL_Window* window) { windowRef = window; }

    // Returns a pointer to the underlying SDL window
    SDL_Window* GetNativeWindow() const { return windowRef; }
    // Returns the width of the application window
    unsigned int& GetWidth() { return winProps.Width; }
    // Returns the height of the application window
    unsigned int& GetHeight() { return winProps.Height; }
    // Returns the title of the application window
    std::string& GetTitle() { return winProps.Title; }

private:
    // Internal pointer to the underlying SDL window
    SDL_Window* windowRef = nullptr;
    // Struct of properties for the application window
    WindowProps winProps;
};

}

#endif
