//@es #version 300 es
//@dt #version 330 core

layout(location=0) in vec3 a_v3_position;
layout(location=1) in vec3 a_v3_normal;
layout(location=2) in vec2 a_v2_tex_coord;

uniform mat4 u_m4_model_view_proj;
uniform mat4 u_m4_model;
uniform mat4 u_m4_model_view;
uniform mat4 u_m4_projection;
uniform float u_v1_face_camera;

smooth out vec2 v_v2_tex_coord;

void main()
{
	vec4 v3_pos_ws = u_m4_model * vec4(a_v3_position, 1.0);
	v_v2_tex_coord = a_v2_tex_coord;
	
	if(u_v1_face_camera == 1.)
	{
		vec3 v3_origin = u_m4_model[3].xyz;
		// switch to view space
		vec4 v4_pos_vs = u_m4_model_view * vec4(v3_origin, 1.0);
		
		// this makes a billboard perpendicular to the camera (facing the plane of the camera)
		//v4_pos_vs.xy += a_v3_position.xy;
		
		// this makes a billboard spherical (facing the camera orientation)
		vec3 v3_cam_up = vec3(0., 1., 0.);
		vec3 v3_x_axis = cross(v3_cam_up, -v4_pos_vs.xyz);
		v3_x_axis = normalize(v3_x_axis);
		vec3 v3_y_axis = cross(-v4_pos_vs.xyz, v3_x_axis);
		v3_y_axis = normalize(v3_y_axis);
		v4_pos_vs.xyz += v3_x_axis * v3_pos_ws.x;
		v4_pos_vs.xyz += v3_y_axis * v3_pos_ws.y;
		
		// write to position
		gl_Position = u_m4_projection * v4_pos_vs;
	}
	else
	{
		gl_Position = u_m4_model_view_proj * vec4(v3_pos_ws.xyz, 1.0);
	}
}
