#include <Windows.h>

class Window {
    public:
        Window();
        // initialize the window
        bool init();
        bool broadcast();
        // release the window
        bool release();
        bool isRunning();

        // EVENTS
        virtual void onCreate() = 0;
        virtual void onUpdate() = 0;
        virtual void onDestroy() = 0;

        ~Window();
    protected:
        HWND m_hwnd;
        bool m_is_running;
    };