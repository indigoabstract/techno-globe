attribute vec3 a_v3_position;
attribute vec3 a_v3_normal;
attribute vec2 a_v2_tex_coord;

uniform mat4 u_m4_model;
uniform mat4 u_m4_model_view;
uniform mat4 u_m4_model_view_proj;

varying vec3 v_v3_pos;
varying vec3 v_v3_norm;
varying vec4 v_v4_view_pos;
varying vec4 v_v4_view_norm;
varying vec2 v_v2_tex_coord;

void main()
{
	v_v3_pos = (u_m4_model * vec4(a_v3_position, 1.0)).xyz;
	v_v3_norm = normalize((u_m4_model * vec4(a_v3_normal, 0.0)).xyz);
	v_v4_view_pos = u_m4_model_view * vec4(a_v3_position, 1.0);
	v_v4_view_norm = normalize(u_m4_model_view * vec4(a_v3_normal, 0.0));
	v_v2_tex_coord = a_v2_tex_coord;
	
	gl_Position = u_m4_model_view_proj * vec4(a_v3_position, 1.0);
}
