// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "pch.h"
#include "CoreNativeModules.h"

// Modules
#include <AsyncStorageModule.h>
#include <Modules/Animated/NativeAnimatedModule.h>
#include <Modules/AppearanceModule.h>
#include <Modules/AsyncStorageModuleWin32.h>
#include <Modules/ClipboardModule.h>
#include <Modules/NativeUIManager.h>
#include <Modules/PaperUIManagerModule.h>
#include <Threading/MessageQueueThreadFactory.h>

// Shared
#include <CreateModules.h>

namespace Microsoft::ReactNative {

namespace {

bool HasPackageIdentity() noexcept {
  static const bool hasPackageIdentity = []() noexcept {
    auto packageStatics = winrt::get_activation_factory<winrt::Windows::ApplicationModel::IPackageStatics>(
        winrt::name_of<winrt::Windows::ApplicationModel::Package>());
    auto abiPackageStatics = static_cast<winrt::impl::abi_t<winrt::Windows::ApplicationModel::IPackageStatics> *>(
        winrt::get_abi(packageStatics));
    winrt::com_ptr<winrt::impl::abi_t<winrt::Windows::ApplicationModel::IPackage>> dummy;
    return abiPackageStatics->get_Current(winrt::put_abi(dummy)) !=
        winrt::impl::hresult_from_win32(APPMODEL_ERROR_NO_PACKAGE);
  }();

  return hasPackageIdentity;
}

} // namespace

std::vector<facebook::react::NativeModuleDescription> GetCoreModules(
    const std::shared_ptr<facebook::react::MessageQueueThread> &batchingUIMessageQueue,
    const std::shared_ptr<facebook::react::MessageQueueThread>
        &jsMessageQueue, // JS engine thread (what we use for external modules)
    Mso::CntPtr<AppearanceChangeListener> &&appearanceListener,
    Mso::CntPtr<Mso::React::IReactContext> &&context) noexcept {
  std::vector<facebook::react::NativeModuleDescription> modules;

  modules.emplace_back(
      "Networking", []() { return Microsoft::React::CreateHttpModule(); }, jsMessageQueue);

  modules.emplace_back(
      "Timing",
      [batchingUIMessageQueue]() { return facebook::react::CreateTimingModule(batchingUIMessageQueue); },
      batchingUIMessageQueue);

  modules.emplace_back(
      NativeAnimatedModule::name,
      [context = std::move(context)]() mutable { return std::make_unique<NativeAnimatedModule>(std::move(context)); },
      batchingUIMessageQueue);

  modules.emplace_back(
      AppearanceModule::Name,
      [appearanceListener = std::move(appearanceListener)]() mutable {
        return std::make_unique<AppearanceModule>(std::move(appearanceListener));
      },
      jsMessageQueue);

  // AsyncStorageModule doesn't work without package identity (it indirectly depends on
  // Windows.Storage.StorageFile), so check for package identity before adding it.
  modules.emplace_back(
      "AsyncLocalStorage",
      []() -> std::unique_ptr<facebook::xplat::module::CxxModule> {
        if (HasPackageIdentity()) {
          return std::make_unique<facebook::react::AsyncStorageModule>(L"asyncStorage");
        } else {
          return std::make_unique<facebook::react::AsyncStorageModuleWin32>();
        }
      },
      jsMessageQueue);

  return modules;
}

} // namespace Microsoft::ReactNative
