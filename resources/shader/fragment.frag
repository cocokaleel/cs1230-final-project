#version 330 core
// UV coordinate in variable representing a pixel in the full-screen quad
in vec2 uvCoords;
out vec4 fragColor;// output color, which goes into the openGL clipping situation

//This main function raytraces a ray through the UV coordinate within a triangle on a full-screen quad. It raytraces only a single ray.
void main() {
    //Initialize fragColor
    fragColor = vec4(vec3(0.f),1.f);

    fragColor = vec4(uvCoords[1], 0.f, 0.f, 1.f);
}

