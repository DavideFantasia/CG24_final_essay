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

    glm::vec3 diffuse_factor;
    float metallic_factor;
    float roughness_factor;

    GLint diffuse_map;
    bool has_diffuse_map;

    GLint metallic_map;
    bool has_metallic_map;

    GLint roughness_map;
    bool has_roughness_map;

    GLint normal_map;
    bool has_normal_map;

    float ao;

    Material(const std::string& material_name)
        : name(material_name), diffuse_factor(1.0f), metallic_factor(0.5f),
          roughness_factor(0.5f), diffuse_map(-1), has_diffuse_map(false), metallic_map(-1),
          has_metallic_map(false), roughness_map(-1), has_roughness_map(false), normal_map(-1),
          has_normal_map(false), ao(1.0f) {}

    virtual void set_shader_uniforms(GLuint shader_program) const {
        
        glUniform1f(glGetUniformLocation(shader_program, "material.metallic_factor"), metallic_factor);
        glUniform1f(glGetUniformLocation(shader_program, "material.roughness_factor"), roughness_factor);
        glUniform1f(glGetUniformLocation(shader_program, "material.ao"), ao);

        if(has_diffuse_map){
            glUniform1i(glGetUniformLocation(shader_program, "material.diffuse_map"), diffuse_map);
            glUniform1i(glGetUniformLocation(shader_program, "material.has_diffuse_map"), 1);
        }else{
            glUniform3fv(glGetUniformLocation(shader_program, "material.diffuse_factor"), 1, glm::value_ptr(diffuse_factor));
            glUniform1i(glGetUniformLocation(shader_program, "material.has_diffuse_map"), 0);
        }

        if (has_metallic_map) {
            glUniform1i(glGetUniformLocation(shader_program, "material.has_metallic_map"), 1);
            glUniform1i(glGetUniformLocation(shader_program, "material.metallic_map"), metallic_map);
        }else{
            glUniform1f(glGetUniformLocation(shader_program, "material.metallic_factor"), metallic_factor);
            glUniform1i(glGetUniformLocation(shader_program, "material.has_metallic_map"), 0);
        }

        if (has_roughness_map) {
            glUniform1i(glGetUniformLocation(shader_program, "material.has_roughness_map"), 1);
            glUniform1i(glGetUniformLocation(shader_program, "material.roughness_map"), roughness_map);
        }
        else {
            glUniform1f(glGetUniformLocation(shader_program, "material.roughness_factor"), roughness_factor);
            glUniform1i(glGetUniformLocation(shader_program, "material.has_roughness_map"), 0);
        }

        if (has_normal_map){
            glUniform1i(glGetUniformLocation(shader_program, "material.has_normal_map"), 1);
            glUniform1i(glGetUniformLocation(shader_program, "material.normal_map"), normal_map);
        }
    }

    virtual ~Material() {}
};

class SandTerrainMaterial : public Material {
public:
    SandTerrainMaterial(): Material("sand_terrain") {
        diffuse_factor = glm::vec3(0.94f, 0.80f, 0.49f);
        metallic_factor = 0.005;
        roughness_factor = 0.95f;
        ao = 0.05f;

        diffuse_map = 1; // ID della diffuse map
        has_diffuse_map = true;

        //passaggio info per il materiale
        /*
        glUniform1i(glGetUniformLocation(heightmap_shader.program, "material.diffuse_map"), 1);
        glUniform1i(glGetUniformLocation(heightmap_shader.program, "material.has_diffuse_map"), 1);
        glUniform3fv(glGetUniformLocation(heightmap_shader.program, "material.specular"), 1, glm::value_ptr(glm::vec3(0.94f, 0.80f, 0.49f)));
        glUniform1i(glGetUniformLocation(heightmap_shader.program, "material.has_normal_map"), 0);
        glUniform1f(glGetUniformLocation(heightmap_shader.program, "material.metallic"), 0.75);
        glUniform1i(glGetUniformLocation(heightmap_shader.program, "material.has_metallic_map"), 0);
        glUniform1f(glGetUniformLocation(heightmap_shader.program, "material.roughness"), 0.5f);
        glUniform1i(glGetUniformLocation(heightmap_shader.program, "material.has_roughness_map"), 0);
        glUniform1f(glGetUniformLocation(heightmap_shader.program, "material.ao"), 0.05f);*/
    }

    void set_shader_uniforms(GLuint shader_program) const override {
        Material::set_shader_uniforms(shader_program);
        /*
        glUniform1i(glGetUniformLocation(shader_program, "material.diffuse_map"), 1);
        glUniform1i(glGetUniformLocation(shader_program, "material.has_diffuse_map"), 1);
        glUniform1i(glGetUniformLocation(shader_program, "material.has_normal_map"), 0);
        glUniform1f(glGetUniformLocation(shader_program, "material.metallic"), 0.75);
        glUniform1i(glGetUniformLocation(shader_program, "material.has_metallic_map"), 0);
        glUniform1f(glGetUniformLocation(shader_program, "material.roughness"), 0.5f);
        glUniform1i(glGetUniformLocation(shader_program, "material.has_roughness_map"), 0);
        glUniform1f(glGetUniformLocation(shader_program, "material.ao"), 0.05f);*/
    }
};