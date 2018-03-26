#pragma once

#include "pfm-def.h"

#if !defined NDEBUG && !defined _DEBUG

   #define CHECK_GL_ERRORS

#endif

#define OPENGL_ES_2_0 20
#define OPENGL_ES_3_0 30
#define OPENGL_ES_3_1 31
#define OPENGL_ES_MAX_AVAILABLE 1

#if defined PLATFORM_ANDROID

	#define USES_OPENGL_ES
	#include <EGL/egl.h>
	#include <GLES3/gl3.h>
	#include <GLES3/gl3ext.h>

#elif defined PLATFORM_EMSCRIPTEN

   #define USES_OPENGL_ES
   #define OPENGL_ES_VERSION OPENGL_ES_3_0

   #if OPENGL_ES_VERSION == OPENGL_ES_2_0

      #define GL_GLEXT_PROTOTYPES
      #include <GLES2/gl2.h>
      #include <GLES2/gl2ext.h>

   #elif OPENGL_ES_VERSION == OPENGL_ES_3_0

      #include <GLES3/gl3.h>
      #include <GLES3/gl2ext.h>

   #endif // OPENGL_ES_VERSION == OPENGL_ES_2_0

#elif defined PLATFORM_IOS

	#define USES_OPENGL_ES
	#include <EGL/egl.h>
	#include <GLES3/gl3.h>
	#include <GLES3/gl3ext.h>

#elif defined PLATFORM_WINDOWS_PC || defined PLATFORM_QT_WINDOWS_PC

	#define USES_OPENGL
	//#define USES_OPENGL_ES
	#if defined USES_OPENGL_ES

		#include <EGL/egl.h>
		#include <GLES/gl.h>
		#include <GLES/glext.h>
		#include <GLU/glu.h>

		#pragma comment (lib, "libEGL.lib")
		#pragma comment (lib, "libgles_cm.lib")
		#pragma comment (lib, "iglu.lib")

	#else

		#ifndef NOMINMAX
			#define NOMINMAX
		#endif
		#ifndef WIN32_LEAN_AND_MEAN
			#define WIN32_LEAN_AND_MEAN
		#endif

		#include <windows.h>
		//#include <stdint.h>

		#define USES_OPENGL_GLEW
		//#define GLEW_STATIC
		#include <gl/glew.h>
		#include <gl/gl.h>
		#include <gl/wglew.h>
		#include <gl/glu.h>

		#pragma comment (lib, "glew32.lib")
		#pragma comment (lib, "opengl32.lib")
		#pragma comment (lib, "glu32.lib")

	#endif

#endif


int mws_is_gl_extension_supported(const char* i_extension);
void mws_tex_img_2d(GLenum target, GLint mipmap_count, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void* pixels);


#if defined USES_OPENGL_ES

	#define ia_glGetProcAddress(name) eglGetProcAddress(name)

	#ifndef GL_TEXTURE_MAX_ANISOTROPY_EXT
	   #define GL_TEXTURE_MAX_ANISOTROPY_EXT 0x84FE
	#endif

   #ifndef GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT
	   #define GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT 0x84FF
	#endif

   #define GL_MULTISAMPLE 0x809D

   #if (OPENGL_ES_VERSION > OPENGL_ES_2_0)

      #define USES_GL_STRINGI
      #define USES_GL_TEX_STORAGE_2D

   #elif (OPENGL_ES_VERSION == OPENGL_ES_2_0) && !defined GL_RGBA8

      #define glReadBuffer(target) vprint("glReadBuffer N/A!\n")
      #define glVertexAttribIPointer(index, size, type, stride, pointer) vprint("glVertexAttribIPointer N/A!\n")

      #define GL_HALF_FLOAT 0x140B

      #define GL_RED 0x1903
      #define GL_GREEN 0x1904
      #define GL_BLUE 0x1905

      #define GL_INTENSITY16 0x804D
      #define GL_RGB4 0x804F
      #define GL_RGB5 0x8050
      #define GL_RGB8 0x8051
      #define GL_RGB10 0x8052
      #define GL_RGB12 0x8053
      #define GL_RGB16 0x8054
      #define GL_RGBA2 0x8055
      #define GL_RGBA4 0x8056
      #define GL_RGB5_A1 0x8057
      #define GL_RGBA8 0x8058
      #define GL_RGB10_A2 0x8059
      #define GL_RGBA12 0x805A
      #define GL_RGBA16 0x805B

      #define GL_DEPTH_COMPONENT16 0x81A5
      #define GL_DEPTH_COMPONENT24 0x81A6
      #define GL_DEPTH_COMPONENT32 0x81A7

      #define GL_COMPRESSED_RED 0x8225
      #define GL_COMPRESSED_RG 0x8226
      #define GL_RG 0x8227
      #define GL_RG_INTEGER 0x8228
      #define GL_R8 0x8229
      #define GL_R16 0x822A
      #define GL_RG8 0x822B
      #define GL_RG16 0x822C
      #define GL_R16F 0x822D
      #define GL_R32F 0x822E
      #define GL_RG16F 0x822F
      #define GL_RG32F 0x8230
      #define GL_R8I 0x8231
      #define GL_R8UI 0x8232
      #define GL_R16I 0x8233
      #define GL_R16UI 0x8234
      #define GL_R32I 0x8235
      #define GL_R32UI 0x8236
      #define GL_RG8I 0x8237
      #define GL_RG8UI 0x8238
      #define GL_RG16I 0x8239
      #define GL_RG16UI 0x823A
      #define GL_RG32I 0x823B
      #define GL_RG32UI 0x823C

      #define GL_TEXTURE_WRAP_R 0x8072
      #define GL_UNSIGNED_BYTE_2_3_3_REV 0x8362
      #define GL_UNSIGNED_SHORT_5_6_5 0x8363
      #define GL_UNSIGNED_SHORT_5_6_5_REV 0x8364
      #define GL_UNSIGNED_SHORT_4_4_4_4_REV 0x8365
      #define GL_UNSIGNED_SHORT_1_5_5_5_REV 0x8366
      #define GL_UNSIGNED_INT_8_8_8_8_REV 0x8367
      #define GL_UNSIGNED_INT_2_10_10_10_REV 0x8368
      #define GL_CURRENT_RASTER_SECONDARY_COLOR 0x845F

      #define GL_DEPTH_STENCIL 0x84F9
      #define GL_UNSIGNED_INT_24_8 0x84FA
      #define GL_DEPTH24_STENCIL8 0x88F0

      #define GL_SAMPLER_3D 0x8B5F
      #define GL_SAMPLER_CUBE 0x8B60
      #define GL_SAMPLER_1D_SHADOW 0x8B61
      #define GL_SAMPLER_2D_SHADOW 0x8B62

      #define GL_PIXEL_PACK_BUFFER 0x88EB
      #define GL_PIXEL_UNPACK_BUFFER 0x88EC
      #define GL_PIXEL_PACK_BUFFER_BINDING 0x88ED
      #define GL_PIXEL_UNPACK_BUFFER_BINDING 0x88EF
      #define GL_FLOAT_MAT2x3 0x8B65
      #define GL_FLOAT_MAT2x4 0x8B66
      #define GL_FLOAT_MAT3x2 0x8B67
      #define GL_FLOAT_MAT3x4 0x8B68
      #define GL_FLOAT_MAT4x2 0x8B69
      #define GL_FLOAT_MAT4x3 0x8B6A
      #define GL_SRGB 0x8C40
      #define GL_SRGB8 0x8C41
      #define GL_SRGB_ALPHA 0x8C42
      #define GL_SRGB8_ALPHA8 0x8C43
      #define GL_SLUMINANCE_ALPHA 0x8C44
      #define GL_SLUMINANCE8_ALPHA8 0x8C45
      #define GL_SLUMINANCE 0x8C46
      #define GL_SLUMINANCE8 0x8C47
      #define GL_COMPRESSED_SRGB 0x8C48
      #define GL_COMPRESSED_SRGB_ALPHA 0x8C49
      #define GL_COMPRESSED_SLUMINANCE 0x8C4A
      #define GL_COMPRESSED_SLUMINANCE_ALPHA 0x8C4B

      #define GL_DEPTH_COMPONENT32F 0x8CAC
      #define GL_DEPTH32F_STENCIL8 0x8CAD
      #define GL_FLOAT_32_UNSIGNED_INT_24_8_REV 0x8DAD

      #define GL_MAX_CLIP_DISTANCES GL_MAX_CLIP_PLANES
      #define GL_CLIP_DISTANCE5 GL_CLIP_PLANE5
      #define GL_CLIP_DISTANCE1 GL_CLIP_PLANE1
      #define GL_CLIP_DISTANCE3 GL_CLIP_PLANE3
      #define GL_COMPARE_REF_TO_TEXTURE GL_COMPARE_R_TO_TEXTURE_ARB
      #define GL_CLIP_DISTANCE0 GL_CLIP_PLANE0
      #define GL_CLIP_DISTANCE4 GL_CLIP_PLANE4
      #define GL_CLIP_DISTANCE2 GL_CLIP_PLANE2
      #define GL_MAX_VARYING_COMPONENTS GL_MAX_VARYING_FLOATS
      #define GL_CONTEXT_FLAG_FORWARD_COMPATIBLE_BIT 0x0001
      #define GL_MAJOR_VERSION 0x821B
      #define GL_MINOR_VERSION 0x821C
      #define GL_NUM_EXTENSIONS 0x821D
      #define GL_CONTEXT_FLAGS 0x821E
      #define GL_DEPTH_BUFFER 0x8223
      #define GL_STENCIL_BUFFER 0x8224
      #define GL_COMPRESSED_RED 0x8225
      #define GL_COMPRESSED_RG 0x8226
      #define GL_RGBA32F 0x8814
      #define GL_RGB32F 0x8815
      #define GL_RGBA16F 0x881A
      #define GL_RGB16F 0x881B
      #define GL_VERTEX_ATTRIB_ARRAY_INTEGER 0x88FD
      #define GL_MAX_ARRAY_TEXTURE_LAYERS 0x88FF
      #define GL_MIN_PROGRAM_TEXEL_OFFSET 0x8904
      #define GL_MAX_PROGRAM_TEXEL_OFFSET 0x8905
      #define GL_CLAMP_VERTEX_COLOR 0x891A
      #define GL_CLAMP_FRAGMENT_COLOR 0x891B
      #define GL_CLAMP_READ_COLOR 0x891C
      #define GL_FIXED_ONLY 0x891D
      #define GL_TEXTURE_RED_TYPE 0x8C10
      #define GL_TEXTURE_GREEN_TYPE 0x8C11
      #define GL_TEXTURE_BLUE_TYPE 0x8C12
      #define GL_TEXTURE_ALPHA_TYPE 0x8C13
      #define GL_TEXTURE_LUMINANCE_TYPE 0x8C14
      #define GL_TEXTURE_INTENSITY_TYPE 0x8C15
      #define GL_TEXTURE_DEPTH_TYPE 0x8C16
      #define GL_TEXTURE_1D_ARRAY 0x8C18
      #define GL_PROXY_TEXTURE_1D_ARRAY 0x8C19
      #define GL_TEXTURE_2D_ARRAY 0x8C1A
      #define GL_PROXY_TEXTURE_2D_ARRAY 0x8C1B
      #define GL_TEXTURE_BINDING_1D_ARRAY 0x8C1C
      #define GL_TEXTURE_BINDING_2D_ARRAY 0x8C1D
      #define GL_R11F_G11F_B10F 0x8C3A
      #define GL_UNSIGNED_INT_10F_11F_11F_REV 0x8C3B
      #define GL_RGB9_E5 0x8C3D
      #define GL_UNSIGNED_INT_5_9_9_9_REV 0x8C3E
      #define GL_TEXTURE_SHARED_SIZE 0x8C3F
      #define GL_TRANSFORM_FEEDBACK_VARYING_MAX_LENGTH 0x8C76
      #define GL_TRANSFORM_FEEDBACK_BUFFER_MODE 0x8C7F
      #define GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_COMPONENTS 0x8C80
      #define GL_TRANSFORM_FEEDBACK_VARYINGS 0x8C83
      #define GL_TRANSFORM_FEEDBACK_BUFFER_START 0x8C84
      #define GL_TRANSFORM_FEEDBACK_BUFFER_SIZE 0x8C85
      #define GL_PRIMITIVES_GENERATED 0x8C87
      #define GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN 0x8C88
      #define GL_RASTERIZER_DISCARD 0x8C89
      #define GL_MAX_TRANSFORM_FEEDBACK_INTERLEAVED_COMPONENTS 0x8C8A
      #define GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_ATTRIBS 0x8C8B
      #define GL_INTERLEAVED_ATTRIBS 0x8C8C
      #define GL_SEPARATE_ATTRIBS 0x8C8D
      #define GL_TRANSFORM_FEEDBACK_BUFFER 0x8C8E
      #define GL_TRANSFORM_FEEDBACK_BUFFER_BINDING 0x8C8F
      #define GL_RGBA32UI 0x8D70
      #define GL_RGB32UI 0x8D71
      #define GL_RGBA16UI 0x8D76
      #define GL_RGB16UI 0x8D77
      #define GL_RGBA8UI 0x8D7C
      #define GL_RGB8UI 0x8D7D
      #define GL_RGBA32I 0x8D82
      #define GL_RGB32I 0x8D83
      #define GL_RGBA16I 0x8D88
      #define GL_RGB16I 0x8D89
      #define GL_RGBA8I 0x8D8E
      #define GL_RGB8I 0x8D8F
      #define GL_RED_INTEGER 0x8D94
      #define GL_GREEN_INTEGER 0x8D95
      #define GL_BLUE_INTEGER 0x8D96
      #define GL_ALPHA_INTEGER 0x8D97
      #define GL_RGB_INTEGER 0x8D98
      #define GL_RGBA_INTEGER 0x8D99
      #define GL_BGR_INTEGER 0x8D9A
      #define GL_BGRA_INTEGER 0x8D9B
      #define GL_SAMPLER_1D_ARRAY 0x8DC0
      #define GL_SAMPLER_2D_ARRAY 0x8DC1
      #define GL_SAMPLER_1D_ARRAY_SHADOW 0x8DC3
      #define GL_SAMPLER_2D_ARRAY_SHADOW 0x8DC4
      #define GL_SAMPLER_CUBE_SHADOW 0x8DC5
      #define GL_UNSIGNED_INT_VEC2 0x8DC6
      #define GL_UNSIGNED_INT_VEC3 0x8DC7
      #define GL_UNSIGNED_INT_VEC4 0x8DC8
      #define GL_INT_SAMPLER_1D 0x8DC9
      #define GL_INT_SAMPLER_2D 0x8DCA
      #define GL_INT_SAMPLER_3D 0x8DCB
      #define GL_INT_SAMPLER_CUBE 0x8DCC
      #define GL_INT_SAMPLER_1D_ARRAY 0x8DCE
      #define GL_INT_SAMPLER_2D_ARRAY 0x8DCF
      #define GL_UNSIGNED_INT_SAMPLER_1D 0x8DD1
      #define GL_UNSIGNED_INT_SAMPLER_2D 0x8DD2
      #define GL_UNSIGNED_INT_SAMPLER_3D 0x8DD3
      #define GL_UNSIGNED_INT_SAMPLER_CUBE 0x8DD4
      #define GL_UNSIGNED_INT_SAMPLER_1D_ARRAY 0x8DD6
      #define GL_UNSIGNED_INT_SAMPLER_2D_ARRAY 0x8DD7
      #define GL_QUERY_WAIT 0x8E13
      #define GL_QUERY_NO_WAIT 0x8E14
      #define GL_QUERY_BY_REGION_WAIT 0x8E15
      #define GL_QUERY_BY_REGION_NO_WAIT 0x8E16

      #define GL_RED_SNORM 0x8F90
      #define GL_RG_SNORM 0x8F91
      #define GL_RGB_SNORM 0x8F92
      #define GL_RGBA_SNORM 0x8F93
      #define GL_R8_SNORM 0x8F94
      #define GL_RG8_SNORM 0x8F95
      #define GL_RGB8_SNORM 0x8F96
      #define GL_RGBA8_SNORM 0x8F97
      #define GL_R16_SNORM 0x8F98
      #define GL_RG16_SNORM 0x8F99
      #define GL_RGB16_SNORM 0x8F9A
      #define GL_RGBA16_SNORM 0x8F9B
      #define GL_SIGNED_NORMALIZED 0x8F9C
      #define GL_ALPHA_SNORM 0x9010
      #define GL_LUMINANCE_SNORM 0x9011
      #define GL_LUMINANCE_ALPHA_SNORM 0x9012
      #define GL_INTENSITY_SNORM 0x9013
      #define GL_ALPHA8_SNORM 0x9014
      #define GL_LUMINANCE8_SNORM 0x9015
      #define GL_LUMINANCE8_ALPHA8_SNORM 0x9016
      #define GL_INTENSITY8_SNORM 0x9017
      #define GL_ALPHA16_SNORM 0x9018
      #define GL_LUMINANCE16_SNORM 0x9019
      #define GL_LUMINANCE16_ALPHA16_SNORM 0x901A
      #define GL_INTENSITY16_SNORM 0x901B

      #define GL_RGB10_A2UI 0x906F

   #endif

#elif defined USES_OPENGL_GLEW

	#define ia_glGetProcAddress(name) wglGetProcAddress(name)
   #define USES_GL_STRINGI
   #define USES_GL_TEX_STORAGE_2D

	#define GL_POINT_SPRITE_OES GL_POINT_SPRITE
	#define GL_FRAMEBUFFER_BINDING_OES GL_FRAMEBUFFER_BINDING
	#define GL_FRAMEBUFFER_OES GL_FRAMEBUFFER
	#define GL_COLOR_ATTACHMENT0_OES GL_COLOR_ATTACHMENT0
	#define GL_FRAMEBUFFER_COMPLETE_OES GL_FRAMEBUFFER_COMPLETE
	#define GL_RENDERBUFFER_OES GL_RENDERBUFFER
	#define GL_DEPTH_COMPONENT16_OES GL_DEPTH_COMPONENT16
	#define GL_STENCIL_INDEX8_OES GL_STENCIL_INDEX8
	#define GL_DEPTH_ATTACHMENT_OES GL_DEPTH_ATTACHMENT
	#define GL_STENCIL_ATTACHMENT_OES GL_STENCIL_ATTACHMENT

	#ifndef GLAPIENTRYP
		#define GLAPIENTRYP GLAPIENTRY *
	#endif

#endif
