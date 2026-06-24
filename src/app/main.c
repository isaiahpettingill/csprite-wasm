#include "os/gfx.h"
#include "app/gui.h"
#include "app/imgui.h"
#include "app/editor.h"

#ifdef TARGET_WEB
	#include <emscripten.h>
#endif

#include <stdint.h>
#include <string.h>

typedef struct {
	OS_Handle window;
	Editor ed;
	bool do_open_new_file_modal;
} App;

static App g_app;
static B32 g_app_ready;

#ifdef TARGET_WEB
	#define CSPRITE_EXPORT EMSCRIPTEN_KEEPALIVE

static U32 app_pixels_size(Editor* ed) {
	U64 size = (U64)ed->image.dim.w * (U64)ed->image.dim.h * sizeof(RGBAU8);
	return size <= UINT32_MAX ? (U32)size : 0;
}

CSPRITE_EXPORT U32 csprite_editor_width(void) {
	return g_app_ready ? g_app.ed.image.dim.w : 0;
}

CSPRITE_EXPORT U32 csprite_editor_height(void) {
	return g_app_ready ? g_app.ed.image.dim.h : 0;
}

CSPRITE_EXPORT U32 csprite_editor_pixels_size(void) {
	return g_app_ready ? app_pixels_size(&g_app.ed) : 0;
}

CSPRITE_EXPORT U8* csprite_editor_pixels(void) {
	return g_app_ready ? (U8*)g_app.ed.image.data : NULL;
}

CSPRITE_EXPORT B32 csprite_editor_load_rgba(U32 width, U32 height, const U8* pixels, U32 size) {
	if (!g_app_ready || width == 0 || height == 0 || pixels == NULL) {
		return 0;
	}

	U64 expected_size64 = (U64)width * (U64)height * sizeof(RGBAU8);
	if (expected_size64 > UINT32_MAX || size < (U32)expected_size64) {
		return 0;
	}

	ed_release(&g_app.ed);
	g_app.ed = ed_init(width, height);
	memcpy(g_app.ed.image.data, pixels, (size_t)expected_size64);
	r_tex_update(g_app.ed.image_tex, 0, 0, width, height, width, (U8*)g_app.ed.image.data);

	ImGuiIO* io = igGetIO_Nil();
	g_app.ed.view_scale = 5;
	Editor_UpdateView(&g_app.ed);
	Editor_CenterView(&g_app.ed, (Rect){ io->DisplaySize.x, io->DisplaySize.y });
	return 1;
}
#endif

static void app_frame(void* user_data) {
	App* app = user_data;
	Editor* ed = &app->ed;

	if (os_window_should_close(app->window)) {
#ifdef TARGET_WEB
		emscripten_cancel_main_loop();
#endif
		return;
	}

	gui_begin_frame(app->window);

	ImGuiIO* io = igGetIO_Nil();
	ImVec2 mBarPos;
	ImVec2 mBarSize;
	igBeginMainMenuBar();
		if (igBeginMenu("File", true)) {
			if (igMenuItem_Bool("New", NULL, false, true)) {
				app->do_open_new_file_modal = true;
			}
			// if (igMenuItem_Bool("Open", NULL, false, true)) {
			// 	_app_open_file(ed);
			// }
			// if (igMenuItem_Bool("Save", NULL, false, true)) {
			// 	_app_save_file(ed);
			// }
			igEndMenu();
		}
		if (igBeginMenu("Help", true)) {
			if (igMenuItem_Bool("About", NULL, false, true)) {
				os_open_in_browser(str8_lit("https://csprite.github.io"));
			}
			igEndMenu();
		}

		ImVec2 winSize = {0}, textSize = {0};
		igGetWindowSize(&winSize);
		// igCalcTextSize(&textSize, ed->file.name, NULL, false, -1);
		igSetCursorPosX((winSize.x - textSize.x) * 0.5);
		igTextUnformatted("untitled", NULL);

		igGetWindowPos(&mBarPos);
		igGetWindowSize(&mBarSize);
	igEndMainMenuBar();

	static ImVec2 SidebarPos, SidebarSize;
	SidebarSize.y = io->DisplaySize.y - (mBarPos.y + mBarSize.y); // Used as Constraint & To Reduce Duplicate Calculations
	igSetNextWindowPos((ImVec2){ 0, mBarPos.y + mBarSize.y }, ImGuiCond_Always, (ImVec2){ 0, 0 });
	igSetNextWindowSize((ImVec2){ 200, 0 }, ImGuiCond_Once);
	igSetNextWindowSizeConstraints((ImVec2){ 100, SidebarSize.y }, (ImVec2){ io->DisplaySize.x / 3, SidebarSize.y }, NULL, NULL);
	igPushStyleVar_Float(ImGuiStyleVar_WindowBorderSize, 0);
	if (igBegin("##Sidebar", NULL, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoTitleBar)) {
		float color_float[4] = {
			ed->tool_color.r / 255.0f,
			ed->tool_color.g / 255.0f,
			ed->tool_color.b / 255.0f,
			ed->tool_color.a / 255.0f
		};
		ImVec2 availReg;
		igGetContentRegionAvail(&availReg);
		igSetNextItemWidth(availReg.x);
		if (igColorPicker4("##ColorPicker", (float*)&color_float, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoSmallPreview, NULL)) {
			ed->tool_color.r = color_float[0] * 255;
			ed->tool_color.g = color_float[1] * 255;
			ed->tool_color.b = color_float[2] * 255;
			ed->tool_color.a = color_float[3] * 255;
		}
		if (igIsItemHovered(0)) {
			igSetMouseCursor(ImGuiMouseCursor_Hand);
		}

		igGetWindowPos(&SidebarPos);
		igGetWindowSize(&SidebarSize);
		igEnd();
	}
	igPopStyleVar(1);

	ImVec2 statusBarPos, statusBarSize;
	igSetNextWindowPos((ImVec2){ SidebarPos.x + SidebarSize.x, mBarPos.y + mBarSize.y }, ImGuiCond_Always, (ImVec2){ 0, 0 });
	igSetNextWindowSize((ImVec2){ io->DisplaySize.x, 0 }, ImGuiCond_Always);
	if (igBegin("##StatusBar", NULL, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_AlwaysAutoResize)) {
		ImVec2 tWidth;
		igCalcTextSize(&tWidth, "BRUSH_XXX", NULL, false, -1);
		igSetNextItemWidth(tWidth.x);
		if (igBeginCombo("##ToolSelector", Editor_ToolGetHumanReadable(ed->tool_curr), 0)) {
			for (S32 i = 0; i < TOOL_NONE; i++) {
				if (igSelectable_Bool(Editor_ToolGetHumanReadable(i), (EdTool)i == ed->tool_curr, 0, (ImVec2){0,0})) {
					ed->tool_curr = i;
				}
			}
			igEndCombo();
		}

		if (ed->tool_curr == TOOL_BRUSH || ed->tool_curr == TOOL_ERASER) {
			igSameLine(0, -1);
			igCalcTextSize(&tWidth, "WWWWWWWWW", NULL, false, -1);
			igSetNextItemWidth(tWidth.x);
			if (igInputInt("##BrushSize", (S32*)&ed->tool_size, 1, 1, 0)) {
				if (ed->tool_size < 1) {
					ed->tool_size = 1;
				}
			}

			igSameLine(0, -1);
			static bool checked = 0;
			if (igCheckbox("Filled", &checked)) {
				ed->tool_fill = checked;
			}
		}

		float step = ed->view_scale > 1 ? 0.15 : 0.05;
		igCalcTextSize(&tWidth, "ZOOM_XXXXXXX", NULL, false, -1);
		igSetNextItemWidth(tWidth.x);
		igSameLine(0, -1);
		if (igInputFloat("##ZoomControl", &ed->view_scale, step, step, "x%.2f", 0)) {
			ed->view_scale = ed->view_scale < 0.05 ? 0.05 : ed->view_scale;
			Editor_UpdateView(ed);
		}

		igSameLine(0, -1);
		igText("x: %d, y: %d", (S32)((io->MousePos.x - ed->view_pos.x)/ed->view_scale), (S32)((io->MousePos.y - ed->view_pos.y)/ed->view_scale));

		igGetWindowPos(&statusBarPos);
		igGetWindowSize(&statusBarSize);
		igEnd();
	}

	igSetNextWindowPos((ImVec2){ SidebarPos.x + SidebarSize.x, statusBarPos.y + statusBarSize.y }, ImGuiCond_Always, (ImVec2){ 0, 0 });
	igSetNextWindowSize((ImVec2){ io->DisplaySize.x, io->DisplaySize.y - (statusBarPos.y + statusBarSize.y) + 1 }, ImGuiCond_Always);
	if (igBegin("Main", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBringToFrontOnFocus)) {
		ImDrawList_AddRect(
		    igGetWindowDrawList(),
		    (ImVec2){ ed->view_pos.x, ed->view_pos.y }, (ImVec2){ ed->view_pos.x + ed->view_size.w, ed->view_pos.y + ed->view_size.h },
		    igGetColorU32_Col(ImGuiCol_Border, 1), 0, 0, 1
		);
		ImDrawList_AddImage(
		    igGetWindowDrawList(), r_tex_to_ImTextureID(ed->checker_tex),
		    (ImVec2){ ed->view_pos.x, ed->view_pos.y }, (ImVec2){ ed->view_pos.x + ed->view_size.w, ed->view_pos.y + ed->view_size.h },
		    (ImVec2){ 0, 0 }, (ImVec2){ 1, 1 }, 0xFFFFFFFF
		);
		ImDrawList_AddImage(
		    igGetWindowDrawList(), r_tex_to_ImTextureID(ed->image_tex),
		    (ImVec2){ ed->view_pos.x, ed->view_pos.y }, (ImVec2){ ed->view_pos.x + ed->view_size.w, ed->view_pos.y + ed->view_size.h },
		    (ImVec2){ 0, 0 }, (ImVec2){ 1, 1 }, 0xFFFFFFFF
		);

		if (igIsWindowHovered(0)) {
			igSetMouseCursor(ImGuiMouseCursor_None);
			ed_process_input(ed);
		}

		igEnd();
	}

	if (app->do_open_new_file_modal) {
		app->do_open_new_file_modal = false;
		igOpenPopup_Str("##NewFileModal", 0);
	}
	if (igBeginPopupModal("##NewFileModal", NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse)) {
		static S32 width = 20, height = 20;
		if (igInputInt("Width", &width, 1, 5, 0)) width = width < 2 ? 2 : width;
		if (igInputInt("Height", &height, 1, 5, 0)) height = height < 2 ? 2 : height;

		if (igButton("Create", (ImVec2){0,0})) {
			ed_release(ed);
			*ed = ed_init(width, height);
			Editor_CenterView(ed, (Rect){ io->DisplaySize.x, io->DisplaySize.y });
			Editor_UpdateView(ed);
			igCloseCurrentPopup();
			width = 20;
			height = 20;
		}
		igSameLine(0, -1);
		if (igButton("Cancel", (ImVec2){0,0})) {
			igCloseCurrentPopup();
			width = 20;
			height = 20;
		}

		igEndPopup();
	}

	gui_end_frame(app->window);
}

static App app_init(void) {
	App app = {0};
	app.window = os_window_init(320, 240, str8_lit("csprite"));
	app.ed = ed_init(120, 90);

	gui_init(app.window);
	os_window_show(app.window);

	gui_begin_frame(app.window);
	gui_end_frame(app.window);

	ImGuiIO* io = igGetIO_Nil();
	app.ed.view_scale = 5;
	Editor_UpdateView(&app.ed);
	Editor_CenterView(&app.ed, (Rect){ io->DisplaySize.x, io->DisplaySize.y });

	return app;
}

#ifndef TARGET_WEB
static void app_release(App* app) {
	ed_release(&app->ed);
	gui_release(app->window);
	os_window_release(app->window);
}
#endif

int main(void) {
	g_app = app_init();
	g_app_ready = true;

#ifdef TARGET_WEB
	emscripten_set_main_loop_arg(app_frame, &g_app, 0, true);
#else
	while (!os_window_should_close(g_app.window)) {
		app_frame(&g_app);
	}
	app_release(&g_app);
#endif

	return 0;
}
