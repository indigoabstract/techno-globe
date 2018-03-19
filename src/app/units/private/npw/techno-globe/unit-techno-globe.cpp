#include "stdafx.h"

#include "unit-techno-globe.hpp"

#ifdef UNIT_TECHNO_GLOBE

#include "media/res-ld/res-ld.hpp"
#include "com/ux/ux-com.hpp"
#include "com/ux/ux-camera.hpp"
#include "com/ux/ux-font.hpp"
#include "com/unit/input-ctrl.hpp"
#include "cpp-property.hpp"
#include "gfx.hpp"
#include "gfx-rt.hpp"
#include "gfx-camera.hpp"
#include "gfx-shader.hpp"
#include "gfx-quad-2d.hpp"
#include "gfx-tex.hpp"
#include "gfx-util.hpp"
#include "gfx-vxo.hpp"
#include "gfx-state.hpp"
#include "ext/gfx-surface.hpp"
#include "utils/free-camera.hpp"
#include "tlib/rng/rng.hpp"
#include <glm/glm.hpp>
#include <queue>

//#define USE_GLOBE_DOT_BORDER_MESHES
//#define BUILD_RESOURCES

#if defined BUILD_RESOURCES
#include "rapidcsv.hpp"
#endif


namespace techno_globe_ns
{
   float ANIM_SPEED = 120.f;
   float START_SIZE = 0.35f;
   float MIDDLE_SIZE = 0.35f;
   float END_SIZE = 0.35f;

   class hot_spot
   {
   public:
      hot_spot(float i_latitude, float i_longitude, glm::vec3 i_position, glm::vec3 i_normal)
      {
         latitude = i_latitude;
         longitude = i_longitude;
         position = i_position;
         normal = i_normal;
      }

      float latitude;
      float longitude;
      glm::vec3 position;
      glm::vec3 normal;
   };

   void get_hot_spot_data(float latitude, float longitude, float globe_radius, glm::vec3& position, glm::vec3& normal)
   {
      // offsets for latitude/longitude
      float lat = 90.f - latitude;
      float lng = 270 - longitude;
      float latitude_rad = glm::radians(lat);
      float longitude_rad = glm::radians(lng);
      float px = glm::sin(latitude_rad) * glm::cos(longitude_rad);
      float py = glm::cos(latitude_rad);
      float pz = glm::sin(latitude_rad) * glm::sin(longitude_rad);
      glm::vec3 unit_sphere_position = glm::vec3(px, py, pz);

      normal = glm::normalize(unit_sphere_position);
      position = normal * globe_radius;
   }

   std::shared_ptr<hot_spot> new_hot_spot(float latitude, float longitude, float globe_radius)
   {
      glm::vec3 position;
      glm::vec3 normal;

      get_hot_spot_data(latitude, longitude, globe_radius, position, normal);

      return std::make_shared<hot_spot>(latitude, longitude, position, normal);
   }

   std::shared_ptr<hot_spot> new_random_hot_spot(float globe_radius)
   {
      static RNG rng;
      float sign_lat = 1.f;
      float sign_long = 1.f;

      if (rng.percent(50))
      {
         sign_lat = -sign_lat;
      }

      if (rng.percent(50))
      {
         sign_long = -sign_long;
      }

      float latitude = sign_lat * rng.nextInt(90);
      float longitude = sign_long * rng.nextInt(180);

      return new_hot_spot(latitude, longitude, globe_radius);
   }

   glm::vec3 transform_point(const glm::vec3& point, const glm::mat4& transform)
   {
      glm::vec4 pos = transform * glm::vec4(point, 0.f);
      glm::vec3 pos_3(pos.x, pos.y, pos.z);

      return pos_3;
   }


   class hot_spot_link : public gfx_vxo
   {
   public:
      struct link_size
      {
         float pos;
         float size;
      };

      int step_count;
      std::shared_ptr<hot_spot> start_point;
      std::shared_ptr<hot_spot> end_point;
      glm::vec3 base;
      glm::vec3 base_ortho;
      std::vector<float> distances;
      std::vector<float> cumulated_distances;
      std::vector<glm::vec3> positions;
      std::vector<glm::vec3> vertices;
      std::vector<glm::vec2> tex_coord;
      std::vector<glm::vec2> vertex_index;
      std::vector<glm::vec3> segment_dir;
      std::vector<gfx_indices_type> indices;
      std::vector<link_size> lk_size;

      hot_spot_link() : gfx_vxo(vx_info("a_v3_position, a_v3_normal, a_v2_tex_coord, a_v2_tex_coord_1"))
      {
      }

      float get_total_distance()
      {
         return cumulated_distances[step_count - 1];
      }

      void gen_link(std::shared_ptr<hot_spot> i_start_point, std::shared_ptr<hot_spot> i_end_point, float i_globe_radius)
      {
         start_point = i_start_point;
         end_point = i_end_point;
         base = end_point->position - start_point->position;
         lk_size =
         {
            { 0.f, START_SIZE },
         { 0.45f, MIDDLE_SIZE },
         { 0.55f, MIDDLE_SIZE },
         { 1.f, END_SIZE },
         };

         float dist = glm::length(base);
         float ratio = 0.5f * dist / i_globe_radius;
         float xfact = 4;

         step_count = int(xfact * glm::floor((1 - ratio) * 15.f + 50.f * ratio));

         positions.resize(step_count);
         distances.resize(step_count);
         cumulated_distances.resize(step_count);
         tex_coord.resize(step_count * 2);
         segment_dir.resize(step_count * 2);

         base = glm::normalize(base);
         base_ortho = (end_point->normal + start_point->normal) * 0.5f;
         base_ortho = glm::normalize(base_ortho);

         glm::vec3 org = (end_point->position + start_point->position) * 0.5f;
         float r = dist * 0.5f;
         float start_angle = (1.5f - ratio) * 45.f;
         float r2 = r / glm::cos(glm::radians(start_angle));
         float off = r2 * glm::sin(glm::radians(start_angle));

         for (int k = 0; k < step_count; k++)
         {
            float p = k / (step_count - 1.f);
            float angle = glm::mix(glm::radians(start_angle), glm::radians(180.f - start_angle), p);
            float x = r2 * glm::cos(angle);
            float y = r2 * glm::sin(angle) - off;

            positions[k] = org + base * x + base_ortho * y;
         }

         for (int k = 1; k < step_count; k++)
         {
            distances[k] = glm::distance(positions[k], positions[k - 1]);
            cumulated_distances[k] = cumulated_distances[k - 1] + distances[k];
         }

         float total_distance = cumulated_distances[step_count - 1];

         for (int i = 0; i < step_count; i++)
         {
            float iv = cumulated_distances[i] / total_distance;
            int idx1 = 2 * i + 0;
            tex_coord[idx1] = glm::vec2(-0.5f, iv);
            segment_dir[idx1] = glm::vec3();

            int idx2 = 2 * i + 1;
            tex_coord[idx2] = glm::vec2(0.5f, iv);
            segment_dir[idx2] = glm::vec3();
         }

         int indices_size = step_count - 1;
         int index = 0;

         indices.resize(indices_size * 6);

         for (int k = 0; k < indices_size; k++)
         {
            int i4 = 2 * k;
            indices[index] = i4 + 0;
            index = index + 1;
            indices[index] = i4 + 3;
            index = index + 1;
            indices[index] = i4 + 1;
            index = index + 1;
            indices[index] = i4 + 3;
            index = index + 1;
            indices[index] = i4 + 0;
            index = index + 1;
            indices[index] = i4 + 2;
            index = index + 1;
         }
      }

      void calc_vertex_positions()
      {
         float total_distance = cumulated_distances[step_count - 1];

         vertices.resize(step_count * 2);
         vertex_index.resize(step_count * 2);
         segment_dir.resize(step_count * 2);

         for (int i = 0; i < step_count - 1; i++)
         {
            float vsize = get_size_from_index(i);
            auto tvect = positions[i];
            auto seg = tvect - positions[i + 1];
            seg = glm::normalize(seg);

            int idx1 = 2 * i + 0;
            vertices[idx1] = tvect;
            vertex_index[idx1] = glm::vec2(i, vsize);
            segment_dir[idx1] = seg;

            int idx2 = 2 * i + 1;
            vertices[idx2] = tvect;
            vertex_index[idx2] = glm::vec2(i, vsize);
            segment_dir[idx2] = seg;
         }

         int i = step_count - 1;
         float iv = cumulated_distances[i] / total_distance;
         float vsize = get_size_from_index(i);
         auto tvect = positions[i];
         auto seg = positions[i - 1] - tvect;
         seg = glm::normalize(seg);

         int idx1 = 2 * i + 0;
         vertices[idx1] = tvect;
         vertex_index[idx1] = glm::vec2(i, vsize);
         segment_dir[idx1] = seg;

         int idx2 = 2 * i + 1;
         vertices[idx2] = tvect;
         vertex_index[idx2] = glm::vec2(i, vsize);
         segment_dir[idx2] = seg;

         auto& r = *this;

         r[MP_BLENDING] = MV_ADD;
         r[MP_DEPTH_FUNCTION] = MV_LESS;
         r[MP_DEPTH_TEST] = true;
         r[MP_DEPTH_WRITE] = false;
         r[MP_SHADER_NAME] = "hot-spot-lines";
         r["u_s2d_tex"] = "line.png";
         //r["u_s2d_tex"] = "trail.png";
         r[MP_CULL_BACK] = false;
         r[MP_CULL_FRONT] = false;

         struct vx_fmt_p3f_n3f_t2f_t2f
         {
            glm::vec3 pos;
            glm::vec3 nrm;
            glm::vec2 tex;
            glm::vec2 tex_1;
         };

         int vert_count = vertices.size();
         std::vector<vx_fmt_p3f_n3f_t2f_t2f> tvertices_data(vert_count);

         for (int k = 0; k < vert_count; k++)
         {
            auto& rf = tvertices_data[k];

            rf.pos = vertices[k];
            rf.nrm = segment_dir[k];
            rf.tex = tex_coord[k];
            rf.tex_1 = vertex_index[k];
         }

         gfx_vxo_util::set_mesh_data(
            (const uint8*)tvertices_data.data(), sizeof(vx_fmt_p3f_n3f_t2f_t2f) * tvertices_data.size(),
            indices.data(), indices.size() * sizeof(gfx_indices_type), std::static_pointer_cast<gfx_vxo>(get_shared_ptr()));
      }

      void set_visible_vertices(int sv_idx, int ev_idx)
      {
         auto& sv = positions[sv_idx];
         auto& sseg_dir = segment_dir[2 * sv_idx];
         auto& ev = positions[ev_idx];
         auto& eseg_dir = segment_dir[2 * ev_idx];

         auto& r = *this;

         r["u_vertex_color"] = glm::vec4(0.5f, 0.5f, 1.f, 1.f);
         r["u_v4_start_vertex"] = glm::vec4(sv, sv_idx);
         r["u_v3_start_vertex_seg_dir"] = sseg_dir;
         r["u_v4_end_vertex"] = glm::vec4(ev, ev_idx);
         r["u_v3_end_vertex_seg_dir"] = eseg_dir;
      }

      void draw_link(std::shared_ptr<gfx_camera> camera, const glm::mat4& i_globe_tf)
      {
         float s = 0.05f;
         glm::vec3 bsize(s, s, s);
         auto src = camera->position() + glm::vec3(0.01f, 0, 0);

         for (int k = 0; k < step_count; k++)
         {
            auto pos = transform_point(positions[k], i_globe_tf);
            camera->draw_line(pos, src, gfx_color::colors::green.to_vec4(), 0.25f);
            camera->draw_box(pos, bsize, glm::quat(), gfx_color::colors::blue.to_vec4(), 0.25f);
         }
      }

      float get_size_from_index(int index)
      {
         float position = index / (step_count - 1.f);

         return get_size_from_position(position);
      }

      // 0 <= i_position <= 1
      float get_size_from_position(float i_position)
      {
         int size_length = lk_size.size();
         int start_idx = 0;
         int end_idx = 0;
         float size = 0;

         for (int i = 0; i < size_length; i++)
         {
            start_idx = i;
            end_idx = i + 1;

            if (end_idx > size_length)
            {
               end_idx = size_length;
            }

            auto& es = lk_size[start_idx];
            auto& ee = lk_size[end_idx];

            if (i_position <= ee.pos)
            {
               float e1 = es.size;
               float e2 = ee.size;
               float dif = ee.pos - es.pos;
               float iv = i_position - es.pos;

               if (dif > 0)
               {
                  iv = iv / dif;
               }

               size = e1 * (1.f - iv) + e2 * iv;
               break;
            }
         }

         return size;
      }
   };


   class hot_spot_chain
   {
   public:
      hot_spot_chain()
      {
         loop_count = 1;
         //loop_index = -1;
         loop_index = 1;
         l_show = false;
         l_makevisible = false;
      }

      uint32 last_update_time;
      float chain_distance;
      int last_link_idx;
      float current_position;
      std::vector<std::shared_ptr<hot_spot_link> > hot_spot_link_list;
      std::vector<float> cumulated_distances;
      bool l_makevisible;
      int loop_count;
      int loop_index;
      bool l_show;

      void add_link(std::shared_ptr<hot_spot_link> _link)
      {
         int sv_idx = 0;
         int ev_idx = _link->step_count - 1;

         _link->set_visible_vertices(sv_idx, ev_idx);
         _link->visible = false;

         hot_spot_link_list.resize(hot_spot_link_list.size() + 1);
         cumulated_distances.resize(hot_spot_link_list.size());

         hot_spot_link_list[hot_spot_link_list.size()] = _link;
         cumulated_distances[hot_spot_link_list.size() - 1] = cumulated_distances[hot_spot_link_list.size() - 2] + _link->get_total_distance();
         chain_distance = cumulated_distances[hot_spot_link_list.size() - 1];
      }

      void set_hot_spot_link_list(std::vector<std::shared_ptr<hot_spot_link> > i_hot_spot_link_list)
      {
         int hot_spot_link_list_length = i_hot_spot_link_list.size();

         last_update_time = pfm::time::get_time_millis();
         hot_spot_link_list = i_hot_spot_link_list;
         chain_distance = 0;
         last_link_idx = 0;
         current_position = 0;

         cumulated_distances.resize(hot_spot_link_list_length);
         cumulated_distances[0] = hot_spot_link_list[0]->get_total_distance();

         for (int k = 0; k < hot_spot_link_list_length; k++)
         {
            auto link = hot_spot_link_list[k];
            int sv_idx = 0;
            int ev_idx = link->step_count - 1;

            link->set_visible_vertices(sv_idx, ev_idx);
            link->visible = false;
         }

         for (int k = 1; k < hot_spot_link_list_length; k++)
         {
            cumulated_distances[k] = cumulated_distances[k - 1] + hot_spot_link_list[k]->get_total_distance();
         }

         chain_distance = cumulated_distances[hot_spot_link_list_length - 1];
         l_makevisible = true;
      }

      void show()
      {
         l_show = true;
         set_visible(true);
      }

      void hide()
      {
         l_show = false;
         set_visible(false);
      }

      void force_visible()
      {
         int hot_spot_link_list_length = hot_spot_link_list.size();

         for (int k = 0; k < hot_spot_link_list_length; k++)
         {
            auto lk = hot_spot_link_list[k];
            float delta_d;

            if (k > 0)
            {
               delta_d = cumulated_distances[k] - cumulated_distances[k - 1];
            }
            else
            {
               delta_d = cumulated_distances[k];
            }

            int ev_idx = get_end_vertex_idx(delta_d, k);
            hot_spot_link_list[k]->set_visible_vertices(0, ev_idx);
         }
      }

      void set_visible(bool visible)
      {
         int hot_spot_link_list_length = hot_spot_link_list.size();

         for (int k = 0; k < hot_spot_link_list_length; k++)
         {
            auto lk = hot_spot_link_list[k];

            lk->visible = visible;
         }

         l_makevisible = true;
      }

      void restart_animation_for_last(int _nb)
      {
         loop_index = 1;
         _nb = glm::clamp<int>(_nb, 0, hot_spot_link_list.size());
         last_link_idx = hot_spot_link_list.size() - _nb;
         current_position = cumulated_distances[hot_spot_link_list.size() - _nb];
         last_update_time = pfm::time::get_time_millis();
         l_makevisible = true;
      }

      void restart_animation()
      {
         loop_index = 1;
         rewind_animation();
      }

      void stop_animation()
      {
         loop_index = -1;
      }

      void rewind_animation()
      {
         int hot_spot_chain_list_length = hot_spot_link_list.size();
         auto link = hot_spot_link_list[0];

         current_position = 0;
         link->set_visible_vertices(0, 0);
         last_link_idx = 0;
         l_makevisible = true;
         last_update_time = pfm::time::get_time_millis();

         set_visible(false);
      }

      std::shared_ptr<hot_spot> get_last_hot_spot()
      {
         int idx = hot_spot_link_list.size() - 1;

         return hot_spot_link_list[idx]->start_point;
      }

      void update()
      {
         if (!l_show)
         {
            return;
         }

         if ((loop_index > 0) && (loop_count == -1 || loop_index <= loop_count))
         {
            int hot_spot_chain_list_length = hot_spot_link_list.size();
            auto link = hot_spot_link_list[last_link_idx];
            float delta_t = (pfm::time::get_time_millis() - last_update_time) / 1000.f;

            last_update_time = pfm::time::get_time_millis();
            current_position = current_position + ANIM_SPEED * delta_t;

            if (current_position > cumulated_distances[last_link_idx])
            {
               while (last_link_idx < hot_spot_chain_list_length && current_position > cumulated_distances[last_link_idx])
               {
                  last_link_idx = last_link_idx + 1;
               }

               if (last_link_idx >= hot_spot_chain_list_length)
               {
                  loop_index = loop_index + 1;

                  if (loop_count == -1 || loop_index <= loop_count)
                  {
                     // repeat animation
                     rewind_animation();
                  }
                  else
                  {
                     // end animation
                     for (int k = 0; k < hot_spot_chain_list_length; k++)
                     {
                        auto link = hot_spot_link_list[k];

                        link->set_visible_vertices(0, link->step_count - 1);
                     }
                  }
               }
               else
               {
                  float delta_d = cumulated_distances[last_link_idx] - current_position;
                  int ev_idx = get_end_vertex_idx(delta_d, last_link_idx);

                  link->set_visible_vertices(0, link->step_count - 1);
                  link = hot_spot_link_list[last_link_idx];
                  link->set_visible_vertices(0, ev_idx);
                  l_makevisible = true;
               }
            }
            else
            {
               float delta_d = current_position;

               if (last_link_idx > 0)
               {
                  delta_d = current_position - cumulated_distances[last_link_idx - 1];
               }

               int ev_idx = get_end_vertex_idx(delta_d, last_link_idx);

               link->set_visible_vertices(0, ev_idx);

               if (l_makevisible)
               {
                  l_makevisible = false;
                  link->visible = true;
               }
            }
         }
      }

      int get_end_vertex_idx(float position_on_link, int link_idx)
      {
         auto link = hot_spot_link_list[link_idx];
         float dist = link->get_total_distance();
         float pos = position_on_link / dist;
         int idx = int(glm::floor((link->step_count - 1) * pos));

         return idx;
      }

      void destroy()
      {
         for (size_t k = 0; k < hot_spot_link_list.size(); k++)
         {
            hot_spot_link_list[k]->detach();
         }

         hot_spot_link_list.clear();
      }
   };


   class hot_spot_connector
   {
   public:
      float globe_radius;
      std::vector<std::shared_ptr<hot_spot_chain> > hot_spot_chain_list;
      bool is_init;
      bool visible;
      std::shared_ptr<gfx_vxo> globe;

      hot_spot_connector(std::shared_ptr<gfx_vxo> i_globe, float i_globe_radius)
      {
         globe = i_globe;
         globe_radius = i_globe_radius;
         is_init = false;
         visible = true;
      }

      void destroy()
      {
         destroy_old_chain();
      }

      void update()
      {
         if (globe)
         {
            if (!is_init)
            {
               is_init = true;

               int hot_spot_chain_list_length = hot_spot_chain_list.size();

               for (int i = 0; i < hot_spot_chain_list_length; i++)
               {
                  auto chain = hot_spot_chain_list[i];
                  int hot_spot_chain_list_length = chain->hot_spot_link_list.size();

                  for (int j = 0; j < hot_spot_chain_list_length; j++)
                  {
                     auto link = chain->hot_spot_link_list[j];

                     link->calc_vertex_positions();
                     link->position = globe->position;
                  }
               }
            }

            int hot_spot_chain_list_length = hot_spot_chain_list.size();

            for (int i = 0; i < hot_spot_chain_list_length; i++)
            {
               auto chain = hot_spot_chain_list[i];
               int hot_spot_chain_list_length = chain->hot_spot_link_list.size();

               for (int j = 0; j < hot_spot_chain_list_length; j++)
               {
                  chain->hot_spot_link_list[j]->orientation = globe->orientation;
                  //chain->hot_spot_link_list[j]->draw_link(nullptr, globe->transform_mx);
               }

               chain->update();
            }
         }
      }

      void set_visible(bool isvisible)
      {
         visible = isvisible;

         for (size_t k = 0; k < hot_spot_chain_list.size(); k++)
         {
            auto chain = hot_spot_chain_list[k];
            chain->set_visible(visible);
         }
      }

      bool is_visible()
      {
         return visible;
      }

      void show()
      {
         for (size_t k = 0; k < hot_spot_chain_list.size(); k++)
         {
            auto chain = hot_spot_chain_list[k];
            chain->show();
         }
      }

      void hide()
      {
         for (size_t k = 0; k < hot_spot_chain_list.size(); k++)
         {
            auto chain = hot_spot_chain_list[k];
            chain->hide();
         }
      }

      void force_visible()
      {
         hot_spot_chain_list[0]->force_visible();
         set_visible(true);
      }

      void restart_animation_for_last(int _nb)
      {
         hot_spot_chain_list[0]->restart_animation_for_last(_nb);
         // show();
      }

      void restart_animation()
      {
         int hot_spot_chain_list_length = hot_spot_chain_list.size();

         for (int k = 0; k < hot_spot_chain_list_length; k++)
         {
            hot_spot_chain_list[k]->restart_animation();
         }

         //show();
      }

      void stop_animation()
      {
         int hot_spot_chain_list_length = hot_spot_chain_list.size();

         for (int k = 0; k < hot_spot_chain_list_length; k++)
         {
            hot_spot_chain_list[k]->stop_animation();
         }
      }

      void destroy_old_chain()
      {
         for (size_t k = 0; k < hot_spot_chain_list.size(); k++)
         {
            hot_spot_chain_list[k]->destroy();
         }

         hot_spot_chain_list.clear();
      }

      void set_hot_spots(std::vector<glm::vec2> globe_hot_spot_list)
      {
         std::vector<std::shared_ptr<hot_spot> > globe_hot_spot_data_list;

         for (size_t k = 0; k < globe_hot_spot_list.size(); k++)
         {
            glm::vec3 position;
            glm::vec3 normal;
            auto hs = globe_hot_spot_list[k];

            get_hot_spot_data(hs.x, hs.y, globe_radius, position, normal);

            auto hs_2 = std::make_shared<hot_spot>(hs.x, hs.y, position, normal);

            globe_hot_spot_data_list.push_back(hs_2);
         }

         is_init = false;
         destroy_old_chain();

         if (globe_hot_spot_list.empty())
         {
            RNG rng;
            int hot_spot_chain_list_length = 4 + rng.nextInt(4);
            bool default_hot_spot = false;
            auto scene = globe->get_scene();

            vprint("hot_spot_chain_list_length: [%d]\n", hot_spot_chain_list_length);

            for (int i = 0; i < hot_spot_chain_list_length; i++)
            {
               std::vector<std::shared_ptr<hot_spot> > hot_spot_list;
               int hot_spot_list_length = 6 + rng.nextInt(5);
               std::vector<std::shared_ptr<hot_spot_link> > hot_spot_link_list;

               hot_spot_chain_list.push_back(std::make_shared<hot_spot_chain>());
               hot_spot_chain_list[i]->loop_count = -1;

               for (int j = 0; j < hot_spot_list_length; j++)
               {
                  auto hs = new_random_hot_spot(globe_radius);

                  hot_spot_list.push_back(hs);
               }

               if (!default_hot_spot)
               {
                  default_hot_spot = true;
                  hot_spot_list_length = hot_spot_list_length + 1;
                  hot_spot_list.push_back(new_hot_spot(44.435278f, 26.102778f, globe_radius));
               }

               for (int j = 1; j < hot_spot_list_length; j++)
               {
                  auto start_point = hot_spot_list[j - 1];
                  auto end_point = hot_spot_list[j];
                  auto link = std::make_shared<hot_spot_link>();

                  scene->attach(link);
                  link->gen_link(end_point, start_point, globe_radius);
                  hot_spot_link_list.push_back(link);
               }

               vprint("hs idx[%d] hot_spot_list_length: [%d]\n", i, hot_spot_list_length);
               hot_spot_chain_list[i]->set_hot_spot_link_list(hot_spot_link_list);
            }
         }
         else
         {
            std::vector<std::shared_ptr<hot_spot> > hot_spot_list;
            int hot_spot_list_length = globe_hot_spot_list.size();
            std::vector<std::shared_ptr<hot_spot_link> > hot_spot_link_list;
            auto scene = globe->get_scene();
            auto hot_spot_chain_inst = std::make_shared<hot_spot_chain>();

            hot_spot_chain_inst->loop_count = -1;
            hot_spot_chain_list.push_back(hot_spot_chain_inst);

            for (int j = 0; j < hot_spot_list_length; j++)
            {
               auto point = globe_hot_spot_list[j];
               auto hs = new_hot_spot(point.x, point.y, globe_radius);

               hot_spot_list.push_back(hs);
            }

            int index = 0;

            for (int j = 1; j < hot_spot_list_length; j++)
            {
               auto start_point = hot_spot_list[index];
               auto end_point = hot_spot_list[j];
               auto link = std::make_shared<hot_spot_link>();

               scene->attach(link);
               link->gen_link(end_point, start_point, globe_radius);
               hot_spot_link_list.push_back(link);
               index = index + 1;
            }

            hot_spot_chain_list[0]->set_hot_spot_link_list(hot_spot_link_list);
         }
      }

      void add_hot_spot(glm::vec2 _hot_spot)
      {
         auto start_point = hot_spot_chain_list[0]->get_last_hot_spot();
         auto end_point = new_hot_spot(_hot_spot.x, _hot_spot.y, globe_radius);
         auto link = std::make_shared<hot_spot_link>();

         link->gen_link(end_point, start_point, globe_radius);
         hot_spot_chain_list[0]->add_link(link);
         is_init = false;
      }
   };


   const std::string RES_MAP_NAME = "res-file";
   const float TEX_SCALE = 0.25f;

   struct globe_dot_vx
   {
      glm::vec3 pos;
      glm::vec3 nrm;
      glm::vec2 tx;
   };


   struct globe_coord
   {
      float lat;
      float lng;
   };


   struct globe_map_dot
   {
      uint16 x;
      uint16 y;
   };


   // this class builds the compacted resources loaded at runtime from their original/master versions,
   // which are high resolution, but impractical to use at runtime
   class master_resource_builder
   {
   public:
      master_resource_builder(std::shared_ptr<ux_camera> i_ux_cam)
      {
         ux_cam = i_ux_cam;
      }

#if defined BUILD_RESOURCES

      void build_resources()
      {
         build_res_step_0();
         build_res_step_1();
         build_res_step_2();
         save_resources();
      }

      void build_res_step_0()
      {
         // borders and continental dots will be synthesized from this map
         globe_src_img = res_ld::inst()->load_image("continents.png");
         globe_tex_width = globe_src_img->width;
         globe_tex_height = globe_src_img->height;
      }

      void build_res_step_1()
      {
         rgba_32_fmt zero = { 0 };
         rgba_buffer = std::vector<rgba_32_fmt>(globe_tex_width * globe_tex_height, zero);
         int threshold = 10;

         for (int k = 0; k < globe_tex_width * globe_tex_height; k++)
         {
            uint8 r = *(globe_src_img->data + k);
            rgba_buffer[k].r = 0;
            rgba_buffer[k].g = (r > threshold) ? 255 : 0;
            rgba_buffer[k].b = 0;
            rgba_buffer[k].a = 0;
         }

         detect_land_borders(rgba_buffer, globe_tex_width, globe_tex_height);
         build_globe_dots(rgba_buffer, globe_tex_width, globe_tex_height);
         build_globe_borders(rgba_buffer, globe_tex_width, globe_tex_height);
      }

      void build_res_step_2()
      {
         for (int k = 0; k < globe_tex_width * globe_tex_height; k++)
         {
            rgba_buffer[k].g = 0;
         }

         int tex_width = globe_tex_width;
         int tex_height = globe_tex_height;
         //float TEX_SCALE = 1024.f / tex_width;
         gfx_tex_params prm;

         prm.wrap_s = prm.wrap_t = gfx_tex_params::e_twm_clamp_to_edge;
         prm.max_anisotropy = 0.f;
         //prm.min_filter = gfx_tex_params::e_tf_linear_mipmap_linear;
         //prm.mag_filter = gfx_tex_params::e_tf_linear;
         prm.min_filter = gfx_tex_params::e_tf_nearest;
         prm.mag_filter = gfx_tex_params::e_tf_nearest;
         prm.gen_mipmaps = false;

         {
            map_tex = gfx::tex::new_tex_2d(gfx_tex::gen_id(), tex_width, tex_height, &prm);
            map_quad = std::make_shared<gfx_quad_2d>();
            gfx_quad_2d& mq = *map_quad;

            mq.set_dimensions(1, 1);
            mq.set_scale(tex_width * TEX_SCALE, tex_height * TEX_SCALE);
            mq.set_translation(20, 20.f);
            //mq[MP_CULL_FRONT] = false;
            //mq[MP_CULL_BACK] = false;
            //mq[MP_DEPTH_TEST] = false;
            mq[MP_SHADER_NAME] = "basic_tex";
            mq["u_s2d_tex"] = map_tex->get_name();
            mq.camera_id_list.push_back(ux_cam->camera_id());
         }

         map_tex->update(0, (const char*)rgba_buffer.data());
      }

      void detect_land_borders(std::vector<rgba_32_fmt>& rgba_buffer, int width, int height)
      {
         std::vector<rgba_32_fmt>& b = rgba_buffer;

         for (int k = 1; k < height - 1; k++)
         {
            for (int l = 1; l < width - 1; l++)
            {
               uint32 tm = (k - 1) * width + l - 0;
               uint32 ml = (k - 0) * width + l - 1;
               uint32 mm = (k - 0) * width + l - 0;
               uint32 mr = (k - 0) * width + l + 1;
               uint32 bm = (k + 1) * width + l - 0;

               if (b[mm].g > 0)
               {
                  if (b[ml].g == 0 || b[mr].g == 0 || b[tm].g == 0 || b[bm].g == 0)
                  {
                     globe_map_dot d;

                     d.x = l;
                     d.y = k;
                     b[mm].r = 255;
                     //b[idx].g = 0;
                  }
               }
            }
         }

         //for (int k = 1; k < height - 1; k++)
         //{
         //   for (int l = 1; l < width - 1; l++)
         //   {
         //      uint32 tl = (k - 1) * width + l - 1;
         //      uint32 tm = (k - 1) * width + l - 0;
         //      uint32 tr = (k - 1) * width + l + 1;
         //      uint32 ml = (k - 0) * width + l - 1;
         //      uint32 mm = (k - 0) * width + l - 0;
         //      uint32 mr = (k - 0) * width + l + 1;
         //      uint32 bl = (k + 1) * width + l - 1;
         //      uint32 bm = (k + 1) * width + l - 0;
         //      uint32 br = (k + 1) * width + l + 1;
         //      uint32 sum = b[tl].g + b[tm].g + b[tr].g + b[ml].g + b[mm].g + b[mr].g + b[bl].g + b[bm].g + b[br].g;
         //      uint32 sum_div = sum / 9;

         //      if (sum_div > 0 && sum_div < 255)
         //      {
         //         int idx = k * width + l;

         //         b[idx].r = 255;
         //         //b[idx].g = 0;
         //      }
         //   }
         //}

         rgba_32_fmt yellow;
         yellow.r = 255;
         yellow.g = 255;
         yellow.b = 0;
         yellow.a = 255;

         for (int k = 0; k < width * height; k++)
         {
            if (rgba_buffer[k].rgba == yellow.rgba)
            {
               rgba_buffer[k].g = 0;
            }

            //rgba_buffer[k].g = 0;
            //if (rgba_buffer[k].r == 255)
            //{
            //}
         }
      }

      void build_globe_dots(std::vector<rgba_32_fmt>& rgba_buffer, int width, int height)
      {
         // dot_row_count must be greater then 1
         int dot_row_count = 227;// 129 * 2;
         int equator_dot_count = 439;// 259 * 2;
         double start_angle = 0;
         double stop_angle = 180;
         double unit_globe_radius = 1.f;
         double equator_circumference = 2. * glm::pi<double>() * unit_globe_radius;
         int point_count = 0;
         float tex_aspect_ratio = float(width) / height;

         for (int k = 0; k < dot_row_count; k++)
         {
            double p = (double)k / (dot_row_count - 1);
            double angle = glm::mix<double>(start_angle, stop_angle, p);
            double angle_rad = glm::radians<double>(angle);
            double small_circle_radius = unit_globe_radius * glm::sin(angle_rad);
            double small_circle_circumference = 2. * glm::pi<double>() * small_circle_radius;
            double circle_circumference_prop = small_circle_circumference / equator_circumference;
            int small_cicle_dot_count = int(circle_circumference_prop * equator_dot_count);
            small_cicle_dot_count = glm::max(small_cicle_dot_count, 1);
            int row_idx = int(p * (height - 1));
            int y_offset = row_idx * width;
            int x_start = width / (small_cicle_dot_count + 1);
            glm::vec3 globe_up(0, 1, 0);
            int h_size = 1;

            if (small_cicle_dot_count > 1)
            {
               h_size = small_cicle_dot_count - 1;
            }

#if defined USE_GLOBE_DOT_BORDER_MESHES

            float dot_half_width = float(2. * glm::pi<double>() * globe_radius / 2500.);
            float dot_half_height = dot_half_width;

#else

            float dot_half_width = 0.f;
            float dot_half_height = width / 2000.f;

            if (small_cicle_dot_count > 1)
            {
               dot_half_width = float((dot_half_height) / circle_circumference_prop);
            }
            else
            {
               dot_half_width = width / 2.f;
            }

#endif

            for (int l = 0; l < small_cicle_dot_count; l++)
            {
               int x_offset = int(l / double(h_size) * width);
               int idx = y_offset + x_offset + x_start;
               rgba_32_fmt& rgba = rgba_buffer[idx];

               //if (rgba.r > 0 || rgba.g > 0)
               if (rgba.g > 0)
               {
                  rgba.b = 255;
                  point_count++;

                  globe_dot_vx dot;
                  uint16 px = uint16(x_offset + x_start);
                  uint16 py = uint16((height - 1.f) * (1.f - p));
                  globe_map_dot md = { px, py };

                  globe_dots.push_back(md);

#if defined USE_GLOBE_DOT_BORDER_MESHES

                  float latitude = float(angle + 90.f);
                  float longitude = glm::degrees(float(2. * glm::pi<double>() * double(x_offset + x_start) / width));

                  get_hot_spot_data(latitude, longitude, (float)globe_radius, dot.pos, dot.nrm);
                  glm::vec3 pos = dot.pos;
                  glm::vec3 left = glm::normalize(glm::cross(dot.nrm, globe_up));
                  glm::vec3 up = glm::normalize(glm::cross(left, dot.nrm));

#else

                  glm::vec3 pos(px, py, 0.f);
                  glm::vec3 left(-1, 0, 0);
                  glm::vec3 up(0, 1, 0);

#endif

                  dot.pos = pos - left * dot_half_width + up * dot_half_height;
                  globe_dots_vertices.push_back(dot);
                  dot.pos = pos - left * dot_half_width - up * dot_half_height;
                  globe_dots_vertices.push_back(dot);
                  dot.pos = pos + left * dot_half_width - up * dot_half_height;
                  globe_dots_vertices.push_back(dot);
                  dot.pos = pos + left * dot_half_width + up * dot_half_height;
                  globe_dots_vertices.push_back(dot);
               }
            }
         }

         int indices_size = globe_dots_vertices.size() / 4 * 6;

         globe_dots_indices.resize(indices_size);

         for (int k = 0, vx_idx = 0; k < indices_size;)
         {
            globe_dots_indices[k++] = vx_idx + 0;
            globe_dots_indices[k++] = vx_idx + 2;
            globe_dots_indices[k++] = vx_idx + 1;
            globe_dots_indices[k++] = vx_idx + 2;
            globe_dots_indices[k++] = vx_idx + 0;
            globe_dots_indices[k++] = vx_idx + 3;
            vx_idx += 4;
         }

         int vx_size = sizeof(globe_dot_vx);

         //gfx_vxo_util::set_mesh_data((const uint8*)&globe_dots_vertices[0], globe_dots_vertices.size() * vx_size,
         //   &globe_dots_indices[0], globe_dots_indices.size() * sizeof(uint32), globe_dots_vxo);

         vprint("--- total dot points: %d total vertices: %d ---\n", point_count, point_count * 4);
      }

      void build_globe_borders(std::vector<rgba_32_fmt>& rgba_buffer, int width, int height)
      {
         glm::vec3 globe_up(0, 1, 0);
         int idx = 0;
         int vx_idx = 0;
         int point_count = 0;
         float tex_aspect_ratio = float(width) / height;
         double start_angle = 0;
         double stop_angle = 180;
         double unit_globe_radius = 1.f;
         double equator_circumference = 2. * glm::pi<double>() * unit_globe_radius;

         for (int k = 0; k < height; k++)
         {
#if defined USE_GLOBE_DOT_BORDER_MESHES

            float dot_half_width = float(2. * glm::pi<double>() * globe_radius / 6500.);
            float dot_half_height = dot_half_width;

#else

            double p = (double)k / (height - 1);
            double angle = glm::mix<double>(start_angle, stop_angle, p);
            double angle_rad = glm::radians<double>(angle);
            double small_circle_radius = unit_globe_radius * glm::sin(angle_rad);
            double small_circle_circumference = 2. * glm::pi<double>() * small_circle_radius;
            double circle_circumference_prop = small_circle_circumference / equator_circumference;
            float dot_half_width = width / 8200.f;
            float dot_half_height = width / 8200.f;

            if (k > 0 && k < height - 1)
            {
               dot_half_width = float((dot_half_height) / circle_circumference_prop);
            }
            else
            {
               dot_half_width = width / 2.f;
            }

#endif

            for (int l = 0; l < width; l++, idx++)
            {
               if (rgba_buffer[idx].r > 0)
               {
                  globe_dot_vx dot;
                  uint16 px = uint16(l);
                  uint16 py = uint16(height - 1 - k);
                  globe_map_dot md = { px, py };

                  border_dots.push_back(md);

#if defined USE_GLOBE_DOT_BORDER_MESHES

                  float angle = float(k / double(height - 1.) * 180.);
                  float latitude = float(angle + 90.f);
                  float longitude = glm::degrees(float(2. * glm::pi<double>() * l / double(width - 1.)));

                  get_hot_spot_data(latitude, longitude, (float)globe_radius, dot.pos, dot.nrm);
                  glm::vec3 pos = dot.pos;
                  glm::vec3 left = glm::normalize(glm::cross(dot.nrm, globe_up));
                  glm::vec3 up = glm::normalize(glm::cross(left, dot.nrm));
                  tex_aspect_ratio = 1.f;

#else

                  glm::vec3 pos(l, height - 1 - k, 0.f);
                  glm::vec3 left(-1, 0, 0);
                  glm::vec3 up(0, 1, 0);

#endif

                  dot.pos = pos - left * dot_half_width + up * dot_half_height;
                  globe_borders_vertices.push_back(dot);
                  dot.pos = pos - left * dot_half_width - up * dot_half_height;
                  globe_borders_vertices.push_back(dot);
                  dot.pos = pos + left * dot_half_width - up * dot_half_height;
                  globe_borders_vertices.push_back(dot);
                  dot.pos = pos + left * dot_half_width + up * dot_half_height;
                  globe_borders_vertices.push_back(dot);

                  globe_borders_indices.push_back(vx_idx + 0);
                  globe_borders_indices.push_back(vx_idx + 2);
                  globe_borders_indices.push_back(vx_idx + 1);
                  globe_borders_indices.push_back(vx_idx + 2);
                  globe_borders_indices.push_back(vx_idx + 0);
                  globe_borders_indices.push_back(vx_idx + 3);
                  vx_idx += 4;
                  point_count++;
               }
            }
         }

         int vx_size = sizeof(globe_dot_vx);

         //gfx_vxo_util::set_mesh_data((const uint8*)&globe_borders_vertices[0], globe_borders_vertices.size() * vx_size,
         //   &globe_borders_indices[0], globe_borders_indices.size() * sizeof(uint32), globe_borders_vxo);

         vprint("--- total red points: %d total vertices: %d ---\n", point_count, point_count * 4);
      }

      // counterpart is 'load_resources' in runtime_resource_builder
      void save_resources()
      {
         // fill latitude/longitude binary data
         auto csv_path = pfm::filesystem::get_path("simplemaps-worldcities-basic.csv");
         auto drtf_path = pfm::filesystem::get_path("dummy-runtime-file");
         auto res_path = mws_util::path::get_directory_from_path(drtf_path);
         std::string res_file_path = res_path + "/" + RES_MAP_NAME;
         res_file = pfm_file::get_inst(res_file_path);
         res_rw = rw_file_sequence::new_inst(res_file);

         rapidcsv::Document doc(csv_path);

         std::vector<std::string> city = doc.GetColumn<std::string>("city_ascii");
         std::vector<float> lat = doc.GetColumn<float>("lat");
         std::vector<float> lng = doc.GetColumn<float>("lng");
         int globe_coords_count = lat.size();


         // start writing
         uint32 globe_coords_pos = 4 * sizeof(uint32);
         uint32 border_dots_pos = 0;
         uint32 globe_dots_pos = 0;

         res_file->io.open("wb");

         res_rw->w.write_uint32(globe_coords_pos);
         res_rw->w.write_uint32(border_dots_pos);
         res_rw->w.write_uint32(globe_dots_pos);
         res_rw->w.write_uint32(globe_tex_width);

         // globe_coords
         res_rw->w.write_uint32(globe_coords_count);

         for (int k = 0; k < globe_coords_count; k++)
         {
            res_rw->w.write_real32(lat[k]);
            res_rw->w.write_real32(lng[k]);
         }

         // border_dots
         border_dots_pos = (uint32)res_rw->get_write_position();
         res_rw->seek(sizeof(uint32));
         res_rw->w.write_uint32(border_dots_pos);
         res_rw->seek(border_dots_pos);

         res_rw->w.write_uint32(border_dots.size());
         res_rw->w.write_uint16((uint16*)border_dots.data(), border_dots.size() * 2, 0);

         // globe_dots
         globe_dots_pos = (uint32)res_rw->get_write_position();
         res_rw->seek(2 * sizeof(uint32));
         res_rw->w.write_uint32(globe_dots_pos);
         res_rw->seek(globe_dots_pos);

         res_rw->w.write_uint32(globe_dots.size());
         res_rw->w.write_uint16((uint16*)globe_dots.data(), globe_dots.size() * 2, 0);

         res_rw = nullptr;
         res_file->io.close();
         // finished writing
      }

#endif

      std::shared_ptr<pfm_file> res_file;
      std::shared_ptr<rw_file_sequence> res_rw;
      std::vector<globe_coord> globe_coords;
      std::vector<globe_map_dot> border_dots;
      std::vector<globe_map_dot> globe_dots;

      std::shared_ptr<gfx_vxo> globe_dots_vxo;
      std::vector<globe_dot_vx> globe_dots_vertices;
      std::vector<uint32> globe_dots_indices;

      std::shared_ptr<gfx_vxo> globe_borders_vxo;
      std::vector<globe_dot_vx> globe_borders_vertices;
      std::vector<uint32> globe_borders_indices;

      int globe_tex_width;
      int globe_tex_height;
      std::shared_ptr<raw_img_data> globe_src_img;
      std::vector<rgba_32_fmt> rgba_buffer;
      std::shared_ptr<gfx_tex> map_tex;
      std::shared_ptr<gfx_quad_2d> map_quad;
      std::shared_ptr<ux_camera> ux_cam;
   };


   // this class will load the runtime resources from a file and build any objects using them (like the globe mesh/texture)
   class runtime_resource_builder
   {
   public:
      runtime_resource_builder(std::shared_ptr<master_resource_builder> i_mrb, std::shared_ptr<ux_camera> i_ux_cam)
      {
         mrb = i_mrb;
         ux_cam = i_ux_cam;
      }

      void build_runtime_objects()
      {
         if (mrb)
         {
            load_resources();
            build_gfx_objects();
            set_globe_borders_and_dots();
         }
         else
         {
            load_resources();
            build_gfx_objects();
            set_globe_borders_and_dots();
         }
      }

      // counterpart is 'save_resources' in master_resource_builder
      void load_resources()
      {
         res_file = pfm_file::get_inst(RES_MAP_NAME);
         res_rw = rw_file_sequence::new_inst(res_file);


         // start reading
         res_file->io.open("rb");
         uint32 globe_coords_pos = res_rw->r.read_uint32();
         uint32 border_dots_pos = res_rw->r.read_uint32();
         uint32 globe_dots_pos = res_rw->r.read_uint32();
         globe_tex_width = res_rw->r.read_uint32();
         globe_tex_height = globe_tex_width / 2;

         // globe_coords
         res_rw->seek(globe_coords_pos);
         uint32 globe_coords_count = res_rw->r.read_uint32();

         globe_coords.resize(globe_coords_count);
         res_rw->r.read_real32((float*)globe_coords.data(), globe_coords_count * 2, 0);

         // border_dots
         res_rw->seek(border_dots_pos);
         uint32 border_dots_count = res_rw->r.read_uint32();

         border_dots.resize(border_dots_count);
         res_rw->r.read_uint16((uint16*)border_dots.data(), border_dots_count * 2, 0);

         // globe_dots
         res_rw->seek(globe_dots_pos);
         uint32 globe_dots_count = res_rw->r.read_uint32();

         globe_dots.resize(globe_dots_count);
         res_rw->r.read_uint16((uint16*)globe_dots.data(), globe_dots_count * 2, 0);

         auto file_size = res_file->length();
         auto bytes_read = res_rw->get_total_bytes_read();

         ia_assert(file_size == bytes_read);

         res_rw = nullptr;
         res_file->io.close();
         // finished reading
      }

      void set_globe_borders_and_dots()
      {
#if !defined USE_GLOBE_DOT_BORDER_MESHES

         rt_inst = gfx::rt::new_rt();
         rt_inst->set_color_attachment(globe_tex);

         gfx::rt::set_current_render_target(rt_inst);

         shared_ptr<gfx_state> gl_st = gfx::get_gfx_state();
         decl_scgfxpl(plist)
         {
            { gl::COLOR_CLEAR_VALUE, 0.f, 1.f, 1.f, 0.025f }, { gl::CLEAR_MASK, gl::COLOR_BUFFER_BIT_GL | gl::DEPTH_BUFFER_BIT_GL }, {},
         };
         gl_st->set_state(plist);

         globe_dots_vxo->render_mesh(ux_cam);
         globe_borders_vxo->render_mesh(ux_cam);
         //ux_cam->draw_axes(glm::vec3(20, 20, 0), 500, 5);
         ux_cam->update_camera_state();

         gfx::rt::set_current_render_target(nullptr);

#endif
      }

      void build_gfx_objects()
      {
         globe_radius = 100.f;
         u_v3_light_dir = -glm::vec3(-1.f, 0.f, 0.5f);

         // globe dots
         {
            globe_dots_vxo = std::make_shared<gfx_vxo>(vx_info("a_v3_position, a_v3_normal, a_v2_tex_coord"));
            auto& dots_vxo = *globe_dots_vxo;

            dots_vxo[MP_SHADER_NAME] = "globe-dot";
            //dots_vxo["u_v3_light_dir"] = u_v3_light_dir;
            dots_vxo[MP_CULL_FRONT] = false;
            dots_vxo[MP_CULL_BACK] = false;
            dots_vxo[MP_DEPTH_WRITE] = false;
            dots_vxo[MP_DEPTH_TEST] = false;
            dots_vxo[MP_BLENDING] = MV_ALPHA;
            //dots_vxo.render_method = GLPT_POINTS;
         }

         // globe borders
         {
            globe_borders_vxo = std::make_shared<gfx_vxo>(vx_info("a_v3_position, a_v3_normal, a_v2_tex_coord"));
            auto& borders_vxo = *globe_borders_vxo;

            borders_vxo[MP_SHADER_NAME] = "globe-border";
            //dots_vxo["u_v3_light_dir"] = u_v3_light_dir;
            borders_vxo[MP_CULL_FRONT] = false;
            borders_vxo[MP_CULL_BACK] = false;
            borders_vxo[MP_DEPTH_WRITE] = false;
            borders_vxo[MP_DEPTH_TEST] = false;
            borders_vxo[MP_BLENDING] = MV_ALPHA;
            //dots_vxo.render_method = GLPT_POINTS;
         }

         // globe vertex object
         {
            globe_vxo = std::make_shared<gfx_vpc_ring_sphere>();
            auto& globe = *globe_vxo;

            globe.get_vx_info().uses_tangent_basis = false;
            //globe.render_method = GLPT_LINES;
            globe.set_dimensions(globe_radius, 128);
            globe.position = glm::vec3(0.f, 0.f, 0.f);
            globe[MP_SHADER_NAME] = "3d-globe-outline";
            globe["u_v3_light_dir"] = u_v3_light_dir;
            globe[MP_CULL_FRONT] = false;
            globe[MP_CULL_BACK] = false;
            globe[MP_DEPTH_WRITE] = true;
            globe[MP_DEPTH_TEST] = false;
            globe[MP_BLENDING] = MV_ALPHA;
            //globe[MP_WIREFRAME_MODE] = MV_WF_OVERLAY;
         }

         // globe texture
         {
            gfx_tex_params prm;
            auto& globe = *globe_vxo;

            prm.wrap_s = prm.wrap_t = gfx_tex_params::e_twm_clamp_to_edge;
            prm.max_anisotropy = 16.f;
            prm.min_filter = gfx_tex_params::e_tf_linear_mipmap_linear;
            prm.mag_filter = gfx_tex_params::e_tf_linear;
            prm.gen_mipmaps = true;

            globe_tex = gfx::tex::new_tex_2d(gfx_tex::gen_id(), globe_tex_width, globe_tex_height, &prm);
            globe["u_s2d_tex"] = globe_tex->get_name();
         }

         // globe tex quad
         {
            globe_tex_quad = std::make_shared<gfx_quad_2d>();
            gfx_quad_2d& mq = *globe_tex_quad;

            mq.set_dimensions(1, 1);
            mq.set_scale(globe_tex_width * TEX_SCALE, globe_tex_height * TEX_SCALE);
            mq.set_translation(20, 20.f);
            mq[MP_BLENDING] = MV_ALPHA;
            //mq[MP_CULL_BACK] = false;
            //mq[MP_DEPTH_TEST] = false;
            mq[MP_SHADER_NAME] = "basic_tex";
            mq["u_s2d_tex"] = globe_tex->get_name();
            mq.camera_id_list.push_back(ux_cam->camera_id());
         }

         build_globe_dots();
         build_globe_borders();
      }

      void build_globe_dots()
      {
         int globe_dots_count = globe_dots.size();
         globe_dot_vx dot;

         for (int k = 0; k < globe_dots_count; k++)
         {
#if defined USE_GLOBE_DOT_BORDER_MESHES

            float dot_half_width = float(2. * glm::pi<double>() * globe_radius / 2500.);
            float dot_half_height = dot_half_width;

#else

            float dot_half_width = float(2. * glm::pi<double>() * globe_radius / 400.);
            float dot_half_height = dot_half_width;

#endif

            globe_map_dot& md = globe_dots[k];

#if defined USE_GLOBE_DOT_BORDER_MESHES

            float latitude = float(angle + 90.f);
            float longitude = glm::degrees(float(2. * glm::pi<double>() * double(x_offset + x_start) / width));

            get_hot_spot_data(latitude, longitude, (float)globe_radius, dot.pos, dot.nrm);
            glm::vec3 pos = dot.pos;
            glm::vec3 left = glm::normalize(glm::cross(dot.nrm, globe_up));
            glm::vec3 up = glm::normalize(glm::cross(left, dot.nrm));

#else

            glm::vec3 pos(md.x, md.y, 0.f);
            glm::vec3 left(-1, 0, 0);
            glm::vec3 up(0, 1, 0);

#endif

            dot.pos = pos - left * dot_half_width + up * dot_half_height;
            globe_dots_vertices.push_back(dot);
            dot.pos = pos - left * dot_half_width - up * dot_half_height;
            globe_dots_vertices.push_back(dot);
            dot.pos = pos + left * dot_half_width - up * dot_half_height;
            globe_dots_vertices.push_back(dot);
            dot.pos = pos + left * dot_half_width + up * dot_half_height;
            globe_dots_vertices.push_back(dot);
         }

         int indices_size = globe_dots_vertices.size() / 4 * 6;

         globe_dots_indices.resize(indices_size);

         for (int k = 0, vx_idx = 0; k < indices_size;)
         {
            globe_dots_indices[k++] = vx_idx + 0;
            globe_dots_indices[k++] = vx_idx + 2;
            globe_dots_indices[k++] = vx_idx + 1;
            globe_dots_indices[k++] = vx_idx + 2;
            globe_dots_indices[k++] = vx_idx + 0;
            globe_dots_indices[k++] = vx_idx + 3;
            vx_idx += 4;
         }

         int vx_size = sizeof(globe_dot_vx);

         gfx_vxo_util::set_mesh_data((const uint8*)&globe_dots_vertices[0], globe_dots_vertices.size() * vx_size,
            &globe_dots_indices[0], globe_dots_indices.size() * sizeof(uint32), globe_dots_vxo);
      }

      void build_globe_borders()
      {
         int border_dots_count = border_dots.size();
         glm::vec3 globe_up(0, 1, 0);
         globe_dot_vx dot;

         for (int k = 0, vx_idx = 0; k < border_dots_count; k++)
         {
#if defined USE_GLOBE_DOT_BORDER_MESHES

            float dot_half_width = float(2. * glm::pi<double>() * globe_radius / 6500.);
            float dot_half_height = dot_half_width;

#else

            float dot_half_width = float(2. * glm::pi<double>() * globe_radius / 1000.);
            float dot_half_height = dot_half_width;

#endif

            globe_map_dot& md = border_dots[k];

#if defined USE_GLOBE_DOT_BORDER_MESHES

            float angle = float(k / double(height - 1.) * 180.);
            float latitude = float(angle + 90.f);
            float longitude = glm::degrees(float(2. * glm::pi<double>() * l / double(width - 1.)));

            get_hot_spot_data(latitude, longitude, (float)globe_radius, dot.pos, dot.nrm);
            glm::vec3 pos = dot.pos;
            glm::vec3 left = glm::normalize(glm::cross(dot.nrm, globe_up));
            glm::vec3 up = glm::normalize(glm::cross(left, dot.nrm));
            tex_aspect_ratio = 1.f;

#else

            glm::vec3 pos(md.x, md.y, 0.f);
            glm::vec3 left(-1, 0, 0);
            glm::vec3 up(0, 1, 0);

#endif

            dot.pos = pos - left * dot_half_width + up * dot_half_height;
            globe_borders_vertices.push_back(dot);
            dot.pos = pos - left * dot_half_width - up * dot_half_height;
            globe_borders_vertices.push_back(dot);
            dot.pos = pos + left * dot_half_width - up * dot_half_height;
            globe_borders_vertices.push_back(dot);
            dot.pos = pos + left * dot_half_width + up * dot_half_height;
            globe_borders_vertices.push_back(dot);

            globe_borders_indices.push_back(vx_idx + 0);
            globe_borders_indices.push_back(vx_idx + 2);
            globe_borders_indices.push_back(vx_idx + 1);
            globe_borders_indices.push_back(vx_idx + 2);
            globe_borders_indices.push_back(vx_idx + 0);
            globe_borders_indices.push_back(vx_idx + 3);
            vx_idx += 4;
         }

         int vx_size = sizeof(globe_dot_vx);

         gfx_vxo_util::set_mesh_data((const uint8*)&globe_borders_vertices[0], globe_borders_vertices.size() * vx_size,
            &globe_borders_indices[0], globe_borders_indices.size() * sizeof(uint32), globe_borders_vxo);
      }

      std::shared_ptr<pfm_file> res_file;
      std::shared_ptr<rw_file_sequence> res_rw;
      std::vector<globe_coord> globe_coords;
      std::vector<globe_map_dot> border_dots;
      std::vector<globe_map_dot> globe_dots;

      int globe_tex_width;
      int globe_tex_height;
      float globe_radius;
      glm::vec3 u_v3_light_dir;
      std::shared_ptr<gfx_tex> globe_tex;
      std::shared_ptr<gfx_vpc_ring_sphere> globe_vxo;
      std::shared_ptr<gfx_rt> rt_inst;
      std::shared_ptr<gfx_quad_2d> globe_tex_quad;

      std::shared_ptr<gfx_vxo> globe_dots_vxo;
      std::vector<globe_dot_vx> globe_dots_vertices;
      std::vector<uint32> globe_dots_indices;

      std::shared_ptr<gfx_vxo> globe_borders_vxo;
      std::vector<globe_dot_vx> globe_borders_vertices;
      std::vector<uint32> globe_borders_indices;

      std::shared_ptr<master_resource_builder> mrb;
      std::shared_ptr<ux_camera> ux_cam;
   };


   class main_page : public ux_page
   {
   public:
      main_page(shared_ptr<ux_page_tab> iparent) : ux_page(iparent) {}

      virtual void init()
      {
         ux_page::init();

#if defined BUILD_RESOURCES

         mrb = std::make_shared<master_resource_builder>(get_unit()->ux_cam);
         mrb->build_resources();

#endif

         rrb = std::make_shared<runtime_resource_builder>(mrb, get_unit()->ux_cam);
         rrb->build_runtime_objects();

         t = 0;
         free_cam = std::make_shared<free_camera>(get_unit());
         persp_cam = gfx_camera::new_inst();
         persp_cam->camera_id = "default";
         persp_cam->rendering_priority = 0;
         persp_cam->near_clip_distance = 0.1f;
         persp_cam->far_clip_distance = 50000.f;
         persp_cam->fov_y_deg = 60.f;
         persp_cam->clear_color = true;
         persp_cam->clear_color_value = gfx_color::colors::black;
         persp_cam->clear_depth = true;
         persp_cam->position = glm::vec3(0.f, 0.f, 250.f);
         free_cam->persp_cam = persp_cam;
         free_cam->mw_speed_factor = 1.f;

         {
            skybox = std::make_shared<gfx_box>();
            gfx_box& skb = *skybox;
            float s = persp_cam->far_clip_distance * 0.5f;

            skb.set_dimensions(s, s, s);
            skb[MP_SHADER_NAME] = "sky-box";
            skb["u_scm_skybox"] = "skybx";
            skb[MP_CULL_BACK] = false;
            skb[MP_CULL_FRONT] = true;
         }

         auto scene = get_unit()->gfx_scene_inst;

         scene->attach(persp_cam);
         scene->attach(skybox);

#if defined USE_GLOBE_DOT_BORDER_MESHES

         scene->attach(globe_dots_vxo);
         scene->attach(globe_borders_vxo);

#endif

         scene->attach(rrb->globe_vxo);
         //globe_vxo->visible = false;

         if (mrb)
         {
            scene->attach(mrb->map_quad);
            scene->attach(rrb->globe_tex_quad);

            mrb->map_quad->visible = false;
            rrb->globe_tex_quad->visible = false;
         }
         scene->attach(rrb->globe_tex_quad);

         rrb->globe_tex_quad->visible = false;


         cam_end_pos = glm::vec3(-100.f, 0.f, -200.f) * 0.75f;
         cam_start_pos = cam_end_pos * 3.2f;
         intro_duration = 5;
         reset_cam_pos();

         hot_spot_connex = std::make_shared<hot_spot_connector>(rrb->globe_vxo, rrb->globe_radius);
         set_random_hot_spots();
         start_intro_anim();

         gfx_util::check_gfx_error();
      }

      virtual void receive(shared_ptr<iadp> idp)
      {
         free_cam->update_input(idp);

         if (!idp->is_processed())
         {
            if (idp->is_type(key_evt::KEYEVT_EVT_TYPE))
            {
               shared_ptr<key_evt> ke = key_evt::as_key_evt(idp);

               if (ke->is_pressed())
               {
                  bool isAction = true;

                  switch (ke->get_key())
                  {
                  case KEY_M:
                  {
                     if (mrb && mrb->map_quad)
                     {
                        mrb->map_quad->visible = !mrb->map_quad->visible;
                     }

                     break;
                  }

                  case KEY_N:
                  {
                     set_random_hot_spots();
                     break;
                  }

                  case KEY_G:
                  {
                     if (rrb->globe_tex_quad)
                     {
                        rrb->globe_tex_quad->visible = !rrb->globe_tex_quad->visible;
                     }

                     break;
                  }

                  case KEY_R:
                  {
                     hot_spot_connex->restart_animation();
                     break;
                  }

                  case KEY_T:
                  {
                     start_intro_anim();
                     break;
                  }

                  case KEY_Y:
                  {
                     reset_cam_pos();
                     break;
                  }

                  default:
                     isAction = false;
                  }

                  if (isAction)
                  {
                     ke->process();
                  }
               }
            }
         }

         ux_page::receive(idp);
      }

      virtual void update_state()
      {
         ux_page::update_state();

         rrb->globe_vxo->orientation = glm::quat(glm::vec3(0, t, 0));
         rrb->globe_dots_vxo->orientation = rrb->globe_vxo->orientation;
         rrb->globe_borders_vxo->orientation = rrb->globe_vxo->orientation;
         t += 0.001f;

         if (cam_slider.is_enabled())
         {
            cam_slider.update();

            double sv = cam_slider.get_value();
            double v = sv;
            double N = 20.f;
            double w = 1.f;

            v = 1.f - (1.f - v) * (1.f - v);
            v = 1.f - (1.f - v) * (1.f - v);
            v = ((v * (N - 1)) + w) / N;
            //v = glm::sqrt(v);
            persp_cam->position = glm::mix(cam_start_pos, cam_end_pos, v);
         }

         free_cam->update();
         skybox->position = persp_cam->position;
         hot_spot_connex->update();

         //persp_cam->draw_axes(globe_vxo->position, 5 * globe_radius, 1);

         gfx_util::check_gfx_error();
      }

      virtual void update_view(shared_ptr<ux_camera> g)
      {
         ux_page::update_view(g);
      }

      void set_random_hot_spots()
      {
         RNG rng;
         int hot_spot_chain_list_length = 4 + rng.nextInt(4);
         std::vector<glm::vec2> hs_list;
         int list_size = rrb->globe_coords.size();
         int elem_size = sizeof(double);

         for (int k = 0; k < hot_spot_chain_list_length; k++)
         {
            int elem_idx = rng.nextInt(list_size);
            globe_coord& coord = rrb->globe_coords[elem_idx];
            glm::vec2 lat_lng(coord.lat, coord.lng);

            hs_list.push_back(lat_lng);
         }

         hs_list =
         {
            glm::vec2(44.4333718, 26.09994665), glm::vec2(51.49999473, -0.116721844), glm::vec2(40.74997906,-73.98001693),
            glm::vec2(37.74000775,-122.4599777), glm::vec2(21.30687644,-157.8579979), glm::vec2(37.5663491,126.999731),
            glm::vec2(1.293033466,103.8558207), glm::vec2(30.04996035,31.24996822), glm::vec2(-26.17004474,28.03000972),
            glm::vec2(-22.92502317,-43.22502079), glm::vec2(-33.92001097,151.1851798)
         };
         hot_spot_connex->set_hot_spots(hs_list);
         hot_spot_connex->show();
         hot_spot_connex->restart_animation();
      }

      void start_intro_anim()
      {
         hot_spot_connex->restart_animation();
         cam_slider.start(intro_duration);
         reset_cam_pos();
      }

      void reset_cam_pos()
      {
         persp_cam->position = cam_start_pos;
         free_cam->look_at_dir = glm::normalize(-persp_cam->position());
         free_cam->up_dir = glm::vec3(0.0f, 1.0f, 0.0f);
         free_cam->target_ref_point = rrb->globe_vxo->position;
      }

      std::shared_ptr<gfx_box> skybox;

      std::shared_ptr<gfx_camera> persp_cam;
      float t;
      std::shared_ptr<free_camera> free_cam;
      glm::vec3 cam_start_pos;
      glm::vec3 cam_end_pos;
      float intro_duration;
      basic_time_slider cam_slider;
      std::shared_ptr<hot_spot_connector> hot_spot_connex;

      std::shared_ptr<master_resource_builder> mrb;
      std::shared_ptr<runtime_resource_builder> rrb;
   };
}
using namespace techno_globe_ns;


unit_techno_globe::unit_techno_globe()
{
   set_name("techno-globe");
}

shared_ptr<unit_techno_globe> unit_techno_globe::new_instance()
{
   return shared_ptr<unit_techno_globe>(new unit_techno_globe());
}

void unit_techno_globe::init_ux()
{
   ux_page::new_shared_instance(new main_page(uxroot));
}

#endif
