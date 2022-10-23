#version 330 core
 
out vec4 color;

void main() {
    vec3 result = vec3(0);
    result = vec3(0.9, 0.9, 0);
    color = vec4(result, 1);
}
 