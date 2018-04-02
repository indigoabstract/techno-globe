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
	vec2 v2_coord = abs(v_v2_tex_coord - vec2(0.5)) * 2.;
	float dist = length(v2_coord);
	float c = dist * 1.67 - 1.331;
	c = c * step(0., c);
	c = pow(c, 0.125);
	vec4 v4_tex = texture(u_s2d_tex, vec2(c));
	v4_tex.rgb *= vec3(0.1, 0.7, 1.) * 0.65;
	
	v4_frag_color = v4_tex;
	//v4_frag_color = vec4(0.);
}
