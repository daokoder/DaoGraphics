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
"//#version 300 es\n\
precision highp float;\n\
";
static const char *const daox_fragment_shader_header =
"//#version 300 es\n\
precision highp float;\n\
";

#else

static const char *const daox_vertex_shader_header =
"//#version 150\n\
";
static const char *const daox_fragment_shader_header =
"//#version 150\n\
";

#endif



static const char *const daox_vector_graphics_shader_body =
"uniform int   hasColorTexture; \n\
uniform int   hasBumpTexture; \n\
uniform float alphaBlending; \n\
uniform vec4  brushColor; \n\
uniform float pathLength; \n\
uniform int   dashCount;         // 0: none; \n\
uniform int   gradientType;      // 0: none; 1: linear; 2: radial; 3: stroke; \n\
uniform int   gradientStops;     // number of grandient stops; \n\
uniform vec2  gradientPoint1;    // start for linear; center for radial; \n\
uniform vec2  gradientPoint2;    // end for linear; focal for radial; \n\
uniform float gradientRadius;    // radius of radial gradient; \n\
uniform sampler2D dashSampler;   // dash,gap,dash,gap,... \n\
uniform sampler2D gradientSampler; // (s,0,0,0),(s,0,0,0),(r,g,b,a),(r,g,b,a); \n\
uniform sampler2D colorTexture; \n\
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
	if( gradientType == 1 ) return ComputeLinearGradient( point ); \n\
	else if( gradientType == 2 ) return ComputeRadialGradient( point ); \n\
	else if( gradientType == 3 ) return SampleGradientColor( pathOffset / pathLength ); \n\
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
	if( dashCount > 0 ) alphaBlending2 *= HandleDash( pathOffset ); \n\
	if( gradientType > 0 ) fragColor = ComputeGradient( vertexPosition, pathOffset ); \n\
	if( hasColorTexture > 0 ) fragColor = texture( colorTexture, texUV ); \n\
	float klm = abs( bezierKLM[0] ) + abs( bezierKLM[1] ) + abs( bezierKLM[2] ); \n\
	if( klm > 1E-16 ) fragColor = ComputeCubicBezier( bezierKLM, fragColor ); \n\
	//fragColor = ComputeQuadraticBezier( vec2( texcoord ), fragColor ); \n\
	fragColor.a *= alphaBlending2; \n\
	return fragColor; \n\
}";




static const char *const daox_vertex_shader2d_body =
"uniform mat4 modelMatrix; \n\
uniform mat4 viewMatrix; \n\
uniform mat4 projMatrix; \n\
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
	vertexPosition = position; \n\
	gl_Position = projMatrix * viewMatrix * modelMatrix * vec4( position, 0.0, 1.0 ); \n\
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
uniform vec3 cameraPosition;\n\
uniform mat4 projMatrix;\n\
uniform mat4 viewMatrix;\n\
uniform mat4 modelMatrix;\n\
\n\
in vec3 position;\n\
in vec3 normal;\n\
in vec3 tangent;\n\
in vec2 texCoord;\n\
in vec2 texMO;\n\
\n\
out vec2  vertexPosition; \n\
out vec4  vertexColor;\n\
out vec3  bezierKLM; \n\
out float pathOffset; \n\
\n\
out vec3 varPosition; \n\
out vec3 varNormal;  \n\
out vec3 varTangent; \n\
out vec2 varTexCoord;\n\
\n\
void main(void)\n\
{\n\
	vec3 worldPosition = vec3( modelMatrix * vec4( position, 1.0 ) );\n\
	varPosition = position;\n\
	varNormal = normal;\n\
	varTangent = tangent;\n\
	varTexCoord = texCoord;\n\
	vertexPosition = vec2( position ); \n\
	bezierKLM = vec3( texCoord, texMO[0] ); \n\
	pathOffset = texMO[1]; \n\
	gl_Position = projMatrix * viewMatrix * vec4( worldPosition, 1.0 );\n\
}\n";


static const char *const daox_fragment_shader3d_body =
"uniform int  vectorGraphics;\n\
uniform int  terrainTileType; // 0: none; 1: square; 2: hexagon; \n\
uniform int  lightCount;\n\
uniform vec4 ambientColor;\n\
uniform vec4 diffuseColor;\n\
uniform vec4 specularColor;\n\
uniform vec4 emissionColor;\n\
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
\n\
in  vec3 varPosition;\n\
in  vec3 varNormal;\n\
in  vec3 varTangent;\n\
in  vec2 varTexCoord;\n\
in  vec2 vertexPosition; \n\
in  vec3 bezierKLM; \n\
in  float pathOffset; \n\
out vec4 fragColor;\n\
\n\
vec3 worldPosition;\n\
vec4 tileTextureInfo = vec4(0.0,0.0,0.0,0.0);\n\
float tileBlendingWidth = 0.1;\n\
int hasColorTexture2 = hasColorTexture;\n\
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
vec3 Slerp3( vec3 v1, vec3 v2, float t )\n\
{\n\
	float angle = acos( dot( v1, v2 ) );\n\
	float sine = sin( angle );\n\
	float sine1 = sin( angle * (1.0 - t) );\n\
	float sine2 = sin( angle * t );\n\
	return (sine1 * v1 + sine2 * v2) / sine;\n\
}\n\
\n\
\n\
vec4 ComputeLight( vec3 lightDir, vec4 lightIntensity, vec4 texColor )\n\
{\n\
	vec3 camDir = normalize( cameraPosition - worldPosition );\n\
	vec3 normal = normalize( varNormal );\n\
	vec3 tangent = normalize( varTangent );\n\
	vec3 binormal = cross( normal, tangent );\n\
	if( hasBumpTexture > 0 ){\n\
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
		//normal2 = normalize( normal2 );\n\
		float factor = 0.5;\n\
		if( terrainTileType != 0 && tileTextureInfo.y < tileBlendingWidth ){ \n\
			factor *= tileTextureInfo.y / tileBlendingWidth;\n\
		}\n\
		normal = Slerp3( normal, normal2, factor );\n\
		lightDir = Slerp3( lightDir, lightDir2, factor );\n\
		camDir = Slerp3( camDir, camDir2, factor );\n\
	}\n\
	float cosAngIncidence = dot( normal, lightDir );\n\
	cosAngIncidence = clamp(cosAngIncidence, 0.0, 1.0);\n\
	vec3 reflection = 0.5*(1.0 + cosAngIncidence) * normal;\n\
	vec4 vertexColor = lightIntensity * texColor * cosAngIncidence;\n\
	float dotvalue = dot(reflection, camDir);\n\
	dotvalue = clamp(dotvalue, 0.0, 1.0);\n\
	vertexColor += lightIntensity * texColor;\n\
	vertexColor += lightIntensity * specularColor * dotvalue;\n\
	vertexColor += lightIntensity * ambientColor + emissionColor;\n\
	return vertexColor;\n\
}\n\
vec4 ComputeAllLights( vec4 texColor )\n\
{\n\
	vec4 vertexColor = vec4( 0.0, 0.0, 0.0, 0.0 );\n\
	for(int i=0; i<lightCount; ++i){\n\
		vec3 lightDir = normalize( lightSource[i] - worldPosition );\n\
		vertexColor += ComputeLight( lightDir, lightIntensity[i], texColor );\n\
	}\n\
	return vertexColor;\n\
}\n\
\n\
\n\
\n\
\n\
void main(void)\n\
{\n\
	vec4 texColor = diffuseColor;\n\
	worldPosition = vec3( modelMatrix * vec4( varPosition, 1.0 ) );\n\
	if( terrainTileType == 1 ) tileTextureInfo = RectLocateTex( varTexCoord );\n\
	if( terrainTileType == 2 ) tileTextureInfo = HexLocateTex( varTexCoord );\n\
	if( tileTextureCount > 0 ) hasColorTexture2 = 1;\n\
	if( hasColorTexture2 > 0 ){\n\
		texColor = texture( colorTexture, varTexCoord );\n\
		if( terrainTileType != 0 ){ \n\
			texColor = BlendTerrainTextures( texColor, varTexCoord );\n\
		}\n\
	}\n\
	fragColor = ComputeAllLights( texColor );\n\
	if( hasColorTexture2 > 0 ){\n\
		if( texColor[3] < 0.9 ) discard;\n\
		fragColor[3] = texColor[3];\n\
	}\n\
	if( lightCount == 0 ) fragColor = texColor;\n\
	//fragColor = texColor;\n\
	if( vectorGraphics > 0 ){ \n\
		vec4 color = RenderVectorGraphics( vertexPosition, bezierKLM, varTexCoord, pathOffset ); \n\
		fragColor = texColor * color; \n\
	}\n\
}\n";



void DaoxShader_CompileShader( DaoxShader *self, int type, DList *strings );

void DaoxShader_Init( DaoxShader *self )
{
	int width = 2*DAOX_MAX_GRADIENT_STOPS;
	GLfloat data[2*DAOX_MAX_GRADIENT_STOPS*4];
	GLfloat dash[DAOX_MAX_DASH];
	GLuint tid = 0;

	memset( self, 0, sizeof(DaoxShader) );
	memset( data, 0, width*4*sizeof(GLfloat) );
	memset( dash, 0, DAOX_MAX_DASH*sizeof(GLfloat) );

	self->vertexSources = DList_New( DAO_DATA_STRING );
	self->fragmentSources = DList_New( DAO_DATA_STRING );

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
void DaoxShader_Init2D( DaoxShader *self )
{
	DaoxShader_Init( self );
	self->program = glCreateProgram();

	DaoxShader_AddShader( self, GL_VERTEX_SHADER, daox_vertex_shader_header );
	DaoxShader_AppendShader( self, GL_VERTEX_SHADER, daox_vertex_shader2d_body );

	DaoxShader_AddShader( self, GL_FRAGMENT_SHADER, daox_fragment_shader_header );
	DaoxShader_AppendShader( self, GL_FRAGMENT_SHADER, daox_vector_graphics_shader_body );
	DaoxShader_AppendShader( self, GL_FRAGMENT_SHADER, daox_fragment_shader2d_body );

	DaoxShader_CompileShader( self, GL_VERTEX_SHADER, self->vertexSources );
	DaoxShader_CompileShader( self, GL_FRAGMENT_SHADER, self->fragmentSources );
}
void DaoxShader_Init3D( DaoxShader *self )
{
	DaoxShader_Init( self );
	self->program = glCreateProgram();

	DaoxShader_AddShader( self, GL_VERTEX_SHADER, daox_vertex_shader_header );
	DaoxShader_AppendShader( self, GL_VERTEX_SHADER, daox_vertex_shader3d_body );

	DaoxShader_AddShader( self, GL_FRAGMENT_SHADER, daox_fragment_shader_header );
	DaoxShader_AppendShader( self, GL_FRAGMENT_SHADER, daox_vector_graphics_shader_body );
	DaoxShader_AppendShader( self, GL_FRAGMENT_SHADER, daox_fragment_shader3d_body );

	DaoxShader_CompileShader( self, GL_VERTEX_SHADER, self->vertexSources );
	DaoxShader_CompileShader( self, GL_FRAGMENT_SHADER, self->fragmentSources );
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
		DString *log = DString_New(1);
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
	self->uniforms.hasColorTexture = glGetUniformLocation(self->program, "hasColorTexture");
	self->uniforms.hasBumpTexture = glGetUniformLocation(self->program, "hasBumpTexture");
	self->uniforms.colorTexture = glGetUniformLocation(self->program, "colorTexture");
	self->uniforms.bumpTexture = glGetUniformLocation(self->program, "bumpTexture");
	self->attributes.position = glGetAttribLocation(self->program, "position");
	self->attributes.texKLMO = glGetAttribLocation(self->program, "texKLMO");
	printf( "DaoxShader_Finalize: %i\n", self->attributes.position );
	printf( "DaoxShader_Finalize: %i\n", self->attributes.texKLMO );
}
void DaoxShader_Finalize3D( DaoxShader *self )
{
	DaoxShader_Finalize( self );
	if( self->program == 0 ) return;
	DaoxShader_GetVectorGraphicsUniforms( self );
	self->uniforms.vectorGraphics = glGetUniformLocation(self->program, "vectorGraphics");
	self->uniforms.projMatrix = glGetUniformLocation(self->program, "projMatrix");
	self->uniforms.viewMatrix = glGetUniformLocation(self->program, "viewMatrix");
	self->uniforms.modelMatrix = glGetUniformLocation(self->program, "modelMatrix");
	self->uniforms.cameraPosition = glGetUniformLocation(self->program, "cameraPosition");
	self->uniforms.lightCount = glGetUniformLocation(self->program, "lightCount");
	self->uniforms.lightSource = glGetUniformLocation(self->program, "lightSource");
	self->uniforms.lightIntensity = glGetUniformLocation(self->program, "lightIntensity");
	self->uniforms.ambientColor = glGetUniformLocation(self->program, "ambientColor");
	self->uniforms.diffuseColor = glGetUniformLocation(self->program, "diffuseColor");
	self->uniforms.specularColor = glGetUniformLocation(self->program, "specularColor");
	self->uniforms.emissionColor = glGetUniformLocation(self->program, "emissionColor");
	self->uniforms.hasColorTexture = glGetUniformLocation(self->program, "hasColorTexture");
	self->uniforms.hasBumpTexture = glGetUniformLocation(self->program, "hasBumpTexture");
	self->uniforms.colorTexture = glGetUniformLocation(self->program, "colorTexture");
	self->uniforms.bumpTexture = glGetUniformLocation(self->program, "bumpTexture");
	self->uniforms.terrainTileType = glGetUniformLocation(self->program, "terrainTileType");
	self->uniforms.tileTextureCount = glGetUniformLocation(self->program, "tileTextureCount");
	self->uniforms.tileTextureScale = glGetUniformLocation(self->program, "tileTextureScale");
	self->uniforms.tileTextures[0] = glGetUniformLocation(self->program, "tileTexture1");
	self->uniforms.tileTextures[1] = glGetUniformLocation(self->program, "tileTexture2");
	self->uniforms.tileTextures[2] = glGetUniformLocation(self->program, "tileTexture3");
	self->uniforms.tileTextures[3] = glGetUniformLocation(self->program, "tileTexture4");
	self->uniforms.tileTextures[4] = glGetUniformLocation(self->program, "tileTexture5");
	self->uniforms.tileTextures[5] = glGetUniformLocation(self->program, "tileTexture6");

	//self->uniforms.material = glGetUniformBlockIndex(self->program, "material");
	self->attributes.position = glGetAttribLocation(self->program, "position");
	self->attributes.normal = glGetAttribLocation(self->program, "normal");
	self->attributes.tangent = glGetAttribLocation(self->program, "tangent");
	self->attributes.texCoord = glGetAttribLocation(self->program, "texCoord");
	self->attributes.texMO = glGetAttribLocation(self->program, "texMO");
	printf( "DaoxShader_Finalize: %i\n", self->attributes.position );
	printf( "DaoxShader_Finalize: %i\n", self->uniforms.projMatrix );
	printf( "DaoxShader_Finalize: %i\n", self->uniforms.viewMatrix );
}
void DaoxShader_Free( DaoxShader *self )
{
	if( self->vertexShader ) glDeleteShader( self->vertexShader );
	if( self->fragmentShader ) glDeleteShader( self->fragmentShader );
	if( self->program ) glDeleteShader( self->program );
	if( self->vertexSources ) DList_Delete( self->vertexSources );
	if( self->fragmentSources ) DList_Delete( self->fragmentSources );
	self->vertexSources = NULL;
	self->fragmentSources = NULL;
	self->vertexShader = 0;
	self->fragmentShader = 0;
	self->program = 0;
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
			DString_Append( self->vertexSources->items.pString[self->vertexSources->size-1], source );
		}else{
			DList_Append( self->vertexSources, source );
		}
		break;
	case GL_FRAGMENT_SHADER :
		if( self->fragmentSources->size ){
			DString_Append( self->fragmentSources->items.pString[self->fragmentSources->size-1], source );
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
		if( i == 0 ) sources[i] += 2; /* skip //; */
	}

	glShaderSource( shader, n, sources, NULL );
	glCompileShader( shader );
	dao_free( sources );

	glGetShaderiv(shader, GL_COMPILE_STATUS, &shader_ok);
	if( !shader_ok ){
		const char *log2;
		DString *log = DString_New(1);
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

	n = brush->dash;
	if( n > DAOX_MAX_DASH ) n = DAOX_MAX_DASH;
	memset( dash, 0, n*sizeof(GLfloat) );
	for(i=0; i<n; ++i) dash[i] = brush->dashPattern[i];

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

void DaoxBuffer_Init( DaoxBuffer *self )
{
	memset( self, 0, sizeof(DaoxBuffer) );
	self->vertexCapacity = 16*1024;
	self->triangleCapacity = 16*1024;
}
void DaoxBuffer_InitBuffers( DaoxBuffer *self );
void DaoxBuffer_Init2D( DaoxBuffer *self, int pos, int klmo )
{
	DaoGLVertex2D *vertex = NULL;
	DaoxBuffer_Init( self );

	self->traitCount = 2;
	self->vertexSize = sizeof(DaoGLVertex2D);
	self->triangleSize = sizeof(DaoGLTriangle);
	self->traits[0].uniform = pos;
	self->traits[1].uniform = klmo;
	self->traits[0].count = 2;
	self->traits[1].count = 4;
	self->traits[0].offset = NULL;
	self->traits[1].offset = (void*) & vertex->texKLMO;

	DaoxBuffer_InitBuffers( self );
}
void DaoxBuffer_Init3D( DaoxBuffer *self, int pos, int norm, int tan, int texuv, int texmo )
{
	DaoGLVertex3D *vertex = NULL;
	DaoxBuffer_Init( self );

	self->traitCount = 5;
	self->vertexSize = sizeof(DaoGLVertex3D);
	self->triangleSize = sizeof(DaoGLTriangle);
	self->traits[0].uniform = pos;
	self->traits[1].uniform = norm;
	self->traits[2].uniform = tan;
	self->traits[3].uniform = texuv;
	self->traits[4].uniform = texmo;
	self->traits[0].count = 3;
	self->traits[1].count = 3;
	self->traits[2].count = 3;
	self->traits[3].count = 2;
	self->traits[4].count = 2;
	self->traits[0].offset = NULL;
	self->traits[1].offset = (void*) & vertex->norm;
	self->traits[2].offset = (void*) & vertex->tan;
	self->traits[3].offset = (void*) & vertex->tex;
	self->traits[4].offset = (void*) & vertex->tex;

	DaoxBuffer_InitBuffers( self );
}
void DaoxBuffer_Init3DVG( DaoxBuffer *self, int pos, int norm, int texuv, int texmo )
{
	DaoGLVertex3DVG *vertex = NULL;
	DaoxBuffer_Init( self );

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

	DaoxBuffer_InitBuffers( self );
}
void DaoxBuffer_Free( DaoxBuffer *self )
{
	// TODO
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
void DaoxBuffer_InitBuffers( DaoxBuffer *self )
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
		if( (self->vertexOffset + count) > self->vertexCapacity ) self->vertexCapacity = 1.2 * count;
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
		if( (self->triangleOffset + count) > self->triangleCapacity ) self->triangleCapacity = 1.2 * count;
		glBufferData( GL_ELEMENT_ARRAY_BUFFER, self->triangleCapacity*self->triangleSize, NULL, GL_STREAM_DRAW );
		self->triangleOffset = 0;
	}
	return (DaoGLTriangle*) glMapBufferRange( GL_ELEMENT_ARRAY_BUFFER, self->triangleOffset*self->triangleSize, dataSize, GL_MAP_WRITE_BIT|GL_MAP_UNSYNCHRONIZED_BIT|GL_MAP_INVALIDATE_BUFFER_BIT );
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
