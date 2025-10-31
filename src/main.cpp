// ESP + Aimbot Base - Offline CS2 Dev Tool
// Copyright (c) 2025 - Open Source for Educational Use
// ONLY FOR OFFLINE, PRIVATE GAMES YOU OWN

#include <windows.h>
#include <tlhelp32.h>
#include <thread>
#include <chrono>
#include <mutex>
#include <atomic>
#include <vector>
#include <string>
#include <iostream>
#include <cmath>
#include <gdiplus.h>
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "kernel32.lib")
#pragma comment(lib, "gdiplus.lib")

using namespace Gdiplus;

// UPDATE THESE WITH cs2-dumper AFTER EVERY CS2 UPDATE
namespace offsets {
    constexpr uintptr_t dwEntityList = 0x1D07A80;
    constexpr uintptr_t dwLocalPlayerController = 0x1BE2D10;
    constexpr uintptr_t dwViewMatrix = 0x1E25F30;
    constexpr uintptr_t m_hPawn = 0x7BC;
    constexpr uintptr_t m_iTeamNum = 0x3EB;
    constexpr uintptr_t m_iHealth = 0x34C;
    constexpr uintptr_t m_pGameSceneNode = 0x330;
    constexpr uintptr_t m_modelState = 0x170;
    constexpr uintptr_t m_boneArray = 0x80;
    constexpr uintptr_t m_vecAbsOrigin = 0xD0;
    constexpr uintptr_t m_hPlayerPawn = 0x2300;
}

struct Vector3 { float x, y, z; };
struct Vector2 { float x, y; };
struct Matrix4 { float m[16]; };
struct PlayerInfo {
    Vector3 head, feet;
    float health;
    int team;
};

HANDLE hProcess = nullptr;
HWND hCS2 = nullptr, hOverlay = nullptr;
uintptr_t client = 0;
Matrix4 viewMatrix;
RECT rect;
std::vector<PlayerInfo> players;
std::mutex mtx;

std::atomic<bool> enabled = true, espOn = true, aimOn = false;
float fov = 90.0f, smooth = 0.5f;
Vector3 localHead = {};
int localTeam = 0;

template<typename T> T Read(uintptr_t addr) {
    T v; ReadProcessMemory(hProcess, (LPCVOID)addr, &v, sizeof(T), nullptr); return v;
}

bool W2S(const Vector3& pos, Vector2& out) {
    float w = pos.x * viewMatrix.m[3] + pos.y * viewMatrix.m[7] + pos.z * viewMatrix.m[11] + viewMatrix.m[15];
    if (w < 0.1f) return false;
    float x = pos.x * viewMatrix.m[0] + pos.y * viewMatrix.m[4] + pos.z * viewMatrix.m[8] + viewMatrix.m[12];
    float y = pos.x * viewMatrix.m[1] + pos.y * viewMatrix.m[5] + pos.z * viewMatrix.m[9] + viewMatrix.m[13];
    out.x = (rect.right / 2.0f) * (1 + x / w);
    out.y = (rect.bottom / 2.0f) * (1 - y / w);
    return true;
}

bool FindCS2() {
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    PROCESSENTRY32 pe{ sizeof(pe) };
    if (Process32First(snap, &pe)) {
        do {
            if (strcmp(pe.szExeFile, "cs2.exe") == 0) {
                hProcess = OpenProcess(PROCESS_VM_READ | PROCESS_VM_OPERATION, FALSE, pe.th32ProcessID);
                if (hProcess) {
                    MODULEINFO mi;
                    HMODULE hMod = GetModuleHandleA("client.dll");
                    if (!hMod) return false;
                    GetModuleInformation(hProcess, hMod, &mi, sizeof(mi));
                    client = (uintptr_t)mi.lpBaseOfDll;
                    hCS2 = FindWindowA(NULL, "Counter-Strike 2");
                    CloseHandle(snap);
                    return hCS2 != nullptr;
                }
            }
        } while (Process32Next(snap, &pe));
    }
    CloseHandle(snap);
    return false;
}

Vector3 GetBone(uintptr_t pawn, int id) {
    uintptr_t node = Read<uintptr_t>(pawn + offsets::m_pGameSceneNode);
    if (!node) return {};
    uintptr_t state = node + offsets::m_modelState;
    uintptr_t array = Read<uintptr_t>(state + offsets::m_boneArray);
    if (!array) return {};
    Matrix4 m = Read<Matrix4>(array + id * 0x30);  // 0x30 = correct size
    return {m.m[12], m.m[13], m.m[14]};
}

void MemoryLoop() {
    while (enabled) {
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
        if (!client) continue;

        viewMatrix = Read<Matrix4>(client + offsets::dwViewMatrix);
        uintptr_t list = client + offsets::dwEntityList;
        uintptr_t localCtrl = client + offsets::dwLocalPlayerController;

        // Local player
        uintptr_t localPawn = Read<uintptr_t>(localCtrl + offsets::m_hPawn) & 0x3FFF;
        localPawn = Read<uintptr_t>(list + localPawn * 0x10);
        if (localPawn) {
            localTeam = Read<int>(localPawn + offsets::m_iTeamNum);
            localHead = GetBone(localPawn, 6);
        }

        std::vector<PlayerInfo> listOut;
        for (int i = 1; i < 64; ++i) {
            uintptr_t entry = Read<uintptr_t>(list + i * 0x78);
            if (!entry) continue;
            uintptr_t ctrl = Read<uintptr_t>(entry + 0x8 * (i & 0x7F));
            if (!ctrl) continue;
            uintptr_t pawn = Read<uintptr_t>(ctrl + offsets::m_hPlayerPawn) & 0x3FFF;
            pawn = Read<uintptr_t>(list + pawn * 0x10);
            if (!pawn || pawn == localPawn) continue;

            int health = Read<int>(pawn + offsets::m_iHealth);
            if (health < 1 || health > 100) continue;
            int team = Read<int>(pawn + offsets::m_iTeamNum);
            if (team == localTeam) continue;

            Vector3 pos = Read<Vector3>(pawn + offsets::m_vecAbsOrigin);
            Vector3 head = GetBone(pawn, 6);

            listOut.push_back({head, pos, (float)health, team});
        }
        { std::lock_guard<std::mutex> lock(mtx); players = listOut; }
    }
}

void HotkeyLoop() {
    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        if (GetAsyncKeyState(VK_INSERT) & 1) enabled = !enabled;
        if (GetAsyncKeyState(VK_HOME) & 1) espOn = !espOn;
        if (GetAsyncKeyState(VK_END) & 1) aimOn = !aimOn;

        system("cls");
        std::cout << "CS2 Offline Dev Tool\n";
        std::cout << "Status: " << (enabled ? "ON" : "OFF")
                  << " | ESP: " << (espOn ? "ON" : "OFF")
                  << " | Aimbot: " << (aimOn ? "ON" : "OFF") << "\n";
        std::cout << "Insert=Toggle | Home=ESP | End=Aimbot (Hold LMB)\n";
    }
}

void AimbotLoop() {
    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
        if (!aimOn || !(GetAsyncKeyState(VK_LBUTTON) & 0x8000)) continue;

        Vector3 target; float best = 99999;
        { std::lock_guard<std::mutex> lock(mtx);
            for (const auto& p : players) {
                float dist = sqrt(pow(p.head.x - localHead.x, 2) + pow(p.head.z - localHead.z, 2));
                if (dist < best && dist < fov) { best = dist; target = p.head; }
            }
        }
        if (best > fov) continue;

        Vector2 scr; if (!W2S(target, scr)) continue;
        POINT mouse; GetCursorPos(&mouse);
        int dx = (scr.x - mouse.x) * smooth;
        int dy = (scr.y - mouse.y) * smooth;

        INPUT in = {}; in.type = INPUT_MOUSE; in.mi.dwFlags = MOUSEEVENTF_MOVE;
        in.mi.dx = dx; in.mi.dy = dy; SendInput(1, &in, sizeof(in));
    }
}

LRESULT CALLBACK WndProc(HWND h, UINT m, WPARAM w, LPARAM l) { return DefWindowProc(h, m, w, l); }
void CreateOverlay() {
    WNDCLASSEX wc = {sizeof(wc), CS_CLASSDC, WndProc, 0, 0, GetModuleHandle(NULL), 0, 0, 0, 0, "OV", 0};
    RegisterClassEx(&wc);
    hOverlay = CreateWindowEx(WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_LAYERED, "OV", "Overlay", WS_POPUP, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, NULL, NULL, GetModuleHandle(NULL), NULL);
    SetLayeredWindowAttributes(hOverlay, 0, 255, LWA_ALPHA);
    ShowWindow(hOverlay, SW_SHOW);
}

void RenderLoop() {
    HDC hdc = GetDC(hOverlay);
    int w = rect.right - rect.left, h = rect.bottom - rect.top;
    HDC memDC = CreateCompatibleDC(hdc);
    HBITMAP bmp = CreateCompatibleBitmap(hdc, w, h);
    SelectObject(memDC, bmp);
    Graphics g(memDC);
    Pen red(Color(255,255,0,0), 2);

    while (enabled) {
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
        GetWindowRect(hCS2, &rect);
        SetWindowPos(hOverlay, HWND_TOPMOST, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, 0);

        g.Clear(Color(0,0,0,0));
        if (espOn) {
            std::lock_guard<std::mutex> lock(mtx);
            for (const auto& p : players) {
                Vector2 h, f;
                if (!W2S(p.head, h) || !W2S(p.feet, f)) continue;
                float bh = f.y - h.y, bw = bh / 2;
                g.DrawRectangle(&red, h.x - bw/2, h.y, bw, bh);
            }
        }
        POINT pt{}; SIZE sz{w, h};
        BLENDFUNCTION bf{AC_SRC_OVER, 0, 255, AC_SRC_ALPHA};
        UpdateLayeredWindow(hOverlay, hdc, NULL, &sz, memDC, &pt, 0, &bf, ULW_ALPHA);
    }
    DeleteObject(bmp); DeleteDC(memDC); ReleaseDC(hOverlay, hdc);
}

int main() {
    std::cout << "CS2 Offline Dev Tool - Press Enter to start\n";
    std::cin.get();

    while (!FindCS2()) {
        std::cout << "Waiting for CS2 (start offline mode)...\n";
        std::this_thread::sleep_for(std::chrono::seconds(5));
    }

    GetWindowRect(hCS2, &rect);
    CreateOverlay();

    std::thread(MemoryLoop).detach();
    std::thread(HotkeyLoop).detach();
    std::thread(AimbotLoop).detach();
    std::thread(RenderLoop).join();

    CloseHandle(hProcess);
    return 0;
}
