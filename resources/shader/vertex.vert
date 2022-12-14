#version 330 core
//Adapted from lab 10

//information laid out from the fullscreen quad VAO
layout (location = 0) in vec3 position;
layout (location = 1) in vec2 uv;

// a variable to send uv coordinates to the frag shader
out vec2 uvCoords;

void main() {
    // Task 16: assign the UV layout variable to the UV "out" variable
    uvCoords = uv;

    gl_Position = vec4(position, 1.0);
}
