#pragma once

namespace detail {

template <template <typename...> class Template>
struct TemplateIdentity {
    template <typename... Ts>
    using type = Template<Ts...>;  // NOLINT
};

}  // namespace detail
