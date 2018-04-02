uniform mat4 u_m4_model_view;
uniform mat4 u_m4_model_view_proj;
uniform mat4 u_m4_projection;

attribute vec3 a_v3_position;
attribute vec3 a_v3_normal;
attribute vec2 a_v2_tex_coord;

void main()
{
	float v1_spike_width = 0.35;
	float v1_spike_height = a_v2_tex_coord.y;
	vec3 v3_spike_direction_ws = a_v3_normal;
	vec3 v3_spike_direction_vs = normalize((u_m4_model_view * vec4(v3_spike_direction_ws, 0.0)).xyz);
	vec3 v3_position_ws = a_v3_position + v3_spike_direction_ws * v1_spike_height;
	vec3 v3_position_vs = (u_m4_model_view * vec4(v3_position_ws, 1.0)).xyz;
	// rotate the vector by 90 degrees. we only need the x and y components
	vec2 v2_spike_direction_perp_vs = normalize(vec2(-v3_spike_direction_vs.y, v3_spike_direction_vs.x));
	
	// offset the 2 ends of the quad along the perpendicular to the line direction (in viewspace)
	v3_position_vs.xy += v2_spike_direction_perp_vs * v1_spike_width * a_v2_tex_coord.x;
	
	gl_Position = u_m4_projection * vec4(v3_position_vs, 1.0);
}
