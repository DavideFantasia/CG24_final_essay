#pragma once

#include <string>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>  
#include <glm/gtx/string_cast.hpp>

#include "..\common\texture.h"

class Material {
public:
    std::string name;

    bool has_heightmap;

    glm::vec3 diffuse_factor;
    float metallic_factor;
    float roughness_factor;
    float ao_factor;

    GLint diffuse_map;
    bool has_diffuse_map;

    GLint metallic_map;
    bool has_metallic_map;

    GLint roughness_map;
    bool has_roughness_map;

    GLint emissive_map;
    bool has_emissive_map;

    GLint normal_map;
    bool has_normal_map;

    GLint ao_map;
    bool has_ao_map;

    Material(const std::string& material_name)
        : name(material_name), has_heightmap(false), diffuse_factor(1.0f), metallic_factor(0.5f),
        roughness_factor(0.5f), diffuse_map(-1), has_diffuse_map(false), metallic_map(-1),
        has_metallic_map(false), roughness_map(-1), has_emissive_map(false), emissive_map(-1) ,has_roughness_map(false), normal_map(-1),
        has_normal_map(false), ao_factor(0.05f), ao_map(-1), has_ao_map(false) {}

    virtual void set_shader_uniforms(GLuint shader_program) const {
        if (has_heightmap) {
            glUniform1i(glGetUniformLocation(shader_program, "has_heightmap"), 1);
        }else
            glUniform1i(glGetUniformLocation(shader_program, "has_heightmap"), 0);


        if(has_diffuse_map){
            glUniform1i(glGetUniformLocation(shader_program, "material.diffuse_map"), diffuse_map);
            glUniform1i(glGetUniformLocation(shader_program, "material.has_diffuse_map"), 1);
        }else{
            glUniform3fv(glGetUniformLocation(shader_program, "material.diffuse_factor"), 1, glm::value_ptr(diffuse_factor));
            glUniform1i(glGetUniformLocation(shader_program, "material.has_diffuse_map"), 0);
        }

        if (has_metallic_map) {
            glUniform1i(glGetUniformLocation(shader_program, "material.metallic_map"), metallic_map);
            glUniform1i(glGetUniformLocation(shader_program, "material.has_metallic_map"), 1);
        }else{
            glUniform1f(glGetUniformLocation(shader_program, "material.metallic_factor"), metallic_factor);
            glUniform1i(glGetUniformLocation(shader_program, "material.has_metallic_map"), 0);
        }

        if (has_roughness_map) {
            glUniform1i(glGetUniformLocation(shader_program, "material.roughness_map"), roughness_map);
            glUniform1i(glGetUniformLocation(shader_program, "material.has_roughness_map"), 1);
        }
        else {
            glUniform1f(glGetUniformLocation(shader_program, "material.roughness_factor"), roughness_factor);
            glUniform1i(glGetUniformLocation(shader_program, "material.has_roughness_map"), 0);
        }

        if (has_emissive_map) {
            glUniform1i(glGetUniformLocation(shader_program, "material.emissive_map"), emissive_map);
            glUniform1i(glGetUniformLocation(shader_program, "material.has_emissive_map"), 1);
        }else{
            glUniform1i(glGetUniformLocation(shader_program, "material.has_emissive_map"), 0);
        }
        
        if (has_ao_map) {
            glUniform1i(glGetUniformLocation(shader_program, "material.ao_map"), ao_map);
            glUniform1f(glGetUniformLocation(shader_program, "material.ao_factor"), ao_factor);
            glUniform1i(glGetUniformLocation(shader_program, "material.has_ao_map"), 1);      
        }else {
            glUniform1f(glGetUniformLocation(shader_program, "material.ao_factor"), ao_factor);
            glUniform1i(glGetUniformLocation(shader_program, "material.has_ao_map"), 0);
        }

        if (has_normal_map){
            glUniform1i(glGetUniformLocation(shader_program, "material.normal_map"), normal_map);
            glUniform1i(glGetUniformLocation(shader_program, "material.has_normal_map"), 1);
        }else{
            glUniform1i(glGetUniformLocation(shader_program, "material.has_normal_map"), 0);
        }
    }

    virtual ~Material() {}
};

class SandTerrainMaterial : public Material {
public:
    SandTerrainMaterial(): Material("sand_terrain") {
        diffuse_factor = glm::vec3(0.94f, 0.80f, 0.49f);
        metallic_factor = 0.f;
        roughness_factor = 1.f;
        ao_factor = 0.05f;

        has_diffuse_map = true;
        has_heightmap = true;
        diffuse_map = 1; // ID della diffuse map

        has_ao_map = false;
        has_normal_map = false;
        has_metallic_map = false;
        has_roughness_map = false;
    }

    void set_shader_uniforms(GLuint shader_program) const override {
        Material::set_shader_uniforms(shader_program);
    }
};

class BustMaterial : public Material {
public:
    BustMaterial() : Material("bust_material") {}

    void init(GLint _diffuse_map, GLint _roughness_map, GLint _metallic_map, GLint _normal_map) {
        has_diffuse_map = true;
        diffuse_map = _diffuse_map;

        has_roughness_map = true;
        roughness_map = _roughness_map;

        has_metallic_map = true;
        metallic_map = _metallic_map;

        has_normal_map = true;
        normal_map = _normal_map;

        ao_factor = 0.05f;
    }
    void set_shader_uniforms(GLuint shader_program) const override {
        Material::set_shader_uniforms(shader_program);
    }
};

class LampMaterial : public Material {
public:
    LampMaterial() : Material("lamp_material") {}

    void init(GLint _diffuse_map, GLint _roughness_map, GLint _metallic_map, GLint _normal_map, GLint _emissive_map, GLint _ao_map) {
        has_diffuse_map = true;
        diffuse_map = _diffuse_map;
        diffuse_factor = glm::vec3(0.67f, 0.76f, 0.94f);

        has_roughness_map = true;
        roughness_map = _roughness_map;
        roughness_factor = 0.5f;

        has_metallic_map = true;
        metallic_map = _metallic_map;
        metallic_factor = 0.90f;

        has_normal_map = true;
        normal_map = _normal_map;

        has_emissive_map = true;
        emissive_map = _emissive_map;

        has_ao_map = true;
        ao_map = _ao_map;
        ao_factor = 0.005f;
    }
    void set_shader_uniforms(GLuint shader_program) const override {
        Material::set_shader_uniforms(shader_program);
    }
};