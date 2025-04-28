#ifndef RIMPL_IMPL_H
#define RIMPL_IMPL_H

#include "rimpl_declare.hpp"

// This file provides the implementation for the declaration created by the Rimpl class template

namespace Rimpl {

   // --- Helper classes for static assertions with readable error message ---

   template <size_t type_size>
   struct Type_size {
      static constexpr size_t size = type_size;
   };

   template <typename Type>
   struct Needs_at_least {
      template <typename Buffer>
      static constexpr void as_buffer(Buffer b)
      {
         static_assert(Buffer::size >= Type::size,
            "Buffer is too small for type size:  " __FUNCTION__);
      }
   };

   // --- Rimpl class template implementation --

   template <typename HiddenType_t, size_t buffer_size>
   template <typename... Args>
   Rimpl<HiddenType_t, buffer_size>::Rimpl(Args&&... args)
   {
      using HiddenTypeSize_t = Type_size<sizeof(HiddenType_t)>;
      using BufferSize_t = Type_size<buffer_size>;

      // Ensure that buffer size is sufficient for the hidden type
      Needs_at_least<HiddenTypeSize_t>::as_buffer(BufferSize_t());

      // Ensure that alignement is sufficient for the hidden type
      // Note: This is not strictly necessary, since the buffer is aligned to std::max_align_t. But better to check in case of future changes.
      static_assert(alignment >= alignof(HiddenType_t), "Alignment doesn't match");

      // Construct the hidden type
      new (m_buffer.data()) HiddenType_t(std::forward<Args>(args)...);

      // force instantiation of copy and move assignment operator
      *this = *this;
      *this = std::move(*this);
   }


   template <typename HiddenType_t, size_t buffer_size>
   Rimpl<HiddenType_t, buffer_size>::Rimpl(const Rimpl<HiddenType_t, buffer_size>& original)
   {
      if (!std::is_trivially_destructible<HiddenType_t>::value) {
         // Call the destructor of the hidden type
         get().~HiddenType_t();
      }

      // Construct the hidden type
      new (m_buffer.data()) HiddenType_t(*original);
   }


   template <typename HiddenType_t, size_t buffer_size>
   Rimpl<HiddenType_t, buffer_size>::Rimpl(Rimpl<HiddenType_t, buffer_size>&& original) noexcept
   {
      new (m_buffer.data()) HiddenType_t(std::move(original));
   }


   template <typename HiddenType_t, size_t buffer_size>
   Rimpl<HiddenType_t, buffer_size>::~Rimpl()
   {
      // No need to call destructor for trivially destructible types
      if (!std::is_trivially_destructible<HiddenType_t>::value) {
         // Call the destructor of the hidden type
         get().~HiddenType_t();
      }
   }


   template <typename HiddenType_t, size_t buffer_size>
   Rimpl<HiddenType_t, buffer_size>& Rimpl<HiddenType_t, buffer_size>::operator=(const Rimpl<HiddenType_t, buffer_size>& rhs)
   {
      // Check for self-assignment to avoid unnecessary work that would happen due to forced instantiation the operator
      if (this == &rhs) {
         return *this;
      }

      auto copied{ rhs.get() };

      std::swap(get(), copied);

      return *this;
   }



   template <typename HiddenType_t, size_t buffer_size>
   Rimpl<HiddenType_t, buffer_size>& Rimpl<HiddenType_t, buffer_size>::operator=(Rimpl<HiddenType_t, buffer_size>&& rhs) noexcept
   {
      // Check for self-assignment to avoid unnecessary work that would happen due to forced instantiation the operator
      if (this == &rhs) {
         return *this;
      }

      std::swap(get(), rhs.get());

      return *this;
   }

} // namespace Rimpl

#endif // !RIMPL_IMPL_H
