#include <nanogui/texture.h>
#include <stb_image.h>
#include <memory>

NAMESPACE_BEGIN(nanogui)

Texture::Texture(PixelFormat pixel_format,
				 ComponentFormat component_format,
				 const Vector2i &size,
				 InterpolationMode min_interpolation_mode,
				 InterpolationMode mag_interpolation_mode,
				 WrapMode wrap_mode,
				 uint8_t samples,
				 uint8_t flags,
				 bool mipmap_manual)
: m_pixel_format(pixel_format),
m_component_format(component_format),
m_min_interpolation_mode(min_interpolation_mode),
m_mag_interpolation_mode(mag_interpolation_mode),
m_wrap_mode(wrap_mode),
m_samples(samples),
m_flags(flags),
m_size(size),
m_mipmap_manual(mipmap_manual) {
	
	init();
}


Texture::Texture(const unsigned char* data, 			 int size,
				 int raw_width,
				 int raw_height,
				 InterpolationMode min_interpolation_mode,
				 InterpolationMode mag_interpolation_mode,
				 WrapMode wrap_mode)
: m_component_format(ComponentFormat::UInt8),
m_min_interpolation_mode(min_interpolation_mode),
m_mag_interpolation_mode(mag_interpolation_mode),
m_wrap_mode(wrap_mode),
m_samples(1),
m_flags(TextureFlags::ShaderRead),
m_mipmap_manual(false) {
	
	int channels = 0;
	// Attempt to load image data using stb_image
	unsigned char* image_data = stbi_load_from_memory(data, size, &m_size.x(), &m_size.y(), &channels, 4);
	
	// Define a shared_ptr with a custom deleter to handle both loaded and raw data
	std::shared_ptr<uint8_t> texture_data_holder;
	
	if (image_data) {
		// Successfully loaded with stb_image
		channels = 4; // We requested 4 channels (RGBA)
		texture_data_holder = std::shared_ptr<uint8_t>(image_data, stbi_image_free);
	} else {
		// Failed to load with stb_image; attempt to load raw data
		
		// Ensure raw dimensions are provided
		if (raw_width <= 0 || raw_height <= 0) {
			throw std::runtime_error("Texture::Texture(): Raw data loading failed and invalid raw dimensions provided.");
		}
		
		m_size.x() = raw_width;
		m_size.y() = raw_height;
		
		// Allocate memory for raw data
		uint8_t* raw_data = new uint8_t[size];
		std::memcpy(raw_data, data, size);
		
		// Assume 4 channels (RGBA) as per original code
		channels = 4;
		
		// Calculate the number of bytes per row
		int bytes_per_pixel = 4; // Since channels = 4 (RGBA)
		size_t row_size = raw_width * bytes_per_pixel;
		
		// Flip the raw data vertically
		for(int y = 0; y < raw_height / 2; ++y) {
			uint8_t* row_top = raw_data + y * row_size;
			uint8_t* row_bottom = raw_data + (raw_height - y - 1) * row_size;
			
			// Swap the top and bottom rows
			for(int x = 0; x < row_size; ++x) {
				std::swap(row_top[x], row_bottom[x]);
			}
		}
		
		// Assign to shared_ptr with delete[] as the deleter
		texture_data_holder = std::shared_ptr<uint8_t>(raw_data, [](uint8_t* p) { delete[] p; });
	}
	
	// Set pixel format based on the number of channels
	switch (channels) {
		case 1: m_pixel_format = PixelFormat::R;    break;
		case 2: m_pixel_format = PixelFormat::RA;   break;
		case 3: m_pixel_format = PixelFormat::RGB;  break;
		case 4: m_pixel_format = PixelFormat::RGBA; break;
		default:
			throw std::runtime_error("Texture::Texture(): Unsupported channel count!");
	}
	
	// Initialize the texture (e.g., create GPU resources)
	init();
	
	// Verify that the pixel format is supported by the hardware
	PixelFormat pixel_format = m_pixel_format;
	if (m_pixel_format != pixel_format) {
		throw std::runtime_error("Texture::Texture(): Pixel format not supported by the hardware!");
	}
	
	// Upload the texture data to the GPU
	upload(texture_data_holder.get());
}

Texture::Texture(const unsigned char* data, int size,
				 InterpolationMode min_interpolation_mode,
				 InterpolationMode mag_interpolation_mode,
				 WrapMode wrap_mode)
: m_component_format(ComponentFormat::UInt8),
m_min_interpolation_mode(min_interpolation_mode),
m_mag_interpolation_mode(mag_interpolation_mode),
m_wrap_mode(wrap_mode),
m_samples(1),
m_flags(TextureFlags::ShaderRead),
m_mipmap_manual(false) {
	
	int n = 0;
	unsigned char* image_data = stbi_load_from_memory(data, size, &m_size.x(), &m_size.y(), &n, 4);
	
	n = 4;
	
	std::unique_ptr<uint8_t[], void(*)(void*)> texture_data_holder(image_data, stbi_image_free);
	if (!texture_data_holder) {
		throw std::runtime_error("Could not load texture data from memory.");
	}
	
	switch (n) {
		case 1: m_pixel_format = PixelFormat::R;    break;
		case 2: m_pixel_format = PixelFormat::RA;   break;
		case 3: m_pixel_format = PixelFormat::RGB;  break;
		case 4: m_pixel_format = PixelFormat::RGBA; break;
		default:
			throw std::runtime_error("Texture::Texture(): unsupported channel count!");
	}
	
	PixelFormat pixel_format = m_pixel_format;
	init();
	if (m_pixel_format != pixel_format) {
		throw std::runtime_error("Texture::Texture(): pixel format not supported by the hardware!");
	}
	upload((const uint8_t*) texture_data_holder.get());
}


Texture::Texture(const std::string &filename,
				 InterpolationMode min_interpolation_mode,
				 InterpolationMode mag_interpolation_mode,
				 WrapMode wrap_mode)
: m_component_format(ComponentFormat::UInt8),
m_min_interpolation_mode(min_interpolation_mode),
m_mag_interpolation_mode(mag_interpolation_mode),
m_wrap_mode(wrap_mode),
m_samples(1),
m_flags(TextureFlags::ShaderRead),
m_mipmap_manual(false) {
	int n = 0;
	unsigned char* image_data = stbi_load(filename.c_str(), &m_size.x(), &m_size.y(), &n, 4);
		
	std::unique_ptr<uint8_t[], void(*)(void*)> texture_data_holder(image_data, stbi_image_free);

	
	n = 4;
	
	if (!texture_data_holder)
		throw std::runtime_error("Could not load texture data from file \"" + filename + "\".");
	
	switch (n) {
		case 1: m_pixel_format = PixelFormat::R;    break;
		case 2: m_pixel_format = PixelFormat::RA;   break;
		case 3: m_pixel_format = PixelFormat::RGB;  break;
		case 4: m_pixel_format = PixelFormat::RGBA; break;
		default:
			throw std::runtime_error("Texture::Texture(): unsupported channel count!");
	}
	PixelFormat pixel_format = m_pixel_format;
	init();
	if (m_pixel_format != pixel_format)
		throw std::runtime_error("Texture::Texture(): pixel format not supported by the hardware!");
	upload((const uint8_t *) texture_data_holder.get());
}

void Texture::decompress_into(const std::vector<uint8_t>& data, Texture& texture){
	
	int n = 0;
	Vector2i new_size;
	unsigned char* image_data = stbi_load_from_memory(data.data(), data.size(), &new_size.x(), &new_size.y(), &n, 4);
	
	n = 4;
	
	std::unique_ptr<uint8_t[], void(*)(void*)> texture_data_holder(image_data, stbi_image_free);
	if (!texture_data_holder) {
		throw std::runtime_error("Could not load texture data from memory.");
	}

	texture.resize(new_size);

	texture.upload((const uint8_t *) texture_data_holder.get());
}


size_t Texture::bytes_per_pixel() const {
	size_t result = 0;
	switch (m_component_format) {
		case ComponentFormat::UInt8:   result = 1; break;
		case ComponentFormat::Int8:    result = 1; break;
		case ComponentFormat::UInt16:  result = 2; break;
		case ComponentFormat::Int16:   result = 2; break;
		case ComponentFormat::UInt32:  result = 4; break;
		case ComponentFormat::Int32:   result = 4; break;
		case ComponentFormat::Float16: result = 2; break;
		case ComponentFormat::Float32: result = 4; break;
		default: throw std::runtime_error("Texture::bytes_per_pixel(): "
										  "invalid component format!");
	}
	
	return result * channels();
}

size_t Texture::channels() const {
	size_t result = 1;
	switch (m_pixel_format) {
		case PixelFormat::R:            result = 1;  break;
		case PixelFormat::RA:           result = 2;  break;
		case PixelFormat::RGB:          result = 3;  break;
		case PixelFormat::RGBA:         result = 4;  break;
		case PixelFormat::BGR:          result = 3;  break;
		case PixelFormat::BGRA:         result = 4;  break;
		case PixelFormat::Depth:        result = 1;  break;
		case PixelFormat::DepthStencil: result = 2;  break;
		default: throw std::runtime_error("Texture::channels(): invalid "
										  "pixel format!");
	}
	return result;
}

NAMESPACE_END(nanogui)
