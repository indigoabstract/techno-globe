#ifdef GL_ES
	precision lowp float;
#endif

const float av = 0.0;
const vec4 u_v4_ambient_color = vec4(av, av, av, 1.0);
const vec4 u_v4_diffuse_color = vec4(1.0, 1.0, 1.0, 1.0);
const vec4 u_v4_specular_color = vec4(1.0, 1.0, 1.0, 1.0);
const float u_f_shininess = 55.0;

uniform sampler2D u_s2d_tex;
//uniform vec3 u_v3_light_dir;
vec3 u_v3_light_dir = -vec3(0., 1., 0.);

varying vec4 v_v4_view_pos;
varying vec4 v_v4_view_norm;
varying vec3 v_v3_light_dir;
varying vec3 v_v3_pos_ws;
varying vec2 v_v2_tex_coord;
varying vec3 v_v3_normal;

void main()
{
	float v1_r = texture2D(u_s2d_tex, v_v2_tex_coord * 0.321);
	float fogDistance = gl_FragCoord.z / gl_FragCoord.w;
	float attenuation = 0.015;
	float fogAmount = exp2(-fogDistance * attenuation);
	const vec3 fogColor = vec3(.0); // white

	vec4 v4_color = vec4(0., 0.4, 1., 0.05);
	float v1_alpha = v4_color.a;
	vec3 v3_normal = normalize(v_v3_normal);
	float f_diffuse_fact = clamp(dot(-u_v3_light_dir, v3_normal), 0.0, 1.0);

	vec3 v3_diffuse_color = v4_color.rgb * 0.5 + v4_color.rgb * f_diffuse_fact;
	//v3_diffuse_color = mix(v3_diffuse_color, fogColor, 1.-fogAmount);
	//v1_alpha = mix(v1_alpha, 0., 1.-fogAmount);
	float v1_cap = ((1. - v_v2_tex_coord.y) - 0.5)*1.5;
	v1_alpha += v1_cap*v1_cap*v1_cap;
	v3_diffuse_color += vec3(0., 0., v1_cap);
	gl_FragColor = vec4(v3_diffuse_color, v1_alpha);
}
