#ifdef TARGET_WEB

#include "os/gfx.h"

#include <emscripten.h>
#include <GLFW/glfw3.h>

static void web_glfw_error_callback(int error, const char* desc) {
	NoOp(error);
	NoOp(desc);
}

OS_Handle os_window_init(U64 width, U64 height, String8 title) {
	glfwSetErrorCallback(web_glfw_error_callback);
	if (!glfwInit()) {
		os_abort_with_message(1, str8_lit("Failed to initialize GLFW"));
	}

	glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
	glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE);

	GLFWwindow* window = glfwCreateWindow(width, height, (char*)title.str, NULL, NULL);
	if (window == NULL) {
		os_abort_with_message(1, str8_lit("Failed to create GLFW window"));
	}

	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);

	OS_Handle handle = { .value = (U64)window };
	return handle;
}

void os_window_show(OS_Handle window) {
	glfwShowWindow((GLFWwindow*)window.value);
}

void os_window_swap(OS_Handle window) {
	glfwSwapBuffers((GLFWwindow*)window.value);
}

void os_window_set_title(OS_Handle window, String8 title) {
	glfwSetWindowTitle((GLFWwindow*)window.value, (char*)title.str);
}

void os_window_poll_events(OS_Handle window) {
	NoOp(window);
	glfwPollEvents();
}

void* os_window_get_native_handle(OS_Handle window) {
	return (void*)window.value;
}

B32 os_window_should_close(OS_Handle window) {
	return glfwWindowShouldClose((GLFWwindow*)window.value);
}

void os_window_release(OS_Handle window) {
	glfwDestroyWindow((GLFWwindow*)window.value);
	glfwTerminate();
}

void os_show_message_box(OS_MessageBoxIcon icon, String8 title, String8 message) {
	NoOp(icon);
	NoOp(title);
	EM_ASM({ alert(UTF8ToString($0)); }, message.str);
}

void os_open_in_browser(String8 url) {
	EM_ASM({ window.open(UTF8ToString($0), "_blank", "noopener"); }, url.str);
}

#endif // TARGET_WEB
