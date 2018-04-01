#pragma once

#include "pfm.hpp"
#include "gfx-material.hpp"
#include "gfx-scene.hpp"
#include "gfx-state.hpp"
#include "gfx-util.hpp"
#include <glm/glm.hpp>
#include <string>
#include <vector>
#include <unordered_map>

class gfx;
class gfx_camera;

namespace gfx_vxo_util
{
   void set_mesh_data(const uint8* tvertices_data, int tvertices_data_size, const gfx_indices_type* tindices_data, int tindices_data_size, std::shared_ptr<gfx_vxo> imesh);
}

enum gfx_primitive
{
   GLPT_POINTS,
   GLPT_LINES,
   GLPT_LINE_LOOP,
   GLPT_LINE_STRIP,
   GLPT_TRIANGLES,
   GLPT_TRIANGLE_STRIP,
   GLPT_TRIANGLE_FAN,
};


class vx_info
{
public:
   vx_info()
   {
      vertex_size = 0;
      aux_vertex_size = 0;
      has_tangent_basis = false;
      uses_tangent_basis = false;
   }

   vx_info(std::string ivx_attr_list)
   {
      bool a_pos = false;
      bool a_tex = false;
      bool a_nrm = false;

      vertex_size = 0;
      aux_vertex_size = 0;
      has_tangent_basis = false;
      uses_tangent_basis = false;
      vx_attr_vect = gfx_util::parse_attribute_list(ivx_attr_list);

      for (std::vector<std::shared_ptr<vx_attribute> >::iterator it = vx_attr_vect.begin(); it != vx_attr_vect.end(); it++)
      {
         const std::string& attr_name = (*it)->get_name();

         if (attr_name == "a_v3_position")
         {
            a_pos = true;
         }
         else if (attr_name == "a_v2_tex_coord")
         {
            a_tex = true;
         }
         else if (attr_name == "a_v3_normal")
         {
            a_nrm = true;
         }
      }

      if (a_pos && a_tex && a_nrm)
         // calc tangents and bitangents
      {
         has_tangent_basis = true;
         vx_aux_attr_vect = gfx_util::parse_attribute_list("a_v3_tangent, a_v3_bitangent");

         for (std::vector<std::shared_ptr<vx_attribute> >::iterator it = vx_aux_attr_vect.begin(); it != vx_aux_attr_vect.end(); it++)
         {
            aux_vertex_size += (*it)->get_aligned_size();
         }
      }

      for (std::vector<std::shared_ptr<vx_attribute> >::iterator it = vx_attr_vect.begin(); it != vx_attr_vect.end(); it++)
      {
         vertex_size += (*it)->get_aligned_size();
      }
   }

   bool has_tangent_basis;
   bool uses_tangent_basis;
   std::vector<std::shared_ptr<vx_attribute> > vx_attr_vect;
   std::vector<std::shared_ptr<vx_attribute> > vx_aux_attr_vect;
   int vertex_size;
   int aux_vertex_size;
};


struct vx_pos_coord_3f
{
   gfx_float x, y, z;

   void set(const glm::vec3& iv)
   {
      x = iv.x;
      y = iv.y;
      z = iv.z;
   }
};

struct vx_norm_coord_3f
{
   gfx_float nx, ny, nz;

   void set(const glm::vec3& iv)
   {
      nx = iv.x;
      ny = iv.y;
      nz = iv.z;
   }
};

struct vx_color_coord_4b
{
   gfx_ubyte r, g, b, a;
};

struct vx_tex_coord_2f
{
   gfx_float u, v;

   void set(float is, float it)
   {
      u = is;
      v = it;
   }
};

struct vx_fmt_p3f_t2f
{
   vx_pos_coord_3f pos;
   vx_tex_coord_2f tex;
};

struct vx_fmt_p3f_n3f_t2f
{
   vx_pos_coord_3f pos;
   vx_norm_coord_3f nrm;
   vx_tex_coord_2f tex;
};

struct vx_fmt_p3f_c4b_n3f_t2f
{
   vx_pos_coord_3f pos;
   vx_color_coord_4b clr;
   vx_norm_coord_3f nrm;
   vx_tex_coord_2f tex;
};


class gfx_vxo : public gfx_node
{
public:
   gfx_vxo(vx_info i_vxi, std::shared_ptr<gfx> i_gi = nullptr);
   gfx_vxo(vx_info i_vxi, bool i_is_submesh, std::shared_ptr<gfx> i_gi = nullptr);
   virtual ~gfx_vxo();
   virtual e_gfx_obj_type get_type()const override;
   virtual void set_mesh_name(const std::string& imesh_name);
   void operator=(const std::string& imesh_name);
   std::vector<uint8>& get_vx_buffer();
   std::vector<gfx_indices_type>& get_ix_buffer();
   virtual void set_data(const std::vector<uint8>& ivertices_buffer, const std::vector<gfx_indices_type>& iindices_buffer);
   void update_data();
   virtual gfx_material_entry& operator[](const std::string iname);
   std::shared_ptr<gfx_material> get_material();
   void set_material(std::shared_ptr<gfx_material> imaterial);
   vx_info& get_vx_info();
   virtual void add_to_draw_list(const std::string& icamera_id, std::vector<shared_ptr<gfx_vxo> >& idraw_list);
   virtual void render_mesh(std::shared_ptr<gfx_camera> icamera);
   void push_material_params();
   void set_size(int ivx_count, int iidx_count);

   std::vector<std::string> camera_id_list;
   gfx_primitive render_method;

protected:
   friend class gfx;
   friend class gfx_scene;
   friend class gfx_camera;

   virtual void render_mesh_impl(std::shared_ptr<gfx_camera> icamera);
   void compute_tangent_basis();

   vx_info vxi;
   std::string mesh_name;
   bool name_changed;
   bool setup_tangent_basis;
   std::shared_ptr<gfx_material> material;
   std::vector<uint8> vertices_buffer;
   std::vector<uint8> aux_vertices_buffer;
   std::vector<gfx_indices_type> indices_buffer;
   uint32 index_count;
   int vx_count;
   int idx_count;
   gfx_uint array_buffer_id;
   gfx_uint elem_buffer_id;
   bool buffer_changed;
   bool is_submesh;
   std::vector<gfx_param> plist;
   static gfx_uint method_type[];
   bool keep_geometry_data;
};
