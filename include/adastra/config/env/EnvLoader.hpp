#ifndef ENV_LOADER_HPP
#define ENV_LOADER_HPP

#include <string>
#include <cstdlib>
#include <stdexcept>
#include <sstream>
#include <algorithm>
#include <fstream>

namespace adastra::config::env
{
    class EnvLoader
    {
    public:
        // Récupère une variable requise ou lance une exception
        static std::string require(const std::string &key)
        {
            const char *val = std::getenv(key.c_str());
            if (!val)
            {
                throw std::runtime_error("La variable d'environnement '" + key + "' est requise mais non définie.");
            }
            return std::string(val);
        }

        // Récupère une variable avec une valeur par défaut si elle n'existe pas
        static std::string get(const std::string &key, const std::string &defaultValue = "")
        {
            const char *val = std::getenv(key.c_str());
            return val ? std::string(val) : defaultValue;
        }

        // Version typée pour int
        static int getInt(const std::string &key, int defaultValue = 0)
        {
            const char *val = std::getenv(key.c_str());
            if (!val)
                return defaultValue;

            return std::stoi(val);
        }

        // Version typée pour bool (1, true, oui → true)
        static bool getBool(const std::string &key, bool defaultValue = false)
        {
            const char *val = std::getenv(key.c_str());
            if (!val)
                return defaultValue;

            std::string s = val;
            std::transform(s.begin(), s.end(), s.begin(), ::tolower);
            return (s == "1" || s == "true" || s == "yes" || s == "oui");
        }

        static void loadDotenv(const std::string &file)
        {
#ifdef _WIN32
            auto set_env = [](const std::string &k, const std::string &v)
            { _putenv_s(k.c_str(), v.c_str()); };
#else
            auto set_env = [](const std::string &k, const std::string &v)
            { setenv(k.c_str(), v.c_str(), 0); };
#endif
            std::ifstream in(file);
            if (!in.is_open())
                return;
            std::string line;
            while (std::getline(in, line))
            {
                if (line.empty() || line[0] == '#')
                    continue;
                auto pos = line.find('=');
                if (pos == std::string::npos)
                    continue;
                std::string key = line.substr(0, pos);
                std::string val = line.substr(pos + 1);
                // trim simple
                key.erase(0, key.find_first_not_of(" \t"));
                key.erase(key.find_last_not_of(" \t") + 1);
                val.erase(0, val.find_first_not_of(" \t"));
                val.erase(val.find_last_not_of(" \t") + 1);
                if (!key.empty() && std::getenv(key.c_str()) == nullptr)
                    set_env(key, val);
            }
        }
    };
}

#endif // ENV_LOADER_HPP
