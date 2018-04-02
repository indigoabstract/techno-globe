uniform mat4 u_m4_model_view;
uniform mat4 u_m4_model_view_proj;

attribute vec3 a_v3_position;
attribute vec3 a_v3_normal;
attribute vec2 a_v2_tex_coord;

varying float v_v1_vx_id;

void main()
{
	v_v1_vx_id = a_v3_normal.x;
	
	gl_Position = u_m4_model_view_proj * vec4(a_v3_position, 1.0);
}
