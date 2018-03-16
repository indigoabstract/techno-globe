#ifdef GL_ES
	precision lowp float;
#endif

const float av = 0.0;
const vec4 u_v4_ambient_color = vec4(av, av, av, 1.0);
const vec4 u_v4_diffuse_color = vec4(1.0, 1.0, 1.0, 1.0);
const vec4 u_v4_specular_color = vec4(1.0, 1.0, 1.0, 1.0);
const float u_f_shininess = 55.0;

uniform mat4 u_m4_model;
uniform mat4 u_m4_model_view;
uniform mat4 u_m4_view_inv;
uniform sampler2D u_s2d_tex;
//uniform vec3 u_v3_light_dir;
vec3 u_v3_light_dir = -glm::vec3(0., 1.f, 0.);

varying vec4 v_v4_view_pos;
varying vec4 v_v4_view_norm;
varying vec3 v_v3_light_dir;
varying vec3 v_v3_pos_ws;
varying vec2 v_v2_tex_coord;
varying vec3 v_v3_normal;

void main()
{
	vec4 v4_color = texture2D(u_s2d_tex, v_v2_tex_coord);
	float v1_alpha = v4_color.a;
	vec3 v3_normal = normalize(v_v3_normal);
	float f_diffuse_fact = clamp(dot(-u_v3_light_dir, v3_normal), 0.0, 1.0);
	
	vec3 v3_diffuse_color = v4_color.rgb * 0.5 + v4_color.rgb * f_diffuse_fact;
	gl_FragColor = vec4(v3_diffuse_color, v1_alpha);
}
