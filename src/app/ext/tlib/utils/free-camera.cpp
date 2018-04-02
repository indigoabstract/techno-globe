#include "stdafx.h"

#include "free-camera.hpp"
#include "pfm.hpp"
#include "com/unit/input-ctrl.hpp"
#include "com/util/util.hpp"
#include "gfx-camera.hpp"
#include "unit.hpp"


free_camera::free_camera(std::shared_ptr<unit> i_u)
{
   u = i_u;
   look_at_dir = glm::vec3(0.f, 0.f, -1.f);
   up_dir = glm::vec3(0.f, 1.f, 0.f);
   speed = 0.f;
   mw_speed_factor = 25.f;
   inf_phi_deg = 1.f;
   sup_phi_deg = 179.f;
   phi_deg = 90.f;
   theta_deg = 210.f;
   ks = std::make_shared<kinetic_scrolling>();
   mov_type = e_roll_view_axis;
}

void free_camera::update_input(shared_ptr<iadp> idp)
{
   if (idp->is_processed())
   {
      return;
   }

   bool ctrl_held = u.lock()->key_ctrl->key_is_held(KEY_CONTROL);

   if (ctrl_held)
   {
      mov_type = e_translation;
   }
   else
   {
      mov_type = e_roll_view_axis;
   }

   if (idp->is_type(touch_sym_evt::TOUCHSYM_EVT_TYPE))
   {
      shared_ptr<touch_sym_evt> ts = touch_sym_evt::as_touch_sym_evt(idp);

      //vprint("tn %s\n", ts->get_type_name(ts->get_type()).c_str());
      switch (ts->get_type())
      {
      case touch_sym_evt::TS_PRESSED:
      {
         ks->grab(ts->crt_state.te->points[0].x, ts->crt_state.te->points[0].y);
         ts->process();
         break;
      }

      case touch_sym_evt::TS_RELEASED:
      {
         ts->process();
         break;
      }

      case touch_sym_evt::TS_PRESS_AND_DRAG:
      {
         // rotation about the camera's view/target axis.
         if (ctrl_held)
         {
            float dx = ts->crt_state.te->points[0].x - ts->prev_state.te->points[0].x;
            float dy = ts->crt_state.te->points[0].y - ts->prev_state.te->points[0].y;
            float dx_rad = glm::radians(dx / 2);
            float dy_rad = glm::radians(dy / 2);

            glm::vec3 right_dir = glm::cross(look_at_dir, up_dir);
            glm::quat rot_around_right_dir = glm::angleAxis(dy_rad, right_dir);
            look_at_dir = glm::normalize(look_at_dir * rot_around_right_dir);
            up_dir = glm::normalize(glm::cross(right_dir, look_at_dir));

            glm::quat rot_around_up_dir = glm::angleAxis(dx_rad, up_dir);
            look_at_dir = glm::normalize(look_at_dir * rot_around_up_dir);
            ts->process();
         }
         // translation movement.
         else
         {
            float dx = ts->crt_state.te->points[0].x - ts->prev_state.te->points[0].x;
            float dy = ts->crt_state.te->points[0].y - ts->prev_state.te->points[0].y;

            if (ts->is_finished)
            {
               uint32 delta_t = ts->crt_state.te->time - ts->prev_state.te->time;

               if (delta_t < 150)
               {
                  ks->start_slowdown();
               }
               else
               {
                  ks->reset();
               }
            }
            else
            {
               ks->begin(ts->crt_state.te->points[0].x, ts->crt_state.te->points[0].y);
            }

            float camera_radius = glm::distance(persp_cam->position(), target_ref_point);

            theta_deg -= dx * camera_radius * 0.00045f;
            phi_deg -= dy * camera_radius * 0.00045f;
            clamp_angles();
            //vprint("tdx %f pdx %f\n", theta_deg, phi_deg);
            mov_type = e_roll_view_axis;
            ts->process();
         }
         break;
      }

      case touch_sym_evt::TS_MOUSE_WHEEL:
      {
         shared_ptr<mouse_wheel_evt> mw = static_pointer_cast<mouse_wheel_evt>(ts);

         persp_cam->position += look_at_dir * mw_speed_factor * float(mw->wheel_delta);
         ts->process();
         break;
      }
      }
   }
   else if (idp->is_type(key_evt::KEYEVT_EVT_TYPE))
   {
      shared_ptr<key_evt> ke = key_evt::as_key_evt(idp);

      if (!ke->is_released())
      {
         bool isAction = true;

         switch (ke->get_key())
         {
         case KEY_Q:
         {
            persp_cam->position -= look_at_dir * mw_speed_factor * 1.5f;
            break;
         }

         case KEY_E:
         {
            persp_cam->position += look_at_dir * mw_speed_factor * 1.5f;
            break;
         }

         case KEY_A:
         {
            glm::quat rot_around_look_at_dir = glm::angleAxis(glm::radians(+5.f), look_at_dir);
            up_dir = glm::normalize(up_dir * rot_around_look_at_dir);
            break;
         }

         case KEY_S:
         {
            speed = 0.f;
            break;
         }

         case KEY_D:
         {
            glm::quat rot_around_look_at_dir = glm::angleAxis(glm::radians(-5.f), look_at_dir);
            up_dir = glm::normalize(up_dir * rot_around_look_at_dir);
            break;
         }

         case KEY_Z:
         {
            speed -= 0.05f;
            break;
         }

         case KEY_C:
         {
            speed += 0.05f;
            break;
         }

         default:
            isAction = false;
         }

         //if (!isAction && !ke->is_repeated())
         //{
         //   isAction = true;

         //   switch (ke->get_key())
         //   {
         //   case KEY_SPACE:
         //   case KEY_F1:
         //      break;

         //   default:
         //      isAction = false;
         //   }
         //}

         if (isAction)
         {
            ke->process();
         }
      }
   }

}

void free_camera::update()
{

   point2d scroll_pos = ks->update();

   switch (mov_type)
   {
   case e_translation:
   {
      persp_cam->position += look_at_dir * speed;
      persp_cam->look_at(look_at_dir, up_dir);
      break;
   }

   case e_roll_own_axis:
   {
      break;
   }

   case e_roll_view_axis:
   {
      float camera_radius = glm::distance(persp_cam->position(), target_ref_point);

      //vprint("td0 %f pd0 %f", theta_deg, phi_deg);
      if (ks->is_active())
      {
         theta_deg -= scroll_pos.x * 0.1f;
         phi_deg -= scroll_pos.y * 0.1f;
         clamp_angles();
      }
      //vprint("td1 %f pd1 %f\n", theta_deg, phi_deg);

      float sin_phi = glm::sin(glm::radians(phi_deg));
      float cos_phi = glm::cos(glm::radians(phi_deg));
      float sin_theta = glm::sin(glm::radians(theta_deg));
      float cos_theta = glm::cos(glm::radians(theta_deg));
      glm::vec3 sphere_position(camera_radius * sin_theta * sin_phi, camera_radius * cos_phi, camera_radius * cos_theta * sin_phi);

      glm::vec3 view_pos = target_ref_point + sphere_position;
      persp_cam->position = view_pos;
      persp_cam->look_at_pos(target_ref_point, up_dir);
      look_at_dir = glm::normalize(target_ref_point - persp_cam->position());

      up_dir = glm::vec3(0, 1, 0);
      float dot = glm::abs(glm::dot(look_at_dir, up_dir));

      if (dot > 0.95)
      {
         int x = 3;
      }

      glm::vec3 right_dir = glm::normalize(glm::cross(look_at_dir, up_dir));
      up_dir = glm::normalize(glm::cross(right_dir, look_at_dir));
      persp_cam->look_at_pos(target_ref_point, up_dir);
      //vprint("td %f\n", theta_deg);
      break;
   }
   }
}

void free_camera::clamp_angles()
{
   if (phi_deg < inf_phi_deg)
   {
      phi_deg = inf_phi_deg;
   }
   else if (phi_deg > sup_phi_deg)
   {
      phi_deg = sup_phi_deg;
   }

   float mod_theta = 360.f;
   theta_deg = glm::mod(theta_deg, mod_theta);
}
