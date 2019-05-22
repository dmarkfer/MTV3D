struct VSOut {
	float4 pos: SV_Position;
	float3 color: Color;
};

VSOut main(float2 pos: Position, float3 color: Color) {
	VSOut vs;
	vs.pos = float4(pos.x, pos.y, 0.f, 1.f);
	vs.color = color;
	return vs;
}
