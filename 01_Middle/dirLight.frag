#version 130

// pipeline-b�l bej�v� per-fragment attrib�tumok
in vec3 vs_out_pos;
in vec3 vs_out_normal;
in vec2 vs_out_tex0;

// kimen� �rt�k - a fragment sz�ne
out vec4 fs_out_col;

//
// uniform v�ltoz�k
//

// sz�nt�r tulajdons�gok
uniform vec3 eye_pos = vec3(0, 15, 15);

// f�nytulajdons�gok
uniform vec3 light_pos = vec3( 0, 5, 0 );
uniform vec4 La = vec4(0.1f, 0.1f, 0.1f, 1);
uniform vec4 Ld = vec4(0.5f, 0.75f, 0.9f, 1);
uniform vec4 Ls = vec4(1, 1, 1, 1);

// anyagtulajdons�gok
uniform vec4 Ka = vec4(1, 1, 1, 1);
uniform vec4 Kd = vec4(0.4f, 0.2f, 0.7f, 1);
uniform vec4 Ks = vec4(0, 0.75, 0.9, 1);
uniform float specular_power = 16;
uniform sampler2D texImage;

uniform float t;
uniform vec4 fromColor = vec4(0, 1, 0, 1);
uniform vec4 toColor = vec4(0, 0, 1, 1);

void main()
{
	//
	// ambiens sz�n sz�m�t�sa
	//
	vec4 ambient = La * Ka;

	//
	// diff�z sz�n sz�m�t�sa
	//

	/* seg�ts�g:
		- normaliz�l�s: http://www.opengl.org/sdk/docs/manglsl/xhtml/normalize.xml
	    - skal�ris szorzat: http://www.opengl.org/sdk/docs/manglsl/xhtml/dot.xml
	    - clamp: http://www.opengl.org/sdk/docs/manglsl/xhtml/clamp.xml
	*/
	vec3 normal = normalize( vs_out_normal );
	vec3 toLight = normalize(light_pos - vs_out_pos);
	float di = clamp( dot( toLight, normal), 0.0f, 1.0f );
	vec4 diffuse = Ld*Kd*di;

	//
	// f�nyfoltk�pz� sz�n
	//

	/* seg�ts�g:
		- reflect: http://www.opengl.org/sdk/docs/manglsl/xhtml/reflect.xml
		- power: http://www.opengl.org/sdk/docs/manglsl/xhtml/pow.xml
	*/
	vec4 specular = vec4(0);

	if ( di > 0 )
	{
		vec3 e = normalize( eye_pos - vs_out_pos );
		vec3 r = reflect( -toLight, normal );
		float si = pow( clamp( dot(e, r), 0.0f, 1.0f ), specular_power );
		specular = Ls*Ks*si;
	}

	vec4 texel = texture(texImage, vs_out_tex0.st);
	if (texel.x > 0.9 && texel.y > 0.9 && texel.z > 0.9) {
		fs_out_col = (1-t) * fromColor + t * toColor;
	} else {
		fs_out_col = (ambient + diffuse + specular ) * texture(texImage, vs_out_tex0.st);
	}
}