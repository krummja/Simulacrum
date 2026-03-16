#ifdef Simulacrum_EXPORT
#define Simulacrum_API __declspec(dllexport)
#else
#define Simulacrum_API __declspec(dllimport)
#endif