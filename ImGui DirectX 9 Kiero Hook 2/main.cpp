#include "includes.h"

#ifdef _WIN64
#define GWL_WNDPROC GWLP_WNDPROC
#endif

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

EndScene oEndScene = NULL;
WNDPROC oWndProc;
static HWND window = NULL;

void InitImGui(LPDIRECT3DDEVICE9 pDevice)
{
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags = ImGuiConfigFlags_NoMouseCursorChange;
	ImGui_ImplWin32_Init(window);
	ImGui_ImplDX9_Init(pDevice);
}

#include "scgo.hpp"
using namespace hazedumper::signatures;
using namespace hazedumper::netvars;

bool int = false;

bool scgo = true;

int sreenX = GetSystemMetrics(SM_CXSCREEN);
int sreenY = GetSystemMetrics(SM_CYSCREEN);

bool esp = false;
bool bhop = false;
bool rcs = false;
int TriggerCustomDelay;
float rcs_amount = 0;
bool triggerbot = false;
bool triggerRandomness = false;
bool triggerCustomDelay = false;
bool tbDelay = false;

struct Vec3
{
	float x, y, z;
	Vec3 operator+(Vec3 d)
	{
		return{ x + d.x, y + d.y, z + d.z };
	}
	Vec3 operator-(Vec3 d)
	{
		return{ x - d.x, y - d.y, z - d.z };
	}
	Vec3 operator*(float d)
	{
		return{ x * d, y * d, z * d };
	}
	void normalize()
	{
		while (y < -180) { y += 360; }
		{

		}
		while (y > 180) { y -= 360; }
		{

		}
		if (x > 89) { x = 89; }
		if (x < -89) { x = -89; }
	}
};

DWORD clientMod;
DWORD engineMod;
DWORD localplayer;
int* iShotsFired;
Vec3* viewAngles;
Vec3* aimRecoilPunch;
Vec3 oPunch{ 0, 0 ,0 };

void HackInt()
{
	clientMod = (uintptr_t)GetModuleHandle("client.dll");
	engineMod = (uintptr_t)GetModuleHandle("engine.dll");

	localplayer = *(uintptr_t*)(clientMod + dwLocalPlayer);

	iShotsFired = (int*)(localplayer + m_iShotsFired);

	viewAngles = (Vec3*)(*(uintptr_t*)(engineMod + dwClientState) + dwClientState_ViewAngles);

	aimRecoilPunch = (Vec3*)(localplayer + m_aimPunchAngle);

}
bool init = false;
long __stdcall hkEndScene(LPDIRECT3DDEVICE9 pDevice)
{
	if (!init)
	{
		InitImGui(pDevice);
		init = true;
	}
	if (GetAsyncKeyState(VK_INSERT) & 1)
	{
		scgo = !scgo;
	}
	ImGui_ImplDX9_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	if (scgo)
	{
		ImGui::Begin("scgo");
		ImGui::Text("esp");
		ImGui::Checkbox("Esp", &esp);
		ImGui::BeginTabBar("misc");
		ImGui::Text("misc");
		ImGui::Checkbox("Bhop", &bhop);
		ImGui::Checkbox("no recoil", &rcs);
		if (rcs)
		{
			ImGui::SliderFloat("No-Recoil amount", &rcs_amount, 0, 1);
		}

		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Text("triggerbot");
		ImGui::Checkbox("Enable", &triggerbot);
		if (triggerbot)
		{
			ImGui::Checkbox("add Custom Delay", &tbDelay);
			if (tbDelay)
			{
				ImGui::SliderInt("Cutsom Delay", &TriggerCustomDelay, 0, 250);
			}
			else
			{
				ImGui::Checkbox("Random Delay", &triggerRandomness);
			}
		}
		ImGui::End();

		ImGui::EndFrame();
		ImGui::Render();
		ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());

		return oEndScene(pDevice);
	}
}

LRESULT __stdcall WndProc(const HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {

	if (true && ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))
		return true;

	return CallWindowProc(oWndProc, hWnd, uMsg, wParam, lParam);
}

BOOL CALLBACK EnumWindowsCallback(HWND handle, LPARAM lParam)
{
	DWORD wndProcId;
	GetWindowThreadProcessId(handle, &wndProcId);

	if (GetCurrentProcessId() != wndProcId)
		return TRUE; // skip to next window

	window = handle;
	return FALSE; // window found abort search
}

HWND GetProcessWindow()
{
	window = NULL;
	EnumWindows(EnumWindowsCallback, NULL);
	return window;
}

DWORD WINAPI MainThread(LPVOID lpReserved)
{
	bool attached = false;
	do
	{
		if (kiero::init(kiero::RenderType::D3D9) == kiero::Status::Success)
		{
			kiero::bind(42, (void**)& oEndScene, hkEndScene);
			do
				window = GetProcessWindow();
			while (window == NULL);
			oWndProc = (WNDPROC)SetWindowLongPtr(window, GWL_WNDPROC, (LONG_PTR)WndProc);
			attached = true;
		}
	} while (!attached);
	return TRUE;
}



DWORD WINAPI RCSThread(LPVOID lp)
{
	while (true)
	{
		if (rcs && !GetAsyncKeyState(VK_RBUTTON))
		{
			Vec3 punchAngle = *aimRecoilPunch * (rcs_amount * 2);
			if (*iShotsFired > 1 && GetAsyncKeyState(VK_LBUTTON))
			{
				Vec3 newAngle = *viewAngles + oPunch - punchAngle;
				newAngle.normalize();

				*viewAngles = newAngle;
			}
			oPunch = punchAngle;
		}
		Sleep(10);
	}
}

DWORD WINAPI TriggerThread(LPVOID lpReserved)
{
	DWORD gameMod = (DWORD)GetModuleHandle("client.dll");
	while (true)
	{
		if (triggerbot)
		{
			DWORD localPlayer = *(DWORD*)(gameMod + dwLocalPlayer);
			int crosshair = *(int*)(localPlayer + m_iCrosshairId);
			int localTeam = *(int*)(localPlayer + m_iTeamNum);
			if (crosshair != 0 && crosshair <= 64)
			{
				uintptr_t entity = *(DWORD*)(gameMod + dwEntityList + ((crosshair - 1) * 0x10));
				int eTeam = *(int*)(entity + m_iTeamNum);
				int eHealth = *(INT*)(entity + m_iHealth);

				if (eTeam != localTeam && eHealth > 0)
				{
					*(int*)(gameMod + dwForceAttack) = 5;
					Sleep(10);
					*(int*)(gameMod + dwForceAttack) = 4;
				}
			}

		}
		Sleep(10);
	}

}

DWORD WINAPI BhopThread(LPVOID lp)
{
	DWORD gameModule = (DWORD)GetModuleHandle("client.dll");
	DWORD localplayer = *(DWORD*)(gameModule + dwLocalPlayer);
	while (localplayer == NULL)
	{
		DWORD localplayer = *(DWORD*)(gameModule + dwLocalPlayer);
	}
	while (true)
	{
		if (bhop)
		{
			DWORD flags = *(BYTE*)(localplayer + m_fFlags);
			if (GetAsyncKeyState(VK_SPACE) && flags & (1 << 0))
			{
				*(DWORD*)(gameModule + dwForceJump) = 6;
			}
		}
		Sleep(10);
	}
}

BOOL WINAPI DllMain(HMODULE hMod, DWORD dwReason, LPVOID lpReserved)
{
	switch (dwReason)
	{
	case DLL_PROCESS_ATTACH:
		DisableThreadLibraryCalls(hMod);
		CreateThread(nullptr, 0, MainThread, hMod, 0, nullptr);
		break;
	case DLL_PROCESS_DETACH:
		kiero::shutdown();
		break;
	}
	return TRUE;
}
