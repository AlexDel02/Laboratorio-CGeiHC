#version 330

in vec2 TexCoord;

out vec4 outColor;

uniform sampler2D theTexture;
uniform vec3 color;        // tono del humo
uniform float opacidad;    // 0 = invisible, 1 = opaco (cambia con la edad de la particula)

void main()
{
	vec4 t = texture(theTexture, TexCoord);
	outColor = vec4(color * t.rgb, t.a * opacidad);
}
