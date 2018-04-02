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
	
	float z = gl_FragCoord.z / gl_FragCoord.w;
	z = 1. - (z * z) * 0.00001;
	gl_FragColor = vec4(v3_color * z, v1_intensity);
	// const float LOG2 = 1.442695;
	// float z = gl_FragCoord.z / gl_FragCoord.w;
	// const float fog_density = 0.005;
	// float fog_factor = exp2( -fog_density * fog_density * z * z * LOG2 );
	// fog_factor = clamp(fog_factor, 0.0, 1.0);

	// gl_FragColor = mix(vec4(0.), vec4(v3_color.rgb * 1.5, v1_intensity), fog_factor);
}
