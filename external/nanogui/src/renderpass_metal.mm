#include <nanogui/renderpass.h>
#include <nanogui/screen.h>
#include <nanogui/texture.h>
#include <nanogui/shader.h>
#include <nanogui/metal.h>
#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>

NAMESPACE_BEGIN(nanogui)

RenderPass::RenderPass(const std::vector<std::reference_wrapper<Object>> &color_targets,
					   std::optional<std::reference_wrapper<Object>> depth_target,
					   std::optional<std::reference_wrapper<Object>> stencil_target)
: m_targets(color_targets.size() + 2),
m_clear_color(color_targets.size()), m_clear_stencil(0),
m_clear_depth(1.f), m_viewport_offset(0), m_viewport_size(0),
m_framebuffer_size(0), m_depth_test(DepthTest::Less),
m_depth_write(true), m_cull_mode(CullMode::Back),
m_active(false), m_command_buffer(nullptr),
m_command_encoder(nullptr) {
	
	m_targets[0] = depth_target;
	m_targets[1] = stencil_target;
	for (size_t i = 0; i < color_targets.size(); ++i) {
		m_targets[i + 2] = color_targets[i];
		m_clear_color[i] = Color(0.f, 0.f, 0.f, 1.f);
	}
		
	if (!m_targets[0]) {
		m_depth_write = false;
		m_depth_test = DepthTest::Always;
	}
	
	MTLRenderPassDescriptor *pass_descriptor =
	[MTLRenderPassDescriptor renderPassDescriptor];
	
	for (size_t i = 0; i < m_targets.size(); ++i) {
		auto* texture = dynamic_cast<Texture*>(&m_targets[i]->get());
		auto* screen   = dynamic_cast<Screen*>(&m_targets[i]->get());
		
		if (texture) {
			if (!(texture->flags() & Texture::TextureFlags::RenderTarget))
				throw std::runtime_error("RenderPass::RenderPass(): target texture "
										 "must be created with render_target=true!");
			m_framebuffer_size = max(m_framebuffer_size, texture->size());
		} else if (screen) {
			m_framebuffer_size = max(m_framebuffer_size, screen->framebuffer_size());
		} else if (m_targets[i]) {
			throw std::runtime_error("RenderPass::RenderPass(): invalid attachment type!");
		}
	}
	m_viewport_size = m_framebuffer_size;
	
	m_pass_descriptor = (__bridge_retained void *) pass_descriptor;
	
	for (size_t i = 0; i < color_targets.size(); ++i)
		set_clear_color(i, m_clear_color[i]);
	set_clear_depth(m_clear_depth);
	set_clear_stencil(m_clear_stencil);
}

RenderPass::~RenderPass() {	
	(void) (__bridge_transfer MTLRenderPassDescriptor *) m_pass_descriptor;
}
void RenderPass::begin() {
	if (m_active)
		throw std::runtime_error("RenderPass::begin(): render pass is already active!");
	
	id<MTLCommandQueue> command_queue = (__bridge id<MTLCommandQueue>) metal_command_queue();
	id<MTLCommandBuffer> command_buffer = [command_queue commandBuffer];
	MTLRenderPassDescriptor *pass_descriptor = (__bridge MTLRenderPassDescriptor *) m_pass_descriptor;
	
	for (size_t i = 0; i < m_targets.size(); ++i) {
		auto* texture = dynamic_cast<Texture*>(&m_targets[i]->get());
		auto* screen = dynamic_cast<Screen*>(&m_targets[i]->get());
		
		id<MTLTexture> texture_handle = nil;
		
		if (texture) {
			texture_handle = (__bridge id<MTLTexture>) texture->texture_handle();
		} else if (screen) {
			if (i < 2) {
				auto screen_tex = screen->depth_stencil_texture();
				if (screen_tex && (i == 0 ||
								   (i == 1 && (screen_tex->pixel_format() ==
											   Texture::PixelFormat::DepthStencil)))) {
					texture_handle = (__bridge id<MTLTexture>) screen_tex->texture_handle();
				}
			} else {
				texture_handle = (__bridge id<MTLTexture>) screen->metal_texture();
			}
		}
		
		if (i == 0 && m_targets[0]) {  // Depth attachment
			MTLRenderPassDepthAttachmentDescriptor *att = pass_descriptor.depthAttachment;
			// Re-apply existing settings
			MTLLoadAction loadAction = att.loadAction;
			MTLStoreAction storeAction = att.storeAction;
			double clearDepth = att.clearDepth;
			
			// Set the texture
			att.texture = texture_handle;
			
			// Re-apply the settings
			att.loadAction = loadAction;
			att.storeAction = storeAction;
			att.clearDepth = clearDepth;
		} else if (i == 1 && m_targets[1]) {  // Stencil attachment
			MTLRenderPassStencilAttachmentDescriptor *att = pass_descriptor.stencilAttachment;
			// Re-apply existing settings
			MTLLoadAction loadAction = att.loadAction;
			MTLStoreAction storeAction = att.storeAction;
			uint32_t clearStencil = att.clearStencil;
			
			// Set the texture
			att.texture = texture_handle;
			
			// Re-apply the settings
			att.loadAction = loadAction;
			att.storeAction = storeAction;
			att.clearStencil = clearStencil;
		} else if (i >= 2) {  // Color attachments
			MTLRenderPassColorAttachmentDescriptor *att = pass_descriptor.colorAttachments[i - 2];
			// Re-apply existing settings
			MTLLoadAction loadAction = att.loadAction;
			MTLStoreAction storeAction = att.storeAction;
			
			auto& color = m_clear_color[i - 2];
			MTLClearColor clearColor = MTLClearColorMake(color.r(), color.g(), color.b(), color.w());
			
			// Set the texture
			att.texture = texture_handle;
			
			// Re-apply the settings
			att.loadAction = loadAction;
			att.storeAction = storeAction;
			att.clearColor = clearColor;
		}
	}
	
	id<MTLRenderCommandEncoder> command_encoder = [command_buffer renderCommandEncoderWithDescriptor:pass_descriptor];
	[command_encoder setFrontFacingWinding:MTLWindingCounterClockwise];
	
	m_command_buffer = (__bridge_retained void *)command_buffer;
	m_command_encoder = (__bridge_retained void *)command_encoder;
	m_active = true;
	
	set_viewport(m_viewport_offset, m_viewport_size);
	
	// Re-apply depth test and cull mode
	 set_depth_test(m_depth_test, m_depth_write);
	 set_cull_mode(m_cull_mode);
}


void RenderPass::end() {
#if !defined(NDEBUG)
	if (!m_active)
		throw std::runtime_error("RenderPass::end(): render pass is not active!");
#endif
	id<MTLCommandBuffer> command_buffer =
	(__bridge_transfer id<MTLCommandBuffer>) m_command_buffer;
	id<MTLRenderCommandEncoder> command_encoder =
	(__bridge_transfer id<MTLRenderCommandEncoder>) m_command_encoder;
	[command_encoder endEncoding];
	[command_buffer commit];
	m_command_encoder = nullptr;
	m_command_buffer = nullptr;
	m_active = false;
	
	mDepthTestStates.clear();
}

void RenderPass::resize(const Vector2i &size) {
	for (size_t i = 0; i < m_targets.size(); ++i) {
		auto texture = dynamic_cast<Texture*>(&m_targets[i]->get());
		if (texture)
			texture->resize(size);
	}
	m_framebuffer_size = size;
	m_viewport_offset = Vector2i(0, 0);
	m_viewport_size = size;
}


void RenderPass::set_scissor_rect(Vector2i offset, Vector2i size) {
	if (m_active) {
		id<MTLRenderCommandEncoder> command_encoder =
		(__bridge id<MTLRenderCommandEncoder>) m_command_encoder;
		
		// Convert coordinates based on pixel ratio if necessary
		// Adjust Y coordinate since Metal's origin is bottom-left
//(		Vector2i framebuffer_size = m_framebuffer_size; // Ensure this is up-to-date
//		y = framebuffer_size.y() - (y + height);
		// Clamp scissor_offset to be within [0, m_framebuffer_size]
		Vector2i clamped_offset = Vector2i(
										   std::max(0, std::min(offset.x(), m_framebuffer_size.x())),
										   std::max(0, std::min(offset.y(), m_framebuffer_size.y()))
										   );
		
		// Calculate the maximum allowable size based on the clamped offset
		Vector2i max_size = Vector2i(
									 m_framebuffer_size.x() - clamped_offset.x(),
									 m_framebuffer_size.y() - clamped_offset.y()
									 );
		
		// Clamp scissor_size to ensure it doesn't exceed the framebuffer boundaries
		Vector2i clamped_size = Vector2i(
										 std::max(0, std::min(size.x(), max_size.x())),
										 std::max(0, std::min(size.y(), max_size.y()))
										 );
		
		// Set the scissor rectangle with the clamped offset and size
		[command_encoder setScissorRect: (MTLScissorRect) {
			(NSUInteger)clamped_offset.x(),
			(NSUInteger)clamped_offset.y(),
			(NSUInteger)clamped_size.x(),
			(NSUInteger)clamped_size.y()
		}];
	}
}

void RenderPass::set_clear_color(size_t index, const Color &color) {
	m_clear_color.at(index) = color;
	
	MTLRenderPassDescriptor *pass_descriptor =
	(__bridge MTLRenderPassDescriptor *) m_pass_descriptor;
	
	pass_descriptor.colorAttachments[index].clearColor =
	MTLClearColorMake(color.r(), color.g(), color.b(), color.w());
}

void RenderPass::clear_color(size_t index, const Color &color) {
	set_clear_color(index, color);
	
	if (m_active) {
		if (index == 0) {
			id<MTLRenderCommandEncoder> command_encoder =
			(__bridge id<MTLRenderCommandEncoder>) m_command_encoder;
			
			MTLDepthStencilDescriptor *depth_desc = [MTLDepthStencilDescriptor new];
			depth_desc.depthCompareFunction = MTLCompareFunctionAlways;
			depth_desc.depthWriteEnabled = m_targets[0] != std::nullopt;
			id<MTLDevice> device = (__bridge id<MTLDevice>) metal_device();
			id<MTLDepthStencilState> depth_state =
			[device newDepthStencilStateWithDescriptor: depth_desc];
			[command_encoder setDepthStencilState: depth_state];
			
			if (!m_clear_shader) {
				m_clear_shader = std::make_shared<Shader>(
											*this,
											
											"clear_shader",
											
											/* Vertex shader */
 R"(using namespace metal;
 
 struct VertexOut {
  float4 position [[position]];
 };
 
 vertex VertexOut vertex_main(const device float2 *position,
   constant float &clear_depth,
   uint id [[vertex_id]]) {
  VertexOut vert;
  vert.position = float4(position[id], clear_depth, 1.f);
  return vert;
 })",
											
											/* Fragment shader */
 R"(using namespace metal;
 
 struct VertexOut {
  float4 position [[position]];
 };
 
 fragment float4 fragment_main(VertexOut vert [[stage_in]],
 constant float4 &clear_color) {
  return clear_color;
 })"
											);
				
				const float positions[] = {
					-1.f, -1.f, 1.f, -1.f, -1.f, 1.f,
					1.f, -1.f, 1.f,  1.f, -1.f, 1.f
				};
				
				m_clear_shader->set_buffer("position", VariableType::Float32,
										   { 6, 2 }, positions, -1, true);
			}
			
			m_clear_shader->set_uniform("clear_color", m_clear_color.at(0));
			m_clear_shader->set_uniform("clear_depth", m_clear_depth);
			m_clear_shader->begin();
			m_clear_shader->draw_array(Shader::PrimitiveType::Triangle, 0, 6, false);
			m_clear_shader->end();
		} else {
			
			// Ensure the render pass descriptor has the correct load action to clear the color buffer
			MTLRenderPassDescriptor *pass_descriptor =
			(__bridge MTLRenderPassDescriptor *) m_pass_descriptor;
			
			if (pass_descriptor.colorAttachments[index].texture) {
				pass_descriptor.colorAttachments[index].loadAction = MTLLoadActionClear;
				pass_descriptor.colorAttachments[index].clearColor = MTLClearColorMake(
																					   color.r(), color.g(), color.b(), color.w()
																					   );
			} else {
				throw std::runtime_error("RenderPass::clear_color(): No valid color attachment to clear.");
			}
		}

	}
}


void RenderPass::set_clear_depth(float depth) {
	m_clear_depth = depth;
	
	MTLRenderPassDescriptor *pass_descriptor =
	(__bridge MTLRenderPassDescriptor *) m_pass_descriptor;
	pass_descriptor.depthAttachment.clearDepth = depth;
}
void RenderPass::clear_depth(float depth) {
	// Set the clear depth value
	m_clear_depth = depth;
	
	// Ensure the render pass descriptor has the correct load action to clear the depth buffer
	MTLRenderPassDescriptor *pass_descriptor =
	(__bridge MTLRenderPassDescriptor *) m_pass_descriptor;
	
	if (pass_descriptor.depthAttachment.texture) {
		// Set load action to clear the depth buffer before rendering
		pass_descriptor.depthAttachment.loadAction = MTLLoadActionClear;
		
		// Set store action to store depth values if depth writes are enabled,
		// or discard them after rendering the gizmos (since they shouldn't affect the depth buffer)
		pass_descriptor.depthAttachment.storeAction = m_depth_write ? MTLStoreActionStore : MTLStoreActionDontCare;
		
		// Set the clear depth value
		pass_descriptor.depthAttachment.clearDepth = depth;
	} else {
		throw std::runtime_error("RenderPass::clear_depth(): No valid depth attachment to clear.");
	}
}


void RenderPass::set_clear_stencil(uint8_t stencil) {
	m_clear_stencil = stencil;
	
	MTLRenderPassDescriptor *pass_descriptor =
	(__bridge MTLRenderPassDescriptor *) m_pass_descriptor;
	pass_descriptor.stencilAttachment.clearStencil = stencil;
}

void RenderPass::set_viewport(const Vector2i &offset, const Vector2i &size) {
	m_viewport_offset = offset;
	m_viewport_size = size;
	if (m_active) {
		id<MTLRenderCommandEncoder> command_encoder =
		(__bridge id<MTLRenderCommandEncoder>) m_command_encoder;
		[command_encoder setViewport: (MTLViewport) {
			(double) offset.x(), (double) offset.y(),
			(double) size.x(),   (double) size.y(),
			0.0, 1.0 }
		];
		
		set_scissor_rect(offset, size);
	}
}

void RenderPass::set_depth_test(DepthTest depth_test, bool depth_write) {
	m_depth_test = depth_test;
	m_depth_write = depth_write;
	if (m_active) {
		MTLDepthStencilDescriptor *depth_desc = [MTLDepthStencilDescriptor new];
		if (m_targets[0]) {
			MTLCompareFunction func;
			switch (depth_test) {
				case DepthTest::Never:        func = MTLCompareFunctionNever; break;
				case DepthTest::Less:         func = MTLCompareFunctionLess; break;
				case DepthTest::Equal:        func = MTLCompareFunctionEqual; break;
				case DepthTest::LessEqual:    func = MTLCompareFunctionLessEqual; break;
				case DepthTest::Greater:      func = MTLCompareFunctionGreater; break;
				case DepthTest::NotEqual:     func = MTLCompareFunctionNotEqual; break;
				case DepthTest::GreaterEqual: func = MTLCompareFunctionGreater; break;
				case DepthTest::Always:       func = MTLCompareFunctionAlways; break;
				default:
					throw std::runtime_error(
											 "Shader::set_depth_test(): invalid depth test mode!");
			}
			depth_desc.depthCompareFunction = func;
			depth_desc.depthWriteEnabled = depth_write;
		} else {
			depth_desc.depthCompareFunction = MTLCompareFunctionAlways;
			depth_desc.depthWriteEnabled = NO;
		}
				
		id<MTLDevice> device = (__bridge id<MTLDevice>) metal_device();
		id<MTLDepthStencilState> depth_state =
		[device newDepthStencilStateWithDescriptor: depth_desc];
		id<MTLRenderCommandEncoder> command_encoder =
		(__bridge id<MTLRenderCommandEncoder>) m_command_encoder;
		[command_encoder setDepthStencilState: depth_state];
	}
}

void RenderPass::set_cull_mode(CullMode cull_mode) {
	m_cull_mode = cull_mode;
	if (m_active) {
		MTLCullMode cull_mode_mtl;
		switch (cull_mode) {
			case CullMode::Front:    cull_mode_mtl = MTLCullModeFront; break;
			case CullMode::Back:     cull_mode_mtl = MTLCullModeBack; break;
			case CullMode::Disabled: cull_mode_mtl = MTLCullModeNone; break;
			default: throw std::runtime_error("Shader::set_cull_mode(): invalid cull mode!");
		}
		id<MTLRenderCommandEncoder> command_encoder =
		(__bridge id<MTLRenderCommandEncoder>) m_command_encoder;
		[command_encoder setCullMode: cull_mode_mtl];
	}
}

NAMESPACE_END(nanogui)
