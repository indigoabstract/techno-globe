#ifdef GL_ES
	precision lowp float;
#else
	#define highp
#endif

uniform sampler2D u_s2d_tex;

varying vec2 v_v2_tex_coord;

highp float rnd(vec2 co)
{
    highp float a = 12.9898;
    highp float b = 78.233;
    highp float c = 43758.5453;
    highp float dt= dot(co.xy ,vec2(a,b));
    highp float sn= mod(dt,3.14);
    return fract(sin(sn) * c);
}

void main()
{
	float v1_r = texture2D(u_s2d_tex, v_v2_tex_coord);
	float rn = rnd(gl_FragCoord.xy);
	rn = clamp(rn, 0., 1.);
	vec3 v3_color = vec3(v1_r);
	v3_color = v3_color;
	gl_FragColor = vec4(v3_color, 1);
}
