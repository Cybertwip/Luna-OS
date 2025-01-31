#include <nanogui/nanogui.h>
#import <Cocoa/Cocoa.h>

#if defined(NANOGUI_USE_METAL)
#  import <Metal/Metal.h>
#  import <QuartzCore/CAMetalLayer.h>
#  import <QuartzCore/CATransaction.h>
#endif

#if !defined(MAC_OS_X_VERSION_10_15) || \
MAC_OS_X_VERSION_MIN_REQUIRED < MAC_OS_X_VERSION_10_15
@interface NSScreen (ForwardDeclare)
@property(readonly)
CGFloat maximumPotentialExtendedDynamicRangeColorComponentValue;
@end
#endif

NAMESPACE_BEGIN(nanogui)

// Asynchronous File Dialog Function
void file_dialog_async(
					   const std::vector<std::pair<std::string, std::string>> &filetypes,
					   bool save,
					   bool multiple,
					   std::function<void(const std::vector<std::string>&)> callback
					   ) {
	// Validate the input parameters
	if (save && multiple) {
		throw std::invalid_argument("file_dialog_async(): 'save' and 'multiple' must not both be true.");
	}
	
	// Ensure that the callback is valid
	if (!callback) {
		throw std::invalid_argument("file_dialog_async(): callback must be a valid function.");
	}
	
	// Prepare the allowed file types as NSArray<NSString *>
	NSArray<NSString *> *allowedTypes = nil;
	if (!filetypes.empty()) {
		NSMutableArray<NSString *> *mutableTypes = [NSMutableArray arrayWithCapacity:filetypes.size()];
		for (const auto &ft : filetypes) {
			NSString *type = [NSString stringWithUTF8String:ft.first.c_str()];
			if (type) { // Ensure the string conversion was successful
				[mutableTypes addObject:type];
			}
		}
		allowedTypes = [mutableTypes copy]; // Create an immutable copy if necessary
	}
	
	if (save) {
		// Configure and present the Save Panel asynchronously
		NSSavePanel *saveDlg = [NSSavePanel savePanel];
		[saveDlg setAllowedFileTypes:allowedTypes];
		[saveDlg setCanCreateDirectories:YES];
		[saveDlg setCanSelectHiddenExtension:YES];
		
		// Begin the save panel with a completion handler
		[saveDlg beginWithCompletionHandler:^(NSModalResponse result) {
			std::vector<std::string> resultPaths;
			if (result == NSModalResponseOK) {
				NSURL *url = [saveDlg URL];
				if (url) {
					NSString *path = [url path];
					if (path) {
						resultPaths.emplace_back([path UTF8String]);
					}
				}
			}
			// Invoke the callback with the result
			callback(resultPaths);
		}];
	} else {
		// Configure and present the Open Panel asynchronously
		NSOpenPanel *openDlg = [NSOpenPanel openPanel];
		[openDlg setCanChooseFiles:YES];
		[openDlg setCanChooseDirectories:NO];
		[openDlg setAllowsMultipleSelection:multiple];
		[openDlg setAllowedFileTypes:allowedTypes];
		
		// Begin the open panel with a completion handler
		[openDlg beginWithCompletionHandler:^(NSModalResponse result) {
			std::vector<std::string> resultPaths;
			if (result == NSModalResponseOK) {
				NSArray<NSURL *> *urls = [openDlg URLs];
				for (NSURL *url in urls) {
					NSString *path = [url path];
					if (path) {
						resultPaths.emplace_back([path UTF8String]);
					}
				}
			}
			// Invoke the callback with the result
			callback(resultPaths);
		}];
	}
}

void chdir_to_bundle_parent() {
	NSString *path = [[[NSBundle mainBundle] bundlePath] stringByDeletingLastPathComponent];
	chdir([path fileSystemRepresentation]);
}

void disable_saved_application_state_osx() {
	[[NSUserDefaults standardUserDefaults] setBool:YES forKey:@"NSQuitAlwaysKeepsWindows"];
}

#if defined(NANOGUI_USE_METAL)

static void *s_metal_device = nullptr;
static void *s_metal_command_queue = nullptr;

void metal_init() {
	if (s_metal_device || s_metal_command_queue)
		throw std::runtime_error("init_metal(): already initialized!");
	
	id<MTLDevice> device = MTLCreateSystemDefaultDevice();
	if (!device)
		throw std::runtime_error("init_metal(): unable to create system default device.");
	
	id<MTLCommandQueue> command_queue = [device newCommandQueue];
	if (!command_queue)
		throw std::runtime_error("init_metal(): unable to create command queue.");
	
	s_metal_device = (__bridge_retained void *)device;
	s_metal_command_queue = (__bridge_retained void *)command_queue;
}

void metal_shutdown() {
	if (s_metal_command_queue) {
		(void)(__bridge_transfer id<MTLCommandQueue>)s_metal_command_queue;
		s_metal_command_queue = nullptr;
	}
	if (s_metal_device) {
		(void)(__bridge_transfer id<MTLDevice>)s_metal_device;
		s_metal_device = nullptr;
	}
}

void* metal_device() { return s_metal_device; }
void* metal_command_queue() { return s_metal_command_queue; }

void metal_window_init(void *nswin_, bool float_buffer, bool presents_with_transaction) {
	CAMetalLayer *layer = [CAMetalLayer layer];
	if (!layer)
		throw std::runtime_error("metal_window_init(): unable to create layer.");
	
	NSWindow *nswin = (__bridge NSWindow *)nswin_;
	nswin.contentView.layer = layer;
	nswin.contentView.wantsLayer = YES;
	nswin.contentView.layerContentsPlacement = NSViewLayerContentsPlacementTopLeft;
	layer.device = (__bridge id<MTLDevice>)s_metal_device;
	layer.contentsScale = nswin.backingScaleFactor;
	
	if (float_buffer) {
		layer.wantsExtendedDynamicRangeContent = YES;
		layer.pixelFormat = MTLPixelFormatRGBA16Float;
		CGColorSpaceRef colorSpace = CGColorSpaceCreateWithName(kCGColorSpaceExtendedSRGB);
		layer.colorspace = colorSpace;
		CGColorSpaceRelease(colorSpace);
	} else {
		layer.wantsExtendedDynamicRangeContent = NO;
		layer.pixelFormat = MTLPixelFormatRGBA8Unorm;
		CGColorSpaceRef colorSpace = CGColorSpaceCreateWithName(kCGColorSpaceSRGB);
		layer.colorspace = colorSpace;
		CGColorSpaceRelease(colorSpace);
	}
	
	layer.displaySyncEnabled = YES;
	layer.allowsNextDrawableTimeout = NO;
	layer.framebufferOnly = NO;
	
	// Set presentsWithTransaction during initialization
	layer.presentsWithTransaction = presents_with_transaction;
}

std::pair<bool, bool> metal_10bit_edr_support() {
	NSArray<NSScreen *> * screens = [NSScreen screens];
	bool buffer_10bit = false,
	buffer_ext = false;
	
	for (NSScreen * screen in screens) {
		if ([screen canRepresentDisplayGamut: NSDisplayGamutP3]) {
			buffer_10bit = true; // on macOS, P3 gamut is equivalent to 10 bit color depth
		}
		
		if ([screen respondsToSelector:@selector
			 (maximumPotentialExtendedDynamicRangeColorComponentValue)]) {
			if ([screen maximumPotentialExtendedDynamicRangeColorComponentValue] >= 2.f) {
				buffer_10bit = true;
				buffer_ext = true;
			}
		}
	}
	
	return { buffer_10bit, buffer_ext };
}

void* metal_layer(void *nswin_) {
	NSWindow *nswin = (__bridge NSWindow *)nswin_;
	CAMetalLayer *layer = (CAMetalLayer *)nswin.contentView.layer;
	return (__bridge void *)layer;
}

void metal_window_set_size(void *nswin_, const Vector2i &size) {
	NSWindow *nswin = (__bridge NSWindow *)nswin_;
	CAMetalLayer *layer = (CAMetalLayer *)nswin.contentView.layer;
	layer.drawableSize = CGSizeMake(size.x(), size.y());
}

void metal_window_set_content_scale(void *nswin_, float scale) {
	NSWindow *nswin = (__bridge NSWindow *)nswin_;
	CAMetalLayer *layer = (CAMetalLayer *)nswin.contentView.layer;
	
	float old_scale = layer.contentsScale;
	if (old_scale != scale) {
		[CATransaction begin];
		[CATransaction setValue:(id)kCFBooleanTrue
						 forKey:kCATransactionDisableActions];
		layer.contentsScale = scale;
		[CATransaction commit];
	}
}

void* metal_window_layer(void *nswin_) {
	NSWindow *nswin = (__bridge NSWindow *)nswin_;
	CAMetalLayer *layer = (CAMetalLayer *)nswin.contentView.layer;
	return (__bridge void *)layer;
}

void* metal_window_next_drawable(void *nswin_) {
	NSWindow *nswin = (__bridge NSWindow *)nswin_;
	CAMetalLayer *layer = (CAMetalLayer *)nswin.contentView.layer;
	id<MTLDrawable> drawable = layer.nextDrawable;
	return (__bridge_retained void *)drawable;
}

void* metal_drawable_texture(void *drawable_) {
	id<CAMetalDrawable> drawable = (__bridge id<CAMetalDrawable>)drawable_;
	return (__bridge void *)drawable.texture;
}

void metal_present_and_release_drawable(void *drawable_) {
	id<CAMetalDrawable> drawable = (__bridge_transfer id<CAMetalDrawable>)drawable_;
	[drawable present];
}

#endif

NAMESPACE_END(nanogui)
