#pragma hydra vert:MainVS pixel:MainPS

struct FullScreenQuadOutput
{
	float4 position     : SV_Position;
	float2 uv           : TEXCOORD;
};

FullScreenQuadOutput MainVS(uint id : SV_VertexID)
{
	FullScreenQuadOutput OUT;

	uint u = ~id & 1;
	uint v = (id >> 1) & 1;
	OUT.uv = float2(u, v);
	OUT.position = float4(OUT.uv.x * 2.0 - 1.0, OUT.uv.y * 2.0 - 1.0, 0.0, 1.0);

	return OUT;
}

Texture2D _Texture : register(t0);

cbuffer Constants : register(b0)
{
	float2 g_ViewPort;
	float4x4 g_InvProjection;
	float4x4 g_InvView;
	float3 g_ViewPos;
	float3 g_LightDir;
	float _Time;
};

float3 mod289(float3 x)
{
	return x - floor(x / 289.0) * 289.0;
}

float4 mod289(float4 x)
{
	return x - floor(x / 289.0) * 289.0;
}

float4 permute(float4 x)
{
	return mod289((x * 34.0 + 1.0) * x);
}

float4 taylorInvSqrt(float4 r)
{
	return 1.79284291400159 - r * 0.85373472095314;
}

float snoise(float3 v)
{
	const float2 C = float2(1.0 / 6.0, 1.0 / 3.0);

	// First corner
	float3 i = floor(v + dot(v, C.yyy));
	float3 x0 = v - i + dot(i, C.xxx);

	// Other corners
	float3 g = step(x0.yzx, x0.xyz);
	float3 l = 1.0 - g;
	float3 i1 = min(g.xyz, l.zxy);
	float3 i2 = max(g.xyz, l.zxy);

	// x1 = x0 - i1  + 1.0 * C.xxx;
	// x2 = x0 - i2  + 2.0 * C.xxx;
	// x3 = x0 - 1.0 + 3.0 * C.xxx;
	float3 x1 = x0 - i1 + C.xxx;
	float3 x2 = x0 - i2 + C.yyy;
	float3 x3 = x0 - 0.5;

	// Permutations
	i = mod289(i); // Avoid truncation effects in permutation
	float4 p =
		permute(permute(permute(i.z + float4(0.0, i1.z, i2.z, 1.0))
			+ i.y + float4(0.0, i1.y, i2.y, 1.0))
			+ i.x + float4(0.0, i1.x, i2.x, 1.0));

	// Gradients: 7x7 points over a square, mapped onto an octahedron.
	// The ring size 17*17 = 289 is close to a multiple of 49 (49*6 = 294)
	float4 j = p - 49.0 * floor(p / 49.0);  // mod(p,7*7)

	float4 x_ = floor(j / 7.0);
	float4 y_ = floor(j - 7.0 * x_);  // mod(j,N)

	float4 x = (x_ * 2.0 + 0.5) / 7.0 - 1.0;
	float4 y = (y_ * 2.0 + 0.5) / 7.0 - 1.0;

	float4 h = 1.0 - abs(x) - abs(y);

	float4 b0 = float4(x.xy, y.xy);
	float4 b1 = float4(x.zw, y.zw);

	//float4 s0 = float4(lessThan(b0, 0.0)) * 2.0 - 1.0;
	//float4 s1 = float4(lessThan(b1, 0.0)) * 2.0 - 1.0;
	float4 s0 = floor(b0) * 2.0 + 1.0;
	float4 s1 = floor(b1) * 2.0 + 1.0;
	float4 sh = -step(h, 0.0);

	float4 a0 = b0.xzyw + s0.xzyw * sh.xxyy;
	float4 a1 = b1.xzyw + s1.xzyw * sh.zzww;

	float3 g0 = float3(a0.xy, h.x);
	float3 g1 = float3(a0.zw, h.y);
	float3 g2 = float3(a1.xy, h.z);
	float3 g3 = float3(a1.zw, h.w);

	// Normalise gradients
	float4 norm = taylorInvSqrt(float4(dot(g0, g0), dot(g1, g1), dot(g2, g2), dot(g3, g3)));
	g0 *= norm.x;
	g1 *= norm.y;
	g2 *= norm.z;
	g3 *= norm.w;

	// Mix final noise value
	float4 m = max(0.6 - float4(dot(x0, x0), dot(x1, x1), dot(x2, x2), dot(x3, x3)), 0.0);
	m = m * m;
	m = m * m;

	float4 px = float4(dot(x0, g0), dot(x1, g1), dot(x2, g2), dot(x3, g3));
	return 42.0 * dot(m, px);
}

/*float4 Render(float3 position, float3 direction)
{
	float val = 0.0;

	float stepSize = 0.05;
	float maxDistance = 20;

	int stepCount = 400;
	int count = 0;

	for (int i = 0; i < stepCount; i++)
	{
		float3 pos = position + (direction * i * stepSize);

		if (pos.x > 0 && pos.y > 0 && pos.z > 0)
		{
			if (pos.x < 10 && pos.y < 10 && pos.z < 10)
			{
				float noiseVal = snoise(pos);

				val += noiseVal;

				count++;

				if (val >= 1.0) break;
			}
		}
	}

	return val.xxxx / count;
}*/

float fbm(float2 pos)
{
	return snoise(float3(pos.x, pos.y, 0.0) * 2);
}

float4 MainPS(FullScreenQuadOutput IN) : SV_Target
{
	float x = 2.0 * IN.position.x / g_ViewPort.x - 1.0;
	float y = 2.0 * IN.position.y / g_ViewPort.y - 1.0;

	// Flip y coordinate
	float a = (y + 1.0) * 0.5; // Remap values from [-1.0, 1.0] to [0.0, 1.0]
	a = 1.0 - a; // Inverse value
	a = (a - 0.5) * 2.0; // Remap back to original
	//

	float2 ray_nds = float2(x, a);
	float4 ray_clip = float4(ray_nds, -1.0, 1.0);
	float4 ray_view = mul(g_InvProjection, ray_clip);
	ray_view = float4(ray_view.xy, -1.0, 0.0);
	float3 ray_world = (mul(g_InvView, ray_view)).xyz;
	ray_world = normalize(ray_world);

	float4 cloud_color = (0.0).xxxx;
	float4 finalColor = cloud_color;

	float sundot = clamp(dot(ray_world, g_LightDir), 0.0, 1.0);
	float nightdot = clamp(dot(float3(0, -1, 0), g_LightDir), 0.0, 1.0);
	float3 blueSky = float3(0.3, .55, 0.8);
	float3 redSky = float3(0.8, 0.8, 0.6);
	//redSky = mix(redSky, blueSky, 1.0 - nightdot);
	float3 sky = lerp(blueSky, redSky, 1.5 * pow(sundot, 8.));

	sky = lerp(sky, (0.0).xxx, nightdot);

	finalColor.rgb = sky/* *(1.0 - 0.8 * ray_world.y)*/;

	// Stars in night
	float starsValue = snoise(ray_world * 100.0);
	if (starsValue > 0.8)
	{
		float3 backStars = starsValue.xxx;
		backStars = lerp(backStars, 0.0.xxx, 1.0 - nightdot);
		finalColor.rgb += backStars;
	}

	// Sun

	float3 sun = (0.0).xxx;

	sun += 0.1 * float3(0.9, 0.3, 0.9) * pow(sundot, 0.5);
	sun += 0.2 * float3(1.0, 0.7, 0.7) * pow(sundot, 1.0);
	sun += 0.95 * (1.0).xxx * pow(sundot, 256.0);

	finalColor.rgb += lerp(sun, (0.0).xxx, nightdot);

	// Clouds

	/*float cloudSpeed = 0.01;
	float cloudFlux = 0.5;

	float3 cloudColour = lerp(float3(1.0, 0.95, 1.0), 0.35 * redSky, pow(sundot, 2.0));
	cloudColour = lerp(cloudColour, cloudColour * 0.1, nightdot);

	if (ray_world.y > 0)
	{
		float3 ro = float3(g_ViewPos.x + _Time * 100, g_ViewPos.y - 100000, g_ViewPos.z + _Time * 100) / 100.0;

		for(int i = 0; i < 10; i++) {
			float2 sc = cloudSpeed * 50.0 * _Time * ro.xz + ray_world.xz * ((1000.0 + (i * 10.0)) - ro.y) / ray_world.y;
			finalColor.rgb = lerp( finalColor.rgb, cloudColour, 0.5 * smoothstep(0.5, 0.8, fbm(0.0005 * sc + fbm(0.0005 * sc + _Time * cloudFlux))));
		}
	}*/

	// Other

	float pp = pow(1. - max(ray_world.y + 0.1, 0.0), 8.0);
	if (true)
	{
		float3 bottomColor = lerp(finalColor.rgb, 0.9 * float3(0.9, 0.75, 0.8), pp);
		finalColor.rgb = lerp(finalColor.rgb, bottomColor, 1.0 - nightdot);
	}

	// Contrast
	finalColor.rgb = clamp(finalColor.rgb, 0., 1.);
	finalColor.rgb = finalColor.rgb*finalColor.rgb*(3.0 - 2.0*finalColor.rgb);

	// saturation (amplify colour, subtract grayscale)
	float sat = 0.2;
	finalColor.rgb = finalColor.rgb * (1. + sat) - sat * dot(finalColor.rgb, (0.33).xxx);

	finalColor.a = 1.0;
	return finalColor;
}