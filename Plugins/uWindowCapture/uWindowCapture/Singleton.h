#pragma once

#define UWC_SINGLETON(ClassName) \
    private:\
        ClassName() = default;\
        ~ClassName() = default;\
    public:\
        ClassName(const ClassName&) = delete;\
        ClassName& operator=(const ClassName&) = delete;\
        ClassName(ClassName&&) = delete;\
        ClassName& operator=(ClassName&&) = delete;\
        \
        static bool IsNull()\
        {\
            return instance == nullptr;\
        }\
        \
        static ClassName& Get()\
        {\
            return *instance;\
        }\
        \
        static void Create()\
        {\
            if (!instance) instance = new ClassName;\
        }\
        \
        static void Destroy()\
        {\
            if (instance)\
            {\
                delete instance;\
                instance = nullptr;\
            }\
        }\
    private:\
        static ClassName* instance;


#define UWC_SINGLETON_INSTANCE(ClassName) \
    ClassName* ClassName::instance = nullptr;

