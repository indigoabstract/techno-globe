//@es #version 300 es
//@dt #version 330 core

#ifdef GL_ES
	precision lowp float;
#endif

layout(location=0) out vec4 v4_frag_color;

uniform sampler2D u_s2d_tex;

smooth in vec2 v_v2_tex_coord;

void main()
{
	vec4 v4_tex = texture(u_s2d_tex, v_v2_tex_coord);
	
	v4_frag_color = v4_tex;
}
