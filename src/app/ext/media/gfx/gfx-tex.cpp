#include "stdafx.h"

#include "gfx-tex.hpp"
#include "gfx.hpp"
#include "min.hpp"
#include "gfx-util.hpp"
#include "gfx-shader.hpp"
#include "pfmgl.h"
#include "media/res-ld/res-ld.hpp"


gfx_tex_params::gfx_tex_params()
{
   internal_format = GL_RGBA8;
   format = GL_RGBA;
   type = GL_UNSIGNED_BYTE;
   min_filter = e_tf_linear_mipmap_linear;
   mag_filter = e_tf_linear;
   wrap_r = e_twm_clamp_to_edge;
   wrap_s = e_twm_repeat;
   wrap_t = e_twm_repeat;
   max_anisotropy = 16.f;
   gen_mipmaps = true;
   regen_mipmaps = false;
}

gfx_int gfx_tex_params::gl_mag_filter()
{
   switch (mag_filter)
   {
   case e_tf_nearest:
      return GL_NEAREST;

   case e_tf_linear:
      return GL_LINEAR;

   default:
      return GL_NEAREST;
   }
}

gfx_int gfx_tex_params::gl_min_filter()
{
   switch (min_filter)
   {
   case e_tf_nearest:
      return GL_NEAREST;

   case e_tf_linear:
      return GL_LINEAR;

   case e_tf_nearest_mipmap_nearest:
      return GL_NEAREST_MIPMAP_NEAREST;

   case e_tf_linear_mipmap_nearest:
      return GL_LINEAR_MIPMAP_NEAREST;

   case e_tf_nearest_mipmap_linear:
      return GL_NEAREST_MIPMAP_LINEAR;

   case e_tf_linear_mipmap_linear:
      return GL_LINEAR_MIPMAP_LINEAR;

   default:
      return GL_NEAREST;
   }
}

gfx_int gfx_tex_params::gl_wrap_r()
{
   switch (wrap_r)
   {
   case e_twm_repeat:
      return GL_REPEAT;

   case e_twm_clamp_to_edge:
      return GL_CLAMP_TO_EDGE;

   case e_twm_mirroed_repeat:
      return GL_MIRRORED_REPEAT;

   default:
      return GL_CLAMP_TO_EDGE;
   }
}

gfx_int gfx_tex_params::gl_wrap_s()
{
   switch (wrap_s)
   {
   case e_twm_repeat:
      return GL_REPEAT;

   case e_twm_clamp_to_edge:
      return GL_CLAMP_TO_EDGE;

   case e_twm_mirroed_repeat:
      return GL_MIRRORED_REPEAT;

   default:
      return GL_CLAMP_TO_EDGE;
   }
}

gfx_int gfx_tex_params::gl_wrap_t()
{
   switch (wrap_s)
   {
   case e_twm_repeat:
      return GL_REPEAT;

   case e_twm_clamp_to_edge:
      return GL_CLAMP_TO_EDGE;

   case e_twm_mirroed_repeat:
      return GL_MIRRORED_REPEAT;

   default:
      return GL_CLAMP_TO_EDGE;
   }
}

bool gfx_tex_params::anisotropy_enabled()
{
   return max_anisotropy > 0.f;
}


static int texture_name_idx = 0;

gfx_tex::~gfx_tex()
{
   release();
}

bool gfx_tex::mipmaps_supported(gfx_enum i_internal_format)
{
   switch (i_internal_format)
   {
   case GL_R32UI:
      return false;
   }

   return true;
}

std::string gfx_tex::gen_id()
{
   char name[256];

   uint32 time = pfm::time::get_time_millis();
   sprintf(name, "tex-%d-%d", texture_name_idx, time);
   texture_name_idx++;

   return name;
}

gfx_obj::e_gfx_obj_type gfx_tex::get_type()const
{
   return e_gfx_tex;
}

const gfx_tex_params& gfx_tex::get_params() const
{
   return prm;
}

bool gfx_tex::is_valid()const
{
   return is_valid_state;
}

bool gfx_tex::is_external_texture()
{
   check_valid_state();
   return is_external;
}

std::string gfx_tex::get_name()
{
   check_valid_state();
   return tex_name;
}

gfx_tex::gfx_tex_types gfx_tex::get_tex_type()
{
   check_valid_state();
   return uni_tex_type;
}

int gfx_tex::get_texture_gl_id()
{
   check_valid_state();
   return texture_gl_id;
}

void gfx_tex::set_texture_gl_id(int itexture_id)
{
   check_valid_state();

   if (!is_external)
   {
      throw ia_exception("only available for external textures!");
   }

   texture_gl_id = itexture_id;
}

int gfx_tex::get_width()
{
   check_valid_state();
   return width;
}

int gfx_tex::get_height()
{
   check_valid_state();
   return height;
}

void gfx_tex::send_uniform(const std::string iuniform_name, int iactive_tex_index)
{
   check_valid_state();

   shared_ptr<gfx_shader> glp = gfx::shader::get_current_program();
   gfx_int param_location = glp->get_param_location(iuniform_name);

   if (param_location != -1)
   {
      gfx_util::check_gfx_error();
      glUniform1i(param_location, iactive_tex_index);
      gfx_util::check_gfx_error();
      set_active(iactive_tex_index);
      gfx_util::check_gfx_error();
   }
}

void gfx_tex::set_active(int itex_unit_index)
{
   check_valid_state();
   gfx_util::check_gfx_error();
   glActiveTexture(GL_TEXTURE0 + itex_unit_index);
   gfx_util::check_gfx_error();
   glBindTexture(gl_tex_target, texture_gl_id);

   if (texture_updated)
   {
      glTexParameteri(gl_tex_target, GL_TEXTURE_MIN_FILTER, prm.gl_min_filter());
      glTexParameteri(gl_tex_target, GL_TEXTURE_MAG_FILTER, prm.gl_mag_filter());
      glTexParameteri(gl_tex_target, GL_TEXTURE_WRAP_S, prm.gl_wrap_s());
      glTexParameteri(gl_tex_target, GL_TEXTURE_WRAP_T, prm.gl_wrap_t());

      if (prm.anisotropy_enabled())
      {
         glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, prm.max_anisotropy);
      }

      if (prm.gen_mipmaps && mipmaps_supported(prm.internal_format))
      {
         glGenerateMipmap(gl_tex_target);
      }

      texture_updated = false;
   }
   //if (prm.regen_mipmaps && (prm.gen_mipmaps && mipmaps_supported(prm.internal_format)))
   //{
   //   glGenerateMipmap(gl_tex_target);
   //}

   gfx_util::check_gfx_error();
}

void gfx_tex::update(int iactive_tex_index, const char* ibb)
{
   check_valid_state();
   set_active(iactive_tex_index);

   if (!is_external && uni_tex_type == TEX_2D)
   {
      glTexSubImage2D(gl_tex_target, 0, 0, 0, width, height, prm.format, prm.type, ibb);

      glTexParameteri(gl_tex_target, GL_TEXTURE_MIN_FILTER, prm.gl_min_filter());
      glTexParameteri(gl_tex_target, GL_TEXTURE_MAG_FILTER, prm.gl_mag_filter());
      glTexParameteri(gl_tex_target, GL_TEXTURE_WRAP_S, prm.gl_wrap_s());
      glTexParameteri(gl_tex_target, GL_TEXTURE_WRAP_T, prm.gl_wrap_t());

      if (prm.anisotropy_enabled())
      {
         glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, prm.max_anisotropy);
      }

      if (prm.gen_mipmaps && mipmaps_supported(prm.internal_format))
      {
         glGenerateMipmap(gl_tex_target);
      }
   }

   gfx_util::check_gfx_error();
}

void gfx_tex::reload()
{
   if (is_external)
   {
      set_texture_gl_id(0);
   }
   else
   {
      if (is_valid())
      {
         texture_gl_id = gen_texture_gl_id();
         // setActive(0);
         glBindTexture(gl_tex_target, texture_gl_id);

         if (uni_tex_type == TEX_2D)
         {
            int mipmap_count = (prm.gen_mipmaps && mipmaps_supported(prm.internal_format)) ? gfx_util::get_tex_2d_mipmap_count(width, height) : 1;

            glTexStorage2D(gl_tex_target, mipmap_count, prm.internal_format, width, height);
         }

         gfx_util::check_gfx_error();

         glTexParameteri(gl_tex_target, GL_TEXTURE_MIN_FILTER, prm.gl_min_filter());
         glTexParameteri(gl_tex_target, GL_TEXTURE_MAG_FILTER, prm.gl_mag_filter());
         glTexParameteri(gl_tex_target, GL_TEXTURE_WRAP_S, prm.gl_wrap_s());
         glTexParameteri(gl_tex_target, GL_TEXTURE_WRAP_T, prm.gl_wrap_t());

         if (prm.anisotropy_enabled())
         {
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, prm.max_anisotropy);
         }

         if (prm.gen_mipmaps && mipmaps_supported(prm.internal_format))
         {
            glGenerateMipmap(gl_tex_target);
         }

         gfx_util::check_gfx_error();
      }
   }
}

gfx_tex::gfx_tex(const gfx_tex_params* i_prm, std::shared_ptr<gfx> i_gi) : gfx_obj(i_gi)
{
   set_params(i_prm);
   is_external = false;
   texture_updated = false;
}

gfx_tex::gfx_tex(std::string itex_name, const gfx_tex_params* i_prm, std::shared_ptr<gfx> i_gi) : gfx_obj(i_gi)
{
   set_params(i_prm);
   is_external = false;
   uni_tex_type = TEX_2D;
   set_texture_name(itex_name);

   unsigned long iwith, iheight;
   shared_ptr<raw_img_data> rid = res_ld::inst()->load_image(tex_name);
   iwith = rid->width;
   iheight = rid->height;

   init_dimensions(iwith, iheight);

   switch (uni_tex_type)
   {
   case TEX_2D:
      gl_tex_target = GL_TEXTURE_2D;
      break;
   }

   texture_gl_id = gen_texture_gl_id();
   glBindTexture(gl_tex_target, texture_gl_id);

   if (uni_tex_type == TEX_2D)
   {
      int mipmap_count = (prm.gen_mipmaps && mipmaps_supported(prm.internal_format)) ? gfx_util::get_tex_2d_mipmap_count(width, height) : 1;

      glTexStorage2D(gl_tex_target, mipmap_count, prm.internal_format, width, height);
      glTexSubImage2D(gl_tex_target, 0, 0, 0, width, height, prm.format, prm.type, rid->data);
   }

   gfx_util::check_gfx_error();

   glTexParameteri(gl_tex_target, GL_TEXTURE_MIN_FILTER, prm.gl_min_filter());
   glTexParameteri(gl_tex_target, GL_TEXTURE_MAG_FILTER, prm.gl_mag_filter());
   glTexParameteri(gl_tex_target, GL_TEXTURE_WRAP_S, prm.gl_wrap_s());
   glTexParameteri(gl_tex_target, GL_TEXTURE_WRAP_T, prm.gl_wrap_t());

   if (prm.anisotropy_enabled())
   {
      glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, prm.max_anisotropy);
   }

   if (prm.gen_mipmaps && mipmaps_supported(prm.internal_format))
   {
      glGenerateMipmap(gl_tex_target);
   }

   gfx_util::check_gfx_error();
   is_valid_state = true;
   texture_updated = false;
}

gfx_tex::gfx_tex(std::string itex_name, int itexture_id, int iwith, int iheight, gfx_tex_types iuni_tex_type, const gfx_tex_params* i_prm, std::shared_ptr<gfx> i_gi) : gfx_obj(i_gi)
{
   set_params(i_prm);
   is_external = true;
   uni_tex_type = iuni_tex_type;
   set_texture_name(itex_name);

   switch (uni_tex_type)
   {
   case TEX_2D:
      gl_tex_target = GL_TEXTURE_2D;
      break;
   }

   texture_gl_id = itexture_id;
   init_dimensions(iwith, iheight);

   gfx_util::check_gfx_error();
   is_valid_state = true;
   texture_updated = false;
}

gfx_tex::gfx_tex(std::string itex_name, int iwith, int iheight, gfx_tex_types iuni_tex_type, const gfx_tex_params* i_prm, std::shared_ptr<gfx> i_gi) : gfx_obj(i_gi)
{
   set_params(i_prm);
   is_external = false;
   uni_tex_type = iuni_tex_type;
   set_texture_name(itex_name);
   init_dimensions(iwith, iheight);

   switch (uni_tex_type)
   {
   case TEX_2D:
      gl_tex_target = GL_TEXTURE_2D;
      break;
   }

   texture_gl_id = gen_texture_gl_id();
   // setActive(0);
   glBindTexture(gl_tex_target, texture_gl_id);

   if (uni_tex_type == TEX_2D)
   {
      int mipmap_count = (prm.gen_mipmaps && mipmaps_supported(prm.internal_format)) ? gfx_util::get_tex_2d_mipmap_count(width, height) : 1;

      glTexStorage2D(gl_tex_target, mipmap_count, prm.internal_format, width, height);
   }

   gfx_util::check_gfx_error();

   glTexParameteri(gl_tex_target, GL_TEXTURE_MIN_FILTER, prm.gl_min_filter());
   glTexParameteri(gl_tex_target, GL_TEXTURE_MAG_FILTER, prm.gl_mag_filter());
   glTexParameteri(gl_tex_target, GL_TEXTURE_WRAP_S, prm.gl_wrap_s());
   glTexParameteri(gl_tex_target, GL_TEXTURE_WRAP_T, prm.gl_wrap_t());

   if (prm.anisotropy_enabled())
   {
      glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, prm.max_anisotropy);
   }

   if (prm.gen_mipmaps && mipmaps_supported(prm.internal_format))
   {
      glGenerateMipmap(gl_tex_target);
   }

   gfx_util::check_gfx_error();
   is_valid_state = true;
   texture_updated = false;
}

void gfx_tex::set_texture_name(std::string itex_name)
{
   tex_name = itex_name;
}

void gfx_tex::init_dimensions(int iwidth, int iheight)
{
   if (iwidth <= 0 || iheight <= 0)
   {
      throw ia_exception("width and height must be > 0");
   }

   width = iwidth;
   height = iheight;
}

void gfx_tex::set_params(const gfx_tex_params* i_prm)
{
   if (i_prm)
   {
      prm = *i_prm;
   }
}

int gfx_tex::gen_texture_gl_id()
{
   unsigned int tex_id;

   glGenTextures(1, &tex_id);

   vprint("gfx-info gen_texture_gl_id [%d]\n", tex_id);
   gfx_util::check_gfx_error();

   return tex_id;
}

void gfx_tex::check_valid_state()
{
   if (!is_valid_state)
   {
      throw ia_exception("the texture is not in a valid state");
   }
}

void gfx_tex::release()
{
   if (is_valid_state)
   {
      is_valid_state = false;

      if (!is_external)
      {
         glDeleteTextures(1, &texture_gl_id);

         vprint("gfx-info del-tex GlTex2D.release tex-id[%d]\n", texture_gl_id);
      }

      gfx::remove_gfx_obj(this);
   }
}


gfx_tex_2d::~gfx_tex_2d()
{
   release();
}

gfx_tex_2d::gfx_tex_2d(std::string itex_name, const gfx_tex_params* i_prm, std::shared_ptr<gfx> i_gi) : gfx_tex(itex_name, i_prm, i_gi)
{
}


gfx_tex_3d::~gfx_tex_3d()
{
   release();
}

gfx_tex_3d::gfx_tex_3d(std::string itex_name, std::shared_ptr<gfx> i_gi) : gfx_tex(nullptr, i_gi)
{
}


gfx_tex_cube_map::~gfx_tex_cube_map()
{
   release();
}

gfx_tex_cube_map::gfx_tex_cube_map(std::string itex_name, std::shared_ptr<gfx> i_gi) : gfx_tex(nullptr, i_gi)
{
   uni_tex_type = TEX_CUBE_MAP;
   set_texture_name(itex_name);
   gl_tex_target = GL_TEXTURE_CUBE_MAP;
   texture_gl_id = gen_texture_gl_id();
   glActiveTexture(GL_TEXTURE0);
   glBindTexture(gl_tex_target, texture_gl_id);

   std::string ends[] = { "posx", "negx", "posy", "negy", "posz", "negz" };
   bool is_init = false;

   for (int k = 0; k < 6; k++)
   {
      std::string img_name = itex_name + "-" + ends[k] + ".png";
      std::shared_ptr<raw_img_data> rid = res_ld::inst()->load_image(img_name);

      if (!is_init)
      {
         int mipmap_count = mipmaps_supported(prm.internal_format) ? gfx_util::get_tex_2d_mipmap_count(rid->width, rid->height) : 1;

         is_init = true;
         init_dimensions(rid->width, rid->height);
         glTexStorage2D(GL_TEXTURE_CUBE_MAP, mipmap_count, prm.internal_format, width, height);
      }

      glTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + k, 0, 0, 0, width, height, prm.format, prm.type, rid->data);
   }

   gfx_util::check_gfx_error();

   if (mipmaps_supported(prm.internal_format))
   {
      glTexParameteri(gl_tex_target, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
      glTexParameteri(gl_tex_target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
   }
   else
   {
      glTexParameteri(gl_tex_target, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      glTexParameteri(gl_tex_target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
   }

   //glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
   //glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
   glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
   glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
   glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
   if (prm.anisotropy_enabled())glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, prm.max_anisotropy);
   if (prm.gen_mipmaps && mipmaps_supported(prm.internal_format))glGenerateMipmap(gl_tex_target);

   gfx_util::check_gfx_error();
   is_valid_state = true;
}

gfx_tex_cube_map::gfx_tex_cube_map(uint32 isize, std::shared_ptr<gfx> i_gi) : gfx_tex(nullptr, i_gi)
{
}
