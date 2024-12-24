#pragma once
#include <memory>

template <typename T>
using Ref = std::shared_ptr<T>;

template <typename T>
using Weak = std::weak_ptr<T>;