/*
 nanogui/nanogui.cpp -- Basic initialization and utility routines
 
 NanoGUI was developed by Wenzel Jakob <wenzel.jakob@epfl.ch>.
 The widget drawing code is based on the NanoVG demo application
 by Mikko Mononen.
 
 All rights reserved. Use of this source code is governed by a
 BSD-style license that can be found in the LICENSE.txt file.
 */

#include <nanogui/screen.h>

#if defined(_WIN32)
#  ifndef NOMINMAX
#  define NOMINMAX 1
#  endif
#  include <windows.h>
#endif

#include <nanogui/opengl.h>
#include <nanogui/metal.h>
#include <unordered_map>  // Replaced <map> with <unordered_map> for better performance
#include <queue>          // Added for asynchronous function queue
#include <thread>
#include <chrono>
#include <mutex>
#include <iostream>
#include <atomic>         // Added for atomic variables
#include <functional>     // Added for std::function

#if !defined(_WIN32)
#  include <locale.h>
#  include <signal.h>
#  include <dirent.h>
#endif

#if defined(EMSCRIPTEN)
#  include <emscripten/emscripten.h>
#endif

NAMESPACE_BEGIN(nanogui)

// Forward declarations
extern Screen* screen1;
extern GLFWwindow* window1;

// Mutex to protect access to screen1
std::mutex screen1_mutex;

// Redraw thread management variables
static std::atomic<bool> redraw_thread_running(false);
static std::thread redraw_thread;

// Mutex and queue for asynchronous functions
std::mutex m_async_mutex;
// Changed from std::vector to std::queue for better FIFO handling
std::queue<std::function<void()>> m_async_functions;

#if defined(__APPLE__)
extern void disable_saved_application_state_osx();
#endif

/* Calculate pixel ratio for hi-dpi devices. */
static float get_pixel_ratio(GLFWwindow *window) {
#if defined(EMSCRIPTEN)
	return emscripten_get_device_pixel_ratio();
#else
	float xscale, yscale;
	glfwGetWindowContentScale(window, &xscale, &yscale);
	return xscale;
#endif
}

// Function to enqueue asynchronous functions
void async(const std::function<void()> &func) {
	std::lock_guard<std::mutex> guard(m_async_mutex);
	m_async_functions.push(func);
}

// Function to start the redraw thread
void start_redraw_thread() {
	redraw_thread_running = true;
	redraw_thread = std::thread([](){
//		while (redraw_thread_running) {
//			// Enqueue redraw for the visible screen
//			async([](){
//				std::lock_guard<std::mutex> lock(screen1_mutex);
//				if (screen1 && screen1->visible()) {
//					screen1->redraw();
//				}
//			});
//			// Sleep for approximately 16ms to achieve ~60fps
//			std::this_thread::sleep_for(std::chrono::milliseconds(16));
//		}
	});
}

// Function to stop the redraw thread
void stop_redraw_thread() {
	redraw_thread_running = false;
	if (redraw_thread.joinable()) {
		redraw_thread.join();
	}
}

void init() {
#if !defined(_WIN32)
	/* Avoid locale-related number parsing issues */
	setlocale(LC_NUMERIC, "C");
#endif
	
#if defined(__APPLE__)
	disable_saved_application_state_osx();
	glfwInitHint(GLFW_COCOA_CHDIR_RESOURCES, GLFW_FALSE);
#endif
	
	glfwSetErrorCallback(
						 [](int error, const char *descr) {
							 if (error == GLFW_NOT_INITIALIZED)
								 return; /* Ignore */
							 std::cerr << "GLFW error " << error << ": " << descr << std::endl;
						 }
						 );
	
	if (!glfwInit())
		throw std::runtime_error("Could not initialize GLFW!");
	
#if defined(NANOGUI_USE_METAL)
	metal_init();
#endif
	
	glfwSetTime(0);
}

// Screen visibility flag
static bool mainloop_active = false;

#if defined(EMSCRIPTEN)
static double emscripten_last = 0;
static float emscripten_refresh = 0;
#endif

// Function to register all GLFW event callbacks with mutex protection
void register_event_callbacks() {
	// Cursor Position Callback
	glfwSetCursorPosCallback(window1,
							 [](GLFWwindow *w, double x, double y) {
		std::lock_guard<std::mutex> lock(screen1_mutex);
		if (screen1 && screen1->get_process_events()) {
			screen1->cursor_pos_callback_event(x, y);
		}
	}
							 );
	
	// Mouse Button Callback
	glfwSetMouseButtonCallback(window1,
							   [](GLFWwindow *w, int button, int action, int modifiers) {
		std::lock_guard<std::mutex> lock(screen1_mutex);
		if (screen1 && screen1->get_process_events()) {
			screen1->mouse_button_callback_event(button, action, modifiers);
		}
	}
							   );
	
	// Key Callback
	glfwSetKeyCallback(window1,
					   [](GLFWwindow *w, int key, int scancode, int action, int mods) {
		std::lock_guard<std::mutex> lock(screen1_mutex);
		if (screen1 && screen1->get_process_events()) {
			screen1->key_callback_event(key, scancode, action, mods);
		}
	}
					   );
	
	// Character Callback
	glfwSetCharCallback(window1,
						[](GLFWwindow *w, unsigned int codepoint) {
		std::lock_guard<std::mutex> lock(screen1_mutex);
		if (screen1 && screen1->get_process_events()) {
			screen1->char_callback_event(codepoint);
		}
	}
						);
	
	// Drop Callback
	glfwSetDropCallback(window1,
						[](GLFWwindow *w, int count, const char **filenames) {
		std::lock_guard<std::mutex> lock(screen1_mutex);
		if (screen1 && screen1->get_process_events()) {
			screen1->drop_callback_event(count, filenames);
		}
	}
						);
	
	// Scroll Callback
	glfwSetScrollCallback(window1,
						  [](GLFWwindow *w, double x, double y) {
		std::lock_guard<std::mutex> lock(screen1_mutex);
		if (screen1 && screen1->get_process_events()) {
			screen1->scroll_callback_event(x, y);
		}
	}
						  );
	
	// Framebuffer Size Callback
	glfwSetFramebufferSizeCallback(window1,
								   [](GLFWwindow* w, int width, int height) {
		std::lock_guard<std::mutex> lock(screen1_mutex);
		if (screen1 && screen1->get_process_events()) {
			screen1->resize_callback_event(width, height);
		}
	}
								   );
		
	// Window Content Scale Callback
	glfwSetWindowContentScaleCallback(window1,
									  [](GLFWwindow* w, float xscale, float yscale) {
		std::lock_guard<std::mutex> lock(screen1_mutex);
		if (screen1) {
			screen1->set_pixel_ratio(get_pixel_ratio(w));
			screen1->resize_callback_event(screen1->size().x(), screen1->size().y());
		}
	}
									  );
}

void mainloop() {
	if (mainloop_active)
		throw std::runtime_error("Main loop is already running!");
	
	// Ensure window1 and screen1 are initialized
	{
		std::lock_guard<std::mutex> lock(screen1_mutex);
		if (!window1 || !screen1) {
			throw std::runtime_error("window1 and screen1 must be initialized before entering the main loop.");
		}
	}
	
	/* Register event callbacks with mutex protection */
	register_event_callbacks();
	
	// Start the redraw thread
	start_redraw_thread();
	
	auto mainloop_iteration = [&]() {
		int num_screens = 0;
		
#if defined(EMSCRIPTEN)
		double emscripten_now = glfwGetTime();
		bool emscripten_redraw = false;
		if (float((emscripten_now - emscripten_last) * 1000) > emscripten_refresh) {
			emscripten_redraw = true;
			emscripten_last = emscripten_now;
		}
#endif
		
		/* Run async functions */
		{
			std::queue<std::function<void()>> local_queue;
			{
				std::lock_guard<std::mutex> guard(m_async_mutex);
				std::swap(local_queue, m_async_functions);
			}
			while (!local_queue.empty()) {
				auto &f = local_queue.front();
				f();
				local_queue.pop();
			}
		}
		
		{
			std::lock_guard<std::mutex> lock(screen1_mutex);
			// Directly access the single screen
			if (screen1 && screen1->visible()) {
				if (glfwWindowShouldClose(window1)) {
					nanogui::async([](){
						screen1->set_visible(false);
					});
				} else {
#if defined(EMSCRIPTEN)
					if (emscripten_redraw || screen1->tooltip_fade_in_progress())
						screen1->redraw();
#else
					screen1->redraw();
#endif
					screen1->draw_all();
					num_screens++;
				}
			}
		}
		
		if (num_screens == 0) {
			/* Give up if there was nothing to draw */
			mainloop_active = false;
			return;
		}
		
#if !defined(EMSCRIPTEN)
		// Poll for and process events
		glfwPollEvents();
#endif
	};
	
#if defined(EMSCRIPTEN)
	emscripten_refresh = refresh;
	/* The following will throw an exception and enter the main
	 loop within Emscripten. This means that none of the code below
	 (or in the caller, for that matter) will be executed */
	emscripten_set_main_loop(mainloop_iteration, 0, 1);
#endif
	
	mainloop_active = true;
	
	try {
		while (mainloop_active)
			mainloop_iteration();
		
		/* Process events once more */
		glfwPollEvents();
	} catch (const std::exception &e) {
		std::cerr << "Caught exception in main loop: " << e.what() << std::endl;
		leave();
	}
	
	// Stop the redraw thread after exiting the main loop
	stop_redraw_thread();
}

// Function to terminate the main loop
void leave() {
	mainloop_active = false;
}

// Function to check if the main loop is active
bool active() {
	return mainloop_active;
}

// Function to test 10-bit EDR support
std::pair<bool, bool> test_10bit_edr_support() {
#if defined(NANOGUI_USE_METAL)
	return metal_10bit_edr_support();
#else
	return { false, false };
#endif
}

#if defined(__clang__)
#  define NANOGUI_FALLTHROUGH [[clang::fallthrough]];
#elif defined(__GNUG__)
#  define NANOGUI_FALLTHROUGH __attribute__ ((fallthrough));
#else
#  define NANOGUI_FALLTHROUGH
#endif

// Optimized UTF-8 conversion function with reduced branching
std::string utf8(uint32_t c) {
	std::string result;
	if (c < 0x80) {
		result += static_cast<char>(c);
	} else if (c < 0x800) {
		result += static_cast<char>(0xC0 | (c >> 6));
		result += static_cast<char>(0x80 | (c & 0x3F));
	} else if (c < 0x10000) {
		result += static_cast<char>(0xE0 | (c >> 12));
		result += static_cast<char>(0x80 | ((c >> 6) & 0x3F));
		result += static_cast<char>(0x80 | (c & 0x3F));
	} else if (c < 0x200000) {
		result += static_cast<char>(0xF0 | (c >> 18));
		result += static_cast<char>(0x80 | ((c >> 12) & 0x3F));
		result += static_cast<char>(0x80 | ((c >> 6) & 0x3F));
		result += static_cast<char>(0x80 | (c & 0x3F));
	} else if (c < 0x4000000) {
		result += static_cast<char>(0xF8 | (c >> 24));
		result += static_cast<char>(0x80 | ((c >> 18) & 0x3F));
		result += static_cast<char>(0x80 | ((c >> 12) & 0x3F));
		result += static_cast<char>(0x80 | ((c >> 6) & 0x3F));
		result += static_cast<char>(0x80 | (c & 0x3F));
	} else if (c <= 0x7FFFFFFF) {
		result += static_cast<char>(0xFC | (c >> 30));
		result += static_cast<char>(0x80 | ((c >> 24) & 0x3F));
		result += static_cast<char>(0x80 | ((c >> 18) & 0x3F));
		result += static_cast<char>(0x80 | ((c >> 12) & 0x3F));
		result += static_cast<char>(0x80 | ((c >> 6) & 0x3F));
		result += static_cast<char>(0x80 | (c & 0x3F));
	}
	return result;
}

// Function to get or create an image in the NanoVG context
int __nanogui_get_image(NVGcontext *ctx, const std::string &name, uint8_t *data, uint32_t size) {
	// Replaced std::map with std::unordered_map for faster image cache lookups
	static std::unordered_map<std::string, int> icon_cache;
	auto it = icon_cache.find(name);
	if (it != icon_cache.end())
		return it->second;
	int icon_id = nvgCreateImageMem(ctx, 0, data, size);
	if (icon_id == 0)
		throw std::runtime_error("Unable to load resource data.");
	icon_cache.emplace(name, icon_id);
	return icon_id;
}

// Function to load all PNG images from a directory into the NanoVG context
std::vector<std::pair<int, std::string>>
load_image_directory(NVGcontext *ctx, const std::string &path) {
	std::vector<std::pair<int, std::string> > result;
#if !defined(_WIN32)
	DIR *dp = opendir(path.c_str());
	if (!dp)
		throw std::runtime_error("Could not open image directory!");
	struct dirent *ep;
	while ((ep = readdir(dp))) {
		const char *fname = ep->d_name;
#else
		WIN32_FIND_DATA ffd;
		std::string search_path = path + "/*.*";
		HANDLE handle = FindFirstFileA(search_path.c_str(), &ffd);
		if (handle == INVALID_HANDLE_VALUE)
			throw std::runtime_error("Could not open image directory!");
		do {
			const char *fname = ffd.cFileName;
#endif
			if (strstr(fname, "png") == nullptr)
				continue;
			std::string full_name = path + "/" + std::string(fname);
			int img = nvgCreateImage(ctx, full_name.c_str(), 0);
			if (img == 0)
				throw std::runtime_error("Could not open image data!");
			result.emplace_back(
								std::make_pair(img, full_name.substr(0, full_name.length() - 4))
								);
#if !defined(_WIN32)
		}
		closedir(dp);
#else
	} while (FindNextFileA(handle, &ffd) != 0);
	FindClose(handle);
#endif
	return result;
}

#if !defined(__APPLE__)
std::vector<std::string> file_dialog(const std::vector<std::pair<std::string, std::string>> &filetypes, bool save, bool multiple) {
	static const int FILE_DIALOG_MAX_BUFFER = 16384;
	if (save && multiple) {
		throw std::invalid_argument("save and multiple must not both be true.");
	}
	
#if defined(EMSCRIPTEN)
	throw std::runtime_error("Opening files is not supported when NanoGUI is compiled via Emscripten");
#elif defined(_WIN32)
	OPENFILENAME ofn;
	ZeroMemory(&ofn, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	char tmp[FILE_DIALOG_MAX_BUFFER];
	ZeroMemory(tmp, FILE_DIALOG_MAX_BUFFER);
	ofn.lpstrFile = tmp;
	ofn.nMaxFile = FILE_DIALOG_MAX_BUFFER;
	ofn.nFilterIndex = 1;
	
	std::string filter;
	
	if (!save && filetypes.size() > 1) {
		filter.append("Supported file types (");
		for (size_t i = 0; i < filetypes.size(); ++i) {
			filter.append("*.");
			filter.append(filetypes[i].first);
			if (i + 1 < filetypes.size())
				filter.append(";");
		}
		filter.append(")");
		filter.push_back('\0');
		for (size_t i = 0; i < filetypes.size(); ++i) {
			filter.append("*.");
			filter.append(filetypes[i].first);
			if (i + 1 < filetypes.size())
				filter.append(";");
		}
		filter.push_back('\0');
	}
	for (auto pair : filetypes) {
		filter.append(pair.second);
		filter.append(" (*.");
		filter.append(pair.first);
		filter.append(")");
		filter.push_back('\0');
		filter.append("*.");
		filter.append(pair.first);
		filter.push_back('\0');
	}
	filter.push_back('\0');
	ofn.lpstrFilter = filter.data();
	
	if (save) {
		ofn.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;
		if (GetSaveFileNameA(&ofn) == FALSE)
			return {};
	} else {
		ofn.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
		if (multiple)
			ofn.Flags |= OFN_ALLOWMULTISELECT;
		if (GetOpenFileNameA(&ofn) == FALSE)
			return {};
	}
	
	size_t i = 0;
	std::vector<std::string> result;
	while (tmp[i] != '\0') {
		result.emplace_back(&tmp[i]);
		i += result.back().size() + 1;
	}
	
	if (result.size() > 1) {
		for (i = 1; i < result.size(); ++i) {
			result[i] = result[0] + "\\" + result[i];
		}
		result.erase(begin(result));
	}
	
	if (save && ofn.nFilterIndex > 0) {
		auto ext = filetypes[ofn.nFilterIndex - 1].first;
		if (ext != "*") {
			ext.insert(0, ".");
			
			auto &name = result.front();
			if (name.size() <= ext.size() ||
				name.compare(name.size() - ext.size(), ext.size(), ext) != 0) {
				name.append(ext);
			}
		}
	}
	
	return result;
#else
	char buffer[FILE_DIALOG_MAX_BUFFER];
	buffer[0] = '\0';
	
	std::string cmd = "zenity --file-selection ";
	// The safest separator for multiple selected paths is /, since / can never occur
	// in file names. Only where two paths are concatenated will there be two / following
	// each other.
	if (multiple)
		cmd += "--multiple --separator=\"/\" ";
	if (save)
		cmd += "--save ";
	cmd += "--file-filter=\"";
	for (auto pair : filetypes)
		cmd += "\"*." + pair.first + "\" ";
	cmd += "\"";
	FILE *output = popen(cmd.c_str(), "r");
	if (output == nullptr)
		throw std::runtime_error("popen() failed -- could not launch zenity!");
	while (fgets(buffer, FILE_DIALOG_MAX_BUFFER, output) != NULL)
		;
	pclose(output);
	std::string paths(buffer);
	paths.erase(std::remove(paths.begin(), paths.end(), '\n'), paths.end());
	
	std::vector<std::string> result;
	while (!paths.empty()) {
		size_t end = paths.find("//");
		if (end == std::string::npos) {
			result.emplace_back(paths);
			paths = "";
		} else {
			result.emplace_back(paths.substr(0, end));
			paths = paths.substr(end + 1);
		}
	}
	
	return result;
#endif
}
#endif

// Destructor for Object
Object::~Object() { }

// Function to gracefully shutdown NanoGUI
void shutdown() {
	leave(); // Ensure mainloop_active is set to false
	stop_redraw_thread(); // Stop the redraw thread
	
	glfwTerminate();
	
#if defined(NANOGUI_USE_METAL)
	metal_shutdown();
#endif
}

NAMESPACE_END(nanogui)

