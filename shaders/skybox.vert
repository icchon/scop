#version 330 core
layout (location = 0) in vec3 aPos;

out vec3 TexCoords;

uniform mat4 projection;
uniform mat4 view;

void main()
{
    TexCoords = aPos;
    // ビュー行列から平行移動成分を除去
    mat4 view_no_translation = mat4(mat3(view));
    vec4 pos = projection * view_no_translation * vec4(aPos, 1.0);
    // スカイボックスが常に最も奥に描画されるようにする
    gl_Position = pos.xyww;
}
