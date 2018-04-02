#ifdef GL_ES
	precision highp float;
#endif

uniform samplerCube u_scm_tex;	//cubemap texture sampler

//input from the vertex shader
varying vec3 uv;	//interpolated 3D texture coordinate

void main()
{
	//return the colour from the cubemap texture 
	vec4 v4_color = textureCube(u_scm_tex, uv);
	
	v4_color.rgb *= v4_color.b * 2.;
	//v4_color.rgb *= vec3(0.1, 0.7, 1.) * 1.5;
	gl_FragColor = v4_color;
}
