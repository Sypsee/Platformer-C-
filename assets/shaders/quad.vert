#version 430 core


struct Transform
{
    ivec2 atlasOffset;
    ivec2 spriteSize;
    vec2 pos;
    vec2 size;
};

layout (std430, binding = 0) buffer TransformSBO
{
    Transform transforms[];
};

layout (location = 0) out vec2 textureCoordsOut;


uniform vec2 screenSize;
uniform mat4 orthoProjection;


void main()
{
    Transform transform = transforms[gl_InstanceID];

    vec2 vertices[6] =
    {
        transform.pos,
        vec2(transform.pos + vec2(0.0, transform.size.y)),
        vec2(transform.pos + vec2(transform.size.x, 0.0)),
        vec2(transform.pos + vec2(transform.size.x, 0.0)),
        vec2(transform.pos + vec2(0.0, transform.size.y)),
        transform.pos + transform.size
    };
    
    float left = transform.atlasOffset.x;
    float top = transform.atlasOffset.y;
    float right = transform.atlasOffset.x + transform.spriteSize.x;
    float bottom = transform.atlasOffset.y + transform.spriteSize.y;

    vec2 texCoords[6] =
    {
        vec2(left, top),
        vec2(left, bottom),
        vec2(right, top),
        vec2(right, top),
        vec2(left, bottom),
        vec2(right, bottom)
    };

    // Normalize Position
    {
        vec2 vertexPos = vertices[gl_VertexID];
        // vertexPos.y = -vertexPos.y + screenSize.y;
        // vertexPos = 2.0 * (vertexPos / screenSize) - 1.0;
        gl_Position = orthoProjection * vec4(vertexPos, 0.0, 1.0);
    }

    textureCoordsOut = texCoords[gl_VertexID];
}