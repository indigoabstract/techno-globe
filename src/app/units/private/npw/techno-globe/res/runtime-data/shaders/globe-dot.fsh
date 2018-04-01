#ifdef GL_ES
	precision highp float;
#else
	#define highp
#endif

uniform sampler2D u_s2d_tex;
uniform float u_v1_time;

varying vec2 v_v2_tex_coord;
varying vec2 v_v2_global_tex_coord;

void main()
{
	vec2 v2_new_global_tex_coord = v_v2_global_tex_coord;
	
	v2_new_global_tex_coord += vec2(u_v1_time * 0.00051);
	v2_new_global_tex_coord *= 7.7351;
	float v1_r = texture2D(u_s2d_tex, v2_new_global_tex_coord).r;
	//float rn = rnd(v_v2_tex_coord.xy);
	//rn = clamp(rn, 0., 1.);
	v1_r += 0.45;
	v1_r = pow(v1_r, 15.);
	vec3 v3_color = vec3(v1_r * 4.5);
	v3_color = v3_color * vec3(0., 0.7, 1.);
	
	gl_FragColor = vec4(v3_color, 0.8);
}
