#pragma once

#include "pfm.hpp"
#include "gfx-camera.hpp"
#include "gfx-color.hpp"

class ux_font;
class ux_camera;
class ux_camera_impl;
class gfx_tex;


class ux_camera : public gfx_camera
{
public:
	static shared_ptr<ux_camera> new_inst(std::shared_ptr<gfx> i_gi = nullptr);
	void clearScreen(int iargb = 0);
	void clear(int ix, int iy, int iwidth, int iheight, int iargb = 0);
	void drawImage(shared_ptr<gfx_tex> img, float x, float y, float width = 0.f, float height = 0.f);
	void drawLine(float i, float j, float k, float l);
	void drawRect(float x, float y, float width, float height);
	void fillRect(float x, float y, float width, float height);
	shared_ptr<ux_font> get_font()const;
	void set_font(shared_ptr<ux_font> ifont);
	void drawText(const std::string& text, float x, float y, const shared_ptr<ux_font> ifnt = shared_ptr<ux_font>());
	void setColorf(float r, float g, float b, float a = 1.f);
	void setColor(uint8 r, uint8 g, uint8 b, uint8 a = 0xFF);
	void setColor(int argb);
	void push_transform_state();
	void pop_transform_state();
	void rotate(float angle);
	void scale(float sx, float sy);
	void translate(float tx, float ty);
	void fillAlphaRect(int x, int y, int w, int h, int color);
	//void fillAlphaGradientQuad(vxfmt_v2fc4b* iva);
   void update_camera_state() override;

protected:
	ux_camera(std::shared_ptr<gfx> i_gi = nullptr);
	virtual void load(shared_ptr<gfx_camera> inst);
	shared_ptr<ux_camera_impl> p;
};
