// Used only with clang atm.

#if defined(__has_feature)
#if !__has_feature(cxx_auto_type)
#error TYPE INFERENCE SUPPORT MISSING
#endif // !__has_feature(cxx_auto_type)
#endif // defined(__has_feature)