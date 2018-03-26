#ifdef GL_ES
	precision highp float;
#endif

uniform samplerCube u_scm_tex;	//cubemap texture sampler

//input from the vertex shader
varying vec3 uv;	//interpolated 3D texture coordinate

void main()
{
	//return the colour from the cubemap texture 
	gl_FragColor = textureCube(u_scm_tex, uv);
}