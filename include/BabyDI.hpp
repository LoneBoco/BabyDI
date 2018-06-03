#pragma once

#include <memory>
#include <vector>
#include <algorithm>
#include <tuple>
#include <functional>

#ifndef BABYDI_EMBEDDED
#include <iostream>
#endif

namespace BabyDI {
  struct InjectMetaBase {
    virtual ~InjectMetaBase() {};

    virtual bool MatchesType(size_t typeHash) const = 0;
    virtual void Provide(void* provision) = 0;
    virtual bool ProvideIfMatch(size_t typeHash, void* provision) = 0;

    virtual const char* const GetInterfaceName() const = 0;
  };

  struct InjectionRepository {
    static void AddInjectMeta(InjectMetaBase* injectMeta) {
      m_injections.push_back(std::unique_ptr<InjectMetaBase>(injectMeta));
    };

    template <typename T>
    static void Provide(T* provision) {
      m_injections.erase(
        std::remove_if(m_injections.begin(), m_injections.end(),
          [&](auto& meta) {
            return meta->ProvideIfMatch(typeid(T).hash_code(), provision);
          }
        ),
        m_injections.end()
      );
    };

    template<typename F>
    static void AssertAllProvided(F&& assertCallback) {
      if (m_injections.size() != 0) {
        std::vector<const char*> interfaceNames;

        for (auto& meta : m_injections) {
          interfaceNames.push_back(meta->GetInterfaceName());
        }

        assertCallback(interfaceNames);
      }
    };

    static inline std::vector<std::unique_ptr<InjectMetaBase>> m_injections;
  };

  template <typename T>
  struct InjectMeta : public InjectMetaBase {
    constexpr InjectMeta(T** injectAddress, const char* interfaceName) :
      m_injectAddress(injectAddress), m_interfaceName(interfaceName) {
      ::BabyDI::InjectionRepository::AddInjectMeta(this);
    };

    bool MatchesType(size_t typeHash) const override {
      return typeid(T).hash_code() == typeHash;
    };

    void Provide(void* address) override {
      *m_injectAddress = (T*)address;
    };

    bool ProvideIfMatch(size_t typeHash, void* address) override {
      if (MatchesType(typeHash)) {
        Provide(address);

        return true;
      } else {
        return false;
      }
    };

    const char* const GetInterfaceName() const override {
      return m_interfaceName;
    };

    const char* const m_interfaceName;
    T** const m_injectAddress;
  };

  template<typename F>
  static void AssertAllProvided(F&& assertCallback) {
    InjectionRepository::AssertAllProvided(assertCallback);
  }

#ifndef BABYDI_EMBEDDED
  static void AssertAllProvided() {
    AssertAllProvided([](const std::vector<const char*>& interfaceNames) {
      std::cerr << "BabyDI: Injections not provided:" << std::endl;

      for (const auto& interfaceName : interfaceNames) {
        std::cerr << "  " << interfaceName << std::endl;
      }

      std::terminate();
    });
  }
#endif
};

#define PROVIDE(InterfaceType, Implementation) \
  ::BabyDI::InjectionRepository::Provide<InterfaceType>(Implementation);

#define INJECT_2(InterfaceType, MemberName) \
  using BabyDI_##MemberName##_type = InterfaceType; \
  static inline BabyDI_##MemberName##_type* MemberName = nullptr; \
  static inline ::BabyDI::InjectMeta<BabyDI_##MemberName##_type>* BabyDI_##MemberName##_Meta = new ::BabyDI::InjectMeta<BabyDI_##MemberName##_type>((BabyDI_##MemberName##_type**)&MemberName, #InterfaceType);

#define INJECT_1(InterfaceType) \
  INJECT_2(InterfaceType, InterfaceType)

#define INJECT_SELECT_IMPL(_1, _2, NAME, ...) \
  NAME

#define INJECT(...) \
  INJECT_SELECT_IMPL(__VA_ARGS__, INJECT_2, INJECT_1)(__VA_ARGS__)
