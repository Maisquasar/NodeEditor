#pragma once

template <typename T>
using Ref = std::shared_ptr<T>;

template <typename T>
using Weak = std::weak_ptr<T>;