/*
 * Copyright 2018 Google
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

// Right now, FirebaseCredentialsProvider only support APPLE build.
#if !defined(__OBJC__)
#error "This header only supports Objective-C++."
#endif  // !defined(__OBJC__)

#ifndef FIRESTORE_CORE_SRC_FIREBASE_FIRESTORE_AUTH_FIREBASE_CREDENTIALS_PROVIDER_APPLE_H_
#define FIRESTORE_CORE_SRC_FIREBASE_FIRESTORE_AUTH_FIREBASE_CREDENTIALS_PROVIDER_APPLE_H_

#import <Foundation/Foundation.h>

#include <memory>
#include <mutex>  // NOLINT(build/c++11)

#include "Firestore/core/src/firebase/firestore/auth/credentials_provider.h"
#include "Firestore/core/src/firebase/firestore/auth/user.h"
#include "absl/strings/string_view.h"

@class FIRApp;

namespace firebase {
namespace firestore {
namespace auth {

/**
 * `FirebaseCredentialsProvider` uses Firebase Auth via `FIRApp` to get an auth
 * token.
 *
 * NOTE: To simplify the implementation, it requires that you set
 * `userChangeListener` with a non-`nil` value no more than once and don't call
 * `getTokenForcingRefresh:` after setting it to `nil`.
 *
 * This class must be implemented in a thread-safe manner since it is accessed
 * from the thread backing our internal worker queue and the callbacks from
 * FIRAuth will be executed on an arbitrary different thread.
 *
 * For non-Apple desktop build, this is right now just a stub.
 */
class FirebaseCredentialsProvider : public CredentialsProvider {
 public:
  // TODO(zxu123): Provide a ctor to accept the C++ Firebase Games App, which
  // deals all platforms. Right now, only works for FIRApp*.
  /**
   * Initializes a new FirebaseCredentialsProvider.
   *
   * @param app The Firebase app from which to get credentials.
   */
  explicit FirebaseCredentialsProvider(FIRApp* app);

  ~FirebaseCredentialsProvider() override;

  void GetToken(bool force_refresh, TokenListener completion) override;

  void SetUserChangeListener(UserChangeListener listener) override;

 private:
  /**
   * Most contents of the FirebaseCredentialProvider are kept in this
   * Contents object and pointed to with a shared pointer. Callbacks
   * registered with FirebaseAuth use weak pointers to the Contents to
   * avoid races between notifications arriving and C++ object destruction.
   */
  struct Contents {
    Contents(FIRApp* app, const absl::string_view uid)
        : app(app), current_user(uid), mutex() {
    }

    const FIRApp* app;

    /**
     * The current user as reported to us via our AuthStateDidChangeListener.
     */
    User current_user;

    /**
     * Counter used to detect if the user changed while a
     * -getTokenForcingRefresh: request was outstanding.
     */
    int user_counter = 0;

    std::mutex mutex;
  };

  /**
   * Handle used to stop receiving auth changes once userChangeListener is
   * removed.
   */
  id<NSObject> auth_listener_handle_;

  std::shared_ptr<Contents> contents_;
};

}  // namespace auth
}  // namespace firestore
}  // namespace firebase

#endif  // FIRESTORE_CORE_SRC_FIREBASE_FIRESTORE_AUTH_FIREBASE_CREDENTIALS_PROVIDER_APPLE_H_
