#include "ConfigParser.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <stdexcept>

static std::string trim(const std::string& str) {
    const std::string whitespace = " 	";
    const auto strBegin = str.find_first_not_of(whitespace);
    if (strBegin == std::string::npos) return ""; // no content

    const auto strEnd = str.find_last_not_of(whitespace);
    const auto strRange = strEnd - strBegin + 1;

    return str.substr(strBegin, strRange);
}


AppConfig ConfigParser::parse(const std::string& filename) {
    AppConfig config;
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Error: Could not open config file: " + filename);
    }

    std::string line;
    int lineNum = 0;
    ObjectConfig currentObject;
    bool inObjectBlock = false;

    while (std::getline(file, line)) {
        lineNum++;
        line = trim(line);

        if (line.empty() || line[0] == '#') {
            continue;
        }

        if (line == "object {") {
            if (inObjectBlock) {
                throw std::runtime_error("Error: Nested 'object {' block found at line " + std::to_string(lineNum));
            }
            inObjectBlock = true;
            currentObject = ObjectConfig();
            continue;
        }

        if (line == "}") {
            if (!inObjectBlock) {
                throw std::runtime_error("Error: Unexpected '}' found at line " + std::to_string(lineNum));
            }
            config.objects.push_back(currentObject);
            inObjectBlock = false;
            continue;
        }

        std::stringstream ss(line);
        std::string key;
        ss >> key;

        if (inObjectBlock) {
            if (key == "path") ss >> currentObject.path;
            else if (key == "texture") ss >> currentObject.texture;
            else if (key == "position") ss >> currentObject.position.x >> currentObject.position.y >> currentObject.position.z;
            else if (key == "rotation") ss >> currentObject.rotation.x >> currentObject.rotation.y >> currentObject.rotation.z;
            else if (key == "scale") ss >> currentObject.scale;
        } else {
            if (key == "window_width") ss >> config.window_width;
            else if (key == "window_height") ss >> config.window_height;
            else if (key == "window_title") { std::getline(ss, config.window_title); config.window_title = trim(config.window_title); }
            else if (key == "skybox_path") ss >> config.skybox_path;
            else if (key == "camera_speed") ss >> config.camera_speed;
            else if (key == "shader_vertex_path") ss >> config.shader_vertex_path;
            else if (key == "shader_fragment_path") ss >> config.shader_fragment_path;
        }
    }

    if (inObjectBlock) {
        throw std::runtime_error("Error: Unterminated 'object {' block at end of file.");
    }

    std::cout << "Config file '" << filename << "' loaded successfully." << std::endl;
    std::cout << "- Window: " << config.window_width << "x" << config.window_height << " '" << config.window_title << "'" << std::endl;
    std::cout << "- Skybox Path: " << config.skybox_path << std::endl;
    std::cout << "- Found " << config.objects.size() << " objects." << std::endl;


    return config;
}
