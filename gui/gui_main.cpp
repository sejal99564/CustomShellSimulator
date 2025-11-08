#include "imgui.h"
#include "backends/imgui_impl_win32.h"
#include "backends/imgui_impl_opengl3.h"
#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <GL/gl.h>
#pragma comment(lib, "opengl32.lib")

// ---------- Forward declarations ----------
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
void RunCommand(const char* cmd);

// ---------- Globals ----------
static char inputBuf[256] = "";
static ImGuiTextBuffer outputBuf;
static bool showExitMessage = false;

// ---------- Command execution ----------
void RunCommand(const char* cmd) {
    outputBuf.appendf(">> %s\n", cmd);

    FILE* pipe = _popen(cmd, "r");
    if (!pipe) {
        outputBuf.appendf("Error: cannot run command.\n");
        return;
    }
    char buffer[256];
    while (fgets(buffer, sizeof(buffer), pipe))
        outputBuf.append(buffer);
    _pclose(pipe);

    if (strcmp(cmd, "exit") == 0) {
        showExitMessage = true;
        outputBuf.appendf("\nGoodbye, Sejal!\n");
    }
}

// ---------- WinMain ----------
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, int)
{
    // --- Register window class ---
    WNDCLASSEX wc = {
        sizeof(WNDCLASSEX),
        CS_OWNDC,
        WndProc,
        0L, 0L,
        GetModuleHandle(NULL),
        NULL, NULL, NULL, NULL,
        _T("CustomShellClass"),
        NULL
    };
    RegisterClassEx(&wc);

    HWND hwnd = CreateWindow(
        wc.lpszClassName,
        _T("Custom Shell Simulator (GUI)"),
        WS_OVERLAPPEDWINDOW,
        100, 100, 900, 650,
        NULL, NULL, wc.hInstance, NULL
    );

    // --- Create OpenGL context ---
    HDC hdc = GetDC(hwnd);
    PIXELFORMATDESCRIPTOR pfd = {
        sizeof(PIXELFORMATDESCRIPTOR),
        1,
        PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
        PFD_TYPE_RGBA,
        32,
        0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0,
        24,
        8,
        0,
        PFD_MAIN_PLANE,
        0, 0, 0, 0
    };
    int pf = ChoosePixelFormat(hdc, &pfd);
    SetPixelFormat(hdc, pf, &pfd);
    HGLRC glrc = wglCreateContext(hdc);
    wglMakeCurrent(hdc, glrc);

    // --- Initialize ImGui ---
    ShowWindow(hwnd, SW_SHOWDEFAULT);
    UpdateWindow(hwnd);
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();

    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplOpenGL3_Init("#version 130");

    // --- Display welcome banner once ---
    outputBuf.append("============================================\n");
    outputBuf.append("   Welcome to Custom Shell Simulator\n");
    outputBuf.append("   Developed by: Sejal Saquib\n");
    outputBuf.append("============================================\n");
    outputBuf.append("Type 'help' to list built-in commands.\n\n");

    // --- Main loop ---
    bool done = false;
    while (!done)
    {
        MSG msg;
        while (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
                done = true;
        }
        if (done) break;

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        // --- GUI Window ---
        ImGui::Begin("Custom Shell Terminal", NULL, ImGuiWindowFlags_NoResize);

        // Header / Title
        ImGui::TextColored(ImVec4(0.3f, 0.8f, 1.0f, 1.0f), "Custom Shell Simulator (GUI)");
        ImGui::Separator();

        // Input Area
        ImGui::TextWrapped("Enter a command and click Run:");
        ImGui::InputText("##input", inputBuf, IM_ARRAYSIZE(inputBuf));
        ImGui::SameLine();
        if (ImGui::Button("Run")) {
            RunCommand(inputBuf);
            inputBuf[0] = 0;
        }
        ImGui::SameLine();
        if (ImGui::Button("Clear")) outputBuf.clear();

        ImGui::Separator();

        // Output Area
        ImGui::BeginChild("Output", ImVec2(0, 480), true);
        ImGui::TextUnformatted(outputBuf.begin());
        ImGui::EndChild();

        ImGui::Separator();

        // Exit button
        if (ImGui::Button("Exit")) {
            outputBuf.append("\nGoodbye, Sejal!\n");
            showExitMessage = true;
        }

        ImGui::End();

        // --- Render ---
        ImGui::Render();
        glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        SwapBuffers(hdc);

        // --- Exit handling ---
        if (showExitMessage)
            done = true;
    }

    // --- Cleanup ---
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(glrc);
    ReleaseDC(hwnd, hdc);
    DestroyWindow(hwnd);
    UnregisterClass(wc.lpszClassName, wc.hInstance);
    return 0;
}

// ---------- Windows message handler ----------
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    if (msg == WM_SIZE && wParam != SIZE_MINIMIZED)
        return 0;

    if (msg == WM_DESTROY) {
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProc(hWnd, msg, wParam, lParam);
}
