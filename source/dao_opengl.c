/*
// Dao Graphics Engine
// http://www.daovm.net
//
// Copyright (c) 2012-2014, Limin Fu
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
//
// * Redistributions of source code must retain the above copyright notice,
//   this list of conditions and the following disclaimer.
// * Redistributions in binary form must reproduce the above copyright notice,
//   this list of conditions and the following disclaimer in the documentation
//   and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
// OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
// SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
// OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
// HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/



#include <string.h>
#include "dao_opengl.h"
#include "dao_painter.h"


#ifdef DAO_GRAPHICS_USE_GLES

static const char *const daox_vertex_shader_header =
"#version 300 es\n\
precision highp float;\n\
";
static const char *const daox_fragment_shader_header =
"#version 300 es\n\
precision highp float;\n\
";

#else

static const char *const daox_vertex_shader_header =
"#version 150\n\
";
static const char *const daox_fragment_shader_header =
"#version 150\n\
";

#endif



static const char *const daox_vector_graphics_shader_body =
"uniform int   hasDiffuseTexture; \n\
uniform int   hasEmissionTexture; \n\
uniform int   hasBumpTexture; \n\
uniform int   hasDepthTexture; \n\
uniform float alphaBlending; \n\
uniform vec4  brushColor; \n\
uniform float graphScale; \n\
uniform float pathLength; \n\
uniform int   dashCount;         // 0: none; \n\
uniform int   gradientType;      // 0: none; 1: linear; 2: radial; 3: stroke; \n\
uniform int   gradientStops;     // number of grandient stops; \n\
uniform vec2  gradientPoint1;    // start for linear; center for radial; \n\
uniform vec2  gradientPoint2;    // end for linear; focal for radial; \n\
uniform float gradientRadius;    // radius of radial gradient; \n\
uniform sampler2D dashSampler;   // dash,gap,dash,gap,... \n\
uniform sampler2D gradientSampler; // (s,0,0,0),(s,0,0,0),(r,g,b,a),(r,g,b,a); \n\
uniform sampler2D diffuseTexture; \n\
uniform sampler2D emissionTexture; \n\
uniform sampler2D bumpTexture; \n\
\n\
\n\
vec4 InterpolateColor( vec4 C1, vec4 C2, float start, float mid, float end ) \n\
{ \n\
	float at = (mid - start) / (end - start + 1E-16); \n\
	return (1.0 - at) * C1 + at * C2; \n\
} \n\
\n\
\n\
float GradientSampler_GetStop( int i ) \n\
{ \n\
	float gradientMaxStops = float( textureSize( gradientSampler, 0 ) ); \n\
	return texture( gradientSampler, vec2( float(i) / gradientMaxStops, 0.5 ) )[0]; \n\
} \n\
\n\
vec4 GradientSampler_GetColor( int i ) \n\
{ \n\
	float gradientMaxStops = float( textureSize( gradientSampler, 0 ) ); \n\
	return texture( gradientSampler, vec2( float(i + gradientStops)/gradientMaxStops, 0.5 ) ); \n\
	return texture( gradientSampler, vec2( 0.5 + float(i) / gradientMaxStops, 0.5 ) ); \n\
} \n\
\n\
vec4 SampleGradientColor( float at ) \n\
{ \n\
	if( gradientStops == 0 ) return brushColor; \n\
	if( gradientStops == 1 ) return GradientSampler_GetColor(0); \n\
	if( at < 0.0 ) at = 0.0; \n\
	if( at > 1.0 ) at = 1.0; \n\
	if( at < GradientSampler_GetStop(0) || at > GradientSampler_GetStop(gradientStops-1) ){ \n\
		vec4 C1 = GradientSampler_GetColor(gradientStops-1); \n\
		vec4 C2 = GradientSampler_GetColor(0); \n\
		float start = GradientSampler_GetStop(gradientStops-1); \n\
		float end = GradientSampler_GetStop(0) + 1.0; \n\
		if( at < start ) at += 1.0; \n\
		return InterpolateColor( C1, C2, start, at, end ); \n\
	} \n\
	for(int i=1; i<gradientStops; ++i){ \n\
		vec4 C1 = GradientSampler_GetColor(i-1); \n\
		vec4 C2 = GradientSampler_GetColor(i); \n\
		float start = GradientSampler_GetStop(i-1); \n\
		float end = GradientSampler_GetStop(i); \n\
		if( at >= start && at <= end ) return InterpolateColor( C1, C2, start, at, end ); \n\
	} \n\
	return brushColor; \n\
}\n\
\n\
\n\
vec4 ComputeLinearGradient( vec2 point )\n\
{\n\
	vec2 A = gradientPoint1; \n\
	vec2 B = gradientPoint2; \n\
	vec2 C = point; \n\
	float BxAx = B.x - A.x; \n\
	float ByAy = B.y - A.y; \n\
	float CxAx = C.x - A.x; \n\
	float CyAy = C.y - A.y; \n\
	float t = (CxAx * BxAx + CyAy * ByAy) / (BxAx * BxAx + ByAy * ByAy); \n\
	return SampleGradientColor( t ); \n\
}\n\
\n\
\n\
vec4 ComputeRadialGradient( vec2 point )\n\
{\n\
	vec2 C = gradientPoint1; \n\
	vec2 F = gradientPoint2; \n\
	vec2 G = point; \n\
	float R = gradientRadius; \n\
	float GxFx = G.x - F.x; \n\
	float GyFy = G.y - F.y; \n\
	float FxCx = F.x - C.x; \n\
	float FyCy = F.y - C.y; \n\
	float a = GxFx * GxFx + GyFy * GyFy; \n\
	float b = 2.0 * (GxFx * FxCx + GyFy * FyCy); \n\
	float c = FxCx * FxCx + FyCy * FyCy - R * R; \n\
	float t = (- b + sqrt(b * b - 4.0 * a * c)) / (2.0 * a); \n\
	if( t < 1.0 ){ t = 1.0; }else{ t = 1.0 / t; } \n\
	return SampleGradientColor( t ); \n\
}\n\
\n\
\n\
vec4 ComputeGradient( vec2 point, float pathOffset )\n\
{\n\
	if( gradientType == 1 ){\n\
		return ComputeLinearGradient( point ); \n\
	}else if( gradientType == 2 ){\n\
		return ComputeRadialGradient( point ); \n\
	}else if( gradientType == 3 ){\n\
		return SampleGradientColor( pathOffset / pathLength ); \n\
	}\n\
	return brushColor; \n\
}\n\
\n\
\n\
float HandleDash( float offset )\n\
{\n\
	float sum = 0.0; \n\
	float dashMaxCount = float( textureSize( dashSampler, 0 ) ); \n\
	int i; \n\
	for(i=0; i<dashCount; ++i) sum += texture( dashSampler, vec2( float(i)/dashMaxCount, 0.5 ) )[0]; \n\
	offset -= sum * float(int(offset/sum)); \n\
	for(i=0; i<dashCount; ++i){ \n\
		float dash = texture( dashSampler, vec2( float(i)/dashMaxCount, 0.5 ) )[0]; \n\
		if( offset < dash ) break; \n\
		offset -= dash; \n\
	} \n\
	if( i%2 > 0 ) discard; \n\
	float dash = texture( dashSampler, vec2( float(i)/dashMaxCount, 0.5 ) )[0]; \n\
	float dx = dFdx( offset ); \n\
	float dy = dFdy( offset ); \n\
	// implicit lines: offset*(dash-offset) = 0 \n\
	float fx = (dash - 2.0*offset) * dx; \n\
	float fy = (dash - 2.0*offset) * dy; \n\
	float sd = offset*(dash-offset) / sqrt( fx*fx + fy*fy ); \n\
	\n\
	float alpha = (sd - 0.5); \n\
	if( alpha < 0.0 ) discard; \n\
	if( alpha < 2.0 ) return 0.5*alpha; \n\
	return 1.0; \n\
}\n\
\n\
\n\
\n\
// \n\
// Loop, Charles, and Jim Blinn: \n\
// GPU Gems 3: Chapter 25. Rendering Vector Art on the GPU. \n\
// http://http.developer.nvidia.com/GPUGems3/gpugems3_ch25.html \n\
// \n\
// Loop, Charles, and Jim Blinn. 2005: \n\
// Resolution Independent Curve Rendering using Programmable Graphics Hardware. \n\
// In ACM Transactions on Graphics (Proceedings of SIGGRAPH 2005) 24(3), pp. 1000–1008. \n\
// \n\
vec4 ComputeQuadraticBezier( vec2 p, vec4 color )\n\
{\n\
	// Gradients: \n\
	vec2 px = dFdx( p ); \n\
	vec2 py = dFdy( p ); \n\
                         \n\
	// Chain rule: \n\
	float fx = (2.0*p.x)*px.x - px.y; \n\
	float fy = (2.0*p.x)*py.x - py.y; \n\
                                    \n\
	// Signed distance: \n\
	float sd = (p.x*p.x - p.y) / sqrt(fx*fx + fy*fy); \n\
                                                      \n\
	// Linear alpha: \n\
	float alpha = 0.5 - sd; \n\
	if( alpha < 0.0 ) discard; \n\
	if( alpha < 1.0 ) color.a *= alpha; \n\
	return color; \n\
}\n\
\n\
\n\
vec4 ComputeCubicBezier( vec3 p, vec4 color )\n\
{\n\
	// Gradients: \n\
	vec3 px = dFdx( p ); \n\
	vec3 py = dFdy( p ); \n\
                         \n\
	// Chain rule: \n\
	float fx = (3.0*p.x*p.x)*px.x - p.y*px.y - p.z*px.z; \n\
	float fy = (3.0*p.x*p.x)*py.x - p.y*py.y - p.z*py.z; \n\
                                                       \n\
	// Signed distance: \n\
	float sd = -(p.x*p.x*p.x - p.y*p.z) / sqrt(fx*fx + fy*fy); \n\
                                                              \n\
	// Linear alpha: \n\
	float alpha = 0.25 - sd; \n\
	if( alpha < 0.0 ) discard; \n\
	if( alpha < 0.5 ) color.a *= 2.0*alpha; \n\
	return color; \n\
}\n\
\n\
\n\
vec4 RenderVectorGraphics( vec2 vertexPosition, vec3 bezierKLM, vec2 texUV, float pathOffset ) \n\
{ \n\
	float alphaBlending2 = alphaBlending; \n\
	vec4 fragColor = brushColor; \n\
	// switch-case not working with intel graphics cards? \n\
	//switch( gradientType ){ \n\
	//case 0: break; \n\
	//case 1: fragColor = ComputeLinearGradient( vertexPosition ); break; \n\
	//case 2: fragColor = ComputeRadialGradient( vertexPosition ); break; \n\
	//} \n\
	if( dashCount > 0 ) alphaBlending2 *= HandleDash( pathOffset * graphScale ); \n\
	if( gradientType > 0 ) fragColor = ComputeGradient( vertexPosition, pathOffset ); \n\
	if( hasDiffuseTexture > 0 ) fragColor = texture( diffuseTexture, texUV ); \n\
	float klm = abs( bezierKLM[0] ) + abs( bezierKLM[1] ) + abs( bezierKLM[2] ); \n\
	if( klm > 1E-16 ) fragColor = ComputeCubicBezier( bezierKLM, fragColor ); \n\
	//fragColor = ComputeQuadraticBezier( vec2( bezierKLM ), fragColor ); \n\
	fragColor.a *= alphaBlending2; \n\
	return fragColor; \n\
}";




static const char *const daox_vertex_shader2d_body =
"uniform mat4 modelMatrix; \n\
uniform mat4 viewMatrix; \n\
uniform mat4 projMatrix; \n\
uniform float graphScale; \n\
\n\
in  vec2  position; \n\
in  vec4  texKLMO; \n\
out vec2  texcoord; \n\
out vec2  vertexPosition; \n\
out vec3  bezierKLM; \n\
out float pathOffset; \n\
\n\
void main(void) \n\
{ \n\
	texcoord = vec2( texKLMO ); \n\
	bezierKLM = vec3( texKLMO ); \n\
	pathOffset = texKLMO[3]; \n\
	vertexPosition = graphScale * position; \n\
	gl_Position = projMatrix * viewMatrix * modelMatrix * vec4( vertexPosition, 0.0, 1.0 ); \n\
}";

static const char *const daox_fragment_shader2d_body =
"in  vec2  vertexPosition; \n\
in  vec2  texcoord; \n\
in  vec3  bezierKLM; \n\
in  float pathOffset; \n\
out vec4  fragColor; \n\
\n\
\n\
void main(void) \n\
{ \n\
	fragColor = RenderVectorGraphics( vertexPosition, bezierKLM, texcoord, pathOffset ); \n\
}";




static const char *const daox_vertex_shader3d_body =
"uniform int  vectorGraphics;\n\
uniform int  lightCount;\n\
uniform int  skinning;\n\
uniform vec3 cameraPosition;\n\
uniform mat4 projMatrix;\n\
uniform mat4 viewMatrix;\n\
uniform mat4 modelMatrix;\n\
uniform float graphScale; \n\
uniform mat4 skinMatRows[128];\n\
\n\
in vec3 position;\n\
in vec3 normal;\n\
in vec3 tangent;\n\
in vec2 texCoord;\n\
in vec2 texMO;\n\
in vec4 joints;\n\
in vec4 weights;\n\
\n\
out vec2  vertexPosition; \n\
out vec3  bezierKLM; \n\
out float pathOffset; \n\
\n\
out vec3 varPosition; \n\
out vec3 varNormal;  \n\
out vec3 varTangent; \n\
out vec2 varTexCoord;\n\
out vec2 varDeviceCoord;\n\
\n\
void main(void)\n\
{\n\
	varPosition = position;\n\
	if( vectorGraphics > 0 )\{\
		varPosition.x = position.x * graphScale;\n\
		varPosition.y = position.y * graphScale;\n\
	}\n\
	vec4 worldPosition = vec4( varPosition, 1.0 );\n\
	worldPosition = modelMatrix * worldPosition;\n\
	if( skinning != 0 ){ \n\
		worldPosition = \n\
			skinMatRows[int(joints[0])] * worldPosition * weights[0] + \n\
			skinMatRows[int(joints[1])] * worldPosition * weights[1] + \n\
			skinMatRows[int(joints[2])] * worldPosition * weights[2] + \n\
			skinMatRows[int(joints[3])] * worldPosition * weights[3]; \n\
	}\n\
	varNormal = normal;\n\
	varTangent = tangent;\n\
	varTexCoord = texCoord;\n\
	vertexPosition = vec2( varPosition ); \n\
	bezierKLM = vec3( texCoord, texMO[0] ); \n\
	pathOffset = texMO[1]; \n\
//	bezierKLM.x = joints[0]/50.0;\n\
//	bezierKLM.y = joints[1]/50.0;\n\
//	bezierKLM.z = joints[2]/50.0;\n\
//	bezierKLM.x = weights[0];\n\
//	bezierKLM.y = weights[1];\n\
//	bezierKLM.z = weights[2];\n\
	gl_Position = projMatrix * viewMatrix * worldPosition;\n\
	varDeviceCoord = vec2( gl_Position );\n\
}\n";


static const char *const daox_fragment_shader3d_body =
"uniform int  vectorGraphics;\n\
uniform int  terrainTileType; // 0: none; 1: square; 2: hexagon; \n\
uniform int  particleType; \n\
uniform int  lightCount;\n\
uniform vec4 ambientColor;\n\
uniform vec4 diffuseColor;\n\
uniform vec4 specularColor;\n\
uniform vec4 emissionColor;\n\
uniform float shininess;\n\
uniform float time;\n\
\n\
uniform vec3 lightSource[32];\n\
uniform vec4 lightIntensity[32];\n\
uniform vec3 cameraPosition;\n\
uniform mat4 modelMatrix;\n\
\n\
uniform int   tileTextureCount;\n\
uniform float tileTextureScale;\n\
uniform sampler2D tileTexture1;\n\
uniform sampler2D tileTexture2;\n\
uniform sampler2D tileTexture3;\n\
uniform sampler2D tileTexture4;\n\
uniform sampler2D tileTexture5;\n\
uniform sampler2D tileTexture6;\n\
uniform sampler2D depthTexture;\n\
\n\
in  vec3 varPosition;\n\
in  vec3 varNormal;\n\
in  vec3 varTangent;\n\
in  vec2 varTexCoord;\n\
in  vec2 vertexPosition; \n\
in  vec2 varDeviceCoord;\n\
in  vec3 bezierKLM; \n\
in  float pathOffset; \n\
out vec4 fragColor;\n\
\n\
vec3 worldPosition;\n\
vec4 tileTextureInfo = vec4(0.0,0.0,0.0,0.0);\n\
float tileBlendingWidth = 0.1;\n\
int hasDiffuseTexture2 = hasDiffuseTexture;\n\
\n\
\n\
// texture coordinate offset from the center in unit space:\n\
float hexagonTextureRatio = 0.4;\n\
\n\
vec2 rectangleVertices[4] = vec2[4]\n\
(\n\
	vec2(  1.0, -1.0 ),\n\
	vec2(  1.0,  1.0 ),\n\
	vec2( -1.0,  1.0 ),\n\
	vec2( -1.0, -1.0 )\n\
);\n\
\n\
\n\
vec2 hexagonVertices[6] = vec2[6]\n\
(\n\
	vec2(  0.86602540, -0.5 ),\n\
	vec2(  0.86602540,  0.5 ),\n\
	vec2(  0.0,  1.0 ),\n\
	vec2( -0.86602540,  0.5 ),\n\
	vec2( -0.86602540, -0.5 ),\n\
	vec2(  0.0, -1.0 )\n\
);\n\
\n\
\n\
\n\
\n\
vec4 RectLocateTex( vec2 tex )\n\
{\n\
	tex = tex / tileTextureScale;\n\
	vec2 C = vec2(0.5, 0.5);\n\
	vec2 P = (tex - C) / hexagonTextureRatio; // To unit space; \n\
	vec2 Q = vec2(0.0, 0.0);\n\
	float min = 1.0;\n\
	int imin = 0;\n\
	for(int i=0; i<4; ++i){\n\
		vec2 A = rectangleVertices[i];\n\
		vec2 B = rectangleVertices[(i+1)%4];\n\
		vec2 AB = 0.5 * (B - A);\n\
		vec2 AP = P - A;\n\
		vec2 AP2 = dot( AB, AP ) * AB;\n\
		vec2 D = AP - AP2;\n\
		float d = sqrt( D.x * D.x + D.y * D.y );\n\
		if( d < min ){\n\
			min = d;\n\
			imin = i;\n\
		}\n\
	}\n\
	if( imin == 0 ) Q = P + (rectangleVertices[3] - rectangleVertices[0]);\n\
	if( imin == 1 ) Q = P + (rectangleVertices[0] - rectangleVertices[1]);\n\
	if( imin == 2 ) Q = P + (rectangleVertices[0] - rectangleVertices[3]);\n\
	if( imin == 3 ) Q = P + (rectangleVertices[1] - rectangleVertices[0]);\n\
	min *= hexagonTextureRatio;\n\
	Q = hexagonTextureRatio * Q + C;\n\
	Q *= tileTextureScale;\n\
	return vec4( imin, min, Q.x, Q.y );\n\
}\n\
\n\
\n\
vec4 HexLocateTex( vec2 tex )\n\
{\n\
	tex = tex / tileTextureScale;\n\
	vec2 C = vec2(0.5, 0.5);\n\
	vec2 P = (tex - C) / hexagonTextureRatio; // To unit space; \n\
	vec2 Q = vec2(0.0, 0.0);\n\
	float min = 1.0;\n\
	int imin = 0;\n\
	for(int i=0; i<6; ++i){\n\
		vec2 A = hexagonVertices[i];\n\
		vec2 B = hexagonVertices[(i+1)%6];\n\
		vec2 AB = B - A;\n\
		vec2 AP = P - A;\n\
		vec2 AP2 = dot( AB, AP ) * AB;\n\
		vec2 D = AP - AP2;\n\
		float d = sqrt( D.x * D.x + D.y * D.y );\n\
		if( d < min ){\n\
			min = d;\n\
			imin = i;\n\
		}\n\
	}\n\
	if( imin == 0 ) Q = P + (hexagonVertices[4] - hexagonVertices[0]);\n\
	if( imin == 1 ) Q = P + (hexagonVertices[5] - hexagonVertices[1]);\n\
	if( imin == 2 ) Q = P + (hexagonVertices[0] - hexagonVertices[2]);\n\
	if( imin == 3 ) Q = P + (hexagonVertices[0] - hexagonVertices[4]);\n\
	if( imin == 4 ) Q = P + (hexagonVertices[1] - hexagonVertices[5]);\n\
	if( imin == 5 ) Q = P + (hexagonVertices[2] - hexagonVertices[0]);\n\
	min *= hexagonTextureRatio;\n\
	Q = hexagonTextureRatio * Q + C;\n\
	Q *= tileTextureScale;\n\
	return vec4( imin, min, Q.x, Q.y );\n\
}\n\
\n\
vec4 BlendTerrainTextures( vec4 texValue, vec2 tex )\n\
{\n\
	if( terrainTileType == 0 ) return texValue;\n\
	\n\
	if( tileTextureInfo.y < tileBlendingWidth ){ \n\
		vec2 tex2 = vec2(tileTextureInfo[2], tileTextureInfo[3]);\n\
		vec4 texValue2 = texValue;\n\
		if( tileTextureInfo.x == 0.0 ) texValue2 = texture( tileTexture1, tex2 );\n\
		if( tileTextureInfo.x == 1.0 ) texValue2 = texture( tileTexture2, tex2 );\n\
		if( tileTextureInfo.x == 2.0 ) texValue2 = texture( tileTexture3, tex2 );\n\
		if( tileTextureInfo.x == 3.0 ) texValue2 = texture( tileTexture4, tex2 );\n\
		if( tileTextureInfo.x == 4.0 ) texValue2 = texture( tileTexture5, tex2 );\n\
		if( tileTextureInfo.x == 5.0 ) texValue2 = texture( tileTexture6, tex2 );\n\
		float factor = 0.5 + 0.5 * tileTextureInfo.y / tileBlendingWidth;\n\
		float alpha = texValue[3];\n\
		texValue = factor * texValue + (1.0 - factor) * texValue2;\n\
		texValue[3] = alpha;\n\
	}\n\
	return texValue;\n\
}\n\
\n\
\n\
vec3 InterpolateNormal( vec3 v1, vec3 v2, float t )\n\
{\n\
	return (1.0-t)*v1 + t*v2;\n\
}\n\
\n\
\n\
vec3 ComputeLight( vec3 lightDir, vec3 lightIntensity, vec3 diffColor )\n\
{\n\
	vec3 camDir = normalize( cameraPosition - worldPosition );\n\
	vec3 normal = normalize( varNormal );\n\
	if( hasBumpTexture > 0 ){\n\
		vec3 tangent = normalize( varTangent );\n\
		vec3 binormal = cross( normal, tangent );\n\
		float ldx = dot( lightDir, tangent );\n\
		float ldy = dot( lightDir, binormal );\n\
		float ldz = dot( lightDir, normal );\n\
		float cdx = dot( camDir, tangent );\n\
		float cdy = dot( camDir, binormal );\n\
		float cdz = dot( camDir, normal );\n\
		vec3 lightDir2 = normalize( vec3( ldx, ldy, ldz ) );\n\
		vec3 camDir2 = normalize( vec3( cdx, cdy, cdz ) );\n\
		vec3 normal2 = vec3( texture( bumpTexture, varTexCoord ) );\n\
		normal2 = (normal2 - 0.5) * 2.0;\n\
		normal = normal2;\n\
		lightDir = lightDir2;\n\
		camDir = camDir2;\n\
	}\n\
	float cosAngIncidence = dot( normal, lightDir );\n\
	cosAngIncidence = clamp(cosAngIncidence, 0.0, 1.0);\n\
	vec3 halfVec = normalize(camDir + lightDir);\n\
	vec3 vertexColor = lightIntensity * diffColor * cosAngIncidence;\n\
	float dotvalue = dot(halfVec, normal);\n\
	dotvalue = clamp(dotvalue, 0.0, 1.0);\n\
	vertexColor += lightIntensity * vec3(specularColor) * pow( dotvalue, shininess );\n\
	return vertexColor;\n\
}\n\
vec4 ComputeAllLights( vec4 diffColor, vec4 emiColor )\n\
{\n\
	vec3 litColor = vec3( 0.0, 0.0, 0.0 );\n\
	//diffColor = vec4( 0.5, 0.5, 0.5, 1.0 ); // for convenient checking;\n\
	for(int i=0; i<lightCount; ++i){\n\
		vec3 lightDir = normalize( lightSource[i] - worldPosition );\n\
		litColor += ComputeLight( lightDir, vec3(lightIntensity[i]), vec3(diffColor) );\n\
	}\n\
	float alpha2 = diffColor[3];\n\
	float alpha = emiColor[3];\n\
	vec4 vertexColor = vec4( litColor, alpha2 );\n\
	vec4 vertexColor2 = vertexColor + emiColor + 0.1*normalize(ambientColor); // TODO: ambient light;\n\
	vertexColor = (1.0 - alpha)*vertexColor + alpha * vertexColor2;\n\
	vertexColor[3] = alpha2;\n\
	return vertexColor;\n\
}\n\
\n\
\n\
float Noise( vec2 uv )\n\
{\n\
	return fract(sin(dot(uv.xy, vec2(12.9898,78.233))) * 43758.5453);\n\
}\n\
\n\
float Noise3( vec3 uv )\n\
{\n\
	return fract(sin(dot(uv, vec3(12.9898,78.233,37.96))) * 43758.5453);\n\
}\n\
\n\
float Noise2( vec2 uv, int KK )\n\
{\n\
	float x = int(uv.x * KK) / float(KK);\n\
	float y = int(uv.y * KK) / float(KK);\n\
	float d = 1.0 / float(KK);\n\
	float f1 = Noise( vec2( x, y ) );\n\
	float f2 = Noise( vec2( x+d, y ) );\n\
	float f3 = Noise( vec2( x+d, y+d ) );\n\
	float f4 = Noise( vec2( x, y+d ) );\n\
	float n1 = mix( f1, f2, (uv.x - x) * KK );\n\
	float n2 = mix( f4, f3, (uv.x - x) * KK );\n\
	return mix( n1, n2, (uv.y - y) * KK );\n\
}\n\
float Noise4( vec3 pos, int KK )\n\
{\n\
	pos = pos + vec3( 10000, 10000, 10000 );\n\
	float x = int(pos.x * KK) / float(KK);\n\
	float y = int(pos.y * KK) / float(KK);\n\
	float z = int(pos.z * KK) / float(KK);\n\
	float d = 1.0 / float(KK);\n\
	float f1 = Noise3( vec3( x, y, z ) );\n\
	float f2 = Noise3( vec3( x+d, y, z ) );\n\
	float f3 = Noise3( vec3( x+d, y+d, z ) );\n\
	float f4 = Noise3( vec3( x, y+d, z ) );\n\
	float f5 = Noise3( vec3( x, y, z+d ) );\n\
	float f6 = Noise3( vec3( x+d, y, z+d ) );\n\
	float f7 = Noise3( vec3( x+d, y+d, z+d ) );\n\
	float f8 = Noise3( vec3( x, y+d, z+d ) );\n\
	float m1 = mix( f1, f2, (pos.x - x) * KK );\n\
	float m2 = mix( f4, f3, (pos.x - x) * KK );\n\
	float m3 = mix( f5, f6, (pos.x - x) * KK );\n\
	float m4 = mix( f8, f7, (pos.x - x) * KK );\n\
	float n1 = mix( m1, m2, (pos.y - y) * KK );\n\
	float n2 = mix( m3, m4, (pos.y - y) * KK );\n\
	return mix( n1, n2, (pos.z - z) * KK );\n\
}\n\
\n\
float ParticleFactor( float x, float y )\n\
{\n\
	float tm = time * 0.01;\n\
	float var = tm - int(tm);\n\
	vec2 seed = vec2( x+var, y-var );\n\
	float n1 = clamp( Noise4( 0.5*varPosition + vec3(var, var, var), 1), 0.0, 1.0 );\n\
	float n2 = clamp( Noise2( 0.5*seed, 20), 0.0, 1.0 );\n\
	float n = mix( n1, mix( n2, n1, varTangent.z), 0.5 );\n\
	float ds = x - 0.5;\n\
	float dt = y - 0.5;\n\
	float r = sqrt( ds*ds + dt*dt );\n\
	if( r >= 0.5 ) return 0;\n\
	float loc = sqrt(1.0 - 2.0 * r);\n\
	return n * loc * varTangent.z;\n\
}\n\
vec4 ParticleColor( float factor )\n\
{\n\
	vec4 diffColor1 = mix( diffuseColor, emissionColor, sqrt(factor) );\n\
	vec4 diffColor2 = mix( ambientColor, diffuseColor, factor*factor );\n\
	return mix( diffColor2, diffColor1, varTangent.z );\n\
}\n\
\n\
\n\
\n\
\n\
void main(void)\n\
{\n\
	vec4 diffColor = diffuseColor;\n\
	vec4 emiColor = emissionColor;\n\
	worldPosition = vec3( modelMatrix * vec4( varPosition, 1.0 ) );\n\
	if( terrainTileType == 1 ) tileTextureInfo = RectLocateTex( varTexCoord );\n\
	if( terrainTileType == 2 ) tileTextureInfo = HexLocateTex( varTexCoord );\n\
	if( tileTextureCount > 0 ) hasDiffuseTexture2 = 1;\n\
	if( hasDiffuseTexture2 > 0 ){\n\
		diffColor = texture( diffuseTexture, varTexCoord );\n\
		if( terrainTileType != 0 ){ \n\
			diffColor = BlendTerrainTextures( diffColor, varTexCoord );\n\
		}\n\
	}\n\
	if( hasEmissionTexture > 0 ){\n\
		emiColor = texture( emissionTexture, varTexCoord );\n\
	}\n\
	//diffColor = vec4(bezierKLM, 1.0);\n\
	fragColor = ComputeAllLights( diffColor, emiColor );\n\
	if( hasDiffuseTexture2 > 0 ){\n\
		if( diffColor[3] < 0.9 ) discard;\n\
		fragColor[3] = diffColor[3];\n\
	}\n\
	if( lightCount == 0 ) fragColor = diffColor + emiColor;\n\
	//fragColor = diffColor;\n\
	if( particleType > 0 ){\n\
		if( hasDepthTexture > 0 ){\n\
			vec2 sv = vec2( gl_FragCoord.x/960.0, gl_FragCoord.y/800.0 );\n\
			float depth = texture( depthTexture, sv).r;\n\
			if( gl_FragCoord.z > depth ) discard;\n\
		}\n\
		float alpha = ParticleFactor( varTexCoord.x, varTexCoord.y ); \n\
		float cutoff = 0.1;\n\
		if( alpha < cutoff ) discard;\n\
		alpha = (alpha - cutoff) / (1.0 - cutoff);\n\
		fragColor = ParticleColor( alpha );\n\
		if( hasDepthTexture > 0 ){\n\
			fragColor[3] = 1.0 - exp( -25*alpha*alpha );\n\
		}\n\
	}\n\
	if( vectorGraphics > 0 ){ \n\
		vec4 color = RenderVectorGraphics( vertexPosition, bezierKLM, varTexCoord, pathOffset ); \n\
			fragColor = fragColor * color; \n\
	}\n\
}\n";



DaoxShader* DaoxShader_New()
{
	DaoxShader *self = (DaoxShader*) dao_calloc(1,sizeof(DaoxShader));
	DaoCstruct_Init( (DaoCstruct*) self, daox_type_shader );
	self->vertexSources = DList_New( DAO_DATA_STRING );
	self->fragmentSources = DList_New( DAO_DATA_STRING );
	return self;
}
void DaoxShader_Delete( DaoxShader *self )
{
	DList_Delete( self->vertexSources );
	DList_Delete( self->fragmentSources );
	DaoCstruct_Free( (DaoCstruct*) self );
	dao_free( self );
}

void DaoxShader_Init2D( DaoxShader *self )
{
	self->mode = DAOX_GRAPHICS_2D;

	DaoxShader_AddShader( self, GL_VERTEX_SHADER, daox_vertex_shader_header );
	DaoxShader_AppendShader( self, GL_VERTEX_SHADER, daox_vertex_shader2d_body );

	DaoxShader_AddShader( self, GL_FRAGMENT_SHADER, daox_fragment_shader_header );
	DaoxShader_AppendShader( self, GL_FRAGMENT_SHADER, daox_vector_graphics_shader_body );
	DaoxShader_AppendShader( self, GL_FRAGMENT_SHADER, daox_fragment_shader2d_body );
}
void DaoxShader_Init3D( DaoxShader *self )
{
	self->mode = DAOX_GRAPHICS_3D;

	DaoxShader_AddShader( self, GL_VERTEX_SHADER, daox_vertex_shader_header );
	DaoxShader_AppendShader( self, GL_VERTEX_SHADER, daox_vertex_shader3d_body );

	DaoxShader_AddShader( self, GL_FRAGMENT_SHADER, daox_fragment_shader_header );
	DaoxShader_AppendShader( self, GL_FRAGMENT_SHADER, daox_vector_graphics_shader_body );
	DaoxShader_AppendShader( self, GL_FRAGMENT_SHADER, daox_fragment_shader3d_body );
}
void DaoxShader_AddShader( DaoxShader *self, int type, const char *codes )
{
	DString *source = DString_NewChars( codes );
	switch( type ){
	case GL_VERTEX_SHADER :
		DList_Append( self->vertexSources, source );
		break;
	case GL_FRAGMENT_SHADER :
		DList_Append( self->fragmentSources, source );
		break;
	}
	DString_Delete( source );
}
void DaoxShader_AppendShader( DaoxShader *self, int type, const char *codes )
{
	DString *source = DString_NewChars( codes );
	switch( type ){
	case GL_VERTEX_SHADER :
		if( self->vertexSources->size ){
			DString_Append( (DString*) DList_Back( self->vertexSources ), source );
		}else{
			DList_Append( self->vertexSources, source );
		}
		break;
	case GL_FRAGMENT_SHADER :
		if( self->fragmentSources->size ){
			DString_Append( (DString*) DList_Back( self->fragmentSources ), source );
		}else{
			DList_Append( self->fragmentSources, source );
		}
		break;
	}
	DString_Delete( source );
}

void DaoxShader_CompileShader( DaoxShader *self, int type, DList *strings )
{
	daoint i, n = strings->size;
	uint_t shader = glCreateShader( type );
	const GLchar **sources;
	GLint length, shader_ok;

	if( shader == 0 ){
		fprintf(stderr, "Failed to create shader of type %i\n", type );
		return;
	}
	sources = (const GLchar**) dao_malloc( n*sizeof(GLchar*) );
	for(i=0; i<n; ++i){
		sources[i] = (const GLchar*) DString_GetData( strings->items.pString[i] );
	}

	glShaderSource( shader, n, sources, NULL );
	glCompileShader( shader );
	dao_free( sources );

	glGetShaderiv(shader, GL_COMPILE_STATUS, &shader_ok);
	if( !shader_ok ){
		const char *log2;
		DString *log = DString_New();
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length );
		DString_Resize( log, length );
		log2 = DString_GetData(log);
		glGetShaderInfoLog( shader, length, NULL, (char*)log2 );
		fprintf(stderr, "Failed to compile shader!\nWith error message: %s", log2 );
		glDeleteShader(shader);
		return;
	}
	switch( type ){
	case GL_VERTEX_SHADER :
		if( self->vertexShader ) glDeleteShader( self->vertexShader );
		self->vertexShader = shader;
		break;
	case GL_FRAGMENT_SHADER :
		if( self->fragmentShader ) glDeleteShader( self->fragmentShader );
		self->fragmentShader = shader;
		break;
	}
	if( shader && self->program ) glAttachShader( self->program, shader );
}

void DaoxShader_Finalize( DaoxShader *self )
{
	GLint length, program_ok;
	int shaderAttribute = 0;

	if( self->program == 0 ) return;

#ifndef DAO_GRAPHICS_USE_GLES
	glBindFragDataLocation( self->program, 0, "fragColor");
#endif
	glLinkProgram( self->program );
	glGetProgramiv( self->program, GL_LINK_STATUS, &program_ok );
	if( !program_ok ){
		const char *log2;
		DString *log = DString_New();
		glGetProgramiv( self->program, GL_INFO_LOG_LENGTH, &length );
		DString_Resize( log, length );
		log2 = DString_GetData(log);
		glGetProgramInfoLog( self->program, length, NULL, (char*)log2 );
		fprintf(stderr, "Failed to link shader program with error message: %s\n", log2 );
		glDeleteProgram(self->program);
		self->program = 0;
	}
}
void DaoxShader_GetVectorGraphicsUniforms( DaoxShader *self )
{
	self->uniforms.alphaBlending = glGetUniformLocation(self->program, "alphaBlending");
	self->uniforms.graphScale = glGetUniformLocation(self->program, "graphScale");
	self->uniforms.pathLength = glGetUniformLocation(self->program, "pathLength");
	self->uniforms.brushColor = glGetUniformLocation(self->program, "brushColor");
	self->uniforms.dashCount = glGetUniformLocation(self->program, "dashCount");
	self->uniforms.dashSampler = glGetUniformLocation(self->program, "dashSampler");
	self->uniforms.gradientType = glGetUniformLocation(self->program, "gradientType");
	self->uniforms.gradientStops = glGetUniformLocation(self->program, "gradientStops");
	self->uniforms.gradientPoint1 = glGetUniformLocation(self->program, "gradientPoint1");
	self->uniforms.gradientPoint2 = glGetUniformLocation(self->program, "gradientPoint2");
	self->uniforms.gradientRadius = glGetUniformLocation(self->program, "gradientRadius");
	self->uniforms.gradientSampler = glGetUniformLocation(self->program, "gradientSampler");
}
void DaoxShader_Finalize2D( DaoxShader *self )
{
	DaoxShader_Finalize( self );
	if( self->program == 0 ) return;
	DaoxShader_GetVectorGraphicsUniforms( self );
	self->uniforms.modelMatrix = glGetUniformLocation(self->program, "modelMatrix");
	self->uniforms.viewMatrix = glGetUniformLocation(self->program, "viewMatrix");
	self->uniforms.projMatrix = glGetUniformLocation(self->program, "projMatrix");
	self->uniforms.hasDiffuseTexture = glGetUniformLocation(self->program, "hasDiffuseTexture");
	self->uniforms.hasEmissionTexture = glGetUniformLocation(self->program, "hasEmissionTexture");
	self->uniforms.hasBumpTexture = glGetUniformLocation(self->program, "hasBumpTexture");
	self->uniforms.hasDepthTexture = glGetUniformLocation(self->program, "hasDepthTexture");
	self->uniforms.emissionTexture = glGetUniformLocation(self->program, "emissionTexture");
	self->uniforms.bumpTexture = glGetUniformLocation(self->program, "bumpTexture");
	self->uniforms.depthTexture = glGetUniformLocation(self->program, "depthTexture");
	self->uniforms.particleType = glGetUniformLocation(self->program, "particleType");
	self->attributes.position = glGetAttribLocation(self->program, "position");
	self->attributes.texKLMO = glGetAttribLocation(self->program, "texKLMO");
}
void DaoxShader_Finalize3D( DaoxShader *self )
{
	DaoxShader_Finalize( self );
	if( self->program == 0 ) return;
	DaoxShader_GetVectorGraphicsUniforms( self );
	self->uniforms.time = glGetUniformLocation(self->program, "time");
	self->uniforms.vectorGraphics = glGetUniformLocation(self->program, "vectorGraphics");
	self->uniforms.projMatrix = glGetUniformLocation(self->program, "projMatrix");
	self->uniforms.viewMatrix = glGetUniformLocation(self->program, "viewMatrix");
	self->uniforms.modelMatrix = glGetUniformLocation(self->program, "modelMatrix");
	self->uniforms.cameraPosition = glGetUniformLocation(self->program, "cameraPosition");
	self->uniforms.lightCount = glGetUniformLocation(self->program, "lightCount");
	self->uniforms.lightSource = glGetUniformLocation(self->program, "lightSource");
	self->uniforms.lightIntensity = glGetUniformLocation(self->program, "lightIntensity");
	self->uniforms.skinning = glGetUniformLocation(self->program, "skinning");
	self->uniforms.skinMatRows = glGetUniformLocation(self->program, "skinMatRows");
	self->uniforms.ambientColor = glGetUniformLocation(self->program, "ambientColor");
	self->uniforms.diffuseColor = glGetUniformLocation(self->program, "diffuseColor");
	self->uniforms.specularColor = glGetUniformLocation(self->program, "specularColor");
	self->uniforms.emissionColor = glGetUniformLocation(self->program, "emissionColor");
	self->uniforms.shininess = glGetUniformLocation(self->program, "shininess");
	self->uniforms.hasDiffuseTexture = glGetUniformLocation(self->program, "hasDiffuseTexture");
	self->uniforms.hasEmissionTexture = glGetUniformLocation(self->program, "hasEmissionTexture");
	self->uniforms.hasBumpTexture = glGetUniformLocation(self->program, "hasBumpTexture");
	self->uniforms.hasDepthTexture = glGetUniformLocation(self->program, "hasDepthTexture");
	self->uniforms.diffuseTexture = glGetUniformLocation(self->program, "diffuseTexture");
	self->uniforms.emissionTexture = glGetUniformLocation(self->program, "emissionTexture");
	self->uniforms.bumpTexture = glGetUniformLocation(self->program, "bumpTexture");
	self->uniforms.depthTexture = glGetUniformLocation(self->program, "depthTexture");
	self->uniforms.particleType = glGetUniformLocation(self->program, "particleType");
	self->uniforms.terrainTileType = glGetUniformLocation(self->program, "terrainTileType");
	self->uniforms.tileTextureCount = glGetUniformLocation(self->program, "tileTextureCount");
	self->uniforms.tileTextureScale = glGetUniformLocation(self->program, "tileTextureScale");
	self->uniforms.tileTextures[0] = glGetUniformLocation(self->program, "tileTexture1");
	self->uniforms.tileTextures[1] = glGetUniformLocation(self->program, "tileTexture2");
	self->uniforms.tileTextures[2] = glGetUniformLocation(self->program, "tileTexture3");
	self->uniforms.tileTextures[3] = glGetUniformLocation(self->program, "tileTexture4");
	self->uniforms.tileTextures[4] = glGetUniformLocation(self->program, "tileTexture5");
	self->uniforms.tileTextures[5] = glGetUniformLocation(self->program, "tileTexture6");

	self->attributes.position = glGetAttribLocation(self->program, "position");
	self->attributes.normal = glGetAttribLocation(self->program, "normal");
	self->attributes.tangent = glGetAttribLocation(self->program, "tangent");
	self->attributes.texCoord = glGetAttribLocation(self->program, "texCoord");
	self->attributes.texMO = glGetAttribLocation(self->program, "texMO");
	self->attributes.joints = glGetAttribLocation(self->program, "joints");
	self->attributes.weights = glGetAttribLocation(self->program, "weights");
}
void DaoxShader_InitVGSamplers( DaoxShader *self )
{
	int width = 2*DAOX_MAX_GRADIENT_STOPS;
	GLfloat data[2*DAOX_MAX_GRADIENT_STOPS*4];
	GLfloat dash[DAOX_MAX_DASH];
	GLuint tid = 0;

	memset( data, 0, width*4*sizeof(GLfloat) );
	memset( dash, 0, DAOX_MAX_DASH*sizeof(GLfloat) );

	glGenTextures( 1, & tid );
	self->textures.gradientSampler = tid;

	glBindTexture(GL_TEXTURE_2D, tid);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, 1, 0, GL_RGBA, GL_FLOAT, data);
	glBindTexture(GL_TEXTURE_2D, 0);

	glGenTextures( 1, & tid );
	self->textures.dashSampler = tid;

	glBindTexture(GL_TEXTURE_2D, tid);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, DAOX_MAX_DASH, 1, 0, GL_RED, GL_FLOAT, dash);
	glBindTexture(GL_TEXTURE_2D, 0);
}
void DaoxShader_Build2D( DaoxShader *self )
{
	self->program = glCreateProgram();

	DaoxShader_CompileShader( self, GL_VERTEX_SHADER, self->vertexSources );
	DaoxShader_CompileShader( self, GL_FRAGMENT_SHADER, self->fragmentSources );

	DaoxShader_InitVGSamplers( self );

	DaoxShader_Finalize2D( self );
}
void DaoxShader_Build3D( DaoxShader *self )
{
	self->program = glCreateProgram();

	DaoxShader_CompileShader( self, GL_VERTEX_SHADER, self->vertexSources );
	DaoxShader_CompileShader( self, GL_FRAGMENT_SHADER, self->fragmentSources );

	DaoxShader_InitVGSamplers( self );

	DaoxShader_Finalize3D( self );
}
void DaoxShader_Free( DaoxShader *self )
{
	if( self->vertexShader ) glDeleteShader( self->vertexShader );
	if( self->fragmentShader ) glDeleteShader( self->fragmentShader );
	if( self->program ) glDeleteProgram( self->program );
	if( self->textures.dashSampler ) glDeleteTextures( 1, & self->textures.dashSampler );
	if( self->textures.gradientSampler ) glDeleteTextures( 1, & self->textures.gradientSampler );
	self->program = 0;
	self->vertexShader = 0;
	self->fragmentShader = 0;
	self->textures.dashSampler = 0;
	self->textures.gradientSampler = 0;
}

void DaoxShader_MakeGradientSampler( DaoxShader *self, DaoxGradient *gradient, int fill )
{
	GLfloat data[2*DAOX_MAX_GRADIENT_STOPS*4];
	int width = 2*DAOX_MAX_GRADIENT_STOPS;
	int i, n, gradientType;

	if( gradient == NULL ){
		glUniform1i(self->uniforms.gradientType, 0 );
		return;
	}

	n = gradient->stops->size;
	if( n > DAOX_MAX_GRADIENT_STOPS ) n = DAOX_MAX_GRADIENT_STOPS;
	memset( data, 0, n*2*4*sizeof(GLfloat) );
	for(i=0; i<n; ++i){
		DaoxColor color = gradient->colors->data.colors[i];
		GLfloat *stop = data + 4*i;
		GLfloat *rgba = data + 4*(i+n);
		stop[0] = gradient->stops->data.floats[i];
		rgba[0] = color.red;
		rgba[1] = color.green;
		rgba[2] = color.blue;
		rgba[3] = color.alpha;
	}
	gradientType = fill ? gradient->gradient : DAOX_GRADIENT_STROKE;
	//printf( "DaoxShader_MakeGradientSampler..... %i %i\n", gradientType , self->textures.gradientSampler );

	glUniform1i(self->uniforms.gradientType, gradientType );
	glUniform1i(self->uniforms.gradientStops, n );
	glUniform1f(self->uniforms.gradientRadius, gradient->radius );
	glUniform2fv(self->uniforms.gradientPoint1, 1, & gradient->points[0].x );
	glUniform2fv(self->uniforms.gradientPoint2, 1, & gradient->points[1].x );

	glActiveTexture(GL_TEXTURE0 + DAOX_GRADIENT_SAMPLER );
	glBindTexture(GL_TEXTURE_2D, self->textures.gradientSampler);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 2*n, 1, GL_RGBA, GL_FLOAT, data);
	glUniform1i(self->uniforms.gradientSampler, DAOX_GRADIENT_SAMPLER );
}
void DaoxShader_MakeDashSampler( DaoxShader *self, DaoxBrush *brush )
{
	GLfloat dash[DAOX_MAX_DASH];
	int i, n;

	if( brush == NULL ){
		glUniform1i(self->uniforms.dashCount, 0 );
		return;
	}

	n = brush->strokeStyle.dash;
	if( n > DAOX_MAX_DASH ) n = DAOX_MAX_DASH;
	memset( dash, 0, n*sizeof(GLfloat) );
	for(i=0; i<n; ++i) dash[i] = brush->strokeStyle.dashes[i];

	//printf( "DaoxShader_MakeDashSampler: %i\n", n );

	glUniform1i(self->uniforms.dashCount, n );
	glActiveTexture(GL_TEXTURE0 + DAOX_DASH_SAMPLER);
	glBindTexture(GL_TEXTURE_2D, self->textures.dashSampler);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, n, 1, GL_RED, GL_FLOAT, dash);
	glUniform1i(self->uniforms.dashSampler, DAOX_DASH_SAMPLER );
}






/*
// Buffer Object Streaming:
// Server-side multi-buffering (buffer re-specification/orphaning)
// https://www.opengl.org/wiki/Buffer_Object_Streaming
// Array Texture:
// https://www.opengl.org/wiki/Array_Texture
*/

DaoxBuffer* DaoxBuffer_New()
{
	DaoxBuffer *self = (DaoxBuffer*) dao_calloc(1,sizeof(DaoxBuffer));
	DaoCstruct_Init( (DaoCstruct*) self, daox_type_buffer );
	self->vertexCapacity = 16*1024;
	self->triangleCapacity = 16*1024;
	return self;
}
void DaoxBuffer_Delete( DaoxBuffer *self )
{
	DaoCstruct_Free( (DaoCstruct*) self );
	dao_free( self );
}

void DaoxBuffer_Init2D( DaoxBuffer *self, int pos, int klmo )
{
	DaoGLVertex2D *vertex = NULL;

	self->mode = DAOX_GRAPHICS_2D;

	self->traitCount = 2;
	self->vertexSize = sizeof(DaoGLVertex2D);
	self->triangleSize = sizeof(DaoGLTriangle);
	self->traits[0].uniform = pos;
	self->traits[1].uniform = klmo;
	self->traits[0].count = 2;
	self->traits[1].count = 4;
	self->traits[0].offset = NULL;
	self->traits[1].offset = (void*) & vertex->texKLMO;
}
void DaoxBuffer_Init3D( DaoxBuffer *self, int pos, int norm, int tan, int texuv )
{
	DaoGLVertex3D *vertex = NULL;

	self->mode = DAOX_GRAPHICS_3D;

	self->traitCount = 4;
	self->vertexSize = sizeof(DaoGLVertex3D);
	self->triangleSize = sizeof(DaoGLTriangle);
	self->traits[0].uniform = pos;
	self->traits[1].uniform = norm;
	self->traits[2].uniform = tan;
	self->traits[3].uniform = texuv;
	self->traits[0].count = 3;
	self->traits[1].count = 3;
	self->traits[2].count = 3;
	self->traits[3].count = 2;
	self->traits[0].offset = NULL;
	self->traits[1].offset = (void*) & vertex->norm;
	self->traits[2].offset = (void*) & vertex->tan;
	self->traits[3].offset = (void*) & vertex->tex;
}
void DaoxBuffer_Init3DSK( DaoxBuffer *self, int pos, int norm, int tan, int texuv, int joints, int weights )
{
	DaoGLSkinVertex3D *vertex = NULL;

	DaoxBuffer_Init3D( self, pos, norm, tan, texuv );

	self->traitCount = 6;
	self->vertexSize = sizeof(DaoGLSkinVertex3D);

	self->traits[4].uniform = joints;
	self->traits[4].count = 4;
	self->traits[4].offset = (void*) & vertex->joints;

	self->traits[5].uniform = weights;
	self->traits[5].count = 4;
	self->traits[5].offset = (void*) & vertex->weights;
}
void DaoxBuffer_Init3DVG( DaoxBuffer *self, int pos, int norm, int texuv, int texmo )
{
	DaoGLVertex3DVG *vertex = NULL;

	self->mode = DAOX_GRAPHICS_3DVG;

	self->traitCount = 4;
	self->vertexSize = sizeof(DaoGLVertex3DVG);
	self->triangleSize = sizeof(DaoGLTriangle);
	self->traits[0].uniform = pos;
	self->traits[1].uniform = norm;
	self->traits[2].uniform = texuv;
	self->traits[3].uniform = texmo;
	self->traits[0].count = 3;
	self->traits[1].count = 3;
	self->traits[2].count = 2;
	self->traits[3].count = 2;
	self->traits[0].offset = NULL;
	self->traits[1].offset = (void*) & vertex->norm;
	self->traits[2].offset = (void*) & vertex->texKLMO;
	self->traits[3].offset = (void*) & vertex->texKLMO.m;
}
void DaoxBuffer_Free( DaoxBuffer *self )
{
	if( self->vertexVAO ) glDeleteVertexArrays( 1, & self->vertexVAO );
	if( self->vertexVBO ) glDeleteBuffers( 1, & self->vertexVBO );
	if( self->triangleVBO ) glDeleteBuffers( 1, & self->triangleVBO );
}

void DaoxBuffer_SetVertexBufferAttributes( DaoxBuffer *self )
{
	int i, stride = self->vertexSize;
	glBindBuffer( GL_ARRAY_BUFFER, self->vertexVBO );
	for(i=0; i<self->traitCount; ++i){
		int uniform = self->traits[i].uniform;
		int count = self->traits[i].count;
		void *offset = self->traits[i].offset;
		glEnableVertexAttribArray( uniform );
		glVertexAttribPointer( uniform, count, GL_FLOAT, GL_FALSE, stride, offset );
	}
}
void DaoxBuffer_BindBuffers( DaoxBuffer *self )
{
	glGenVertexArrays( 1, & self->vertexVAO );
	glGenBuffers( 1, & self->vertexVBO );
	glGenBuffers( 1, & self->triangleVBO );

	glBindVertexArray( self->vertexVAO );
	glBindBuffer( GL_ARRAY_BUFFER, self->vertexVBO );
	glBufferData( GL_ARRAY_BUFFER, self->vertexCapacity*self->vertexSize, NULL, GL_STREAM_DRAW );
	DaoxBuffer_SetVertexBufferAttributes( self );
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, self->triangleVBO );
	glBufferData( GL_ELEMENT_ARRAY_BUFFER, self->triangleCapacity*self->triangleSize, NULL, GL_STREAM_DRAW );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );
	glBindVertexArray(0);
}
void* DaoxBuffer_MapVertices( DaoxBuffer *self, int count )
{
	int dataSize = count * self->vertexSize;
	glBindBuffer( GL_ARRAY_BUFFER, self->vertexVBO );
	if( self->vertexOffset + count > self->vertexCapacity ){
		if( (self->vertexOffset + count) > self->vertexCapacity ) self->vertexCapacity = 1.5 * count;
		glBufferData( GL_ARRAY_BUFFER, self->vertexCapacity*self->vertexSize, NULL, GL_STREAM_DRAW );
		DaoxBuffer_SetVertexBufferAttributes( self );
		self->vertexOffset = 0;
	}
	return glMapBufferRange( GL_ARRAY_BUFFER, self->vertexOffset*self->vertexSize, dataSize, GL_MAP_WRITE_BIT|GL_MAP_UNSYNCHRONIZED_BIT|GL_MAP_INVALIDATE_BUFFER_BIT );
}
DaoGLVertex2D* DaoxBuffer_MapVertices2D( DaoxBuffer *self, int count )
{
	return (DaoGLVertex2D*) DaoxBuffer_MapVertices( self, count );
}
DaoGLVertex3D* DaoxBuffer_MapVertices3D( DaoxBuffer *self, int count )
{
	return (DaoGLVertex3D*) DaoxBuffer_MapVertices( self, count );
}
DaoGLVertex3DVG* DaoxBuffer_MapVertices3DVG( DaoxBuffer *self, int count )
{
	return (DaoGLVertex3DVG*) DaoxBuffer_MapVertices( self, count );
}
DaoGLTriangle* DaoxBuffer_MapTriangles( DaoxBuffer *self, int count )
{
	int dataSize = count * self->triangleSize;
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, self->triangleVBO );
	if( self->triangleOffset + count > self->triangleCapacity ){
		if( (self->triangleOffset + count) > self->triangleCapacity ) self->triangleCapacity = 1.5 * count;
		glBufferData( GL_ELEMENT_ARRAY_BUFFER, self->triangleCapacity*self->triangleSize, NULL, GL_STREAM_DRAW );
		self->triangleOffset = 0;
	}
	return (DaoGLTriangle*) glMapBufferRange( GL_ELEMENT_ARRAY_BUFFER, self->triangleOffset*self->triangleSize, dataSize, GL_MAP_WRITE_BIT|GL_MAP_UNSYNCHRONIZED_BIT|GL_MAP_INVALIDATE_BUFFER_BIT );
}



DaoxContext* DaoxContext_New()
{
	DaoxContext *self = (DaoxContext*) dao_calloc(1,sizeof(DaoxContext));
	DaoCstruct_Init( (DaoCstruct*) self, daox_type_context );
	self->shaders = DList_New(DAO_DATA_VALUE);
	self->buffers = DList_New(DAO_DATA_VALUE);
	self->textures = DList_New(DAO_DATA_VALUE);
	self->deviceWidth  = 300;
	self->deviceHeight = 200;
	return self;
}
void DaoxContext_Delete( DaoxContext *self )
{
	DaoxContext_Clear( self );
	DaoCstruct_Free( (DaoCstruct*) self );
	dao_free( self );
}
void DaoxTexture_Free( DaoxTexture *self );
void DaoxContext_Clear( DaoxContext *self )
{
	int i;
	for(i=0; i<self->shaders->size; ++i){
		DaoxShader *shader = (DaoxShader*) self->shaders->items.pValue[i];
		DaoxShader_Free( shader );
	}
	for(i=0; i<self->buffers->size; ++i){
		DaoxBuffer *buffer = (DaoxBuffer*) self->buffers->items.pValue[i];
		DaoxBuffer_Free( buffer );
	}
	for(i=0; i<self->textures->size; ++i){
		DaoxTexture *texture = (DaoxTexture*) self->textures->items.pValue[i];
		DaoxTexture_Free( texture );
	}
	DList_Clear( self->shaders );
	DList_Clear( self->buffers );
	DList_Clear( self->textures );
	if( self->colorTexture ) glDeleteTextures( 1, & self->colorTexture );
	if( self->depthTexture ) glDeleteTextures( 1, & self->depthTexture );
	if( self->frameBuffer ) glDeleteFramebuffers( 1, & self->frameBuffer );
}

int DaoxContext_BindShader( DaoxContext *self, DaoxShader *shader )
{
	if( shader->ctx != self ) DList_Append( self->shaders, shader );
	shader->ctx = self;  /* No GC, should only be used for pointer comparison; */
	switch( shader->mode ){
	case DAOX_GRAPHICS_2D : DaoxShader_Build2D( shader ); break;
	case DAOX_GRAPHICS_3D : DaoxShader_Build3D( shader ); break;
	}
	return 1;
}
int DaoxContext_BindBuffer( DaoxContext *self, DaoxBuffer *buffer )
{
	if( buffer->ctx != self ) DList_Append( self->buffers, buffer );
	buffer->ctx = self;  /* No GC, should only be used for pointer comparison; */
	DaoxBuffer_BindBuffers( buffer );
	return 1;
}
void DaoxTexture_Free( DaoxTexture *self )
{
	GLuint tid = self->tid;
	if( tid == 0 ) return;
	glDeleteTextures( 1, & tid );
	self->tid = 0;
}
int DaoxContext_BindTexture( DaoxContext *self, DaoxTexture *texture )
{
	uchar_t *data;
	GLuint tid = 0;
	int W, H;

	if( texture->image == NULL ) return 0;

	data = texture->image->imageData;
	W = texture->image->width;
	H = texture->image->height;

	if( W == 0 || H == 0 ) return 0;

	if( texture->ctx == self && texture->tid ) return 1;
	if( texture->tid ) DaoxTexture_Free( texture );

	if( texture->ctx != self ){
		DList_Append( self->textures, texture );
		texture->ctx = self;
	}

	glGenTextures( 1, & tid );
	texture->tid = tid;

	glBindTexture(GL_TEXTURE_2D, texture->tid);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	if( texture->image->depth == DAOX_IMAGE_BIT24 ){
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, W, H, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	}else if( texture->image->depth == DAOX_IMAGE_BIT32 ){
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, W, H, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	}
	glBindTexture(GL_TEXTURE_2D, 0);
	return 1;
}
void DaoxContext_InitOffscreenBuffer( DaoxContext *self )
{
	int ret;

	if( self->colorTexture ) return;

	glGenTextures(1, & self->colorTexture);
	glBindTexture(GL_TEXTURE_2D, self->colorTexture);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, self->deviceWidth, self->deviceHeight, 0,
			GL_RGBA, GL_UNSIGNED_BYTE, 0);
	glBindTexture(GL_TEXTURE_2D, 0);

	glGenTextures(1, & self->depthTexture);
	glBindTexture(GL_TEXTURE_2D, self->depthTexture);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, self->deviceWidth, self->deviceHeight,
			0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
	glBindTexture(GL_TEXTURE_2D, 0);

	glGenFramebuffers(1, & self->frameBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, self->frameBuffer);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, self->colorTexture, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, self->depthTexture, 0);

	switch( (ret = glCheckFramebufferStatus(GL_FRAMEBUFFER)) ){
	case GL_FRAMEBUFFER_COMPLETE:
		break;
	case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
		printf("Framebuffer Object Error (%d): Attachment Point Unconnected", ret);
		break;
	case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
		printf("Framebuffer Object Error (%d): Missing Attachment", ret);
		break;
	case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
		printf("Framebuffer Object Error (%d): Draw Buffer", ret);
		break;
	case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
		printf("Framebuffer Object Error (%d): Read Buffer", ret);
		break;
	case GL_FRAMEBUFFER_UNSUPPORTED:
		printf("Framebuffer Object Error (%d): Unsupported Framebuffer Configuration", ret);
		break;
	default:
		printf("Framebuffer Object Error (%d): Unkown Framebuffer Object Failure", ret);
		break;
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	self->offscreen = ret == GL_FRAMEBUFFER_COMPLETE;
	printf( "offscreen = %i\n", self->offscreen );
}



void DaoxMatrix4D_Export( DaoxMatrix4D *self, GLfloat matrix[16] )
{
	matrix[0] = self->A11;
	matrix[1] = self->A21;
	matrix[2] = self->A31;
	matrix[3] = 0.0;
	matrix[4] = self->A12;
	matrix[5] = self->A22;
	matrix[6] = self->A32;
	matrix[7] = 0.0;
	matrix[8] = self->A13;
	matrix[9] = self->A23;
	matrix[10] = self->A33;
	matrix[11] = 0.0;
	matrix[12] = self->B1;
	matrix[13] = self->B2;
	matrix[14] = self->B3;
	matrix[15] = 1.0;
}
