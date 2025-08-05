#version 400 core

in vec3 vColour;			// Interpolated colour using colour calculated in the vertex shader
in vec2 vTexCoord;			// Interpolated texture coordinate using texture coordinate from the vertex shader
in float fogDepth;          // Recieve fog depth from vertex shader

out vec4 vOutputColour;		// The output colour

uniform sampler2D sampler0;  // The texture sampler
uniform samplerCube CubeMapTex;
uniform bool bUseTexture;    // A flag indicating if texture-mapping should be applied
uniform bool renderSkybox;
in vec3 worldPosition;

// Fog uniforms
uniform bool fogEnabled = false;
uniform vec3 fogColor = vec3(0.5, 0.5, 0.5);
uniform float fogDensity = 0.002;

void main()
{


	if (renderSkybox) {
		vOutputColour = texture(CubeMapTex, worldPosition);

	} else {

		// Get the texel colour from the texture sampler
		vec4 vTexColour = texture(sampler0, vTexCoord);	

		if (bUseTexture)
			vOutputColour = vTexColour*vec4(vColour, 1.0f);	// Combine object colour and texture 
		else
			vOutputColour = vec4(vColour, 1.0f);	// Just use the colour instead
	}

	if (fogEnabled) {
			float fogFactor = exp(-fogDensity * fogDensity * fogDepth * fogDepth);
            fogFactor = clamp(fogFactor, 0.0, 1.0);
            vOutputColour = mix(vec4(fogColor, 1.0), vOutputColour, fogFactor);
	}
	
}
