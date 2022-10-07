#version 330 core

in vec2 outTexCoor;
//
in vec3 vecPos;
//
in vec3 normal_f;

out vec4 color;

//
uniform vec3 u_lightPos;

// ka
uniform vec3 ka;
uniform vec3 ambIntensity;
// kd
uniform sampler2D image0;
uniform vec3 lightIntensity;
// ks
uniform vec3 ks;

void main() {
    vec3 tex_color = texture(image0, outTexCoor).xyz;
    // 对于 ambient 要不要用 白色代替纹理颜色？
    vec3 amb = ka * tex_color * ambIntensity;
    vec3 lightDir = u_lightPos - vecPos;
    vec3 lightDir_n = normalize(lightDir);
    
    float cos = dot(lightDir_n, normal_f);
    if (cos <= 0) {
        color = vec4(amb, 1);
        return;
    }
    
    float r2 = dot(lightDir, lightDir);
    vec3 diffuse = tex_color * lightIntensity * cos / r2;
        
    vec3 eye_dir_n = normalize(-vecPos);
    vec3 specular = vec3(0, 0, 0);

    // 使用半程而不是反射（Blinn 区别）
    cos = dot(normalize(normalize(-vecPos) + lightDir_n), normal_f);
    
    // 眼睛从背面看，不应该看到高光；如果也不给看 amb、diffuse，那就是不让看背面
    if (dot(eye_dir_n, normal_f) > 0 && cos > 0) {
        // 150 是反光度，shininess
        // 对于 specular 要不要用白色代替纹理颜色？
        specular = ks * lightIntensity * pow(cos, 150) / r2;
    }
    color = vec4(amb + diffuse + specular, 1);
}
