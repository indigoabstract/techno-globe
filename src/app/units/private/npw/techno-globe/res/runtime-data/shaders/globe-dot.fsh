#ifdef GL_ES
	precision highp float;
#else
	#define highp
#endif

uniform sampler2D u_s2d_tex;
uniform float u_v1_time;
uniform mat4 u_m4_view_inv;

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
	float z = gl_FragCoord.z / gl_FragCoord.w;
	z = 1. - (z * z) * 0.00001111111;
	v3_color = v3_color * z;
	gl_FragColor = vec4(v3_color, 1.);
	// const float LOG2 = 1.442695;
	// float z = gl_FragCoord.z / gl_FragCoord.w;
	// const float fog_density = 0.005;
	// float fog_factor = exp2( -fog_density * fog_density * z * z * LOG2 );
	// fog_factor = clamp(fog_factor, 0.0, 1.0);

	// gl_FragColor = mix(vec4(0.), vec4(v3_color * 1.5, 1.), fog_factor);
}
