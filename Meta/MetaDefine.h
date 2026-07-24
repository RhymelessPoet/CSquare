#pragma once

#if defined(CS_ENABLE_META)
#define Meta(...) clang::annotate("Meta("#__VA_ARGS__")")
#define Property(...) clang::annotate("Property("#__VA_ARGS__")")
#define Method(...) clang::annotate("Method("#__VA_ARGS__")")
#else
#define Meta(...)
#define Property(...)
#define Method(...)
#endif