#ifdef GL_ES
	precision lowp float;
#endif
			
varying vec2 v_v2_tex_coord;
uniform sampler2D u_s2d_tex;

float rgb_2_u(vec3 c)
{
	float result = -(0.148 * c.r) - (0.291 * c.g) + (0.439 * c.b) + 0.5;
	
	return result; 
}

void main()
{
    vec3 v3_color = texture2D(u_s2d_tex, v_v2_tex_coord).rgb;
	
	gl_FragColor.r = rgb_2_u(v3_color);
}
