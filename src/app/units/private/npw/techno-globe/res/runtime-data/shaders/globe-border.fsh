#ifdef GL_ES
	precision highp float;
#endif

uniform float u_v1_intensity;

const float v1_intensity_factor = 1. / 2.;

void main()
{
	float v1_intensity = 1. - v1_intensity_factor + u_v1_intensity * v1_intensity_factor;
	vec3 v3_color = vec3(0., 0.8, 1.);
	
	//v3_color *= v1_intensity;
	
	gl_FragColor = vec4(v3_color, v1_intensity);
}
