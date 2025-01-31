#include <nanogui/shader.h>
#include <nanogui/metal.h>
#include <nanogui/screen.h>
#include <nanogui/texture.h>
#include <nanogui/renderpass.h>
#include <iostream>

#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>

#define NANOGUI_BUFFER_THRESHOLD 64

NAMESPACE_BEGIN(nanogui)

id<MTLFunction> compile_metal_shader(id<MTLDevice> device,
									 const std::string &name,
									 const std::string &type_str,
									 const std::string &src) {
	if (src.empty())
		return nil;
	
	id<MTLLibrary> library = nil;
	NSError *error = nil;
	std::string activity;
	if (src.size() > 4 && strncmp(src.data(), "MTLB", 4) == 0) {
		dispatch_data_t data = dispatch_data_create(
													src.data(), src.size(), NULL, DISPATCH_DATA_DESTRUCTOR_DEFAULT);
		library = [device newLibraryWithData:data error:&error];
		activity = "load";
	} else {
		NSString *str = [NSString stringWithUTF8String: src.c_str()];
		MTLCompileOptions *opts = [MTLCompileOptions new];
		library = [device newLibraryWithSource:str options:opts error:&error];
		activity = "compile";
	}
	if (error) {
		const char *error_shader = [[error description] UTF8String];
		throw std::runtime_error(
								 std::string("compile_metal_shader(): unable to ") + activity + " " +
								 type_str + " shader \"" + name + "\":\n\n" + error_shader);
	}
	
	NSArray<NSString *> *function_names = [library functionNames];
	if ([function_names count] != 1)
		throw std::runtime_error("compile_metal_shader(name=\"" + name +
								 "\"): library must contain exactly 1 shader!");
	NSString *function_name = [function_names objectAtIndex: 0];
	
	id<MTLFunction> function = [library newFunctionWithName: function_name];
	if (!function)
		throw std::runtime_error("compile_metal_shader(name=\"" + name +
								 "\"): function not found!");
	
	return function;
}

Shader::Shader(RenderPass& render_pass,
			   const std::string &name,
			   const std::string &vertex_shader,
			   const std::string &fragment_shader,
			   BlendMode blend_mode)
: m_render_pass(render_pass), m_name(name), m_blend_mode(blend_mode), m_pipeline_state(nullptr) {
	id<MTLDevice> device = (__bridge id<MTLDevice>) metal_device();
	id<MTLFunction> vertex_func   = compile_metal_shader(device, name, "vertex", vertex_shader),
	fragment_func = compile_metal_shader(device, name, "fragment", fragment_shader);
	
	MTLRenderPipelineDescriptor *pipeline_desc =
	[MTLRenderPipelineDescriptor new];
	pipeline_desc.vertexFunction = vertex_func;
	pipeline_desc.fragmentFunction = fragment_func;
	
	auto& targets = render_pass.targets();
	int sample_count = 1;
	
	for (size_t i = 0; i < targets.size(); ++i) {
		auto texture = dynamic_cast<Texture*>(&targets[i]->get());
		auto screen = dynamic_cast<Screen*>(&targets[i]->get());
		
		MTLPixelFormat pixel_format;
		if (targets[i] == std::nullopt) {
			continue;
		} else if (screen) {
			if (i == 0 || i == 1) {
				auto depth_stencil_texture = screen->depth_stencil_texture();
				if (!depth_stencil_texture ||
					(i == 1 && depth_stencil_texture->pixel_format() !=
					 Texture::PixelFormat::DepthStencil))
					throw std::runtime_error("Shader::Shader(): Screen not configured for depth/stencil rendering");
				pixel_format = ((__bridge id<MTLTexture>) depth_stencil_texture->texture_handle()).pixelFormat;
			} else {
				pixel_format = ((__bridge CAMetalLayer *) screen->metal_layer()).pixelFormat;
			}
		} else if (texture) {
			pixel_format = ((__bridge id<MTLTexture>) texture->texture_handle()).pixelFormat;
			sample_count = std::max((int) texture->samples(), sample_count);
		} else {
			throw std::runtime_error(
									 "Shader::Shader(): invalid target type! (either a Screen or "
									 "Texture instance must be specified!)");
		}
		
		if (i == 0)
			pipeline_desc.depthAttachmentPixelFormat = pixel_format;
		else if (i == 1)
			pipeline_desc.stencilAttachmentPixelFormat = pixel_format;
		else
			pipeline_desc.colorAttachments[i-2].pixelFormat = pixel_format;
		
		if (i == 2 && blend_mode == BlendMode::AlphaBlend) {
			MTLRenderPipelineColorAttachmentDescriptor *att =
			pipeline_desc.colorAttachments[i - 2];
			
			att.blendingEnabled             = YES;
			att.rgbBlendOperation           = MTLBlendOperationAdd;
			att.alphaBlendOperation         = MTLBlendOperationAdd;
			
			att.sourceRGBBlendFactor        = MTLBlendFactorSourceAlpha;
			att.sourceAlphaBlendFactor      = MTLBlendFactorSourceAlpha;
			
			att.destinationRGBBlendFactor   = MTLBlendFactorOneMinusSourceAlpha;
			att.destinationAlphaBlendFactor = MTLBlendFactorOneMinusSourceAlpha;
		}
	}
	
	pipeline_desc.sampleCount = sample_count;
	
	NSError *error = nil;
	MTLRenderPipelineReflection *reflection = nil;
	id<MTLRenderPipelineState> pipeline_state =
	[device newRenderPipelineStateWithDescriptor: pipeline_desc
										 options: MTLPipelineOptionArgumentInfo
									  reflection: &reflection
										   error: &error];
	
	if (error) {
		const char *error_pipeline = [[error description] UTF8String];
		throw std::runtime_error(
								 "compile_metal_pipeline(): unable to create render pipeline state!\n\n" +
								 std::string(error_pipeline));
	}
	
	m_pipeline_state = (__bridge_retained void *) pipeline_state;
	
	// Initialize buffer definitions
	for (MTLArgument *arg in [reflection vertexArguments]) {
		std::string name = [arg.name UTF8String];
		if (name == "indices")
			throw std::runtime_error(
									 "Shader::Shader(): argument name 'indices' is reserved!");
		
		BufferDefinition buf_def;
		buf_def.index = arg.index;
		if (arg.type == MTLArgumentTypeBuffer)
			buf_def.type = VertexBuffer;
		else if (arg.type == MTLArgumentTypeTexture) {
			buf_def.type = VertexTexture;
			buf_def.size = arg.arrayLength;
		}
		else if (arg.type == MTLArgumentTypeSampler) {
			buf_def.type = VertexSampler;
			buf_def.size = arg.arrayLength;
		}
		else
			throw std::runtime_error("Shader::Shader(): \"" + name +
									 "\": unsupported argument type!");
		
		m_buffer_definitions[name] = buf_def;
	}
	
	for (MTLArgument *arg in [reflection fragmentArguments]) {
		std::string name = [arg.name UTF8String];
		if (name == "indices")
			throw std::runtime_error(
									 "Shader::Shader(): argument name 'indices' is reserved!");
		
		BufferDefinition buf_def;
		buf_def.index = arg.index;
		if (arg.type == MTLArgumentTypeBuffer)
			buf_def.type = FragmentBuffer;
		else if (arg.type == MTLArgumentTypeTexture) {
			buf_def.type = FragmentTexture;
			buf_def.size = arg.arrayLength;
		}
		else if (arg.type == MTLArgumentTypeSampler) {
			buf_def.type = FragmentSampler;
			buf_def.size = arg.arrayLength;
		}
		else
			throw std::runtime_error("Shader::Shader(): \"" + name +
									 "\": unsupported argument type!");
		
		m_buffer_definitions[name] = buf_def;
	}
	
	// Define the 'indices' buffer
	BufferDefinition buf_def;
	buf_def.index = -1;
	buf_def.type = IndexBuffer;
	m_buffer_definitions["indices"] = buf_def;
}

Shader::~Shader() {
	auto release_buffers = [](auto &buffers_map) {
		for (auto &pair : buffers_map) {
			for (auto &index_pair : pair.second) {
				Buffer &buf = index_pair.second;
				if (!buf.buffer)
					continue;
				if (buf.type == VertexBuffer ||
					buf.type == FragmentBuffer ||
					buf.type == IndexBuffer) {
					if (buf.size <= NANOGUI_BUFFER_THRESHOLD)
						delete[] (uint8_t *) buf.buffer;
					else
						(void) (__bridge_transfer id<MTLBuffer>) buf.buffer;
				} else if (buf.type == VertexTexture ||
						   buf.type == FragmentTexture) {
					(void) (__bridge_transfer id<MTLTexture>) buf.buffer;
				} else if (buf.type == VertexSampler ||
						   buf.type == FragmentSampler) {
					(void) (__bridge_transfer id<MTLSamplerState>) buf.buffer;
				} else {
					std::cerr << "Shader::~Shader(): unknown buffer type!" << std::endl;
				}
			}
		}
	};
	
	release_buffers(m_persisted_buffers);
	release_buffers(m_queued_buffers);
	
	(void) (__bridge_transfer id<MTLRenderPipelineState>) m_pipeline_state;
}

void Shader::set_buffer(const std::string &name,
						VariableType dtype,
						size_t ndim,
						const size_t *shape,
						const void *data,
						int index,
						bool persist) {
	auto it = m_buffer_definitions.find(name);
	if (it == m_buffer_definitions.end())
		throw std::runtime_error(
								 "Shader::set_buffer(): could not find argument named \"" + name + "\"");
	const BufferDefinition &buf_def = it->second;
	if (!(buf_def.type == VertexBuffer ||
		  buf_def.type == FragmentBuffer ||
		  buf_def.type == IndexBuffer))
		throw std::runtime_error(
								 "Shader::set_buffer(): argument named \"" + name + "\" is not a buffer!");
	
	Buffer buf;
	buf.dtype = dtype;
	buf.ndim  = ndim;
	for (size_t i = 0; i < 3; ++i)
		buf.shape[i] = i < ndim ? shape[i] : 1;
	buf.size  = type_size(dtype) * buf.shape[0] * buf.shape[1] * buf.shape[2];
	buf.type  = buf_def.type;
	buf.index = index != -1 ? index : buf_def.index;
	
	// Allocate or update the buffer
	if (buf.size <= NANOGUI_BUFFER_THRESHOLD && name != "indices") {
		buf.buffer = new uint8_t[buf.size];
		memcpy(buf.buffer, data, buf.size);
	} else {
		/* Procedure recommended by Apple: create a temporary shared buffer and
		 blit into a private GPU-only buffer */
		id<MTLDevice> device = (__bridge id<MTLDevice>) metal_device();
		id<MTLBuffer> mtl_buffer =
		[device newBufferWithLength: buf.size
							options: MTLResourceStorageModePrivate];
		
		id<MTLBuffer> temp_buffer =
		[device newBufferWithBytes: data
							length: buf.size
						   options: MTLResourceStorageModeShared];
		
		id<MTLCommandQueue> command_queue =
		(__bridge id<MTLCommandQueue>) metal_command_queue();
		id<MTLCommandBuffer> command_buffer = [command_queue commandBuffer];
		id<MTLBlitCommandEncoder> blit_encoder =
		[command_buffer blitCommandEncoder];
		
		[blit_encoder copyFromBuffer: temp_buffer
						sourceOffset: 0
							toBuffer: mtl_buffer
				   destinationOffset: 0
								size: buf.size];
		
		[blit_encoder endEncoding];
		[command_buffer commit];
		
		buf.buffer = (__bridge_retained void *) mtl_buffer;
	}
	
	if (persist) {
		m_persisted_buffers[name][buf.index] = buf;
	} else {
		m_queued_buffers[name][buf.index] = buf;
	}
}

size_t Shader::get_buffer_size(const std::string &name) {
	auto it_persisted = m_buffer_definitions.find(name);
	if (it_persisted != m_buffer_definitions.end())
		return it_persisted->second.size;
	throw std::runtime_error("Shader::get_buffer_size(): buffer \"" + name + "\" not found");
}

void Shader::set_texture(const std::string &name, std::shared_ptr<Texture> texture, int index, bool persist) {
	auto it = m_buffer_definitions.find(name);
	if (it == m_buffer_definitions.end())
		throw std::runtime_error(
								 "Shader::set_texture(): could not find argument named \"" + name + "\"");
	
	const BufferDefinition &buf_def = it->second;
	if (!(buf_def.type == VertexTexture || buf_def.type == FragmentTexture))
		throw std::runtime_error(
								 "Shader::set_texture(): argument named \"" + name + "\" is not a texture!");
	
	Buffer buf;
	buf.type = buf_def.type;
	buf.index = index != -1 ? index : buf_def.index;
	buf.buffer = (__bridge_retained void *)((__bridge id<MTLTexture>)texture->texture_handle());
	
	// Handle the sampler as well
	std::string sampler_name;
	if (name.length() > 8 && name.compare(name.length() - 8, 8, "_texture") == 0)
		sampler_name = name.substr(0, name.length() - 8) + "_sampler";
	else
		sampler_name = name + "_sampler";
	
	auto sampler_it = m_buffer_definitions.find(sampler_name);
	if (sampler_it != m_buffer_definitions.end()) {
		const BufferDefinition &sampler_def = sampler_it->second;
		Buffer buf2;
		buf2.type = sampler_def.type;
		buf2.index = index != -1 ? index : sampler_def.index;
		buf2.buffer = (__bridge_retained void *)((__bridge id<MTLSamplerState>)texture->sampler_state_handle());
		
		if (persist) {
			m_persisted_buffers[sampler_name][buf2.index] = buf2;
		} else {
			m_queued_buffers[sampler_name][buf2.index] = buf2;
		}
	}
	
	if (persist) {
		m_persisted_buffers[name][buf.index] = buf;
	} else {
		m_queued_buffers[name][buf.index] = buf;
	}
}

void Shader::begin() {
	m_persisted_buffers["indices"][m_buffer_definitions["indices"].index] = m_persisted_buffers["indices"][m_buffer_definitions["indices"].index];
	
	id<MTLRenderPipelineState> pipeline_state = (__bridge id<MTLRenderPipelineState>)m_pipeline_state;
	id<MTLRenderCommandEncoder> command_enc = (__bridge id<MTLRenderCommandEncoder>)render_pass().command_encoder();
	
	[command_enc setRenderPipelineState: pipeline_state];
	
	// Process all persisted buffers
	for (auto &[key, buffers] : m_persisted_buffers) {
		for (auto &[index, buf] : buffers) {
			if (!buf.buffer) {
//				bool indices = buf.type == IndexBuffer;
//				if (!indices)
//					fprintf(stderr,
//							"Shader::begin(): shader \"%s\" has an unbound "
//							"argument \"%s\"!\n",
//							m_name.c_str(), key.c_str());
				continue;
			}
			
			switch (buf.type) {
				case VertexTexture: {
					id<MTLTexture> texture = (__bridge id<MTLTexture>)buf.buffer;
					[command_enc setVertexTexture: texture atIndex: buf.index];
					break;
				}
					
				case FragmentTexture: {
					id<MTLTexture> texture = (__bridge id<MTLTexture>)buf.buffer;
					[command_enc setFragmentTexture: texture atIndex: buf.index];
					break;
				}
					
				case VertexSampler: {
					id<MTLSamplerState> state = (__bridge id<MTLSamplerState>)buf.buffer;
					[command_enc setVertexSamplerState: state atIndex: buf.index];
					break;
				}
					
				case FragmentSampler: {
					id<MTLSamplerState> state = (__bridge id<MTLSamplerState>)buf.buffer;
					[command_enc setFragmentSamplerState: state atIndex: buf.index];
					break;
				}
					
				default:
					if (buf.size <= NANOGUI_BUFFER_THRESHOLD && buf.type != IndexBuffer) {
						if (buf.type == VertexBuffer)
							[command_enc setVertexBytes: buf.buffer
												 length: buf.size
												atIndex: buf.index];
						else if (buf.type == FragmentBuffer)
							[command_enc setFragmentBytes: buf.buffer
												   length: buf.size
												  atIndex: buf.index];
						else
							throw std::runtime_error("Shader::begin(): unexpected buffer type!");
					} else {
						id<MTLBuffer> buffer = (__bridge id<MTLBuffer>) buf.buffer;
						if (buf.type == VertexBuffer)
							[command_enc setVertexBuffer: buffer
												  offset: 0
												 atIndex: buf.index];
						else if (buf.type == FragmentBuffer)
							[command_enc setFragmentBuffer: buffer
													offset: 0
												   atIndex: buf.index];
					}
					break;
			}
		}
	}
	
	// Process all queued buffers
	for (auto &[key, buffers] : m_queued_buffers) {
		for (auto &[index, buf] : buffers) {
			if (!buf.buffer) {
//				bool indices = buf.type == IndexBuffer;
//				if (!indices)
//					fprintf(stderr,
//							"Shader::begin(): shader \"%s\" has an unbound "
//							"argument \"%s\"!\n",
//							m_name.c_str(), key.c_str());
				continue;
			}
			
			switch (buf.type) {
				case VertexTexture: {
					id<MTLTexture> texture = (__bridge id<MTLTexture>)buf.buffer;
					[command_enc setVertexTexture: texture atIndex: buf.index];
					break;
				}
					
				case FragmentTexture: {
					id<MTLTexture> texture = (__bridge id<MTLTexture>)buf.buffer;
					[command_enc setFragmentTexture: texture atIndex: buf.index];
					break;
				}
					
				case VertexSampler: {
					id<MTLSamplerState> state = (__bridge id<MTLSamplerState>)buf.buffer;
					[command_enc setVertexSamplerState: state atIndex: buf.index];
					break;
				}
					
				case FragmentSampler: {
					id<MTLSamplerState> state = (__bridge id<MTLSamplerState>)buf.buffer;
					[command_enc setFragmentSamplerState: state atIndex: buf.index];
					break;
				}
					
				default:
					if (buf.size <= NANOGUI_BUFFER_THRESHOLD && buf.type != IndexBuffer) {
						if (buf.type == VertexBuffer)
							[command_enc setVertexBytes: buf.buffer
												 length: buf.size
												atIndex: buf.index];
						else if (buf.type == FragmentBuffer)
							[command_enc setFragmentBytes: buf.buffer
												   length: buf.size
												  atIndex: buf.index];
						else
							throw std::runtime_error("Shader::begin(): unexpected buffer type!");
					} else {
						id<MTLBuffer> buffer = (__bridge id<MTLBuffer>) buf.buffer;
						if (buf.type == VertexBuffer)
							[command_enc setVertexBuffer: buffer
												  offset: 0
												 atIndex: buf.index];
						else if (buf.type == FragmentBuffer)
							[command_enc setFragmentBuffer: buffer
													offset: 0
												   atIndex: buf.index];
					}
					break;
			}
		}
	}
}

void Shader::end() {
	m_queued_buffers.clear();
}

void Shader::draw_array(PrimitiveType primitive_type,
						size_t offset, size_t count,
						bool indexed) {
	MTLPrimitiveType primitive_type_mtl;
	switch (primitive_type) {
		case PrimitiveType::Point:         primitive_type_mtl = MTLPrimitiveTypePoint;         break;
		case PrimitiveType::Line:          primitive_type_mtl = MTLPrimitiveTypeLine;          break;
		case PrimitiveType::LineStrip:     primitive_type_mtl = MTLPrimitiveTypeLineStrip;     break;
		case PrimitiveType::Triangle:      primitive_type_mtl = MTLPrimitiveTypeTriangle;      break;
		case PrimitiveType::TriangleStrip: primitive_type_mtl = MTLPrimitiveTypeTriangleStrip; break;
		default: throw std::runtime_error("Shader::draw_array(): invalid primitive type!");
	}
	
	id<MTLRenderCommandEncoder> command_enc =
	(__bridge id<MTLRenderCommandEncoder>) render_pass().command_encoder();
	
	if (!indexed) {
		[command_enc drawPrimitives: primitive_type_mtl
						vertexStart: offset
						vertexCount: count];
	} else {
		id<MTLBuffer> index_buffer;
		
		if (m_persisted_buffers.find("indices") != m_persisted_buffers.end()) {
			index_buffer = (__bridge id<MTLBuffer>) m_persisted_buffers["indices"][-1].buffer;
			
		} else {
			throw std::runtime_error("Shader::draw_array(): index buffer 'indices' not found or queued index buffers are not supported.");
		}
		
		[command_enc drawIndexedPrimitives: primitive_type_mtl
								indexCount: count
								 indexType: MTLIndexTypeUInt32
							   indexBuffer: index_buffer
						 indexBufferOffset: offset * 4];
	}
}

NAMESPACE_END(nanogui)
