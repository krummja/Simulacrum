#ifndef SIMULACRUM_SCRIPT_MANAGER_HPP_
#define SIMULACRUM_SCRIPT_MANAGER_HPP_

// #include <LuaCpp.hpp>

namespace Simulacrum
{
  class ScriptManager
  {
  public:
    static ScriptManager& Instance()
    {
      static ScriptManager instance;
      return instance;
    }

    void execute(std::string code)
    {
      // lua_.CompileStringAndRun(code);
    }

  private:
    ScriptManager() = default;
    ~ScriptManager() = default;

    // No copy and no assignment
    ScriptManager(const ScriptManager&) = delete;
    ScriptManager& operator=(const ScriptManager&) = delete;

    // LuaCpp::LuaContext lua_;
  };
}

#endif