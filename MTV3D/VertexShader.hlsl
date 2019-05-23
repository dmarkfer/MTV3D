struct VSOut {
	float4 pos: SV_Position;
	float4 color: Color;
};

cbuffer CBuf {
	matrix transform;
};

VSOut main(float3 pos: Position, float4 color: Color) {
	VSOut vs;
	vs.pos = mul(float4(pos, 1.f), transform);
	vs.color = color;
	return vs;
}
