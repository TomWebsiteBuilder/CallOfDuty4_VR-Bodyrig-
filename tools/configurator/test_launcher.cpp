#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shellapi.h>

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    (void)hInstance;
    (void)hPrevInstance;
    (void)lpCmdLine;
    (void)nCmdShow;

    SetEnvironmentVariableA("KISAK_VR_TEST_MODE", "1");
    ShellExecuteA(
        NULL,
        "open",
        "KisakCOD-sp.exe",
        "+set vr_testMode 1 +spdevmap killhouse +set thereisacow 1337",
        NULL,
        SW_SHOWNORMAL);

    return 0;
}
