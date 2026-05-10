#pragma once
#include "base/Singleton.h"
#include <map>
#include <memory>
#include <string>

namespace CS
{
class Shader;
class BuiltInShaders : public Singleton<BuiltInShaders>
{
public:
    std::shared_ptr<Shader> GetVertexShader(std::string_view name);
    std::shared_ptr<Shader> GetFragmentShader(std::string_view name);

private:
    friend class Singleton<BuiltInShaders>;
    BuiltInShaders();
    void createPanoramicSkyShader();
    void createPBRShader();
    void createShadowMapShader();
    void createInfiniteGrid3DShader();

private:
    std::map<std::string, std::shared_ptr<Shader>> m_vertexShaders;
    std::map<std::string, std::shared_ptr<Shader>> m_fragmentShaders;
};

} // namespace CS
